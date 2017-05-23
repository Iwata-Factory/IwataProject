/*
 * test1
 * 
 * まだ外部関数化はしていない
 * 
 */

#include <SoftwareSerial.h>

#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define AERO ("SPACE") // 定数を用意

void setup() {
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
}

void loop() { 
  // 1秒ごとにSPACEと表示
  Serial.println(AERO);
  delay(1000);
}
