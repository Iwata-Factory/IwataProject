#ifndef __DEFINE_H__
#define __DEFINE_H__

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

//ニクロム線
#define NICROM_1 28
#define NICROM_2 30


// その他
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define pi 3.14159265359

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
Servo servo1;

#endif
