#include <Servo.h> //Hinzufügen der Servobibliothek. Diese Bibliothek wird verwendet, da ein ESC aus elektrischer Sicht in gleicher Weise wie ein Servo angesteuert wird.
Servo ESC1;     // Der ESC-Controller (Electronic Speed Controller bzw. elektronischer Fahrtregler) wird als Objekt mit dem Namen "ESC" festgelegt.
Servo ESC2;
Servo ESC3;
Servo ESC4;

void setup() 
{
  // Der ESC wird im Setup genauer definiert.
  ESC1.attach(8); // Die drei Werte bedeuten Pin, minimale Pulsweite und maximale Pulsweite in Mikrosekunden. 1000 und 2000 sind dabei Standartwerte, die sich fpr die meisten Speedcontroller eignen. Wenn es mit diesen Werten nicht funktioniert, müssen die Werte des verwendeten ESC recherchiert und eingesetzt werden.
  ESC2.attach(9);
  ESC3.attach(10);
  ESC4.attach(11);
}
void loop() 
{
  ESC1.write(100);    // Der endgültige Wert für den ESC wird an den ESC gesendet. Der ESC nimmt das Signal an dieser Stelle auf und steuert den Motor entsprechend der gesendeten Werte an.
  ESC2.write(100);
  ESC3.write(100);
  ESC4.write(100);
}
