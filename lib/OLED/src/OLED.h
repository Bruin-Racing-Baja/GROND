#ifndef oled_h
#define oled_h

#include <Adafruit_SSD1306.h>
#include <log_message.pb.h>
#include <pb.h>

const int OLED_SCROLL_LEFT = 0;
const int OLED_SCROLL_RIGHT = 1;
const int OLED_NUMBER_OF_PAGES = 2;
const int OLED_SCREEN_WIDTH = 128;
const int OLED_SCREEN_HEIGHT = 64;
const uint8_t OLED_RESET_PIN = -1;
const uint8_t OLED_SCREEN_ADDRESS = 0x3D;

class OLED {
 public:
  OLED();
  bool init();
  void refresh();
  void printInitMessage();
  void printDebug(LogMessage* log_message);
  int scroll(int direction);
  void troll();

 private:
  void setFormat();
  Adafruit_SSD1306 display;
  int current_page = 0;
};

#endif