#include <WiFi101.h>
#include <WiFiClient.h>
#include <WiFiSSLClient.h>
#include <WiFiUdp.h>
 
/*
 * This example is modified from the original file 
 * https://github.com/arduino-libraries/WiFi101/blob/master/examples/SimpleWebServerWiFi/SimpleWebServerWiFi.ino
 */
#include <SPI.h>
 
char ssid[] = "HAMKWlan";
char pass[] = "savupiipputeollisuus";
bool val = true;
 
int status = WL_IDLE_STATUS;
char server[] = "www.hamk.fi"; 
WiFiClient client;
 
void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.println("Waiting for WiFi to connect...");
  while ( status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("\nStarting connection to server...");
  if (client.connect(server, 80)) {
    getURL();
  }
}
void loop() {
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
    
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (client.connect(server, 80)) {
      Serial.println("Attempting again in 5s -------->");
      delay(5000);
      getURL();
    }
  }
}

void getURL() {
  Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /Sivut/default.aspx HTTP/1.1");
    client.println("Host: www.hamk.fi");
    client.println("Connection: close");
    client.println();
}

