#include <Audio.h>
#include "effect_passthrough.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            line_in;        //xy=392,368
AudioFilterBiquad        biquad1;        //xy=636,398
AudioMixer4              mixer1;         //xy=818,392
AudioOutputI2S           line_out;       //xy=1001,399
AudioConnection          patchCord1(line_in, 0, mixer1, 0);
AudioConnection          patchCord2(line_in, 0, biquad1, 0);
AudioConnection          patchCord3(biquad1, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, line_out, 0);
AudioConnection          patchCord6(mixer1, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=560,230
// GUItool: end automatically generated code

#define FILTER_CH 1
#define DRY_CH 0

void setup() {
  AudioMemory(40);
  
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);  // minimum sensitivity/max voltage swing
  //sgtl5000_1.lineOutLevel(13);  // set output level to max
  
  sgtl5000_1.volume(0.3);

  mixer1.gain(FILTER_CH, 0.5);
  mixer1.gain(DRY_CH, 0.5);
  mixer1.gain(2, 0.0);
  mixer1.gain(3, 0.0);

  biquad1.setLowpass(0, 1000, 0.707);

  Serial.begin(115200);
  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tv: headphone volume, 0.0 to 1.0");
  Serial.println("\tw: wet/dry control, 0.0 to 1.0");
  Serial.println("\tf: filter cutoff");
  Serial.println("\tS: print out various stats");
  Serial.println("\tR: reset maximum stat values");
  Serial.println("*********************************************************************************");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'w') {
      float W = Serial.parseFloat();
      float D = 1.0 - W;
      mixer1.gain(FILTER_CH, W);
      mixer1.gain(DRY_CH, D);
      Serial.print("Set wet/dry to: ");
      Serial.println(W);
    }
    if (cmd == 'v') {
      float V = Serial.parseFloat();
      if (V < 0.8) {
        sgtl5000_1.volume(V);
        Serial.print("Set volume to: ");
        Serial.println(V);
      }
    }
    if (cmd == 'f') {
      float F = Serial.parseFloat();
      if (F < 15010) {
        biquad1.setLowpass(0, F, 0.707);
        Serial.print("Set cutoff freq to: ");
        Serial.println(F);
      }
    }
    if (cmd == 'S') {
      Serial.print("Memory usage: ");
      Serial.println(AudioMemoryUsage());
      Serial.print("Max memory usage: ");
      Serial.println(AudioMemoryUsageMax());
      Serial.print("CPU usage: ");
      Serial.println(AudioProcessorUsage());
      Serial.print("Max CPU usage: ");
      Serial.println(AudioProcessorUsageMax());
    }
    if (cmd == 'R') {
      AudioMemoryUsageMaxReset();
      AudioProcessorUsageMaxReset();
      Serial.println("Maximum stat values reset");
    }
  }
}
