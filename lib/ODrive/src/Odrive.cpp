#include <Constants.h>
#include <Odrive.h>

// This is magic sauce that makes the << operator work with the Odrive
template <class T>
inline Print& operator<<(Print& obj, T arg)
{
  obj.print(arg);
  return obj;
}
template <>
inline Print& operator<<(Print& obj, float arg)
{
  obj.print(arg, 4);
  return obj;
}

Odrive::Odrive(HardwareSerial& serial) : odrive_serial(serial)
{}

int Odrive::init_connection()
{
    odrive_serial.begin(ODRIVE_BAUD_RATE);
    return 0;
}

bool Odrive::set_state(int state, int axis)
{
    if (state == current_state) return false;
    odrive_serial << "w axis" << axis << ".requested_state " << state << '\n';
    current_state = state;
    return true;
}