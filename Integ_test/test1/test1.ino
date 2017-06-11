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

  // i2c関連
  writeI2c(0x02, 0x00, HMC5883L); //HMC5883Lの初期設定0x02レジスタに0x00書き込み
  writeI2c(0x31, 0x00, ADXL345);  //上と同様
  writeI2c(0x2d, 0x08, ADXL345);  //上と同様

  //xbee関連
  //  xbee_init(0);  //初期化
  //  xbee_atcb(4);  //ネットワーク初期化
  //  xbee_atnj(0);  //孫機のジョイン拒否
  //  while (xbee_atai() > 0x01) { //ネットワーク参加状況を確認
  //    delay(3000);
  //    xbee_atcb(1);  //ネットワーク参加ボタン押下
  //  }

  //eeprom関連
  //eep_clear();   //EEPROMのリセット。４KB全てに書き込むので時間かかる。
  EEPROM.write( EEP_STATUS, flag_phase[0] ); // status1で初期化
  EEPROM.write( EEP_CENSOR_STATUS, 0xff);  //eepのflag類の初期化

  //SD関連
  pinMode(SS, OUTPUT);
  int sd_ok_counter = 0;
  while (1) {
    if (!SD.begin(chipSelect)) {
      sd_ok_counter += 1;
      Serial.println("Card failed, or not present");
      // 失敗、何もしない
      delay(1000);
      if (sd_ok_counter == 60) {
        Serial.println("SD CARD DEATH");
        renew_status(STATUS_SD, 0);
        break;
      }
    } else {
      break;
    }
  }

  // モーター用ピンの設定
  pinMode(M1_1, OUTPUT);
  pinMode(M1_2, OUTPUT);
  pinMode(M2_1, OUTPUT);
  pinMode(M2_2, OUTPUT);

  //照度センサ用のピン
  pinMode(LIGHT_PIN, INPUT);
  //距離センサ用のピン
  pinMode(DISTANCE, INPUT);
  //サーボモーター用のピン
  servo1.attach(26);
  Serial.println("setup完了");
  Serial.print("メイン処理へ移行");

}






void loop() {

  delay(2000);

  ROVER rover;  // 自身の情報を初期化
  rover.status_number = 1;  // 現在ステータスを1に更新
  rover.time_from_start = time;  // 機体時間を取得

  get_censor_status(&rover);  // 自身のステータスを更新する関数

  Serial.print("EEPROMから情報を取得。現在のステータスは");
  Serial.println(rover.status_number);



  do {

    switch (rover.status_number) {

      case 1:
        Serial.println("ステータス1を開始");
        if (status1() == 1) {
          Serial.println("ステータス1をスキップ");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }
        break;

      case 2:
        Serial.println("ステータス2を開始");
        if (status2() == 1) {
          Serial.println("ステータス2をクリア");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 3:
        Serial.println("ステータス3を開始");
        if (status3() == 1) {
          Serial.println("ステータス3をスキップ");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 4:
        Serial.println("ステータス4を開始");
        if (status4() == 1) {
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 5:
        Serial.println("ステータス5を開始");
        if (status5(&rover) == 1) {
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 6:
        Serial.println("ステータス6を開始");
        if (status6(&rover) == 1) {
          Serial.println("ステータス6をスキップ");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }
    }
  } while (0 < rover.status_number < 7);
  Serial.println("ステータス7に到達");
  Serial.println("制御を終了します");
  while (1) {
    delay(10000);
  }
}

