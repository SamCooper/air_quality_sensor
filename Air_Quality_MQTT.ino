#include <WiFi.h>
#include <NTPClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <CircularBuffer.hpp>

#include "SparkFun_BMV080_Arduino_Library.h"  // CTRL+Click here to get the library: http://librarymanager/All#SparkFun_BMV080
#include "mqtt-sn.h"

// wifi_secrets.h must contain the following
//const char *ssid = "xxxx";   // Change this to your WiFi SSID
//const char *password = "xxxx";  // Change this to your WiFi password
#include "wifi_secrets.h"


#define LOCATION_WOOD_SHOP_MAIN 110
#define BROADCAST_WOOD_SHOP_MAIN "env/b/pm/v1.0.0/ws/main"

typedef struct {
  uint16_t pm1;
  uint16_t pm2_5;
  uint16_t pm10;
  int16_t temperature;
  uint16_t relative_humidity;
  uint16_t pressure;
  uint16_t flags;
  uint16_t location;
  uint64_t timestamp;
} data_record;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
SparkFunBMV080 bmv080;
Adafruit_BME280 bme;


const unsigned long delayTime = 1000;
const unsigned long dutyCycleTimeS = 15;
const int bufferSize = (6 * 60 * 60) / dutyCycleTimeS; // every duty cycle time in 6h

CircularBuffer<data_record, bufferSize> ringBuffer;


void setup() {
  Serial.begin(19200);
  while (!Serial)
    ;  // time to get serial running

  delay(1000);
  Serial.println("Starting up....");

  setupSensors();

  setupWiFi();

  timeClient.begin();

  setupMQTT();
}

void loop() {
  data_record to_send;

  timeClient.update();

  if (readSensor(to_send)) {
    sendValues(to_send);
  }

  Serial.println("Loop...");
  delay(delayTime);
}
