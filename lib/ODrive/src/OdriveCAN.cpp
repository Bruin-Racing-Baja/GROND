#include <FlexCAN_T4.h>
#include <OdriveCAN.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> OdriveCAN::odrive_can;

bool OdriveCAN::init(void (*parse)(const CAN_message_t& msg)) {
  OdriveCAN::odrive_can.begin();
  OdriveCAN::odrive_can.setBaudRate(250000);
  OdriveCAN::odrive_can.setMaxMB(16);
  OdriveCAN::odrive_can.enableFIFO();
  OdriveCAN::odrive_can.enableFIFOInterrupt();
  OdriveCAN::odrive_can.onReceive(parse);
  return true;
}

int OdriveCAN::send_command(int axis, int cmd_id, bool remote, uint8_t buf[8]) {
  CAN_message_t msg;
  msg.id =
      (axis << 5) | cmd_id;  // TODO: ensure axis is 0/1; ensure cmd_id is valid
  msg.len = 8;
  memcpy(&msg.buf, buf, 8);
  msg.flags.remote = remote;
  OdriveCAN::odrive_can.write(msg);
  return 1;
}

int OdriveCAN::send_command(int cmd_id, bool remote, uint8_t buf[8]) {
  return OdriveCAN::send_command(0, cmd_id, remote, buf);
}

int OdriveCAN::send_command(int axis_id, int cmd_id, bool remote) {
  uint8_t buf[8] = {0};
  return OdriveCAN::send_command(0, cmd_id, remote, buf);
}

int OdriveCAN::send_command(int cmd_id, bool remote) {
  return OdriveCAN::send_command(0, cmd_id, remote);
}

void OdriveCAN::parse_message(const CAN_message_t& msg) {
  uint32_t axis = msg.id >> 5;
  uint32_t cmd_id = msg.id & 0x1F;

  switch (cmd_id) {
    case CAN_HEARTBEAT_MSG:
      // Cyclic message; sent every 100ms
      last_heartbeat = millis();
      memcpy(&axis_error[axis], msg.buf, 4);
      memcpy(&axis_state[axis], msg.buf + 4, 1);
      memcpy(&motor_flags[axis], msg.buf + 5, 1);
      memcpy(&encoder_flags[axis], msg.buf + 6, 1);
      memcpy(&controller_flags[axis], msg.buf + 7, 1);
      break;
    case CAN_GET_MOTOR_ERROR:
      memcpy(&motor_error[axis], msg.buf, 4);
      break;
    case CAN_GET_ENCODER_ERROR:
      memcpy(&encoder_error[axis], msg.buf, 4);
      break;
    case CAN_GET_SENSORLESS_ERROR:
      memcpy(&sensorless_error[axis], msg.buf, 4);
      break;
    case CAN_GET_ENCODER_ESTIMATES:
      // Cyclic message; sent every 10ms
      memcpy(&vel_estimate[axis], msg.buf, 4);
      memcpy(&pos_estimate[axis], msg.buf + 4, 4);
      break;
    case CAN_GET_ENCODER_COUNT:
      memcpy(&shadow_count[axis], msg.buf, 4);
      memcpy(&count_in_cpr[axis], msg.buf + 4, 4);
      break;
    case CAN_GET_IQ:
      memcpy(&iq_measured[axis], msg.buf + 4, 4);
      memcpy(&iq_setpoint[axis], msg.buf, 4);
      break;
    case CAN_GET_SENSORLESS_ESTIMATES:
      memcpy(&sensorless_vel_estimate[axis], msg.buf + 4, 4);
      memcpy(&sensorless_pos_estimate[axis], msg.buf, 4);
      break;
    case CAN_GET_VBUS_VOLTAGE:
      memcpy(&vbus_voltage, msg.buf, 4);
      memcpy(&vbus_current, msg.buf, 4);
      break;
  }
}

void OdriveCAN::request_motor_error(int axis) {
  send_command(axis, CAN_GET_MOTOR_ERROR, 1);
}

void OdriveCAN::request_encoder_error(int axis) {
  send_command(axis, CAN_GET_ENCODER_ERROR, 1);
}

void OdriveCAN::request_sensorless_error(int axis) {
  send_command(axis, CAN_GET_SENSORLESS_ERROR, 1);
}

void OdriveCAN::request_encoder_count(int axis) {
  send_command(axis, CAN_GET_ENCODER_COUNT, 1);
}

void OdriveCAN::request_iq(int axis) {
  send_command(axis, CAN_GET_IQ, 1);
}

void OdriveCAN::request_sensorless_estimates(int axis) {
  send_command(axis, CAN_GET_SENSORLESS_ESTIMATES, 1);
}

void OdriveCAN::request_vbus_voltage() {
  send_command(CAN_GET_VBUS_VOLTAGE, 1);
}

