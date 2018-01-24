#include "SPI_controller.h"

const int MISOpin = 10; // Pin number for MISO
const int tempCS = 5; // Pin number for temp. sensors chip select
const int micCS = 4; // Pin number for mics chip select
const int humidCS = 3; // Pin number for humidity sensors chip select
const int CLK = 9; // Pin number for clock 

void setup() {
  SPI_init(MISOpin, CLK, tempCS, micCS, humidCS);
}

void loop() {
  SPI_Temp_RAW();
}
