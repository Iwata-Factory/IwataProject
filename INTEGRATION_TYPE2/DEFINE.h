#ifndef __DEFINE_H__
#define __DEFINE_H__

// 定数の定義
#define GOAL_LATITUDE 35.756165
#define GOAL_LONGITUDE 139.770740

//GPS関連
#define PIN_GPS_Rx  10 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_Tx  12 // GPSのシリアル通信でデータを送信するピン
#define LATITUDE_MINIMUM 35  //緯度の最小値
#define LATITUDE_MAXIMUM 45  //緯度の最大値
#define LONGITUDE_MINIMUM 133  //経度の最小値
#define LONGITUDE_MAXIMUM 143  //経度の最大値
#define GPSBAUDRATE 14400 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数

//地磁気センサ関連
#define HMC5883L 0x1E   //HMC5883L(地磁気センサ)のスレーブアドレス
#define TM_DIFFERENCE -7.2

//加速度センサ関連
#define ADXL345 0x53  //ADXL345(加速度センサ)のスレーブアドレス

//モーター関連
#define M1_1 4 // モーター制御用ピン
#define M1_2 5 // モーター制御用ピン
#define M2_1 6 // モーター制御用ピン
#define M2_2 7 // モーター制御用ピン

// PID制御関連
#define PID_SURPULS 0.500
#define PID_KP 0.100
#define PID_KI 0.400
#define PID_KD 0.100

//スピーカー関連
#define BEAT_LONG 300   // 音の長さを指定
#define TONE_PINNO 33   // 圧電スピーカを接続したピン番号
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
#define DESTINATION_HEAD 1  //以下のアドレスは実験によって確定させます。
#define DESTINATION_END  16
#define START_HEAD 17
#define START_END  32
#define STOP_HEAD 33
#define STOP_END  48
#define AVESPEED_HEAD 49
#define AVESPEED_END  56
#define EEP_STATUS 0  //eeprom内のフラグアドレス
#define FLAG_START 0x01
#define FLAG_LAUNCH 0x02
#define FLAG_FALL 0x04
#define FLAG_LAND 0x08
#define FLAG_GND1 0x10
#define FLAG_GND2 0x20
#define FLAG_END 0x40

#define EEP_CENSOR_STATUS 1  //eeprom内のセンサステータスのアドレス
#define STATUS_GPS 0x01
#define STATUS_AC 0x02
#define STATUS_TM 0x04
#define STATUS_LIGHT 0x08
#define STATUS_GPS2 0x10
#define STATUS_SD 0x20
#define STATUS_DIS 0x40
#define STATUS_XBEE 0x80
#define SD_FLAG ("mis_flag.byt")

//ニクロム線
#define NICROM_1 28
#define NICROM_2 30

//SD関連
#define SS 53

// SDが挿入されているかどうかを判定（回路は未実装）
#define SD_INSERT 25  //SDが挿入されているかを判定する回路からの入力１なら挿入中？？ （回路未実装）

// その他
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define pi 3.14159265359

// グローバル変数の定義(ごちゃごちゃしているためいずれ整理したい)
static unsigned long time; //タイマー起動

//byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xB9, 0x3D, 0xCD}; //XBEE親機アドレス（古い機体の海外版）
byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xCA, 0x9A, 0x3D};  //XBEE親機アドレス（６月１９日購入版）

static const uint8_t length = 6;   //読み出しデータの個数
char g_szReadBuffer[READBUFFERSIZE] = "";
int  g_iIndexChar = 0;
byte flag[8] = { //flag配列SDへの書き込みが１byte単位なので書き込む値は最大１バイトまでにしたい。
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07
};

byte flag_phase[8] = {
  FLAG_START, FLAG_LAUNCH, FLAG_FALL,
  FLAG_LAND, FLAG_GND1, FLAG_GND2, FLAG_END
};
const int chipSelect = 4;
// 地磁気のキャリブレーションに関するやつ
double tm_x_offset = 0.0;
double tm_y_offset = 0.0;
double x_def = 1.0;
double y_def = 1.0;

// 立ち入り禁止エリア(10個まで生成可能)
POINT danger_area_points[10];

char xbee_send[63];  //とりあえずのxbee送信用配列

SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx); // ArduinoとGPS間のシリアル通信用に
Servo servo1;

#endif

