/*
*   THIS IS THE LIBRARY CONTAINING FUNCTIONS FOR SPI COMMUNICATIONS WITH THE SENSORS
*   WE CONTROL THE SPI MANUALLY FROM SOFTWARE SIDE, THIS WAS DONE PURELY TO GET DEEPER UNDERSTANDING OF SPI INTERFACE
*   IT WOULD BE ADVISED TO CHANGE THESE FUNCTIONS TO USE THE INTEGRATED SPI FUNCTIONALY IF DEVELOPED FURTHER
*/
const unsigned int _SPI_Temp_Delay = 1;
int _MISO,_CLK,_tempCS,_micCS;

// Run this function to initialize the interface
void SPI_init(int MISOpin, int CLKpin, int tempPin, int micPin) {
    _MISO = MISOpin;
    _CLK = CLKpin;
    _tempCS = tempPin;
    _micCS = micPin;

    pinMode(_MISO, INPUT);
    pinMode(_tempCS, OUTPUT);
    pinMode(_micCS, OUTPUT);
    pinMode(_CLK, OUTPUT);

    digitalWrite(_tempCS, HIGH);
    digitalWrite(_micCS, HIGH);
    delay(200); // Powerup time for PmodTC1
}

/* Function reading the input from temperature sensor
* Returns the temperature as Double
*/
double SPI_Temp() {
    uint16_t data = 0;
    uint8_t decimals = 0;
    uint16_t bit;
    int sign = 1;

    // Write chip select LOW so that temperature sensor starts listening to SPI traffic
    digitalWrite(_tempCS, LOW);
    
    delay(100); // Delay for the temperature sensor

    // Following code reads the first bit that is the sign bit for the temperature
    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    sign = (digitalRead(_MISO) == HIGH) ? -1 : 1;
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);

    // Next we read next 11 bits telling us the temperature as integer
    for (int i=0; i<11;i++) {
        digitalWrite(_CLK,HIGH);
        delay(_SPI_Temp_Delay);
        data = data << 1;
        bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
        data |= bit;
        digitalWrite(_CLK, LOW);
        delay(_SPI_Temp_Delay);
    }

    // We read two more bits to get the fractions
    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;

    if (bit == 1) decimals += 50;
        
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);
    
    digitalWrite(_CLK,HIGH);
    delay(_SPI_Temp_Delay);
    bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;

    if (bit == 1) decimals += 25;
        
    digitalWrite(_CLK, LOW);
    delay(_SPI_Temp_Delay);

    // If the sign-bit was true we multiply the temperature values with -1 to make them negative
    data *= sign;
    decimals *= sign;

    // Write chip select high to stop temperature sensor listening SPI traffic
    digitalWrite(_tempCS, HIGH);

    // Constructs the floating point number presentation of temperature
    return sign*(double)data+((double)decimals/100);
}

/* function reading the input from the microphone
* Returns the raw data as 16-bit unsigned int
*/
uint16_t SPI_audio_RAW() {
    uint16_t data = 0;
    uint16_t bit;

    digitalWrite(_micCS, LOW);

    for (int i=0; i<16;i++) {
        digitalWrite(_CLK,HIGH);
        data = data << 1;
        bit = (digitalRead(_MISO) == HIGH) ? 1 : 0;
        data |= bit;
        digitalWrite(_CLK, LOW);
    }

    digitalWrite(_micCS, HIGH);
    
    return data;
}


