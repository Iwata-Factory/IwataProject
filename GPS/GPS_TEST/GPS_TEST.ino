#include <SoftwareSerial.h>
#include <SD.h>

#define SD_SS  53
#define PIN_GPS_Rx 50
#define PIN_GPS_Tx 52
#define SERIAL_BAUDRATE 9600
#define GPSBAUDRATE 9600

Sd2Card card;
SdVolume volume;
SdFile root;

SoftwareSerial sGps(PIN_GPS_Rx, PIN_GPS_Tx);
const int chipSelect = 4;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("Software Serial Test Start!");
  pinMode( SS, OUTPUT );

  sGps.begin(GPSBAUDRATE);
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    // 失敗、何もしない
    while (1);
  }
  Serial.println("loop start");
}
void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println(Serial.available());
  if (sGps.available()) {
    Serial.println(sGps.read());
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      Serial.print("size:");
      Serial.println(dataFile.size());
      dataFile.seek(dataFile.size());
      //   dataFile.println(sGps.read());
      dataFile.println("aaaaa");
      Serial.print("pos:");
      Serial.println(dataFile.position());
      dataFile.close();
      delay(500);
    } else {
      Serial.println("death");
    }
  }
}
