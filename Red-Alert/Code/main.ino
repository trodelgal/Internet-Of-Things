#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <Adafruit_NeoPixel.h>

// Pin Definitions
#define LED_PIN 15
#define LED_COUNT 12
#define TONE_OUTPUT_PIN 26

const int TONE_PWM_CHANNEL = 0;
const int ALERET_LENGTH_IN_MILIS = 10000; // 10 seconds

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include "flasher.h"
#include "space_beeper.h"

Flasher leds(0xFF0000, 100, 100);
SpaceBeeper beeper1(5, 100, 6);

// WiFi and API credentials
const char* ssid = "Eitam_A";
const char* password = "04012017";
const char* googleApiKey = "AIzaSyAjN_6yqMn7kjQLziey9ORose_FtE0HOl4";

// API endpoints
const char* redAlertHost = "https://www.oref.org.il/WarningMessages/History/AlertsHistory.json";
const char* googleGeoLocation = "https://www.googleapis.com/geolocation/v1/geolocate?key=";
const char* googleMapsCityName = "https://maps.googleapis.com/maps/api/geocode/json?latlng=";
const char* orefUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.0.0 Safari/537.36";

// Global variables for location and city name
float latitude = 0.0;
float longitude = 0.0;
String city;
String cookies = "";


/**
 * @brief Fetches the current geolocation using the Google Geolocation API.
 * 
 * This function sends a POST request to the Google Geolocation API with the 
 * specified API key. The response is parsed to extract the latitude and 
 * longitude, which are stored in global variables.
 */
void getLocation() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = googleGeoLocation + String(googleApiKey);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"considerIp\": \"true\"}";
    int httpResponseCode = http.POST(body);

    if (httpResponseCode > 0) {
      String response = http.getString();
      if (httpResponseCode == 200){
        Serial.println("Successfully got Google Location");
      }

      // Parse JSON response
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      // Extract latitude and longitude
      latitude = doc["location"]["lat"];
      longitude = doc["location"]["lng"];

      Serial.print("Latitude: ");
      Serial.println(latitude, 6);  // Print latitude with 6 decimal places
      Serial.print("Longitude: ");
      Serial.println(longitude, 6);  // Print longitude with 6 decimal places
      Serial.println("=================");
    } else {
      Serial.println("Error on HTTP request: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}


/**
 * @brief Fetches the city name based on latitude and longitude using the Google Maps Geocoding API.
 * 
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @return String The name of the city corresponding to the provided latitude and longitude.
 */
String getLocationName(float latitude, float longitude) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = googleMapsCityName + String(latitude, 6) + "," + String(longitude, 6) + "&key=" + String(googleApiKey) + "&language=he";

    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      http.end();

      // Find the start index of the city name
      int formatted_addressIdx = response.indexOf("\"compound_code\" : ") + 23;
      int startIdx = response.indexOf(" ", formatted_addressIdx) + 1;

      // Find the end index of the city name
      int endIdx = response.indexOf(",", startIdx);

      // Extract the city name
      String cityName = response.substring(startIdx, endIdx);

      Serial.println("City name: " + cityName);
      Serial.println("=================");
      return cityName;
    } else {
      Serial.println("Error on HTTP request: " + String(httpResponseCode));
      return "";
    }
  } else {
    Serial.println("WiFi not connected");
    return "";
  }
}


/**
 * @brief Retrieves cookies required for the Red Alert request.
 * 
 * This function sends a GET request to the main page of the Red Alert service to
 * retrieve the necessary cookies for subsequent requests.
 */
void getCookies() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://www.oref.org.il/");
    http.addHeader("User-Agent", orefUserAgent);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      cookies = http.header("Set-Cookie");
      //Serial.println("Cookies received: " + cookies);
    } else {
      Serial.println("Error getting cookies: " + String(httpResponseCode));
    }

    http.end();
  }
}


/**
 * @brief Checks if the alert date is within the last minute from the current time.
 * 
 * @param alertDate The date and time of the alert in the format "YYYY-MM-DD HH:MM:SS".
 * @return true if the alert date is within the last minute, false otherwise.
 */
