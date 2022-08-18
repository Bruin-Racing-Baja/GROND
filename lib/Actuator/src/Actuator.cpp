#include <Actuator.h>
#include <Constants.h>

Actuator::Actuator(Odrive odrive)
{
    int huh = 0;
}


// Speed functions

/**
 * If the targeted actuator speed is different than the current speed set it to the updated speed
 * 
 * Returns the current set speed of the actuator
 */
float Actuator::update_speed(float target_speed)
{
    if (target_speed != current_speed)
    {
        return Actuator::set_speed(target_speed);
    }
    else
    {
        return current_speed;
    }
}

/**
 * Instructs the ODrive object to set given speed
 * 
 * Returns the speed that is set
 */
float Actuator::set_speed(float set_speed)
{
    // Odrive stuff here
    current_speed = set_speed;
    return current_speed;
}

// Homing functions
/**
 * Instructs the ODrive to home the encoder
 * 
 * Returns whether or not the operation succeeded or timed out
 */
bool Actuator::encoder_homing()
{
    return 1;
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