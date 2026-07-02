/*
 * (c) (2022-2025), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../HwDefinitions.h"
#include "8191_b11/IfxRfe_Specific.h"
#include "GmslSpiWrapper.h"
#include "IfxRfe.h"
#include "IfxRfe_FirmwareCommands.h"
#include "IfxRfe_I2cWrapper.h"
#include "IfxRfe_SpiWrapper.h"
#include "Max2043xPec.h"
#include "PlatformGpio.h"
#include "PlatformI2c.h"
#include "PlatformLogCallbacks.h"
#include "PlatformTime.h"
#include "config.h"
#include "sequencer_data.h"

// Constants
#define CTRX_DEV_COUNT           2
#define DEVICE_PRIMARY           0
#define DEVICE_SECONDARY         1
#define EXPECTED_OPERATION_STATE 0x10000000
#define EXPECTED_INITIAL_STATE   0x20000000
#define SPI_CLOCK_SPEED          3125000  // TODO: use max possible
#define RAMP_SCENARIO_TIMEOUT_US 20000000
#define LOOP_DELAY_US            100000
#define RESET_DELAY_US           5000   // 5ms delay after reset toggle
#define FINISH_RAMP_DELAY_US     50000  // 50ms delay to finish ramp scenario
#define NUM_ITERATIONS           50
#define LO_CONFIG_PRIMARY        15  // Primary using LOIN 2
#define LO_CONFIG_SECONDARY      4   // Secondary using LOIN 1


//AG: 
#define NUM_RX 4
#define NUM_RAMPS_DEV 512
#define NUM_DEV 2
#define NUM_RAMPS_TOTAL NUM_RAMPS_DEV*NUM_DEV
#define FPS 30

#define EXIT_ON_IFXRFE_ERROR(expression)                                                                                                     \
    {                                                                                                                                        \
        error_t const ret_code = expression;                                                                                                 \
        if (ret_code != IFXRFE_E_SUCCESS)                                                                                                    \
        {                                                                                                                                    \
            cleanup_platform();                                                                                                              \
            printf("Error in function %s() in file %s:%d -> IfxRfe returned %d (0x%X)\n", __func__, __FILE__, __LINE__, ret_code, ret_code); \
            return ret_code;                                                                                                                 \
        }                                                                                                                                    \
    }

/**
 * @brief Cleanup platform resources
 * 
 * Deinitializes all platform components.
 * This function should be called before program termination or on error
 * to ensure proper resource cleanup.
 * 
 * Resources cleaned up:
 * - I2C interface
 * - SPI interface  
 * - GPIO interface
 */
static void cleanup_platform(void)
{
    PlatformI2c_deinit();     // Deinitialize I2C interface
    GmslSpiWrapper_deinit();  // Deinitialize SPI interface
    PlatformGpio_deinit();    // Deinitialize GPIO interface
}

/**
 * @brief Perform the power up sequence
 *
 * Performs the power up sequence for
 * the CARKIT2C0 board
 *
 * @return IFXRFE_E_SUCCESS on success
 * @return Error code (non-zero) if any step fails
 */
static error_t power_up(void)
{
    // Setup PMIC devices for power management
    Max2043xPec_t pmicA = {.devAddrI2c = 0x3A};  // PMIC A at I2C address 0x3A
    Max2043xPec_t pmicB = {.devAddrI2c = 0x3B};  // PMIC B at I2C address 0x3B

    // Configure PMIC_B voltage output 2 to 1.3V (default is 1.8V)
    EXIT_ON_IFXRFE_ERROR(Max2043x_setVoltageOutput(&pmicB, MAX2043x_VOLTAGE_OUT2, 1300));

    // Enable required voltage outputs on both PMICs
    EXIT_ON_IFXRFE_ERROR(Max2043x_switchVoltageOutput(&pmicA, MAX2043x_VOLTAGE_OUT2, true));
    EXIT_ON_IFXRFE_ERROR(Max2043x_switchVoltageOutput(&pmicA, MAX2043x_VOLTAGE_OUT4, true));
    EXIT_ON_IFXRFE_ERROR(Max2043x_switchVoltageOutput(&pmicB, MAX2043x_VOLTAGE_OUT2, true));
    EXIT_ON_IFXRFE_ERROR(Max2043x_switchVoltageOutput(&pmicB, MAX2043x_VOLTAGE_OUT4, true));

    return IFXRFE_E_SUCCESS;
}

