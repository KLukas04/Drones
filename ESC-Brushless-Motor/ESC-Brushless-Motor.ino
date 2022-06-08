/*#include <Servo.h> //Hinzufügen der Servobibliothek. Diese Bibliothek wird verwendet, da ein ESC aus elektrischer Sicht in gleicher Weise wie ein Servo angesteuert wird.
Servo ESC1;     // Der ESC-Controller (Electronic Speed Controller bzw. elektronischer Fahrtregler) wird als Objekt mit dem Namen "ESC" festgelegt.
Servo ESC2;
Servo ESC3;
Servo ESC4;

int analogPin = A0;

int val = -1;

void setup() 
{

  Serial.begin(9600);
  
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

*/

#include <Servo.h> //Hinzufügen der Servobibliothek. Diese Bibliothek wird verwendet, da ein ESC aus elektrischer Sicht in gleicher Weise wie ein Servo angesteuert wird.
Servo ESC;     // Der ESC-Controller (Electronic Speed Controller bzw. elektronischer Fahrtregler) wird als Objekt mit dem Namen "ESC" festgelegt.
int Drehregler;  // Ausgabewert des Drehreglers
int Geschwindigkeit; // Das Wort "Geschwindigkeit" steht als Variable für den Ansteuerungswert am ESC.

void setup() 
{
  // Der ESC wird im Setup genauer definiert.
  ESC.attach(8,1000,2000); // Die drei Werte bedeuten Pin, minimale Pulsweite und maximale Pulsweite in Mikrosekunden. 1000 und 2000 sind dabei Standartwerte, die sich fpr die meisten Speedcontroller eignen. Wenn es mit diesen Werten nicht funktioniert, müssen die Werte des verwendeten ESC recherchiert und eingesetzt werden.
}
void loop() 
{
  Drehregler = analogRead(A0);   // Dieser Befehl liest den Wert des Potentiometers am analogen Pin A0 aus und speichert ihn unter der Variablen "Drehregler". Der Wert liegt zwischen 0 und 1023.
  Geschwindigkeit = map(Drehregler, 0, 1023, 0, 180);   // Der "MAP-" Befehl wandelt den Messwert aus der Variablen "Drehregler" um, damit er am ESC verarbeitet werden kann. Der Zahlenbereich 0 bis 1023 wird dabei in einen Zahlenwert zwischen 0 und 180 umgewandelt.
  ESC.write(Geschwindigkeit);    // Der endgültige Wert für den ESC wird an den ESC gesendet. Der ESC nimmt das Signal an dieser Stelle auf und steuert den Motor entsprechend der gesendeten Werte an.
}
