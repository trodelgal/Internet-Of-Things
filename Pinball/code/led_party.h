// Led_Party class for managing LED effects and animations for cheat mode

class Led_Party {

    long OnTime;
    long OffTime;
    int state;
    int index;
    int color;
    unsigned long previousMillis;

  public:

    Led_Party(int ledIndex, int ledColor, long on, long off) {
      index = ledIndex;
      color = ledColor;
      OnTime = on;
      OffTime = off;
      state = LOW;
      previousMillis = 0;
    }

    void Update() {
      unsigned long currentMillis = millis();
      if ((state == HIGH) && (currentMillis - previousMillis >= OnTime)) {
        state = LOW;
        previousMillis = currentMillis;
        strip.setPixelColor(index, 0, 0, 0);
        strip.show();
      }
      else if ((state == LOW) && (currentMillis - previousMillis >= OffTime)) {
        state = HIGH;
        previousMillis = currentMillis;
        strip.setPixelColor(index, strip.ColorHSV(color << 8, 255, 255));
        strip.show();
      }
    }

    void Turn_Off(){
        strip.setPixelColor(index, 0, 0, 0);
        strip.show();
    }
};
