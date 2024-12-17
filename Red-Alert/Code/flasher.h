class Flasher {

    int color;
    long OnTime;
    long OffTime;
    int ledState;
    unsigned long previousMillis;

  public:

    Flasher(int col, long on, long off) {
      color = col;
      OnTime = on;
      OffTime = off;
      ledState = LOW;
      previousMillis = 0;
    }

    void Update() {
      unsigned long currentMillis = millis();
      if ((ledState == HIGH) && (currentMillis - previousMillis >= OnTime)) {
        ledState = LOW;
        previousMillis = currentMillis;
        for (int ledIndex = 0; ledIndex <= 11; ledIndex++) {
           strip.setPixelColor(ledIndex, 0, 0, 0);
        }
        strip.show();
      }
      else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime)) {
        ledState = HIGH;
        previousMillis = currentMillis;
        for (int ledIndex = 0; ledIndex <= 11; ledIndex++) {
          strip.setPixelColor(ledIndex, strip.ColorHSV(color << 8, 255, 255));
        }
        strip.show();
      }
    }

    void Turn_Off(){
      for (int ledIndex = 0; ledIndex <= 11; ledIndex++) {
          strip.setPixelColor(ledIndex, 0, 0, 0);
        }
        strip.show();
    }
};
