#include "SPI_controller.h"

const int MISOpin = 10; // Pin number for MISO
const int tempCS = 5; // Pin number for temp. sensors chip select
const int micCS = 4; // Pin number for mics chip select
const int humidCS = 3; // Pin number for humidity sensors chip select
const int CLK = 9; // Pin number for clock 

uint64_t lastMillis = 0;
uint64_t currentMillis = 0;


void setup() {
  // Initialize custom SPI
  SPI_init(MISOpin, CLK, tempCS, micCS, humidCS);
}

void loop() {
  currentMillis = millis();

  // Everything goes inside this if-statement
  if (currentMillis - lastMillis >= 1000) {
    SPI_Temp_RAW(); // Get temperature data
  }
}
