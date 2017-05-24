#include <SoftwareSerial.h>
#include <SD.h>

#define SD_SS  53
#define PIN_GPS_Rx 50
#define PIN_GPS_Tx 52
#define SERIAL_BAUDRATE 9600
#define GPSBAUDRATE 9600
SoftwareSerial sGps(PIN_GPS_Rx, PIN_GPS_Tx);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("Software Serial Test Start!");
  pinMode( SS, OUTPUT );
  SD.begin( SD_SS );
  sGps.begin(GPSBAUDRATE);
}
void loop() {
  // put your main code here, to run repeatedly:
  if (sGps.available()) {
    Serial.write(sGps.read());
  }
}
