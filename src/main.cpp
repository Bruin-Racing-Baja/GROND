#include <Arduino.h>

// Libraries
#include <SPI.h>
#include <ArduinoLog.h>
#include <SD.h>
#include <TimeLib.h>

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
bool sd_init = false;

// Geartooth counts
volatile unsigned long eg_count = 0;
volatile unsigned long gb_count = 0;

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void getTimeString(char *buf){
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
}

// Control Function ඞ
void control_function() {
  Serial.println(micros());
}

void setup() {
  if (WAIT_SERIAL) { while(!Serial) { } }

  // Initialize clock
  setSyncProvider(getTeensy3Time);
  if (timeStatus() != timeSet) {
    Serial.println("Failed to sync with RTC");
  }

  // SD initialization
  sd_init = SD.begin(BUILTIN_SDCARD);
  if(!sd_init){
    Serial.println("SD failed to init");
  }

  // log file determination and initialization
  // TODO skip log if SD failed?
  char log_name[35];
  sprintf(log_name, "log_%04d-%02d-%02d_%02d-%02d-%02d.txt", year(), month(), day(), hour(), minute(), second());
  if(SD.exists(log_name)){
    char log_name_duplicate[35];
    int i = 1;
    do{
      sprintf(log_name_duplicate,"%.*s_%03d.txt",23,log_name,i);
      i++;
    }while(SD.exists(log_name_duplicate));
    strcpy(log_name, log_name_duplicate);
  }

  log_file = SD.open(log_name, FILE_WRITE);
  
  if(log_file){
    char timestamp[25];
    getTimeString(timestamp);

    Serial.printf("Logging at: %s\n", log_name);
    Log.begin(LOG_LEVEL, &log_file, false);
    Log.notice("Initialization Started (%s) - Model: %d " CR, timestamp, MODEL_NUMBER);
    log_file.close();
  }
  else{
    Serial.printf("Failed to open log file: %s\n", log_name);
  }

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

