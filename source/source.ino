

#include "SPI_controller.h"
#include "WiFiCred.h"
#include <MQTTClient.h>



MQTTClient MQTTc;

char buff;

const int MISOpin = 10; // Pin number for MISO
const int tempCS = 5; // Pin number for temp. sensors chip select
const int micCS = 4; // Pin number for mics chip select
const int humidCS = 3; // Pin number for humidity sensors chip select
const int CLK = 9; // Pin number for clock 

const int relayPin = 2;
const int led = 6;

uint64_t lastMillis = 0;
uint64_t lastTempMillis = 0;
uint64_t WiFilastMillis = 0;
uint64_t currentMillis = 0;
uint64_t micLastMillis = 0;
uint64_t lastWatsonMillis = 0;
int runtime = 0;

double temp = 0;
uint16_t sLevel = 0;
int sAVG = 0;
int sHigh = 0;
int sLow = 0;
int debug_counter = 0;
bool lightState = false;
int samples = 0;
uint64_t totalSample = 0;

void setup() {
  // Initialize custom SPI
  SPI_init(MISOpin, CLK, tempCS, micCS, humidCS);
  //SPI.begin();
//  initMic();
  delay(2000);
  pinMode(relayPin, OUTPUT);
  pinMode(led, OUTPUT);

  Serial.print("Waiting for WiFi to connect");
  while ( status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(1000);
  }
  //client.connect(server, 80);

  /*
    client.begin("<Address Watson IOT>", 1883, net);
    Address Watson IOT: <WatsonIOTOrganizationID>.messaging.internetofthings.ibmcloud.com
    Example:
    client.begin("iqwckl.messaging.internetofthings.ibmcloud.com", 1883, net);
  */
  MQTTc.begin(ibm_hostname, 1883, client);  // Cut for testing without Watson

  connect();
}

void loop() {

  if (millis() - lastMillis >=1000) {
    Serial.println();
    lastMillis = millis();
    runtime++;
    Serial.print("Runtime: ");
    Serial.print(runtime);
    Serial.print("s, Mic Samples: ");
    Serial.println(samples);
    Serial.print("Sample AVG: ");
    sAVG = totalSample/samples;
    Serial.print(sAVG);
    Serial.println();
    totalSample = 0;
    samples = 0;
  }
  
  // Everything goes inside this if-statement
  if (millis() - lastTempMillis >= 2000) {
    //Serial.println("Second started...");
    lastTempMillis = millis();
    Serial.print("Reading temperature... ");
    temp = SPI_Temp_RAW(); // Get temperature data
    Serial.print("DONE (");
    Serial.print(temp);
    Serial.print(")\n");
  }

    //Serial.print("Gettin samples... ");
    if (micros()-micLastMillis >= 100) {
      sLevel = SPI_audio_RAW();
      micLastMillis = micros();
      samples++;
      totalSample += sLevel;
    }

    if (millis() - lastWatsonMillis >= 1000) {
      Serial.println("Publishing to Watson...");
    if(!MQTTc.connected()) {    // Cut for testing without Watson
    connect();                 // Cut for testing without Watson
    }                           // Cut for testing without Watson
    lastWatsonMillis = millis();
    //Cut for testing without Watson
    // for the node Red analysis it is better to have the json topics written without special marks without spaces..
    MQTTc.publish("iot-2/evt/Sensor/fmt/json", "{\"Sensor values\":\"Temperature\",\"temp\":\"" + String(temp)+"\"}");  
    Serial.println("Succeeded publishing");
    }

  /*
  if (millis() - WiFilastMillis > 5000) {
    WiFilastMillis = millis();
    digitalWrite(led, HIGH);
    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      if (status != WL_CONNECTED) {
        Serial.print("*** WIFI RECONNECTING ***");
        while ( status != WL_CONNECTED) {
          status = WiFi.begin(ssid, pass);
          Serial.print(".");
          // wait 10 seconds for connection:
          delay(1000);
        }
      }
      Serial.println("*** SERVER RECONNECTING ***");
      client.connect(server, 80);
    }
    if (client.connected()) getURL();

    Serial.print("Reading response from server... ");
    while (client.available()) {
      buff = client.read();
      //Serial.print(buff);
      debug_counter++;
      if (debug_counter > 2000) break;
    }
    Serial.println("DONE");
    
    if (buff == 'd') {
      digitalWrite(relayPin, HIGH);
      Serial.println("Light ON");
      lightState= true;
    }
    else if (buff == 'e') {
      digitalWrite(relayPin, LOW);
      Serial.println("Light OFF");
      lightState = false;
    }
    else {
      digitalWrite(relayPin, lightState);
      Serial.println("SERVER ERROR");
    }
    debug_counter = 0;
    digitalWrite(led, LOW);
  }*/
}

void connect() 
{
  Serial.print("checking WLAN...");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");       // printing a dot every half second
      WiFi.begin(ssid, pass);
      delay(2000); // Wait for WiFi to connect
      Serial.println("Connected to WLAN");
    delay(500);
  }
  
  Serial.print("\nconnecting Watson with MQTT....");
  // Cut for testing without Watson
  while (!MQTTc.connect(client_id,user_id,authToken)) 
  {
    Serial.print(".");
    delay(3000);
  }
  Serial.println("\nconnected!");
} // end of connect()
