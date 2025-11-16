void setupSensors() {
  unsigned status;
  Serial.println("Starting sensors...");

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

  //#define BMV080_ADDR 0x57  // SparkFun BMV080 Breakout defaults to 0x57
  if (bmv080.begin(0x57, Wire) == false) {
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

  // // switch off obstruction monitoring
  // bmv080.setDoObstructionDetection(false);

  if (bmv080.setMode(SF_BMV080_MODE_DUTY_CYCLE) == true) {
    Serial.println("BMV080 set to cycle mode");
  } else {
    Serial.println("Error setting BMV080 mode");
  }
  Serial.println("Sensors started");
}

bool readSensor(data_record &to_send) {
  bool returnValue = false;

  if (bmv080.readSensor()) {
    uint64_t timestamp = timeClient.getEpochTime();

    float pm10 = bmv080.PM10();
    float pm25 = bmv080.PM25();
    float pm1 = bmv080.PM1();
    bool obstructed = bmv080.isObstructed();

    float temp = bme.readTemperature();
    float pressure = bme.readPressure();
    float humidity = bme.readHumidity();

    printValues(pm1, pm25, pm10, temp, pressure, humidity, obstructed);

    to_send.timestamp = timestamp;
    to_send.pm1 = (uint16_t)pm1;
    to_send.pm2_5 = (uint16_t)pm25;
    to_send.pm10 = (uint16_t)pm10;
    to_send.temperature = (int)(temp * 100.0F);
    to_send.relative_humidity = (uint16_t)(humidity * 100.0F);
    to_send.pressure = (uint16_t)(pressure / 10.0F);
    to_send.flags = obstructed ? 1 : 0;
    to_send.location = LOCATION_WOOD_SHOP_MAIN;

    returnValue = true;
  }

  return returnValue;
}

void printValues(float pm1, float pm25, float pm10, float temp, float pressure, float humidity, bool obstructed) {
  Serial.println();
  Serial.print("Temperature:");
  Serial.print(temp);

  Serial.print(",Pressure:");
  Serial.print(pressure / 100.0F);

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
