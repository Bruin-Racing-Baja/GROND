#include <Arduino.h>

// Libraries
#include <SPI.h>
#include <ArduinoLog.h>
#include <SD.h>
#include <HardwareSerial.h>

// Classes
#include <Actuator.h>
#include <Constants.h>
#include <Odrive.h>

/*
GROND GROND GROND GROND
GROND GROND GROND GROND
GROND GROND GROND GROND
GROND GROND GROND GROND

The main file is organized as such:
[ Settings ]
[ Object Declarations ]
[ File-Scope Variable Declarations ]
[ Control Function ]
-- Setup Function --
  [ Serial wait (if enabled) ]
  * Log Begins *
  [ Object initializtions ]
  [ Attach control function interrupt ]

-- Main Function --
Nothing :)

Modes:
0 - Normal Operation
1 - Debug Mode [Teensy Power]
2 - Debug Mode [Main Power]
*/
#define MODE 0

// Startup Settings
#define WAIT_SERIAL 1
#define HOME_ON_STARTUP 1 // Controls index search and home

// Object Declarations
Odrive odrive(Serial1);
Actuator actuator(&odrive);
IntervalTimer timer;
File log_file;
// File-Scope Variable Declarations

// Geartooth counts
volatile unsigned long eg_count = 0;
volatile unsigned long wl_count = 0;

// Control Function Variables
u_int32_t last_exec_us;

long int last_eg_count;
long int last_wl_count;




// Control Function ඞ
void control_function() {
  u_int32_t start_us = micros();
  u_int32_t dt_us = start_us - last_exec_us;

  noInterrupts();
  int current_eg_count = eg_count;
  int current_wl_count = wl_count;
  interrupts();

  // First, calculate rpms
  float eg_rpm = ( current_eg_count - last_eg_count ) / dt_us * ROTATIONS_PER_ENGINE_COUNT * MICROSECONDS_PER_SECOND;
  float wl_rpm = ( current_wl_count - last_wl_count ) / dt_us * ROTATIONS_PER_WHEEL_COUNT * MICROSECONDS_PER_SECOND;

  float error = TARGET_RPM - eg_rpm;

  float velocity_command = error * PROPORTIONAL_GAIN;

  actuator.update_speed(velocity_command);

  
}

void setup() {
  if (WAIT_SERIAL) { while(!Serial) { } }

  // Log file determination and initialization
  int log_file_number = 0;
  while (SD.exists(("log_" + String(log_file_number) + ".txt").c_str()))
  {
    log_file_number++;
  }
  String log_name = "log_" + String(log_file_number) + ".txt";
  Serial.println("Logging at: " + log_name);
  log_file = SD.open(log_name.c_str(), FILE_WRITE);
  Log.begin(LOG_LEVEL, &log_file, false);
  Log.notice("Initialization Started - Model: %d " CR, MODEL_NUMBER);
  

  // Create interrupts to count gear teeth
  attachInterrupt(EG_INTERRUPT_PIN, [](){++eg_count;}, RISING);
  attachInterrupt(WL_INTERRUPT_PIN, [](){++wl_count;}, RISING);


  last_exec_us = micros();
  timer.begin(control_function, CONTROL_FUNCTION_INTERVAL);
  // And so it begins...
}

void loop() {
  // There should be nothing here
  // If you put something here GROND GROND GROND GROND
}

