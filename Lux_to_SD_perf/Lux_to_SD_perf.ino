/*
   照度センサの値が閾値を超えたときにFLAGファイルに書き込みを行う
   プログラムです。FLAGファイルは１bytでミッションシーケンスの
   節目を超えたことを記録することを目的とするイメージです。
*/


#include <SD.h>
#include <Wire.h>
#include <SPI.h>

#define SD_SS   10
#define FLAG_INIT ( (byte)0x00)           //キャストかけないとコンパイルエラー
#define LUXBORDER  (15)                   //照度センサ閾値
#define LUX_Y (0x01)                      //照度センサが閾値を超えたときに代入する値
#define FLAG ( "mis_flag.byt" )           //SD内FLAG判断用bytファイルの名前

const int chipselect = 4;
int val0 = 0;
int sno = 0;   //iran

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  Serial.write("START");

  //SDカード初期化
  Serial.print("Initializig SD Card...");
  pinMode( SS , OUTPUT );
  SD.begin( SD_SS );
  if (!SD.begin(chipselect)) {
    Serial.println("Card Failed, or not present");
    return;
  }
  Serial.println("CARD ON");                        //初期化完了

  SD.remove( FLAG );                               //SD内の同一ファイル消去

  File initial = SD.open( FLAG, FILE_WRITE );
  initial.write( FLAG_INIT );                      //FLAGの初期化
  initial.seek(0);                                 //カーソルを先頭に。
  Serial.print( initial.peek() );                  //初期値表示
  initial.close();
  Serial.println( "FLAG INITIALIZED" );

}

void loop() {
  // put your main code here, to run repeatedly:
  sno++;
  val0 = analogRead( 0 );               //照度センサ回路にかかる分圧が計測されます。
  Serial.print( val0 );
  Serial.print( "\n" );

  if ( val0 > LUXBORDER ) {
    Serial.println( "get Lux" );
    loglux_yn();

    if ( sno > 50 ) {                   //ただ単に実行回数制限しているだけ
      while ( 1 );
    }
  }
  delay(500);
}


void loglux_yn() {
  File loglux = SD.open( FLAG, FILE_READ );
  byte loging = loglux.peek();
  loglux.close();

  loging = loging | LUX_Y;                       //bit毎論理和をとる

  loglux = SD.open( FLAG, FILE_WRITE );
  loglux.seek(0);
  loglux.write( loging );                        //上書き完了
  loglux.close();
  Serial.println( "Lux overwritten" );
}
