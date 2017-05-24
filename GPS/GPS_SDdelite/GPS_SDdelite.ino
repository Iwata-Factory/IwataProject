/*
   SDGPSファイルの削除
*/

#include <SD.h>
#include <Wire.h>
#include <SPI.h>

#define SD_SS   53
#define GPS_TEXT ( "GPSlog.txt" )

const int chipselect = 4;
int sno = 0;                   //サーチをかけるバイト数が何バイトめか(1+sno)byt

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  Serial.write("START");

  //SDカード初期化
  Serial.print("Initializig SD Card...");
  pinMode( SS , OUTPUT );
  SD.begin( SD_SS );
  while (!SD.begin(chipselect)) {
    ;
  }
  Serial.println("CARD ON");                        //初期化完了

  SD.remove( GPS_TEXT );

}

void loop() {
  // put your main code here, to run repeatedly:
  sno++;
  File initial = SD.open( GPS_TEXT, FILE_WRITE );
  Serial.println( initial.read() );
  initial.close();

  if (sno > 50) {
    while ( 1 );
  }
  delay(500 );

}
