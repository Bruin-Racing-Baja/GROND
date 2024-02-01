#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <oled.h>

OLED::OLED(int pin) {
  button_pin = pin;
  Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);
}

void OLED::init() {
  lastButtonState = LOW;
  currentScreen = -1;

  Serial.print("hellloooo");
  //displays initial screen on OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Click the Button");
  display.println("to rotate through values.");
  display.display();

  pinMode(button_pin, INPUT);
}

void OLED::displayScreens(String valName, double valData) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(valName);
  display.println(valData);

  display.display();
}

void OLED::toggleScreen(int numVars) {
  if (currentScreen < numVars - 1) {
    currentScreen++;
  } else {
    currentScreen = 0;
  }
}

void OLED::monitorScreen(String valueNames[], double valueData[], int numVars) {
  int reading = digitalRead(button_pin);
  if (reading != lastButtonState) {
    //reset debouncer timer
    lastDebounceTime = millis();
  }

  //if valid reading
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {  //ASK IF THERE'S HARM IN JUST ASSIGNING IT
      buttonState = reading;
    }

    if (buttonState == HIGH) {
      displayScreens(valueNames[currentScreen], valueData[currentScreen]);
      toggleScreen(numVars);
    }
  }

  lastButtonState = reading;
}