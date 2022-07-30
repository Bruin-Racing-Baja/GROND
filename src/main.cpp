#include <Arduino.h>

// Libraries
#include <ArduinoLog.h>
#include <SD.h>

// Classes
#include <Constants.h>

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

IntervalTimer timer;
File log_file;
// File-Scope Variable Declarations

// Geartooth counts
volatile unsigned long eg_count = 0;
volatile unsigned long gb_count = 0;

// Control Function ඞ
void control_function() {
  Serial.println(micros());
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
  attachInterrupt(EG_PIN, [](){++eg_count;}, RISING);
  attachInterrupt(GB_PIN, [](){++gb_count;}, RISING);

  timer.begin(control_function, control_function_interval);
  // And so it begins...
}

void loop() {
  // There should be nothing here
  // If you put something here GROND GROND GROND GROND
}

