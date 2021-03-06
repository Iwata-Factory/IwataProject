#ifndef __DEFINE_H__
#define __DEFINE_H__

#if MACHINE == 1  // FM1の場合

// 定数の定義
// ゴール位置(能代)
#define GOAL_LATITUDE 35.71484
#define GOAL_LONGITUDE 139.75908


#define GOAL_CIRCLE 3
#define LAST_GOAL_CIRCLE 1.0
// 海抜制限
#define ALT_REGULATION 100
//SD関連
#define LOG_TIME ("timelog.txt")
#define LOG_AC ("aclog.txt")
#define LOG_TM ("tmlog.txt")
#define LOG_GPS ("gpslog.txt")
#define LOG_CRITICAL ("critical.txt")  // 現在使っていない
#define LOG_CONTROL ("cnt_log.txt")
//GPS関連
#define PIN_GPS1_Rx  10 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS2_Rx  12 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_TX_DUMMY 13 // GPSのTXピンのダミー
#define LATITUDE_MINIMUM 35  //緯度の最小値
#define LATITUDE_MAXIMUM 45  //緯度の最大値
#define LONGITUDE_MINIMUM 133  //経度の最小値
#define LONGITUDE_MAXIMUM 143  //経度の最大値
#define GPSBAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数
#define GPS_JUMP_DISTANCE 30 // GPSのジャンプ許容距離
//地磁気センサ関連
#define HMC5883L 0x1E   //HMC5883L(地磁気センサ)のスレーブアドレス
#define SINGLE_MODE 0x01   //single mode
#define CONTINUOUS_MODE 0x00  //continuous mode
#define TM_DIFFERENCE -7.2
//加速度センサ関連
#define ADXL345 0x53  //ADXL345(加速度センサ)のスレーブアドレス
//モーター関連
#define M1_1 8
#define M1_2 7
#define M2_1 5
#define M2_2 6
// PID制御関連
#define PI_RIGHT_DEFAULT 240
#define PI_LEGHT_DEFAULT 250
#define PI_INTEGRAL_RISET 10
#define PID_STREIGHT_BETWEEN 100
#define PI_KP 1.2
#define PI_KP2 0
#define PI_KI 0.2
#define PI_MIN 80
#define PI_MAX 250
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
//XBEE関連
#define XBEE_BUFFERSIZE 63
#define ENTER 0x0D
#define O_CAP 0x4F
// その他
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define pi 3.14159265359

//camera関連
#define PIN_CAM_Rx 24
#define PIN_CAM_Tx 26
#define CAMBAUDRATE 115200

//SoftwareSerial g_cam( PIN_CAM_Rx, PIN_CAM_Tx );  //arduino-camera間のSerial通信用これだと空撮中のgpsが取れない

#define PIC_PKT_LEN    128                  //data length of each read, dont set this too big because ram is limited 
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     Serial3
#define CAM_BUTTON     24
#define PIC_FMT        PIC_FMT_VGA

File cam_pic;

const byte cameraAddr = (CAM_ADDR << 5);  // addr
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;


// グローバル変数の定義(ごちゃごちゃしているためいずれ整理したい)
static unsigned long time; //タイマー起動
byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xED, 0x61};  // XBEE親機アドレス（６月３０日改造版）
//byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xB9, 0x3D, 0xCD};  // XBEE親機アドレス

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
double tm_x_offset = -94.5;
double tm_y_offset = 33.0;
double x_def = 711.0;
double y_def = 646.0;
// 立ち入り禁止エリア(10個まで生成可能)
POINT danger_area_points[10];
char xbee_send[XBEE_BUFFERSIZE];  //とりあえずのxbee送信用配列
SoftwareSerial g_gps1( PIN_GPS1_Rx, PIN_GPS_TX_DUMMY); // ArduinoとGPS間のシリアル通信用に
SoftwareSerial g_gps2( PIN_GPS2_Rx, PIN_GPS_TX_DUMMY); // ArduinoとGPS間のシリアル通信用に
int use_which_gps = 2;  // 1か2どちらのGPSを使用するか
int gps_timeout_counter_global = 0;
int sd_count = 0;
unsigned long time_out = 0;
int time_out_flag = 1;

#elif MACHINE == 2

// 定数の定義
#define GOAL_LATITUDE 35.760275
#define GOAL_LONGITUDE 139.765946

