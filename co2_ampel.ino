#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "MHZ19.h"                                        
#include "SoftwareSerial.h"                                    
#include "DHTesp.h"
#include "HTTPClient.h"                                   // Thingspeak
#include "LiquidCrystal_I2C.h"                            // 1602 Display


// Wifi Setup
const char* ssid =  "<SSID>";
const char* password =  "<Passwort>";
 
// CO2 Sensor Setup serielle Schnittstelle
#define RX_PIN 33                                          // Verbindung des RX Pin vom MHZ19 zum ESP32
#define TX_PIN 32                                          // Verbindung des TX Pin vom MHZ19 zum ESP32
#define BAUDRATE 9600                                      // Baudrate des MH-Z19

// dht22 Sensor Steup
#define dhtPin 4                                            // Verbindung des Data-Pin des DHT22 mit dem ESP32

// Ampel Setup
#define LED_RED 25
#define LED_YELLOW 26
#define LED_GREEN 27
#define LIMIT_GREEN 1000
#define LIMIT_YELLOW 1200

// Systeme initialisieren
AsyncWebServer server(80);                                 // Standardport für Ausgabe über IP
DHTesp dht;                                                // dht22 definieren
MHZ19 myMHZ19;                                             // MHZ19 definieren
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // serielle Schnittstelle konfigurieren

//  Thingspeak Setup
String serverName = "https://api.thingspeak.com/update?api_key=<Thingspeak Write API KEY>";

// 1602 Display Setup
int lcdColumns = 16;                                        // Anzahl Spalten
int lcdRows = 2;                                            // Anzahl Zeilen
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);           // LCD Adresse, Anzahl Spalten und Zeilen

void setup()
{
    // Ampel: Pins als Ausgang festlegen
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    // alle LEDs ausschalten
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    // Startsequenz der LEDs
    digitalWrite(LED_GREEN, HIGH);
    delay(250);
    digitalWrite(LED_YELLOW, HIGH);
    delay(250);
    digitalWrite(LED_RED, HIGH);
    delay(500);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    
    // 1602 Display
    lcd.init();                                             // LCD inizialisieren
    lcd.backlight();                                        // LCD backlight anmachen

    // Systemkonfiguration
    Serial.begin(115200);                                   // Baudrate serieller Monitor
    dht.setup(dhtPin, DHTesp::DHT22);                       // dht22 konfigurieren
    mySerial.begin(BAUDRATE);                               // Serielle Schnittstelle staren
    myMHZ19.begin(mySerial);                                
    
    // Kalibriern des CO2 Sensors
    myMHZ19.autoCalibration(false);                         // automatisch kalibrieren AUS [AN: autoCalibration()]
    Serial.println("Kalibrieren..");                        // Wenn autoCalibration AN kann folgendes raus!
    lcd.clear();                                            // LCD leeren
    lcd.setCursor(0, 0);                                    // Cursor auf erste Spalte, erste Reihe 
    lcd.print(String("Kalibriermodus:"));                   // Ausgabe auf LCD
    lcd.setCursor(0, 1);                                    // Cursor auf erste Spalte, zweite Reihe 
    lcd.print(String("Fenster ""\xEF""ffnen"));             // Ausgabe auf LCD
    delay(120000);                                          // 2 Minuten warten auf frische Luft (20 wären besser!)
    myMHZ19.calibrate();                                    // Kalibrierung des "0" Punktes
    lcd.clear();                                            // LCD leeren
    lcd.setCursor(0, 0);                                    // Cursor auf erste Spalte, erste Reihe 
    lcd.print(String("Kalibriermodus:"));                   // Ausgabe auf LCD
    lcd.setCursor(0, 1);                                    // Cursor auf erste Spalte, zweite Reihe 
    lcd.print(String("beendet"));                           // Ausgabe auf LCD
    delay(5000);                
    
    // Wifi Setup und Verbindung 
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      //WiFi.begin(ssid, password);
      delay(1000);
      Serial.println("Verbinden mit Wifi...");
     }
     Serial.println(WiFi.localIP());                         // Ausgabe der IP-Adr. über seriellen Monitor
     lcd.clear();                                            // LCD leeren
     lcd.setCursor(0, 0);                                    // Cursor auf erste Spalte, erste Reihe 
     lcd.print(String("IP:"));                               // Ausgabe "IP:" auf LCD
     lcd.setCursor(0, 1);                                    // Cursor auf erste Spalte, zweite Reihe 
     lcd.print(String(WiFi.localIP().toString()));           // Ausgabe IP auf LCD

     // Ausgabe über IP vorbereiten
     server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) { //temperature Route
       float temperature = dht.getTemperature();
       request->send(200, "text/plain", String(temperature) + " C");
     });
     
     server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {    //humidity Route
       float humidity = dht.getHumidity();
       request->send(200, "text/plain", String(humidity) + " %");
     });
    
     server.on("/co2", HTTP_GET, [](AsyncWebServerRequest * request) {         //co2 Route 
       int co2 = myMHZ19.getCO2();
       request->send(200, "text/plain", String(co2) + " ppm");
     });
     server.begin();                        // Start des Servers
}

// Variable für loop festlegen
unsigned long getDataTimer = 0;
    
void loop()
{

  if (millis() - getDataTimer >= 60000)
    {
      float temperature = dht.getTemperature();
      float humidity = dht.getHumidity();
      Serial.print("Temperatur: ");
      Serial.print(temperature);
      Serial.print(" °C  Feuchtigkeit: ");
      Serial.println(humidity);
      int  CO2 = myMHZ19.getCO2();                            // Request CO2 (as ppm)
      Serial.print("CO2 (ppm): ");                      
      Serial.print(CO2);                                
      int8_t Temp = myMHZ19.getTemperature();                 // Request Temperature (as Celsius)
      Serial.print("  Temperatur CO2Sens (°C): ");                  
      Serial.println(Temp);
      // Daten an Thingspeak senden
      sendData(temperature, humidity, CO2, Temp); 
      // Ausgabe Daten Display
      lcd.clear();                                            // LCD leeren
      lcd.setCursor(0, 0);                                    // Cursor auf erste Spalte, erste Reihe 
      lcd.print(String("CO2: ") + String(CO2) + String("ppm"));  // Ausgabe Daten CO2
      lcd.setCursor(0,1);                                     // Cursor auf auf erste Spalte, zweite Reihe
      lcd.print(String("Temp: ") + String(temperature) + String("\337C")); // Ausgabe Daten dht22
                              
      getDataTimer = millis();

      // Ampel je nach Messwert schalten
      if (CO2 < LIMIT_GREEN) {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
      } else if (CO2 < LIMIT_YELLOW) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_RED, LOW);
      } else {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, HIGH);
      }
    }
}

// function Daten zu Thingspeak senden
void sendData(float temp, float hum, int co2, int8_t co2temp){
  HTTPClient http; // Initialisieren des HTTP client
  String url = serverName + "&field1=" + temp + "&field2=" + hum + "&field3=" + co2 + "&field4=" + co2temp; // URL erzeugen
  http.begin(url.c_str()); // Initialisieren der HTTP Anfrage
  int httpResponseCode = http.GET(); // Senden
  if (httpResponseCode > 0){ // Antwort abfragen und verarbeiten
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }else{
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
