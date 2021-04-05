#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "MHZ19.h"                                        
#include "SoftwareSerial.h"                                    
#include "DHTesp.h"
#include "HTTPClient.h"                                   // Thingspeak
#include "LiquidCrystal_I2C.h"                            // 1602 Display


// Wifi Setup
const char* ssid =  "<ssid>";
const char* password =  "<passwort>";
 
// CO2 Sensor Setup
#define RX_PIN 33                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 32                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)

// Temp&Humidity Sensor Setup
#define dhtPin 4                                            // verbunden mit Data-Pin des DHT22

// Ampel Setup
#define LED_RED 25
#define LED_YELLOW 26
#define LED_GREEN 27
#define LIMIT_GREEN 1000
#define LIMIT_YELLOW 1200

AsyncWebServer server(80);                                 // http port 80
DHTesp dht;
MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // create device to MH-Z19 serial

//  Thingspeak Setup
String serverName = "https://api.thingspeak.com/update?api_key=<thingspeak_write_api_key>";

// 1602 Display Setup
int lcdColumns = 16;
int lcdRows = 2;
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
    lcd.init();                                             // initialize LCD
    lcd.backlight();                                        // turn on LCD backlight
    
    Serial.begin(115200);                                   // Datenrate serieller Monitor
   
    dht.setup(dhtPin, DHTesp::DHT22);                       // am dhtPin ist ein DHT22 angeschlossen
    mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start   
    myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 

    myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

    WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //WiFi.begin(ssid, password);
    delay(1000);
    Serial.println("Connecting to WiFi..");
   }
 
  Serial.println(WiFi.localIP());                         // Ausgabe der IP-Adr. über seriellen Monitor
  lcd.clear();                                            // LCD leeren
  lcd.setCursor(0, 0);                                    // Cursor auf erste Spalte, erste Reihe 
  lcd.print(String("IP:"));                               // Ausgabe "IP:" auf LCD
  lcd.setCursor(0, 1);                                    // Cursor auf erste Spalte, zweite Reihe 
  lcd.print(String(WiFi.localIP().toString()));           // Ausgabe IP auf LCD
   
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
  
  sendData(temperature, humidity, CO2, Temp);             // Call the sendData function defined below for Thingspeak  

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

void sendData(float temp, float hum, int co2, int8_t co2temp){
  HTTPClient http; // Initialize our HTTP client

  String url = serverName + "&field1=" + temp + "&field2=" + hum + "&field3=" + co2 + "&field4=" + co2temp; // Define our entire url
      
  http.begin(url.c_str()); // Initialize our HTTP request
      
  int httpResponseCode = http.GET(); // Send HTTP request
      
  if (httpResponseCode > 0){ // Check for good HTTP status code
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }else{
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
