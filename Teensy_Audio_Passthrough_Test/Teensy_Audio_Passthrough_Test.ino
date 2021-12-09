#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            line_in;           //xy=388,500
AudioOutputI2S           line_out;           //xy=615,514
AudioConnection          patchCord1(line_in, 0, line_out, 0);
AudioConnection          patchCord2(line_in, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=497,335
// GUItool: end automatically generated code

const int myInput = AUDIO_INPUT_LINEIN;

void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.lineInLevel(0);
  //sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.volume(0.3);

  Serial.begin(115200);
  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tv: headphone volume, 0.0 to 1.0");
  Serial.println("\tf: filter cutoff");
  Serial.println("\tS: print out various stats");
  Serial.println("\tR: reset maximum stat values");
  Serial.println("*********************************************************************************");
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
