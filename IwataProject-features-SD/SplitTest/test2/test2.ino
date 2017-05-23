/*
 * test2
 * 
 * test1の外部関数化
 * 
 */

/* 結果
 * メインのファイルでだけ#includeや#defineすればいいみたい 
 * By toma
 */
 
#include <SoftwareSerial.h>

#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define AERO ("SPACE") // 定数を用意


void setup() {
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化

}

void loop() {
  output_space();
}
