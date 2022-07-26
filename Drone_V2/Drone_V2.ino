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

//Gyro
#include <Wire.h>
int gyro_x, gyro_y, gyro_z;
long acc_x, acc_y, acc_z, acc_total_vector;
long gyro_x_cal, gyro_y_cal, gyro_z_cal;
float angle_pitch, angle_roll;
float angle_roll_acc, angle_pitch_acc;
float angle_pitch_output, angle_roll_output;
boolean set_gyro_angles;
int temperature;

void setup() {
  //Power for the Gyro

  Serial.begin(57600);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
  
  //Gyro
  pinMode(53, OUTPUT); 
  digitalWrite(53, HIGH);
  
  Wire.begin(); 
  setup_mpu_6050_registers();
  
  Serial.println("Calibrating gyro"); 
                                        //Print text to screen
  for (int cal_int = 0; cal_int < 1000 ; cal_int++){                  //Run this code 2000 times
    if(cal_int % 125 == 0)Serial.println(".");                              //Print a dot on the LCD every 125 readings
    read_mpu_6050_data();                                              //Read the raw acc and gyro data from the MPU-6050
    gyro_x_cal += gyro_x;                                              //Add the gyro x-axis offset to the gyro_x_cal variable
    gyro_y_cal += gyro_y;                                              //Add the gyro y-axis offset to the gyro_y_cal variable
    gyro_z_cal += gyro_z;                                              //Add the gyro z-axis offset to the gyro_z_cal variable
    delay(10);                                                          //Delay 3us to simulate the 250Hz program loop
  }
  gyro_x_cal /= 2000;                                                  //Divide the gyro_x_cal variable by 2000 to get the avarage offset
  gyro_y_cal /= 2000;                                                  //Divide the gyro_y_cal variable by 2000 to get the avarage offset
  gyro_z_cal /= 2000;                                                  //Divide the gyro_z_cal variable by 2000 to get the avarage offset
  //Serial.println("Pitch:");                                                 //Print text to screen                                                //Set the LCD cursor to position to position 0,1
  //Serial.println("Roll :");                                                 //Print text to screen

  Serial.println("Finish Calibration");
  
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
      delay(1000);
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
}

void loop() {
  
  ps2x.read_gamepad(false, vibration);

  //ESC-Calibration
  //Stufe 1
  if(ps2x.Button(PSB_L1)){
      Serial.println("Sending 180 throttle");
      setAllFull();
  }
  //Hier Motor anschließen, Stufe 2
  if(ps2x.Button(PSB_R1) || ps2x.Button(PSB_CIRCLE)){
    Serial.println("Sending 0 throttle");
    setAllZero();
    isCalibrated = true;
  }

  //Gyro
  
  read_mpu_6050_data();                                                //Read the raw acc and gyro data from the MPU-6050

  gyro_x -= gyro_x_cal;                                                //Subtract the offset calibration value from the raw gyro_x value
  gyro_y -= gyro_y_cal;                                                //Subtract the offset calibration value from the raw gyro_y value
  gyro_z -= gyro_z_cal;                                                //Subtract the offset calibration value from the raw gyro_z value
  
  //Gyro angle calculations
  //0.0000611 = 1 / (250Hz / 65.5)
  angle_pitch += gyro_x * 0.0000611;                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
  angle_roll += gyro_y * 0.0000611;                                    //Calculate the traveled roll angle and add this to the angle_roll variable
  
  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the pitch angle to the roll angel
  
  //Accelerometer angle calculations
  acc_total_vector = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));  //Calculate the total accelerometer vector
  //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
  angle_pitch_acc = asin((float)acc_y/acc_total_vector)* 57.296;       //Calculate the pitch angle
  angle_roll_acc = asin((float)acc_x/acc_total_vector)* -57.296;       //Calculate the roll angle
  
  //Place the MPU-6050 spirit level and note the values in the following two lines for calibration
  angle_pitch_acc -= 0.4;                                              //Accelerometer calibration value for pitch
  angle_roll_acc -= -2.32;                                               //Accelerometer calibration value for roll

  if(set_gyro_angles){                                                 //If the IMU is already started
    angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;     //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
    angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;        //Correct the drift of the gyro roll angle with the accelerometer roll angle
  }
  else{                                                                //At first start
    angle_pitch = angle_pitch_acc;                                     //Set the gyro pitch angle equal to the accelerometer pitch angle 
    angle_roll = angle_roll_acc;                                       //Set the gyro roll angle equal to the accelerometer roll angle 
    set_gyro_angles = true;                                            //Set the IMU started flag
  }
  
  //To dampen the pitch and roll angles a complementary filter is used
  angle_pitch_output = angle_pitch_output * 0.9 + angle_pitch * 0.1;   //Take 90% of the output pitch value and add 10% of the raw pitch value
  angle_roll_output = angle_roll_output * 0.9 + angle_roll * 0.1;      //Take 90% of the output roll value and add 10% of the raw roll value

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

  //Only change motor values if calibration is finished
  if(isCalibrated){
    fLeftM.write(fLeftS);
    fRightM.write(fRightS);
    bLeftM.write(bLeftS);
    bRightM.write(bRightS);
  }
 
  Serial.print("Pitch: ");
  Serial.print(angle_pitch_output);
  Serial.print(" --- Roll: ");
  Serial.println(angle_roll_output);
  /*
  Serial.print(" --- y-Axis: ");
  Serial.print(y);
  Serial.print(" --- FL: ");
  Serial.print(fLeftS);
  Serial.print(" - FR: ");
  Serial.print(fRightS);
  Serial.print(" - BL: ");
  Serial.print(bLeftS);
  Serial.print(" - BR: ");
  Serial.println(bRightS);
*/
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

void read_mpu_6050_data(){                                             //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x3B);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,14);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 14);                                        //Wait until all the bytes are received
  acc_x = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_x variable
  acc_y = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_y variable
  acc_z = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_z variable
  temperature = Wire.read()<<8|Wire.read();                            //Add the low and high byte to the temperature variable
  gyro_x = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_x variable
  gyro_y = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_y variable
  gyro_z = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_z variable
}

void setup_mpu_6050_registers(){
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1C);                                                    //Send the requested starting register
  Wire.write(0x10);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
}
