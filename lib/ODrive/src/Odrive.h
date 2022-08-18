#ifndef odrive_h
#define odrive_h

#include <HardwareSerial.h>

class Odrive
{
    public:
        Odrive(HardwareSerial& serial);
        int init_connection();
        void set_velocity(float velocity, int axis);


    private:
        int status;
        int current_state;
        bool set_state(int state, int axis);
        HardwareSerial& odrive_serial;
}

#endif