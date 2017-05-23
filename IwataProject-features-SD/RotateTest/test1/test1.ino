/*
 * test1
 * 
 * 回転の閾値を決定
 * 
 */

#include <SoftwareSerial.h>

#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define M1_1 8 // モーター制御用ピン
#define M1_2 9 // モーター制御用ピン
#define M2_1 10 // モーター制御用ピン
#define M2_2 11 // モーター制御用ピン

typedef struct { // モーター制御
  int right1 = 0; // 8番ピン対応
  int right2 = 0; // 9番ピン対応
  int leght1 = 0; // 10番ピン対応
  int leght2 = 0; // 11番ピン対応
} DRIVE;

void rover_degital(DRIVE drive) {
  digitalWrite(M1_1, drive.right1);
  digitalWrite(M1_2, drive.right2);
  digitalWrite(M2_1, drive.leght1);
  digitalWrite(M2_2, drive.leght1);
}



void setup() {
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
}

# 30秒回転
void loop() { 
  delay(50000);

  DRIVE turn;
  drive.right1 = 1;
  drive.right2 = 0;
  drive.leght1 = 0;
  drive.leght2 = 1;

  rover_degital(turn);

  delay(300000);
  
  drive.right1 = 1;
  drive.right2 = 1;
  drive.leght1 = 1;
  drive.leght2 = 1;

  rover_degital(turn);
  
}
