#include "SPI_controller.h"

const int MISO = 0; // Pin number for MISO
const int tempCS = 0; // Pin number for temp. sensors chip select
const int micCS = 0; // Pin number for mics chip select
const int humidCS = 0; // Pin number for humidity sensors chip select
const int CLK = 0; // Pin number for clock 

void setup() {
  SPI_init(MISO, CLK, tempCS, micCS, humidCS);
}

void loop() {
  digitalWrite();
  SPI_Temp_RAW(MISO, );
}
