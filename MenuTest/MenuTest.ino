/* T9 Effects Pedal Main Menu Code
 * 5/9/2022
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
#include <Bounce.h>

elapsedMillis peakCheckTime;
#define IN_PEAK_LED 14
#define OUT_PEAK_LED 22

// Audio library variables
int currEffect = 0;
int prevEffect = 0;
const int myInput = AUDIO_INPUT_LINEIN;

//Encoder pin assignments
Encoder encoderA(5, 6);
Encoder encoderB(2, 3);

// Encoder variables
int encoderAPosition = 0; // Current value of encoder A constrained to different ranges depending on menu level
int oldPositionA = 0;
int encoderBPosition = 0; // Current value of encoder B used only for effect menu parameter B changes
int oldPositionB = 0;

// Button pin assignments
int encoderAPin = 4;
int encoderBPin = 1;
int buttonAPin = 12;
int buttonBPin = 11;
// Button press timing variables
const int longPressDelay = 400;
const int debounceTime = 10;
elapsedMillis pressedTime;
// Bounce library setup
Bounce encoderAButton = Bounce(encoderAPin, debounceTime);
Bounce encoderBButton = Bounce(encoderBPin, debounceTime);
Bounce buttonA = Bounce(buttonAPin, 100);
Bounce buttonB = Bounce(buttonBPin, 100);
byte encoderAPress = HIGH;
byte encoderBPress = HIGH;
byte buttonBPress = HIGH;
byte buttonAPress = HIGH;

// LCD Pin and I2C address assignment
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Menu "State Machine" Variables
int menuScroll = 0; // Used for scrolling through the main menu
int oldMenuLevel = 0;
int menuLevel = 0; // Stores menu level 0=Main menu, 1=Preset menu, 2=Effect menu, 3=Settings
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
String presetName[11] = {"Preset 0", "Preset 1", "Preset 2", "Preset 3", "Preset 4", "Preset 5", "Preset 6", "Preset 7", "Preset 8", "Preset 9","Settings"};

// 0-Bypass, 1-LPF, 2-Reverb, 3-Tremelo, 4-Delay
int presetEffect[10] = {0,1,2,3,4,0,1,2,3,4};
int presetParams[10][3] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};

// Toggle list variables
int togglePresetIndex = 0;
int togglePresets[3] = {0, 1, 2};

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
  
  if(encoderAPosition != oldPositionA) { // If encoderA position change, update menu, update knob value constraints based on number of parameters in effect
    oldPositionA = encoderAPosition; // Store position
    if(menuLevel == 2){ // If entering the parameter menu limit encoder A values to number of parameters
      paramNum = T9PB_get_parameter_num(currentEffect) - 1;
      if(encoderAPosition > paramNum){
        encoderA.write(0);
        encoderAPosition = 0;
      } 
      // If read position is greater than 2 set position to 0
      if(encoderAPosition < 0){
        encoderA.write(paramNum * 4);
        encoderAPosition = paramNum;
      }
      encoderBPosition = presetParams[currentPreset][encoderAPosition]; // If in parameter menu, update encoderB position to selected parameter value
      encoderB.write(presetParams[currentPreset][encoderAPosition] * 4);
    }

    // If in settings menu limit encoder A position then set encoder B position to current preset effect
    if(menuLevel == 3){
      if(encoderAPosition > 9){encoderA.write(0); encoderAPosition = 0;} // If read position is greater than 9 set position to 0
      if(encoderAPosition < 0){encoderA.write(36); encoderAPosition = 9;} // If read position is less than 0 set position to 9
      encoderBPosition = presetEffect[encoderAPosition]; // If in parameter menu, update encoderB position to selected parameter value
      encoderB.write(presetEffect[encoderAPosition] * 4);        
    }  
    menuUpdate(); // Update Menu  
  } // End position update

  if((encoderBPosition != oldPositionB) && (menuLevel == 2)) { // If encoderB position change, update menu only for menulevel 2 (effect parameter changes)
    menuUpdate(); // Update Menu
    oldPositionB = encoderBPosition; // Store position
  } // End position update

  if((encoderBPosition != oldPositionB) && (menuLevel == 3)) { // If encoderB position change, update menu only for menulevel 3 (effect parameter changes)
    
    presetEffect[encoderAPosition] = encoderBPosition;
    oldPositionB = encoderBPosition; // Store position
    for(int i = 0; i<3; i++){
    presetParams[encoderAPosition][i] = 10;  
    }
    menuUpdate(); // Update Menu
  } // End position update

  if(oldMenuLevel != menuLevel){ // If menu level is updated by a button press
    menuUpdate(); // Update the menu
    oldMenuLevel = menuLevel; // Update oldMenuLevel value
    encoderBPosition = presetParams[currentPreset][encoderAPosition];
    encoderB.write(presetParams[currentPreset][encoderAPosition] * 4);
  }

  if(oldPreset != currentPreset){ // If current preset is updated by a button press
    // menuUpdate(); // Update the menu
    oldPreset = currentPreset; // Update the oldPreset value 
    currentEffect = presetEffect[currentPreset];
    effectChange = T9PB_change_effect(currentEffect, presetEffect[currentPreset]);
    for(int i = 0; i< T9PB_get_parameter_num(currentEffect); i++){
    paramChange = T9PB_change_parameter(presetEffect[currentPreset], i, presetParams[currentPreset][i]);  
    }
    menuUpdate();
  }

  if(encoderAButton.update()){
    if(encoderAButton.fallingEdge()){
      pressedTime = 0;
    }
    if(encoderAButton.risingEdge()){
      updated = false; // Updated bool used to determine if full screen needs to be updated or just certain parts depending on menu level
      encoderButtonACheck(); // Determines long press or short press and updates state variables accordingly   
    }
  }  
  if(encoderBButton.update()){
    if(encoderBButton.fallingEdge()){
      pressedTime = 0;
    }
    if(encoderBButton.risingEdge()){
      updated = false; // Updated bool used to determine if full screen needs to be updated or just certain parts depending on menu level
      encoderButtonBCheck(); // Determines long press or short press and updates state variables accordingly   
    }
  }
  
  if(buttonA.update()){
    if(buttonA.fallingEdge()){
      pressedTime = 0;
    }
    if(buttonA.risingEdge()){
      updated = false; // Updated bool used to determine if full screen needs to be updated or just certain parts depending on menu level
      buttonACheck(); // Determines long press or short press and updates state variables accordingly   
    }
  }
  
  if(buttonB.update()){
    if(buttonB.fallingEdge()){
      pressedTime = 0;
    }
    if(buttonB.risingEdge()){
      updated = false; // Updated bool used to determine if full screen needs to be updated or just certain parts depending on menu level
      buttonBCheck(); // Determines long press or short press and updates state variables accordingly   
    }
  }

    if (peakCheckTime >= 100) {
    peakCheckTime -= 100;
    if (T9PB_peak_detect(0) >= 0.98) {
      digitalWrite(IN_PEAK_LED, HIGH);
    } else {
      digitalWrite(IN_PEAK_LED, LOW);
    }
    if (T9PB_peak_detect(1) >= 0.98) {
      digitalWrite(OUT_PEAK_LED, HIGH);
    } else {
      digitalWrite(OUT_PEAK_LED, LOW);
    }
  }
} // end loop

void initUI(){

  pinMode(IN_PEAK_LED, OUTPUT);
  pinMode(OUT_PEAK_LED, OUTPUT);
  digitalWrite(IN_PEAK_LED, HIGH);
  digitalWrite(OUT_PEAK_LED, HIGH);

  T9PB_begin();
  AudioMemory(500);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(5,1);
  lcd.print("T9 UI Test");

  pinMode(encoderAPin, INPUT_PULLUP);
  pinMode(encoderBPin, INPUT_PULLUP);
  pinMode(buttonAPin, INPUT_PULLUP);
  pinMode(buttonBPin, INPUT_PULLUP);

  // Initialize encoder positions
  encoderA.write(0);
  encoderB.write(0);
  
  Serial.begin(115200);

  // Wait a second to show UI test text, then update menu and enter main loop
  delay(500);
  menuUpdate();
}

void initEEPROM(){ // EEPROM loading, run at startup to initialize effect and parameter arrays
  for(int i = 0; i<10; i++){ // Loop through all presets
    for(int j = 0; j<3; j++){// Loop through max 3 parameters per effect
      byte paramByteA = EEPROM.read(i*6 + j*2); // Read first byte of parameter value
      byte paramByteB = EEPROM.read(i*6 + j*2 + 1); // read second byte of parameter value
      presetParams[i][j] = paramByteA + (paramByteB * 256); // Store parameter value into list
    }
  }
  for(int i = 0; i<10; i++){ // Preset effect loading
    presetEffect[i] = EEPROM.read(70 + i); // Store preset effects into array
  }
}

void saveEEPROM(){
  
  for(int i = 0; i<10; i++){ // Effect Parameter Saving
    for(int j = 0; j<3; j++){
      int paramVal = presetParams[i][j];
      byte paramByteA = paramVal % 256; // Convert parameter value into
      byte paramByteB = paramVal / 256;
      EEPROM.write(i*6 + j*2, paramByteA);
      EEPROM.write(i*6 + j*2 + 1, paramByteB);
    }
  }  
  for(int i = 0; i<10; i++){
    EEPROM.write(70 + i, presetEffect[i]);  
  }
}

// Encoder A button logic contained within
// Encoder A button functions similar to the select footswitch
// Encoder A button allows you to step through all the menus
// Used to enter a preset, then select an effect, then effect parameters can be changed
// Short press: Select currently hovered menu object increases menu level
// Long press: Functions as a back or return button, decreases menu level
void encoderButtonACheck(){

  // Serial.println(pressedTime);
  
  if(pressedTime > longPressDelay){
    if(menuLevel == 3){menuLevel = 1;}
    menuLevel--;
    if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0  
    encoderAPress = false;        
  } // Long press = back/return button
    
  if(pressedTime < longPressDelay){ // Only count short presses between 100ms and 500ms
    menuLevel++; // Short press = select/step into button
    if(menuLevel == 1){ // If stepping into preset menu, remember current preset
      if(encoderAPosition == 10){
          menuLevel = 3;
          encoderAPress = false;
          return;
      }
      currentPreset = encoderAPosition;
    }
  
    if(menuLevel > 2){menuLevel = 2;} // Limit max menu level to 2
    if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0
    
  }
     encoderAPress = false;   
    
}

void encoderButtonBCheck(){

  // If encoder button is pressed wait till unpressed and check time
  // If encoder is short pressed, step into next menu level
  // If encoder is long pressed, return to previous menu level
      
  if(pressedTime > longPressDelay){
    saveEEPROM(); 
    lcd.setCursor(15,0);
    lcd.print("saved");
  }

  encoderBPress = false;
}

// Select footswitch/buttonA logic contained within
// Select footswitch allows the user to scroll through the main menu and enable presets
// Short press: Scroll through main menu in one direction with wrap around
// Long press: select currently hovered preset, if already in a preset then returns to main menu
// allowing a new preset to be choosen
void buttonACheck(){
  
  // Long press logic    
  if(pressedTime > longPressDelay){
    menuLevel++;
    if(menuLevel == 1){ // If stepping into preset menu, remember current preset
      currentPreset = encoderAPosition;
      if(encoderAPosition == 10){
          menuLevel = 3;
          encoderAPress = false;
          return;
      }      
    }
    }

  // Short press logic
  if(pressedTime <= longPressDelay){ // Only count short presses between 100ms and 500ms
    encoderAPosition++;
    encoderA.write(encoderAPosition*4);
  }

  // Menu level limits and wrapping
  buttonAPress = false;
  if(menuLevel == 3){return;}
  if(menuLevel > 1){menuLevel = 0;} // Limit max menu level to 1 reset to 0
  if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0
  
}


// Button B is the toggle button
// Short presses switch between the togglePreset[] list of presets
// Use the select button to hover over a preset you want to add to the list
// Long press toggle switch to add the preset to the end of the list
// Push index 0 of the list out
// Toggle presets are indicated by an index number on the right side of the screen
// in the main menu
void buttonBCheck(){
  
  if(pressedTime > longPressDelay){
    if(menuLevel == 3){encoderBPosition++; buttonBPress = false; return;}
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
  if(pressedTime <= longPressDelay){ // Only count short presses between 100ms and 500ms
    // Set current preset to the toggle preset
    if(menuLevel == 3){encoderBPosition++; buttonBPress = false; return;}
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


void menuUpdate(){

  switch(menuLevel){ // Call correct menu update function based on menu level
    
      case 0: // Main Menu
        // In main menu selection is limited to 0-9, with wrap around
        if(encoderAPosition > 10){encoderA.write(0); encoderAPosition = 0;} // If read position is greater than 9 set position to 0
        if(encoderAPosition < 0){encoderA.write(40); encoderAPosition = 10;} // If read position is less than 0 set position to 9
        mainMenuDraw(encoderAPosition);
        break;
        
      case 1: // Preset Menu
        // In preset menu selection is limited to one effects 0
        if(encoderAPosition > 0){encoderA.write(0); encoderAPosition = 0;} // If read position is greater than 1 set position to 0
        if(encoderAPosition < 0){encoderA.write(0); encoderAPosition = 0;} // If read position is less than 0 set position to 1
        presetMenuDraw(encoderAPosition);
        break;
        
      case 2: // Effect Menu
        // In effect menu encoder A selects parameter, encoder B sets value
        paramMax = T9PB_get_parameter_max(currentEffect, encoderAPosition+1);
        paramMin = T9PB_get_parameter_min(currentEffect, encoderAPosition+1);
        if(encoderBPosition > paramMax){encoderB.write(paramMax*4); encoderBPosition = paramMax;} // If read position is greater than 99 set position to 99
        if(encoderBPosition < paramMin){encoderB.write(paramMin*4); encoderBPosition = paramMin;} // If read position is less than 0 set position to 0       
        presetParams[currentPreset][encoderAPosition] = encoderBPosition;
        effectMenuDraw(encoderAPosition, encoderBPosition);
        paramChange = T9PB_change_parameter(currentEffect, encoderAPosition + 1, encoderBPosition);
        break;

      case 3:
        // Settings menu, encoder A selects preset, encoder B selects effect
        if(encoderAPosition > 9){encoderA.write(0); encoderAPosition = 0;} // If read position is greater than 9 set position to 0
        if(encoderAPosition < 0){encoderA.write(36); encoderAPosition = 9;} // If read position is less than 0 set position to 9
        if(encoderBPosition > 4){encoderB.write(0); encoderBPosition = 0;} // If read position is greater than 9 set position to 0
        if(encoderBPosition < 0){encoderB.write(16); encoderBPosition = 4;} // If read position is less than 0 set position to 9
        settingsMenuDraw(encoderAPosition, encoderBPosition);
        break;
              
  }
 }

void mainMenuDraw(int x){ // Main menu drawing, x = selected preset (0-9)
  lcd.clear();
  menuScroll = x; // Use menuScroll variable to track preset scroll level
  // Can select 0-9 but, only scroll until settings shows on bottom line
  if(menuScroll > 7){menuScroll = 7;}
  if(menuScroll < 0){menuScroll = 0;}
  // Menu scroll limits, at scroll = 0 preset 0 is on top line
  // at scroll = 7 preset 7 is on top line, settings on bottom line
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
// List effect in preset
// Preset: Preset Name
// *Chorus
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
}

// Menu Level 2, draw effect name on top line
// List parameters within effect
// Preset: Preset Name
// Effect: Effect Name
// Parameter 1 = Encoder A Rotation
// Parameter 2 = Encoder B Rotation
void effectMenuDraw(int x, int y){ // x is encoder A values 0-2 y is encoder B values paramMin-paramMax
  
  menuScroll = x; // Use menuScroll variable to track preset scroll level

  if(x<2){menuScroll = 0;}
  else{menuScroll=x-2;}
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
    lcd.print(presetParams[currentPreset][i-1]);
  }
  
  // Use * to indicate hovered/selected preset
  lcd.setCursor(0,x+1);
  lcd.print("*");

}  

void settingsMenuDraw(int x, int y){
  
  if(!updated){ 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(presetName[10]); // Print effect name
    updated = true;
  }  
  lcd.setCursor(0,1);
  lcd.print("Edit Preset Effects");
  lcd.setCursor(0,2);
  lcd.print("P:");    
  lcd.setCursor(2,2);
  lcd.print(presetName[x]);
  lcd.setCursor(0,3);
  lcd.print("E:");    
  lcd.setCursor(2,3);
  lcd.print("          ");    
  lcd.setCursor(2,3);    
  lcd.print(T9PB_get_effect_name(y).c_str());             
}
