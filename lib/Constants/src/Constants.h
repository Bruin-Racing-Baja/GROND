#ifndef constants_h
#define constants_h

// COMMONLY CHANGED PARAMETERS
const int TARGET_RPM = 3500;
const float PROPORTIONAL_GAIN = 0.016;
const float EG_RPM_BUTTERWORTH_CONSTANT = 0.4;
const int VEL_LIMIT = 10;
#define model 21

// Per car constants
#if model == 21
const int MODEL_NUMBER = model;
// Pins
const int EG_INTERRUPT_PIN = 10;
const int WL_INTERRUPT_PIN = 11;
const int ESTOP_IN_PIN = 33;
const int ESTOP_OUT_PIN = 32;
const int LED_1_PIN = 28;
const int LED_2_PIN = 29;
const int BUTTON_UP = 0;
const int BUTTON_LEFT = 1;
const int BUTTON_CENTER = 2;
const int BUTTON_RIGHT = 3;
const int BUTTON_DOWN = 4;
const int BUTTON_PINS[5] = {28, 29, 30, 31, 32};
const int LED_PINS[5] = {2, 3, 4, 5};

// Physical constants
const float ROTATIONS_PER_ENGINE_COUNT = 1.0 / 16;
const float ROTATIONS_PER_WHEEL_COUNT = 1.0 / 12;  // Actual rotation of wheel
const float SECONDARY_ROTATIONS_PER_WHEEL_COUNT =
    1.0 / ((45 / 17) * (57 / 18));  //Rotation of secondary

#elif model == 22
#define dancing 13
#endif

// Car-Indepenedent Constants
const int LOG_LEVEL = 4;
const int CONTROL_FUNCTION_INTERVAL_US = 1e5;  //microseconds
const int ODRIVE_BAUD_RATE = 115200;           //hz
const int ACTUATOR_AXIS = 1;

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

// Serial debug settings
const int SERIAL_DEBUGGER_INTERVAL_US = 100000;

#endif