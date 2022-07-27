#include <Wire.h>
#include <Servo.h>

//PS2 Controller
#include <PS2X_lib.h>  //for v1.6

#define PS2_DAT        13  //14    
#define PS2_CMD        11  //15
#define PS2_SEL        10  //16
#define PS2_CLK        12  //17
byte errorps = 0;
byte type = 0;
PS2X ps2x;
int motorSpeed = 0;
byte vibration = 0;

Servo right_prop;
Servo left_prop;
Servo left_back_prop;
Servo right_back_prop;

bool isCalibrated = false;

int MAX_SPEED = 1400;

/*MPU-6050 gives you 16 bits data so you have to create some 16int constants
 * to store the data for accelerations and gyro*/

int16_t Acc_rawX, Acc_rawY, Acc_rawZ,Gyr_rawX, Gyr_rawY, Gyr_rawZ;
 
float Acceleration_angle[2];
float Gyro_angle[2];
float Total_angle[2];

float elapsedTime, time, timePrev;
int i;
float rad_to_deg = 180/3.141592654;

float PID[2];
float error[2];
float previous_error[2];

float pwmFrontLeft, pwmBackLeft, pwmFrontRight, pwmBackRight;
float pid_p[2] = {0, 0};
float pid_i[2] = {0, 0};
float pid_d[2] = {0, 0};
/////////////////PID CONSTANTS/////////////////
double kp=2.5;//3.55
double ki=0.02;//0.003
double kd=5.0;//2.05
///////////////////////////////////////////////

double throttle=1000; //initial value of throttle to the motors
float desired_angle[2] = {0, 0}; //This is the angle in which we whant the
                         //balance to stay steady

int counter = 0;

