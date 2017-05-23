
/*
 * test3
 * 
 * staticに宣言した変数の外部での利用が可能かどうか
 * 
 */

 /* 結果
 * 余裕で可能 
 * By toma
 */

 

#include <SoftwareSerial.h>
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)

// staticな変数を宣言してみる。
static int a = 0;

void setup() {
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
}

void loop() {
  count_up_a();
  Serial.println(a);
  delay(1000);
}
