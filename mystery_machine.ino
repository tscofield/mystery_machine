#include <SdFat.h>
SdFat SD;
//#include <SD.h>                      // need to include the SD library
#include <TMRpcm.h>
#include "Wire.h"
#include <avr/pgmspace.h>

// Wav files
//prog_char theme[] PROGMEM = "SDtheme.wav";
//prog_char chase[] PROGMEM = "SDchase.wav";
//prog_char final[] PROGMEM = "SDfinal.wav";


// Set Pins
const byte led_under_blue_pin  = 3;      // pwm
const byte led_under_green_pin = 5;      // pwm
const byte led_under_red_pin   = 6;      // pwm
//const byte speaker_pin         = 9;      // pwm
const byte SD_ChipSelectPin    = 10;     // pwm
// MOSI pin for SD	11              // pwm
// MISO pin for SD	12
// SCK  pin for SD	13
const byte led_rear_red_pin    = A0;     // A0 = d14
const byte led_front_white_pin = A3; 
const byte SDA_pin             = A4;	// needed for gyro
const byte SDL_pin             = A5;	// needed for gyro

const int MPU=0x68;  // I2C address of the MPU-6050
//int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int16_t AcX,AcY,AcZ;
int16_t AcX_last,AcY_last,AcZ_last;
//float fAcX,fAcY,fAcZ;
//float fAcX_last,fAcY_last,fAcZ_last;

int led_under_green_bright=255;
int led_under_green_fade_amt=5;
int led_under_red_bright=255;
int led_under_red_fade_amt=5;
int led_under_blue_bright=255;
int led_under_blue_fade_amt=5;

byte led_rear_red_bright=0;

byte led_front_white_bright=0;

// mode = 1 pre race mode
// mode = 2 race mode
// mode = 3 post race mode
byte mode;
byte lastmode = 0;
byte modeinit = 0;

// loop counter
unsigned int loopctr=0;

// create an object for use in this sketch
TMRpcm tmrpcm;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  Serial.println(F("Lets get ready to race v. 6"));
  delay(100);

  // initialize the digital pin as an output.
  pinMode(led_under_red_pin, OUTPUT);
  pinMode(led_under_blue_pin, OUTPUT);
  pinMode(led_under_green_pin, OUTPUT);
  pinMode(led_rear_red_pin, OUTPUT);
  pinMode(led_front_white_pin, OUTPUT);

  // set initial leds
  digitalWrite(led_rear_red_pin, LOW);
  digitalWrite(led_front_white_pin, LOW);
  analogWrite(led_under_red_pin, 255);
  analogWrite(led_under_blue_pin, 255);
  analogWrite(led_under_green_pin, 255);

  // set mode = 0 for startup
  mode=0;

  tmrpcm.speakerPin = 9; 
//  tmrpcm.setVolume(4);

  Serial.print(F("\nInitializing SD card..."));
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    //return;   // don't do anything more if not
  }
  pinMode(SD_ChipSelectPin, OUTPUT);
  //tmrpcm.play("SDtheme.wav");
  
  Serial.println(F("Setup gyro"));
  Serial.println(F("Wire begin"));
  //delay(100);
  Wire.begin();
  Serial.println(F("begin transmission"));
  //delay(100);
  Wire.beginTransmission(MPU);
  Serial.println(F("Write 6b"));
  //delay(100);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Serial.println(F("Write 0"));
  //delay(100);
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Serial.println(F("end transmission"));
  Serial.print(F("freeRAM "));
  Serial.println(freeRam());
  //delay(100);
  Wire.endTransmission(true);
  Serial.println(F("End setup"));
  //delay(1000);
}

