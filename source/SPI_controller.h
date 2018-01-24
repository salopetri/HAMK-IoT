/*
*   THIS IS THE LIBRARY CONTAINING FUNCTIONS FOR SPI COMMUNICATIONS WITH THE SENSORS
*
*/
const unsigned int SPI_Temp_Delay = 10;

uint16_t SPI_Temp_RAW(int MISO) {
    uint16_t data;
    uint16_t bit;

    for (int i=0; i<12;i++) {
        digitalWrite(clock,HIGH);
        delay(SPI_Temp_Delay);
        data = data << 1;
        bit = (digitalRead(MISO)) ? 1 : 0;
        data |= bit;
        Serial.write(bit);
        digitalWrite(clock, LOW);
        delay(SPI_Temp_Delay);
    }

    return data;
}