#define GOAL_CIRCLE 3
#define LAST_GOAL_CIRCLE 1.0
// 海抜制限
#define ALT_REGULATION 100
//SD関連
#define LOG_TIME ("timelog.txt")
#define LOG_AC ("aclog.txt")
#define LOG_TM ("tmlog.txt")
#define LOG_GPS ("gpslog.txt")
#define LOG_CRITICAL ("critical.txt")  // 現在使っていない
#define LOG_CONTROL ("cnt_log.txt")
//GPS関連
#define PIN_GPS1_Rx  10 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS2_Rx  12 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_TX_DUMMY 13 // GPSのTXピンのダミー
#define LATITUDE_MINIMUM 35  //緯度の最小値
#define LATITUDE_MAXIMUM 45  //緯度の最大値
#define LONGITUDE_MINIMUM 133  //経度の最小値
#define LONGITUDE_MAXIMUM 143  //経度の最大値
#define GPSBAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数
#define GPS_JUMP_DISTANCE 30 // GPSのジャンプ許容距離
//地磁気センサ関連
#define HMC5883L 0x1E   //HMC5883L(地磁気センサ)のスレーブアドレス
#define SINGLE_MODE 0x01   //single mode
#define CONTINUOUS_MODE 0x00  //continuous mode
#define TM_DIFFERENCE -7.2
//加速度センサ関連
#define ADXL345 0x53  //ADXL345(加速度センサ)のスレーブアドレス
//モーター関連
#define M1_1 8
#define M1_2 7
#define M2_1 5
#define M2_2 6
// PID制御関連
#define PI_RIGHT_DEFAULT 230
#define PI_LEGHT_DEFAULT 250
#define PI_INTEGRAL_RISET 10
#define PID_STREIGHT_BETWEEN 100
#define PI_KP 1.2
#define PI_KP2 0
#define PI_KI 0.2
#define PI_MIN 80
#define PI_MAX 250
//スピーカー関連
//#define BEAT_LONG 300   // 音の長さを指定
//#define TONE_PINNO 33   // 圧電スピーカを接続したピン番号
//#define C_TONE  262    //ド
//#define D_TONE  294    //レ
//#define E_TONE  330    //ミ
//#define F_TONE  349
//#define G_TONE  392
//#define A_TONE  440
//#define B_TONE  494
//#define HIGH_C  523
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
//XBEE関連
#define XBEE_BUFFERSIZE 63
#define ENTER 0x0D
#define O_CAP 0x4F
// その他
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define pi 3.14159265359

//camera関連
//#define PIN_CAM_Rx 24
//#define PIN_CAM_Tx 26
//#define CAMBAUDRATE 115200
//#define PIC_PKT_LEN    128                  //data length of each read, dont set this too big because ram is limited 
//#define PIC_FMT_VGA    7
//#define PIC_FMT_CIF    5
//#define PIC_FMT_OCIF   3
//#define CAM_ADDR       0
//#define CAM_SERIAL     Serial3
//#define CAM_BUTTON     24
//#define PIC_FMT        PIC_FMT_VGA

//File cam_pic;

//const byte cameraAddr = (CAM_ADDR << 5);  // addr
//unsigned long picTotalLen = 0;            // picture length
//int picNameNum = 0;
// グローバル変数の定義(ごちゃごちゃしているためいずれ整理したい)
static unsigned long time; //タイマー起動
byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xEB, 0xBA}; 

//byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xED, 0x61};  // XBEE親機アドレス（６月３０日改造版）
//byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xB9, 0x3D, 0xCD};  // XBEE親機アドレス

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
double tm_x_offset = 31;
double tm_y_offset = -105.0;
double x_def = 788.0;
double y_def = 787.0;
// 立ち入り禁止エリア(10個まで生成可能)
POINT danger_area_points[10];
char xbee_send[XBEE_BUFFERSIZE];  //とりあえずのxbee送信用配列
SoftwareSerial g_gps1( PIN_GPS1_Rx, PIN_GPS_TX_DUMMY); // ArduinoとGPS間のシリアル通信用に
SoftwareSerial g_gps2( PIN_GPS2_Rx, PIN_GPS_TX_DUMMY); // ArduinoとGPS間のシリアル通信用に
int use_which_gps = 2;  // 1か2どちらのGPSを使用するか
int gps_timeout_counter_global = 0;
int sd_count = 0;
unsigned long time_out = 0;
int time_out_flag = 1;


#endif

#endif

