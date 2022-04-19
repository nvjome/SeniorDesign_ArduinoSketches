/* T9 Effects Pedal Main Menu Test Code
 * 2/8/2022
 * Created by: Devan Metz
 * 
 * Description: This program is the basis for the T9 guitar pedal User Interface.
 * A 20x4 LCD, two rotary encoders, and two footswitch buttons are used to control
 * the T9 guitar pedal. This program utilizes these components to create a an easy
 * to use LCD menu. This menu allows the user to select different presets and edit the
 * parameters of the effects within the preset.
 * 
 * Connections:
 * Encoder A conneceted to pin 5,6
 * Encoder A button on pin 4
 * Encoder B conneceted to pin 2,3
 * Encoder B button on pin 1
 * 
 * Select Switch on pin 12
 * Toggle switch on pin 11
 * 
 * 20x4 LCD is conneceted to the second I2C port not used by the audio shield
 * SDA to pin 17
 * SCL to pin 16
 * 
 * TODO
 * 
 * Implement a memory bank of effect presets and parameter values(To work with Nolans code)
 * 
 * 
 * Revision History
 * V1  2/8/2022
 * Base menu system created
 * State machine implemented with 3 states
 * Main, Preset, and Effect menus
 * 
 * V1.1  2/10/2022
 * Implemented toggle switch functionality
 * Toggle switch allows you to select a "bank" of presets, and instantly swap between
 * them with a single fast press of the toggle switch
 * 
 * 
 * 
 * 
 * 
 */


#include "T9_Pedal_Bundle.h"
#include <LiquidCrystal_I2C.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#define ENCODER_USE_INTERRUPTS
#include <Encoder.h>
#include <string>
#include <EEPROM.h>

unsigned int EEPROMParameters = 0;


// GUItool: end automatically generated code
int currEffect = 0;
int prevEffect;
const int myInput = AUDIO_INPUT_LINEIN;

Encoder encoderA(5, 6); // Encoder A on pins 5,6
Encoder encoderB(2, 3); // Encoder B on pins 2,3
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Encoder variables
int encoderAPosition = 0; // Current value of encoder A constrained to different ranges depending on menu level
int oldPositionA = 0;
int encoderBPosition = 0; // Current value of encoder B used only for effect menu parameter B changes
int oldPositionB = 0;

int encoderAPin = 4; // Encoder A button connected to pin 4
boolean encoderAPress = false;
int encoderBPin = 1; // Encoder A button connected to pin 1
boolean encoderBPress = false;

// Footswitch variables
int buttonAPin = 12;
int buttonBPin = 11;
boolean buttonAPress = false;
boolean buttonBPress = false;

// Button press timing variables
const int shortPressDelay = 500;
int pressedTime = 0;
int releasedTime = 0;
int pressDelay = 0;

// Menu "State Machine" Variables
int menuScroll = 0; // Used for scrolling through the main menu
int oldMenuLevel = 0;
int menuLevel = 0; // Stores menu level 0=Main menu, 1=Preset menu, 2=Effect menu
boolean updated = false; // bool to determine when menu needs to be updated after button presses

// Effect and Preset state variables
int currentPreset = 0; // Stores current preset selection 0-9
int oldPreset = 0; // Store previous preset for menu updating functions
int currentEffect = 0; // Stores current effect selection 0-1
int parameterNum = 0;
int paramMax = 0;
int paramMin = 0;
int paramNum = 0;
int effectChange = 0;
int paramChange = 0;

// Name of each preset
String presetName[10] = {"Preset 0", "Preset 1", "Preset 2", "Preset 3", "Preset 4", "Preset 5", "Preset 6", "Preset 7", "Preset 8", "Preset 9"};

// Effects in order of design report effect table, skipping auto wah
// 0-Bypass, 1-LPF, 2-Reverb, 3-Tremelo, 4-Delay
int presetEffect[10] = {0,1,2,3,4,0,1,2,3,4};
byte presetParams[10][3] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int togglePresetIndex = 0;
int togglePresets[3] = {0, 1, 2};
// Variable to store what index in the toggle array is active
String currentEffectName;

void setup() {
  initUI(); // Initialize LCD, Audio adapter, Button interrupts
  initEEPROM();
}


