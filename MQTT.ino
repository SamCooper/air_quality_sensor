void setupMQTT() {
  int64_t now;

  Serial.println("Starting MQTT...");

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

void sendValues(data_record &to_send) {
  ringBuffer.push(to_send);

  hexDump(&to_send, sizeof(to_send));

  if (WiFi.isConnected()) {
    Serial.print("MQTT sending: ");
    Serial.println(ringBuffer.size());

    while (!ringBuffer.isEmpty()) {
      data_record hd_send;
      hd_send = ringBuffer.shift();

      mqtt_sn_send_with_mac(MQTT_SN_MESSAGE_ENV_PM, &hd_send, sizeof(hd_send));
      mqtt_sn_broadcast_with_mac(BROADCAST_WOOD_SHOP_MAIN, strlen(BROADCAST_WOOD_SHOP_MAIN), &hd_send, sizeof(hd_send));
    }
  } else {
    Serial.print("No WiFi, appending to buffer, current size: ");
    Serial.println(ringBuffer.size());
  }
}

void hexDump(void *addr, int len) {
  unsigned char *p = (unsigned char *)addr;
  for (int i = 0; i < len; i++) {
    pByte(p[i]);
  }
  Serial.println();
}

void pByte(byte X) {
  if (X < 10) {
    Serial.print("0");
  }

  Serial.print(X, HEX);
}
