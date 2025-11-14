#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "SparkFun_BMV080_Arduino_Library.h"  // CTRL+Click here to get the library: http://librarymanager/All#SparkFun_BMV080
#include "mqtt-sn.h"

// wifi_secrets.h must contain the following
//const char *ssid = "xxxx";   // Change this to your WiFi SSID
//const char *password = "xxxx";  // Change this to your WiFi password
#include "wifi_secrets.h"

#define BMV080_ADDR 0x57  // SparkFun BMV080 Breakout defaults to 0x57

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
} env_pm_record;

const unsigned long delayTime = 1000;
const unsigned long dutyCycleTimeS = 15;

SparkFunBMV080 bmv080;  // Create an instance of the BMV080 class
Adafruit_BME280 bme;


void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  // time to get serial running

  setupSensors();

  setupWiFi();

  setupMQTT();
}

void loop() {
  if (bmv080.readSensor()) {
    float pm10 = bmv080.PM10();
    float pm25 = bmv080.PM25();
    float pm1 = bmv080.PM1();
    bool obstructed = bmv080.isObstructed();

    float temp = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0F;
    float humidity = bme.readHumidity();

    printValues(pm1, pm25, pm10, temp, pressure, humidity, obstructed);

    sendValues(pm1, pm25, pm10, temp, pressure, humidity, obstructed);
  }

  delay(delayTime);
}

void printValues(float pm1, float pm25, float pm10, float temp, float pressure, float humidity, bool obstructed) {
  Serial.print("Temperature:");
  Serial.print(temp);

  Serial.print(",Pressure:");
  Serial.print(pressure);

  Serial.print(",Humidity:");
  Serial.print(humidity);

  Serial.print(",PM10:");
  Serial.print(pm10);

  Serial.print(",PM2.5:");
  Serial.print(pm25);

  Serial.print(",PM1:");
  Serial.print(pm1);

  Serial.print(",Obstructed:");
  Serial.print(obstructed);

  Serial.println();
}
