// This project uses the Arduino framework.
#include <BleKeyboard.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 15
#define LED_COUNT 12

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include "led_party.h"  // Custom library for LED effects
#include "button.h"     // Custom library for button handling

#define COND_THR 30     // Threshold for touch sensor to detect a touch

#define CONDUCTIVE_SENSOR_PIN_1 27
#define CONDUCTIVE_SENSOR_PIN_2 32
#define CONDUCTIVE_SENSOR_PIN_3 33

int touch;              // Variable to store the touch sensor reading

// Initialize button objects with their respective pins and thresholds
Button right_bot(CONDUCTIVE_SENSOR_PIN_2, COND_THR, 215);
Button left_bot(CONDUCTIVE_SENSOR_PIN_3, COND_THR, 216);

// Initialize LED effect objects with their respective parameters
Led_Party led1(4, 70, 150, 100);
Led_Party led2(5, 10, 100, 80);
Led_Party led3(6, 100, 150, 100);
Led_Party led4(7, 40, 100, 110);
Led_Party led5(8, 90, 150, 70);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  strip.setBrightness(150);
  Keyboard.begin();
}

void loop() {
  touch = touchRead(CONDUCTIVE_SENSOR_PIN_1);
  while (touch < COND_THR){           // If the cheat touch sensor is pressed
    Keyboard.press(KEY_LEFT_ARROW);   // Simulate pressing the left arrow key
    Keyboard.press(KEY_RIGHT_ARROW);  // Simulate pressing the right arrow key
    // Update LED effects
    led1.Update();
    led2.Update();
    led3.Update();
    led4.Update();
    led5.Update();
    delay(10);
    Keyboard.releaseAll();
    touch = touchRead(CONDUCTIVE_SENSOR_PIN_1);
  }

  // Turn off all LED effects
  led1.Turn_Off();
  led2.Turn_Off();
  led3.Turn_Off();
  led4.Turn_Off();
  led5.Turn_Off();

  // Update the button states
  right_bot.Update();
  left_bot.Update();
}