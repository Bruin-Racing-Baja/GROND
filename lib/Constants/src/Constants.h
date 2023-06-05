#ifndef constants_h
#define constants_h
#include <stdint.h>

// COMMONLY CHANGED PARAMETERS
/*
Maneuverability:
const float PROPORTIONAL_GAIN = 0.03;
const float DERIVATIVE_GAIN = 0.00125;
const float WHEEL_REF_LOW_RPM = 2800;
const float WHEEL_REF_HIGH_RPM = 3100;
*/
const float PROPORTIONAL_GAIN = 0.04;
const float DERIVATIVE_GAIN = 0.002;
const float EG_RPM_WINTER_CUTOFF_FREQ = 1.2;
const float SD_RPM_WINTER_CUTOFF_FREQ = 0.8;
const float VEL_LIMIT = 78;
const float WHEEL_REF_LOW_RPM = 2100;
const float WHEEL_REF_HIGH_RPM = 3200;
const float WHEEL_REF_BREAKPOINT_SECONDARY_RPM = 875;
const float WHEEL_REF_PIECEWISE_SLOPE =
    ((WHEEL_REF_HIGH_RPM - WHEEL_REF_LOW_RPM) /
     WHEEL_REF_BREAKPOINT_SECONDARY_RPM);

const int BRAKE_BIAS_CUTOFF = 200;
const float BRAKE_BIAS_VELOCITY = 50;
#define model 21

// Per car constants
#if model == 21
const int MODEL_NUMBER = model;
// Pins
const int EG_INTERRUPT_PIN = 10;
const int WL_INTERRUPT_PIN = 11;
const int LED_1_PIN = 28;
const int LED_2_PIN = 29;
const int BUTTON_UP = 0;
const int BUTTON_LEFT = 1;
const int BUTTON_CENTER = 2;
const int BUTTON_RIGHT = 3;
const int BUTTON_DOWN = 4;
const int BUTTON_PINS[5] = {28, 29, 30, 31, 32};
const int LED_PINS[5] = {2, 3, 4, 5};
const int BRAKE_LIGHT = 26;

// ODrive Pins
const int ESTOP_IN_ODRIVE_PIN = 7;
const int ESTOP_OUT_ODRIVE_PIN = 8;

// Physical constants
const float ROTATIONS_PER_ENGINE_COUNT = 1.0 / 100;
const float MEASURED_GEAR_ROTATIONS_PER_COUNT =
    1.0 / 6;  // Actual rotation of wheel
const float MEASURED_GEAR_TO_SECONDARY_ROTATIONS = 45.0 / 17;
const float SECONDARY_TO_WHEEL_ROTATIONS =
    (17.0 / 45.0) * (18.0 / 57.0);  // Actual rotation of wheel
const float WHEEL_DIAMETER_IN = 23;
const float WHEEL_MPH_PER_RPM = (WHEEL_DIAMETER_IN * M_PI) / (12 * 5280);

#elif model == 22
#define dancing 13
#endif

// Car-Indepenedent Constants
const int CONTROL_FUNCTION_INTERVAL_US = 2e4;  //microseconds
const int ODRIVE_BAUD_RATE = 115200;           //hz
const int ACTUATOR_AXIS = 1;
const int NUMBER_CYCLES_PER_SD_FLUSH = 10;
const int MEASURED_RPM_CACLULATION_WINDOW = 10;

// Odrive constants
const int ODRIVE_DEFAULT_TIMEOUT = 1000;  //ms

// Odrive enums
const int ODRIVE_UNKNOWN_STATE = 0;
const int ODRIVE_IDLE_STATE = 1;
const int ODRIVE_ENCODER_INDEX_SEARCH_STATE = 6;
const int ODRIVE_VELOCITY_CONTROL_STATE = 8;

// Unit conversions
const int MICROSECONDS_PER_SECOND = 1e6;

// GROND Software modes
const int OPERATING_MODE = 0;
const int SERIAL_DEBUG_MODE = 1;

// Logging Message IDs
const int HEADER_MESSAGE_ID = 0;
const int LOG_MESSAGE_ID = 1;

// Serial debug settings
const int SERIAL_DEBUGGER_INTERVAL_US = 100000;

// Generated using scipy.signal.butter (TODO: add butterworth filter class)
const float ENGINE_RPM_FILTER_B[] = {0.02785976611713602, 0.05571953223427204,
                                     0.02785976611713602};
const float ENGINE_RPM_FILTER_A[] = {1.0, -1.475480443592646,
                                     0.5869195080611902};
const float SECONDARY_RPM_FILTER_B[] = {
    0.002357208772852323, 0.004714417545704646, 0.002357208772852323};
const float SECONDARY_RPM_FILTER_A[] = {1.0, -1.8580432987002593,
                                        0.8674721337916687};

const float BRAKE_LIGHT_FILTER_B[] = {0.39133577250176854, 0.7826715450035371,
                                      0.39133577250176854};
const float BRAKE_LIGHT_FILTER_A[] = {1.0, 0.3695273773512411,
                                      0.19581571265583297};
const uint32_t ENGINE_RPM_FILTER_M =
    sizeof(ENGINE_RPM_FILTER_B) / sizeof(ENGINE_RPM_FILTER_B[0]);
const uint32_t ENGINE_RPM_FILTER_N =
    sizeof(ENGINE_RPM_FILTER_A) / sizeof(ENGINE_RPM_FILTER_A[0]);
const uint32_t SECONDARY_RPM_FILTER_M =
    sizeof(SECONDARY_RPM_FILTER_B) / sizeof(SECONDARY_RPM_FILTER_B[0]);
const uint32_t SECONDARY_RPM_FILTER_N =
    sizeof(SECONDARY_RPM_FILTER_A) / sizeof(SECONDARY_RPM_FILTER_A[0]);
const uint32_t BRAKE_LIGHT_FILTER_M =
    sizeof(BRAKE_LIGHT_FILTER_B) / sizeof(BRAKE_LIGHT_FILTER_B[0]);
const uint32_t BRAKE_LIGHT_FILTER_N =
    sizeof(BRAKE_LIGHT_FILTER_A) / sizeof(BRAKE_LIGHT_FILTER_A[0]);

#endif