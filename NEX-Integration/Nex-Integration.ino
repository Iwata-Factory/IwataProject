
/*
  メインプログラム
*/

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


// include文
#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <SD.h>
#include <avr/sleep.h>
//#include <xbee.h>  //このライブラリはslackを参照して各自PCに入れてください。
//#include <EEPROM.h>

// 定数の定義

//GPS関連
#define PIN_GPS_Rx  53 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_Tx  51 // GPSのシリアル通信でデータを送信するピン
#define LATITUDE_MINIMUM 30  //緯度の最小値
#define LATITUDE_MAXIMUM 40  //緯度の最大値
#define LONGITUDE_MINIMUM 130  //経度の最小値
#define LONGITUDE_MAXIMUM 140  //経度の最大値
#define GPSBAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数

//地磁気センサ関連
#define HMC5883L 0x1E   //HMC5883L(地磁気センサ)のスレーブアドレス

//加速度センサ関連
#define ADXL345 0x53  //ADXL345(加速度センサ)のスレーブアドレス

//モーター関連
#define M1_1 8 // モーター制御用ピン
#define M1_2 9 // モーター制御用ピン
#define M2_1 10 // モーター制御用ピン
#define M2_2 11 // モーター制御用ピン

//スピーカー関連
#define BEAT_LONG 300   // 音の長さを指定
#define TONE_PINNO 8   // 圧電スピーカを接続したピン番号
#define C_TONE  262    //ド
#define D_TONE  294    //レ
#define E_TONE  330    //ミ
#define F_TONE  349
#define G_TONE  392
#define A_TONE  440
#define B_TONE  494
#define HIGH_C  523

// 照度センサ関連
#define LIGHT_PIN 2  //照度センサピン

// 距離センサ関連
#define DISTANCE A12  //測距センサの距離読み取り用ピン
#define DIS_SAMPLE 50  //距離のサンプリング数

// EEPROM関連
#define EEP_LENGTH 4096  //EEPROMはMegaは4096byte、Unoは1024byteの容量です。
#define EEP_FLAG 0  //flagの記録場所は現在は０です。
#define DESTINATION_HEAD 1  //以下のアドレスは実験によって確定させます。
#define DESTINATION_END  16
#define START_HEAD 17
#define START_END  32
#define STOP_HEAD 33
#define STOP_END  48
#define AVESPEED_HEAD 49
#define AVESPEED_END  56

// その他
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define pi 3.14159265359

// 構造体を宣言

typedef struct {  // 機体自身の情報を定義
  double laditude = -1.0;  // 緯度
  double longitude = -1.0;  // 経度
  double Direction = -1.0;  // ターゲットの方角
  double distance = -1.0;  //ターゲットまでの距離
  long time_from_start = 0.0;  // 機体の現在時刻
  int status_number = 0;  // 自分の現在のシーケンスを把握
  int gpa1_arive = 1;  // 1個目のgpsが生きているか（1:生存、0:死亡）
  int gps2_arive = 1;
  int tm_arive = 1;
  int ac_arive = 1;
  int ill_arive = 1;
  int distance_arive = 1;
} ROVER;

typedef struct { // 2次元のベクトル
  double x = 0.0; //2次元ベクトルのx座標
  double y = 0.0; //2次元ベクトルのy座標
} Vector2D;

typedef struct { // 3次元のベクトル
  double x = 0.0; //3次元ベクトルのx座標
  double y = 0.0; //3次元ベクトルのy座標
  double z = 0.0; //3次元ベクトルのz座標
} Vector3D;

struct GPS { // GPS関連    /* これだけ良くわからなかったのでtypedefしていません */
  double utc = 0.0;       //グリニッジ天文時
  double latitude = 0.0;   //経度
  double longitude = 0.0;   //緯度
  double Speed = 0.0;    //移動速度
  double course = 0.0;    //移動方位
  double Direction = -1.0;   //目的地方位
  double distance = -1.0;     //目的地との距離
  /*Speedとdistanceは小文字が予約語だったのでとりあえず大文字にしてあります*/
};

typedef struct { // 加速度センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
} AC;

typedef struct { // 地磁気センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
} TM;

typedef struct { // モーター制御
  int right1 = 0; // 8番ピン対応
  int right2 = 0; // 9番ピン対応
  int leght1 = 0; // 10番ピン対応
  int leght2 = 0; // 11番ピン対応
} DRIVE;

// グローバル変数の定義(ごちゃごちゃしているためいずれ整理したい)
static unsigned long time; //タイマー起動
byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xCA, 0x9A, 0x3D}; //XBEE親機アドレス
static const uint8_t length = 6;   //読み出しデータの個数
char g_szReadBuffer[READBUFFERSIZE] = "";
int  g_iIndexChar = 0;
byte flag[8] = { //flag配列SDへの書き込みが１byte単位なので書き込む値は最大１バイトまでにしたい。
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07
};
SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx); // ArduinoとGPS間のシリアル通信用に

/*
   セットアップ
*/
void setup() {

  // 各種初期化処理
  Wire.begin();           //I2C通信の初期化
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  g_gps.begin(GPSBAUDRATE); //シリアル通信の初期化
  writeI2c(0x02, 0x00, HMC5883L); //HMC5883Lの初期設定0x02レジスタに0x00書き込み
  writeI2c(0x31, 0x00, ADXL345);  //上と同様
  writeI2c(0x2d, 0x08, ADXL345);  //上と同様
  //  xbee_init(0);  //初期化
  //  xbee_atcb(4);  //ネットワーク初期化
  //  xbee_atnj(0);  //孫機のジョイン拒否
  //  while (xbee_atai() > 0x01) { //ネットワーク参加状況を確認
  //    delay(3000);
  //    xbee_atcb(1);  //ネットワーク参加ボタン押下
  //  }
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
