/*
  メインプログラム
*/

// include文
#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <avr/sleep.h>
#include <SD.h>

// 定数の定義
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数
// GPSのピンはSDと干渉しないように
#define PIN_GPS_Rx  11 // GPSのシリアル通信でデータを受信するピン
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
#define BEAT 300   // 音の長さを指定
#define TONE_PINNO 12   // 圧電スピーカを接続したピン番号
#define C  262    //ド
#define D  294    //レ
#define E  330    //ミ
#define F  349
#define G  392
#define A  440
#define B  494
#define HIGH_C  523

//距離用の定義
#define DISTANCE A12
#define DIS_SAMPLE 50

// グローバル変数の定義
static unsigned long time; //タイマー起動
static float last_distance = -1; // 目的地までの距離(m)。負の値で初期化。
static const uint8_t length = 6;   //読み出しデータの個数


// 構造体を宣言
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
  pinMode(LIGHT_PIN, INPUT);

  pinMode(DISTANCE, INPUT);  //距離センサ用のアナログピン
  
  Serial.println("setup完了");

}

void loop() {

  Serial.println("loopスタート");
  Serial.println("光センサ起動まで5秒待機します");

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

  Serial.println("放出判定をパス");
  Serial.println("1秒待機します");

  // 投下中待機時間
  delay(1000); /* 現在適当な値 */

  while (1) { // 着陸の判定を行う
    break;
    static int i = 0; // 判定の繰り返し回数を調べる
    if (determine_landing()) {
      delay(5000);
      break;
    } else {
      i += 1;
      delay(5000);
    }
  }

  Serial.println("着陸判定をパス");
  Serial.println("3秒後に2秒回転します。");

  delay(3000);

  /* 着陸判定をパスしたら2秒回転します。 */
  digitalWrite(M1_1, 0);
  digitalWrite(M1_2, 1);
  digitalWrite(M2_1, 1);
  digitalWrite(M2_2, 0);
  delay(2000);
  // 停止
  digitalWrite(M1_1, 1);
  digitalWrite(M1_2, 1);
  digitalWrite(M2_1, 1);
  digitalWrite(M2_2, 1);

  Serial.println("回転終了");

  // need　ケーシングを開く処理を書く

  // need パラシュートから安全に離れる処理を書く

  Serial.println("Statusを地上1に移行します。");

  while (1) { // この部分をひたすらに繰り返す //===

    static int i = 0; // 繰り返し数のカウント

    double my_direction = -1; //自分の向いている方位（北を0として時計回りに0~360の値を取る）
    double dst_direction = -1; //目的地の方位。負の値で初期化。
    double my_rotation = 500; //自分が回転すべき大きさ(-180~180までの値を取る)

    Serial.println("GPSにより目的地データを取得します。");
    Serial.println("5サンプルを取得します。");

    //GPSから目的地までの距離と方角を得る
    while (1) { //!!!


      static int j = 0; // GPS受信の成功回数のカウント
      if (j > 0) { // 初期化
        j = 0;
      }
      static int k = 0; // GPS受信の試行数のカウント
      if (k > 0) { // 初期化
        k = 0;
      }

      double gps_direction_array[5]; // サンプルを入れる箱
      double gps_distance_array[5]; // サンプルを入れる箱
      double distance_hold = -1;   //gosの距離の値の前回のやつを保持

      while (j < 5) { // 成功サンプルを5個取得したい

        struct GPS gps; // 構造体宣言


   
        
        while (!gps_get(&gps)) { //gpsの値が正常になるまで取り続ける
          delay(50);
        }

        if (gps.Direction >= 0 && gps.distance >= 0) {
          gps_direction_array[j] = gps.Direction;
          gps_distance_array[j] = gps.distance;
          delay(10);
          j += 1;
          k += 1;

          Serial.print(j - 1);
          Serial.println("番目のサンプル取得");

        } else {
          delay(10);
          k += 1;
        }
      }

      Serial.println("サンプルの処理を行います");
      delay(1500);

      // gps_direction_arrayを投げて向きの平均を計算
      dst_direction = rad_ave(5, gps_direction_array); /*注意:引数の渡し方検討*/
      // gps_distance_arrayを投げて向きの平均を計算
      last_distance = value_ave(5, gps_distance_array); /*注意:引数の渡し方検討*/

      distance_hold = last_distance;

      Serial.print("dst_direction:");
      Serial.println(dst_direction);
      Serial.print("last_distance:");
      Serial.println(last_distance);

      break;
    } //!!!

    dst_direction = 100;
    last_distance = 1000;


    // 5回以内の回転で位置補正

    Serial.println("回転フローへ移行");

    delay(1500);

    int m = 0;
    while (m < 5) { //$$$

      //自分が向いている角度を取得
      while (1) { //$%$

        Serial.println("自身の方角を5サンプル取得します。");

        static int j = 0; // 方角取得の成功数をカウント
        if (j > 0) { // 初期化
          j = 0;
        }
        static int k = 0; // 方角取得の試行数をカウント
        if (k > 0) {
          k = 0;
        }

        double my_direction_array[5]; // 自身の方角を格納、中央値を使う

        while (j < 5) { // 5個の方位のサンプルを取得
          my_direction_array[j] = get_my_direction();

          if (my_direction_array[j] >= 0 && my_direction_array[j] <= 360) { // 正しく取れていればmy_directionは0~360
            j += 1;
            k += 1;
            Serial.print(j);
            Serial.print("個目のサンプルの値:");
            Serial.println(my_direction_array[j - 1]);
            delay(1500);

          } else {
            k += 1;
            delay(1500);
          }
        }

        // my_direction_arrayの中央値を取得
        my_direction = value_ave(5, my_direction_array);

        Serial.println("サンプル取得完了");
        Serial.print("方向の平均値は");
        Serial.println(my_direction);
        delay(1500);

        break;


      } //$%$


      // 必要な回転量を計算する(-180~180で出力)
      Serial.println("回転量を計算します。");

      while (1) {
        delay(1500);


        //相対偏角
        double relative_args = 0; // 自分の位置が基準

        // Vector2D型のベクトルを定義
        Vector2D my_vector;
        my_vector.x = cos(rad2deg(my_direction));
        my_vector.y = sin(rad2deg(my_direction));
        Vector2D dst_vector;
        dst_vector.x = cos(rad2deg(dst_direction));
        dst_vector.y = sin(rad2deg(dst_direction));

        // 偏角を計算
        double my_args = atan2(my_vector.y, my_vector.x);
        double dst_args = atan2(dst_vector.y, dst_vector.x);

        if (dst_args - my_args >= 0) {
          relative_args = rad2deg(dst_args - my_args);
        } else {
          relative_args = rad2deg(dst_args + 2 * pi - my_args); // M_PIはπ
        }

        Serial.print("相対偏角は");
        Serial.println(relative_args);

        // 内積を計算(単位ベクトル同士だからこれがcosθ)
        double inner_product = my_vector.x * dst_vector.x + my_vector.y * dst_vector.y;
        my_rotation = rad2deg(acos(inner_product)); //(初期値は500だがこれによって0~180に収まる)

        Serial.print("必要な回転量の絶対値は");
        Serial.println(my_rotation);


        // ここでmy_rotationを-180~180に直す(どちら向きの回転が早いか)
        if (relative_args < 10 || 350 <= relative_args) { // 方角がほぼ問題ないなら回転しないようにする。
          my_rotation = 0;
        } else if (10 <= relative_args && relative_args < 180) { //この時は正方向(右向き)　の回転が早い
          my_rotation = my_rotation;
        } else if (180 <= relative_args && relative_args < 350) {
          my_rotation = -1 * my_rotation;
        }

        Serial.print("計算を施して");
        Serial.println(my_rotation);

        break; /*現状ではここをwhile文にする理由は無いが念のため*/
      }


      //ここに回転部分を書く
      if (!(my_rotation == 0)) { // 回転する人用があれば回転し、向きの取得から繰り返す
        Serial.println("回転します");
        delay(1500);
        go_rotate(my_rotation);
        m += 1;
        Serial.print(m);
        Serial.println("回目の回転を終えました。");
        delay(1500);
      } else {
        Serial.println("回転は不要です");
        delay(1500);
        break; // 直進部分へ移行
      }
    } //$$$

    Serial.println("直進します。");
    delay(1000);
    //直進する
    go_straight(5000); /* 引数は暫定です */
    i += 1; // 繰り返し数を1増やす
    Serial.println("直進完了です。");
    Serial.println("whileの先頭に戻ります。");
    delay(1500);


  } //===

  // ここに来たらゴール近傍

}

