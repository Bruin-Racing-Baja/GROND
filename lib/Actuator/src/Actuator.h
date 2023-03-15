#ifndef actuator_h
#define actuator_h

#include <Constants.h>
#include <OdriveCAN.h>

class Actuator {
 public:
  Actuator(OdriveCAN* odrive_in);
  bool init();
  bool encoder_index_search();
  bool actuator_homing();
  float update_speed(float target_speed);

  // Getters
  int query_readout();
  int get_readout(float readout[8]);

 private:
  int status;
  float current_speed = 0.0;
  int axis_number = ACTUATOR_AXIS;
  float set_speed(float set_speed);

  OdriveCAN* odrive;
};

#endif