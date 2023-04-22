

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OLED.h>
#include <log_message.pb.h>
#include <pb.h>

OLED::OLED()
    : display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire2, OLED_RESET_PIN) {}

bool OLED::init() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_ADDRESS)) {
    return 0;
  }
  // TODO: clean up
  display.display();
  delay(2000);
  display.clearDisplay();
  setFormat();
  display.display();
  return 1;
}

void OLED::refresh() {
  display.clearDisplay();
  display.display();
}

void OLED::printInitMessage() {
  display.clearDisplay();
  setFormat();
  display.println("Initializing...");
  display.display();
}

void OLED::printDebug(LogMessage* log_message) {
  display.clearDisplay();
  setFormat();
  switch (current_page) {
    case 0:
      display.printf("t: %.1f \nec: %u \nwc: %u \nenc: %u",
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
}

void OLED::troll() {
  display.clearDisplay();
  setFormat();
  display.printf(
      "Never gonna give you up, never gonna let you down, never gonna run "
      "around, and hurt you. Never gonna make you cry, never gonna say "
      "goodbye, never gonna tell a lie, and hurt you.\n");
  display.display();
}

void OLED::setFormat() {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
}

/**
 * @brief Scroll info displayed on OLED
 * @param direction 1 for right, 0 for left
 * @return Number of commanded screen
*/
int OLED::scroll(int direction) {
  if (direction == OLED_SCROLL_RIGHT &&
      (current_page + 1) < OLED_NUMBER_OF_PAGES) {
    current_page++;
  } else if (direction == OLED_SCROLL_LEFT && (current_page - 1) >= 0) {
    current_page--;
  }
  return current_page;
}