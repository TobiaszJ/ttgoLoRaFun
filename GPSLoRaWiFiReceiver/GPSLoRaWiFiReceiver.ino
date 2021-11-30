
#include <LoRa.h>
#include "boards.h"

void setup()
{
  initBoard();
  // When the power is turned on, a delay is required.
  delay(1500);

  Serial.println("LoRa Receiver");

  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DI0_PIN);
  if (!LoRa.begin(LoRa_frequency)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.setSignalBandwidth(LoRa_bandwith);
  LoRa.setSyncWord(0x89);
  LoRa.setSpreadingFactor(12);
  LoRa.enableCrc();
}

void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    String recv = "";
    // read packet
    while (LoRa.available()) {
      recv += (char)LoRa.read();
    }

    Serial.println(recv);

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
#ifdef HAS_DISPLAY
    if (u8g2) {
      u8g2->clearBuffer();
      char buf[256];
      snprintf(buf, sizeof(buf), "RSSI:%i", LoRa.packetRssi());
      u8g2->drawStr(0, 12, buf);
      snprintf(buf, sizeof(buf), "SNR:%.1f", LoRa.packetSnr());
      u8g2->drawStr(62, 12, buf);
      u8g2->drawStr(0, 26, recv.c_str());
      u8g2->sendBuffer();
    }
#endif
  }
}
