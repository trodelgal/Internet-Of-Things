#include "pitches.h"

// Pin Definitions
#define TONE_OUTPUT_PIN 26
#define LIGHT_SENSOR_PIN 33
#define TOUCH_SENSOR_PIN 34
#define CONDUCTIVE_SENSOR_PIN T0

// Thresholds
#define TOUCH_THRESHOLD 1
#define CONDUCTIVE_THRESHOLD 30

// Constants
const double LIGHT_RATE_DEVIDE_CONST = 2000.0;
const int TONE_PWM_CHANNEL = 0;
const int MELODY_LENGTH = 24;

// Melody for "Happy Birthday"
int happy_birthday_melody[] = {
  NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_G4, NOTE_FS4,
  NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_A4, NOTE_G4,
  NOTE_D4, NOTE_D4, NOTE_D5, NOTE_B4, NOTE_G4, NOTE_FS4,
  NOTE_D4, NOTE_D4, NOTE_D5, NOTE_B4, NOTE_A4, NOTE_G4
};

// Note durations for "Happy Birthday"
int happy_birthday_note_durations[] = {
  4, 4, 2, 4, 4, 2,
  4, 4, 2, 4, 4, 2,
  4, 4, 2, 4, 4, 2,
  4, 4, 2, 4, 4, 2
};

int next_tone = 0; // Next tone to play, given as a parameter from loop() to play_note(int i)

void play_note(int i) {
  /**
    play_note - Plays a musical note based on the given index.

    This function calculates the duration of the note dynamically based on the ambient light intensity.
    It reads the light sensor value (LDR) and adjusts the note duration accordingly.
    The melody is played using the specified melody array and note durations.
    After playing the note, the tone is turned off.

    Parameters:
    - @param i: The index of the note in the melody array (0 to 23).
  */

  // Calculate note duration based on light rate
  int light_rate = analogRead(LIGHT_SENSOR_PIN);
  double light_rate_const = light_rate / LIGHT_RATE_DEVIDE_CONST;
  double noteDuration = (1000.0 / happy_birthday_note_durations[i]) * light_rate_const;

  // Play the note
  ledcWriteTone(TONE_PWM_CHANNEL, happy_birthday_melody[i]);
  delay(noteDuration);
  ledcWrite(TONE_PWM_CHANNEL, 0); // Turn off the tone
}

void setup() {
  Serial.begin(115200);

  // Attach the tone output pin
  ledcAttachPin(TONE_OUTPUT_PIN, TONE_PWM_CHANNEL);
}

void loop() {
  int touch_read_val = analogRead(TOUCH_SENSOR_PIN);
  int conduct_read_val = touchRead(CONDUCTIVE_SENSOR_PIN);

  // Check if touch or conductive threshold is met
  if (touch_read_val > TOUCH_THRESHOLD || conduct_read_val < CONDUCTIVE_THRESHOLD) {
    play_note(next_tone);

    // Cycle through the melody
    if (next_tone < MELODY_LENGTH) {
      next_tone++;
    } else {
      next_tone = 0;
    }
  }
}