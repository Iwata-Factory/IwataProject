/*
   test2

   SDカード読み取り関するテスト

*/

// include文
#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <SD.h>

//#define READBUFFERSIZE  (256)
//#define DELIMITER   (",")  // 区切り文字定数
//#define PIN_GPS_Rx  11 // GPSのシリアル通信でデータを受信するピン
//#define PIN_GPS_Tx  12 // GPSのシリアル通信でデータを送信するピン
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define GPSBAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define SS 53

const int chipSelect = 4;


// 構造体を宣言
typedef struct { // 2次元のベクトル
  double x; //2次元ベクトルのx座標
  double y; //2次元ベクトルのy座標
} Vector2D;

typedef struct { // 3次元のベクトル
  double x; //3次元ベクトルのx座標
  double y; //3次元ベクトルのy座標
  double z; //3次元ベクトルのz座標
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

void setup() {

  Wire.begin();           //I2C通信の初期化
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  // SDセットアップ
  pinMode(SS, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // 失敗、何もしない
    while (1);
  }
  Serial.println("ok");
  Serial.println("setup完了");
}

static int pos = 1;

void loop() {

  //  static int i = 1;
  //
  //  static AC ac;
  //
  //  if (i == 1) {
  //    ac.x = 0;
  //    ac.y = 100;
  //    ac.z = 50;
  //  }
  //
  //  int a = write_ac_sd(ac);
  //  Serial.println(a);
  //
  //  ac.x += 1;
  //  ac.y += 2;
  //  ac.z -= 1;
  //
  //  i += 1;
  //
  //  delay(3000);


  // 配列を用意
  // numは過去の何個分のデータが欲しいか
  int num = 15;
  AC ac[100];
  int a = read_ac_sd(ac, num);

  Serial.println(ac[0].x);
  Serial.println(ac[0].y);
  Serial.println(ac[0].z);
  Serial.println(ac[1].x);
  Serial.println(ac[1].y);
  Serial.println(ac[1].z);
  Serial.println(ac[2].x);
  Serial.println(ac[2].y);
  Serial.println(ac[2].z);
  Serial.println(ac[3].x);
  Serial.println(ac[3].y);
  Serial.println(ac[3].z);
  Serial.println(a);
  delay(5000);
  

  //Serial.println("111");

}



