#include <WiFiManager.h> 
#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <BMP280.h>
#include <PIR.h>
#include <GY30.h>
#include <API.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MEASURE_INTERVAL 900000 // 15 min

WiFiManager wifiManager;
PIR pir(D5);
DHT dht(D3, DHT11);
GY30 gy30;
BMP280 bmp280;
API api("http://192.168.1.27:81/api");
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

unsigned long millisCurrent;
unsigned long millisLastMeasurement = 0;

float temp = 0;
float RH = 0;
float HI = 0;
unsigned int pressure = 0;
unsigned int movement = 0;
unsigned int lightLevel = 0;

void collectMeasurements() {
  temp = bmp280.readTemperature();
  pressure = bmp280.readPressure() / 100;
  RH = dht.readHumidity();
  HI = dht.computeHeatIndex(temp, RH);
  lightLevel = gy30.readLightLevel();

  millisLastMeasurement = millisCurrent; 
}

static const unsigned char PROGMEM emoji_smile[] = {
 B00000000,
 B00000000,
 B00100100,
 B00000000,
 B00000000,
 B01000010,
 B00111100,
 B00000000,
};

static const unsigned char PROGMEM emoji_neutral[] = {
 B00000000,
 B00000000,
 B00100100,
 B00000000,
 B00000000,
 B01111110,
 B00000000,
 B00000000,
};

static const unsigned char PROGMEM emoji_sad[] = {
 B00000000,
 B00000000,
 B00100100,
 B00000000,
 B00000000,
 B00111100,
 B01000010,
 B00000000,
};

static const unsigned char PROGMEM emoji_scared[] = {
 B00000000,
 B00000000,
 B00100100,
 B00000000,
 B00000000,
 B00111100,
 B01111110,
 B00000000,
};

static const unsigned char PROGMEM emoji_danger[] = {
 B11011011,
 B11011011,
 B11011011,
 B11011011,
 B11011011,
 B00000000,
 B11011011,
 B11011011,
};


void drawHeatIndexEmoji() {
  if (26 >= HI && HI <= 32) {
        display.drawBitmap(display.getCursorX(), display.getCursorY(), emoji_neutral, 8, 8, WHITE);
      } else if (32 < HI && HI <= 41) {
        display.drawBitmap(display.getCursorX(), display.getCursorY(), emoji_sad, 8, 8, WHITE);
      } else if (41 < HI && HI <= 54) {
        display.drawBitmap(display.getCursorX(), display.getCursorY(), emoji_scared, 8, 8, WHITE);
      } else if (HI > 54) {
        display.drawBitmap(display.getCursorX(), display.getCursorY(), emoji_danger, 8, 8, WHITE);
      }
  display.drawBitmap(display.getCursorX(), display.getCursorY(), emoji_smile, 8, 8, WHITE); 
}


void displayMeasurements() {
  int progress = ((millisCurrent - millisLastMeasurement) * 128) / ((MEASURE_INTERVAL)); 

  display.setCursor(0,0);
  display.clearDisplay();

  
  display.drawRect(0, 0, 128, 8, WHITE);
  display.fillRect(2, 2, progress - 4 , 4, WHITE);
    
  display.println("");
  display.println("--Last measurements--");
  display.print("Temperature: " + (String)temp); display.write(248); display.println("C");
  display.println("Humidity: " + (String)RH + "%");
  display.println("Mov: " + (String)movement + " Lux: " + (String)lightLevel + " lx");
  display.println("Atm Pressure: " + (String)pressure + "hPa");
  display.print("Heat Index: "); drawHeatIndexEmoji(); display.println("");

  display.display();
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++
// Setup + Loop
// +++++++++++++++++++++++++++++++++++++++++++++++++++

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.cp437(true);
  display.println("Loading...");
  display.display();

  Serial.begin(115200);

  display.println("Starting Wifi...");
  display.display();
  
  wifiManager.autoConnect("Home-meter");

  // reset settings - wipe credentials for testing
  // wifiManager.resetSettings();

  display.println("Starting DHT...");
  display.display();

  dht.begin(); 

  display.println("Starting GY30...");
  display.display();
  gy30.begin();

  display.println("Starting BMP280...");
  display.display();
  bmp280.begin();

  display.println("Checking server...");
  display.display();
  api.setup();

  collectMeasurements();
}

void loop() {  
  millisCurrent = millis();

  if((millisCurrent - millisLastMeasurement) >= MEASURE_INTERVAL) {
    collectMeasurements();
    
    api.sendMeasurements(
      temp,
      RH,
      pressure,
      lightLevel,
      movement,
      HI
    );

    movement = 0;
  } 
  else if ((millisCurrent - pir.millisLast) >= 3000) {
    movement += pir.read();
    pir.millisLast = millisCurrent;
  }

  displayMeasurements();
}
