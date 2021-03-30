# luftwaechter

Im Original von repair-frank als reine Messaperatur mit Ausgabe über eigenen Webserver
"CO2-Temperatur-Feuchtigkeits-Sensor mit ESP32 und MH-Z19B, DHT22 und WLAN-Funktionalität

libraries.zip herunterladen und entpacken in ~/Arduino/libraries"

Ich habe das ganze noch etwas erweitert und modifiziert:

1) Anbindung an Thingspeak
2) Ausgabe der Messwerte im Display
3) Anzeige der aktuellen CO2 Lage anhand einer Ampel 


1) Anbindung an Thingspeak

Ich nutze dazu einen kostenlosen Account (vier Channel möglich, einer reicht für das Messgerät)
Nach der Anmeldung muss ein neuer Channel angelegt werden. Es macht Sinn gleich der Messgrößen entsprechende Fields anzulegen und diese entsprechend zu benennen (Field 1: Temperatur, Field 2: Feuchtigkeit, Field 3: CO2, Field 4: CO2 Sensor Temperatur), auch der Name des Channels selbst sollte befüllt werden. Unter Api Keys kann die URL unter Write a Channel Feed gefunden werden und im Code ersetzt werden. Die entsprechenden Daten werden den Fields mittels der URL alle 60 Sekunden übertragen, ein kürzerer Intervall ist in der kostenlosen Variante von Thingspeak nicht zu empfehlen.

2) Ausgabe der Messwerte im Display

Als erstes ist die Library [LiquidCrystal_I2C](https://github.com/marcoschwartz/LiquidCrystal_I2C/archive/master.zip) herunterzuladen und zu entpacken. Das Master-Verzeichnis umbenennen in "LiquidCrystal_I2C" und in das Library Verzeichnis der Arduino IDE packen.
