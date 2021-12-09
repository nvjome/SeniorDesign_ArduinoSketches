#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "effect_hardclip.h"
#include "effect_passthrough.h"

// GUItool: begin automatically generated code
AudioInputI2S            line_in;           //xy=172,504
AudioEffectHardclip      clip1;      //xy=480,599
AudioOutputI2S           line_out;           //xy=1083,537
AudioConnection          patchCord1(line_in, 0, clip1, 0);
AudioConnection          patchCord2(clip1, 0, line_out, 0);
AudioConnection          patchCord3(clip1, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=497,335
// GUItool: end automatically generated code


void setup() {
  AudioMemory(40);
  
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);  // minimum sensitivity/max voltage swing
  
  sgtl5000_1.volume(0.3);

  clip1.clipLevel(1.0);

  Serial.begin(115200);
  delay(500);
  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tv: headphone volume, 0.0 to 1.0");
  Serial.println("\tc: clip level, 0.0 to 1.0");
  Serial.println("\tS: print out various stats");
  Serial.println("\tR: reset maximum stat values");
  Serial.println("*********************************************************************************");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'c') {
      float C = Serial.parseFloat();
      clip1.clipLevel(C);
      Serial.print("Set clip level to: ");
      Serial.println(C);
    }
    if (cmd == 'v') {
      float V = Serial.parseFloat();
      if (V < 0.8) {
        sgtl5000_1.volume(V);
        Serial.print("Set volume to: ");
        Serial.println(V);
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
