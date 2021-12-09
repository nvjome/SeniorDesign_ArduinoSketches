#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            line_in;           //xy=172,504
AudioFilterStateVariable filter1;        //xy=325,505
AudioEffectWaveshaper    waveshape1;     //xy=503,562
AudioMixer4              mixer1;         //xy=717,528
AudioOutputI2S           line_out;           //xy=1083,537
AudioConnection          patchCord1(line_in, 0, filter1, 0);
AudioConnection          patchCord2(filter1, 0, mixer1, 0);
AudioConnection          patchCord3(filter1, 0, waveshape1, 0);
AudioConnection          patchCord4(waveshape1, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, line_out, 0);
AudioConnection          patchCord6(mixer1, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=497,335
// GUItool: end automatically generated code

float WAVESHAPE_EXAMPLE[33] = {
  -0.588,
  -0.588,
  -0.588,
  -0.588,
  -0.588,
  -0.588,
  -0.588,
  -0.588,
  -0.588,
  -0.579,
  -0.549,
  -0.488,
  -0.396,
  -0.320,
  -0.228,
  -0.122,
  0,
  0.122,
  0.228,
  0.320,
  0.396,
  0.488,
  0.549,
  0.579,
  0.588,
  0.588,
  0.588,
  0.588,
  0.588,
  0.588,
  0.588,
  0.588,
  0.588
};

#define LINE_CH 0
#define VERB_CH 1

void setup() {
  AudioMemory(40);
  
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.volume(0.8);

  waveshape1.shape(WAVESHAPE_EXAMPLE, 33);

  mixer1.gain(VERB_CH, 0.5);
  mixer1.gain(LINE_CH, 0.5);

  filter1.frequency(400);
  filter1.resonance(0.707);

  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
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
      sgtl5000_1.volume(V);
      Serial.print("Set volume to: ");
      Serial.println(V);
    }
    if (cmd == 'f') {
      float F = Serial.parseFloat();
      filter1.frequency(F);
      Serial.print("Set cutoff freq to: ");
      Serial.println(F);
    }
    if (cmd == 'q') {
      float Q = Serial.parseFloat();
      filter1.resonance(Q);
      Serial.print("Set resonance to: ");
      Serial.println(Q);
    }
  }
}
