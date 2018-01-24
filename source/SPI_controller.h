/*
*   THIS IS THE LIBRARY CONTAINING FUNCTIONS FOR SPI COMMUNICATIONS WITH THE SENSORS
*
*/
const unsigned int _SPI_Temp_Delay = 1;
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

    digitalWrite(_tempCS, HIGH);
    delay(200); // Powerup time for T-sensor

    Serial.begin(9600);
}

uint16_t SPI_Temp_RAW() {
    uint16_t data = 0;
    uint8_t decimals = 0;
    uint16_t bit;
    int sign = 1;

    digitalWrite(_tempCS, LOW);
    delay(100); // Delay for tempsens

    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    sign = (digitalRead(_MISO) == HIGH) ? -1 : 1;
    Serial.print(bit);
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);

    for (int i=0; i<11;i++) {
        digitalWrite(_CLK,HIGH);
        delay(_SPI_Temp_Delay);
        data = data << 1;
        bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
        data |= bit;
        Serial.print(bit);
        digitalWrite(_CLK, LOW);
        delay(_SPI_Temp_Delay);
    }

    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
    Serial.print(bit);

    if (bit == 1) decimals += 50;
        
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);
    
    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
    Serial.print(bit);

    if (bit == 1) decimals += 25;
        
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);


    data *= sign;

    digitalWrite(_tempCS, HIGH);
    Serial.println();
    Serial.print(data);
    Serial.print(".");
    Serial.print(decimals);
    Serial.print(" C\n");
    return data;
}