// the loop routine runs over and over again forever:
void loop() {
  // silence the speaker if nothing is playing
  //if(!tmrpcm.isPlaying()) {
  //  Serial.println(F("Silence the speaker"));
  //  digitalWrite(tmrpcm.speakerPin, LOW);
  //}  

  //  Serial.println("begin loop");

  //Serial.print(F("freeRAM "));
  //Serial.println(freeRam());
  
  // read gyro data
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
//  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
//  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
//  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
//  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  // convert accelerometer readings to floating point and 1G
//  fAcX=AcX/16384.0;
//  fAcY=AcY/16384.0;
//  fAcZ=AcZ/16384.0;
  Serial.print(F("loop "));
  Serial.print(loopctr);
  Serial.print(F(" | AcX = "));
  Serial.print(AcX/16384.0);
  Serial.print(F(" | AcY = "));
  Serial.print(AcY/16384.0);
  Serial.print(F(" | AcZ = "));
  Serial.print(AcZ/16384.0);
//  Serial.print(F(" | Tmp = "));
//  Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
//  Serial.print(F(" | GyX = "));
//  Serial.print(GyX);
//  Serial.print(F(" | GyY = "));
//  Serial.print(GyY);
//  Serial.print(F(" | GyZ = "));
//  Serial.println(GyZ);

  Serial.print(F(" | mode "));
  Serial.print(mode);
  Serial.print(F(" |  "));
  print_under_RGB();

  //Serial.println(F(" "));
  // done reading gyro data

  //delay(4);
  delay(4);
  loopctr++;

  // Set the mode based on accel and gyro
  // If my physics is correct fAcZ should be ~ 0.5 on a 30deg ramp
  if ((AcX/16384.0) > 0.7 && (AcX/16384.0) < 0.9 && (AcZ_last/16384.0) > 0.45) {
    //Serial.println(F("we appear to be on the ramp, changing mode"));
    //delay(1000);
    mode = 1;
  } 
  else if ( mode == 1 && (AcX/16384.0) < 0.9 && (AcZ/16384.0) < 0.4 ) {
    Serial.println(F("We appear to be moving"));
    mode = 2;
  } 
  else if ( mode == 2 && (AcZ/16384.0) < 0 && (AcX/16384.0) > 0.9 ) {
    mode = 3;
  } 
  else if ( mode == 3 && (AcX/16384.0) < -0.96) {
    // if its upside down reset to mode 0
    mode = 0;
  }
  else if ( mode > 3 ) {
    mode=0;
  } 
  //  Serial.print("Now in mode ");
  //  Serial.println(mode, DEC);
  //  button_state_last=button_state;

  // if we have changed mode, stop audio playback
  if(mode != lastmode) {
    // Stop any existing playback
    //Serial.println("check to see if any sounds are playing");
    //delay(1000);

    if(tmrpcm.isPlaying()){
      Serial.println(F("Changed mode, stopping playback"));
      tmrpcm.stopPlayback();
    }
    lastmode=mode;
    modeinit = 1;
  } 
  else {
    modeinit = 0;
  }

  // Take action for the mode we are in
  //Serial.println("Take action for the mode that is set");
  //delay(1000);
  
  switch (mode) {
    // race mode
  case 2:
    race();
    break;
    // pre race mode
  case 1:
    //Serial.println(F("about to call pre_race function"));

    pre_race();
    break;
    // post race mode
  case 3:
    post_race();
    break;
  case 0:
    // in mode 0, take a bit of a break to save battery
    // set initial leds
    digitalWrite(led_rear_red_pin, LOW);
    digitalWrite(led_front_white_pin, LOW);
    analogWrite(led_under_red_pin, 255);
    analogWrite(led_under_blue_pin, 255);
    analogWrite(led_under_green_pin, 255);
    delay(1000);
  }
  AcX_last=AcX;
  AcY_last=AcY;
  AcZ_last=AcZ;
}

void pre_race() {
  //Serial.println(F("Entering pre race mode"));

  // Set rear lights to red
  digitalWrite(led_rear_red_pin, HIGH);

  // Turn off under lights
  led_under_red_bright=255;
  led_under_blue_bright=255;
  led_under_green_bright=255;
  analogWrite(led_under_red_pin, led_under_red_bright);
  analogWrite(led_under_blue_pin, led_under_blue_bright);
  analogWrite(led_under_green_pin, led_under_green_bright);

  // turn on front lights to bright white
  led_front_white_bright=255;
  analogWrite(led_front_white_pin, led_front_white_bright);

  //Serial.println(F("done with lights, now get setup for music"));

  if(modeinit == 1) {
    // Play the SD theme
    Serial.println(F("play Scoby Doo Theme ............................."));
    //Serial.print(F("freeRAM "));
    //Serial.println(freeRam());
    tmrpcm.play("SDtheme.wav");
    //delay(10000);
  }
}

