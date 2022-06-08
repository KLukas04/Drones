#include <Servo.h> //Hinzufügen der Servobibliothek. Diese Bibliothek wird verwendet, da ein ESC aus elektrischer Sicht in gleicher Weise wie ein Servo angesteuert wird.
Servo ESC;     // Der ESC-Controller (Electronic Speed Controller bzw. elektronischer Fahrtregler) wird als Objekt mit dem Namen "ESC" festgelegt.

void setup() 
{
  // Der ESC wird im Setup genauer definiert.
  ESC.attach(8,1000,2000); // Die drei Werte bedeuten Pin, minimale Pulsweite und maximale Pulsweite in Mikrosekunden. 1000 und 2000 sind dabei Standartwerte, die sich fpr die meisten Speedcontroller eignen. Wenn es mit diesen Werten nicht funktioniert, müssen die Werte des verwendeten ESC recherchiert und eingesetzt werden.
}
void loop() 
{
  ESC.write(100);    // Der endgültige Wert für den ESC wird an den ESC gesendet. Der ESC nimmt das Signal an dieser Stelle auf und steuert den Motor entsprechend der gesendeten Werte an.
}
