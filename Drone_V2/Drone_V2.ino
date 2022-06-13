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
#define MAX_SPEED 180
Servo fLeftM, fRightM, bLeftM, bRightM;
int fLeftS = 0;
int fRightS = 0;
int bLeftS = 0;
int bRightS = 0;
bool isCalibrated = false;
bool runLoopOnce = false;

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
  fLeftM.attach(4, 1000, 2000); // 1000 and 2000 are very important ! Values can be different with other ESCs.
  fRightM.attach(5, 1000, 2000);
  bLeftM.attach(6, 1000, 2000);
  bRightM.attach(7, 1000, 2000);
  Serial.println("Attached ESCs");

  setAllFull();
}

void loop() {
  
  ps2x.read_gamepad(false, vibration);

  //ESC-Calibration
  //Stufe 1
  if(!isCalibrated && ps2x.Button(PSB_L1)){
      Serial.println("Sending 180 throttle");
      setAllFull();
  }
  //Hier Motor anschließen, Stufe 2
  if(ps2x.Button(PSB_R1) || ps2x.Button(PSB_CIRCLE)){
    Serial.println("Sending 0 throttle");
    setAllZero();
    isCalibrated = true;
  }

  int y = ps2x.Analog(PSS_LY);

  //adjusting speed based on values
  if(y < 116){
    if(fLeftS < MAX_SPEED){
      fLeftS += 1;  
    }
    if(fRightS < MAX_SPEED){
      fRightS += 1;  
    }
    if(bLeftS < MAX_SPEED){
      bLeftS += 1;  
    }
    if(bRightS < MAX_SPEED){
      bRightS += 1;  
    }
  }
  else if(y > 140){
    if(fLeftS > 0){
      fLeftS -= 1;  
    }
    if(fRightS > 0){
      fRightS -= 1;  
    }
    if(bLeftS > 0){
      bLeftS -= 1;  
    }
    if(bRightS > 0){
      bRightS -= 1;  
    }
  }

  //Only change motor values if calibration is finihed
  if(isCalibrated){
    fLeftM.write(fLeftS);
    fRightM.write(fRightS);
    bLeftM.write(bLeftS);
    bRightM.write(bRightS);
  }
 
  delay(10);
}

void setAllZero(){
  fLeftM.write(0);
  fRightM.write(0);
  bLeftM.write(0);
  bRightM.write(0);

  fLeftS = 0;
  fRightS = 0;
  bLeftS = 0;
  bRightS = 0;
}

void setAllFull(){
  fLeftM.write(180);
  fRightM.write(180);
  bLeftM.write(180);
  bRightM.write(180);
}
