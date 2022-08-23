#ifndef odrive_h
#define odrive_h

#include <HardwareSerial.h>

class Odrive
{
    public:
        Odrive(HardwareSerial& serial);
        bool init_connection();
        bool set_velocity(float velocity, int axis);
        bool encoder_index_search(int axis);
        float get_bus_voltage();
        bool idle(int axis);


    private:
        int status;
        int current_state;

        bool set_state(int state, int axis);

        // Functions to query information from Odrive
        

        // Helper functions to read Odrive messages
        String read_string();
        float read_float();
        int32_t read_int();


        HardwareSerial& odrive_serial;
};

#endif