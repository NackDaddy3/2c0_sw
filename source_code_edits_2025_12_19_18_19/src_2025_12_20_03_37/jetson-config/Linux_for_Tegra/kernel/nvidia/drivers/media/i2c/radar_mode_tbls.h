/*
 * radar_mode_tbls.h - radar sensor mode tables
 *
 * Copyright (c) 2020-2022, Leopard CORPORATION.  All rights reserved.
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
#ifndef __RADAR_I2C_TABLES__
#define __RADAR_I2C_TABLES__

#include <media/camera_common.h>
#include <linux/miscdevice.h>

#define RADAR_TABLE_WAIT_MS	0
#define RADAR_TABLE_END	1
#define RADAR_MAX_RETRIES	3
#define RADAR_WAIT_MS_STOP	1
#define RADAR_WAIT_MS_START	30
#define RADAR_WAIT_MS_STREAM	210
#define RADAR_GAIN_TABLE_SIZE 255

/* #define INIT_ET_INSETTING 1 */

#define radar_reg struct reg_8

static radar_reg radar_start[] = {

	{ RADAR_TABLE_END, 0x00 }
};

static radar_reg radar_stop[] = {

	{RADAR_TABLE_END, 0x00 }
};

static  radar_reg radar_1024x256_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_2048x256_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_4096x256_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_8192x256_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_16384x256_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};


static  radar_reg radar_1024x512_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_2048x512_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_4096x512_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_8192x512_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_16384x512_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};


static  radar_reg radar_1024x640_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_2048x640_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_4096x640_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_8192x640_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_16384x640_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};


static  radar_reg radar_1024x1024_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_2048x1024_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_4096x1024_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_8192x1024_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};
static  radar_reg radar_16384x1024_crop_30fps[] = {
	{RADAR_TABLE_END, 0x00}
};


enum {
	RADAR_MODE_START_STREAM,
	RADAR_MODE_STOP_STREAM,
	RADAR_MODE_1024x256_CROP_30FPS,
	RADAR_MODE_2048x256_CROP_30FPS,
	RADAR_MODE_4096x256_CROP_30FPS,
	RADAR_MODE_8192x256_CROP_30FPS,
	RADAR_MODE_16384x256_CROP_30FPS,
	RADAR_MODE_1024x512_CROP_30FPS,
	RADAR_MODE_2048x512_CROP_30FPS,
	RADAR_MODE_4096x512_CROP_30FPS,
	RADAR_MODE_8192x512_CROP_30FPS,
	RADAR_MODE_16384x512_CROP_30FPS,
	RADAR_MODE_1024x640_CROP_30FPS,
	RADAR_MODE_2048x640_CROP_30FPS,
	RADAR_MODE_4096x640_CROP_30FPS,
	RADAR_MODE_8192x640_CROP_30FPS,
	RADAR_MODE_16384x640_CROP_30FPS,
	RADAR_MODE_1024x1024_CROP_30FPS,
	RADAR_MODE_2048x1024_CROP_30FPS,
	RADAR_MODE_4096x1024_CROP_30FPS,
	RADAR_MODE_8192x1024_CROP_30FPS,
	RADAR_MODE_16384x1024_CROP_30FPS,
};

