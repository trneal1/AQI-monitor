/*
 *  Json parametric GET REST response
  *  by Mischianti Renzo <https://www.mischianti.org>
 *
 *  https://www.mischianti.org/
 *
 */
 
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Wire.h>
#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
//#define CCS811_ADDR 0x5B //Default I2C Address
#define CCS811_ADDR 0x5A //Alternate I2C Address

CCS811 mySensor(CCS811_ADDR);

#include <ArduinoOTA.h>
 
const char* ssid = "TRNNET-2G";
const char* password = "ripcord1";
const char *hostname="ESP_AQI";
 
ESP8266WebServer server(80);
 
// Serving Hello world
void getHelloWord() {
    server.send(200, "text/json", "{\"name\": \"Hello world\"}");
}

void getaqi(){

  String message;
   

    if (mySensor.dataAvailable())
  {
    //If so, have the sensor read and calculate the results.
    //Get them later
    mySensor.readAlgorithmResults();

    message+=mySensor.getCO2();
    message+="\t";
    message+=mySensor.getTVOC();
    message+="\n";
    server.send(200, "text/plain", message);
  }

}

void getSettings() {
    String response = "{";
 
    response+= "\"ip\": \""+WiFi.localIP().toString()+"\"";
    response+= ",\"gw\": \""+WiFi.gatewayIP().toString()+"\"";
    response+= ",\"nm\": \""+WiFi.subnetMask().toString()+"\"";
 
    if (server.arg("signalStrength")== "true"){
        response+= ",\"signalStrengh\": \""+String(WiFi.RSSI())+"\"";
    }
 
    if (server.arg("chipInfo")== "true"){
        response+= ",\"chipId\": \""+String(ESP.getChipId())+"\"";
        response+= ",\"flashChipId\": \""+String(ESP.getFlashChipId())+"\"";
        response+= ",\"flashChipSize\": \""+String(ESP.getFlashChipSize())+"\"";
        response+= ",\"flashChipRealSize\": \""+String(ESP.getFlashChipRealSize())+"\"";
    }
    if (server.arg("freeHeap")== "true"){
        response+= ",\"freeHeap\": \""+String(ESP.getFreeHeap())+"\"";
    }
    response+="}";
 
    server.send(200, "text/json", response);
}
 
// Define routing
void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html",
            F("Welcome to the REST Web Server"));
    });
    server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
    server.on(F("/settings"), HTTP_GET, getSettings);
    server.on(F("/aqi"),HTTP_GET,getaqi);
}
 
// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
 
void setup(void) {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");

  }

  pinMode(D3,OUTPUT);
  digitalWrite(D3,LOW);

  Wire.begin(); //Inialize I2C Hardware

  if (mySensor.begin() == false)
  {
    Serial.print("CCS811 error. Please check wiring. Freezing...");
    while (1)
      ;
  }
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");

  ArduinoOTA.begin();
}
 
void loop(void) {
  server.handleClient();
  MDNS.update();

  ArduinoOTA.handle();
}