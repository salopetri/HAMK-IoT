/*
*   THIS IS THE LIBRARY CONTAINING FUNCTIONS FOR SPI COMMUNICATIONS WITH THE SENSORS
*
*/
const unsigned int _SPI_Temp_Delay = 10;
int _MISO,_CLK,_tempCS,_micCS,_humidCS;

void SPI_init(int MISOpin, int CLKpin, int tempPin, int micPin, int humidPin) {
    _MISO = MISOpin;
    _CLK = CLKpin;
    _tempCS = tempPin;
    _micCS = micPin;
    _humidCS = humidPin;

    pinMode(_MISO, INPUT);
    pinMode(_tempCS, OUTPUT);
    pinMode(_micCS, OUTPUT);
    pinMode(_humidCS, OUTPUT);
    pinMode(_CLK, OUTPUT);
}

uint16_t SPI_Temp_RAW() {
    uint16_t data;
    uint16_t bit;

    for (int i=0; i<12;i++) {
        digitalWrite(_CLK,HIGH);
        delay(_SPI_Temp_Delay);
        data = data << 1;
        bit = (digitalRead(_MISO)) ? 1 : 0;
        data |= bit;
        Serial.write(bit);
        digitalWrite(_CLK, LOW);
        delay(_SPI_Temp_Delay);
    }

    return data;
}