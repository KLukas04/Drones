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
#define MAX_SPEED 100
Servo fLeftM, fRightM, bLeftM, bRightM;
int fLeftS = 0;
int fRightS = 0;
int bLeftS = 0;
int bRightS = 0;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
