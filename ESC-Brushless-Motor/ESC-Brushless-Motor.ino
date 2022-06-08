#include <Servo.h> //Hinzufügen der Servobibliothek. Diese Bibliothek wird verwendet, da ein ESC aus elektrischer Sicht in gleicher Weise wie ein Servo angesteuert wird.
Servo ESC1;     // Der ESC-Controller (Electronic Speed Controller bzw. elektronischer Fahrtregler) wird als Objekt mit dem Namen "ESC" festgelegt.
Servo ESC2;
Servo ESC3;
Servo ESC4;

int analogPin = A0;

int val = -1;

void setup() 
{
  // Der ESC wird im Setup genauer definiert.
  ESC1.attach(8); // Die drei Werte bedeuten Pin, minimale Pulsweite und maximale Pulsweite in Mikrosekunden. 1000 und 2000 sind dabei Standartwerte, die sich fpr die meisten Speedcontroller eignen. Wenn es mit diesen Werten nicht funktioniert, müssen die Werte des verwendeten ESC recherchiert und eingesetzt werden.
}
void loop() 
{
  ESC1.write(0);

  val = analogRead(analogPin);
  Serial.println(val); 
  
  delay(5000);
  
  ESC1.write(100);

  val = analogRead(analogPin);
  Serial.println(val); 

  delay(5000);
}
