void setupMQTT() {
  int64_t now;

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

void sendValues(float pm1, float pm25, float pm10, float temp, float pressure, float humidity, bool obstructed) {
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
