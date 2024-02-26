#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
const int OLED_RESET_PIN = -1;

class OLED {

  public: 
    OLED(int pin); 
    void init(); 
    void monitorScreen(String valueNames[], double valueData[], int numVars); 
    

  private:
    // Variables will change:
    int buttonState;      // the current reading from the input pin
    int lastButtonState;  // the previous reading from the input pin
    Adafruit_SSD1306 display; 
    int button_pin; 
    
    int currentScreen; 
    unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long debounceDelay = 10; // the debounce time; increase if the output flickers

    //Helper Functions:
    void toggleScreen(int numVars); 
    void displayScreens(String valName, double valData);

};

#endif //OLED_H