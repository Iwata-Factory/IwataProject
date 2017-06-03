/*
   arduinoMegaに内蔵されているEEPROMを扱っています。
   容量は４KBなのでそんなに大きうの情報は書き込めません。。
*/

#include <EEPROM.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// 定数の定義
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数
// GPSのピンはSDと干渉しないように
//speekerとも干渉しないように
#define PIN_GPS_Rx  10 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_Tx  12 // GPSのシリアル通信でデータを送信するピン
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define GPSBAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define LATITUDE_MINIMUM 30  //緯度の最小値
#define LATITUDE_MAXIMUM 40  //緯度の最大値
#define LONGITUDE_MINIMUM 130  //経度の最小値
#define LONGITUDE_MAXIMUM 140  //経度の最大値
#define HMC5883L 0x1E   //HMC5883L(地磁気センサ)のスレーブアドレス
#define ADXL345 0x53  //ADXL345(加速度センサ)のスレーブアドレス
#define M1_1 8 // モーター制御用ピン
#define M1_2 9 // モーター制御用ピン
#define M2_1 10 // モーター制御用ピン
#define M2_2 11 // モーター制御用ピン
/*適当に1番にしてある*/
#define LIGHT_PIN 1  //照度センサピン
#define pi 3.14159265359

#define EEP_LENGTH 4096  //EEPROMはMegaは4096byte、Unoは1024byteの容量です。
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define EEP_FLAG 0  //flagの記録場所は現在は０です。
#define DESTINATION_HEAD 1
#define DESTINATION_END  16
#define START_HEAD 17
#define START_END  32
#define STOP_HEAD 33
#define STOP_END  48
#define AVESPEED_HEAD 49
#define AVESPEED_END  56
#define INTERVAL 2

// グローバル変数の定義
static unsigned long time; //タイマー起動
static float last_distance = -1; // 目的地までの距離(m)。負の値で初期化。
static const uint8_t length = 6;   //読み出しデータの個数

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
  double x = 2.56; // x軸方向
  double y = 3.14; // y軸方向
  double z = 4.44; // z軸方向
} AC;

typedef struct { // 地磁気センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
} TM;

int phase = 0;
byte flag[4] = {
  0x00, 0x01, 0x03, 0x07
};

// 50,51をArduinoとGPS間のシリアル通信用に
SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx);


void setup() {
  // put your setup code here, to run once:
  Wire.begin();           //I2C通信の初期化
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  Serial.begin(SERIAL_BAUDRATE);
  eep_clear();
  Serial.println("setup done.");
}

void loop() {
  AC ac;
  // put your main code here, to run repeatedly:
  Serial.println("looping...");
  if ((int)millis > ((phase + 1)*INTERVAL*100)) {
    Serial.println(phase);
    Serial.println("create new flag...");
    get_ac();
    eep_acwrite( 1, ac);
    eep_flagwrite( flag[phase], flag[phase + 1] );
    Serial.println(EEPROM.read(EEP_FLAG));
    phase++;
    if ( phase > 2) {
      while (1);
    }
  }
  delay(100);
}
