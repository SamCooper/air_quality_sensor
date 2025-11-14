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
  
  // switch off obstruction monitoring
  bmv080.setDoObstructionDetection(false);

  if (bmv080.setMode(SF_BMV080_MODE_DUTY_CYCLE) == true) {
    Serial.println("BMV080 set to cycle mode");
  } else {
    Serial.println("Error setting BMV080 mode");
  }
}
