#ifndef constants_h
#define constants_h

#define model 21

// Per car constants
#if model == 21
const int MODEL_NUMBER = model;
// Pins
#define EG_PIN 1
#define GB_PIN 2

#elif model == 22
#define dancing 13
#endif

// Car-Indepenedent Constants
const int LOG_LEVEL = 0;

// Car-Independent Constants
const int control_function_interval = 1e6;  //microseconds
const int ODRIVE_BAUD_RATE = 115200;        //hz
const int ACTUATOR_AXIS = 0;

// Odrive constants
const int ODRIVE_DEFAULT_TIMEOUT = 1000; //ms

// Odrive state constants
const int ODRIVE_UNKNOWN_STATE = 0;
const int ODRIVE_IDLE_STATE = 1;
const int ODRIVE_ENCODER_INDEX_SEARCH_STATE = 6;
const int ODRIVE_VELOCITY_CONTROL_STATE = 8;

#endif