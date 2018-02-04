#include "SPI_controller.h"
#include "WiFiCred.h"
#include "Seeed_BME280.h"
#include <MQTTClient.h>
#include <Wire.h>



MQTTClient MQTTc;
BME280 bme280;

char buff;

const int MISOpin = 10; // Pin number for MISO
const int tempCS = 5; // Pin number for temp. sensors chip select
const int micCS = 4; // Pin number for mics chip select
const int humidCS = 3; // Pin number for humidity sensors chip select
const int CLK = 9; // Pin number for clock 

const int relayPin = 2; // Pin number for ULN controlling the relay switch for the lamp
const int led = 6; // Internal LED of MKR1000

uint64_t lastMillis = 0;
uint64_t lastTempMillis = 0;
uint64_t WiFilastMillis = 0;
uint64_t currentMillis = 0;
uint64_t micLastMillis = 0;
uint64_t lastWatsonMillis = 0;
int runtime = 0; // Runtime of the program in seconds

double temp = 0;
double lastTemp = 0;
uint16_t sLevel = 0; // Raw input from the microphone
int sAVG = 0; // Calculated average mic level in second
int sHigh = 0; // Highest input from microphone
int sLow = 0; // Lowest input from microphone
int sDiff = 0; // Difference between lowest and highest
bool lightState = false; // State of the lamp controlled by the relay
int samples = 0; // How many samples from the mic in second
uint64_t totalSample = 0; // Used for calculating the average mic input level

float bmeTemp, bmePress, bmeHumid; // Floating point variables for the input from BME280

void setup() {
  // Initialize custom SPI
  SPI_init(MISOpin, CLK, tempCS, micCS);

  // Initialize BME280
  if(!bme280.init()){
    Serial.println("Device error!");
  }
  
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
    sHigh = sAVG;
    sLow = sAVG;
    getBME();
  }
  
  // Everything goes inside this if-statement
  if (millis() - lastTempMillis >= 2000) {
    lastTemp = temp;
    //Serial.println("Second started...");
    lastTempMillis = millis();
    Serial.print("Reading temperature... ");
    temp = SPI_Temp_RAW(); // Get temperature data
    Serial.print("DONE (");
    Serial.print(temp);
    Serial.print(")\n");
    //if (temp-lastTemp > 10 || temp-lastTemp < -10) temp = lastTemp;
    //SPI_humid_RAW();
  }

    //Serial.print("Gettin samples... ");
    if (micros()-micLastMillis >= 10) {
      sLevel = SPI_audio_RAW();
      micLastMillis = micros();
      samples++;
      totalSample += sLevel;
      if (sLevel > sHigh) sHigh = sLevel;
      if (sLevel < sLow) sLow = sLevel;
      sDiff = sHigh - sLow;
    }

    if (millis() - lastWatsonMillis >= 1000) {
      Serial.println("Publishing to Watson...");
    if(!MQTTc.connected()) {    // Cut for testing without Watson
    connect();                 // Cut for testing without Watson
    }                          // Cut for testing without Watson
    lastWatsonMillis = millis();
    //Cut for testing without Watson
    // for the node Red analysis it is better to have the json topics written without special marks without spaces..

    //MQTTc.publish("iot-2/evt/Sensor/fmt/json", "{\"Sensor values\":\"Temperature\",\"temp\":\"" + String(temp)+"\",\"mic\":\""+String(sAVG)+"\",\"micdiff\":\""+String(sDiff)+"\"}");
    //MQTTc.publish("iot-2/evt/Sensor/fmt/json", "{\"Sensor values\":\"Temperature\",\"temp\":\"" + String(temp)+"\",\"mic\":\""+String(sAVG)+"\",\"micdiff\":\""+String(sDiff)+"\",\"pressure\":\"" + String(bmePress) + "\"}"); 
    MQTTc.publish("iot-2/evt/Sensor/fmt/json", "{\"humidity\":\"" + String(bmeHumid) + "\",\"temp\":\"" + String(temp)+"\",\"mic\":\""+String(sAVG)+"\",\"micdiff\":\""+String(sDiff)+"\",\"pressure\":\"" + String(bmePress) + "\"}"); 
    
     //MQTTc.publish("iot-2/evt/Sensor/fmt/json", "{\"Sensor values\":\"Temperature\",\"temp\":\"" + String(temp)+"\",\"mic\":\""+String(sAVG)+"\",\"micdiff\":\""+String(sDiff)+"\",\"pressure\":\"" + String(bmePress) + "\",\"kosteus\":\"" + String(bmeHumid) + "\"}");
    
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
  /*while (!MQTTc.connect(client_id,user_id,authToken)) 
  {
    Serial.print(".");
    delay(3000);
  }*/
  MQTTc.connect(client_id,user_id,authToken);
  Serial.println("\nconnected!");
} // end of connect()

void getBME() {
  bmeTemp = bme280.getTemperature();
  bmePress = bme280.getPressure();
  bmeHumid = bme280.getHumidity();
}

