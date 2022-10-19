#ifndef actuator_h
#define actuator_h

#include <Constants.h>
#include <Odrive.h>

class Actuator
{
    public:
        Actuator(Odrive* odrive_in);
        bool init();
        bool encoder_index_search();
        bool actuator_homing();
        float update_speed(float target_speed);

        // Getters
        float get_current_speed();
        int get_status();



    private:
        int status;
        float current_speed = 0.0;
        int axis_number = car_const::ACTUATOR_AXIS;
        float set_speed(float set_speed);

        Odrive* odrive;
};

#endif