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
 * Select Switch on pin 11
 * Toggle switch on pin 12
 * 
 * 20x4 LCD is conneceted to the second I2C port not used by the audio shield
 * SDA to pin 17
 * SCL to pin 16
 * 
 * TODO
 * Implement toggle switch functionality
 * Toggle switch allows you to select a "bank" of presets, and instantly swap between
 * them with a single fast press of the toggle switch
 * 
 * Implement a memory bank of effect presets and parameter values(To work with Nolan code)
 * 
 * 
 * 
 */


 
#include "effect_passthrough.h"
#include <LiquidCrystal_I2C.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#define ENCODER_USE_INTERRUPTS
#include <Encoder.h>

// GUItool: begin automatically generated code
AudioInputI2S            line_in;           //xy=388,500
AudioOutputI2S           line_out;           //xy=615,514
AudioConnection          patchCord1(line_in, 0, line_out, 0);
AudioConnection          patchCord2(line_in, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=497,335
// GUItool: end automatically generated code

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
boolean encoderAPress = false; // 

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
boolean updated = false; // bool to determine when menu needs to be updated
int currentPreset = 0; // Stores current preset selection 0-9
int currentEffect = 0; // Stores current effect selection 0-1
int currentEffects[2] = {0,0}; // Stores effectNames numbers for effects in selected preset
String effectName = "LPF"; // Store currently selected effect name



// Name of each preset
String presetName[10] = {"Preset 0", "Preset 1", "Preset 2", "Preset 3", "Preset 4", "Preset 5", "Preset 6", "Preset 7", "Preset 8", "Preset 9"};
                               
// Array of effect names with parameters
// Effects in order of design report effect table, skipping auto wah
// 0-LPF, 1-Reverb, 2-Chorus, 3-Delay, 4-ToneStack, 5-Overdrive, 6-Crush, 7-Flanger, 8-Tremolo, 9-Vibrato
String effectNames[10][3] = {{"LPF", "Cutoff", "Resonance"},{"Reverb", "Size", "Damping"},{"Chorus", "Voices", "Depth"},{"Delay", "Time", "Taps"},
                                  {"ToneStack", "Low", "High"},{"Overdrive", "Gain", "Drive"},{"Crush", "Resolution", "Samples"},{"Flanger", "Depth", "Rate"},{"Tremolo", "Depth", "Rate"},{"Vibrato", "Depth", "Rate"}};

// Effects within each preset use numbers from effectParameters
int presetEffects[10][2] = {{4, 1},{3, 1},{2, 1},{2, 3},{8, 2},{5, 0},{5, 1},{5, 3},{7, 8}, {9, 8}};



void setup() {
  initUI(); // Initialize LCD, Audio adapter, 
}



void loop() {

  encoderAPosition = encoderA.read()/4; // Read encoderA position 4 counts per click
  encoderBPosition = encoderB.read()/4; // Read encoderB position 4 counts per click
  
  if(encoderAPosition != oldPositionA) { // If encoderA position change, update menu   
    menuUpdate(); // Update Menu
    oldPositionA = encoderAPosition; // Store position
  } // End position update

  if((encoderBPosition != oldPositionB) && (menuLevel == 2)) { // If encoderB position change, update menu only for menulevel 2
    menuUpdate(); // Update Menu
    oldPositionB = encoderBPosition; // Store position
  } // End position update

  if(oldMenuLevel != menuLevel){
    menuUpdate();
    oldMenuLevel = menuLevel;  
  }

  if(digitalRead(encoderAPin) && encoderAPress){
    updated = false;
    encoderButtonACheck();
    menuUpdate(); // Update Menu system
  }
  if(digitalRead(buttonAPin) & buttonAPress){
    updated = false;
    buttonACheck();
  }
  


}

void initUI(){

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T9 UI Test");

  // Setup audio passthrough
  AudioMemory(40);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.lineInLevel(0);
  //sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.volume(0.7);

  // Setup encoderA button interrupt
  pinMode(encoderAPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderAPin), encoderAISR, FALLING);

  pinMode(buttonAPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonAPin), buttonAISR, FALLING);  

  pinMode(buttonBPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonBPin), buttonBISR, FALLING);
  // Initialize encoder positions
  encoderA.write(0);
  
  Serial.begin(115200);

  delay(1000);
  menuUpdate();
}

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
      currentPreset = encoderAPosition;
    }
    if(menuLevel == 2){ // If stepping into effect menu, remember current effect
      currentEffect = encoderAPosition;  
    }
    }
    
  if(menuLevel > 2){menuLevel = 2;} // Limit max menu level to 2
  if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0
    
  encoderAPress = false;
}

