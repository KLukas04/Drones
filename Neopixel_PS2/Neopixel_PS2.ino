//PS2 Controller
#include <PS2X_lib.h>  //for v1.6

#define PS2_DAT        13  //14    
#define PS2_CMD        11  //15
#define PS2_SEL        10  //16
#define PS2_CLK        12  //17

PS2X ps2x; // create PS2 Controller Class
int error = 0;
byte type = 0;
//Neopixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN 2 // Hier wird angegeben, an welchem digitalen Pin die WS2812 LEDs bzw. NeoPixel angeschlossen sind
#define NUMPIXELS 12 // Hier wird die Anzahl der angeschlossenen WS2812 LEDs bzw. NeoPixel angegeben
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int pos = 0;

void setup(){
 
  Serial.begin(57600);

  //ps2-Controller
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  
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
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
  case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
   }

   //Neopixel
   pixels.begin();
}

void loop(){

  pixels.setPixelColor(pos, pixels.Color(0,0,0));

  ps2x.read_gamepad();

  int x = ps2x.Analog(PSS_LX);
  int y = ps2x.Analog(PSS_LY);

  //pixels.setPixelColor(1, pixels.Color(0,255,0)); // Pixel1 leuchtet in der Farbe Grün
  //pixels.show(); // Durchführen der Pixel-Ansteuerung

  if(x > 140){

    if(pos == 11){
      pos = 0;  
    }
    else{
      pos = pos + 1;    
    } 
  }
  else if(x < 116){
    if(pos == 0){
      pos = 11;  
    }  
    else{
      pos = pos - 1;  
    }
  }

  pixels.setPixelColor(pos, pixels.Color(100,100,100));

  pixels.show();

  delay(50);
    
}
