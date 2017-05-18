/*
 * test5
 * 
 * 3つファイルで試してみる
 * 
 */

 
#include <SoftwareSerial.h>
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)

static int a = 1;

void setup() {
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
}

void loop() {
  count_up_a();
  Serial.println(powpow(a));
  delay(1000);
}
