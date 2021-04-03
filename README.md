# luftwaechter 2.0
Im Original von repair-frank: CO2-Temperatur-Feuchtigkeits-Sensor mit ESP32 und MH-Z19B, DHT22 und WLAN-Funktionalität. Ich habe das ganze noch etwas erweitert und modifiziert:
## Aufbau CO2 Monitor
[Reichelt](https://www.reichelt.de/magazin/reichelt-magazin/co2-messgeraet-einfach-und-guenstig-selber-bauen/) hat eine etwas ausführlichere Anleitung parat.

Kurzform:
### Benötigt werden
| Teile             | Werkzeug          |
| ----------------- | ----------------- |
| NodeMCU ESP32     | Lötkolben         |
| MH-Z19C           | Lötzinn           |
| DHT22             | Pinzette          |
| LCD 16x2          | Platinenhalter    |
| IIC/I2C Interface | PC                |
| LED rot           |
| LED gelb          |
| LED grün          |
| Widerstand 4,7k   |
| 3x Widerstand 220 |
| Lochrasterplatine |
| Kleingehäuse      |
| Micro USB 5V. 1A  |
| Kupferlitze iso.  |
| Kupferschaltdraht |
| Buchsen & Stifte  |
| Abstandhalter     |
| Schrauben         |
### Hardware
Ich nutze die Entwicklungsplatine NodeMCU mit ESP32 von Joy-IT, diese besitzt 30 Pins (siehe Schaltskizze). Bei anderen Platinen ist selbstverständlich eine Prüfung und Anpassung der Pins notwendig! Der CO2 Sensor besitzt eine AUTOCALIBRATION, er nutzt dabei den tiefsten gemessenen Wert aus den letzten 24 Stunden als Grundwert (400ppm) und gleicht diesen auch mit Werten aus vorherigen Wochen ab. Daher ist es ratsam einmal am Tag dem Sensor eine ordentliche Portion Frischluft zuzuführen.

ESP32 und Sensoren auf der Platine anordnen, Abstand wegen Abwärme vom ESP32 berücksichtigen (3cm sind ein gutes Maß). Verdrahtung gemäß Schaltskizze durchführen. Eventuell Trennstellen mit Stiftleiste und Buchse vorsehen, um beispielsweise die Gehäuseoberseite inklusive Display entfernen zu können.

### Software (je nach ESP32 Board teilweise unterschiedlich)
1. [Arduino IDE installieren](https://www.arduino.cc/en/software)
2. Unter Datei -> Voreinstellungen die URLs `https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json` unter zusätzliche Boardverwalter-URLs eintragen und unter Werkzeuge -> Bibliotheken verwalten `esp32`von `Espressif Systems` installieren.
4. Als nächstes aktualisierten [CP210x USB-UART Treiber](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers) für das passende Betriebssystem herunterladen und installieren. 
5. Offizielle [ESP32 Bibliothek](https://github.com/espressif/arduino-esp32) herunterladen, entpacken in `~/Arduino/hardware`
6. `libraries.zip` herunterladen und entpacken in `~/Arduino/libraries`
7. Neustart Arduino IDE, unter Werkzeuge -> Board muss `ESP32 Dev Module` ausgewählt werden, Baudrate `115200`.
### Arduino IDE
1. ESP32 an PC anschließen, neues sketch erstellen, Code einfügen, Anpassungen durchführen (Wifi, Thingspeak, Display), überprüfen und hochladen
2. Ausgabe über seriellen Monitor beachten
### Messen
Aufstellungsort sollte nicht direkt im Durchzug und nicht zu nah an Störquellen sein. Frohes Messen!

## Anbindung an Thingspeak
Ich nutze dazu einen kostenlosen Account (vier Channel möglich, einer reicht für das Messgerät)
Nach der Anmeldung muss ein neuer Channel angelegt werden. Es macht Sinn gleich der Messgrößen entsprechende Fields anzulegen und diese entsprechend zu benennen (Field 1: Temperatur, Field 2: Feuchtigkeit, Field 3: CO2, Field 4: CO2 Sensor Temperatur), auch der Name des Channels selbst sollte befüllt werden. Unter Api Keys kann die URL unter Write a Channel Feed gefunden werden und im Code ersetzt werden. Die entsprechenden Daten werden den Fields mittels der URL alle 60 Sekunden übertragen, ein kürzerer Intervall ist in der kostenlosen Variante von Thingspeak nicht zu empfehlen.

## Ausgabe der Messwerte im Display
Ausgabe der aktuellen Messung (CO2 und Temperatur) über Display
### Library einrichten
Als erstes ist die Library [LiquidCrystal_I2C](https://github.com/marcoschwartz/LiquidCrystal_I2C/archive/master.zip) herunterzuladen und zu entpacken (Archiv vom 31/03/2021 ist im Git enthalten). Das Master-Verzeichnis umbenennen in `LiquidCrystal_I2C` und in `~/Arduino/Library` packen und anschließend Arduino IDE neustarten.

### I2C Adresse finden
Um die korrekte Adresse des Displays zu erhalten folgende sketch auf den ESP32 hochladen

```
/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <Wire.h>
 
void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}
 
void loop() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);       
}
```
Im seriellen Monitor (Baudrate 115200) sollte jetzt die Adresse `I2C device fount at adress 0x27` angezeigt werden. Diese im Code bei Bedarf anpassen.

Sollte mehr Text angezeigt werden, kann eine Scroll-Funktion integriert werden. Aufzuufen ist diese mit `scrollText(1, messageToScroll, 250, lcdColumns);`
```
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message; 
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}
```
Als Übergabeparameter dienen row (0 oder 1), message (Text der Zeile), delayTime (Verbleib in ms auf einer Zelle des Displays) und lcdColums (Anzahl Spalten des LCD, bereits im Code definiert).

## Anzeige der aktuellen CO2 Lage anhand einer Ampel
Leds sind schon in der Schaltskizze eingebaut. Programmiertechnisch ist das letzte Wort noch nicht gesprochen.

## Erweiterungen
- Das Display ist recht hell, vielleicht lässt es sich noch etwas dimmen oder abschalten wenn nicht benötigt
- Momentan läuft der CO2 Sensor auf AUTOCALIBRATION, ein händisches kalibrieren ist auch möglich und könnte zusätzlich angeboten werden

## nützliche Links
- [MH-Z19](https://github.com/WifWaf/MH-Z19) Library von Jonathan Dempsey
- Kein Unsinn aber viel Text und Wissen bei der [Unsinnbasis](https://unsinnsbasis.de/co2-sensor-mhz19b)
- [Reichelt](https://www.reichelt.de/magazin/reichelt-magazin/co2-messgeraet-einfach-und-guenstig-selber-bauen/)

