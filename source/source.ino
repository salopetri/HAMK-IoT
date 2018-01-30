#include <WiFi101.h>
#include <WiFiClient.h>
#include <WiFiSSLClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include "SPI_controller.h"
#include "WiFiCred.h"



bool val = true;
int status = WL_IDLE_STATUS;

WiFiClient client;
char buff;

const int MISOpin = 10; // Pin number for MISO
const int tempCS = 5; // Pin number for temp. sensors chip select
const int micCS = 4; // Pin number for mics chip select
const int humidCS = 3; // Pin number for humidity sensors chip select
const int CLK = 9; // Pin number for clock 

const int relayPin = 2;
const int led = 6;

uint64_t lastMillis = 0;
uint64_t WiFilastMillis = 0;
uint64_t currentMillis = 0;
uint64_t micLastMillis = 0;

double temp = 0;
uint16_t sLevel = 0;
int sAVG = 0;
int sHigh = 0;
int sLow = 0;
int debug_counter = 0;
bool lightState = false;

void setup() {
  // Initialize custom SPI
  SPI_init(MISOpin, CLK, tempCS, micCS, humidCS);
  //SPI.begin();
//  initMic();
  delay(2000);
  pinMode(relayPin, OUTPUT);
  pinMode(led, OUTPUT);

  //Serial.print("Waiting for WiFi to connect");
  while ( status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    //Serial.print(".");
    // wait 10 seconds for connection:
    delay(1000);
  }
}

void loop() {
  // Everything goes inside this if-statement
  if (millis() - lastMillis >= 2000) {
    //Serial.println("Second started...");
    lastMillis = millis();
    //Serial.print("Reading temperature... ");
    temp = SPI_Temp_RAW(); // Get temperature data
    //Serial.print("DONE (");
    //Serial.print(temp);
    //Serial.print(")\n");
  }

    //Serial.print("Gettin samples... ");
    if (millis()-micLastMillis > 1) {
      sLevel = SPI_audio_RAW();
      micLastMillis = millis();
    }
  
  if (millis() - WiFilastMillis > 1000) {
    WiFilastMillis = millis();
    digitalWrite(led, HIGH);
    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      if (status != WL_CONNECTED) {
        //Serial.print("*** WIFI RECONNECTING ***");
        while ( status != WL_CONNECTED) {
          status = WiFi.begin(ssid, pass);
          //Serial.print(".");
          // wait 10 seconds for connection:
          delay(1000);
        }
      }
      //Serial.println("*** SERVER RECONNECTING ***");
      client.connect(server, 80);
    }
    if (client.connected() getURL();

    //Serial.print("Reading response from server... ");
    while (client.available()) {
      buff = client.read();
      //Serial.print(buff);
      debug_counter++;
      if (debug_counter > 2000) break;
    }
    //Serial.println("DONE");
    
    if (buff == 'd') {
      digitalWrite(relayPin, HIGH);
      //Serial.println("Light ON");
      lightState= true;
    }
    else if (buff == 'e') {
      digitalWrite(relayPin, LOW);
      //Serial.println("Light OFF");
      lightState = false;
    }
    else {
      digitalWrite(relayPin, lightState);
      //Serial.println("SERVER ERROR");
    }
    debug_counter = 0;
    digitalWrite(led, LOW);
  }
}
