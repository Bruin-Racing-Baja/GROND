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

        // Debugging / testing functions
        int get_state(int axis);
        int get_encoder_count(int axis);

    private:
        int status;
        int axis_state[2];

        bool set_state(int state, int axis);

        // Helper functions to read Odrive messages
        String read_string();
        float read_float();
        int32_t read_int();


        HardwareSerial& odrive_serial;
};

#endif