void OdriveCAN::start_anticogging(int axis) {
  send_command(axis, CAN_START_ANTICOGGING, 0);
}

void OdriveCAN::reboot() {
  send_command(CAN_REBOOT_ODRIVE, 0);
}

void OdriveCAN::clear_errors() {
  send_command(CAN_CLEAR_ERRORS, 0);
}

void OdriveCAN::set_state(int axis, int state) {
  uint8_t buf[8] = {0};
  buf[0] = state;
  send_command(axis, CAN_SET_AXIS_REQUESTED_STATE, 0, buf);
}

uint32_t OdriveCAN::get_axis_error(int axis) {
  return axis_error[axis];
}

uint8_t OdriveCAN::get_axis_state(int axis) {
  return axis_state[axis];
}

uint8_t OdriveCAN::get_motor_flags(int axis) {
  return motor_flags[axis];
}

uint8_t OdriveCAN::get_encoder_flags(int axis) {
  return encoder_flags[axis];
}

uint8_t OdriveCAN::get_controller_flags(int axis) {
  return controller_flags[axis];
}

uint32_t OdriveCAN::get_motor_error(int axis) {
  return motor_error[axis];
}

uint32_t OdriveCAN::get_encoder_error(int axis) {
  return encoder_error[axis];
}

uint32_t OdriveCAN::get_sensorless_error(int axis) {
  return sensorless_error[axis];
}

float OdriveCAN::get_vel_estimate(int axis) {
  return vel_estimate[axis];
}

float OdriveCAN::get_pos_estimate(int axis) {
  return pos_estimate[axis];
}

int32_t OdriveCAN::get_shadow_count(int axis) {
  return shadow_count[axis];
}

int32_t OdriveCAN::get_count_in_cpr(int axis) {
  return count_in_cpr[axis];
}

float OdriveCAN::get_iq_setpoint(int axis) {
  return iq_setpoint[axis];
}

float OdriveCAN::get_iq_measured(int axis) {
  return iq_measured[axis];
}

float OdriveCAN::get_sensorless_vel_estimate(int axis) {
  return sensorless_vel_estimate[axis];
}

float OdriveCAN::get_sensorless_pos_estimate(int axis) {
  return sensorless_pos_estimate[axis];
}

float OdriveCAN::get_voltage() {
  return vbus_voltage;
}

float OdriveCAN::get_current() {
  return vbus_current;
}
/*
  {
    case CAN_SET_AXIS_NODE_ID:
      uint32_t axis_node_id;
      memcpy(&axis_node_id, msg.buf, 4);
      break;
    case CAN_SET_CONTROLLER_MODES:
      int32_t control_mode, input_mode;
      memcpy(&control_mode, msg.buf, 4);
      memcpy(&input_mode, msg.buf + 4, 4);
      break;
    case CAN_SET_INPUT_POS:
      float input_pos;
      int16_t vel_ff, torque_ff;
      memcpy(&input_pos, msg.buf, 4);
      memcpy(&vel_ff, msg.buf + 4, 2);
      memcpy(&torque_ff, msg.buf + 6, 2);
      break;
    case CAN_SET_INPUT_VEL:
      float input_torque_ff, input_vel;
      memcpy(&input_torque_ff, msg.buf, 4);
      memcpy(&input_vel, msg.buf + 4, 4);
      break;
    case CAN_SET_INPUT_TORQUE:
      float input_torque;
      memcpy(&input_torque, msg.buf, 4);
      break;
    case CAN_SET_LIMITS:
      float current_limit, velocity_limit;
      memcpy(&current_limit, msg.buf, 4);
      memcpy(&velocity_limit, msg.buf + 4, 4);
      break;
    case CAN_SET_TRAJ_VEL_LIMIT:
      float traj_vel_limit;
      memcpy(&traj_vel_limit, msg.buf, 4);
      break;
    case CAN_SET_TRAJ_ACCEL_LIMITS:
      float traj_decel_limit, traj_accel_limit;
      memcpy(&traj_decel_limit, msg.buf + 4, 4);
      memcpy(&traj_accel_limit, msg.buf, 4);
      break;
    case CAN_SET_TRAJ_INERTIA:
      float traj_inertia;
      memcpy(&traj_inertia, msg.buf, 4);
      break;
    case CAN_SET_LINEAR_COUNT:
      float position;
      memcpy(&position, msg.buf, 4);
      break;
    case CAN_SET_LINEAR_COUNT:
      float pos_gain;
      memcpy(&pos_gain, msg.buf, 4);
      break;
    case CAN_SET_POSITION_GAIN:
      float vel_gain, vel_integrator_gain;
      memcpy(&vel_gain, msg.buf, 4);
      memcpy(&vel_integrator_gain, msg.buf + 4, 4);
      break;
  }
*/