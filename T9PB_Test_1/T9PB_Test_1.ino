//#define DEBUG_USB_INPUT
#include <T9_Pedal_Bundle.h>

void setup() {
  T9PB_begin();
  AudioMemory(64);

  // set bypass as default
  (*effectObjects_a[0]).connect();

  Serial.begin(115200);
  delay(500);  // wait for USB serial connection when using USB C adapter
  printCommandText();

  pinMode(13, OUTPUT);
}

void loop() {
  processSerialCommands();

  if ((T9PB_peak_detect(0) >= 0.98) || (T9PB_peak_detect(1) >= 0.98)) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }

  delay(10);
}

void printCommandText() {
  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tv: headphone volume, 0.0 to 0.8");
  Serial.println("\tP: activate passthrough effect");
  Serial.println("\tF: activate filter effect");
  Serial.println("\tf: LPF cutoff");
  Serial.println("\tB: activeate Freeverb effect");
  Serial.println("\tr: Freeverb roomsize");
  Serial.println("\td: Freeverb damping");
  Serial.println("\tw: Freeverb wet/dry");
  Serial.println("\tS: print out various stats");
  Serial.println("\tR: reset maximum stat values");
  Serial.println("\tQ: print peak sample");
  Serial.println("*********************************************************************************");
}

void processSerialCommands() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'v') {
      float V = Serial.parseFloat();
      if (V < 0.8) {
        T9PB_hp_volume(V);
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
    if (cmd == 'Q') {
      Serial.print("Pre: ");
      Serial.print(T9PB_peak_detect(0));
      Serial.print("\tPost: ");
      Serial.println(T9PB_peak_detect(1));
    }
    if (cmd == 'P') {
      // activate bypass (effect 0)
      (*effectObjects_a[1]).disconnect();
      (*effectObjects_a[2]).disconnect();
      (*effectObjects_a[0]).connect();
      Serial.println("Passthrough active");
    }
    if (cmd == 'F') {
      // activate LFP (effect 1)
      (*effectObjects_a[0]).disconnect();
      (*effectObjects_a[2]).disconnect();
      (*effectObjects_a[1]).connect();
      Serial.println("Filter active");
    }
    if (cmd == 'f') {
      float F = Serial.parseFloat();
      if (F < 15010) {
        (*effectObjects_a[1]).modParameter1(F);
        Serial.print("Set cutoff freq to: ");
        Serial.println(F);
      }
    }
    if (cmd == 'B') {
      // activate freeverb (effect 2)
      (*effectObjects_a[0]).disconnect();
      (*effectObjects_a[1]).disconnect();
      (*effectObjects_a[2]).connect();
      Serial.println("Freeverb active");
    }
    if (cmd == 'r') {
      float R = Serial.parseFloat();
      (*effectObjects_a[2]).modParameter1(R);
        Serial.print("Set roomsize to: ");
        Serial.println(R);
    }
    if (cmd == 'd') {
      float D = Serial.parseFloat();
      (*effectObjects_a[2]).modParameter2(D);
        Serial.print("Set damping to: ");
        Serial.println(D);
    }
    if (cmd == 'w') {
      float W = Serial.parseFloat();
      (*effectObjects_a[2]).modParameter3(W);
        Serial.print("Set wet/dry to: ");
        Serial.println(W);
    }
  }
}
