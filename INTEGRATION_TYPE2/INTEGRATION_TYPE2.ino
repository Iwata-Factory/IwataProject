/*

  メインプログラム
*/

// 諸々の処理をこれ一文で
#include "INCLUDE.h"

char xbee_send[63];  //とりあえずのxbee送信用配列

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
/*
//  xbee関連
    xbee_init(0);  //初期化
    xbee_atcb(4);  //ネットワーク初期化
    xbee_atnj(0);  //孫機のジョイン拒否
    while (xbee_atai() > 0x01) { //ネットワーク参加状況を確認
      delay(3000);
      xbee_atcb(1);  //ネットワーク参加ボタン押下
    }
*/
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
      xbee_uart( dev,"Card failed, or not present\r");
      // 失敗、何もしない
      delay(1000);
      if (sd_ok_counter == 60) {
        xbee_uart( dev,"SD CARD DEATH\r");
        renew_status(STATUS_SD, 0);
        break;
      }
    } else {
      xbee_uart( dev,"SD OK\r");
      break;
    }
  }

  //照度センサ用のピン
  pinMode(LIGHT_PIN, INPUT);
  //距離センサ用のピン
  pinMode(DISTANCE, INPUT);
  //サーボモーター用のピン
  servo1.attach(26);
  xbee_uart( dev,"setup done\rchange to main phase\r");

  // モーター用ピンの設定
  pinMode(M1_1, OUTPUT);
  pinMode(M1_2, OUTPUT);
  pinMode(M2_1, OUTPUT);
  pinMode(M2_2, OUTPUT);

  DRIVE set;
  set.right1 = 1;
  set.right2 = 1;
  set.leght1 = 1;
  set.leght2 = 1;
  rover_degital(set);

}






void loop() {

  speaker(C_TONE);

  delay(2000);

  ROVER rover;  // 自身の情報を初期化
  rover.status_number = 1;  // 現在ステータスを1に更新
  rover.time_from_start = millis();  // 機体時間を取得
  write_timelog_sd(rover.time_from_start, 1);

  get_censor_status(&rover);  // 自身のステータスを更新する関数

  sprintf(xbee_send, "get data from EEP\rPresent status is %d\r", rover.status_number);
  xbee_uart( dev,xbee_send);



  do {

    switch (rover.status_number) {

      case 1:

        xbee_uart( dev,"start status1\r");

        rover.time_from_start = millis();  // 機体時間を取得
        write_timelog_sd(rover.time_from_start, 1);

        if (status1(&rover) == 1) {
          xbee_uart( dev,"skip status1\r");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }
        break;

      case 2:
        xbee_uart( dev,"start status2\r");

        rover.time_from_start = millis();  // 機体時間を取得
        write_timelog_sd(rover.time_from_start, 2);

        if (status2(&rover) == 1) {
          xbee_uart( dev,"clear status2\r");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 3:
        xbee_uart( dev,"start status3\r");

        rover.time_from_start = millis();  // 機体時間を取得
        write_timelog_sd(rover.time_from_start, 3);

        if (status3(&rover) == 1) {
          xbee_uart( dev,"skip status3\r");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 4:
        xbee_uart( dev,"start status4\r");

        rover.time_from_start = millis();  // 機体時間を取得
        write_timelog_sd(rover.time_from_start, 4);

        if (status4(&rover) == 1) {
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 5:
        xbee_uart( dev,"start status5\r");

        rover.time_from_start = millis();  // 機体時間を取得
        write_timelog_sd(rover.time_from_start, 5);

        if (status5(&rover) == 1) {
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }

      case 6:
        xbee_uart( dev,"start status6\r");

        rover.time_from_start = millis();  // 機体時間を取得
        write_timelog_sd(rover.time_from_start, 6);

        if (status6(&rover) == 1) {
          xbee_uart( dev,"skip status6\r");
          trans_phase(rover.status_number);
          rover.status_number += 1;
          break;
        } else {
          break;
        }
    }
  } while (0 < rover.status_number < 7);
  xbee_uart( dev,"reach status7\rEND CONTROL\r");
  while (1) {
    rover.time_from_start = millis();  // 機体時間を取得
    write_timelog_sd(rover.time_from_start, 7);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    delay(10000);
  }
}

