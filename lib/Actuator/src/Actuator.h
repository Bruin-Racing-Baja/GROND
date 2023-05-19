#ifndef actuator_h
#define actuator_h

#include <Arduino.h>
#include <Constants.h>
#include <OdriveCAN.h>

class Actuator {
 public:
  Actuator(OdriveCAN* odrive_in);
  bool init();
  bool encoder_index_search();
  bool homing_sequence();
  float update_speed(float target_speed);

  int32_t set_position(int32_t set_pos);
  int go_to_relative_belt_pos(int num_turns_offset);

  // Getters
  int get_readout(float readout[5]);

 private:
  int actuator_error = 0;
  int homing_error = 0;
  int homing_timer = 0;
  float current_speed = 0.0;
  uint8_t commanded_axis_state = -1;
  uint8_t commanded_control_mode = -1;
  float commanded_axis_velocity = 0.0;

  uint32_t estop_out_pos = 0;
  uint32_t belt_pos = 0;
  uint32_t outbound_limit_pos = 0;

  // Constants
  int axis_number = ACTUATOR_AXIS;
  // Functions
  float set_speed(float set_speed);
  OdriveCAN* odrive;
};

#endif