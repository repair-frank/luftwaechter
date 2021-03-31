# luftwaechter 2.0

Im Original von repair-frank als reine Messaperatur mit Ausgabe über eigenen Webserver
"CO2-Temperatur-Feuchtigkeits-Sensor mit ESP32 und MH-Z19B, DHT22 und WLAN-Funktionalität

libraries.zip herunterladen und entpacken in ~/Arduino/libraries"

Ich habe das ganze noch etwas erweitert und modifiziert:

## Aufbau CO2 Monitor
## Anbindung an Thingspeak
## Ausgabe der Messwerte im Display
## Anzeige der aktuellen CO2 Lage anhand einer Ampel 

## Aufbau CO2 Monitor

## Anbindung an Thingspeak
Ich nutze dazu einen kostenlosen Account (vier Channel möglich, einer reicht für das Messgerät)
Nach der Anmeldung muss ein neuer Channel angelegt werden. Es macht Sinn gleich der Messgrößen entsprechende Fields anzulegen und diese entsprechend zu benennen (Field 1: Temperatur, Field 2: Feuchtigkeit, Field 3: CO2, Field 4: CO2 Sensor Temperatur), auch der Name des Channels selbst sollte befüllt werden. Unter Api Keys kann die URL unter Write a Channel Feed gefunden werden und im Code ersetzt werden. Die entsprechenden Daten werden den Fields mittels der URL alle 60 Sekunden übertragen, ein kürzerer Intervall ist in der kostenlosen Variante von Thingspeak nicht zu empfehlen.

## Ausgabe der Messwerte im Display

### Library einrichten
Als erstes ist die Library [LiquidCrystal_I2C](https://github.com/marcoschwartz/LiquidCrystal_I2C/archive/master.zip) herunterzuladen und zu entpacken. Das Master-Verzeichnis umbenennen in "LiquidCrystal_I2C" und in das Library Verzeichnis der Arduino IDE packen und anschließend Arduino IDE neustarten.

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
