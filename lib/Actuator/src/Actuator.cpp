#include <Actuator.h>
#include <Constants.h>
#include <Odrive.h>

// Startup functions

/**
 * Constructor assigns odrive pointer as class member
 */
Actuator::Actuator(OdriveCAN* odrive_in) {
  odrive = odrive_in;
}

/**
 * Initializes connection to physical odrive
 * Returns bool if successful
 */
bool Actuator::init() {
  // Due to CAN interrupt handler weirdness
  return 1;
}

/**
 * Instructs Odrive to attempt encoder homing
 * Returns a bool if successful
 */
bool Actuator::encoder_index_search() {
  return odrive->encoder_index_search(ACTUATOR_AXIS);
}

// Speed functions

/**
 * If the targeted actuator speed is different than the current speed set it to the updated speed
 * 
 * Returns the current set speed of the actuator
 */
float Actuator::update_speed(float target_speed) {
  return Actuator::set_speed(target_speed);
}

/**
 * Instructs the ODrive object to set given speed
 * 
 * Returns the speed that is set
 */
float Actuator::set_speed(float set_speed) {
  odrive->set_velocity(set_speed, ACTUATOR_AXIS);
  current_speed = set_speed;
  return current_speed;
}

// Readout Functions

/**
 * Asks the ODrive to query current actuator information
 * 
 * Returns success code
*/
int Actuator::query_readout() {
  int result = 0;
  result += odrive->request_motor_error(ACTUATOR_AXIS);
  result += odrive->request_encoder_error(ACTUATOR_AXIS);
  result += odrive->request_sensorless_error(ACTUATOR_AXIS);

  result += odrive->request_encoder_count(ACTUATOR_AXIS);
  result += odrive->request_sensorless_estimates(ACTUATOR_AXIS);

  result += odrive->request_vbus_voltage();

  return result;
}

/**
 * Returns the previously queried actuator readout
*/
int Actuator::get_readout(float readout[8]) {
  readout[0] = odrive->get_motor_error(ACTUATOR_AXIS);
  readout[1] = odrive->get_encoder_error(ACTUATOR_AXIS);
  readout[2] = odrive->get_sensorless_error(ACTUATOR_AXIS);

  readout[3] = odrive->get_encoder_count(ACTUATOR_AXIS);
  readout[4] = odrive->get_sensorless_estimates(ACTUATOR_AXIS);

  readout[5] = odrive->get_vbus_voltage();

  return 0;
}