#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            line_in;        //xy=392,368
AudioEffectChorus        chorus1;        //xy=547,451
AudioMixer4              mixer1;         //xy=713,385
AudioOutputI2S           line_out;       //xy=897,388
AudioConnection          patchCord1(line_in, 0, mixer1, 0);
AudioConnection          patchCord2(line_in, 0, chorus1, 0);
AudioConnection          patchCord3(chorus1, 0, mixer1, 1);
AudioConnection          patchCord4(mixer1, 0, line_out, 0);
AudioConnection          patchCord5(mixer1, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=560,230
// GUItool: end automatically generated code

// chorus delay line length
#define CHORUS_DELAY_LENGTH 16*AUDIO_BLOCK_SAMPLES
#define CHORUS_N 10

// create delay lines
short chorusDelayLine[CHORUS_DELAY_LENGTH];

#define DRY_CH 0
#define CHOR_CH 1

void setup() {
  AudioMemory(40);
  
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);  // minimum sensitivity/max voltage swing
  //sgtl5000_1.lineOutLevel(13);  // set output level to max
  
  sgtl5000_1.volume(0.3);

  chorus1.begin(chorusDelayLine, CHORUS_DELAY_LENGTH, 2);

  mixer1.gain(CHOR_CH, 0.5);
  mixer1.gain(DRY_CH, 0.5);
  mixer1.gain(2, 0.0);
  mixer1.gain(3, 0.0);

  Serial.begin(115200);

  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tw: wet/dry control, 0.0 to 1.0");
  Serial.println("\tv: headphone volume, 0.0 to 1.0");
  Serial.println("\tS: print out various stats");
  Serial.println("*********************************************************************************");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'w') {
      float W = Serial.parseFloat();
      float D = 1.0 - W;
      mixer1.gain(CHOR_CH, W);
      mixer1.gain(DRY_CH, D);
      Serial.print("Set wet/dry to: ");
      Serial.println(W);
    }
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
  }
}
