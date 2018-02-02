/*
*   THIS IS THE LIBRARY CONTAINING FUNCTIONS FOR SPI COMMUNICATIONS WITH THE SENSORS
*
*/
const unsigned int _SPI_Temp_Delay = 10;
int _MISO,_CLK,_tempCS,_micCS,_humidCS, _MOSI;

void SPI_init(int MISOpin, int CLKpin, int tempPin, int micPin, int humidPin) {
    _MISO = MISOpin;
    _CLK = CLKpin;
    _tempCS = tempPin;
    _micCS = micPin;
    _humidCS = humidPin;
    _MOSI = 8;

    pinMode(_MISO, INPUT);
    pinMode(_tempCS, OUTPUT);
    pinMode(_micCS, OUTPUT);
    pinMode(_humidCS, OUTPUT);
    pinMode(_CLK, OUTPUT);
    pinMode(_MOSI, OUTPUT);

    digitalWrite(_tempCS, HIGH);
    digitalWrite(_micCS, HIGH);
    digitalWrite(_humidCS, HIGH);
    delay(200); // Powerup time for T-sensor

    Serial.begin(9600);
}

double SPI_Temp_RAW() {
    uint16_t data = 0;
    uint8_t decimals = 0;
    uint16_t bit;
    int sign = 1;

    digitalWrite(_tempCS, LOW);
    delay(100); // Delay for tempsens

    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    sign = (digitalRead(_MISO) == HIGH) ? -1 : 1;
    //Serial.print(bit);
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);

    for (int i=0; i<11;i++) {
        digitalWrite(_CLK,HIGH);
        delay(_SPI_Temp_Delay);
        data = data << 1;
        bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
        data |= bit;
        //Serial.print(bit);
        digitalWrite(_CLK, LOW);
        delay(_SPI_Temp_Delay);
    }

    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
    //Serial.print(bit);

    if (bit == 1) decimals += 50;
        
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);
    
    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
    //Serial.print(bit);

    if (bit == 1) decimals += 25;
        
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);


    data *= sign;

    digitalWrite(_tempCS, HIGH);
    /*Serial.println();
    Serial.print(data);
    Serial.print(".");
    Serial.print(decimals);
    Serial.print(" C\n");*/

    // Constructs the floating point number presentation of temperature
    return sign*(double)data+((double)decimals/100);
}

uint16_t SPI_audio_RAW() {
    uint16_t data = 0;
    uint16_t bit;

    digitalWrite(_micCS, LOW);
    //delay(1); // Delay for tempsens

    for (int i=0; i<16;i++) {
        digitalWrite(_CLK,HIGH);
        //delay(_SPI_Temp_Delay);
        data = data << 1;
        bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
        data |= bit;
        //Serial.print(bit);
        digitalWrite(_CLK, LOW);
        //delay(_SPI_Temp_Delay);
    }

    digitalWrite(_micCS, HIGH);
    //Serial.println();
    return data;
}

void SPI_humid_RAW() {
  int bit = 0;
  int reg[8] = {0,1,0,0,1,1,0,0};

  digitalWrite(_humidCS, LOW);

  digitalWrite(_CLK, HIGH);
  delay(1);

  for (int i = 0; i<8; i++) {
    int writebit = (reg[i]) ? HIGH : LOW;
    digitalWrite(_MOSI, writebit);
    digitalWrite(_CLK, LOW);
    delay(1);
    digitalWrite(_CLK, HIGH);
    delay(1);
  }
  
    for (int i=0; i<16;i++) {
        digitalWrite(_CLK,HIGH);
        delay(_SPI_Temp_Delay);
        bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
        Serial.print(bit);
        digitalWrite(_CLK, LOW);
        delay(_SPI_Temp_Delay);
    }

    digitalWrite(_humidCS, HIGH);
}





