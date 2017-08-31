/*
  メインコード
*/
/*
 * 1号機は死亡
 * 2号機　生存
 * 3号機　製作中
 */
#define MACHINE 2  // 1 or 2を指定
#include "INCLUDE.h"

/*
   セットアップ　　
*/

void setup() {
  
  // 各種初期化処理
  Wire.begin();           //I2C通信の初期化
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  g_gps1.begin(GPSBAUDRATE); //シリアル通信の初期化
  g_gps2.begin(GPSBAUDRATE); //シリアル通信の初期化

  // i2c関連
  writeI2c(0x02, CONTINUOUS_MODE, HMC5883L); //HMC5883Lの初期設定0x02レジスタに0x00書き込み
  writeI2c(0x31, 0x00, ADXL345);  //上と同様
  writeI2c(0x2d, 0x08, ADXL345);  //上と同様

  //  xbee関連
  if (XBEE_SWITCH == 1) {
    xbee_init(0);  //初期化
    xbee_atcb(4);  //ネットワーク初期化
    xbee_atnj(0);  //孫機のジョイン拒否
    while (xbee_atai() > 0x01) { //ネットワーク参加状況を確認
      delay(3000);
      xbee_atcb(1);  //ネットワーク参加ボタン押下
    }
  }

  //eeprom関連
  //eep_clear();   //EEPROMのリセット。４KB全てに書き込むので時間かかる。
  EEPROM.write( EEP_STATUS, flag_phase[0] ); // status1で初期化
  EEPROM.write( EEP_CENSOR_STATUS, 0xff);  //eepのflag類の初期化

  //SD関連
  if (SD_LOG_FLAG == 1) {
    pinMode(SS, OUTPUT);
    int sd_ok_counter = 0;
    while (1) {
      if (!SD.begin(chipSelect)) {  // SDのセットアップに失敗
        sd_ok_counter += 1;
        xbee_uart( dev, "Card failed, or not present\r");
        delay(1000);
        if (sd_ok_counter == 60) {  // 60回セットアップを試みても駄目だったらループを抜ける
          xbee_uart( dev, "SD CARD DEATH\r");
          renew_status(STATUS_SD, 0);
          break;
        }
      } else {  // SDのセットアップに成功
        xbee_uart( dev, "SD OK\r");
        break;
      }
    }
  }

  write_control_sd("setup start");

  write_critical_sd(0);  // クリティカルログを残す

  //照度センサ用のピン
  pinMode(LIGHT_PIN, INPUT);
  //距離センサ用のピン
  pinMode(DISTANCE, INPUT);
  // モーター用ピンの設定
  pinMode(M1_1, OUTPUT);
  pinMode(M1_2, OUTPUT);
  pinMode(M2_1, OUTPUT);
  pinMode(M2_2, OUTPUT);

  // 明示的なモーターのオフ
  DRIVE set;
  set.right1 = 1;
  set.right2 = 1;
  set.leght1 = 1;
  set.leght2 = 1;
  rover_degital(set);

  //ニクロム線のピンモード
  //明示的なオフ
  pinMode(NICROM_1, OUTPUT);
  pinMode(NICROM_2, OUTPUT);
  digitalWrite(NICROM_1, LOW);
  digitalWrite(NICROM_2, LOW);

  //xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  xbee_uart( dev, "setup done\rchange to main phase\r");
  write_control_sd("setup end");

}



void loop() {

  speaker(C_TONE);
  delay(2000);

  // 自身の情報を初期化
  ROVER rover;
  // モーターの明示的なオフ
  DRIVE reset;
  reset.right1 = 1;
  reset.right2 = 1;
  reset.leght1 = 1;
  reset.leght2 = 1;

  // 実験用の部分
  if (STACK_EXP == 0) {
    rover.status_number = 1;  // 現在ステータスを1に更新
    write_timelog_sd(&rover);
    get_censor_status(&rover);
  } else if (STACK_EXP == 1) { // 強制的にスタックのフラグを立てる
    rover.status_number = 5;  // 現在ステータスを5に更新
    write_timelog_sd(&rover);
  }

  do {

    // ここでセンサーの状態チェックを走らせる
    get_censor_status(&rover);  // 最新のセンサーの状態を取得

    switch (rover.status_number) {

      case 1:

        xbee_uart( dev, "start status1\r");
        write_control_sd("status1 start");

        delay(1000);


        if (status1(&rover) == 1) {
          rover_degital(reset);
          trans_phase(rover.status_number);
          rover.status_number += 1;
          xbee_uart( dev, "success status1\r");
          break;
        } else {
          break;
        }
        break;

      case 2:
        xbee_uart( dev, "start status2\r");
        write_control_sd("status2 start");

        if (status2(&rover) == 1) {
          rover_degital(reset);
          trans_phase(rover.status_number);
          rover.status_number += 1;
          xbee_uart( dev, "success status2\r");
          break;
        } else {
          break;
        }

      case 3:
        xbee_uart( dev, "start status3\r");
        write_control_sd("status3 start");


        if (status3(&rover) == 1) {
          rover_degital(reset);
          trans_phase(rover.status_number);
          rover.status_number += 1;
          xbee_uart( dev, "success status3\r");
          break;
        } else {
          break;
        }

      case 4:
        xbee_uart( dev, "start status4\r");
        write_control_sd("status4 start");


        if (status4(&rover) == 1) {
          rover_degital(reset);
          trans_phase(rover.status_number);
          rover.status_number += 1;
          xbee_uart( dev, "success status4\r");
          break;
        } else {
          break;
        }

      case 5:
        xbee_uart( dev, "start status5\r");
        write_control_sd("status5 start");


        write_timelog_sd(&rover);

        if (status5(&rover) == 1) {
          rover_degital(reset);
          trans_phase(rover.status_number);
          rover.status_number += 1;
          xbee_uart( dev, "success status5-1\r");
          break;
        } else {
          break;
        }


      case 6:
        xbee_uart( dev, "start status6\r");
        write_control_sd("status6 start");

        write_timelog_sd(&rover);
        if (status6(&rover) == 1) {
          rover_degital(reset);
          trans_phase(rover.status_number);
          rover.status_number += 1;
          write_critical_sd(2);  // 制御終了
          xbee_uart( dev, "success status6\r");
          break;
        } else {
          break;
        }
    }

  } while (0 < rover.status_number && rover.status_number < 7);

  xbee_uart( dev, "reach status7\rEND CONTROL\r");
  write_control_sd("all end");


  while (1) {
    write_timelog_sd(&rover);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    speaker(HIGH_C);
    delay(100000);
    delay(100000);
    delay(100000);

  }


}

