/*
  digital pin  pin mapping 使用するピン
  0 TX  Xbee RX
  1 RX  Xbee TX
  2 PWM スリープ解除ピン
  3 PWM speeker
  4 PWM MOTOR
  5 PWM MOTOR
  6 PWM MOTOR
  7 PWM MOTOR
  8 PWM 圧電スピーカー
  9 PWM
  10  PWM(software serial)  GPS RX
  11  PWM(software serial)
  12  PWM(software serial)  GPS TX
  13  PWM(software serial)  speeker
  14  TX3
  15  RX3
  16  TX2
  17  RX2
  18  TX1
  19  RX1
  20  SDA I2C for sensor
  21  SCL I2C for sensor
  22  digital
  23  digital
  24  digital
  25  digital
  26  digital
  27  digital
  28  digital
  29  digital
  30  digital
  31  digital
  32  digital
  33  digital
  34  digital
  35  digital
  36  digital
  37  digital
  38  digital
  39  digital
  40  digital
  41  digital
  42  digital
  43  digital 圧電ブザー
  44  digital
  45  digital
  46  digital
  47  digital
  48  digital
  49  digital
  50  digital SD MISO
  51  digital SD MOSI
  52  digital SD CLK
  53  digital SD CS


  analog pin
  0 analog in
  1 analog in
  2 analog in
  3 analog in
  4 analog in
  5 analog in
  6 analog in
  7 analog in
  8 analog in
  9 analog in
  10  analog in
  11  analog in
  12  analog in distance
  13  analog in
  14  analog in
  15  analog in
*/


/*
  メインプログラム
*/

// 諸々の処理をこれ一文で
#include "INCLUDE.h"

/*
   セットアップ
*/
void setup() {

  // 各種初期化処理
  Wire.begin();           //I2C通信の初期化

  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  g_gps.begin(GPSBAUDRATE); //シリアル通信の初期化
  //  writeI2c(0x02, 0x00, HMC5883L); //HMC5883Lの初期設定0x02レジスタに0x00書き込み
  //  writeI2c(0x31, 0x00, ADXL345);  //上と同様
  //  writeI2c(0x2d, 0x08, ADXL345);  //上と同様
  xbee_init(0);  //初期化
  xbee_atcb(4);  //ネットワーク初期化
  xbee_atnj(0);  //孫機のジョイン拒否
  while (xbee_atai() > 0x01) { //ネットワーク参加状況を確認
    delay(3000);
    xbee_atcb(1);  //ネットワーク参加ボタン押下
  }
  //eep_clear();   //EEPROMのリセット。４KB全てに書き込むので時間かかる。
  //EEPROM.write(EEP_FLAG,0);  //flagの部分のみ初期化。

  // モーター用ピンの設定
  pinMode(M1_1, OUTPUT);
  pinMode(M1_2, OUTPUT);
  pinMode(M2_1, OUTPUT);
  pinMode(M2_2, OUTPUT);

  //照度センサ用のピン
  pinMode(LIGHT_PIN, INPUT);
  //距離センサ用のピン
  pinMode(DISTANCE, INPUT);
  Serial.println("setup完了");
}

void loop() {
  ROVER rover;  // 自身の情報を初期化
  rover.status_number = 1;  // 現在ステータスを1に更新
  rover.time_from_start = time;  // 機体時間を取得

  GPS gps;
  AC ac;
  TM tm;

  // EEPROMからフラグを読んで、ROBER型のstatus_numberを更新する。
  // EEPRPMからフラグを読んで、各センサが生きているか検知してROVER型の各種センサ_ariveを更新する。

  while (1) {

    if (rover.status_number == 1) {

      ;  // 本来であればここは起動シーケンスを行う(別ファイルに起動時の処理を作る予定)
      rover.status_number += 1;
      continue;

    } else if (rover.status_number == 2) {

      int status2_result = status2();  // Stutas2の関数へ任せる
      if (status2_result == 1) {
        rover.status_number += 1;
        continue;
      } else {
        continue;
      }

    } else if (rover.status_number == 3) {

      int status3_result = status3();   // Stutas3の関数へ任せる
      if (status3_result == 1) {
        rover.status_number += 1;
        continue;
      } else {


        continue;
      }

    } else if (rover.status_number == 4) {

      int status4_result = status4();   // Stutas4の関数へ任せる
      if (status4_result == 1) {
        rover.status_number += 1;
        continue;
      } else {
        continue;
      }

    } else if (rover.status_number == 5) {

      //ROVER rover = status5(rover);   // Stutas5の関数へ任せる
      if (rover.status_number == 6) {
        continue;
      } else {
        continue;
      }
    }

  }
}
