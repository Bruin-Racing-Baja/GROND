#ifndef oled_h
#define oled_h

#include <header_message.pb.h>
#include <log_message.pb.h>
#include <pb.h>
#include <pb_common.h>

class OLED {
 public:
  bool init();
  bool refresh();
  bool printInt(int n);
  bool printDebug(LogMessage* log_message);

 private:
  void setFormat();
};

#endif