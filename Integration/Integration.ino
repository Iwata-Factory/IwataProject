/*
  メインプログラム
*/

// include文
#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>

// 定数の定義
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数
#define PIN_GPS_Rx  53 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_Tx  51 // GPSのシリアル通信でデータを送信するピン
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define GPSBAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define LATITUDE_MINIMUM 30  //緯度の最小値
#define LATITUDE_MAXIMUM 40  //緯度の最大値
#define LONGTITUDE_MINIMUM 130  //経度の最小値
#define LONGTITUDE_MAXIMUM 140  //経度の最大値
#define HMC5883L 0x1E   //HMC5883L(地磁気センサ)のスレーブアドレス
#define ADXL345 0x53  //ADXL345(加速度センサ)のスレーブアドレス
#define M1_1 8 // モーター制御用ピン
#define M1_2 9 // モーター制御用ピン
#define M2_1 10 // モーター制御用ピン
#define M2_2 11 // モーター制御用ピン

// グローバル変数の定義
static unsigned long time; //タイマー起動
static float last_distance = -1; // 目的地までの距離(m)。負の値で初期化。
static const uint8_t length = 6;   //読み出しデータの個数

// 構造体を宣言
struct Vector2D { // 2次元のベクトル
  double x; //2次元ベクトルのx座標
  double y; //2次元ベクトルのy座標
};

struct Vector3D { // 3次元のベクトル
  double x; //3次元ベクトルのx座標
  double y; //3次元ベクトルのy座標
  double z; //3次元ベクトルのz座標
};

struct GPS { // GPS関連
  double utc = 0.0;       //グリニッジ天文時
  double latitude = 0.0;   //経度
  double longtitude = 0.0;   //緯度
  double Speed = 0.0;    //移動速度
  double course = 0.0;    //移動方位
  double Direction = -1.0;   //目的地方位
  double distance = -1.0;     //目的地との距離
  /*Speedとdistanceは小文字が予約語だったのでとりあえず大文字にしてあります*/
};

struct AC { // 加速度センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
};

struct TM { // 地磁気センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
};

struct DRIVE { // モーター制御
  int right1 = 0; // 8番ピン対応
  int right2 = 0; // 9番ピン対応
  int leght1 = 0; // 10番ピン対応
  int leght2 = 0; // 11番ピン対応
};

// 50,51をArduinoとGPS間のシリアル通信用に
SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx);



/*
   セットアップ
*/
void setup() {
  Wire.begin();           //I2C通信の初期化
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  g_gps.begin(GPSBAUDRATE); //シリアル通信の初期化
  writeI2c(0x02, 0x00, HMC5883L); //HMC5883Lの初期設定0x02レジスタに0x00書き込み
  writeI2c(0x31, 0x00, ADXL345);  //上と同様
  writeI2c(0x2d, 0x08, ADXL345);  //上と同様
  pinMode(M1_1, OUTPUT);
  pinMode(M1_2, OUTPUT);
  pinMode(M2_1, OUTPUT);
  pinMode(M2_2, OUTPUT);
}

void loop() {
  delay(5000);

  // 光センサ始動(準備が整いしだい外部関数化)
  while (1) {
    break; /* 今は即breakさせています */
    time = millis(); //現在の時間を取得
    if (1) { //投下の判定
      delay(1000);
      break; //パスしたらループを抜ける
    } else {
      delay(1000); //パスしていない場合1秒待って再判定
      continue;
    }
  }

  // 投下中待機時間
  delay(1000); /* 現在適当な値 */

  while (1) { // 着陸の判定を行う
    static int i = 0; // 判定の繰り返し回数を調べる
    if (determine_landing()) {
      delay(5000);
      break;
    } else {
      i += 1;
      delay(5000);
    }
  }

  /* 着陸判定をパスしたら2秒回転します。 */
  digitalWrite(M1_1, 0);
  digitalWrite(M1_2, 1);
  digitalWrite(M2_1, 1);
  digitalWrite(M2_2, 0);
  delay(200);
  // 停止
  digitalWrite(M1_1, 1);
  digitalWrite(M1_2, 1);
  digitalWrite(M2_1, 1);
  digitalWrite(M2_2, 1);


  // need　ケーシングを開く処理を書く

  // need パラシュートから安全に離れる処理を書く

  while (1) {
    static int i = 0; // 繰り返し数のカウント

    static double my_direction = 1; //自分の向いている方位（北を0として時計回りに0~360の値を取る）
    static double my_rotation = 500; //自分が回転すべき大きさ(-180~180までの値を取る)

    //GPSから目的地までの距離と方角を得る
    while (1) {

      struct GPS gps; // 構造体宣言
      static int j = 0; // GPS受信の思考回数のカウント
      static int k = 0; // GPS受信の成功数のカウント

      double gps_direction_array[5]; // サンプルを入れる箱
      double gps_distance_array[5]; // サンプルを入れる箱

      while (!gps_get(&gps)) { //
        delay(50);
      }

      while (j < 5) { // 成功サンプルを5個取得したい
        if (gps.Direction >= 0 && gps.distance >= 0) {
          gps_direction_array[j] = gps.Direction;
          gps_distance_array[j] = gps.distance;
          delay(1000);
          j += 1;
          k += 1;
        } else {
          delay(1000);
          k += 1;
        }
      }
      
      // ここで平均値を取る
      // gps_direction_arrayを投げて向きの中央値を計算
      // gps_distance_arrayを投げて距離の中央値を計算
      
    }






  }




}
