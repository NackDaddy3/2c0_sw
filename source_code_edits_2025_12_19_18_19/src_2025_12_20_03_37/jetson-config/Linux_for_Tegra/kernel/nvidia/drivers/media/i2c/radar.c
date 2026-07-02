/*
 * radar.c - radar sensor driver
 *
 * Copyright (c) 2020-2023, Leopard CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include <media/tegra_v4l2_camera.h>
#include <media/tegracam_core.h>
#include "radar_mode_tbls.h"
#define CREATE_TRACE_POINTS

#include <linux/i2c.h>
#include <linux/err.h>

#define RADAR_MIN_FRAME_LENGTH	(1125)
#define RADAR_MAX_FRAME_LENGTH	(0x1FFFF)
#define RADAR_MIN_SHS1_1080P_HDR	(5)
#define RADAR_MIN_SHS2_1080P_HDR	(82)
#define RADAR_MAX_SHS2_1080P_HDR	(RADAR_MAX_FRAME_LENGTH - 5)
#define RADAR_MAX_SHS1_1080P_HDR	(RADAR_MAX_SHS2_1080P_HDR / 16)

#define RADAR_FRAME_LENGTH_ADDR_MSB		0x301A
#define RADAR_FRAME_LENGTH_ADDR_MID		0x3019
#define RADAR_FRAME_LENGTH_ADDR_LSB		0x3018
#define RADAR_COARSE_TIME_SHS1_ADDR_MSB	0x3022
#define RADAR_COARSE_TIME_SHS1_ADDR_MID	0x3021
#define RADAR_COARSE_TIME_SHS1_ADDR_LSB	0x3020
#define RADAR_COARSE_TIME_SHS2_ADDR_MSB	0x3025
#define RADAR_COARSE_TIME_SHS2_ADDR_MID	0x3024
#define RADAR_COARSE_TIME_SHS2_ADDR_LSB	0x3023
#define RADAR_GAIN_ADDR					0x3014
#define RADAR_GROUP_HOLD_ADDR				0x3001
#define RADAR_SW_RESET_ADDR			0x3003
#define OX40_X8B_ISP_VERSION				0x80FE
// extern int max929x_switch_channel(int channel, int phy_num, bool val);
extern int max929x_setup_streaming(u32 channel);

static const struct of_device_id radar_of_match[] = {
	{ .compatible = "infineon,radar",},
	{ },
};
MODULE_DEVICE_TABLE(of, radar_of_match);

static const u32 ctrl_cid_list[] = {
	TEGRA_CAMERA_CID_GAIN,
	TEGRA_CAMERA_CID_EXPOSURE,
	TEGRA_CAMERA_CID_FRAME_RATE,
	TEGRA_CAMERA_CID_SENSOR_MODE_ID,
};

struct radar {
	struct i2c_client	*i2c_client;
	struct v4l2_subdev	*subdev;
	u32				frame_length;
	u32				port_index;
	s64 last_wdr_et_val;
	struct camera_common_data	*s_data;
	struct tegracam_device		*tc_dev;
	u32 channel;
	int phy_num;
};

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
#if KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE
	.use_single_rw = true,
#else
	.use_single_read = true,
	.use_single_write = true,
#endif
};

static inline struct i2c_client *radar_get_client(struct camera_common_data *s_data)
{
	/* Typical case: camera_common_data already holds i2c_client pointer */
	/* If not, you can use: return to_i2c_client(s_data->dev); */
	return ((struct radar *)(s_data->priv))->i2c_client;
}

/* Pack 16-bit register address as big-endian (most devices use this) */
static inline void radar_pack_addr_be(u16 addr, u8 *dst)
{
	dst[0] = (addr >> 8) & 0xFF;
	dst[1] = addr & 0xFF;
}

static inline int radar_read_reg(struct camera_common_data *s_data, u16 addr, u8 *val)
{
	int ret;
	u8 addr_buf[2];
	struct i2c_client *client = radar_get_client(s_data);
	struct device *dev = s_data->dev;
	struct i2c_msg msgs[2];

	if (!client || !client->adapter)
		return -ENODEV;

	radar_pack_addr_be(addr, addr_buf);

	/* First message: write register address */
	msgs[0].addr  = client->addr;
	msgs[0].flags = 0;
	msgs[0].len   = sizeof(addr_buf);   /* 16-bit register address */
	msgs[0].buf   = addr_buf;

	/* Second message: read register data */
	msgs[1].addr  = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len   = 1;                  /* 8-bit data */
	msgs[1].buf   = val;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(dev, "%s: i2c read failed, reg=0x%04x, ret=%d\n",
			__func__, addr, ret);
		return (ret < 0) ? ret : -EIO;
	}

	return 0;
}

