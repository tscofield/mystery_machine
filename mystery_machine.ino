#include <SD.h>                      // need to include the SD library
#include <TMRpcm.h>
#include "Wire.h"

// Set Pins
const int led_under_green_pin = 3;      // pwm
const int led_under_blue_pin  = 5;      // pwm
const int led_under_red_pin   = 6;      // pwm
const int speaker_pin         = 9;      // pwm
#define SD_ChipSelectPin 10             // pwm
// MOSI pin for SD	11              // pwm
// MISO pin for SD	12
// SCK  pin for SD	13
const int led_rear_red_pin    = A0;     // A0 = d14
const int led_front_white_pin   = A3; 
const int SDA_pin             = A4;	// needed for gyro
const int SDL_pin             = A5;	// needed for gyro

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float fAcX,fAcY,fAcZ;
float fAcX_last,fAcY_last,fAcZ_last;

int led_under_green_bright=255;
int led_under_green_fade_amt=5;
int led_under_red_bright=255;
int led_under_red_fade_amt=5;
int led_under_blue_bright=255;
int led_under_blue_fade_amt=5;

int led_rear_red_bright=0;
int led_rear_red_fade_amt=5;

int led_front_white_bright=0;
int led_front_white_fade_amt=5;

// mode = 1 pre race mode
// mode = 2 race mode
// mode = 3 post race mode
int mode;
int lastmode = 0;
int modeinit = 0;

// loop counter
int loopctr=0;

// create an object for use in this sketch
TMRpcm tmrpcm;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  Serial.println("Lets get ready to race v. 2");
  
  // initialize the digital pin as an output.
  pinMode(led_under_red_pin, OUTPUT);
  pinMode(led_under_blue_pin, OUTPUT);
  pinMode(led_under_green_pin, OUTPUT);

  pinMode(led_rear_red_pin, OUTPUT);

  pinMode(led_front_white_pin, OUTPUT);

  // set mode = 0 for startup
  mode=0;
  
  Sd2Card card;
  SdVolume volume;
  SdFile root;

  tmrpcm.speakerPin = speaker_pin; 
  tmrpcm.setVolume(4);

  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    //return;   // don't do anything more if not
  }

  Serial.println("Setup gyro");
//  Serial.println("Wire begin");
  Wire.begin();
//  Serial.println("begin transmission");
  Wire.beginTransmission(MPU);
//  Serial.println("Write 6b");
  Wire.write(0x6B);  // PWR_MGMT_1 register
//  Serial.println("Write 0");
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
//  Serial.println("end transmission");
  Wire.endTransmission(true);

  Serial.println("End setup");
}

