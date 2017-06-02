/*
 * ６月２日時点で動作未確認です
 */

#include <SoftwareSerial.h>
#include <xbee.h>
#include <SD.h>
#include <Wire.h>
#include <math.h>

#define READBUFFERSIZE  (256)
#define DELIMITER   (",")
#define PIN_GPS_Rx  10
#define PIN_GPS_Tx  12
#define SERIAL_BAUDRATE 9600
#define GPSBAUDRATE 9600
#define LATITUDE_MINIMUM 30  //緯度の最小値
#define LATITUDE_MAXIMUM 40  //緯度の最大値
#define LONGITUDE_MINIMUM 130  //経度の最小値
#define LONGITUDE_MAXIMUM 140  //経度の最大値
#define HMC5883L 0x1E   //HMC5883L(地磁気センサ)のスレーブアドレス
#define ADXL345 0x53  //ADXL345(加速度センサ)のスレーブアドレス

static unsigned long time; //タイマー起動
static float last_distance = -1; // 目的地までの距離(m)。負の値で初期化。
static const uint8_t length = 6;   //読み出しデータの個数

byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xCA, 0x9A, 0x3D}; //XBEE親機アドレス
int sno = 0;

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

SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx);

void setup() {
  // put your setup code here, to run once:
  xbee_init(0);  //初期化
  xbee_atcb(4);  //ネットワーク初期化
  xbee_atnj(0);  //孫機のジョイン拒否
  while (xbee_atai() > 0x01) { //ネットワーク参加状況を確認
    delay(3000);
    xbee_atcb(1);  //ネットワーク参加ボタン押下
  }
  g_gps.begin(GPSBAUDRATE);
}

void loop() {
  // put your main code here, to run repeatedly:
  struct GPS gps;
  AC ac;
  TM tm;
  while(!gps_get(&gps)){
    sno++;
    if((sno %   1000) == 0){
      xbee_gpslog(gps);
      get_ac();
      xbee_aclog(ac);
      get_tm();
      xbee_tmlog(tm);
    }
    delay(1);
  }

}