static int radar_write_reg(struct camera_common_data *s_data, u16 addr, u8 val)
{
	int ret;
	u8 buf[3];
	struct i2c_client *client = radar_get_client(s_data);
	struct device *dev = s_data->dev;
	struct i2c_msg msg;

	if (!client || !client->adapter)
		return -ENODEV;

	radar_pack_addr_be(addr, buf);
	buf[2] = val;                       /* 8-bit data */

	/* Single message: write (2-byte address + 1-byte data) */
	msg.addr  = client->addr;
	msg.flags = 0;
	msg.len   = sizeof(buf);            /* total length: address + data */
	msg.buf   = buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1) {
		dev_err(dev, "%s: i2c write failed, reg=0x%04x, val=0x%02x, ret=%d\n",
			__func__, addr, val, ret);
		return (ret < 0) ? ret : -EIO;
	}

	return 0;
}

static int radar_read_reg_with_slaveaddr8(struct camera_common_data *s_data, u8 slaveaddr8, u16 addr, u8 *val)
{
	struct i2c_client *client = radar_get_client(s_data);
	unsigned short addrBAK;
	int ret;

	addrBAK = client->addr;
	client->addr = slaveaddr8>>1;
	ret = radar_read_reg(s_data,addr,val);
	client->addr = addrBAK;
	return ret;
}

static int radar_write_reg_with_slaveaddr8(struct camera_common_data *s_data, u8 slaveaddr8, u16 addr, u8 val)
{
	struct i2c_client *client = radar_get_client(s_data);
	unsigned short addrBAK;
	int ret;

	addrBAK = client->addr;
	client->addr = slaveaddr8>>1;
	ret = radar_write_reg(s_data,addr,val);
	client->addr = addrBAK;
	return ret;
}


static int radar_write_table(struct radar *priv,
				const radar_reg table[])
{
	struct camera_common_data *s_data = priv->s_data;

	return regmap_util_write_table_8(s_data->regmap,
					 table,
					 NULL, 0,
					 RADAR_TABLE_WAIT_MS,
					 RADAR_TABLE_END);
}

static int radar_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	return 0;
}

static int radar_set_gain(struct tegracam_device *tc_dev, s64 val)
{
	return 0;
}

static int radar_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{
	return 0;
}

static int radar_set_exposure(struct tegracam_device *tc_dev, s64 val)
{
	return 0;
}

static struct tegracam_ctrl_ops radar_ctrl_ops = {
	.numctrls = ARRAY_SIZE(ctrl_cid_list),
	.ctrl_cid_list = ctrl_cid_list,
	.set_gain = radar_set_gain,
	.set_exposure = radar_set_exposure,
	.set_frame_rate = radar_set_frame_rate,
	.set_group_hold = radar_set_group_hold,
};

static int radar_power_on(struct camera_common_data *s_data)
{
	int err = 0;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct device *dev = s_data->dev;

	dev_dbg(dev, "%s: power on\n", __func__);
	if (pdata && pdata->power_on) {
		err = pdata->power_on(pw);
		if (err)
			dev_err(dev, "%s failed.\n", __func__);
		else
			pw->state = SWITCH_ON;
		return err;
	}

	pw->state = SWITCH_ON;
	return 0;

}

static int radar_power_off(struct camera_common_data *s_data)
{
	int err = 0;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct device *dev = s_data->dev;

	dev_dbg(dev, "%s: power off\n", __func__);

	if (pdata && pdata->power_off) {
		err = pdata->power_off(pw);
		if (!err)
			goto power_off_done;
		else
			dev_err(dev, "%s failed.\n", __func__);
		return err;
	}

power_off_done:
	pw->state = SWITCH_OFF;

	return 0;
}

static int radar_power_get(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	const char *mclk_name;
	struct clk *parent;
	int err = 0;

	mclk_name = pdata->mclk_name ?
		    pdata->mclk_name : "extperiph1";
	pw->mclk = devm_clk_get(dev, mclk_name);
	if (IS_ERR(pw->mclk)) {
		dev_err(dev, "unable to get clock %s\n", mclk_name);
		return PTR_ERR(pw->mclk);
	}

	parent = devm_clk_get(dev, "pllp_grtba");
	if (IS_ERR(parent))
		dev_err(dev, "devm_clk_get failed for pllp_grtba");
	else
		clk_set_parent(pw->mclk, parent);


	pw->state = SWITCH_OFF;
	return err;
}

