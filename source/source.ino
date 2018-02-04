#include "SPI_controller.h" // Custom made library handling SPI connection with sensors
#include "WiFiCred.h" // This header includes needed credentials for WiFi, Watson and HTTP connection
#include "Seeed_BME280.h" // Library handling BME280 sensor
#include <MQTTClient.h> // Library for MQTT connection with IBM Watson
#include <Wire.h> // Library for the IC2 connection with BME280


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
  // Initialize serial for debugging messages
  Serial.begin(9600);
  
  // Initialize custom SPI
  SPI_init(MISOpin, CLK, tempCS, micCS);

  // Initialize BME280
  if(!bme280.init()){
    Serial.println("Device error!");
  }
  
  delay(2000);
  pinMode(relayPin, OUTPUT);
  pinMode(led, OUTPUT);

  Serial.print("Connecting to WiFi");
  while ( status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(1000);
  }
  
  // HTTP connection for light controlling server
  //client.connect(server, 80);

  // Begin MQTT connection with IBM Watson
  MQTTc.begin(ibm_hostname, 1883, client);
  watsonConnect();
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
  
    if (millis() - lastTempMillis >= 2000) {
      lastTemp = temp;
      lastTempMillis = millis();
      Serial.print("Reading temperature... ");
      temp = SPI_Temp(); // Get temperature data
      Serial.print("DONE (");
      Serial.print(temp);
      Serial.print(")\n");
      //if (temp-lastTemp > 10 || temp-lastTemp < -10) temp = lastTemp;
    }

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
        watsonConnect();                 // Cut for testing without Watson
      }                          // Cut for testing without Watson
      lastWatsonMillis = millis();
      MQTTc.publish("iot-2/evt/Sensor/fmt/json", "{\"humidity\":\"" + String(bmeHumid) + "\",\"temp\":\"" + String(temp)+"\",\"mic\":\""+String(sAVG)+"\",\"micdiff\":\""+String(sDiff)+"\",\"pressure\":\"" + String(bmePress) + "\"}"); 
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

void watsonConnect() 
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
}

/*  Function getting the values from BME280 using IC2 interface
 * 
 */
void getBME() {
  bmeTemp = bme280.getTemperature();
  bmePress = bme280.getPressure();
  bmeHumid = bme280.getHumidity();
}

