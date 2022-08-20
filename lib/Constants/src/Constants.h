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

// Car-Indepenedent Bools
const int LOG_LEVEL = 0;

// Car-Independent Constants
const int control_function_interval = 1e6;   // in microseconds
const int ODRIVE_BAUD_RATE = 1;
const int ACTUATOR_AXIS = 0;
const int ODRIVE_RUN_STATE = 8;
const int ODRIVE_ENCODER_CALIBRATION_STATE = 3;
const int ODRIVE_DEFAULT_TIMEOUT = 1000; //ms


#endif