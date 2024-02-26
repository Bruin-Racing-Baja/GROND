#include <Arduino.h>
#include <Oled.h>
// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

OLED::OLED(int pin) : display(OLED_WIDTH, OLED_HEIGHT, &Wire2, OLED_RESET_PIN) {
  button_pin = pin;
}

void OLED::init() {
  lastButtonState = HIGH;
  buttonState = HIGH;
  currentScreen = 0;

  //displays initial screen on OLED
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

  Serial.printf("Entered Monitor: %d\n", currentScreen);
  int reading = digitalRead(button_pin);
  // if (!reading)
  //   buttonState = LOW;
  // else
  //   buttonState = HIGH;

  Serial.println(reading);
  if (reading != lastButtonState) {
    //reset debouncer timer
    lastDebounceTime = millis();
  }

  //if valid reading
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        displayScreens(valueNames[currentScreen], valueData[currentScreen]);
        toggleScreen(numVars);
      }
    }
  }

  lastButtonState = reading;
}

//something like this to implement a scrolling system

// void /*OLED::*/displayScreens(String valName1, String valName2, String valName3, double valData1, double valData2, double valData3) {
//     display.clearDisplay();
//     display.setTextSize(1);
//     display.setTextColor(WHITE);
//     display.setCursor(0, 0);
//     display.print(valName1);
//     display.println(valData1);
//     display.print(valName2);
//     display.println(valData2);
//     display.print(valName3);
//     display.println(valData3);

//     display.display();
//     display.display();
// }
