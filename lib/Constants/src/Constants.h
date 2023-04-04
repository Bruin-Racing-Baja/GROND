#ifndef constants_h
#define constants_h

// COMMONLY CHANGED PARAMETERS
const int TARGET_RPM = 2400;
const float PROPORTIONAL_GAIN = 0.01;
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

// Physical constants
const float ROTATIONS_PER_ENGINE_COUNT = 1.0 / 16;
const float ROTATIONS_PER_WHEEL_COUNT = 1.0 / 12;  // Actual rotation of wheel
const float SECONDARY_ROTATIONS_PER_WHEEL_COUNT =
    3.0 / 4;  //Rotation of secondary

#elif model == 22
#define dancing 13
#endif

// Car-Indepenedent Constants
const int LOG_LEVEL = 4;

// Car-Independent Constants
const int CONTROL_FUNCTION_INTERVAL = 1e5;  //microseconds
const int ODRIVE_BAUD_RATE = 115200;        //hz
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

#endif