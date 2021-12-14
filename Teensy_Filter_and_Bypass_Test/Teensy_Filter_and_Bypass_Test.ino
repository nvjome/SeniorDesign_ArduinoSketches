/*
 * Teensy_Filter_and_Bypass_Test.ino
 * Nolan Jome
 * 
 * An example/test program to swithc between a LPF effect and a software
 * bypass block (a custom audio block). All functions are controlled via
 * commands isent over the serial port.
 * 
 * Both connections from the line input to the filter and passthrough block are defined,
 * along with the connections at the output, but both are disconnected before the audio
 * engine is started using AudioMemory().
 * 
 * The audio data is controlled using the connect and disconnect functions,
 * controlling with audio block receives the audio data for processing. The active
 * cord is disconnected before the other cord is connected (break before make),
 * as the behavior of having two cords connected to one input is likely undefined.
 */

#include <Audio.h>
#include "effect_passthrough.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            line_in;        //xy=392,368
AudioFilterStateVariable filter1;        //xy=636,398
AudioEffectPassthrough   bypass1;           //xy=639,361
AudioOutputI2S           line_out;       //xy=1001,399
AudioConnection          bypassInputCord(line_in, 0, bypass1, 0);
AudioConnection          filterInputCord(line_in, 0, filter1, 0);
AudioConnection          filterOutputCordL(filter1, 0, line_out, 0);
AudioConnection          filterOutputCordR(filter1, 0, line_out, 1);
AudioConnection          bypassOutputCordL(bypass1, 0, line_out, 0);
AudioConnection          bypassOutputCordR(bypass1, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=560,230
// GUItool: end automatically generated code

void setup() {
  // disconnect all non-static cords
  disconnectBypass();
  disconnectFilter();

  // enable audio engine
  AudioMemory(40);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);  // minimum sensitivity/max voltage swing
  //sgtl5000_1.lineOutLevel(13);  // set output level to max
  sgtl5000_1.volume(0.3);

  // default filter parameters
  filter1.frequency(1000);
  filter1.resonance(0.707);

  // connect filter by default
  connectFilter();

  Serial.begin(115200);
  delay(500);  // wait for USB serial connection when using USB C adapter
  printCommandText();
}

void loop() {
  processSerialCommands();
}

void printCommandText() {
  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tv: headphone volume, 0.0 to 0.8");
  Serial.println("\tf: LPF cutoff");
  Serial.println("\tP: activate passthrough effect");
  Serial.println("\tF: activate filter effect");
  Serial.println("\tS: print out various stats");
  Serial.println("\tR: reset maximum stat values");
  Serial.println("*********************************************************************************");
}

void processSerialCommands() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
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
    if (cmd == 'F') {
      disableAudioOutput();
      disconnectBypass();
      connectFilter();
      enableAudioOutput();
      Serial.println("Filter active");
    }
    if (cmd == 'P') {
      disableAudioOutput();
      disconnectFilter();
      connectBypass();
      enableAudioOutput();
      Serial.println("Passthrough active");
    }
  }
}

/*
 * Functions to control the audio output and the cords between the audio blocks.
 */

void disableAudioOutput() {
  sgtl5000_1.muteHeadphone();
  sgtl5000_1.muteLineout();
}

void enableAudioOutput() {
  sgtl5000_1.unmuteHeadphone();
  sgtl5000_1.unmuteLineout();
}

void connectFilter() {
  filterInputCord.connect();
  filterOutputCordL.connect();
  filterOutputCordR.connect();
}

void disconnectFilter() {
  filterInputCord.disconnect();
  filterOutputCordL.disconnect();
  filterOutputCordR.disconnect();
}

void connectBypass() {
  bypassInputCord.connect();
  bypassOutputCordL.connect();
  bypassOutputCordR.connect();
}

void disconnectBypass() {
  bypassInputCord.disconnect();
  bypassOutputCordL.disconnect();
  bypassOutputCordR.disconnect();
}