/**
 * @brief Print device error status information
 * 
 * This function displays detailed error status information for debugging
 * purposes when CTRX initialization or operation encounters issues.
 * 
 * @param deviceName Human-readable device name for output (e.g., "CTRX_A")
 * @param result Pointer to error result structure
 */
static void print_error_status(const char *deviceName, const IfxRfe_handleErrorResult_t *result)
{
    printf("HandleError %s after initialization\n", deviceName);
    printf("-------------------------------------\n");
    printf("\tError Status 1 Word 0: 0x%X\n", result->error_status_1);
    for (int i = 0; i < result->length; i++)
    {
        printf("\tError Status 2 Word %d: 0x%X\n", i, result->error_status_2[i]);
    }
    printf("-------------------------------------\n");
}

/**
 * @brief Main application entry point
 * 
 * This function demonstrates the complete initialization and operation sequence
 * for a radar system using two CTRXs. The application performs the following
 * major steps:
 * 
 * 1. Platform initialization (I2C, SPI, GPIO interfaces)
 * 2. IfxRfe library initialization with function callbacks
 * 3. PMIC configuration for power management
 * 4. CTRX status checking and conditional initialization
 * 5. RF parameter configuration for both CTRXs
 * 6. CTRX transition to operational state
 * 7. TX calibration execution on both CTRXs
 * 8. Continuous radar measurement cycles (ramp scenarios)
 * 9. Final cleanup and power down
 * 
 * The application is designed to work with two CTRX radar devices in a
 * primary/secondary configuration, commonly used in automotive radar systems.
 * 
 * @return EXIT_SUCCESS (0) on successful completion
 * @return Error code (non-zero) if any step fails
 */