void setup() {

  Serial.begin(9600);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  //PS4
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  errorps = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, true);
  
  if(errorps == 0){
    Serial.print("Found Controller, configured successful ");
  }  
  else if(errorps == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(errorps == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(errorps == 3)
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
  
  //Gyro
  pinMode(53, OUTPUT); 
  digitalWrite(53, HIGH);
  
  Wire.begin(); //begin the wire comunication
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  //ESC and Motors
  right_prop.attach(4, 1000, 2000); //attach the right motor to pin 4
  left_prop.attach(5, 1000, 2000);  //attach the left motor to pin 5
  left_back_prop.attach(7, 1000, 2000); //attach the right motor to pin 4
  right_back_prop.attach(6, 1000, 2000); //attach the right motor to pin 4

  time = millis(); //Start counting time in milliseconds
  /*In order to start up the ESCs we have to send a min value
   * of PWM to them before connecting the battery. Otherwise
   * the ESCs won't start up or enter in the configure mode.
   * The min value is 1000us and max is 2000us, REMEMBER!*/
   /*
  This would be the auto calibration
  Serial.println("Sending 2000 throttle"); 
  left_prop.writeMicroseconds(2000); 
  right_prop.writeMicroseconds(2000);
  left_back_prop.writeMicroseconds(2000);
  right_back_prop.writeMicroseconds(2000);
  delay(10000); /*Give some delay, 7s, to have time to connect
                the propellers and let everything start up
  Serial.println("Sending 1000 throttle");
  left_prop.writeMicroseconds(1000); 
  right_prop.writeMicroseconds(1000);
  left_back_prop.writeMicroseconds(1000);
  right_back_prop.writeMicroseconds(1000);
  delay(10000);
  Serial.println("Finish Calibration");
  */
}

void loop() {

  //read the controller
  ps2x.read_gamepad(false, vibration);

  //ESC-Calibration
  //Stufe 1
  if(!isCalibrated && ps2x.Button(PSB_L1)){
      Serial.println("Sending 180 throttle");
      
      right_prop.write(180);
      left_prop.write(180);
      right_back_prop.write(180);
      left_back_prop.write(180);
      
  }
  //Hier Motor anschließen, Stufe 2
  if(ps2x.Button(PSB_R1) || ps2x.Button(PSB_CIRCLE)){
    //Serial.println("L2 or Circle");
    Serial.println("Sending 0 throttle");
    setAllZero();
    throttle = 1000;
    isCalibrated = true;
    pwmFrontRight = 0;
    pwmFrontLeft = 0;
    pwmBackLeft = 0;
    pwmBackRight = 0;
  }

  if(isCalibrated){
    int y = ps2x.Analog(PSS_LY);

    if(y < 116){
      if(throttle < MAX_SPEED){
        throttle = throttle + 10;  
      }  
    }
    else if(y > 140){
      if(throttle > 1000){
        throttle = throttle - 10;     
      }
    }
  
  /////////////////////////////I M U/////////////////////////////////////
      timePrev = time;  // the previous time is stored before the actual time read
      time = millis();  // actual time read
      elapsedTime = (time - timePrev) / 1000; 
    
    /*The timeStamp is the time that elapsed since the previous loop. 
     * This is the value that we will use in the formulas as "elapsedTime" 
     * in seconds. We work in ms so we haveto divide the value by 1000 
     to obtain seconds*/
  
    /*Read the values that the accelerometre gives.
     * We know that the slave adress for this IMU is 0x68 in
     * hexadecimal. For that in the RequestFrom and the 
     * begin functions we have to put this value.*/
     
       Wire.beginTransmission(0x68);
       Wire.write(0x3B); //Ask for the 0x3B register- correspond to AcX
       Wire.endTransmission(false);
       Wire.requestFrom(0x68,6,true); 
     
     /*We have asked for the 0x3B register. The IMU will send a brust of register.
      * The amount of register to read is specify in the requestFrom function.
      * In this case we request 6 registers. Each value of acceleration is made out of
      * two 8bits registers, low values and high values. For that we request the 6 of them  
      * and just make then sum of each pair. For that we shift to the left the high values 
      * register (<<) and make an or (|) operation to add the low values.*/
      
       Acc_rawX=Wire.read()<<8|Wire.read(); //each value needs two registres
       Acc_rawY=Wire.read()<<8|Wire.read();
       Acc_rawZ=Wire.read()<<8|Wire.read();
  
   
      /*///This is the part where you need to calculate the angles using Euler equations///*/
      
      /* - Now, to obtain the values of acceleration in "g" units we first have to divide the raw   
       * values that we have just read by 16384.0 because that is the value that the MPU6050 
       * datasheet gives us.*/
      /* - Next we have to calculate the radian to degree value by dividing 180º by the PI number
      * which is 3.141592654 and store this value in the rad_to_deg variable. In order to not have
      * to calculate this value in each loop we have done that just once before the setup void.
      */
  
      /* Now we can apply the Euler formula. The atan will calculate the arctangent. The
       *  pow(a,b) will elevate the a value to the b power. And finnaly sqrt function
       *  will calculate the rooth square.*/
       /*---X---*/
       Acceleration_angle[0] = atan((Acc_rawY/16384.0)/sqrt(pow((Acc_rawX/16384.0),2) + pow((Acc_rawZ/16384.0),2)))*rad_to_deg;
       /*---Y---*/
       Acceleration_angle[1] = atan(-1*(Acc_rawX/16384.0)/sqrt(pow((Acc_rawY/16384.0),2) + pow((Acc_rawZ/16384.0),2)))*rad_to_deg;
   
     /*Now we read the Gyro data in the same way as the Acc data. The adress for the
      * gyro data starts at 0x43. We can see this adresses if we look at the register map
      * of the MPU6050. In this case we request just 4 values. W don¡t want the gyro for 
      * the Z axis (YAW).*/
      
     Wire.beginTransmission(0x68);
     Wire.write(0x43); //Gyro data first adress
     Wire.endTransmission(false);
     Wire.requestFrom(0x68,4,true); //Just 4 registers
     
     Gyr_rawX=Wire.read()<<8|Wire.read(); //Once again we shif and sum
     Gyr_rawY=Wire.read()<<8|Wire.read();
   
     /*Now in order to obtain the gyro data in degrees/seconda we have to divide first
     the raw value by 131 because that's the value that the datasheet gives us*/
  
     /*---X---*/
     Gyro_angle[0] = Gyr_rawX/131.0; 
     /*---Y---*/
     Gyro_angle[1] = Gyr_rawY/131.0;
  
     /*Now in order to obtain degrees we have to multiply the degree/seconds
     *value by the elapsedTime.*/
     /*Finnaly we can apply the final filter where we add the acceleration
     *part that afects the angles and ofcourse multiply by 0.98 */
  
     /*---X axis angle---*/
     Total_angle[0] = 0.98 *(Total_angle[0] + Gyro_angle[0]*elapsedTime) + 0.02*Acceleration_angle[0];
     /*---Y axis angle---*/
     Total_angle[1] = 0.98 *(Total_angle[1] + Gyro_angle[1]*elapsedTime) + 0.02*Acceleration_angle[1];  
  
     //Serial.println(Total_angle[0]);
  
    if(throttle > 1001){
      /*///////////////////////////P I D///////////////////////////////////*/
      /*Remember that for the balance we will use just one axis. I've choose the x angle
      to implement the PID with. That means that the x axis of the IMU has to be paralel to
      the balance*/
      
      /*First calculate the error between the desired angle and 
      *the real measured angle*/
      error[0] = Total_angle[0] + 4 - desired_angle[0];
      error[1] = Total_angle[1] + 4 - desired_angle[1];
          
      /*Next the proportional value of the PID is just a proportional constant
      *multiplied by the error*/
      
      pid_p[0] = kp*error[0];
      pid_p[1] = kp*error[1];
      
      /*The integral part should only act if we are close to the
      desired position but we want to fine tune the error. That's
      why I've made a if operation for an error between -2 and 2 degree.
      To integrate we just sum the previous integral value with the
      error multiplied by  the integral constant. This will integrate (increase)
      the value each loop till we reach the 0 point*/
      if(-3 < error[0] <3)
      {
        pid_i[0] = pid_i[0]+(ki*error[0]);  
      }
      if(-3 < error[1] <3)
      {
        pid_i[1] = pid_i[1]+(ki*error[1]);  
      }
      
      /*The last part is the derivate. The derivate acts upon the speed of the error.
      As we know the speed is the amount of error that produced in a certain amount of
      time divided by that time. For that we will use a variable called previous_error.
      We substract that value from the actual error and divide all by the elapsed time. 
      Finnaly we multiply the result by the derivate constant*/
      
      pid_d[0] = kd*((error[0] - previous_error[0])/elapsedTime);
      pid_d[1] = kd*((error[1] - previous_error[1])/elapsedTime);
      
      /*The final PID values is the sum of each of this 3 parts*/
      PID[0] = pid_p[0] + pid_i[0] + pid_d[0];
      PID[1] = pid_p[1] + pid_i[1] + pid_d[1];
      
      /*We know that the min value of PWM signal is 1000us and the max is 2000. So that
      tells us that the PID value can/s oscilate more than -1000 and 1000 because when we
      have a value of 2000us the maximum value taht we could sybstract is 1000 and when
      we have a value of 1000us for the PWM sihnal, the maximum value that we could add is 1000
      to reach the maximum 2000us*/
      if(PID[0] < -1000)
      {
        PID[0] =-1000;
      }
      if(PID[0] > 1000)
      {
        PID[0] =1000;
      }
      if(PID[1] < -1000)
      {
        PID[1] =-1000;
      }
      if(PID[1] > 1000)
      {
        PID[1] =1000;
      }
      
      /*Finally we calculate the PWM width. We sum the desired throttle and the PID value*/
      //pwmLeft = throttle + PID;
      //pwmRight = throttle - PID;
      pwmFrontLeft = throttle + PID[0];
      pwmBackLeft = throttle + PID[0];
      pwmFrontRight = throttle - PID[0];
      pwmBackRight = throttle - PID[0];
      
      pwmFrontLeft = throttle + PID[1];
      pwmFrontRight = throttle + PID[1];
      pwmBackLeft = throttle - PID[1];
      pwmBackRight = throttle - PID[1];
      
      /*Once again we map the PWM values to be sure that we won't pass the min
      and max values. Yes, we've already mapped the PID values. But for example, for 
      throttle value of 1300, if we sum the max PID value we would have 2300us and
      that will mess up the ESC.*/
      //pwmFrontRight
      if(pwmFrontRight < 1000)
      {
        pwmFrontRight= 1100;
      }
      if(pwmFrontRight > 2000)
      {
        pwmFrontRight=2000; 
      }
      //pwmFrontLeft
      if(pwmFrontLeft < 1000)
      {
        pwmFrontLeft= 1100;
      }
      if(pwmFrontLeft > 2000)
      {
        pwmFrontLeft =2000;
      }
      
      //pwmBackRight
      if(pwmBackRight < 1000)
      {
        pwmBackRight = 1010;
      }
      if(pwmBackRight > MAX_SPEED)
      {
        pwmBackRight = MAX_SPEED; 
      }
      //pwmBackLeft
      if(pwmBackLeft < 1000)
      {
        pwmBackLeft = 1010;
      }
      if(pwmBackLeft > MAX_SPEED)
      {
        pwmBackLeft = MAX_SPEED;
      }  
    }
  /*Finally using the servo function we create the PWM pulses with the calculated
  width for each pulse*/
  //Serial.print(" --- LEFT:");
  //Serial.print(pwmLeft);
  //Serial.print(" --- RIGHT: ");
  //Serial.println(pwmRight);
  
  left_prop.writeMicroseconds(pwmFrontLeft);
  right_prop.writeMicroseconds(pwmFrontRight);
  left_back_prop.writeMicroseconds(pwmBackLeft);
  right_back_prop.writeMicroseconds(pwmBackRight);
  previous_error[0] = error[0];
  previous_error[1] = error[1];  
}

if(counter % 10 == 0){
  Serial.print("Roll: ");  
  Serial.print(Total_angle[0]);
  Serial.print(" Pitch: ");
  Serial.println(Total_angle[1]);
}

delay(50);
counter++;
}

void setAllZero(){
  right_prop.write(0);
  left_prop.write(0);
  right_back_prop.write(0);
  left_back_prop.write(0);  
}
