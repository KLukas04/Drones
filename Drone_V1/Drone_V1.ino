//PS2 Controller
#include <PS2X_lib.h>  //for v1.6

#define PS2_DAT        13  //14    
#define PS2_CMD        11  //15
#define PS2_SEL        10  //16
#define PS2_CLK        12  //17
byte error = 0;
byte type = 0;
PS2X ps2x;
int motorSpeed = 0;
byte vibration = 0;

//ESC
#include <Servo.h>

Servo motA, motB, motC, motD;
int mSpeed = 0;
#define MAX_SPEED 60

void setup() {
  Serial.begin(57600);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  //ps2-Controller
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, true);
  
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  
  type = ps2x.readType(); 
  
  switch(type) {
    case 0:
      Serial.println("Unknown Controller type found ");
      break;
    case 1:
      Serial.println("DualShock Controller found ");
      break;
    case 2:
      Serial.println("GuitarHero Controller found ");
      break;
  case 3:
      Serial.println("Wireless Sony DualShock Controller found ");
      break;
   }

  //ESC
  motA.attach(4, 1000, 2000); // 1000 and 2000 are very important ! Values can be different with other ESCs.
  motB.attach(5, 1000, 2000);
  motC.attach(6, 1000, 2000);
  motD.attach(7, 1000, 2000);

}

void loop() {
  
  ps2x.read_gamepad(false, vibration);

  //ESC-Calibration
  //Stufe 1
  if(ps2x.Button(PSB_L1)){
      Serial.println("Sending 180 throttle");
      motA.write(180);
      motB.write(180);
      motC.write(180);
      motD.write(180);
  }
  //Hier Motor anschließen, Stufe 2
  if(ps2x.Button(PSB_L2) || ps2x.Button(PSB_CIRCLE)){
    Serial.println("Sending 0 throttle");
    setAllZero();
  }
  //Stufe 3
  if(ps2x.Button(PSB_TRIANGLE)){
    
    setAllZero();

    for(int i = 0; i < 21; i++){
        motA.write(i);
        motB.write(i);
        motC.write(i);
        motD.write(i);
        delay(200);
    }
    for(int i = 21; i > 0 ; i++){
        motA.write(i);
        motB.write(i);
        motC.write(i);
        motD.write(i);
        delay(200);
    }

    setAllZero();
    
  }
  

  int y = ps2x.Analog(PSS_LY);

  if(y > 140){
    if(mSpeed < MAX_SPEED){
      mSpeed = mSpeed + 1;  
    }  
  }
  else if(y < 116){
    mSpeed = mSpeed - 1;  
  }
  
}

void setAllZero(){
  motA.write(0);
  motB.write(0);
  motC.write(0);
  motD.write(0);  
}