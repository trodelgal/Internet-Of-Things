// Button class for handling touch sensor input and corresponding keypress actions

class Button {

    int pinIndex;
    int threshold;
    int button;
    int state;

  public:

    Button(int pin, int condThreshold, int bot) {
      pinIndex = pin;
      threshold = condThreshold;
      button = bot;
      state = 0;
    }

    void Update() {
      int touch = touchRead(pinIndex);
      if (touch < threshold && state == 0){
        Keyboard.press(button);
        state = 1;
      }
      if (touch > threshold && state == 1){
        Keyboard.release(button);
        state = 0;
      }
    }
};
