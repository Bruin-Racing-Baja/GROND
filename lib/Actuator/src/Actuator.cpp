#include <Actuator.h>
#include <Constants.h>
#include <Odrive.h>

// Startup functions

/**
 * Constructor assigns odrive pointer as class member
 * @param odrive_in pointer to odrive CAN object
 */
Actuator::Actuator(OdriveCAN* odrive_in) {
  odrive = odrive_in;
}

/**
 * Initializes connection to physical odrive
 * @return bool if successful
 */
bool Actuator::init() {
  // Due to CAN interrupt handler weirdness
  commanded_axis_state = odrive->get_axis_state(ACTUATOR_AXIS);
  return 1;
}

/**
 * Instructs Odrive to attempt encoder homing
 * @return bool if successful
 */
bool Actuator::encoder_index_search() {
  int state =
      odrive->set_state(ACTUATOR_AXIS, ODRIVE_STATE_ENCODER_INDEX_SEARCH);
  commanded_axis_state = ODRIVE_STATE_ENCODER_INDEX_SEARCH;
  delayMicroseconds(5e6);
  if (state == 0)
    return true;
  else
    return false;
}

/**
 * Run the actuator homing sequence
 * @return bool if successful
 */
bool Actuator::homing_sequence() {
  Serial.println("Start home out");
  update_speed(ACTUATOR_HOMING_VELOCITY);
  delayMicroseconds(2e6);
  while (fabs(odrive->get_vel_estimate(ACTUATOR_AXIS)) >
         ACTUATOR_HOMING_VELOCITY_SPIKE) {
    // !odrive->get_gpio(ODRIVE_ESTOP_OUT_PIN)
    odrive->request_encoder_count(ACTUATOR_AXIS);
    delayMicroseconds(1000);
    estop_out_pos = odrive->get_shadow_count(ACTUATOR_AXIS);
  }
  Serial.println("Start home in");
  update_speed(-ACTUATOR_HOMING_VELOCITY);
  delayMicroseconds(2e6);
  while (fabs(odrive->get_iq_measured(ACTUATOR_AXIS)) <
         ACTUATOR_HOMING_CURRENT_SPIKE) {
    odrive->request_encoder_count(ACTUATOR_AXIS);
    delayMicroseconds(1000);
    belt_pos = odrive->get_shadow_count(ACTUATOR_AXIS);
    odrive->request_iq(ACTUATOR_AXIS);
    delayMicroseconds(1000);
    Serial.println(fabs(odrive->get_iq_measured(ACTUATOR_AXIS)));
  }

  Serial.println("start home disengage");

  set_position((belt_pos + ACTUATOR_HOMING_DISENGAGE_OFFSET) / 8192.0);
  outbound_limit_pos = odrive->get_shadow_count(ACTUATOR_AXIS);
  Serial.printf("belt pos: %d\n", belt_pos);
  delayMicroseconds(5e6);
  return true;
}

// Speed functions

/**
 * If the targeted actuator speed is different than the current speed set it to the updated speed
 * @param target_speed the speed to updtate to
 * @return the current set speed of the actuator
 */
float Actuator::update_speed(float target_speed) {
  if (commanded_axis_state == ODRIVE_STATE_CLOSED_LOOP_CONTROL &&
      target_speed == current_speed) {
    return target_speed;
  }
  return Actuator::set_speed(target_speed);
}

/**
 * Instructs the ODrive object to set given speed
 * @param set_speed the speed to set
 * @return the speed that is set
 */
float Actuator::set_speed(float set_speed) {
  bool can_error =
      !!odrive->set_state(ACTUATOR_AXIS, ODRIVE_STATE_CLOSED_LOOP_CONTROL);
  can_error |= !!odrive->set_controller_modes(ACTUATOR_AXIS,
                                              ODRIVE_CONTROL_MODE_VELOCITY,
                                              ODRIVE_INPUT_MODE_PASSTHROUGH);
  commanded_axis_state = ODRIVE_STATE_CLOSED_LOOP_CONTROL;
  commanded_control_mode = ODRIVE_CONTROL_MODE_VELOCITY;
  can_error |= !!odrive->set_input_vel(ACTUATOR_AXIS, set_speed, 0);
  if (can_error) {
    Serial.printf("Error Setting Speed (CAN Error\n");
  }
  current_speed = set_speed;
  return current_speed;
}

/**
 * Instructs the ODrive object to go to a position
 * @param set_pos the position to set
 * @return the position that is set
 */
int32_t Actuator::set_position(int32_t set_pos) {
  bool can_error =
      !!odrive->set_state(ACTUATOR_AXIS, ODRIVE_STATE_CLOSED_LOOP_CONTROL);
  can_error |= !!odrive->set_controller_modes(ACTUATOR_AXIS,
                                              ODRIVE_CONTROL_MODE_POSITION,
                                              ODRIVE_INPUT_MODE_PASSTHROUGH);
  commanded_axis_state = ODRIVE_STATE_CLOSED_LOOP_CONTROL;
  commanded_control_mode = ODRIVE_CONTROL_MODE_POSITION;
  can_error |= !!odrive->set_input_pos(ACTUATOR_AXIS, set_pos, 0, 0);
  if (can_error) {
    Serial.printf("Error Setting Speed (CAN Error %d)\n", can_error);
  }
  return set_pos;
}

void Actuator::go_to_belt() {
  set_position((belt_pos + ACTUATOR_HOMING_DISENGAGE_OFFSET) / 8192.0);
}

// Readout Functions
/**
 * Provides a readout passed through float array
 * @param readout[5] array to be filled with readout
 * @return 0 as all are member variables
*/
int Actuator::get_readout(float readout[5]) {
  readout[0] = commanded_axis_state;
  readout[1] = commanded_axis_velocity;
  readout[2] = actuator_error;
  readout[3] = homing_error;
  readout[4] = homing_timer;

  return 0;
}