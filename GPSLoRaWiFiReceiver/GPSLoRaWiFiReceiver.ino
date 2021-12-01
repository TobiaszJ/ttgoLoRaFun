
#include <LoRa.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <HTTPClient.h>
#include "boards.h"

AsyncWebServer server(80);

const char* ssid = "BearTest";          // Your WiFi SSID
const char* password = "testBEAR";      // Your WiFi Password
const char* serverName = "http://iot.bear-ict.ch/insert.php";
String apiKeyValue = "tPmAT5Ab3j7F921";

void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
}

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

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();
}

void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    WebSerial.print("Received packet '");

    String recv = "";
    // read packet
    while (LoRa.available()) {
      recv += (char)LoRa.read();
    }

    WebSerial.print(recv);

    // print RSSI of packet
    WebSerial.print("' with RSSI ");
    WebSerial.print(LoRa.packetRssi());
    WebSerial.print("' and SNR ");
    WebSerial.println(LoRa.packetSnr());

    
    HTTPClient http;
    WiFiClient client;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "api_key=" + apiKeyValue + "&data=" + recv + "&rssi=" + LoRa.packetRssi() + "&snr=" + LoRa.packetSnr() + "&device=" + "test1" + "";
    int httpResponseCode = http.POST(httpRequestData);
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
    
#ifdef HAS_DISPLAY
    if (u8g2) {
      u8g2->clearBuffer();
      char buf[256];
      snprintf(buf, sizeof(buf), "RSSI:%i", LoRa.packetRssi());
      u8g2->drawStr(0, 12, buf);
      snprintf(buf, sizeof(buf), "SNR:%.1f", LoRa.packetSnr());
      u8g2->drawStr(62, 12, buf);
      u8g2->drawStr(0, 26, recv.c_str());
//      u8g2->drawStr(0, 40, String(WiFi.localIP()));
      u8g2->sendBuffer();
    }
#endif
  }
}
