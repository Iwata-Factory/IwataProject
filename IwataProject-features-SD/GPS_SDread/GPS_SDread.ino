/*
 * SDGPSファイルの削除
 */

#include <SD.h>
#include <Wire.h>
#include <SPI.h>

#define SD_SS   53
#define GPS_TEXT ( "GPSLOG.TXT" )

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
    Serial.println("Card Failed, or not present");  
  }
  Serial.println("CARD ON");                        //初期化完了
  
  File initial = SD.open( GPS_TEXT, FILE_READ );       //ファイルの１バイト目を呼び出し。
  initial.seek(0);
  Serial.println( initial.read() );
  initial.close();

}

void loop() {
  // put your main code here, to run repeatedly:
  sno++;
  File initial = SD.open( GPS_TEXT, FILE_WRITE );
  Serial.println( initial.read() );
  initial.close();

  if(sno >50){
    while( 1 );
  }
  delay(500);

}