// Main loop that runs state machine menu system
// Updates the menu when inputs are detected
// All logic not pertaining to input checking is contained within menuUpdate and button check functions
// Menu update function checks the current state and runs the correct menuDraw function
// Button check functions implement the logic for each buttons specfic controls
void loop() {
  // Encoder A position controlls most functions in the menu
  // Encoder A is the selection encoder, the position determines what menu item is selected
  // The variable is used in the footswitch button checks to step through the menu
  encoderAPosition = encoderA.read()/4; // Read encoderA position 4 counts per click
  encoderBPosition = encoderB.read()/4; // Read encoderB position 4 counts per click
  
  if(encoderAPosition != oldPositionA) { // If encoderA position change, update menu   
    
    oldPositionA = encoderAPosition; // Store position
    if(menuLevel == 2){
      paramNum = T9PB_get_parameter_num(currentEffect) - 1;
      if(encoderAPosition > paramNum){encoderA.write(paramNum*4); encoderAPosition = paramNum;} // If read position is greater than 2 set position to 0
      if(encoderAPosition < 0){encoderA.write(0); encoderAPosition = 0;}
      encoderBPosition = presetParams[currentPreset][encoderAPosition];
      encoderB.write(presetParams[currentPreset][encoderAPosition] * 4);
      }
    menuUpdate(); // Update Menu  
  } // End position update

  if((encoderBPosition != oldPositionB) && (menuLevel == 2)) { // If encoderB position change, update menu only for menulevel 2 (effect parameter changes)
    menuUpdate(); // Update Menu
    oldPositionB = encoderBPosition; // Store position
  } // End position update

  if(oldMenuLevel != menuLevel){ // If menu level is updated by a button press
    menuUpdate(); // Update the menu
    oldMenuLevel = menuLevel; // Update oldMenuLevel value
  }

  if(oldPreset != currentPreset){ // If current preset is updated by a button press
    menuUpdate(); // Update the menu
    oldPreset = currentPreset; // Update the oldPreset value 
  }

  if(digitalRead(encoderAPin) && encoderAPress){ // If the encoderA button is pressed then released
    updated = false; // Updated bool used to determine if full screen needs to be updated or just certain parts depending on menu level
    encoderButtonACheck(); // Determines long press or short press and updates state variables accordingly
  }
  if(digitalRead(encoderBPin) && encoderBPress){ // If the encoderA button is pressed then released
    updated = false; // Updated bool used to determine if full screen needs to be updated or just certain parts depending on menu level
    encoderButtonBCheck(); // Determines long press or short press and updates state variables accordingly
  }
  
  if(digitalRead(buttonAPin) & buttonAPress){ // If the select footswitch (buttonA) is pressed then released
    updated = false;
    buttonACheck(); // Determines long press or short press and updates the state variables according to the select button functionality
  }
  if(digitalRead(buttonBPin) & buttonBPress){ // If the toggle footswitch (buttonB) is pressed then released
    updated = false;
    buttonBCheck(); // Determines long press or short press and updates the state variables according to the toggle button functionality
  }


}

void initUI(){

  T9PB_begin();
  AudioMemory(500);
  

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T9 UI Test");

  // Setup audio passthrough
  //AudioMemory(40);
  //sgtl5000_1.enable();
  //sgtl5000_1.inputSelect(myInput);
  //sgtl5000_1.lineInLevel(0);
  //sgtl5000_1.lineOutLevel(13);
  //sgtl5000_1.volume(0.3);

  // Setup encoderA button interrupt
  pinMode(encoderAPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderAPin), encoderAISR, FALLING);

  // Setup encoderB button interrupt
  pinMode(encoderBPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderBPin), encoderBISR, FALLING);

  // Setup select footswitch interrupt (ButtonA)
  pinMode(buttonAPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonAPin), buttonAISR, FALLING);  

  // Setup toggle footswitch interrupt (ButtonB)
  pinMode(buttonBPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonBPin), buttonBISR, FALLING);
  
  // Initialize encoder positions
  encoderA.write(0);
  encoderB.write(0);
  
  Serial.begin(115200);

  // Wait a second to show UI test text, then update menu and enter main loop
  delay(500);
  menuUpdate();
}

void initEEPROM(){
  for(int i = 0; i<10; i++){
    for(int j = 0; j<3; j++){        
      presetParams[i][j] = EEPROM.read(i*3 + j);
    }
  }
}
void saveEEPROM(){
  for(int i = 0; i<10; i++){
    for(int j = 0; j<3; j++){
      EEPROM.write(i*3 + j, presetParams[i][j]);
    }
  }  
}

