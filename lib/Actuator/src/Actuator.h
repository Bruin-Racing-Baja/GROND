#ifndef actuator_h
#define actuator_h

class Actuator
{
    public:
        Actuator();
        float update_speed(float target_speed);
        float get_current_speed();



    private:
        float current_speed;
        float set_speed(float set_speed);
}

#endif