// the loop routine runs over and over again forever:
void loop() {
  // silence the speaker if nothing is playing
  if(!tmrpcm.isPlaying()) {
    digitalWrite(tmrpcm.speakerPin, LOW);
  }  
  
//  Serial.println("begin loop");

  // read gyro data
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  // convert accelerometer readings to floating point and 1G
  fAcX=AcX/16384.0;
  fAcY=AcY/16384.0;
  fAcZ=AcZ/16384.0;
  Serial.print("AcX = "); Serial.print(fAcX);
  Serial.print(" | AcY = "); Serial.print(fAcY);
  Serial.print(" | AcZ = "); Serial.print(fAcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
  // done reading gyro data

  print_under_RGB();
  
  //delay(4);
  delay(300);
  loopctr++;

  // Set the mode based on accel and gyro
  // If my physics is correct fAcZ should be ~ 0.5 on a 30deg ramp
  if (fAcZ > 0.3 && fAcZ_last <= 0.45 && mode != 2) {
    Serial.println("we appear to be on the ramp, changing mode");
    mode = 1;
  } else if (fAcZ > 0.45 && mode == 1) {
    Serial.println("We appear to be moving");
    mode = 2;
  } else if ( mode == 2 && fAcZ < 0 ) {
    mode = 3;
  } else if ( mode > 3 ) {
     mode=0;
  } 
//  Serial.print("Now in mode ");
//  Serial.println(mode, DEC);
//  button_state_last=button_state;
  
  // if we have changed mode, stop audio playback
  if(mode != lastmode) {
    // Stop any existing playback
    if(tmrpcm.isPlaying()){
      tmrpcm.stopPlayback();
    }
    lastmode=mode;
    modeinit = 1;
  } else {
    modeinit = 0;
  }
  
  // Take action for the mode we are in
  switch (mode) {
    // race mode
    case 2:
      race();
      break;
    // pre race mode
    case 1:
      pre_race();
      break;
    // post race mode
    case 3:
      post_race();
      break;
  }
  fAcX_last=fAcX;
  fAcY_last=fAcY;
  fAcZ_last=fAcZ;
}

void pre_race() {
  Serial.println("Entering pre race mode");
  
  // Set rear lights to red
  led_rear_red_bright=255;
  analogWrite(led_rear_red_pin, led_rear_red_bright);
  
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

  if(modeinit == 1) {
    // Play the SD theme
    Serial.println("play Scoby Doo Theme"); tmrpcm.play("SDtheme.wav");
  }
  }

void race() {
  Serial.println("Entering race mode");
  
  // reduce brightness on the rear red light
  led_rear_red_bright=0;
  analogWrite(led_rear_red_pin, led_rear_red_bright);

  // set under colors
  fade_under_green();
  led_under_red_bright=255;
  led_under_blue_bright=255;
  analogWrite(led_under_red_pin, led_under_red_bright);
  analogWrite(led_under_blue_pin, led_under_blue_bright);
  print_under_RGB();
  
  // Play sound effects for the race
  if(modeinit == 1) {
    // Play the race sounds
    Serial.println("play race sound effects"); tmrpcm.play("SDrun.wav");
  }
}

void post_race() {
  Serial.println("Entering post race mode");
  led_rear_red_bright=255;
  analogWrite(led_rear_red_pin, led_rear_red_bright);
  
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
  print_under_RGB();
  
  // flash the front and rear lights
  if ((loopctr % 50) == 0) {
    led_rear_red_bright=255;
    analogWrite(led_rear_red_pin, led_rear_red_bright);
    led_front_white_bright=255;
    analogWrite(led_front_white_pin, led_front_white_bright);
  }
  if ((loopctr % 100) == 0) {
    led_rear_red_bright=0;
    analogWrite(led_rear_red_pin, led_rear_red_bright);
    led_front_white_bright=0;
    analogWrite(led_front_white_pin, led_front_white_bright);
  }
    
}

void print_under_RGB() {
  Serial.print("Under RGB ");
  Serial.print(led_under_red_bright, DEC);
  Serial.print(", ");
  Serial.print(led_under_green_bright, DEC);
  Serial.print(", ");
  Serial.print(led_under_blue_bright, DEC);
  Serial.println(" ");
}

/*
void print_rear_RGB() {
  Serial.print("Rear RGB ");
  Serial.print(led_rear_red_bright, DEC);
  Serial.println(" ");
}
*/

void fade_under_green() {
  if (led_under_green_bright <= 0) { 
    led_under_green_fade_amt = abs(led_under_green_fade_amt) ; 
  } else if (led_under_green_bright >= 255) {
    led_under_green_fade_amt = -abs(led_under_green_fade_amt) ; 
  }
  led_under_green_bright = led_under_green_bright + led_under_green_fade_amt;
  analogWrite(led_under_green_pin, led_under_green_bright);
}

void fade_under_blue() {
  if (led_under_blue_bright <= 0) { 
    led_under_blue_fade_amt = abs(led_under_blue_fade_amt) ; 
  } else if (led_under_blue_bright >= 255) {
    led_under_blue_fade_amt = -abs(led_under_blue_fade_amt) ; 
  }
  led_under_blue_bright = led_under_blue_bright + led_under_blue_fade_amt;
  analogWrite(led_under_blue_pin, led_under_blue_bright);
}

void fade_under_red() {
  if (led_under_red_bright <= 0) { 
    led_under_red_fade_amt = abs(led_under_red_fade_amt) ; 
  } else if (led_under_red_bright >= 255) {
    led_under_red_fade_amt = -abs(led_under_red_fade_amt) ; 
  }
  led_under_red_bright = led_under_red_bright + led_under_red_fade_amt;
  analogWrite(led_under_red_pin, led_under_red_bright);
}