static int radar_power_put(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;

	if (unlikely(!pw))
		return -EFAULT;

	return 0;
}

static struct camera_common_pdata *radar_parse_dt(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct device_node *np = dev->of_node;
	struct camera_common_pdata *board_priv_pdata;
	const struct of_device_id *match;
	int err;

	if (!np)
		return NULL;

	match = of_match_device(radar_of_match, dev);
	if (!match) {
		dev_err(dev, "Failed to find matching dt id\n");
		return NULL;
	}

	board_priv_pdata = devm_kzalloc(dev,
					sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;

	err = of_property_read_string(np, "mclk",
				      &board_priv_pdata->mclk_name);
	if (err)
		dev_err(dev, "mclk not in DT\n");

	return board_priv_pdata;
}

static int radar_set_mode(struct tegracam_device *tc_dev)
{
	struct radar *priv = (struct radar *)tegracam_get_privdata(tc_dev);
	struct camera_common_data *s_data = tc_dev->s_data;
	struct device *dev = tc_dev->dev;
	const struct of_device_id *match;
	int err;

	const struct sensor_mode_properties *mode = NULL;
	int idx = s_data->mode_prop_idx;

	match = of_match_device(radar_of_match, dev);
	if (!match) {
		dev_err(dev, "Failed to find matching dt id\n");
		return -EINVAL;
	}

	dev_info(dev, "radar_set_mode ModeIdx = %d\n",
		 s_data->mode_prop_idx);

	err = radar_write_table(priv, mode_table[s_data->mode_prop_idx]);
	if (err)
		return err;

	// Debug Infos
	mode = &s_data->sensor_props.sensor_modes[idx];
	dev_info(dev, "radar mipi_clock = %llu\n",
		 mode->signal_properties.mipi_clock.val);
	dev_info(dev, "radar pixel_clock = %llu\n",
		 mode->signal_properties.pixel_clock.val);
	dev_info(dev, "radar width = %d\n",
			 mode->image_properties.width);
	dev_info(dev, "radar height = %d\n",
			 mode->image_properties.height);
	dev_info(dev, "radar line_length = %d\n",
		 mode->image_properties.line_length);

	return 0;
}

static int radar_start_streaming(struct tegracam_device *tc_dev)
{
	struct radar *priv = (struct radar *)tegracam_get_privdata(tc_dev);
	struct device *dev = tc_dev->dev;
	int err;
	u8 val;

	err = radar_write_table(priv,
		mode_table[RADAR_MODE_START_STREAM]);
	if (err)
		return err;

	/*
	* Note: Enable the phy and video pipe according to port index
	* but do not disable them in stop_streaming to avoid losing
	* the first frame after stream on.
	*/
	err = radar_read_reg_with_slaveaddr8(priv->s_data,0x80,0x0002,&val);
	if(err){
		dev_err(dev, "Failed to read 0x80's 0x0002 regisger at radar_start_streaming\n");
		return -1;
	}

	if(priv->port_index){
		val |= 0x20;
	}else{
		val |= 0x10;
	}
	err = radar_write_reg_with_slaveaddr8(priv->s_data,0x80,0x0002,val);
	if(err){
		dev_err(dev, "Failed to write 0x80's 0x0002 regisger at radar_start_streaming\n");
		return -1;
	}

	err = radar_read_reg_with_slaveaddr8(priv->s_data,0x90,0x0332,&val);
	if(err){
		dev_err(dev, "Failed to read 0x90's 0x0332 regisger at radar_start_streaming\n");
		return -1;
	}

	if(priv->port_index){
		val |= 0xC0;
	}else{
		val |= 0x30;
	}
	err = radar_write_reg_with_slaveaddr8(priv->s_data,0x90,0x0332,val);
	if(err){
		dev_err(dev, "Failed to write 0x90's 0x0332 regisger at radar_start_streaming\n");
		return -1;
	}


	return 0;
}

static int radar_stop_streaming(struct tegracam_device *tc_dev)
{
	struct radar *priv = (struct radar *)tegracam_get_privdata(tc_dev);
	int err;

	err = radar_write_table(priv, mode_table[RADAR_MODE_STOP_STREAM]);
	if (err)
		return err;

	/* NOTE:
	 * We intentionally do NOT toggle the max9295d serializer (0x80:0x0002) video pipe
	 * nor the deserializer PHY standby (0x90:0x0332) during start_streaming.
	 * Doing so here caused the very first captured frame after v4l2 start
	 * to be truncated by one line, producing CHANSEL_SHORT_FRAME.
	 * Probably the root cause: link/pipe enable latency overlaps first sensor frame.
	 * Since the application prefers to avoid losing the first frame and
	 * continuous power/EMI savings are not critical (for the moment), we leave pipe/PHY
	 * enabled. If low-power cycling is needed later, implement a pre-stream enable
	 * sequence with stabilization delay before sensor start rather than here.
	 */
	 
	return 0;
}


static struct camera_common_sensor_ops radar_common_ops = {
	.numfrmfmts = ARRAY_SIZE(radar_frmfmt),
	.frmfmt_table = radar_frmfmt,
	.power_on = radar_power_on,
	.power_off = radar_power_off,
	.write_reg = radar_write_reg,
	.read_reg = radar_read_reg,
	.parse_dt = radar_parse_dt,
	.power_get = radar_power_get,
	.power_put = radar_power_put,
	.set_mode = radar_set_mode,
	.start_streaming = radar_start_streaming,
	.stop_streaming = radar_stop_streaming,
};

static int radar_board_setup(struct radar *priv)
{
	struct camera_common_data *s_data = priv->s_data;
	struct device *dev = s_data->dev;
	int err = 0;

	dev_dbg(dev, "%s++\n", __func__);

	err = camera_common_mclk_enable(s_data);
	if (err) {
		dev_err(dev,
			"Error %d turning on mclk\n", err);
		return err;
	}

	err = radar_power_on(s_data);
	if (err) {
		dev_err(dev,
			"Error %d during power on sensor\n", err);
		return err;
	}

	radar_power_off(s_data);
	camera_common_mclk_disable(s_data);
	return err;
}

static int radar_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "%s:\n", __func__);

	return 0;
}

