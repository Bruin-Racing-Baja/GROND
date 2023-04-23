/* GROND */

#include <Arduino.h>
#include <FlexCAN_T4.h>
// clang-format off
#include <SPI.h>
// clang-format on
#include <HardwareSerial.h>
#include <SD.h>
#include <TimeLib.h>
#include <header_message.pb.h>
#include <log_message.pb.h>
#include <pb.h>
#include <pb_common.h>
#include <pb_encode.h>

// Classes
#include <Actuator.h>
#include <Constants.h>
#include <OdriveCAN.h>

// Startup Settings
static constexpr int kMode = OPERATING_MODE;
static constexpr int kWaitSerial = 0;
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
uint8_t buffer[256];
LogMessage log_message;
uint32_t cycle_count = 0;
u_int32_t last_exec_us;
long int last_eg_count = 0;
long int last_wl_count = 0;
float desired_speed = 0;
bool button_states[5];
bool last_button_states[5];
int cycles_per_log_flush = 10;
int last_log_flush = 0;
bool pressed = false;
bool flushed = false;
bool sd_init = false;
float last_error = 0;

// Geartooth counts
volatile unsigned long eg_count = 0;
volatile unsigned long wl_count = 0;

time_t get_teensy3_time() {
  return Teensy3Clock.get();
}

void get_time_string(char* buf) {
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(),
          minute(), second());
}

bool encode_string(pb_ostream_t* stream, const pb_field_t* field,
                   void* const* arg) {
  const char* str = (const char*)(*arg);
  if (!pb_encode_tag_for_field(stream, field)) {
    return false;
  }
  return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}

//ඞ
float last_filtered_sd_rpm = 0;
float alpha = 0.4;

