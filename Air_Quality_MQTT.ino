#include <WiFiSTA.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
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


SparkFunBMV080 bmv080;  // Create an instance of the BMV080 class
Adafruit_BME280 bme;

unsigned long delayTime = 1000;
unsigned long dutyCycleTimeS = 15;

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


void setup() {
  int64_t now;

  Serial.begin(115200);
  while (!Serial)
    ;  // time to get serial running

  setupSensors();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed");
    while (1) {
      delay(1000);
    }
  }

  Serial.print("WiFi connected: ");
  Serial.println(WiFi.macAddress());

  mqtt_sn_init();

  for (int sn_attempts = 0; sn_attempts < 3; sn_attempts++) {
    if (mqtt_sn_is_ready()) {
      now = millis();
      Serial.println("MQTT hello..");
      mqtt_sn_send_with_mac(MQTT_SN_MESSAGE_HELLO, &now, sizeof(now));
      break;
    } else {
      delay(1000);
    }
  }
  Serial.println("MQTT started");
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

    setValues(pm1, pm25, pm10, temp, pressure, humidity, obstructed);
  }

  delay(delayTime);
}

void setValues(float pm1, float pm25, float pm10, float temp, float pressure, float humidity, bool obstructed) {
  env_pm_record to_send;

  to_send.pm1 = (uint16_t)pm1;
  to_send.pm2_5 = (uint16_t)pm25;
  to_send.pm10 = (uint16_t)pm10;
  to_send.temperature = (int)(temp * 100.0F);
  to_send.relative_humidity = (uint16_t)(humidity * 100.0F);
  to_send.pressure = (uint16_t)(pressure * 100.0F);
  to_send.flags = obstructed ? 1 : 0;
  to_send.location = LOCATION_WOOD_SHOP_MAIN;

  mqtt_sn_send_with_mac(MQTT_SN_MESSAGE_ENV_PM, &to_send, sizeof(to_send));
  mqtt_sn_broadcast_with_mac(BROADCAST_WOOD_SHOP_MAIN, strlen(BROADCAST_WOOD_SHOP_MAIN), &to_send, sizeof(to_send));
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

void setupSensors() {
  unsigned status;

  // default settings
  status = bme.begin();

  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  Wire.begin();

  if (bmv080.begin(BMV080_ADDR, Wire) == false) {
    Serial.println(
      "BMV080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1)
      ;
  }
  Serial.println("BMV080 found!");

  /* Initialize the Sensor (read driver, open, reset, id etc.)*/
  bmv080.init();

  /* Set the sensor mode to continuous mode */
  bmv080.setDutyCyclingPeriod(dutyCycleTimeS);

  if (bmv080.setMode(SF_BMV080_MODE_DUTY_CYCLE) == true) {
    Serial.println("BMV080 set to cycle mode");
  } else {
    Serial.println("Error setting BMV080 mode");
  }
}
