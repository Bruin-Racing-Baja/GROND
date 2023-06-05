/* GROND */

#include <Actuator.h>
#include <Arduino.h>
#include <Constants.h>
#include <FlexCAN_T4.h>
#include <HardwareSerial.h>
#include <IIRFilter.h>
#include <OdriveCAN.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include <header_message.pb.h>
#include <log_message.pb.h>
#include <math.h>
#include <pb.h>
#include <pb_common.h>
#include <pb_encode.h>

// Startup Settings
static constexpr int kMode = OPERATING_MODE;
static constexpr int kWaitSerial = 0;
static constexpr int kHomeOnStartup = 1;  // Controls index search and home
static constexpr bool kSerialDebugging = 0;

// Object Declarations
OdriveCAN odrive_can;
Actuator actuator(&odrive_can);
IntervalTimer timer;
File log_file;
IIRFilter engine_rpm_filter(ENGINE_RPM_FILTER_B, ENGINE_RPM_FILTER_A,
                            ENGINE_RPM_FILTER_M, ENGINE_RPM_FILTER_N);

IIRFilter secondary_rpm_filter(SECONDARY_RPM_FILTER_B, SECONDARY_RPM_FILTER_A,
                               SECONDARY_RPM_FILTER_M, SECONDARY_RPM_FILTER_N);

IIRFilter brake_light_filter(BRAKE_LIGHT_FILTER_B, BRAKE_LIGHT_FILTER_A,
                             BRAKE_LIGHT_FILTER_M, BRAKE_LIGHT_FILTER_N);

// CAN parsing interrupt function
void odrive_can_parse(const CAN_message_t& msg) {
  odrive_can.parse_message(msg);
}

// Control Function Variables
uint8_t buffer[512];
LogMessage log_message;
uint32_t cycle_count = 0;
uint32_t last_sample_time_us;
uint32_t last_eg_count = 0;
uint32_t last_wl_count = 0;
float last_error = 0;
volatile uint32_t eg_count = 0;
volatile uint32_t wl_count = 0;
float cur_ms_rpm = 0.0;

// Real Time Clock functions
time_t get_teensy3_time() {
  return Teensy3Clock.get();
}

// Protobuf encoding function
bool encode_string(pb_ostream_t* stream, const pb_field_t* field,
                   void* const* arg) {
  const char* str = (const char*)(*arg);
  if (!pb_encode_tag_for_field(stream, field)) {
    return false;
  }
  return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}

