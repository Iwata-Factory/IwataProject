
/*
 * test4
 * 
 * 関数からの返り値を受け取れるのか
 * 
 */

#include <SoftwareSerial.h>
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)

void setup() {
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
}

void loop() {
  int a = 5;
  int b = powpow(5);
  Serial.print(b);
  delay(1000);
}