// Encoder A button logic contained within
// Encoder A button functions similar to the select footswitch
// Encoder A button allows you to step through all the menus
// Used to enter a preset, then select an effect, then effect parameters can be changed
// Short press: Select currently hovered menu object increases menu level
// Long press: Functions as a back or return button, decreases menu level
void encoderButtonACheck(){
  
  releasedTime = millis(); // Check released time
  pressDelay = releasedTime - pressedTime; // Compare to pressed time

  // If encoder button is pressed wait till unpressed and check time
  // If encoder is short pressed, step into next menu level
  // If encoder is long pressed, return to previous menu level
      
  if(pressDelay > shortPressDelay){menuLevel--;} // Long press = back/return button
    
  if((pressDelay <= shortPressDelay) && pressDelay > 100){ // Only count short presses between 100ms and 500ms
    menuLevel++; // Short press = select/step into button
      
    if(menuLevel == 1){ // If stepping into preset menu, remember current preset
      effectChange = T9PB_change_effect(currentEffect, presetEffect[encoderAPosition]);
      currentPreset = encoderAPosition;
      oldPreset = encoderAPosition;
      currentEffect = presetEffect[encoderAPosition];
    }
    if(menuLevel == 2){ // If stepping into effect menu, remember current effect
      
    }
    }
    
  if(menuLevel > 2){menuLevel = 2;} // Limit max menu level to 2
  if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0
    
  encoderAPress = false;
}

void encoderButtonBCheck(){
  
  releasedTime = millis(); // Check released time
  pressDelay = releasedTime - pressedTime; // Compare to pressed time

  // If encoder button is pressed wait till unpressed and check time
  // If encoder is short pressed, step into next menu level
  // If encoder is long pressed, return to previous menu level
      
  if(pressDelay > shortPressDelay){saveEEPROM(); Serial.print("saved");}

    
  encoderBPress = false;
}

// Select footswitch/buttonA logic contained within
// Select footswitch allows the user to scroll through the main menu and enable presets
// Short press: Scroll through main menu in one direction with wrap around
// Long press: select currently hovered preset, if already in a preset then returns to main menu
// allowing a new preset to be choosen
void buttonACheck(){
  
  releasedTime = millis(); // Check released time
  pressDelay = releasedTime - pressedTime; // Compare to pressed time

  // Long press logic    
  if(pressDelay > shortPressDelay){
    menuLevel++;
    if(menuLevel == 1){ // If stepping into preset menu, remember current preset
      currentPreset = encoderAPosition;
    }
    }

  // Short press logic
  if((pressDelay <= shortPressDelay) && pressDelay > 100){ // Only count short presses between 100ms and 500ms
    encoderAPosition++;
    encoderA.write(encoderAPosition*4);
  }

  // Menu level limits and wrapping
  if(menuLevel > 1){menuLevel = 0;} // Limit max menu level to 1 reset to 0
  if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0
  buttonAPress = false;
}


// Button B is the toggle button
// Short presses switch between the togglePreset[] list of presets
// Use the select button to hover over a preset you want to add to the list
// Long press toggle switch to add the preset to the end of the list
// Push index 0 of the list out
// Toggle presets are indicated by an index number on the right side of the screen
// in the main menu
void buttonBCheck(){
  
  releasedTime = millis(); // Check released time
  pressDelay = releasedTime - pressedTime; // Compare to pressed time

  // Long press logic
  if(pressDelay > shortPressDelay){
    if(menuLevel == 0){ // Only able to add presets to toggle list in main menu
      // Use current encoderAPosition/selected preset number and add to toggle list
      // Remove index 0, add selected preset to the end of the togglePresets array
      for(int i = 0; i < 2; i++){
        togglePresets[i] = togglePresets[i+1];
      }
      togglePresets[2] = encoderAPosition;
      menuUpdate();
    }
    }

  // Short press logic
  if((pressDelay <= shortPressDelay) && pressDelay > 100){ // Only count short presses between 100ms and 500ms


    // Set current preset to the toggle preset
    currentPreset = togglePresets[togglePresetIndex];
    // Increment toggle preset index
    togglePresetIndex++;
    // Limit toggle preset index 0-2
    if(togglePresetIndex > 2){togglePresetIndex = 0;}
    if(togglePresetIndex < 0){togglePresetIndex = 2;}    
    // Make sure we are in the preset menu state    
    menuLevel = 1;
    
  }
    
  if(menuLevel > 1){menuLevel = 0;} // Limit max menu level to 1 reset to 0
  if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0
  buttonBPress = false;
}