void buttonACheck(){
  
  releasedTime = millis(); // Check released time
  pressDelay = releasedTime - pressedTime; // Compare to pressed time
      
  if(pressDelay > shortPressDelay){
    menuLevel++;
    if(menuLevel == 1){ // If stepping into preset menu, remember current preset
      currentPreset = encoderAPosition;
    }
    }
    
  if((pressDelay <= shortPressDelay) && pressDelay > 100){ // Only count short presses between 100ms and 500ms
    encoderAPosition++;
    encoderA.write(encoderAPosition*4);
  }
    
  if(menuLevel > 1){menuLevel = 0;} // Limit max menu level to 1 reset to 0
  if(menuLevel < 0){menuLevel = 0;} // Limit min menu level to 0
  buttonAPress = false;
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
        // In preset menu selection is limited to the two effects 0-1, with wrap around
        if(encoderAPosition > 1){encoderA.write(0); encoderAPosition = 0;} // If read position is greater than 1 set position to 0
        if(encoderAPosition < 0){encoderA.write(4); encoderAPosition = 1;} // If read position is less than 0 set position to 1
        presetMenuDraw(encoderAPosition);
        
        // Insert effect wiring commands here
        
        break;
        
      case 2: // Effect Menu
        // In effect menu encoder positions directly change effect parameters 0-99
        if(encoderAPosition > 99){encoderA.write(396); encoderAPosition = 99;} // If read position is greater than 99 set position to 99
        if(encoderAPosition < 0){encoderA.write(0); encoderAPosition = 0;} // If read position is less than 0 set position to 0

        if(encoderBPosition > 99){encoderB.write(396); encoderBPosition = 99;} // If read position is greater than 99 set position to 99
        if(encoderBPosition < 0){encoderB.write(0); encoderBPosition = 0;} // If read position is less than 0 set position to 0        
        effectMenuDraw(encoderAPosition, encoderBPosition);

        // Insert effect parameter change commands here
        
        break;
  }
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
  // *P1:Preset Name
  //  P2:Preset Name
  //  P3:Preset Name
  //  P4:Preset Name
  for(int i = 0; i <= 3; i++){
    lcd.setCursor(1,i);
    lcd.print("P");
    lcd.setCursor(2,i);
    lcd.print(menuScroll+i, DEC);
    lcd.setCursor(3,i);
    lcd.print(":");
    lcd.setCursor(4,i);
    lcd.print(presetName[menuScroll+i]);
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

  currentEffects[0] = presetEffects[currentPreset][0];
  currentEffects[1] = presetEffects[currentPreset][1];
  
  for(int i = 0; i < 2; i++){
    lcd.setCursor(1,i + 1);
    lcd.print(effectNames[currentEffects[i]][0]);  
  }
    
  }
  // Print selection *
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.setCursor(0, 2);
  lcd.print(" ");
  lcd.setCursor(0, x+1);
  lcd.print("*");


}

// Menu Level 2, draw preset and effect name on top line
// List parameters within effect
// Preset: Preset Name
// Effect: Effect Name
// Parameter 1 = Encoder A Rotation
// Parameter 2 = Encoder B Rotation
void effectMenuDraw(int x, int y){ // x is encoder A values 0-99 y is encoder B values 0-99
  // effectName gets the String of the effect name from the effectNames list
  // currentEffects store the index 0-9 of the two effects in the preset
  // currentEffect stores the index 0-1 of the selected effect in the preset
  // effectNames[*][0] Holds the String for the name of each effect
  effectName = effectNames[currentEffects[currentEffect]][0];
  
  if(!updated){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Preset:"); 
  lcd.setCursor(8,0);
  lcd.print(presetName[currentPreset]); // Print preset name
  lcd.setCursor(0,1);
  lcd.print("Effect:");
  lcd.setCursor(8,1);
  lcd.print(effectName); // Print effect name
  updated = true;
  }

  for(int i = 0; i < 2; i++){
    lcd.setCursor(0, i+2);
    lcd.print(effectNames[currentEffects[currentEffect]][i+1]);
    lcd.setCursor(16, i+2);
    lcd.print("  ");
    lcd.setCursor(16, i+2);
    if(!i){lcd.print(encoderAPosition);} // For i = 0 print encoder A position 0-99
    if(i){lcd.print(encoderBPosition);} // For i = 1 print encoder B position 0-99
  }  
  
}

// Encoder A Button ISR
void encoderAISR() {
  encoderAPress = true;
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
