/* GROND */

#include <Arduino.h>
#include <FlexCAN_T4.h>
// clang-format off
#include <SPI.h>
// clang-format on
#include <HardwareSerial.h>
#include <SD.h>
#include <TimeLib.h>

// Classes
#include <Actuator.h>
#include <Constants.h>
#include <OdriveCAN.h>

// Startup Settings
static constexpr int kMode = OPERATING_MODE;
static constexpr int kWaitSerial = 1;
static constexpr int kHomeOnStartup = 1;  // Controls index search and home

// Object Declarations
OdriveCAN odrive_can;
Actuator actuator(&odrive_can);
IntervalTimer timer;
File log_file;

// Parses CAN messages when received
void odrive_can_parse(const CAN_message_t& msg) {
  odrive_can.parse_message(msg);
}

// Control Function Variables
u_int32_t last_exec_us;
long int last_eg_count = 0;
long int last_wl_count = 0;
float desired_speed = 0;
int last_left_button = 0;
int last_right_button = 0;
int cycles_per_log_flush = 10;
int last_log_flush = 0;
bool pressed = false;
bool flushed = false;
bool sd_init = false;

// Geartooth counts
volatile unsigned long eg_count = 0;
volatile unsigned long wl_count = 0;

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

void getTimeString(char* buf) {
  sprintf(buf, "%d-%02d-%02d %d:%02d:%02d", year(), month(), day(), hour(),
          minute(), second());
}

//ඞ
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

  int left_button = !digitalRead(BUTTON_LEFT_PIN);
  int right_button = !digitalRead(BUTTON_RIGHT_PIN);
  int center_button = !digitalRead(BUTTON_CENTER_PIN);
  int down_button = !digitalRead(BUTTON_DOWN_PIN);
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
  if (last_log_flush == cycles_per_log_flush) {
    log_file.flush();
    odrive_can.odrive_can.reset();
    odrive_can.init(&odrive_can_parse);
    last_log_flush = 0;
    flushed = true;
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
  can_error += !!odrive_can.request_iq(ACTUATOR_AXIS);

  last_log_flush++;
  Serial.printf(
      "ms: %d, voltage: %.2f, current: %.5f, iq_set: %.5f, iq_m: %.5f, "
      "heartbeat: %d, enc: %d, "
      "can_error: %d, vel_cmd: "
      "%.2f, flushed: %d\n",
      millis(), odrive_can.get_voltage(), odrive_can.get_current(),
      odrive_can.get_iq_setpoint(ACTUATOR_AXIS),
      odrive_can.get_iq_measured(ACTUATOR_AXIS),
      odrive_can.get_time_since_heartbeat_ms(),
      odrive_can.get_shadow_count(ACTUATOR_AXIS), can_error, velocity_command,
      flushed);

  log_file.printf(
      "%d, %.2f, %d, %.2f, %.2f, %.2f, %.2f, %d, %d, %d, %.5f, %d, %d, %d, "
      "%.5f, %d, %d, %.5f, %d, %d, %d\n",
      dt_us, odrive_can.get_voltage(), odrive_can.get_time_since_heartbeat_ms(),
      wl_rpm, eg_rpm, TARGET_RPM, velocity_command,
      odrive_can.get_shadow_count(ACTUATOR_AXIS), -1, -1,
      odrive_can.get_iq_measured(ACTUATOR_AXIS), flushed, current_wl_count,
      current_eg_count, odrive_can.get_iq_setpoint(ACTUATOR_AXIS), start_us,
      stop_us, odrive_can.get_current(),
      odrive_can.get_axis_error(ACTUATOR_AXIS),
      odrive_can.get_motor_error(ACTUATOR_AXIS),
      odrive_can.get_encoder_error(ACTUATOR_AXIS));

  pressed = false;
  flushed = false;
}

void serial_debugger() {
  u_int32_t start_us = micros();
  u_int32_t dt_us = start_us - last_exec_us;

  noInterrupts();
  long current_eg_count = eg_count;
  long current_wl_count = wl_count;
  interrupts();

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
}

void setup() {
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_LEFT_PIN, INPUT);
  pinMode(BUTTON_RIGHT_PIN, INPUT);
  pinMode(BUTTON_CENTER_PIN, INPUT);

  if (kWaitSerial) {
    while (!Serial) {}
  }

  // Initialize clock
  setSyncProvider(getTeensy3Time);
  if (timeStatus() != timeSet) {
    Serial.println("Failed to sync with RTC");
  }

  // SD initialization
  sd_init = SD.sdfs.begin(SdioConfig(DMA_SDIO));
  if (!sd_init) {
    Serial.println("SD failed to init");
  }

  // TODO skip log if SD failed?
  char log_name[35];
  sprintf(log_name, "log_%d-%02d-%02d_%d-%02d-%02d.txt", year(), month(), day(),
          hour(), minute(), second());

  log_file = SD.open(log_name, FILE_WRITE);

  if (log_file) {
    char timestamp[25];
    getTimeString(timestamp);

    Serial.printf("Logging at: %s\n", log_name);
    log_file.printf("Initialization Started (%s) - Model: %d ", timestamp,
                    MODEL_NUMBER);
    log_file.flush();
  } else {
    Serial.printf("Failed to open log file: %s\n", log_name);
  }

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
  Serial.print("Attaching interrupt mode " + String(kMode) + "\n");
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
