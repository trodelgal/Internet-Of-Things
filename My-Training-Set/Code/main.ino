// To define a board for the Arduino IDE, go to Tools > Board > Boards Manager and install the ESP32 board.
/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6LqdOkAuo"
#define BLYNK_TEMPLATE_NAME         "MyTrainingSet"
#define BLYNK_AUTH_TOKEN            "Nd0bxFWakYgdnQ-U69Lw0QRXqb_SlYMQ"
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
/* Sonar sensor*/
#define TRIGGER_PIN  16
#define ECHO_PIN     17
#define MAX_DISTANCE 100

/*Temp Sensors*/
#define DHTPIN 15
#define DHTTYPE    DHT22     // DHT 22 (AM2302)

/* Light sensor*/
#define LIGHT_SENSOR_PIN 33
// Pin Definitions
#define LED_PIN 18
#define LED_COUNT 12

#include <Adafruit_NeoPixel.h>    // Library for controlling NeoPixel LEDs
#include <WiFi.h>                 // Library for Wi-Fi
#include <WiFiClient.h>           // Library for Wi-Fi client
#include <BlynkSimpleEsp32.h>     // Library for Blynk
#include <Adafruit_Sensor.h>      // Base library for various sensors
#include <DHT.h>                  // Library for DHT sensor
#include <DHT_U.h>                // Unified sensor library for DHT

// Initialize DHT sensor
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS; // Delay between sensor readings

// Initialize NeoPixel strip
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Trudel's galaxi s24";
char pass[] = "xset5880";

// Blynk timer
BlynkTimer timer;

// Variables for pushup and drinking detection
int pushups = 10;
int pushupCounter = 0;
boolean isDown = false;
int pushupTrashold = 15;
int drinkCounter = 0;
int lightTrashold = 3500;
boolean isDrinking = false;
boolean trainingOn = false;
unsigned long prevMillisDrink;
unsigned long currMillisDrink;
unsigned long prevMillisPush;
unsigned long currMillisPush;
// color for led strip
int color = 0x00FF00;


// This function will be called every time Slider Widget
// in Blynk app writes values to the Virtual Pins
// Switch pin
BLYNK_WRITE(V3)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // Switch between On and Off states
  if(pinValue == 1 && trainingOn == false){
    trainingOn = true;
    drinkCounter = 0;
    pushupCounter = 0;
    // Turn on leds
    for (int ledIndex = 0; ledIndex < (pushups - pushupCounter); ledIndex++) {
      strip.setPixelColor(ledIndex, strip.ColorHSV(color << 8, 255, 255));
    }
    strip.show();
  }
  if(pinValue == 0 && trainingOn == true){
    trainingOn = false;
    drinkCounter = 0;
    pushupCounter = 0;
    // Turn off leds
    for (int ledIndex = 0; ledIndex < (pushups - pushupCounter); ledIndex++) {
      strip.setPixelColor(ledIndex, 0, 0, 0);
    }
    strip.show();
  }
}
// Pushups pin
BLYNK_WRITE(V4)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  pushups = pinValue;
  Serial.println(pinValue);
}


// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void myTimerEvent()
{
  // Read temperature from the DHT sensor
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  // Check if the temperature reading is valid
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Blynk.virtualWrite(V0, event.temperature);
  }

  // Read humidity from the DHT sensor
  dht.humidity().getEvent(&event);
  // Check if the humidity reading is valid
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Blynk.virtualWrite(V1, event.relative_humidity);
  }
  // Send the drink counter value to Blynk virtual pin V2
  // This value can be updated at any time, but avoid sending more than 10 values per second
  Blynk.virtualWrite(V2, drinkCounter);
}




void setup()
{
  // Debug console
  Serial.begin(115200);
  // Initialize Blynk with the authentication token and WiFi credentials
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
  // Initialize the DHT sensor
  dht.begin();
  // Initialize the sensor structures
  sensor_t sensor;
  prevMillisDrink = millis();
  prevMillisPush = millis();
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  strip.begin();
}

void loop()
{
  // Handle Blynk communication
  Blynk.run();
  // Check if training mode is on
  if(trainingOn == true){
    currMillisDrink = millis();
    int lightRate = analogRead(LIGHT_SENSOR_PIN);
    int cm = ultrasonic_measure(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
    // Check if the light sensor indicates drinking and enough time has passed
    if(lightRate < lightTrashold && isDrinking == false && currMillisDrink - prevMillisDrink >= 2000){
      isDrinking = true;
      drinkCounter++;
      prevMillisDrink = currMillisDrink;
    }
    // Reset drinking status when the light sensor indicates no drinking
    if(lightRate > lightTrashold && isDrinking == true){
      isDrinking = false;
    }
    currMillisPush = millis();
    // Check if the ultrasonic sensor indicates a pushup and enough time has passed
    if(cm < pushupTrashold && isDown == false && currMillisPush - prevMillisPush >= 4000){
      isDown = true;
      pushupCounter++;
      if(pushupCounter == pushups + 1){
        pushupCounter = pushups;
      }
      // Update the NeoPixel LED strip to indicate the pushup count
      strip.setPixelColor(pushups - pushupCounter, 0, 0, 0);
      strip.show();
      prevMillisPush = currMillisPush;
      Serial.println(pushups - pushupCounter);
    }
    // Reset pushup status when the ultrasonic sensor indicates no pushup
    if(cm > pushupTrashold && isDown == true){
      isDown = false;
    }
  }
  // Run the Blynk timer to handle timed events
  timer.run(); // Initiates BlynkTimer
}

// Distance function
int ultrasonic_measure(int trigPin, int echoPin, int max_distance) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);
  int duration = pulseIn(echoPin, HIGH, max_distance * 59);
  return duration / 59;
}