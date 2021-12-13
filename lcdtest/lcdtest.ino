#include <LiquidCrystal_I2C.h>
#include <synth_simple_drum.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>

Encoder myEnc(5, 6);
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// GUItool: begin automatically generated code
AudioInputI2S            line_in;           //xy=388,500
AudioOutputI2S           line_out;           //xy=615,514
AudioConnection          patchCord1(line_in, 0, line_out, 0);
AudioConnection          patchCord2(line_in, 0, line_out, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=497,335
// GUItool: end automatically generated code

const int myInput = AUDIO_INPUT_LINEIN;

static uint32_t next;

void setup() {
  // put your setup code here, to run once:

  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T9 Encoder LCD Test"); 
  Serial.begin(115200);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.lineInLevel(0);
  //sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.volume(0.3);

}

long oldPosition  = 10;
float volumeKnob = 0.3;

void loop() {

  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    if(newPosition < 0){oldPosition = 0;}
    else if(newPosition > 100){oldPosition = 100;}
    else{oldPosition = newPosition;}
    
    Serial.println(oldPosition);
    sgtl5000_1.volume(0);
    lcd.setCursor(0,1);
    lcd.print("Encoder: ");    
    lcd.setCursor(9,1);
    lcd.print("     ");
    lcd.setCursor(9,1);
    lcd.print(oldPosition, DEC);

    volumeKnob = (float)oldPosition/100;
    lcd.setCursor(0,2);
    lcd.print("Volume: ");  
    lcd.setCursor(8,2);
    lcd.print("        ");
    lcd.setCursor(8,2);
    lcd.print(volumeKnob, DEC);
    sgtl5000_1.volume(volumeKnob);
  }
  
}

float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  float mapped = (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
  float rounded = ((int)mapped*100)/100;
  return rounded;
}
