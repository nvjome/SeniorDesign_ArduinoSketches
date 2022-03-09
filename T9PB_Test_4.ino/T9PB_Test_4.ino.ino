/*
 * Basic program to test multiple effects.
 * Builds off of T9PB_Test_1, but now allows arbitrary access
 * to any effect and effect parameter.
 * Uses T9PB change effect function.
 * Uses T9PB effect and parameter name functions.
 */

#include <T9_Pedal_Bundle.h>
#include <string>

int currEffect = 0;
int prevEffect;

elapsedMillis peakCheckTime;

void setup() {
  T9PB_begin();
  AudioMemory(64);

  Serial.begin(115200);
  delay(500); // wait for USB serial connection when using USB C adapter
  print_command_text();
}

void loop() {
  process_serial_commands(&currEffect, &prevEffect);

  if (peakCheckTime >= 100) {
    peakCheckTime -= 100;
    if ((T9PB_peak_detect(0) >= 0.98) || (T9PB_peak_detect(1) >= 0.98)) {
      digitalWrite(13, HIGH);
    } else {
      digitalWrite(13, LOW);
    }
  }
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
      T9PB_change_effect(*currEffect_p, 0);
      *prevEffect_p = *currEffect_p;
      *currEffect_p = 0;
      Serial.println("Bypass active");
    }
    if (cmd == 'E') {
      // activate arbitrary effect
      int E = Serial.parseInt();
      int tmp = T9PB_change_effect(*currEffect_p, E);
      if (tmp < 0) {
        // not a valid effect
        Serial.println("ERROR: Invalid effect index");
      } else{
        // valid effect was activated
        *prevEffect_p = *currEffect_p;
        *currEffect_p = tmp;
        Serial.print(T9PB_get_effect_name(*currEffect_p).c_str());
        //Serial.print(effectObjects_a[*currEffect_p]->getEffectName().c_str());
        Serial.println(" active");
      }
    }
    if (cmd == 'a') {
      float p = Serial.parseFloat();
      T9PB_change_parameter(*currEffect_p, 1, p);
      //effectObjects_a[*currEffect_p]->modParameter1(p);
      Serial.print(T9PB_get_parameter_name(*currEffect_p, 1).c_str());
      //Serial.print(effectObjects_a[*currEffect_p]->getParameterName(1).c_str());
      Serial.print(": ");
      Serial.println(p);
    }
    if (cmd == 'b') {
      float p = Serial.parseFloat();
      T9PB_change_parameter(*currEffect_p, 2, p);
      //effectObjects_a[*currEffect_p]->modParameter2(p);
      Serial.print(T9PB_get_parameter_name(*currEffect_p, 2).c_str());
      //Serial.print(effectObjects_a[*currEffect_p]->getParameterName(2).c_str());
      Serial.print(": ");
      Serial.println(p);
    }
    if (cmd == 'c') {
      float p = Serial.parseFloat();
      T9PB_change_parameter(*currEffect_p, 3, p);
      //effectObjects_a[*currEffect_p]->modParameter3(p);
      Serial.print(T9PB_get_parameter_name(*currEffect_p, 3).c_str());
      //Serial.print(effectObjects_a[*currEffect_p]->getParameterName(3).c_str());
      Serial.print(": ");
      Serial.println(p);
    }
    if (cmd == 'P') {
      // print all available effects
      for (int i = 0; i <= NUM_EFFECTS; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(T9PB_get_effect_name(i).c_str());
      }
    }
    if (cmd == 'p') {
      // print active effect parameters
      for (int i = 1; i < 4; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(T9PB_get_parameter_name(*currEffect_p, i).c_str());
      }
    }
  }
}