bool isWithinLastMinute(const String& alertDate) {
  // Parse the alert date string
  int year, month, day, hour, minute, second;
  sscanf(alertDate.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);

  // Get the current time
  time_t now = time(nullptr);
  struct tm* currentTime = localtime(&now);

  // Compare the alert date with the current time
  if (currentTime->tm_year + 1900 == year &&
      currentTime->tm_mon + 1 == month &&
      currentTime->tm_mday == day &&
      currentTime->tm_hour == hour &&
      currentTime->tm_min == minute &&
      abs(currentTime->tm_sec - second) <= 60) {  // Within the last minute
    return true;
  }
  return false;
}


/**
 * @brief Fetches Red Alerts from the server and checks if there's an alert for the specified city.
 * 
 * @param city The name of the city to check for alerts.
 * @return true if there's a recent alert for the specified city, false otherwise.
 */
bool getRedAlerts(String city) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(redAlertHost);

    // Add necessary headers
    http.addHeader("Host", "www.oref.org.il");
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("charset", "utf-8");
    http.addHeader("X-Requested-With", "XMLHttpRequest");
    http.addHeader("sec-ch-ua-mobile", "?0");
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.0.0 Safari/537.36");
    http.addHeader("sec-ch-ua-platform", "macOS");
    http.addHeader("Accept", "*/*");
    http.addHeader("sec-ch-ua", "\".Not/A)Brand\";v=\"99\", \"Google Chrome\";v=\"103\", \"Chromium\";v=\"103\"");
    http.addHeader("Sec-Fetch-Site", "same-origin");
    http.addHeader("Sec-Fetch-Mode", "cors");
    http.addHeader("Sec-Fetch-Dest", "empty");
    http.addHeader("Referer", "https://www.oref.org.il/12481-he/Pakar.aspx");
    http.addHeader("Accept-Encoding", "identity");  // Disable gzip compression
    http.addHeader("Accept-Language", "en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7");

    if (cookies.length() > 0) {
      http.addHeader("Cookie", cookies);
    }

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();

      if (httpResponseCode == 200){
        Serial.println("RedAlert update recived");
      }
      else{
        Serial.println("Alert HTTP Response code: " + String(httpResponseCode));  
      }

      // Check if response is empty
      if (response.indexOf("{") == -1) {
        Serial.println("No alert for now");
        return false;
      }

      // Parse JSON response
      DynamicJsonDocument doc(16384);
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return false;
      }

      // Check if the response is an array
      if (doc.is<JsonArray>()) {
        // Iterate through the array of alerts
        for (JsonVariant alert : doc.as<JsonArray>()) {
          // Check if the "data" field of the current alert is "תקומה"
          if (alert["data"] == city && isWithinLastMinute(alert)) {
            Serial.println("ALERT IN YOUR AREA: " + city);
            return true;
          } 
        }
        // No alert in your area:
        Serial.println("No alerts for " + city);
        
      } else {
        Serial.println("Invalid JSON format: Expected array");
      }
      doc.clear();
    } else {
      Serial.println("Error on HTTP request: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
  
  return false;
}


void setup() {
  delay(20000);
  Serial.begin(115200);
  Serial.println("WIFI starting");
  WiFi.begin(ssid, password);

  strip.begin();
  strip.show();
  strip.setBrightness(150);
  ledcAttachPin(TONE_OUTPUT_PIN, TONE_PWM_CHANNEL);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("=================");

  getCookies();
  getLocation();
  city = getLocationName(latitude, longitude);
}

void loop() {
  bool alert = getRedAlerts(city);
  
  if (alert){
    unsigned long startMillis = millis();
    unsigned long currentMillis = millis();
    while ((currentMillis - startMillis < ALERET_LENGTH_IN_MILIS)){
      leds.Update();
      beeper1.Update();
      currentMillis = millis();
    }
    leds.Turn_Off();
    beeper1.Turn_off();
  }
  delay(1000); // Check every second
}