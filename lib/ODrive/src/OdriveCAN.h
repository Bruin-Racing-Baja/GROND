#ifndef odrive_can_h
#define odrive_can_h

#include <Arduino.h>
#include <FlexCAN_T4.h>

#define COMMAND_SUCCESS 0
#define COMMAND_ERROR_INVALID_AXIS 1
#define COMMAND_ERROR_INVALID_COMMAND 2
#define COMMAND_ERROR_WRITE_FAILED 3

#define CAN_HEARTBEAT_MSG 0x1
#define CAN_ESTOP_MSG 0x2
#define CAN_GET_MOTOR_ERROR 0x3
#define CAN_GET_ENCODER_ERROR 0x4
#define CAN_GET_SENSORLESS_ERROR 0x5
#define CAN_SET_AXIS_NODE_ID 0x6
#define CAN_SET_AXIS_REQUESTED_STATE 0x7
#define CAN_SET_AXIS_STARTUP_CONFIG 0x8
#define CAN_GET_ENCODER_ESTIMATES 0x9
#define CAN_GET_ENCODER_COUNT 0xA
#define CAN_SET_CONTROLLER_MODES 0xB
#define CAN_SET_INPUT_POS 0xC
#define CAN_SET_INPUT_VEL 0xD
#define CAN_SET_INPUT_TORQUE 0xE
#define CAN_SET_LIMITS 0xF
#define CAN_START_ANTICOGGING 0x10
#define CAN_SET_TRAJ_VEL_LIMIT 0x11
#define CAN_SET_TRAJ_ACCEL_LIMITS 0x12
#define CAN_SET_TRAJ_INERTIA 0x13
#define CAN_GET_IQ 0x14
#define CAN_GET_SENSORLESS_ESTIMATES 0x15
#define CAN_REBOOT_ODRIVE 0x16
#define CAN_GET_VBUS_VOLTAGE 0x17
#define CAN_CLEAR_ERRORS 0x18
#define CAN_SET_LINEAR_COUNT 0x19
#define CAN_SET_POSITION_GAIN 0x1A
#define CAN_SET_VEL_GAINS 0x1B
#define CAN_GET_GPIO_STATES CAN_GET_IQ

class OdriveCAN {
 public:
  OdriveCAN() {}
  static FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> odrive_can;

  bool init(void (*parse)(const CAN_message_t& msg));

  void parse_message(const CAN_message_t& msg);

  // Requesters
  int request_readout(int axis);

  int request_motor_error(int axis);
  int request_encoder_error(int axis);
  int request_sensorless_error(int axis);
  int request_encoder_count(int axis);
  int request_iq(int axis);
  int request_sensorless_estimates(int axis);
  int request_vbus_voltage();
  int request_gpio_states();

  // Getters
  int get_readout(int axis, float report[19]);

  uint32_t get_time_since_heartbeat_ms();
  uint32_t get_axis_error(int axis);
  uint8_t get_axis_state(int axis);
  uint8_t get_motor_flags(int axis);
  uint8_t get_encoder_flags(int axis);
  uint8_t get_controller_flags(int axis);
  uint32_t get_motor_error(int axis);
  uint32_t get_encoder_error(int axis);
  uint32_t get_sensorless_error(int axis);
  float get_vel_estimate(int axis);
  float get_pos_estimate(int axis);
  int32_t get_shadow_count(int axis);
  int32_t get_count_in_cpr(int axis);
  float get_iq_setpoint(int axis);
  float get_iq_measured(int axis);
  float get_sensorless_vel_estimate(int axis);
  float get_sensorless_pos_estimate(int axis);
  float get_voltage();
  float get_current();
  uint32_t get_gpio_states();
  uint8_t get_gpio(uint8_t pin);

  // Commands
  int start_anticogging(int axis);
  int reboot();
  int clear_errors();

  // Setters
  int set_state(int axis, int state);
  int set_axis_node_id(int axis, int axis_node_id);
  int set_controller_modes(int axis, int control_mode, int input_mode);
  int set_input_pos(int axis, float input_pos, int16_t vel_ff,
                    int16_t torque_ff);
  int set_input_vel(int axis, float input_vel, float torque_ff);
  int set_input_torque(int axis, float input_torque);
  int set_limits(int axis, float current_limit, float vel_limit);
  int set_traj_vel_limit(int axis, float traj_vel_limit);
  int set_traj_accel_limits(int axis, float traj_decel_limit,
                            float traj_accel_limit);
  int set_traj_intertia(int axis, float traj_inertia);
  int set_linear_count(int axis, float position);
  int set_pos_gain(int axis, float pos_gain);
  int set_vel_gains(int axis, float vel_gain, float vel_integrator_gain);

 private:
  uint32_t last_heartbeat_ms = 0;
  uint32_t axis_error[2];
  uint8_t axis_state[2], motor_flags[2], encoder_flags[2], controller_flags[2];
  uint32_t motor_error[2], encoder_error[2], sensorless_error[2];
  float vel_estimate[2], pos_estimate[2];
  int32_t shadow_count[2], count_in_cpr[2];
  float iq_setpoint[2], iq_measured[2];
  float sensorless_vel_estimate[2], sensorless_pos_estimate[2];
  float vbus_voltage, vbus_current;
  uint32_t gpio_states;

  int send_command(int axis, int cmd_id, bool remote, uint8_t buf[8]);
  int send_command(int cmd_id, bool remote, uint8_t buf[8]);

  int send_empty_command(int axis_id, int cmd_id, bool remote);
  int send_empty_command(int cmd_id, bool remote);
};
#endif