//ඞ
void control_function() {
  uint32_t start_us = micros();

  // Record counts from engine and secondary sensors
  noInterrupts();
  uint32_t current_eg_count = eg_count;
  uint32_t current_wl_count = wl_count;
  interrupts();
  uint32_t sample_time_us = micros();
  uint32_t dt_us = sample_time_us - last_sample_time_us;
  last_sample_time_us = sample_time_us;
  float dt_s = dt_us / 1.e6;

  // Calculate instantaneous RPMs
  float eg_rpm = (current_eg_count - last_eg_count) *
                 ROTATIONS_PER_ENGINE_COUNT / dt_us * MICROSECONDS_PER_SECOND *
                 60.0;

  if (cycle_count % MEASURED_RPM_CACLULATION_WINDOW == 0) {
    cur_ms_rpm = (current_wl_count - last_wl_count) *
                 MEASURED_GEAR_ROTATIONS_PER_COUNT / dt_us *
                 MICROSECONDS_PER_SECOND * 60.0 /
                 MEASURED_RPM_CACLULATION_WINDOW;
    last_wl_count = current_wl_count;
  }
  float ms_rpm = cur_ms_rpm;
  float sd_rpm = ms_rpm * MEASURED_GEAR_TO_SECONDARY_ROTATIONS;
  float wl_rpm = ms_rpm * MEASURED_GEAR_TO_SECONDARY_ROTATIONS *
                 SECONDARY_TO_WHEEL_ROTATIONS;

  last_eg_count = current_eg_count;

  // Filter RPMs
  float filt_eg_rpm = engine_rpm_filter.update(eg_rpm);
  float filt_sd_rpm = secondary_rpm_filter.update(sd_rpm);

  // Calculate reference RPM from wheel speed
  float target_rpm = WHEEL_REF_HIGH_RPM;
  if (filt_sd_rpm <= 0) {
    target_rpm = WHEEL_REF_LOW_RPM;
  } else if (filt_sd_rpm <= WHEEL_REF_BREAKPOINT_SECONDARY_RPM) {
    target_rpm = WHEEL_REF_PIECEWISE_SLOPE * filt_sd_rpm + WHEEL_REF_LOW_RPM;
  }

  // PD controller math
  float unfiltered_error = target_rpm - eg_rpm;
  float error = target_rpm - filt_eg_rpm;
  float d_error = (error - last_error) / dt_s;
  float velocity_command =
      unfiltered_error * PROPORTIONAL_GAIN + max(d_error * DERIVATIVE_GAIN, 0);
  last_error = error;

  // Brake biasing and actuator command
  int brake_light_signal = analogRead(BRAKE_LIGHT);
  int filtered_brake_light_signal =
      brake_light_filter.update(brake_light_signal);
  bool brake_pressed = filtered_brake_light_signal > BRAKE_BIAS_CUTOFF;

  float brake_bias = brake_pressed ? BRAKE_BIAS_VELOCITY : 0;
  float clamped_velocity_command =
      actuator.update_speed(velocity_command, brake_bias);

  uint32_t stop_us = micros();

  // Logging
  int can_error = 0;
  can_error += !!odrive_can.request_vbus_voltage();
  can_error += !!odrive_can.request_encoder_count(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_motor_error(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_encoder_count(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_iq(ACTUATOR_AXIS);
  can_error += !!odrive_can.request_gpio_states();

  if (kSerialDebugging) {
    Serial.printf(
        "ms: %d, vltg: %.2f, crnt: %.2f, iq_set: %.2f, iq_m: %.2f, "
        "hrt: %d, enc: %d, "
        "can_er: %d, vel_cmd: "
        "%.2f (%.2f), w_rpm: %.2f, e_rpm: %.2f, w_cnt: %d, e_cnt: "
        "%d, "
        "ax_err: %d, mtr_err: %d, enc_err: %d, filt_eg_rpm: %.2f, in: %d, "
        "out:%d\n",
        millis(), odrive_can.get_voltage(), odrive_can.get_current(),
        odrive_can.get_iq_setpoint(ACTUATOR_AXIS),
        odrive_can.get_iq_measured(ACTUATOR_AXIS),
        odrive_can.get_time_since_heartbeat_ms(),
        odrive_can.get_shadow_count(ACTUATOR_AXIS), can_error,
        clamped_velocity_command, velocity_command, wl_rpm, eg_rpm,
        current_wl_count, current_eg_count,
        odrive_can.get_axis_error(ACTUATOR_AXIS),
        odrive_can.get_motor_error(ACTUATOR_AXIS),
        odrive_can.get_encoder_error(ACTUATOR_AXIS), filt_eg_rpm,
        odrive_can.get_gpio(ESTOP_IN_ODRIVE_PIN),
        odrive_can.get_gpio(ESTOP_OUT_ODRIVE_PIN));
  }

  log_message.control_cycle_count = cycle_count;
  log_message.control_cycle_start_us = start_us;
  log_message.control_cycle_stop_us = stop_us;
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
  log_message.inbound_estop = odrive_can.get_gpio(ESTOP_IN_ODRIVE_PIN);
  log_message.outbound_estop = odrive_can.get_gpio(ESTOP_OUT_ODRIVE_PIN);
  log_message.shadow_count = odrive_can.get_shadow_count(ACTUATOR_AXIS);
  log_message.velocity_estimate = odrive_can.get_vel_estimate(ACTUATOR_AXIS);
  log_message.filtered_secondary_rpm = filt_sd_rpm;
  log_message.filtered_engine_rpm = filt_eg_rpm;
  log_message.engine_rpm_error = error;
  log_message.engine_rpm_deriv_error = d_error;
  log_message.brake_light_signal = brake_light_signal;
  log_message.filtered_brake_light_signal = filtered_brake_light_signal;
  log_message.brake_pressed = brake_pressed;

  // Write log to SD card buffer
  pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  pb_encode(&ostream, &LogMessage_msg, &log_message);
  size_t message_length = ostream.bytes_written;

  log_file.printf("%01X", LOG_MESSAGE_ID, 1);
  log_file.printf("%04X", message_length, 4);
  log_file.write(buffer, message_length);

  if (cycle_count % NUMBER_CYCLES_PER_SD_FLUSH == 0) {
    log_file.flush();
  }

  cycle_count++;
}

void serial_debugger() {
  uint32_t start_us = micros();
  uint32_t dt_us = start_us - last_sample_time_us;

  noInterrupts();
  uint32_t current_eg_count = eg_count;
  uint32_t current_wl_count = wl_count;
  interrupts();

  float eg_rpm = (current_eg_count - last_eg_count) *
                 ROTATIONS_PER_ENGINE_COUNT / dt_us * MICROSECONDS_PER_SECOND *
                 60.0;
  float ms_rpm = (current_wl_count - last_wl_count) *
                 MEASURED_GEAR_ROTATIONS_PER_COUNT / dt_us *
                 MICROSECONDS_PER_SECOND * 60.0;

  last_eg_count = current_eg_count;
  last_wl_count = current_wl_count;
  last_sample_time_us = start_us;

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
  if (!SD.sdfs.begin(SdioConfig(DMA_SDIO))) {
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

    sprintf((char*)header_message.timestamp_human.arg,
            "%04d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(),
            minute(), second());
    header_message.clock_us = micros();
    header_message.p_gain = PROPORTIONAL_GAIN;
    header_message.d_gain = DERIVATIVE_GAIN;
    header_message.engine_rpm_winter_cutoff_frequency =
        EG_RPM_WINTER_CUTOFF_FREQ;
    header_message.secondary_rpm_winter_cutoff_frequency =
        SD_RPM_WINTER_CUTOFF_FREQ;
    header_message.wheel_ref_low_rpm = WHEEL_REF_LOW_RPM;
    header_message.wheel_ref_high_rpm = WHEEL_REF_HIGH_RPM;
    header_message.wheel_ref_breakpoint_secondary_rpm =
        WHEEL_REF_BREAKPOINT_SECONDARY_RPM;

    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_encode(&ostream, &HeaderMessage_msg, &header_message);
    size_t message_length = ostream.bytes_written;

    log_file.printf("%01X", HEADER_MESSAGE_ID);
    log_file.printf("%04X", message_length);
    log_file.write(buffer, message_length);
    log_file.flush();

    free(header_message.timestamp_human.arg);

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
      EG_INTERRUPT_PIN, []() { ++eg_count; }, CHANGE);
  attachInterrupt(
      WL_INTERRUPT_PIN, []() { ++wl_count; }, RISING);

  // Attach operating mode interrupt
  Serial.printf("Attaching interrupt mode %d\n", kMode);
  last_sample_time_us = micros();
  switch (kMode) {
    case OPERATING_MODE:
      odrive_can.set_input_vel(ACTUATOR_AXIS, 0, 0);
      odrive_can.set_state(ACTUATOR_AXIS, ODRIVE_VELOCITY_CONTROL_STATE);
      //timer.begin(control_function, CONTROL_FUNCTION_INTERVAL_US);
      break;
    case SERIAL_DEBUG_MODE:
      //timer.begin(serial_debugger, SERIAL_DEBUGGER_INTERVAL_US);
      break;
  }
}
void loop() {
  uint32_t loop_start_us = micros();
  control_function();
  while (micros() - loop_start_us < CONTROL_FUNCTION_INTERVAL_US) {}
}
