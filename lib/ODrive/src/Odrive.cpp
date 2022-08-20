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

bool Odrive::encoder_homing()
{
  // Send desired state to encoder homing
  Odrive::set_state(ODRIVE_ENCODER_CALIBRATION_STATE, ACTUATOR_AXIS);

  // Wait until process is done by checking if can read access
  int timeout_counter = ODRIVE_DEFAULT_TIMEOUT;
  do
  {
    delay(100);
    odrive_serial << "r axis" << ACTUATOR_AXIS << ".current_state\n";
  } while (Odrive::read_int() != 1 && --timeout_counter > 0);
  
  if (timeout_counter > 0)
  {
    current_state = ODRIVE_ENCODER_CALIBRATION_STATE;
    return true;
  }
  return false;
}

bool Odrive::set_state(int state, int axis)
{
  if (state == current_state) return false;
  odrive_serial << "w axis" << axis << ".requested_state " << state << '\n';
  current_state = state;
  return true;
}

// Odrive reading functions

String Odrive::read_string()
{
  String str = "";
  static const unsigned long timeout = ODRIVE_DEFAULT_TIMEOUT;
  unsigned long timeout_start = millis();
  for (;;)
  {
    while (!odrive_serial.available())
    {
      if (millis() - timeout_start >= timeout)
      {
        return str;
      }
    }
    char c = odrive_serial.read();
    if (c == '\n')
      break;
    str += c;
  }
  return str;
}

float Odrive::read_float()
{
  return read_string().toFloat();
}

int32_t Odrive::read_int()
{
  return read_string().toInt();
}