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

// Startup Functions

/**
 * Create member containing pointer to passed serial object
 */
Odrive::Odrive(HardwareSerial& serial) : odrive_serial(serial)
{}

/**
 * Begins serial communication
 * Returns bool if successful
 */
bool Odrive::init_connection()
{
  odrive_serial.begin(ODRIVE_BAUD_RATE);
  long start = millis();
  while (Odrive::get_bus_voltage() <= 1)
  {
    if (millis() - start > ODRIVE_DEFAULT_TIMEOUT)
    {
      status = 1;
      return !status;
    }
  }
  status = 0;
  return !status;
}

/**
 * Run the encoder index search state of the odrive
 * Uses default odrive timeout
 * Returns true if succesful, false if timeout
 */
bool Odrive::encoder_index_search()
{
  // Send desired state to encoder index search
  current_state = ODRIVE_ENCODER_INDEX_SEARCH_STATE;
  Odrive::set_state(current_state, ACTUATOR_AXIS);

  // Wait until process is done by checking if can read access
  int timeout_counter = ODRIVE_DEFAULT_TIMEOUT;
  do
  {
    delay(100);
    odrive_serial << "r axis" << ACTUATOR_AXIS << ".current_state\n";
  } while (Odrive::read_int() != 1 && --timeout_counter > 0);
  
  if (timeout_counter > 0)
  {
    // Success, move to idle state
    current_state = ODRIVE_IDLE_STATE;
    return true;
  }
  // Timeout, report timeoue error
  status = 1;
  return false;
}

// Functions to query data from Odrive

/**
 * Read and return bus voltage from odrive
 */
float Odrive::get_bus_voltage()
{
  odrive_serial << "r vbus_voltage\n";
  return Odrive::read_float();
}

// Private functions

/**
 * Given a state (use constants) and axis attempt to set odrive state
 * 
 * Returns bool if successful
 */
bool Odrive::set_state(int state, int axis)
{
  if (state == current_state) return false;
  odrive_serial << "w axis" << axis << ".requested_state " << state << '\n';
  current_state = state;
  return true;
}

// Odrive reading functions

/**
 * Reads a string from the odrive serial buffer (stops at newline)
 * Returns the gathered string
 */
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

/**
 * Reads a string from the odrive serial buffer and casts it to a float
 * Returns the float
 */
float Odrive::read_float()
{
  return read_string().toFloat();
}

/**
 * Reads a string from the odrive serial buffer and casts it to an int
 * Returns the int
 */
int32_t Odrive::read_int()
{
  return read_string().toInt();
}