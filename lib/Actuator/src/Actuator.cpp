#include <Actuator.h>

Actuator::Actuator()
{
    int huh = 0;
}


/**
 * Update speed of actuator
 * 
 * If the targeted actuator speed is different than the current speed set it to the updated speed
 * 
 * Returns the current speed of the actuator
 * 
 * 
 * 
 * 
 * 
 * 
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

float Actuator::set_speed(float set_speed)
{
    // Odrive stuff here
    current_speed = set_speed;
    return current_speed;
}


float Actuator::get_current_speed()
{
    return current_speed;
}