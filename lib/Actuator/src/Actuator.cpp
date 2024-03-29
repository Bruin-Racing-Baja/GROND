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
      odrive->set_state(ACTUATOR_AXIS, ODRIVE_ENCODER_INDEX_SEARCH_STATE);
  commanded_axis_state = ODRIVE_ENCODER_INDEX_SEARCH_STATE;
  delayMicroseconds(5 * 1000000);
  if (state == 0)
    return true;
  else
    return false;
}

// Speed functions

/**
 * If the targeted actuator speed is different than the current speed set it to the updated speed
 * @param target_speed the speed to updtate to
 * @return the current set speed of the actuator
 */
float Actuator::update_speed(float target_speed) {
  if (commanded_axis_state == ODRIVE_VELOCITY_CONTROL_STATE &&
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
  set_speed = constrain(set_speed, -VEL_LIMIT, VEL_LIMIT);
  int can_error =
      odrive->set_state(ACTUATOR_AXIS, ODRIVE_VELOCITY_CONTROL_STATE);
  commanded_axis_state = ODRIVE_VELOCITY_CONTROL_STATE;
  can_error =
      (can_error << 1) | odrive->set_input_vel(ACTUATOR_AXIS, set_speed, 0);
  if (can_error != 0) {
    Serial.printf("Error Setting Speed (CAN Error %d)\n", can_error);
  }
  current_speed = set_speed;
  return current_speed;
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