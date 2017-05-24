/*
 * FLAGファイルの値を確認するプログラムです。
 * このファイルはミッションシーケンスの達成状況の確認できることを目的としているので
 * １byt目しか現在使っていないし、使うこともなかろうかと思います。
 * ただ、書き込みの時にseek(0)の位置を間違えたりするとすぐ２byt目とかに
 * 行くだろうことが予測されますので、ログをFLAGに取るときは正しく書き込めているか確認しましょう
 */


#include <SD.h>
#include <Wire.h>
#include <SPI.h>

#define SD_SS   53
#define FLAG ("mis_flag.byt")  //SD内FLAG判定用bytファイル

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
  if (!SD.begin(chipselect)) {
    Serial.println("Card Failed, or not present");  
    return;
  }
  Serial.println("CARD ON");                        //初期化完了
  
  File initial = SD.open( FLAG, FILE_READ );       //ファイルの１バイト目を呼び出し。
  initial.seek(0);
  Serial.println( initial.read() );
  initial.close();

}

/*
 * FLAGの２バイト目以降の読み込みを行う。
 * ここは読み出した値は-1になるはずで、
 * 他の値の場合はプログラムのミスを疑ってください。
 * ここに入れる１バイトは機体がシーケンスのどこまで
 * 到達したかを判断するのに必要です！！
 */
void loop() {
  // put your main code here, to run repeatedly:
  sno++;
  File initial = SD.open( FLAG, FILE_WRITE );
  Serial.println( initial.read() );
  initial.close();

  if(sno >50){
    while( 1 );
  }

  delay(50);
}