// Pass in current encoder position
void menuUpdate(){

  switch(menuLevel){ // Call correct menu update function based on menu level
    
      case 0: // Main Menu
        // In main menu selection is limited to 0-9, with wrap around
        if(encoderAPosition > 9){encoderA.write(0); encoderAPosition = 0;} // If read position is greater than 9 set position to 0
        if(encoderAPosition < 0){encoderA.write(36); encoderAPosition = 9;} // If read position is less than 0 set position to 9
        mainMenuDraw(encoderAPosition);
        break;
        
      case 1: // Preset Menu
        // In preset menu selection is limited to one effects 0
        if(encoderAPosition > 0){encoderA.write(0); encoderAPosition = 0;} // If read position is greater than 1 set position to 0
        if(encoderAPosition < 0){encoderA.write(0); encoderAPosition = 0;} // If read position is less than 0 set position to 1
        presetMenuDraw(encoderAPosition);
        
        // Insert effect wiring commands here
        
        break;
        
      case 2: // Effect Menu
        // In effect menu encoder positions directly change effect parameters 0-99
        //paramNum = T9PB_get_parameter_num(currentEffect) - 1;
        //if(encoderAPosition > paramNum){encoderA.write(paramNum*4); encoderAPosition = paramNum;} // If read position is greater than 2 set position to 0
        //if(encoderAPosition < 0){encoderA.write(0); encoderAPosition = 0;} // If read position is less than 0 set position to 0
        
        paramMax = T9PB_get_parameter_max(currentEffect, encoderAPosition+1);
        paramMin = T9PB_get_parameter_min(currentEffect, encoderAPosition+1);
        
        if(encoderBPosition > paramMax){encoderB.write(paramMax*4); encoderBPosition = paramMax;} // If read position is greater than 99 set position to 99
        if(encoderBPosition < paramMin){encoderB.write(paramMin*4); encoderBPosition = paramMin;} // If read position is less than 0 set position to 0       
        presetParams[currentPreset][encoderAPosition] = encoderBPosition;
        effectMenuDraw(encoderAPosition, encoderBPosition);
        
        // Insert effect parameter change commands here
        paramChange = T9PB_change_parameter(currentEffect, encoderAPosition + 1, encoderBPosition);
        break;
  }
  Serial.print("Current Effect: ");
  Serial.println(currentEffect, DEC);
  Serial.print("EncoderAPos: ");
  Serial.println(encoderAPosition, DEC);
  Serial.print("EncoderBPos: ");
  Serial.println(encoderBPosition, DEC); 
  Serial.print("ParamMin: ");
  Serial.println(paramMin, DEC);
  Serial.print("ParamMax: ");
  Serial.println(paramMax, DEC); 
  Serial.print("ParamNum: ");
  Serial.println(paramNum, DEC);   
  Serial.print("EffectChange: ");
  Serial.println(effectChange, DEC);   
  Serial.print("ParamChange: ");
  Serial.println(paramChange, DEC);   
 }



void mainMenuDraw(int x){ // Main menu drawing, x = selected preset (0-9)
  lcd.clear();
  menuScroll = x; // Use menuScroll variable to track preset scroll level
  // Can select 0-9 but, only scroll until preset 9 shows on bottom line
  if(menuScroll > 6){menuScroll = 6;}
  if(menuScroll < 0){menuScroll = 0;}
  // Menu scroll limits, at scroll = 0 preset 0 is on top line
  // at scroll = 6 preset 6 is on top line, 9 on bottom line

  // Loop 4 times to print a preset number and name on each line as shown
  // Indicate togglePresets list with index number on right side of preset name
  // *P1:Preset Name       1
  //  P2:Preset Name       2
  //  P3:Preset Name       3
  //  P4:Preset Name
  for(int i = 0; i <= 3; i++){
    lcd.setCursor(1,i);
    lcd.print("P");
    lcd.setCursor(2,i);
    lcd.print(menuScroll+i, DEC);
    lcd.setCursor(3,i);
    lcd.print(":");
    lcd.setCursor(4,i);
    // Use menu scroll int to increment preset name index
    lcd.print(presetName[menuScroll+i]);
    // Check togglePresets array to see if one of the currently displayed
    // presets are in the list, if they are, print their index number
    for(int j = 0; j < 3; j++){
      if(togglePresets[j] == menuScroll+i){
        lcd.setCursor(19, i);
        lcd.print(j, DEC);
      }
    }
  }

  // Use * to indicate hovered/selected preset
  if(x == menuScroll){ // When selected preset = scroll level, top menu item is selected
    lcd.setCursor(0,0);
    lcd.print("*");
  }
  if(x > menuScroll){ // When selected preset is greater than scroll level, move * down for P7-P9
    lcd.setCursor(0, x-menuScroll);
    lcd.print("*");
  }
}

