// Blynk configuration
#define BLYNK_TEMPLATE_ID "Your template id"
#define BLYNK_TEMPLATE_NAME "Your stress reducer"
#define BLYNK_AUTH_TOKEN "Your authentication key"
#define BLYNK_PRINT Serial

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP32Servo.h>
#include <BlynkSimpleEsp32.h>
#include "flasher.h"

// Pin and sensor definitions
#define LED_PIN 4 
#define LED_COUNT 12
#define DHTPIN 15
#define DHTTYPE DHT22
#define SERVO_PIN 23
#define JOYSTICK_SW_PIN 32
#define LIGHT_SENSOR_PIN 34

// DHT sensor setup
DHT_Unified dht(DHTPIN, DHTTYPE);

// LED strip setup
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Servo setup
Servo myservo;
int pos = 0;

// Flasher objects for each LED
Flasher leds[LED_COUNT] = {
  Flasher(0), Flasher(1), Flasher(2), Flasher(3), Flasher(4), 
  Flasher(5), Flasher(6), Flasher(7), Flasher(8), Flasher(9), 
  Flasher(10), Flasher(11)
};

// Breath cycle variables
const int maxBrightness = 164; // Maximum brightness
bool breathingIn = true;
const int fadeInDuration = 5000; // 5 seconds for fade in
const int fadeOutDuration = 3000; // 3 seconds for fade out
const unsigned long fadeDuration = fadeInDuration + fadeOutDuration; // Total duration for one breath cycle
unsigned long startTime = 0;
unsigned long elapsedTime = 0;
int breathCyclesCompleted = 0;
bool breathingOutPhaseEnded = false;

// WiFi credentials
const char ssid[] = "Your SSID";
const char pass[] = "You password";

// Blynk timer
BlynkTimer timer;

// Function declarations
void myTimerEvent();
void led_strip_update();
void led_strip_fade_off();
void send_sms();
void play_music();
void stop_music();

// State variables for Blynk buttons
int breath_train = 0;
int diffude = 0;
unsigned long last_defuse = 0;

// State variables for music and SMS
unsigned long last_sms = 0;
unsigned long last_music_switch = 0;
bool box_is_open = false;

// Duration constants
const unsigned long defuseDuration = 3000;
const unsigned long musicSwitchDelay = 1000;
const unsigned long smsCooldown = 5000;

// Light sensor threshold
const int lightSensorThreshold = 3000;

void setup() {
  // Serial setup
  Serial.begin(115200);
  
  // LED strip setup
  strip.setPin(LED_PIN);
  strip.begin();
  strip.show();
  strip.setBrightness(maxBrightness);

  // DHT sensor setup
  dht.begin();

  // Blynk setup
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);

  // Joystick button setup
  pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP);

  // Servo setup
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50); // standard 50 hz servo
  myservo.attach(SERVO_PIN, 500, 2500);
  myservo.write(pos);
}

// Timer event for reading sensor data and updating Blynk
void myTimerEvent() {
  // Read temperature from the DHT sensor
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    Blynk.virtualWrite(V0, event.temperature);
  }

  // Read humidity from the DHT sensor
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    Blynk.virtualWrite(V1, event.relative_humidity);
  }

  // Write breath cycle counter
  Blynk.virtualWrite(V3, breathCyclesCompleted);
}

// Update LED strip for breathing effect
void led_strip_update() {
  unsigned long currentMillis = millis();
  elapsedTime = currentMillis - startTime;

  if (elapsedTime >= fadeDuration) {
    // Switch phase after the duration
    breathingIn = !breathingIn;
    startTime = currentMillis;
    elapsedTime = 0;

    if (breathingIn) {
      if (breathingOutPhaseEnded) {
        breathCyclesCompleted++;
        Serial.print("Breath cycles completed: ");
        Serial.println(breathCyclesCompleted);
        breathingOutPhaseEnded = false;
      }
    } else {
      breathingOutPhaseEnded = true;
    }
  }

  int activeLeds = map(elapsedTime, 0, fadeDuration, 0, LED_COUNT);

  if (breathingIn) {
    for (int i = 0; i < LED_COUNT; i++) {
      if (i < activeLeds) {
        leds[i].setFadeDirection(1); // Fade in
      } else {
        leds[i].setFadeDirection(-1); // Fade out
      }
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      if (i >= LED_COUNT - activeLeds) {
        leds[i].setFadeDirection(-1); // Fade out
      } else {
        leds[i].setFadeDirection(1); // Fade in
      }
    }
  }

  // Update all LEDs for the fade effect
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Update(strip);
  }
  delay(5); // Small delay to smooth out the animation
}

