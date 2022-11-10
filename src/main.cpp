#include <Arduino.h>
#include <FlexCAN_T4.h>

// Libraries
#include <ArduinoLog.h>
#include <HardwareSerial.h>
#include <SD.h>
#include <SPI.h>

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
static constexpr int kMode = 1;

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

  bool estop_in = digitalReadFast(ESTOP_IN_PIN);
  bool estop_out = digitalReadFast(ESTOP_OUT_PIN);
  // if (estop_in){
  //   if (velocity_command < 0){
  //     velocity_command = 0;
  //   }
  // }
  // if (estop_out){
  //   if (velocity_command > 0){
  //     velocity_command = 0;
  //   }
  // }
  digitalWrite(LED_1_PIN, !estop_in);
  digitalWrite(LED_2_PIN, !estop_out);

  actuator.update_speed(velocity_command);

  u_int32_t stop_us = micros();
  Log.notice("%d, %d, %F, %F, %d, %d, %F, %F, %d, %d" CR, start_us, stop_us,
             eg_rpm, wl_rpm, current_eg_count, current_wl_count, error,
             velocity_command, estop_in, estop_out);
  log_file.close();
  log_file = SD.open(log_name.c_str(), FILE_WRITE);
  Serial.println("End");
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

  Serial.print("Index: ");
  actuator.encoder_index_search();
  Serial.println("after index");

  // Create interrupts to count gear teeth
  attachInterrupt(
      EG_INTERRUPT_PIN, []() { ++eg_count; }, RISING);
  attachInterrupt(
      WL_INTERRUPT_PIN, []() { ++wl_count; }, RISING);

  // Attach correct interrupt based on the desired mode
  last_exec_us = micros();
  switch (kMode) {
    case 0:
      timer.begin(control_function, CONTROL_FUNCTION_INTERVAL);
      break;
    case 1:
      timer.begin(serial_debugger, kSerialDebuggerIntervalUs);
      break;
  }

  // And so it begins...
}

void loop() {
  // put your main code here, to run repeatedly:
  while (!Serial) {
    ;
    ;
  }
  if (can1.read(msg)) {
    Serial.print("CAN1 ");
    Serial.print("MB: ");
    Serial.print(msg.mb);
    Serial.print("  ID: 0x");
    Serial.print(msg.id, HEX);
    Serial.print("  EXT: ");
    Serial.print(msg.flags.extended);
    Serial.print("  LEN: ");
    Serial.print(msg.len);
    Serial.print(" DATA: ");
    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(msg.buf[i]);
      Serial.print(" ");
    }
    Serial.print("  TS: ");
    Serial.println(msg.timestamp);
    if (msg.id == 0x017) {
      Serial.print("Voltage: ");
      float f;
      memcpy(&f, msg.buf, 4);
      Serial.println(f);
    }
  }

  if (millis() - t2 > 100) {
    t2 = millis();
    static uint8_t id = 0x017;
    CAN_message_t msgOut;
    msgOut.id = id;
    msgOut.len = 8;
    msgOut.flags.remote = 1;
    msgOut.flags.extended = 0;
    can1.write(msgOut);
  }
}