// Menu Level 1, draw preset name on top line
// List effects within preset
// Preset: Preset Name
// *Chorus
//  Delay
// Use * to indicated hovered/selected effect
void presetMenuDraw(int x){ // x is current encoder postion limited to 0-1
  //Clear LCD and print preset name at top

  if(!updated){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Preset:"); 
    lcd.setCursor(8,0);
    lcd.print(presetName[currentPreset]);
    
    for(int i = 0; i < 1; i++){
      lcd.setCursor(1,i + 1);
      lcd.print(T9PB_get_effect_name(presetEffect[currentPreset]).c_str());  
    }
    updated = true;
  }
  // Print selection *
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.setCursor(0, 2);
  lcd.print(" ");
  lcd.setCursor(0, x+1);
  lcd.print("*");


}

// Menu Level 2, draw effect name on top line
// List parameters within effect
// Preset: Preset Name
// Effect: Effect Name
// Parameter 1 = Encoder A Rotation
// Parameter 2 = Encoder B Rotation
void effectMenuDraw(int x, int y){ // x is encoder A values 0-2 y is encoder B values paramMin-paramMax
  
  menuScroll = x; // Use menuScroll variable to track preset scroll level

  if(x<2){menuScroll = 0;}else{menuScroll=x-2;}
  
  
  // if !updated code runs once per button press
  // allows static text to be printed to LCD only once
  // checks the number of parameters within the effect once
  if(!updated){ 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Effect:");
    lcd.setCursor(8,0);
    lcd.print(T9PB_get_effect_name(presetEffect[currentPreset]).c_str()); // Print effect name
    parameterNum = T9PB_get_parameter_num(presetEffect[currentPreset]);
    updated = true;
  }

  for(int i = 1; i < parameterNum+1; i++){
    lcd.setCursor(0,menuScroll + i);
    lcd.print(" ");
    lcd.setCursor(1,menuScroll + i);
    lcd.print(T9PB_get_parameter_name(presetEffect[currentPreset],i).c_str());
    lcd.setCursor(12,menuScroll + i);
    lcd.print(":");
    lcd.setCursor(13,menuScroll + i);
    lcd.print("    ");
    lcd.setCursor(13,menuScroll + i);
    if(currentEffect == 1){lcd.print((presetParams[currentPreset][i-1])*10);}
    else{lcd.print(presetParams[currentPreset][i-1]);}
    // Check togglePresets array to see if one of the currently displayed
    // presets are in the list, if they are, print their index number
    }
  
  // Use * to indicate hovered/selected preset
  lcd.setCursor(0,x+1);
  lcd.print("*");


  
}  

/*  for(int i = 0; i < 2; i++){
    lcd.setCursor(0, i+2);
    lcd.print(effectNames[currentEffects[currentEffect]][i+1]);
    lcd.setCursor(16, i+2);
    lcd.print("  ");
    lcd.setCursor(16, i+2);
    if(!i){lcd.print(encoderAPosition);} // For i = 0 print encoder A position 0-99
    if(i){lcd.print(encoderBPosition);} // For i = 1 print encoder B position 0-99
  }  */
  



// Encoder A Button ISR
void encoderAISR() {
  encoderAPress = true;
  pressedTime = millis();
}

// Encoder B Button ISR
void encoderBISR() {
  encoderBPress = true;
  pressedTime = millis();
}

void buttonAISR() {
  buttonAPress = true;
  pressedTime = millis();
}
void buttonBISR() {
  buttonBPress = true;
  pressedTime = millis();
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
