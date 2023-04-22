

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OLED.h>
#include <header_message.pb.h>
#include <log_message.pb.h>
#include <pb.h>
#include <pb_common.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS \
  0x3D  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, OLED_RESET);

int scrollNum = 0;
int ptrPos = 0;
unsigned long ss_m = 0;

bool OLED::init() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    return 0;
  display.display();
  delay(2000);  // Pause for 2 seconds
  display.clearDisplay();
  setFormat();
  // display.println(
  //     "Never gonna give you up, never gonna let you down, never gonna run "
  //     "around, and hurt you. Never gonna make you cry, never gonna say "
  //     "goodbye, never gonna tell a lie, and hurt you.");
  display.display();
  return 1;
}

bool OLED::refresh() {
  display.clearDisplay();
  display.display();
  return 1;
}

bool OLED::printInt(int n) {
  display.clearDisplay();
  setFormat();
  display.println(n);
  display.display();
  return 1;
}

bool OLED::printDebug(LogMessage* log_message) {
  if (!enabled)
    return 1;
  display.clearDisplay();
  setFormat();
  switch (scrollNum) {
    case 0:
      display.printf("t: %.1f \nec: %u \nwc: %u \nenc: %d",
                     log_message->control_cycle_start_us / 1.0e6,
                     log_message->engine_count, log_message->wheel_count,
                     log_message->shadow_count);
      break;

    case 1:
      display.printf("erpm: %.1f\nwrpm: %.1f\ntrpm: %.1f\nvcmd: %.1f",
                     log_message->engine_rpm, log_message->wheel_rpm,
                     log_message->target_rpm, log_message->velocity_command);
      break;

    default:
      display.print("Error: Invalid screen number");
      break;
  }

  display.display();
  return 0;
}

void OLED::setFormat() {
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
}

/**
 * @brief Scroll info displayed on OLED
 * @param direction 1 for right, 0 for left
 * @return Number of commanded screen
*/
int OLED::scroll(bool direction) {
  if (direction) {
    if (scrollNum + 1 < NUMBER_OF_PAGES)
      scrollNum++;
  } else {
    if (scrollNum - 1 >= 0)
      scrollNum--;
  }
  return scrollNum;
}