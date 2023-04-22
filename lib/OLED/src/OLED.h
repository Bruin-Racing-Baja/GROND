#ifndef oled_h
#define oled_h

#include <header_message.pb.h>
#include <log_message.pb.h>
#include <pb.h>
#include <pb_common.h>

const int NUMBER_OF_PAGES = 2;

class OLED {
 public:
  bool init();
  bool refresh();
  bool printInt(int n);
  bool printDebug(LogMessage* log_message);
  int scroll(bool direction);

 private:
  void setFormat();
  int scrollNum = 0;
  bool enabled = false;
};

#endif