# Musical Instrument

## Description

We have created a **digital musical instrument** that merges physical components with digital control, designed for non-musicians who want to play music effortlessly. The instrument plays the iconic **"Happy Birthday"** melody and draws inspiration from classic electronic keyboards and synthesizers.

---

## Key Features

- **Melody Playback**: Plays the "Happy Birthday" song automatically when triggered.
- **Dynamic Note Adjustment**: Adjusts note duration based on ambient light intensity.
- **Interactive Operation**: Features both discrete and continuous triggers for enhanced user experience.

The system combines intuitive sensors with an **ESP32 microcontroller**, enabling dynamic and responsive sound generation.

---

## Components Used

| Component                            | Description                                                                   |
| ------------------------------------ | ----------------------------------------------------------------------------- |
| **ESP32 Microcontroller**            | The main controller responsible for generating tones and managing components. |
| **Light-Dependent Resistor** _(LDR)_ | Adjusts note duration dynamically based on ambient light intensity.           |
| **Touch Sensor**                     | Triggers melody playback interactively (discrete operation).                  |
| **Conductive Sensor**                | Enables continuous interaction using a cable and conductive cup.              |
| **Speaker**                          | Outputs sound via AUX connection.                                             |

---

## System Functionality

1. **Trigger Mechanisms**:
   - **Discrete Trigger**: Use the touch sensor to start the melody.
   - **Continuous Trigger**: Use the conductive sensor for real-time interaction.
2. **Dynamic Adjustment**:
   - The **LDR** adjusts note duration dynamically depending on the ambient light.
3. **Sound Production**:
   - The ESP32 generates musical tones sent to the speaker through an AUX adapter.

---

## Workflow Diagram

```mermaid
graph TD
    Start[Start System] --> Touch[Touch Sensor Trigger]
    Start --> Conductive[Conductive Sensor Trigger]
    Touch --> Melody[Play Melody: Happy Birthday]
    Conductive --> Melody
    Melody --> LDR[Adjust Note Duration (LDR)]
    LDR --> Speaker[Output Sound via Speaker]
    Speaker --> End[End]
```

---

## Demo Video

Watch our **Digi-Physi Musical Instrument** in action here:
[![Watch Video](https://img.youtube.com/vi/A05jaQ_IBR8/0.jpg)](https://www.youtube.com/watch?v=A05jaQ_IBR8)

---

## Code Snippet

Below is an example of generating tones using the ESP32 and controlling note duration with the LDR:

```cpp
ivoid loop() {
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
```

---

## Authors

- **Gal Trodel**
- **Guy Gal**

---

## Future Improvements

- Integrate multiple melodies for variety.
- Add a small display to show the current melody and light levels.
- Include Bluetooth control for wireless operation.

---

## Contact

For any questions or suggestions, reach out to:

- **Gal Trodel**: galtrudel@gmail.com

---