void race() {
  //Serial.println(F("Entering race mode"));

  // reduce brightness on the rear red light
  digitalWrite(led_rear_red_pin, LOW);

  // set under colors
  fade_under_green();
  led_under_red_bright=255;
  led_under_blue_bright=255;
  analogWrite(led_under_red_pin, led_under_red_bright);
  analogWrite(led_under_blue_pin, led_under_blue_bright);
  //print_under_RGB();

  // Play sound effects for the race
  if(modeinit == 1) {
    // Play the race sounds
    Serial.println(F("play race sound effects"));
    tmrpcm.play("SDchase.wav");
  }
}

void post_race() {
  //Serial.println(F("Entering post race mode"));
  digitalWrite(led_rear_red_pin, HIGH);

  // Make the under lights go crazy
  if ((loopctr % 7) == 0) {
    fade_under_green();
  }
  if ((loopctr % 11) == 0) {
    fade_under_blue();
  }
  if ((loopctr % 13) == 0) {
    fade_under_red();
  }
  //print_under_RGB();

  // flash the front and rear lights
  if ((loopctr % 5) == 0) {
    digitalWrite(led_rear_red_pin, HIGH);
    led_front_white_bright=255;
    analogWrite(led_front_white_pin, led_front_white_bright);
  }
  if ((loopctr % 10) == 0) {
    digitalWrite(led_rear_red_pin, LOW);
    led_front_white_bright=0;
    analogWrite(led_front_white_pin, led_front_white_bright);
  }
}

void print_under_RGB() {
  Serial.print(F("Under RGB "));
  Serial.print(led_under_red_bright, DEC);
  Serial.print(F(", "));
  Serial.print(led_under_green_bright, DEC);
  Serial.print(F(", "));
  Serial.print(led_under_blue_bright, DEC);
  Serial.print(F(" | Under fade "));
  Serial.print(led_under_red_fade_amt, DEC);
  Serial.print(F(", "));
  Serial.print(led_under_green_fade_amt, DEC);
  Serial.print(F(", "));
  Serial.print(led_under_blue_fade_amt, DEC);
  Serial.println(F(" "));
}

void fade_under_green() {
  if (led_under_green_bright <= 200) { 
    led_under_green_fade_amt = abs(led_under_green_fade_amt);
    led_under_green_bright = 200;
  } 
  else if (led_under_green_bright >= 255) {
    led_under_green_fade_amt = -abs(led_under_green_fade_amt);
    led_under_green_bright = 255;
  }
  led_under_green_bright = led_under_green_bright + led_under_green_fade_amt;
  analogWrite(led_under_green_pin, led_under_green_bright);
}

void fade_under_blue() {
  if (led_under_blue_bright <= 200) { 
    led_under_blue_fade_amt = abs(led_under_blue_fade_amt) ; 
    led_under_blue_bright = 200;
  } 
  else if (led_under_blue_bright >= 255) {
    led_under_blue_fade_amt = -abs(led_under_blue_fade_amt) ; 
    led_under_blue_bright = 255;
  }
  led_under_blue_bright = led_under_blue_bright + led_under_blue_fade_amt;
  analogWrite(led_under_blue_pin, led_under_blue_bright);
}

void fade_under_red() {
  if (led_under_red_bright <= 200) { 
    led_under_red_fade_amt = abs(led_under_red_fade_amt);
    led_under_red_bright = 100;

  } 
  else if (led_under_red_bright >= 255) {
    led_under_red_fade_amt = -abs(led_under_red_fade_amt) ; 
    led_under_red_bright = 255;
  }
  led_under_red_bright = led_under_red_bright + led_under_red_fade_amt;
  analogWrite(led_under_red_pin, led_under_red_bright);
}

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
