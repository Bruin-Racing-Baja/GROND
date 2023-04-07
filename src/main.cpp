/* GROND */

#include <Arduino.h>
#include <FlexCAN_T4.h>
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

#define BUTTON_UP 28
#define BUTTON_LEFT 29
#define BUTTON_CENTER 30
#define BUTTON_RIGHT 31
#define BUTTON_DOWN 32

// Startup Settings
static constexpr int kMode = OPERATING_MODE;
static constexpr int kWaitSerial = 1;
static constexpr int kHomeOnStartup = 1;  // Controls index search and home

// Object Declarations
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

  Serial.printf("ms: %d ec: %d wc: %d ec_rpm: %f wc_rpm %f\n", millis(),
                current_eg_count, current_wl_count, eg_rpm, wl_rpm);
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

float desired_speed = 0;
int last_left_button = 0;
int last_right_button = 0;
int cycles_per_log_flush = 10;
int last_log_flush = 0;
bool pressed = false;
bool flushed = false;
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

  int left_button = !digitalRead(BUTTON_LEFT);
  int right_button = !digitalRead(BUTTON_RIGHT);
  int center_button = !digitalRead(BUTTON_CENTER);
  if (left_button && !last_left_button) {
    desired_speed -= 1;
    pressed = true;
  } else if (right_button && !last_right_button) {
    desired_speed += 1;
    pressed = true;
  }
  if (center_button) {
    desired_speed = 0;
    pressed = true;
  }
  last_left_button = left_button;
  last_right_button = right_button;
  velocity_command = desired_speed;

  actuator.update_speed(velocity_command);

  u_int32_t stop_us = micros();
  int can_error = 0;
  can_error += !!odrive_can.request_vbus_voltage();
  can_error += !!odrive_can.request_motor_error(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_encoder_count(ACTUATOR_AXIS);
  log_file.printf("%d, %d, %F, %F, %d, %d, %F, %F, %d, %d, %d, %d\n", start_us,
                  stop_us, eg_rpm, wl_rpm, current_eg_count, current_wl_count,
                  error, velocity_command, odrive_can.get_voltage(),
                  odrive_can.get_time_since_heartbeat_ms(),
                  odrive_can.get_shadow_count(ACTUATOR_AXIS), can_error);
  if (last_log_flush == cycles_per_log_flush) {
    log_file.flush();
    last_log_flush = 0;
    flushed = true;
  }
  last_log_flush++;
  Serial.printf(
      "ms: %d, voltage: %.2f, heartbeat: %d, enc: %d, can_error: %d, vel_cmd: "
      "%.2f, flushed: %d\n",
      millis(), odrive_can.get_voltage(),
      odrive_can.get_time_since_heartbeat_ms(),
      odrive_can.get_shadow_count(ACTUATOR_AXIS), can_error, velocity_command,
      flushed);
  /*
  Serial.printf(
      "ms: %d ec: %d wc: %d voltage: %.2f heartbeat: %d enc: %d can_error: "
      "%d axis_state: %d axis_error: %d odrive_velocity_estimate: %f "
      "eg_rpm: "
      "%f vel_cmd: %f (flush: %d pressed: %d\n",
      millis(), current_eg_count, current_wl_count, odrive_can.get_voltage(),
      odrive_can.get_time_since_heartbeat_ms(),
      odrive_can.get_shadow_count(ACTUATOR_AXIS), can_error,
      odrive_can.get_axis_state(ACTUATOR_AXIS),
      odrive_can.get_axis_error(ACTUATOR_AXIS),
      odrive_can.get_vel_estimate(ACTUATOR_AXIS), eg_rpm, velocity_command,
      last_log_flush == 1, pressed);
      */

  pressed = false;
  flushed = false;
}

void setup() {
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  pinMode(BUTTON_CENTER, INPUT);

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
  //Log.begin(LOG_LEVEL_NOTICE, &log_file, false);
  log_file.printf("Initialization Started - Model: %d " CR, MODEL_NUMBER);
  log_file.flush();

  // Establish odrive connection
  odrive_can.init(&odrive_can_parse);
  actuator.init();

  // Home actuator
  if (kHomeOnStartup) {
    Serial.print("Index search: ");
    actuator.encoder_index_search() ? Serial.println("Complete")
                                    : Serial.println("Failed");
  }

  // Attach wl, eg interrupts
  attachInterrupt(
      EG_INTERRUPT_PIN, []() { ++eg_count; }, FALLING);
  attachInterrupt(
      WL_INTERRUPT_PIN, []() { ++wl_count; }, RISING);

  // Attach operating mode interrupt
  Serial.print("Attaching interrupt mode " + String(kMode));
  last_exec_us = micros();
  switch (kMode) {
    case OPERATING_MODE:
      odrive_can.set_state(ACTUATOR_AXIS, ODRIVE_VELOCITY_CONTROL_STATE);
      timer.begin(control_function, CONTROL_FUNCTION_INTERVAL_US);
      break;
    case SERIAL_DEBUG_MODE:
      timer.begin(serial_debugger, SERIAL_DEBUGGER_INTERVAL_US);
      break;
  }
}
void loop() {}
