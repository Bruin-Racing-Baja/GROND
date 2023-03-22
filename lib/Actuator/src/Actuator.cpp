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
  int state = odrive->set_state(ACTUATOR_AXIS, 6);
  if (state == 0) return true;
  else return false;
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
  odrive->set_input_vel(ACTUATOR_AXIS, set_speed, 0);
  current_speed = set_speed;
  return current_speed;
}

// Readout Functions
/**
 * Provides a readout passed through float array
 * 
 * Returns 0 as all member variables
*/
int Actuator::get_readout(float readout[5]) {
  readout[0] = commanded_axis_state;
  readout[1] = commanded_axis_velocity;
  readout[2] = actuator_error;
  readout[3] = homing_error;
  readout[4] = homing_timer;

  return 0;
}