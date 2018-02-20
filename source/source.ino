#include "SPI_controller.h" // Custom library controlling software SPI
#include "Seeed_BME280.h" // Library handling BME280 sensor https://github.com/Seeed-Studio/Grove_BME280.git
#include <MQTTClient.h> // Library for MQTT connection with IBM Watson
#include <Wire.h> // Library for the IC2 connection with BME280
#include <WiFi101.h> // Library for WiFi connection
#include <WiFiClient.h> // Library for WiFi connection
#include "Secrets.h" // This header includes needed credentials for WiFi, Watson and HTTP connection
/*
 *  It should contain the following:
 *  char ssid[] = "XXXXXXXX";
 *  char pass[] = "XXXXXXXXX";
 *  char *client_id = "d:XXXXXX:XXXXXX:XXXX";  // Your IBM Watson organisation, device type, device ID
 *  char *user_id = "use-token-auth";   // telling that authentication will be done with token
 *  char *authToken = "XXXXXXXXXXXXX"; // Your IBM Watson Authentication Token for that particular device ID
 *  char *ibm_hostname = “your-org-id.messaging.internetofthings.ibmcloud.com”;
 */


MQTTClient MQTTc;
BME280 bme280;
WiFiClient client;

bool val = true;
int status = WL_IDLE_STATUS;

const int MISOpin = 10; // Pin number for MISO
const int tempCS = 5; // Pin number for temp. sensors chip select
const int micCS = 4; // Pin number for mics chip select
const int CLK = 9; // Pin number for clock 

const int relayPin = 2; // Pin number for ULN controlling the relay switch for the lamp
const int led = 6; // Internal LED of MKR1000

uint64_t lastMillis = 0;
uint64_t lastTempMillis = 0;
uint64_t WiFilastMillis = 0;
uint64_t currentMillis = 0;
uint64_t micLastMicros = 0;
uint64_t lastWatsonMillis = 0;
uint64_t lastLCMillis = 0;
int runtime = 0; // Runtime of the program in seconds

double temp = 0;
double tempAVG = 0;
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

// Used to count variation in mic input during 10 seconds cycle
int LC_micTotal = 0;
int LC_micAVG = 0;

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
      Serial.print("LCmicAVG: ");
      sAVG = totalSample/samples;
      Serial.print(LC_micAVG);
      Serial.println();
      totalSample = 0;
      samples = 0;
      sHigh = sAVG;
      sLow = sAVG;
      getBME();
      LC_micTotal += sDiff;
      Serial.print("LighState: ");
      Serial.println(lightState);
      
    }
  
    if (millis() - lastTempMillis >= 1000) {
      lastTemp = temp;
      lastTempMillis = millis();
      Serial.print("Reading temperature... ");
      temp = SPI_Temp(); // Get temperature data from PmodTC1
      Serial.print("DONE (");
      Serial.print(temp);
      Serial.print(")\n");
    }

    if (micros()-micLastMicros >= 100) {
      sLevel = SPI_audio_RAW();
      micLastMicros = micros();
      samples++;
      totalSample += sLevel;
      if (sLevel > sHigh) sHigh = sLevel;
      if (sLevel < sLow) sLow = sLevel;
      sDiff = sHigh - sLow;
    }

    if (millis() - lastWatsonMillis >= 1000) {
      lastTemp = tempAVG;
      tempAVG = (bmeTemp+temp)/2;
      if (bmeTemp-temp > 10 || temp-bmeTemp > 10 && lastWatsonMillis != 0) { // If difference between temperature sensors is over 10 degrees we use the BME280 temperature
        temp = bmeTemp;
      }
      
      Serial.println("Publishing to Watson...");
      if(!MQTTc.connected()) {    // Cut for testing without Watson
        watsonConnect();                 // Cut for testing without Watson
      }                          // Cut for testing without Watson
      lastWatsonMillis = millis();
      MQTTc.publish("iot-2/evt/Sensor/fmt/json", "{\"humidity\":\"" + String(bmeHumid) + "\",\"temp\":\"" + String(temp)+"\",\"lamp\":\""+String(lightState)+"\",\"micdiff\":\""+String(sDiff)+"\",\"pressure\":\"" + String(bmePress) + "\"}"); 
    }

    // Check the soundlevels and control the relay every 10 seconds
    if (millis()-lastLCMillis >= 10000) {
      LC_micAVG = LC_micTotal/10;
      LC_micTotal = 0;
      if (LC_micAVG > 100) { // The variation in microphone input seems to be always over 100 when there are people working in the room
        digitalWrite(relayPin, HIGH);
        lightState = true;
      }
      else {
        digitalWrite(relayPin, LOW);
        lightState = false;
      }
      lastLCMillis = millis();
    }
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

/*  Function getting the values from BME280 using I^2C interface
 * 
 */
void getBME() {
  bmeTemp = bme280.getTemperature();
  bmePress = bme280.getPressure();
  bmeHumid = bme280.getHumidity();
}