// Fade off LED strip
void led_strip_fade_off() {
  const int fadeOffDuration = 500;
  unsigned long startFade = millis();

  // Fade off LEDs
  while (millis() - startFade < fadeOffDuration) {
    float progress = float(millis() - startFade) / fadeOffDuration;
    int brightness = int((1 - progress) * maxBrightness);

    for (int i = 0; i < LED_COUNT; i++) {
      uint32_t color = strip.getPixelColor(i);
      uint8_t r = (color >> 16) & 0xFF;
      uint8_t g = (color >> 8) & 0xFF;
      uint8_t b = color & 0xFF;
      
      r = (r * brightness) / maxBrightness;
      g = (g * brightness) / maxBrightness;
      b = (b * brightness) / maxBrightness;
      
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
    delay(5);
  }

  // Ensure all LEDs are turned off at the end of the fade
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();

  // Reset the timer and LED state
  breathingOutPhaseEnded = false;

  // Reset each Flasher object
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].reset();
  }
}

// Send SMS using HTTP POST request
void send_sms() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure(); // Bypass SSL verification for simplicity

  http.begin(client, "https://hook.eu2.make.com/8yh4qcrmt86yohr7ou933u8i4ignap");
  http.addHeader("Content-Type", "application/json");

  // Split JSON payload into separate variables
  const char* phoneNumber = "054*******";
  const char* to = "Your ferind name";
  const char* from = "Your name";

  String payload = String("{\"phoneNumber\":\"") + phoneNumber + "\",\"to\":\"" + to + "\",\"from\":\"" + from + "\"}";

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}


// Play music using HTTP GET request
void play_music() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure(); // Bypass SSL verification for simplicity

  http.begin(client, "https://hook.eu2.make.com/3svsq1fbj3wlhe5qi1ke5incmxhcbh");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending GET: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// Stop music using HTTP GET request
void stop_music() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure(); // Bypass SSL verification for simplicity

  http.begin(client, "https://hook.eu2.make.com/2lj9i9d1aihv8v4i16sja9v1og11a");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending GET: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// Blynk button handler for breath training
BLYNK_WRITE(V2) {
  int pinValue = param.asInt();
  breath_train = pinValue;
  if (breath_train == 0) {
    led_strip_fade_off();
    breathCyclesCompleted = 0;
  } else {
    startTime = millis();
    breathingIn = true;
    elapsedTime = 0;
  }
  Serial.print("Breath pin: ");
  Serial.println(breath_train);
}

// Blynk button handler for diffusion
BLYNK_WRITE(V4) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    diffude = pinValue;
    last_defuse = millis();
    Serial.println("diffusing");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Update strip for breath train
  if (breath_train == 1) {
    led_strip_update();
  }

  // Servo up
  if (diffude == 1 && pos < 90) {
    pos += 2;
    myservo.write(pos);
  }

  // End of defuse
  if (diffude == 1 && currentMillis - last_defuse > defuseDuration) {
    diffude = 0;
  }

  // Servo down
  if (diffude == 0 && pos > 0) {
    pos -= 2;
    myservo.write(pos);
  }
  
  Blynk.run();
  timer.run();

  // Light sensor reader for box open or close
  int sensorValue = analogRead(LIGHT_SENSOR_PIN);
  if (sensorValue < lightSensorThreshold && !box_is_open && currentMillis - last_music_switch > musicSwitchDelay) {
    last_music_switch = currentMillis;
    box_is_open = true;
    Serial.println("Play music");
    Serial.print("light: ");
    Serial.println(sensorValue);
    play_music();
  }
  if (sensorValue > lightSensorThreshold && box_is_open && currentMillis - last_music_switch > musicSwitchDelay) {
    last_music_switch = currentMillis;
    box_is_open = false;
    Serial.println("Stop music");
    Serial.print("light: ");
    Serial.println(sensorValue);
    stop_music();
  }

  // Handle a click on emergency button
  int emergency_button = digitalRead(JOYSTICK_SW_PIN);
  if (emergency_button == 0 && currentMillis - last_sms > smsCooldown) {
    last_sms = currentMillis;
    Serial.println("emergency clicked");
    send_sms();
  }
}