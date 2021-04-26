#include <API.h>

/*!
    @brief Constructor for API object.
    @param baseUrl
    Server base url
    @note Requires EEPROM init before starting.
*/
  API::API(String baseUrl) {
    this->baseUrl = baseUrl;
  }

/*!
    @brief Write String to EEPROM
    @param addrOffset
    Starting EEPROM addres from where we will start reading
    @param strToWrite 
    A const reference to a String object.
    @return void.
*/
void API::writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

/*!
    @brief Read String from EEPROM
    @param addrOffset
    Starting EEPROM addres from where we will start reading
    @return Http response code.
    @note Requires EEPROM init before starting.
*/
String API::readStringFromEEPROM(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}

/*!
    @brief Ask server for token and check if server works.
    Saves token in EEPROM
    @return Http response code.
    @note Requires EEPROM init before starting.
*/
int API::setup() {
  WiFiClientSecure client;
  HTTPClient http;

  client.setInsecure();

  http.setTimeout(10000); //Let it sit on a long timeout.  
  http.begin(client, baseUrl + "/api/monitor");
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST("{\"mac_address\":\"" + WiFi.macAddress() + "\"}");

  if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();

        Serial.println("Response: " + payload);

        if(payload != "Monitor already in DB") {
          API::writeStringToEEPROM(0, payload);
        }
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

  http.end();  

  Serial.println(httpCode);

  return httpCode;
}


/*!
    @brief Sends data to server.
    @param temperature
    Temperature value
    @param humidity
    Relative humidity value
    @param air_pressure
    Air pressure value in hPa
    @param luminosity
    Luminosity value
    @param movement
    Movement value
    @param heatIndex
    Heat index value
    @return Http response code.
    @note Requires all data to be NOT null.
    @note Requires EEPROM init before starting.
*/
int API::sendMeasurements(
    float temperature,
    float humidity,
    float air_pressure,
    float lumionsity,
    byte movement,
    float heatIndex) 
  {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();

    http.begin(client, baseUrl + "/api/measurement");
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);

    String token = API::readStringFromEEPROM(0);

    int httpCode = http.POST(
        "{\"monitor_mac\":\"" + (String)WiFi.macAddress() + "\"" +
        ",\"token\":\"" + token + "\"" +
        ",\"temperature\":\"" + (String)temperature + "\"" +
        ",\"humidity\":\"" + (String)humidity + "\"" +
        ",\"luminosity\":\"" + (String)lumionsity + "\"" +
        ",\"movement\":\"" + (String)movement +"\"" +
        ",\"heat_index\":\"" + (String)heatIndex +"\"" +
        ",\"air_pressure\":\"" + (String)air_pressure + "\"}");

    Serial.println(baseUrl + "/api/measurement");
    
    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    const String& payload = http.getString();
    Serial.println(payload);

    http.end();

    return httpCode;
}