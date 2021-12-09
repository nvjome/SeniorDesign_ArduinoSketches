#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=346,315
AudioAmplifier           amp1;           //xy=583,313
AudioOutputI2S           i2s2;           //xy=794,296
AudioConnection          patchCord1(i2s1, 0, amp1, 0);
AudioConnection          patchCord2(amp1, 0, i2s2, 0);
AudioConnection          patchCord3(amp1, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=590,93
// GUItool: end automatically generated code

void setup() {
  AudioMemory(12);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);
  //sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.volume(0.3);

  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'v') {
      float V = Serial.parseFloat();
      sgtl5000_1.volume(V);
      Serial.print("Set volume to: ");
      Serial.println(V);
    }
    if (cmd == 'g') {
      float G = Serial.parseFloat();
      amp1.gain(G);
      Serial.print("Set gain to: ");
      Serial.println(G);
    }
  }
}
