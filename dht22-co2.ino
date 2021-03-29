#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "MHZ19.h"                                        
#include "SoftwareSerial.h"                                    
#include "DHTesp.h"

const char*     ssid =  "your-ssid";
const char* password =  "your-password";
 

#define RX_PIN 33                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 32                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)

#define dhtPin 4                                            // verbunden mit Data-Pin des DHT22


AsyncWebServer server(80);                                 // http port 80
DHTesp dht;
MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // create device to MH-Z19 serial


void setup()
{
    Serial.begin(115200);                                   // Datenrate serieller Monitor
   
    dht.setup(dhtPin, DHTesp::DHT22);                       // am dhtPin ist ein DHT22 angeschlossen
    mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start   
    myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 

    myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

    WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
   }
 
  Serial.println(WiFi.localIP());                           // Ausgabe der IP-Adr. über seriellen Monitor
 

 
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

  if (millis() - getDataTimer >= 2000)
    {
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
  Serial.print("Temperatur: ");
  Serial.print(temperature);
  Serial.print(" °C  Feuchtigkeit: ");
  Serial.println(humidity);
  int  CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
  Serial.print("CO2 (ppm): ");                      
  Serial.print(CO2);                                
  int8_t Temp = myMHZ19.getTemperature();                 // Request Temperature (as Celsius)
  Serial.print("  Temperatur CO2Sens (°C): ");                  
  Serial.println(Temp);                               
  getDataTimer = millis();
    }
}
