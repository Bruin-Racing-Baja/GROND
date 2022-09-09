#ifndef constants_h
#define constants_h

// COMMONLY CHANGED PARAMETERS
const int TARGET_RPM = 3400;
const float PROPORTIONAL_GAIN = 0.1;

#define model 21

// Per car constants
#if model == 21
const int MODEL_NUMBER = model;
// Pins
const int EG_INTERRUPT_PIN = 0;
const int WL_INTERRUPT_PIN = 0;


// Physical constants
const float ROTATIONS_PER_ENGINE_COUNT = 1/88;
const float ROTATIONS_PER_WHEEL_COUNT = 1/12;       // Actual rotation of wheel
const float SECONDARY_ROTATIONS_PER_WHEEL_COUNT = 3/4;  //Rotation of secondary

#elif model == 22
#define dancing 13
#endif

// Car-Indepenedent Constants
const int LOG_LEVEL = 0;

// Car-Independent Constants
const int CONTROL_FUNCTION_INTERVAL = 1e4;  //microseconds
const int ODRIVE_BAUD_RATE = 115200;        //hz
const int ACTUATOR_AXIS = 1;

// Odrive constants
const int ODRIVE_DEFAULT_TIMEOUT = 1000; //ms

// Odrive enums
const int ODRIVE_UNKNOWN_STATE = 0;
const int ODRIVE_IDLE_STATE = 1;
const int ODRIVE_ENCODER_INDEX_SEARCH_STATE = 6;
const int ODRIVE_VELOCITY_CONTROL_STATE = 8;

// Unit conversions
const int MICROSECONDS_PER_SECOND = 1e6;

#endif