static radar_reg *mode_table[] = {
	[RADAR_MODE_START_STREAM] = radar_start,
	[RADAR_MODE_STOP_STREAM] = radar_stop,
	[RADAR_MODE_1024x256_CROP_30FPS] = radar_1024x256_crop_30fps,
	[RADAR_MODE_2048x256_CROP_30FPS] = radar_2048x256_crop_30fps,
	[RADAR_MODE_4096x256_CROP_30FPS] = radar_4096x256_crop_30fps,
	[RADAR_MODE_8192x256_CROP_30FPS] = radar_8192x256_crop_30fps,
	[RADAR_MODE_16384x256_CROP_30FPS] = radar_16384x256_crop_30fps,
	[RADAR_MODE_1024x512_CROP_30FPS] = radar_1024x512_crop_30fps,
	[RADAR_MODE_2048x512_CROP_30FPS] = radar_2048x512_crop_30fps,
	[RADAR_MODE_4096x512_CROP_30FPS] = radar_4096x512_crop_30fps,
	[RADAR_MODE_8192x512_CROP_30FPS] = radar_8192x512_crop_30fps,
	[RADAR_MODE_16384x512_CROP_30FPS] = radar_16384x512_crop_30fps,
	[RADAR_MODE_1024x640_CROP_30FPS] = radar_1024x640_crop_30fps,
	[RADAR_MODE_2048x640_CROP_30FPS] = radar_2048x640_crop_30fps,
	[RADAR_MODE_4096x640_CROP_30FPS] = radar_4096x640_crop_30fps,
	[RADAR_MODE_8192x640_CROP_30FPS] = radar_8192x640_crop_30fps,
	[RADAR_MODE_16384x640_CROP_30FPS] = radar_16384x640_crop_30fps,
	[RADAR_MODE_1024x1024_CROP_30FPS] = radar_1024x1024_crop_30fps,
	[RADAR_MODE_2048x1024_CROP_30FPS] = radar_2048x1024_crop_30fps,
	[RADAR_MODE_4096x1024_CROP_30FPS] = radar_4096x1024_crop_30fps,
	[RADAR_MODE_8192x1024_CROP_30FPS] = radar_8192x1024_crop_30fps,
	[RADAR_MODE_16384x1024_CROP_30FPS] = radar_16384x1024_crop_30fps,
};

static const int radar_30fps[] = {
	30,
};

/*
 * WARNING: frmfmt ordering need to match mode definition in
 * device tree!
 */
static const struct camera_common_frmfmt radar_frmfmt[] = {
	{{1024, 256}, radar_30fps, 1, 0, RADAR_MODE_1024x256_CROP_30FPS},
	{{2048, 256}, radar_30fps, 1, 0, RADAR_MODE_2048x256_CROP_30FPS},
	{{4096, 256}, radar_30fps, 1, 0, RADAR_MODE_4096x256_CROP_30FPS},
	{{8192, 256}, radar_30fps, 1, 0, RADAR_MODE_8192x256_CROP_30FPS},
	{{16384, 256}, radar_30fps, 1, 0, RADAR_MODE_16384x256_CROP_30FPS},
	{{1024, 512}, radar_30fps, 1, 0, RADAR_MODE_1024x512_CROP_30FPS},
	{{2048, 512}, radar_30fps, 1, 0, RADAR_MODE_2048x512_CROP_30FPS},
	{{4096, 512}, radar_30fps, 1, 0, RADAR_MODE_4096x512_CROP_30FPS},
	{{8192, 512}, radar_30fps, 1, 0, RADAR_MODE_8192x512_CROP_30FPS},
	{{16384, 512}, radar_30fps, 1, 0, RADAR_MODE_16384x512_CROP_30FPS},
	{{1024, 640}, radar_30fps, 1, 0, RADAR_MODE_1024x640_CROP_30FPS},
	{{2048, 640}, radar_30fps, 1, 0, RADAR_MODE_2048x640_CROP_30FPS},
	{{4096, 640}, radar_30fps, 1, 0, RADAR_MODE_4096x640_CROP_30FPS},
	{{8192, 640}, radar_30fps, 1, 0, RADAR_MODE_8192x640_CROP_30FPS},
	{{16384, 640}, radar_30fps, 1, 0, RADAR_MODE_16384x640_CROP_30FPS},
	{{1024, 1024}, radar_30fps, 1, 0, RADAR_MODE_1024x1024_CROP_30FPS},
	{{2048, 1024}, radar_30fps, 1, 0, RADAR_MODE_2048x1024_CROP_30FPS},
	{{4096, 1024}, radar_30fps, 1, 0, RADAR_MODE_4096x1024_CROP_30FPS},
	{{8192, 1024}, radar_30fps, 1, 0, RADAR_MODE_8192x1024_CROP_30FPS},
	{{16384, 1024}, radar_30fps, 1, 0, RADAR_MODE_16384x1024_CROP_30FPS},
	/* Add modes with no device tree support after below */
};
#endif /* __RADAR_I2C_TABLES__ */

