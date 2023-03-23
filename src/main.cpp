#include <Arduino.h>
#include <FlexCAN_T4.h>

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
#include <OdriveCAN.h>

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

// Object Declarations
Odrive odrive(Serial1);
OdriveCAN odrive_can;
Actuator actuator(&odrive_can);
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

// Parses CAN messages when received
void odrive_can_parse(const CAN_message_t& msg) {
  odrive_can.parse_message(msg);
}

static constexpr int kSerialDebuggerIntervalUs = 100000;
void serial_debugger() {
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

  Serial.printf("ms: %d ec: %d wc: %d ec_rpm: %f wc_rpm %f\n", millis(), current_eg_count, current_wl_count, eg_rpm, wl_rpm);
  // int can_error = 0;
  // can_error = (can_error << 1) & odrive_can.request_vbus_voltage();
  // can_error = (can_error << 1) & odrive_can.request_motor_error(1);
  // can_error = (can_error << 1) & odrive_can.request_encoder_count(1);
  // Serial.printf(
  //     "ms: %d ec: %d wc: %d voltage: %.2f heartbeat: %d enc: %d can_error: "
  //     "%d\n",
  //     millis(), current_eg_count, current_wl_count, odrive_can.get_voltage(),
  //     odrive_can.get_time_since_heartbeat_ms(), odrive_can.get_shadow_count(1),
  //     can_error);
}

// Control Function ඞ
void control_function() {
  Serial.println("Start");
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

  actuator.update_speed(velocity_command);

  u_int32_t stop_us = micros();
    int can_error = 0;
  can_error = (can_error << 1) & odrive_can.request_vbus_voltage();
  can_error = (can_error << 1) & odrive_can.request_motor_error(1);
  can_error = (can_error << 1) & odrive_can.request_encoder_count(1);
Log.notice("%d, %d, %F, %F, %d, %d, %F, %F, %d, %d, %f  %d \n" CR, start_us, stop_us,
             eg_rpm, wl_rpm, current_eg_count, current_wl_count, error,
             velocity_command, odrive_can.get_voltage(), odrive_can.get_time_since_heartbeat_ms(), odrive_can.get_shadow_count(1),
            can_error);
  log_file.close();
  log_file = SD.open(log_name.c_str(), FILE_WRITE);


  Serial.printf(
      "ms: %d ec: %d wc: %d voltage: %.2f heartbeat: %d enc: %d can_error: "
      "%d axis_state: %d axis_error: %d odrive_velocity_estimate: %f eg_rpm: %f vel_cmd: %f \n",
      millis(), current_eg_count, current_wl_count, odrive_can.get_voltage(),
      odrive_can.get_time_since_heartbeat_ms(), odrive_can.get_shadow_count(1),
      can_error, odrive_can.get_axis_state(1), odrive_can.get_axis_error(1), odrive_can.get_vel_estimate(1), eg_rpm, velocity_command);

      //need current state, current velocity, 
}

void setup() {
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
  Serial.println("Logging at: " + log_name);
  log_file = SD.open(log_name.c_str(), FILE_WRITE);
  Log.begin(LOG_LEVEL_NOTICE, &log_file, false);
  Log.notice("Initialization Started - Model: %d " CR, MODEL_NUMBER);
  log_file.close();
  log_file = SD.open(log_name.c_str(), FILE_WRITE);

  actuator.init();
  odrive_can.init(&odrive_can_parse);

  Serial.print("Index: ");
  actuator.encoder_index_search();
  Serial.println("after index");

  // Create interrupts to count gear teeth
  attachInterrupt(
      EG_INTERRUPT_PIN, []() { ++eg_count; }, FALLING);
  attachInterrupt(
      WL_INTERRUPT_PIN, []() { ++wl_count; }, RISING);

  // Attach correct interrupt based on the desired mode
  last_exec_us = micros();
  switch (kMode) {
    case 0:
      odrive_can.set_state(1, 8);
      timer.begin(control_function, CONTROL_FUNCTION_INTERVAL);
      break;
    case 1:
      timer.begin(serial_debugger, kSerialDebuggerIntervalUs);
      break;
  }

  // And so it begins...
}
void loop() {}
