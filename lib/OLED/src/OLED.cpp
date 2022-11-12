

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <OLED.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int scrollNum = 0;
int ptrPos = 0;
unsigned long ss_m = 0;

bool OLED::init() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) return 0;
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.display();
  return 1;
}

bool OLED::refresh() {
    display.clearDisplay();
    setFormat();
    display.println("Runtime: "+String((millis()-ss_m)/1000.0));
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

void OLED::setFormat() {
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
}