#include "SparkFun_Ublox_Arduino_Library.h"
#include "boards.h"
#include "SSD1306Wire.h"
#include <MicroNMEA.h>
#include <LoRa.h>

SFE_UBLOX_GPS myGPS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
SSD1306Wire display(0x3c, I2C_SDA, I2C_SCL);
int wait = 1000;
unsigned long sendtime;

void setup()
{
  initBoard();
  // When the power is turned on, a delay is required.
  delay(1500);

  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  if (myGPS.begin(Serial1) == false) {
    display.drawString(0, 0, "Ublox GPS not detected at default I2C address. Please check wiring. Freezing.");
    display.display();
    while (1);
  }

  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DI0_PIN);
  if (!LoRa.begin(LoRa_frequency)) {
    display.drawString(0, 0, "Starting LoRa failed!");
    display.display();
    while (1);
  }
  
  //LoRa.setSignalBandwidth(LoRa_bandwith);  
  LoRa.setSyncWord(0x89);
  LoRa.setSpreadingFactor(12);
  LoRa.setTxPower(20);
  LoRa.enableCrc();
  display.display();
}

void loop()
{
  display.clear();
  myGPS.checkUblox(); //See if new data is available. Process bytes as they come in.

  if (PMU.isBatteryConnect()) {
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    // Get battery voltage
    display.drawString(128, 50, String(PMU.getBattVoltage()) + " mV");

    // To display the charging status, you must first discharge the battery,
    // and it is impossible to read the full charge when it is fully charged
    if (PMU.isChargeing()) {
      display.drawString(128, 40, String(PMU.getBattChargeCurrent()) + " mA");
    } else {
      display.drawString(128, 40, "-" + String(PMU.getBattDischargeCurrent()) + " mA");
      //display.drawString(128, 30, String(PMU.getBattPercentage()) + " %");
    }
    display.setTextAlignment(TEXT_ALIGN_LEFT);
  }
  //sendtime = millis();
  LoRa.beginPacket();
  if (nmea.isValid() == true) {
    long latitude_mdeg = nmea.getLatitude();
    long longitude_mdeg = nmea.getLongitude();

    display.drawString(0, 0, "Latitude (deg): ");
    display.drawString(0, 10, String((float)latitude_mdeg / 1000000.0));
    display.drawString(0, 20, "Longitude (deg): ");
    display.drawString(0, 30, String((float)longitude_mdeg / 1000000.0));
    display.drawString(0, 40, "# Sat: " + String(nmea.getNumSatellites()));

    LoRa.print("/" + String(latitude_mdeg));
    LoRa.print("\\" + String(longitude_mdeg));
    LoRa.print("\\" + String(nmea.getNumSatellites()));
    LoRa.print("\\" + String((int)PMU.getBattVoltage())+ "/.");
  } else {
    display.drawString(0, 0, "No Fix - Num. satellites:");
    display.drawString(0, 10, String(nmea.getNumSatellites()));
    LoRa.print("/\\\\" + String(nmea.getNumSatellites()));
    LoRa.print("\\" + String((int)PMU.getBattVoltage())+ "/.");
  }
  LoRa.endPacket();
  //wait = 100*(millis()-sendtime);
  display.drawString(0, 50, String(wait));
  display.display();
  
  delay(wait); //Don't pound too hard on the I2C bus
}

//This function gets called from the SparkFun Ublox Arduino Library
//As each NMEA character comes in you can specify what to do with it
//Useful for passing to other libraries like tinyGPS, MicroNMEA, or even
//a buffer, radio, etc.
void SFE_UBLOX_GPS::processNMEA(char incoming)
{
  //Take the incoming char from the Ublox I2C port and pass it on to the MicroNMEA lib
  //for sentence cracking
  nmea.process(incoming);
}
