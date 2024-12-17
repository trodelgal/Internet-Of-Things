#include <Adafruit_NeoPixel.h>

class Flasher {
    int ledIndex;
    int brightness;
    int fadeDirection;
    int fadeAmount;

  public:
    Flasher(int index) {
      ledIndex = index;
      brightness = 0;
      fadeDirection = 1; // 1 for fading in, -1 for fading out
      fadeAmount = 5; // Adjust the fade amount for smoother transitions
    }

    void Update(Adafruit_NeoPixel &strip) {
      brightness += fadeDirection * fadeAmount; // Change brightness gradually
      if (brightness > 255) {
        brightness = 255;
      } else if (brightness < 0) {
        brightness = 0;
      }
      // Set the color to pink (R: 255, G: 20, B: 147)
      strip.setPixelColor(ledIndex, strip.Color(brightness, (brightness * 20) / 255, (brightness * 147) / 255));
      strip.show();
    }

    void setFadeDirection(int direction) {
      fadeDirection = direction;
    }

    void reset() {
    // Reset internal state as needed
      brightness = 0;
      fadeDirection = 1; // 1 for fading in, -1 for fading out
      fadeAmount = 5; // Adjust the fade amount for smoother transitions
  }
};