int main(void)
{
    printf("=== IfxRfe App - CARKIT2C0 Example ===\n");

    // =============== Initialize I2C Interface ===================
    // Initialize I2C bus 2 for communication
    EXIT_ON_IFXRFE_ERROR(PlatformI2c_init(2));

    // =============== Configure GPIO Pins ===================
    // Setup control and status pins for the CTRX devices

    // RFT (Ready for Transfer) pins - indicate when CTRXs are ready for SPI communication
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_RFT_A, GPIO_FLAG_INPUT_ENABLE | GPIO_FLAG_PULL_DOWN));
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_RFT_B, GPIO_FLAG_INPUT_ENABLE | GPIO_FLAG_PULL_DOWN));

    // Reset pins
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_RES_N_A, GPIO_FLAG_OUTPUT_DRIVE_HIGH | GPIO_FLAG_PULL_DOWN));
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_RES_N_B, GPIO_FLAG_OUTPUT_DRIVE_HIGH | GPIO_FLAG_PULL_DOWN));



    // Status monitoring pins
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_OK, GPIO_FLAG_INPUT_ENABLE | GPIO_FLAG_PULL_DOWN));
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_SPI_BNE, GPIO_FLAG_INPUT_ENABLE | GPIO_FLAG_PULL_DOWN));

    // Read-only SPI pin
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_SPI_RO, GPIO_FLAG_OUTPUT_DRIVE_LOW | GPIO_FLAG_PULL_UP));

    // DMUX1 pin for primary device
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_configure(GPIO_ID_DMUX1_A, GPIO_FLAG_OUTPUT_DRIVE_HIGH | GPIO_FLAG_PULL_DOWN));  // Need to change tunneling direction, then here

    // =============== Setup SPI Communication ===================
    // Configure SPI interface mapping for communication with CTRXs
    uint8_t deviceIds[] = {DEVICE_PRIMARY, DEVICE_SECONDARY};  // Device indices

    // Map devices to SPI interfaces
    GmslSpiWrapper_setConfig(deviceIds, sizeof(deviceIds) / sizeof(deviceIds[0]), GPIO_ID_SPI_RO, GPIO_ID_SPI_BNE);

    // =============== Initialize IfxRfe ===================
    // Setup function callbacks for SPI, GPIO, timing, and logging operations

    // SPI function callbacks
    IfxRfe_spiFunctions_t spiFncs = {
        .spiConfigure = GmslSpiWrapper_configure,
        .spiTransfer  = GmslSpiWrapper_transfer,
        .spiWrite     = GmslSpiWrapper_write};

    // GPIO function callbacks - handle pin control and status reading
    IfxRfe_gpioFunctions_t gpioFncs = {
        .gpioGet            = PlatformGpio_get,
        .gpioSet            = PlatformGpio_set,
        .gpioConfigure      = PlatformGpio_configure,
        .isGpioConfigOutput = PlatformGpio_isGpioConfigOutput};

    // Timing function callbacks - handle delays and timeouts
    IfxRfe_timeFunctions_t timeFncs = {
        .getDeadLine = PlatformTime_getDeadLine,
        .now         = PlatformTime_now,
        .waitTime    = PlatformTime_waitTime};

    // Logging function callbacks - handle debug output
    IfxRfe_logCallbacks_t logInterface = {
        .errorLog   = PlatformLogCallbacks_error,
        .warningLog = PlatformLogCallbacks_warning,
        .infoLog    = PlatformLogCallbacks_info};

    // Define GPIO pin mapping for each CTRX device
    uint16_t const GPIO_UNUSED                     = 0;
    IfxRfe_gpioDefinitions_t gpios[CTRX_DEV_COUNT] = {
        {.spiRftId = GPIO_ID_RFT_A,  // Primary device RFT pin
         .okId     = GPIO_ID_OK,
         .dmux1Id  = GPIO_ID_DMUX1_A,
         .dmux2Id  = GPIO_UNUSED,
         .dmux3Id  = GPIO_UNUSED},
        {.spiRftId = GPIO_ID_RFT_B,  // Secondary device RFT pin
         .okId     = GPIO_ID_OK,
         .dmux1Id  = GPIO_UNUSED,
         .dmux2Id  = GPIO_UNUSED,
         .dmux3Id  = GPIO_UNUSED},
    };

    // Initialize IfxRfe with all configuration parameters
    EXIT_ON_IFXRFE_ERROR(
        IfxRfe_init(
            CTRX_DEV_COUNT,
            gpios,
            spiFncs,
            gpioFncs,
            timeFncs,
            logInterface));

    // =============== PMIC and Configuration Setup ===================
    // Initialize I2C wrapper
    IfxRfe_i2cFunctions_t i2cFncs = {
        .i2cRead  = PlatformI2c_read,
        .i2cWrite = PlatformI2c_write};
    InitI2cWrapper(i2cFncs);

    // Initialize device configuration parameters
    IfxRfe_demoConfigParams_t configParamsPrim;  // Primary device config
    IrfeDemoConfigInit(&configParamsPrim, PRIMARY);

    IfxRfe_demoConfigParams_t configParamsSec;  // Secondary device config
    IrfeDemoConfigInit(&configParamsSec, SECONDARY);

    // Configure SPI clock speed to 3.125MHz using mode 1 (CPOL=0, CPHA=1)
    uint8_t flags = IFXRFE_SPI_MODE_0;
    Wrapper_SpiConfigure(flags, SPI_CLOCK_SPEED);

    // =============== Check CTRX Status ===================
    // Check if CTRXs are already initialized or need setup
    IfxRfe_getStatusResult_t statusPrimary   = {0};
    IfxRfe_getStatusResult_t statusSecondary = {0};
    IfxRfe_getStatusResult_t status          = {0};

    EXIT_ON_IFXRFE_ERROR(power_up());  // Configure PMICs and power up devices
    printf("PMIC voltage configuration completed\n");

    // Reset both devices by toggling reset pin
    // Toggle RESET_N to properly initialize both CTRX devices
    // TODO: needed for GMSL??
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_set(GPIO_ID_RES_N_A, false));  // Assert reset for primary
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_set(GPIO_ID_RES_N_B, false));  // Assert reset for secondary
    usleep(RESET_DELAY_US);
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_set(GPIO_ID_RES_N_A, true));  // Release reset for primary
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_set(GPIO_ID_RES_N_B, true));  // Release reset for secondary
    usleep(RESET_DELAY_US);                                         // Needed to make it stable

    // Get status from primary device
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_getStatus(&statusPrimary));

    // Get status from secondary device
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_getStatus(&statusSecondary));

    printf("Initial status - CTRX_A (Primary): %#010x\n", statusPrimary.curr_state);
    printf("Initial status - CTRX_B (Secondary): %#010x\n", statusSecondary.curr_state);

    // =============== CTRX Initialization  ===================
    printf("Performing initialization of the CTRXs...\n");

    static uint32_t intitializeIndex[] = {16, 19, 207, 18, 20, 208, 18};
    static uint32_t intitializeValue[] = {0, 0x86, LO_CONFIG_PRIMARY, 11, 15, 256, 11};  // Init for PRIMARY

    // Initialize CTRX_A (Primary)
    printf("# Initializing CTRX_A (Primary)...\n");
    IfxRfe_ctrxInitConfiguration_t configPrimary = prepareCtrxInitConfig(false, PRIMARY);
    // Overwrite default initialize config
    configPrimary.initializeConfig = (IfxRfe_initialize_t) {
        .index  = intitializeIndex,
        .value  = intitializeValue,
        .length = sizeof(intitializeIndex) / sizeof(intitializeIndex[0])};
    configPrimary.spiConfigureFlags = IFXRFE_SPI_MODE_0;
    configPrimary.spiClockSpeed     = SPI_CLOCK_SPEED;
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_ctrxInit(configPrimary));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_getStatus(&status));
    printf("Status of CTRX_A (Primary) after initialization: %#010x\n", status.curr_state);


    // Check for errors during primary device initialization
    IfxRfe_handleError_t errorRequest = {
        .action               = 1,
        .error_mask_1         = 0,
        .error_mask_1_present = 0,
        .length               = 0,
        .error_mask_2         = 0};
    IfxRfe_handleErrorResult_t errorResult;
    EXIT_ON_IFXRFE_ERROR(IfxRfe_handleError(errorRequest, &errorResult));
    print_error_status("CTRX_A (Primary)", &errorResult);

    // Initialize CTRX_B (Secondary)
    printf("# Initializing CTRX_B (Secondary)...\n");
    IfxRfe_ctrxInitConfiguration_t configSecondary = prepareCtrxInitConfig(false, SECONDARY);
    // Overwrite default initialize config
    intitializeValue[2]              = LO_CONFIG_SECONDARY;  // Change to secondary config
    intitializeValue[5]              = 4294967040;
    intitializeValue[6]              = 11;
    configSecondary.initializeConfig = (IfxRfe_initialize_t) {
        .index  = intitializeIndex,
        .value  = intitializeValue,
        .length = sizeof(intitializeIndex) / sizeof(intitializeIndex[0])};
    configSecondary.spiConfigureFlags = IFXRFE_SPI_MODE_0;
    configSecondary.spiClockSpeed     = SPI_CLOCK_SPEED;
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_ctrxInit(configSecondary));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_getStatus(&status));
    printf("Status of CTRX_B (Secondary) after initialization: %#010x\n", status.curr_state);

    // Check the and-ed OK pins
    bool okPinState = false;
    EXIT_ON_IFXRFE_ERROR(PlatformGpio_get(GPIO_ID_OK, &okPinState));
    if (!okPinState)
    {
        printf("The AND-ed OK pin is LOW, not both CTRX devices initialized successfully.\n");
        return IFXRFE_E_FAILED;
    }


    // Check for errors during secondary device initialization
    EXIT_ON_IFXRFE_ERROR(IfxRfe_handleError(errorRequest, &errorResult));
    print_error_status("CTRX_B (Secondary)", &errorResult);

    // =============== RF Configuration ===================
    // Configure RF parameters and sequencer data for both CTRX devices

    // Configure CTRX_A (Primary) RF parameters
    printf("Configuring CTRX_A (Primary) RF parameters...\n");
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_loadSequencerData(0, seqData_prim, sizeof(seqData_prim) / sizeof(uint32_t)));
    printf("Sequencer memory configured for CTRX_A (Primary)\n");

    // Configure ramp scenario, TX power, RX, and RF frequency for primary device
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureRampScenario_exp(0));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureTxPower(configParamsPrim.txpwr));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureRx(configParamsPrim.rxcfg));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureRfFrequency(configParamsPrim.rfFreqCfg));

    // Configure CTRX_B (Secondary) RF parameters
    printf("Configuring CTRX_B (Secondary) RF parameters...\n");
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_loadSequencerData(0, seqData_sec, sizeof(seqData_sec) / sizeof(uint32_t)));
    printf("Sequencer memory configured for CTRX_B (Secondary)\n");

    // Configure ramp scenario, TX power, RX, and RF frequency for secondary device
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureRampScenario_exp(0));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureTxPower(configParamsSec.txpwr));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureRx(configParamsSec.rxcfg));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_configureRfFrequency(configParamsSec.rfFreqCfg));

    // =============== Transition to Operation State ===================
    // Transition both devices to operational state for radar operations

    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoOperation());
    EXIT_ON_IFXRFE_ERROR(IfxRfe_getStatus(&status));

    // Verify primary device reached operation state
    if (status.curr_state != EXPECTED_OPERATION_STATE)
    {
        printf("Error: CTRX_A (Primary) failed to reach operation state. Current state: 0x%X\n", status.curr_state);
        cleanup_platform();
        return IFXRFE_E_FAILED;
    }
    printf("###########################\n");
    printf("CTRX_A (Primary) OPERATION STATE: 0x%X\n", status.curr_state);

    printf("Transitioning CTRX_B (Secondary) to operation state...\n");
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoOperation());
    EXIT_ON_IFXRFE_ERROR(IfxRfe_getStatus(&status));

    // Verify secondary device reached operation state
    if (status.curr_state != EXPECTED_OPERATION_STATE)
    {
        printf("Error: CTRX_B (Secondary) failed to reach operation state. Current state: 0x%X\n", status.curr_state);
        cleanup_platform();
        return IFXRFE_E_FAILED;
    }
    printf("###########################\n");
    printf("CTRX_B (Secondary) OPERATION STATE: 0x%X\n", status.curr_state);

    // =============== TX Calibration ===================
    // Execute TX calibration on both devices to ensure optimal performance

    IfxRfe_executeCalibrationResult_t calibResultPrim;
    IfxRfe_executeCalibrationResult_t calibResultSec;
    IfxRfe_asyncHandle_t handle;

    // Start TX calibration on secondary device (asynchronous)
    configParamsSec.calibration.calib_sub_func_id = 0b000001100001011;
    EXIT_ON_IFXRFE_ERROR(IfxRfe_executeCalibration_start(configParamsSec.calibration, &handle));

    // Execute TX calibration on primary device (synchronous)
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
    configParamsPrim.calibration.calib_sub_func_id = 0b000001100011011;
    EXIT_ON_IFXRFE_ERROR(IfxRfe_executeCalibration(configParamsPrim.calibration, &calibResultPrim));

    // Wait for and finish TX calibration on secondary device
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_executeCalibration_finish(handle, &calibResultSec));

    // =============== Continuous Radar Operation Loop ===================
    // Run continuous radar measurement cycles with ramp scenarios
    printf("Starting continuous radar operation with %d iterations...\n", NUM_ITERATIONS);

