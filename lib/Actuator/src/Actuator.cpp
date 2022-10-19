#include <Actuator.h>
#include <Constants.h>
#include <Odrive.h>

// Startup functions

/**
 * Constructor assigns odrive pointer as class member
 */
Actuator::Actuator(Odrive* odrive_in)
{
    odrive = odrive_in;
}

/**
 * Initializes connection to physical odrive
 * Returns bool if successful
 */
bool Actuator::init()
{
    return odrive->init_connection();
}

/**
 * Instructs Odrive to attempt encoder homing
 * Returns a bool if successful
 */
bool Actuator::encoder_index_search()
{
    return odrive->encoder_index_search(car_const::ACTUATOR_AXIS);
}

// Speed functions

/**
 * If the targeted actuator speed is different than the current speed set it to the updated speed
 * 
 * Returns the current set speed of the actuator
 */
float Actuator::update_speed(float target_speed)
{
        return Actuator::set_speed(target_speed);
}

/**
 * Instructs the ODrive object to set given speed
 * 
 * Returns the speed that is set
 */
float Actuator::set_speed(float set_speed)
{
    odrive->set_velocity(set_speed, car_const::ACTUATOR_AXIS);
    current_speed = set_speed;
    return current_speed;
}

// Getter functions

int Actuator::get_status()
{
    return status;
}

float Actuator::get_current_speed()
{
    return current_speed;
}