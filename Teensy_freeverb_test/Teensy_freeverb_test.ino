#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            line_in;           //xy=172,504
AudioFilterStateVariable filter1;        //xy=325,505
AudioEffectFreeverb      freeverb1;      //xy=480,599
AudioMixer4              mixer1;         //xy=717,528
AudioOutputI2S           line_out;           //xy=1083,537
AudioConnection          patchCord1(line_in, 0, filter1, 0);
AudioConnection          patchCord2(filter1, 0, freeverb1, 0);
AudioConnection          patchCord3(filter1, 0, mixer1, 0);
AudioConnection          patchCord4(freeverb1, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, line_out, 0);
AudioConnection          patchCord6(mixer1, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=497,335
// GUItool: end automatically generated code

#define LINE_CH 0
#define VERB_CH 1

void setup() {
  AudioMemory(40);
  
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);  // minimum sensitivity/max voltage swing
  
  sgtl5000_1.volume(0.3);

  freeverb1.roomsize(0.5);
  freeverb1.damping(0.7);

  mixer1.gain(VERB_CH, 0.5);
  mixer1.gain(LINE_CH, 0.5);
  mixer1.gain(2, 0.0);
  mixer1.gain(3, 0.0);

  filter1.frequency(10000);
  filter1.resonance(0.707);

  Serial.begin(115200);
  delay(500);
  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tw: wet/dry control, 0.0 to 1.0");
  Serial.println("\tv: headphone volume, 0.0 to 1.0");
  Serial.println("\tr: freeverb room size");
  Serial.println("\td: freeverb damping");
  Serial.println("\tf: LPF cutoff");
  Serial.println("\tq: filter resonance");
  Serial.println("\tS: print out various stats");
  Serial.println("\tR: reset maximum stat values");
  Serial.println("*********************************************************************************");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'r') {
      float R = Serial.parseFloat();
      freeverb1.roomsize(R);
      Serial.print("Set roomsize to: ");
      Serial.println(R);
    }
    if (cmd == 'd') {
      float D = Serial.parseFloat();
      freeverb1.damping(D);
      Serial.print("Set damping to: ");
      Serial.println(D);
    }
    if (cmd == 'w') {
      float W = Serial.parseFloat();
      float D = 1.0 - W;
      mixer1.gain(VERB_CH, W);
      mixer1.gain(LINE_CH, D);
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
        filter1.frequency(F);
        Serial.print("Set cutoff freq to: ");
        Serial.println(F);
      }
    }
    if (cmd == 'q') {
      float Q = Serial.parseFloat();
      filter1.resonance(Q);
      Serial.print("Set resonance to: ");
      Serial.println(Q);
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
