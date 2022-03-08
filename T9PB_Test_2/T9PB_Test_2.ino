/*
 * Basic program to test multiple effects.
 * Builds off of T9PB_Test_1, but now allows arbitrary access
 * to any effect and effect parameter.
 */

#include <T9_Pedal_Bundle.h>
#include <string>

int currEffect = 0;
int prevEffect;

void setup() {
  T9PB_begin();
  AudioMemory(64);

  effectObjects_a[0]->connect();

  Serial.begin(115200);
  delay(500); // wait for USB serial connection when using USB C adapter
  print_command_text();
}

void loop() {
  process_serial_commands(&currEffect, &prevEffect);
}

int change_effect(int nextEffect, int nowEffect) {
  int ret = -1;
  // only make changes if effect is valid
  if (nextEffect >= 0 && nextEffect <= NUM_EFFECTS) {
    // disconnect current effect
    effectObjects_a[nowEffect]->disconnect();
    // connect new effect
    effectObjects_a[nextEffect]->connect();
    ret = nextEffect;
  }
  // return the active effect, or -1 for error
  return ret;
}

void print_command_text() {
  Serial.println("********************************************************************************");
  Serial.println("Command format: Ax");
  Serial.println("Where A is command letter and x is associated value if required");
  Serial.println("Valid commands:");
  Serial.println("\tV: Headphone volume");
  Serial.println("\t\tx: volume, 0.0 to 0.8");
  Serial.println("\tB: Activate software bypass");
  Serial.println("\tE: Activate effect x");
  Serial.print("\t\tx: effect, 1 to "); Serial.println(NUM_EFFECTS);
  Serial.println("\ta: Modify parameter 1 of active effect");
  Serial.println("\t\tx: parameter value");
  Serial.println("\tb: Modify parameter 2 of active effect");
  Serial.println("\t\tx: parameter value");
  Serial.println("\tc: Modify parameter 3 of active effect");
  Serial.println("\t\tx: parameter value");
  Serial.println("\tP: Print available effects");
  Serial.println("\tp: Print available parameters for current effect");
  Serial.println("\tS: Print out various stats");
  Serial.println("\tR: Reset stat values");
  Serial.println("*********************************************************************************");
}

void process_serial_commands(int* currEffect_p, int* prevEffect_p) {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'V') {
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
    if (cmd == 'B') {
      // activate bypass (effect 0)
      change_effect(0, *currEffect_p);
      *prevEffect_p = *currEffect_p;
      *currEffect_p = 0;
      Serial.println("Bypass active");
    }
    if (cmd == 'E') {
      // activate arbitrary effect
      int E = Serial.parseInt();
      int tmp = change_effect(E, *currEffect_p);
      if (tmp < 0) {
        // not a valid effect
        Serial.println("ERROR: Invalid effect index");
      } else{
        // valid effect was activated
        *prevEffect_p = *currEffect_p;
        *currEffect_p = tmp;
        Serial.print(effectObjects_a[*currEffect_p]->getEffectName().c_str());
        Serial.println(" active");
      }
    }
    if (cmd == 'a') {
      float p = Serial.parseFloat();
      effectObjects_a[*currEffect_p]->modParameter1(p);
      Serial.print(effectObjects_a[*currEffect_p]->getParameterName(1).c_str());
      Serial.print(": ");
      Serial.println(p);
    }
    if (cmd == 'b') {
      float p = Serial.parseFloat();
      effectObjects_a[*currEffect_p]->modParameter2(p);
      Serial.print(effectObjects_a[*currEffect_p]->getParameterName(2).c_str());
      Serial.print(": ");
      Serial.println(p);
    }
    if (cmd == 'c') {
      float p = Serial.parseFloat();
      effectObjects_a[*currEffect_p]->modParameter3(p);
      Serial.print(effectObjects_a[*currEffect_p]->getParameterName(3).c_str());
      Serial.print(": ");
      Serial.println(p);
    }
    if (cmd == 'P') {
      // print all available effects
      for (int i = 0; i <= NUM_EFFECTS; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(effectObjects_a[i]->getEffectName().c_str());
      }
    }
    if (cmd == 'p') {
      // print active effect parameters
      for (int i = 1; i < 4; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(effectObjects_a[*currEffect_p]->getParameterName(i).c_str());
      }
    }
  }
}