//AG:edit to calculate Payload rate 
// //AG: 
// #define NUM_RX 4
// #define NUM_RAMPS_DEV 512
// #define NUM_DEV 2
// #define NUM_RAMPS_TOTAL NUM_RAMPS_DEV*NUM_DEV
// #define FPS 30


uint32_t num_rx = NUM_RX;
uint32_t num_ramps = NUM_RAMPS_TOTAL;
uint8_t bits_per_sample = configParamsPrim.rxcfg.length;
uint32_t IQ_mult = 2;
uint32_t fps = FPS; 

uint32_t radar_payload;


//AG: Just add the timer
PlatformTime_now()


    //original
    IfxRfe_finishRampScenarioResult_t frsres;

    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        //AG:
        //Calculate payload:
radar_payload = num_rx*num_ramps*bits_per_sample*IQ_mult*fps;

//TODO: check out TOT_NUM_OF FRAMES or whatever
        //TODO: Add Timer 
        // printf("Radar cycle %d/%d\n", i + 1, NUM_ITERATIONS);
printf("Radar cycle %d/%d with payload of %.2f Gbps\n", i + 1, NUM_ITERATIONS, radar_payload);



        // Start ramp scenario on both devices simultaneously
        EXIT_ON_IFXRFE_ERROR(IfxRfe_startRampScenario());  // Secondary (already selected)

        EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
        EXIT_ON_IFXRFE_ERROR(IfxRfe_startRampScenario());

        usleep(FINISH_RAMP_DELAY_US);  // Wait for ramp scenario to complete

        // Wait for ramp scenario completion on both devices
        EXIT_ON_IFXRFE_ERROR(IfxRfe_finishRampScenario(&frsres));
        EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
        EXIT_ON_IFXRFE_ERROR(IfxRfe_finishRampScenario(&frsres));

        // // Transition both devices to low power mode to save energy
        // EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoLowPower());
        // EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
        // EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoLowPower());

        // Wait before next iteration
        usleep(LOOP_DELAY_US);

        // // Wake up both devices for next radar cycle
        // EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoOperation());
        // EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
        // EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoOperation());
    }

    printf("Radar operation completed successfully!\n");

    // =============== Final Cleanup ===================
    // Put both CTRXs in low power mode and cleanup platform resources
    printf("Performing final cleanup...\n");

    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_SECONDARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoLowPower());
    EXIT_ON_IFXRFE_ERROR(IfxRfe_selectDevice(DEVICE_PRIMARY));
    EXIT_ON_IFXRFE_ERROR(IfxRfe_gotoLowPower());

    // Cleanup platform resources
    cleanup_platform();

    printf("=== SUCCESS ===\n");
    return EXIT_SUCCESS;
}