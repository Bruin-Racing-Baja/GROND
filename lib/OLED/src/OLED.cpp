#include <OLED.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

#define BUTTON_UP  13
#define BUTTON_DN  12
#define BUTTON_LT  11
#define BUTTON_RT  10
#define BUTTON_OK   9


bool OLED::init() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    display.display();
    delay(1000);

    // Clear the buffer.
    display.clearDisplay();
    display.display();

    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DN, INPUT_PULLUP);
    pinMode(BUTTON_LT, INPUT_PULLUP);
    pinMode(BUTTON_RT, INPUT_PULLUP);
    pinMode(BUTTON_OK, INPUT_PULLUP);

    // text display tests 
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Screen Test");
    display.display(); // actually display all of the above
    delay(1000);
    display.clearDisplay();
    display.display();

  return 1;
}

bool OLED::refresh() {
    display.clearDisplay();

    if(!digitalRead(BUTTON_UP)) {
        if(!bBools[0]) {
        if(ptrPos>0) ptrPos--;
        bBools[0] = true;
        ss_m = millis();
        }
        if(millis()-ss_m >= 1000) ssBools[0] = true;
        if(ssBools[0] && millis()-ss_m >= 100) {
        if(ptrPos>0) ptrPos--;
        ss_m = millis();
        }
    }
    if(!digitalRead(BUTTON_DN)) {
        if(!bBools[1]) {
        ptrPos++;
        bBools[1] = true;
        ss_m = millis();
        }
        if(millis()-ss_m >= 1000) ssBools[1] = true;
        if(ssBools[1] && millis()-ss_m >= 100) {
        ptrPos++;
        ss_m = millis();
        }
    }
    if(!digitalRead(BUTTON_LT)) display.write(0x1B);
    if(!digitalRead(BUTTON_RT)) display.write(0x1A);
    if(!digitalRead(BUTTON_OK) && !bBools[4]) {
        scrollNum++;
        bBools[4] = true;
    }
    
    if(digitalRead(BUTTON_UP)) {bBools[0] = 0; ssBools[0]=0;}
    if(digitalRead(BUTTON_DN)) {bBools[1] = 0; ssBools[1]=0;}
    if(digitalRead(BUTTON_LT) & bBools[2]) bBools[2] = 0;
    if(digitalRead(BUTTON_RT) & bBools[3]) bBools[3] = 0;
    if(digitalRead(BUTTON_OK) & bBools[4]) bBools[4] = 0;
    delay(10);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    
    String line1 = " Line "+String(scrollNum+1);
    String line2 = " Line "+String(scrollNum+2);
    String line3 = " Line "+String(scrollNum+3);
    
    char ptr = (char)26;
    int dispMaxLine = scrollNum + 2;
    int dispMinLine = scrollNum;
    if(ptrPos>dispMaxLine) scrollNum++;
    else if(ptrPos<dispMinLine) scrollNum--;
    else if(ptrPos == dispMinLine) line1[0]=ptr;
    else if(ptrPos == dispMinLine+1) line2[0]=ptr;
    else line3[0]=ptr;
    
    display.println("I/O Test "+String((millis()-ss_m)/1000.0));
    display.println(line1);
    display.println(line2);
    display.println(line3);
    yield();
    display.display();

    return 1;
}