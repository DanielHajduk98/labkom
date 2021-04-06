#include <BMP280.h>

BMP280::BMP280()
  : Adafruit_BMP280() {}

void BMP280::begin() {
    if (!Adafruit_BMP280::begin(0x76)) {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
        while (1);
    }
    
    Adafruit_BMP280::setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}
