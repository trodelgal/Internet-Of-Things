class SpaceBeeper {
  
    int onDuration;
    int offDuration;
    int noteCount;

    int noteIndex;
    unsigned long previousMillis;
    boolean soundIsOn;

  public:

  SpaceBeeper(int on, int off, int notes) {
    onDuration = on;
    offDuration = off;
    noteCount = notes;
    
    noteIndex = 0;
    previousMillis = 0;
    soundIsOn = false;
  }

  void Update() {
    unsigned long currentMillis = millis();
    if (!soundIsOn && (currentMillis - previousMillis) >= (offDuration)) {
      ledcWriteTone(TONE_PWM_CHANNEL, 2000 + 500 * noteIndex);
      noteIndex = noteIndex + 1;
      if(noteIndex > noteCount) { 
        noteIndex = 0;
      }
      soundIsOn = true;
      previousMillis = millis();
    }
    else if(soundIsOn && (currentMillis - previousMillis) >= (onDuration)) {
      ledcWriteTone(TONE_PWM_CHANNEL, 0);
      soundIsOn = false;
      previousMillis = millis();
    }
  }

  void Turn_off() {
    noteIndex = 0;
  }

};
