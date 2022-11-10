#include <Arduino.h>

// Libraries
// clang-format off
#include <SPI.h>
// clang-format on
#include <ArduinoLog.h>
#include <HardwareSerial.h>
#include <SD.h>

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
static constexpr int kMode = 0;

// Startup Settings
static constexpr int kWaitSerial = 1;
static constexpr int kHomeOnStartup = 1;  // Controls index search and home

// Object Declarations
Odrive odrive(Serial1);
Actuator actuator(&odrive);
IntervalTimer timer;
File log_file;
// File-Scope Variable Declarations
String log_name;

// Geartooth counts
volatile unsigned long eg_count = 0;
volatile unsigned long wl_count = 0;

// Control Function Variables
u_int32_t last_exec_us;

long int last_eg_count = 0;
long int last_wl_count = 0;

static constexpr int kSerialDebuggerIntervalUs = 100000;
void serial_debugger() {
  noInterrupts();
  long current_eg_count = eg_count;
  long current_wl_count = wl_count;
  interrupts();
  Serial.printf("ms: %d ec: %d wc: %d\n", millis(), current_eg_count,
                current_wl_count);
}

// Control Function ඞ
void control_function() {
  u_int32_t start_us = micros();
  u_int32_t dt_us = start_us - last_exec_us;

  noInterrupts();
  long current_eg_count = eg_count;
  long current_wl_count = wl_count;
  interrupts();

  // First, calculate rpms
  float eg_rpm = (current_eg_count - last_eg_count) *
                 ROTATIONS_PER_ENGINE_COUNT / dt_us * MICROSECONDS_PER_SECOND *
                 60.0;
  float wl_rpm = (current_wl_count - last_wl_count) *
                 ROTATIONS_PER_WHEEL_COUNT / dt_us * MICROSECONDS_PER_SECOND *
                 60.0;

  last_eg_count = current_eg_count;
  last_wl_count = current_wl_count;
  last_exec_us = start_us;

  float error = TARGET_RPM - eg_rpm;
  float velocity_command = error * PROPORTIONAL_GAIN;

  bool estop_in = digitalReadFast(ESTOP_IN_PIN);
  bool estop_out = digitalReadFast(ESTOP_OUT_PIN);

  digitalWrite(LED_1_PIN, !estop_in);
  digitalWrite(LED_2_PIN, !estop_out);

  actuator.update_speed(velocity_command);

  u_int32_t stop_us = micros();
  Log.notice("%d, %d, %F, %F, %d, %d, %F, %F, %d, %d" CR, start_us, stop_us,
             eg_rpm, wl_rpm, current_eg_count, current_wl_count, error,
             velocity_command, estop_in, estop_out);
  log_file.close();
  log_file = SD.open(log_name.c_str(), FILE_WRITE);
}

void setup() {
  Serial.println("Starting up, greetings user...");
  if (kWaitSerial) {
    while (!Serial) {}
  }

  // Log file determination and initialization
  SD.begin(BUILTIN_SDCARD);
  int log_file_number = 0;
  while (SD.exists(("log_" + String(log_file_number) + ".txt").c_str())) {
    log_file_number++;
  }
  log_name = "log_" + String(log_file_number) + ".txt";

  // Begin log and save first line
  Serial.println("Logging at: " + log_name);
  log_file = SD.open(log_name.c_str(), FILE_WRITE);
  Log.begin(LOG_LEVEL_NOTICE, &log_file, false);
  Log.notice("Initialization Started - Model: %d " CR, MODEL_NUMBER);
  log_file.close();
  log_file = SD.open(log_name.c_str(), FILE_WRITE);

  Serial.print("Actuator communication init: ");
  actuator.init() ? Serial.println("Complete") : Serial.println("Failed");

  Serial.print("Index search: ");
  actuator.encoder_index_search() ? Serial.println("Complete")
                                  : Serial.println("Failed");

  // Create interrupts to count gear teeth
  Serial.print("Attaching geartooth interrupts: ");
  attachInterrupt(
      EG_INTERRUPT_PIN, []() { ++eg_count; }, RISING);
  attachInterrupt(
      WL_INTERRUPT_PIN, []() { ++wl_count; }, RISING);
  Serial.println("Complete");

  // Attach correct interrupt based on the desired mode
  Serial.print("Attaching timer interrupt: ");
  last_exec_us = micros();
  switch (kMode) {
    case 0:
      timer.begin(control_function, CONTROL_FUNCTION_INTERVAL);
      break;
    case 1:
      timer.begin(serial_debugger, kSerialDebuggerIntervalUs);
      break;
  }
  Serial.println("Complete, godspeed.");

  // And so it begins...
}

void loop() {
  // There should be nothing here
  // If you put something here GROND GROND GROND GROND
}