void control_function() {
  u_int32_t start_us = micros();
  u_int32_t dt_us = start_us - last_exec_us;
  float dt_s = dt_us / 1.e6;

  noInterrupts();
  long current_eg_count = eg_count;
  long current_wl_count = wl_count;
  interrupts();

  // First, calculate rpms
  float eg_rpm = (current_eg_count - last_eg_count) *
                 ROTATIONS_PER_ENGINE_COUNT / dt_us * MICROSECONDS_PER_SECOND *
                 60.0;
  float ms_rpm = (current_wl_count - last_wl_count) *
                 MEASURED_GEAR_ROTATIONS_PER_COUNT / dt_us *
                 MICROSECONDS_PER_SECOND * 60.0;
  float sd_rpm = ms_rpm * MEASURED_GEAR_TO_SECONDARY_ROTATIONS;
  float wl_rpm = ms_rpm * MEASURED_GEAR_TO_SECONDARY_ROTATIONS *
                 SECONDARY_TO_WHEEL_ROTATIONS;
  float filtered_sd_rpm = sd_rpm * alpha + (1 - alpha) * last_filtered_sd_rpm;
  last_filtered_sd_rpm = filtered_sd_rpm;

  last_eg_count = current_eg_count;
  last_wl_count = current_wl_count;
  last_exec_us = start_us;

  float target_rpm = WHEEL_REF_HIGH_RPM;
  if (filtered_sd_rpm <= 0) {
    target_rpm = WHEEL_REF_LOW_RPM;
  } else if (filtered_sd_rpm <= WHEEL_REF_BREAKPOINT_SECONDARY_RPM) {
    target_rpm =
        WHEEL_REF_PIECEWISE_SLOPE * filtered_sd_rpm + WHEEL_REF_LOW_RPM;
  }

  //target_rpm = TARGET_RPM;
  float error = target_rpm - eg_rpm;
  float d_error = (error - last_error) / dt_s;
  float velocity_command =
      error * PROPORTIONAL_GAIN + d_error * DERIVATIVE_GAIN;
  last_error = error;

  for (int i = 0; i < 5; i++) {
    button_states[i] = !digitalRead(BUTTON_PINS[i]);
  }
  if (button_states[BUTTON_LEFT] && !last_button_states[BUTTON_LEFT]) {
    desired_speed -= 1;
    pressed = true;
  } else if (button_states[BUTTON_RIGHT] && !last_button_states[BUTTON_RIGHT]) {
    desired_speed += 1;
    pressed = true;
  }
  if (button_states[BUTTON_CENTER]) {
    desired_speed = 0;
    pressed = true;
  }
  for (int i = 0; i < 5; i++) {
    last_button_states[i] = button_states[i];
  }

  //velocity_command = desired_speed;
  float clamped_velocity_command = actuator.update_speed(velocity_command);

  u_int32_t stop_us = micros();
  int can_error = 0;
  can_error += !!odrive_can.request_vbus_voltage();
  can_error += !!odrive_can.request_encoder_count(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_motor_error(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_encoder_count(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_iq(ACTUATOR_AXIS);

  Serial.printf(
      "ms: %d, vltg: %.2f, crnt: %.2f, iq_set: %.2f, iq_m: %.2f, "
      "hrt: %d, enc: %d, "
      "can_er: %d, vel_cmd: "
      "%.2f (%.2f), flsh: %d, w_rpm: %.2f, e_rpm: %.2f, w_cnt: %d, e_cnt: %d, "
      "ax_err: %d, mtr_err: %d, enc_err: %d\n",
      millis(), odrive_can.get_voltage(), odrive_can.get_current(),
      odrive_can.get_iq_setpoint(ACTUATOR_AXIS),
      odrive_can.get_iq_measured(ACTUATOR_AXIS),
      odrive_can.get_time_since_heartbeat_ms(),
      odrive_can.get_shadow_count(ACTUATOR_AXIS), can_error,
      clamped_velocity_command, velocity_command, flushed, wl_rpm, eg_rpm,
      current_wl_count, current_eg_count,
      odrive_can.get_axis_error(ACTUATOR_AXIS),
      odrive_can.get_motor_error(ACTUATOR_AXIS),
      odrive_can.get_encoder_error(ACTUATOR_AXIS));

  log_message.control_cycle_count = cycle_count;
  log_message.control_cycle_start_us = start_us;
  log_message.control_cycle_stop_us = stop_us;
  log_message.control_cycle_dt_us = dt_us;
  log_message.control_cycle_dt_us = dt_us;
  log_message.wheel_rpm = wl_rpm;
  log_message.engine_rpm = eg_rpm;
  log_message.engine_count = current_eg_count;
  log_message.wheel_count = current_wl_count;
  log_message.target_rpm = target_rpm;
  log_message.velocity_command = clamped_velocity_command;
  log_message.unclamped_velocity_command = velocity_command;
  log_message.last_heartbeat_ms = odrive_can.get_time_since_heartbeat_ms();
  log_message.axis_error = odrive_can.get_axis_error(ACTUATOR_AXIS);
  log_message.motor_error = odrive_can.get_motor_error(ACTUATOR_AXIS);
  log_message.encoder_error = odrive_can.get_encoder_error(ACTUATOR_AXIS);
  log_message.voltage = odrive_can.get_voltage();
  log_message.iq_measured = odrive_can.get_iq_measured(ACTUATOR_AXIS);
  log_message.iq_setpoint = odrive_can.get_iq_setpoint(ACTUATOR_AXIS);
  log_message.odrive_current = odrive_can.get_current();
  log_message.inbound_estop = false;
  log_message.outbound_estop = false;
  log_message.shadow_count = odrive_can.get_shadow_count(ACTUATOR_AXIS);
  log_message.velocity_estimate = odrive_can.get_vel_estimate(ACTUATOR_AXIS);
  log_message.filtered_secondary_rpm = filtered_sd_rpm;

  pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  pb_encode(&ostream, &LogMessage_msg, &log_message);
  size_t message_length = ostream.bytes_written;

  log_file.printf("%01X", LOG_MESSAGE_ID, 1);
  log_file.printf("%04X", message_length, 4);
  log_file.write(buffer, message_length);

  if (cycle_count % cycles_per_log_flush == 0) {
    log_file.flush();
    digitalToggle(LED_PINS[32]);
  }

  cycle_count++;
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
  float ms_rpm = (current_wl_count - last_wl_count) *
                 MEASURED_GEAR_ROTATIONS_PER_COUNT / dt_us *
                 MICROSECONDS_PER_SECOND * 60.0;

  last_eg_count = current_eg_count;
  last_wl_count = current_wl_count;
  last_exec_us = start_us;

  Serial.printf("ms: %d ec: %d wc: %d ec_rpm: %f wc_rpm %f\n", millis(),
                current_eg_count, current_wl_count, eg_rpm, ms_rpm);
}

void setup() {
  for (int i = 0; i < 5; i++) {
    pinMode(BUTTON_PINS[i], INPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  digitalWrite(LED_PINS[2], HIGH);

  if (kWaitSerial) {
    while (!Serial) {}
  }

  // Initialize clock
  setSyncProvider(get_teensy3_time);
  if (timeStatus() != timeSet) {
    Serial.println("Failed to sync with RTC");
  }

  // SD initialization
  sd_init = SD.sdfs.begin(SdioConfig(DMA_SDIO));
  if (!sd_init) {
    digitalWrite(LED_PINS[0], HIGH);
    Serial.println("SD failed to init");
  }

  // log file determination and initialization
  // TODO skip log if SD failed?
  char log_name[35];
  sprintf(log_name, "log_%04d-%02d-%02d_%02d-%02d-%02d.bin", year(), month(),
          day(), hour(), minute(), second());
  if (SD.exists(log_name)) {
    char log_name_duplicate[35];
    int i = 1;
    do {
      sprintf(log_name_duplicate, "%.*s_%03d.bin", 23, log_name, i);
      i++;
    } while (SD.exists(log_name_duplicate));
    strcpy(log_name, log_name_duplicate);
  }

  log_file = SD.open(log_name, FILE_WRITE);

  if (log_file) {
    HeaderMessage header_message;

    header_message.timestamp_human.arg = malloc(20);
    header_message.timestamp_human.funcs.encode = &encode_string;

    get_time_string((char*)header_message.timestamp_human.arg);
    header_message.clock_us = micros();
    header_message.p_gain = PROPORTIONAL_GAIN;

    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_encode(&ostream, &HeaderMessage_msg, &header_message);
    size_t message_length = ostream.bytes_written;

    log_file.printf("%01X", HEADER_MESSAGE_ID);
    log_file.printf("%04X", message_length);
    log_file.write(buffer, message_length);

    Serial.printf("Logging at: %s\n", log_name);
  } else {
    digitalWrite(LED_PINS[0], HIGH);
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
  digitalWrite(LED_PINS[3], HIGH);

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
      odrive_can.set_input_vel(ACTUATOR_AXIS, 0, 0);
      odrive_can.set_state(ACTUATOR_AXIS, ODRIVE_VELOCITY_CONTROL_STATE);
      timer.begin(control_function, CONTROL_FUNCTION_INTERVAL_US);
      break;
    case SERIAL_DEBUG_MODE:
      timer.begin(serial_debugger, SERIAL_DEBUGGER_INTERVAL_US);
      break;
  }
}
void loop() {}