static const struct v4l2_subdev_internal_ops radar_subdev_internal_ops = {
	.open = radar_open,
};

static int radar_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct tegracam_device *tc_dev;
	struct device_node *node = dev->of_node;
	struct radar *priv;
	int err;

	dev_info(dev, "probing v4l2 sensor\n");

	if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
		return -EINVAL;

	priv = devm_kzalloc(dev,
			sizeof(struct radar), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	tc_dev = devm_kzalloc(dev,
			sizeof(struct tegracam_device), GFP_KERNEL);
	if (!tc_dev)
		return -ENOMEM;

	priv->i2c_client = tc_dev->client = client;
	tc_dev->dev = dev;
	strncpy(tc_dev->name, "radar", sizeof(tc_dev->name));
	tc_dev->dev_regmap_config = &sensor_regmap_config;
	tc_dev->sensor_ops = &radar_common_ops;
	tc_dev->v4l2sd_internal_ops = &radar_subdev_internal_ops;
	tc_dev->tcctrl_ops = &radar_ctrl_ops;

	err = tegracam_device_register(tc_dev);
	if (err) {
		dev_err(dev, "tegra camera driver registration failed\n");
		return err;
	}
	priv->tc_dev = tc_dev;
	priv->s_data = tc_dev->s_data;
	priv->subdev = &tc_dev->s_data->subdev;
	tegracam_set_privdata(tc_dev, (void *)priv);

	err = radar_board_setup(priv);
	if (err) {
		tegracam_device_unregister(tc_dev);
		dev_err(dev, "board setup failed\n");
		return err;
	}

	err = of_property_read_u32(node, "port-index", &priv->port_index);
	if (priv->port_index < 0 || priv->port_index > 4) {
		dev_err(dev, "port index exceeds\n");
	}
	err = tegracam_v4l2subdev_register(tc_dev, true);
	if (err) {
		dev_err(dev, "tegra camera subdev registration failed\n");
		return err;
	}

	dev_info(dev, "Detected Infineon Radar device\n");

	return 0;
}

static int
radar_remove(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct radar *priv = (struct radar *)s_data->priv;

	tegracam_v4l2subdev_unregister(priv->tc_dev);
	tegracam_device_unregister(priv->tc_dev);

	return 0;
}

static const struct i2c_device_id radar_id[] = {
	{ "radar", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, radar_id);

static struct i2c_driver radar_i2c_driver = {
	.driver = {
		.name = "Radar",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(radar_of_match),
	},
	.probe = radar_probe,
	.remove = radar_remove,
	.id_table = radar_id,
};

module_i2c_driver(radar_i2c_driver);

MODULE_DESCRIPTION("Media Controller driver for OV OX40_X8B");
MODULE_AUTHOR("Guoxin Wu <guoxinw@leopardimaging.com>");
MODULE_LICENSE("GPL v2");
