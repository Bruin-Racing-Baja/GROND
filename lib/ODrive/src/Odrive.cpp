#include <Arduino.h>

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
bool Odrive::encoder_index_search(int axis)
{
  // Send desired state to encoder index search
  Odrive::set_state(ODRIVE_ENCODER_INDEX_SEARCH_STATE, axis);

  // Wait until process is done by checking if can read access
  int delay_ms = 100;
  int timeout_counter = 10;
  Serial.println("Starting index search loop");
  do
  {
    delay(delay_ms);
    odrive_serial << "r axis" << axis << ".current_state\n";
    Serial.println(timeout_counter);
  } while (Odrive::read_int() != ODRIVE_IDLE_STATE && --timeout_counter > 0);
  
  if (timeout_counter > 0)
  {
    // Success, move to idle state
    axis_state[axis] = ODRIVE_IDLE_STATE;
    return true;
  }
  // Timeout, report timeoue error
  status = 1;
  return false;
}

// General use functions

/**
 * Instructs given axis to go to given velocity
 * Will update state to velocity control if it is not already
 * Returns false if set_state fails
 */
bool Odrive::set_velocity(float velocity, int axis)
{
  if (axis_state[axis] != ODRIVE_VELOCITY_CONTROL_STATE)
  {
    if (!Odrive::set_state(ODRIVE_VELOCITY_CONTROL_STATE, axis))
    {
      // Unable to set correct state
      return false;
    }
  }
  odrive_serial << "v " << axis << " " << velocity << " "
               << "0.0f"
               << "\n";
  ;
  return true;
}

/**
 * Instructs odrive to go to idle state
 * Returns if setting state is successful
 */
bool Odrive::idle(int axis)
{
  return Odrive::set_state(ODRIVE_IDLE_STATE, axis);
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

/**
 * Query encoder count from odrive
 * Returns float of the result
 */
int Odrive::get_encoder_count(int axis)
{
  odrive_serial << "r axis" << axis << ".encoder.shadow_count\n";
  return Odrive::read_int();
}

// Private functions

/**
 * Given a state (use constants) and axis attempt to set odrive state
 * Sets current_state if successful
 * Returns bool if successful
 */
bool Odrive::set_state(int state, int axis)
{
  if (state == axis_state[axis]) return false;
  odrive_serial << "w axis" << axis << ".requested_state " << state << '\n';
  axis_state[axis] = state;
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

// Debugging or testing functions

/**
 * Returns the current state for the given axis
 * Returns int of state
 */
int Odrive::get_state(int axis)
{
  return axis_state[axis];
}