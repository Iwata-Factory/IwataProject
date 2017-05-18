#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>

/*
  定数の定義
*/
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



/* unsigned long型の変数は32ビット(4バイト)の数値を格納します。通常のlong型と違い、負の数は扱えません。 */
unsigned long time; //タイマー起動

SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx);  //50,51をシリアル通信用に

char g_szReadBuffer[READBUFFERSIZE] = "";
int  g_iIndexChar = 0;

float last_distance = -1; // 目的地までの距離(m)。負の値で初期化。

byte tm_axis_buff[6]; //HMC5883Lのデータ格納バッファ(各軸あたり2つずつ)
byte ac_axis_buff[6]; //ADXL345のデータ格納バッファ(各軸あたり2つずつ)

// Vector2Dの型枠を宣言
struct Vector2D {
  double x; //2次元ベクトルのx座標
  double y; //2次元ベクトルのx座標
};

// GPSの型枠を宣言
struct GPS {
  float utc = 0.0;       //グリニッジ天文時
  float latitude = 0.0;   //経度
  float longtitude = 0.0;   //緯度
  float Speed = 0.0;    //移動速度
  float course = 0.0;    //移動方位
  float Direction = -1.0;   //目的地方位
  float distance = -1.0;     //目的地との距離
  /*Speedとdistanceは小文字が予約語だったのでとりあえず大文字にしてあります*/
};

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
  /* unit8_t 変数型の別名定義で、8ビット長の無符号整数を宣言する*/
  uint8_t length = 6;   //読み出しデータの個数

  delay(5000);

  // 光センサ始動
  while (1) {

    break;

    time = millis(); //現在の時間を取得
    if (1) { //投下の判定
      delay(1000);
      break; //パスしたらループを抜ける
    } else {
      delay(1000); //パスしていない場合1秒待って再判定
      continue;
    }
  }

  // ある程度待つ
  delay(1000);

  while (1) { //着陸判定

    break;

    time = millis(); //現在時刻取得

    int ac_x, ac_y, ac_z = 0; //加速度
    // 要素数10の配列を宣言(加速度を格納)
    double ac_array[10];
    // 加速度の和と平均
    double ac_sum = 0;
    double ac_ave = 0;

    // 加速度のサンプルを10個取る
    int k = 0;
    while (k < 10) {
      if (readI2c(0x32, length, ac_axis_buff, ADXL345)) { //ADXL345のデータ(加速度)取得

        ac_x = double((((int)ac_axis_buff[1]) << 8) | ac_axis_buff[0]);     //MSBとLSBの順番も逆になっている
        ac_y = double((((int)ac_axis_buff[3]) << 8) | ac_axis_buff[2]);
        ac_z = double((((int)ac_axis_buff[5]) << 8) | ac_axis_buff[4]);

        // 加速度の大きさを計算
        ac_array[k] = sqrt(pow(ac_x, 2) + pow(ac_y, 2) + pow(ac_z, 2));

        k += 1;
        delay(6000);

      } else {
        Serial.println("Failed: get ac");
        // 加速度を取得出来ないときの処理
        delay(1000);
        continue;
      }
    }

    // 着陸したかの判定(暫定として加速度を用いる)
    // 平均を算出
    for (int l = 0; l < 10; ++l) {
      ac_sum += ac_array[l];
    }
    ac_ave = ac_sum / 10;

    if (9.5 <= ac_ave && ac_ave <= 10.3) {
      break; //パスしたらループを抜ける
    } else {
      continue;
    }
  }

  digitalWrite(M1_1, 0);
  digitalWrite(M1_2, 1);
  digitalWrite(M2_1, 0);
  digitalWrite(M2_2, 1);
  delay(2000);

  // 停止

  digitalWrite(M1_1, 1);
  digitalWrite(M1_2, 1);
  digitalWrite(M2_1, 1);
  digitalWrite(M2_2, 1);

  // ケーシングを開く処理を書く

  // パラシュートから安全に離れる処理を書く

  //GPS取得から距離と方位取得→回転→移動
  int i = 0;


  while ((last_distance < 0) || (last_distance > 10)) {

    struct GPS gps;
    double my_direction = -1; //方位。負の値で初期化。
    double dst_direction = -1; //目的地の方位。負の値で初期化。
    double my_rotation = 500; //回転方向(-180~180になるはず)


    //GPSから目的地までの距離と方角を得る。(これに関してはi=0でのみ行う)
    if (i == 0) {
      while (1) {
        // 具体的な処理
        time = millis(); //現在時刻取得
        while (!gps_get(&gps)) {
          delay(50);
        }

        if (gps.Direction >= 0 && gps.distance >= 0) {
          delay(1000);
          break;
        } else {
          delay(1000);
          continue;
        }
      }
    }

    // 目的地の方角と残り距離を代入
    dst_direction = gps.Direction;
    last_distance = gps.distance;


    //自分が向いている角度を計算してどれだけ回転しなければならないか計算し回転
    int j = 0;
    while (j < 5 && (((-180 <= my_rotation) && (my_rotation < -15)) || (15 < my_rotation))) {

      int ac_x, ac_y, ac_z = 0; //加速度
      int tm_x, tm_y, tm_z = 0; //地磁気

      //加速度を取得する部分
      while (1) {
        if (readI2c(0x32, length, ac_axis_buff, ADXL345)) { //ADXL345のデータ(加速度)取得
          ac_x = (((int)ac_axis_buff[1]) << 8) | ac_axis_buff[0];     //MSBとLSBの順番も逆になっている
          ac_y = (((int)ac_axis_buff[3]) << 8) | ac_axis_buff[2];
          ac_z = (((int)ac_axis_buff[5]) << 8) | ac_axis_buff[4];
          break;
        } else {
          Serial.println("Failed: get ac");
          continue;
        }
      }

      // 地磁気を取得する部分
      while (1) {
        if (readI2c(0x03, length, tm_axis_buff, HMC5883L)) {   //HMC5883Lのデータ(地磁気取得)
          tm_x = (((int)tm_axis_buff[0]) << 8) | tm_axis_buff[1];  //x軸のデータは2バイト分であるMSBは8ビット左シフトさせる
          tm_z = (((int)tm_axis_buff[2]) << 8) | tm_axis_buff[3];  //z軸
          tm_y = (((int)tm_axis_buff[4]) << 8) | tm_axis_buff[5];  //y軸
          break;
        } else {
          Serial.println("Failed: get tm");
          continue;
        }
      }


      // 方角を算出
      while (1) {
        my_direction = get_my_direction(ac_x, ac_y, ac_z, tm_x, tm_y, tm_z);
        // 正しく方角が取れているのか
        if (!(my_direction >= 0)) {
          Serial.println("Failed: get direction");
          continue;
        }

        // 必要な回転量を計算する(-180~180で出力)
        // ベクトルの定義

        struct Vector2D my_vector {
          cos(rad2deg(my_direction)), sin(rad2deg(my_direction))
        };

        struct Vector2D dst_vector {
          cos(rad2deg(dst_direction)), sin(rad2deg(dst_direction))
        };

        // 内積を計算(単位ベクトル同士だからこれがcosθ)
        double inner_product = my_vector.x * dst_vector.x + my_vector.y * dst_vector.y;

        
        my_rotation = acos(inner_product);

        // 0~180に直す
        my_rotation = my_rotation * 180.0 / PI;

        if (0 <= my_rotation && my_rotation <= 10) { //10度以内なら回転しない
          my_rotation = 0;
          break;
        } else if ((int(my_direction + 180) % 360) < dst_direction) { //負の回転の方が早いとき
          my_rotation = -1 * my_rotation;
          break;
        } else {
          my_rotation = my_rotation;
          break;
        }
      }
      // 具体的な処理
      // 回転角が正常な範囲にあるか計算
      if (-180 <= my_rotation && my_rotation <= 180) {
        break;
      } else {
        continue;
      }
      // 回転をする
      go_rotate(my_rotation);
      j += 1;
    }

    while (1) { //進む

      // 具体的な処理
      // 一定時間経ったらbreak
      // スタック回避とかも考える
      go_straight(1000);

      break;
    }

    while (1) {
      // 具体的な処理
      time = millis(); //現在時刻取得
      gps_get(&gps);

      if (gps.Direction >= 0 && gps.distance >= 0) {
        break;
      } else {
        continue;
      }
    }

    // 目的地の方角と残り距離を代入
    dst_direction = gps.Direction;
    last_distance = gps.distance;
    // 繰り返し数をカウント
    i += 1;
  }

}

//ここまで来ればゴールは近いので頑張る





/*------------I2C書き込み-------------------
   register_addr:HMC5883L内のレジスタ番地
   value:レジスタに書き込むデータ
   slave_addr:通信したいスレーブのアドレス
   　
   成功した場合は1
   失敗した場合は0
   を返す
  ------------------------------------------*/

int writeI2c(byte register_addr, byte value, byte slave_addr) {

  Serial.println("writeI2c run");

  Wire.beginTransmission(slave_addr);
  Wire.write(register_addr);
  Wire.write(value);
  int flag = Wire.endTransmission();

  if (flag == 0) { // 成功したか確認
    return 1;
  } else {
    Serial.println(flag);
    return 0;
  }
}


/*-----------I2C読み込み--------------------
   register_addr:HMC5883L内のレジスタ番地
   num: 読み出すデータの個数(連続データ)
   buffer:読み出したデータの格納用バッファ
   slave_addr:通信したいスレーブのアドレス

   成功したら1
   失敗したら0
   を返す
  ------------------------------------------*/
int readI2c(byte register_addr, int num, byte buffer[], byte slave_addr) {
  Wire.beginTransmission(slave_addr);
  Wire.write(register_addr);
  Wire.endTransmission();
  Wire.beginTransmission(slave_addr);
  Wire.requestFrom(slave_addr, num); //受信シーケンスを発行しデータを読み出す

  int i = 0;
  while (Wire.available()) { // 受信バッファ内にあるデータを調べbufferに格納
    buffer[i] = Wire.read();
    i++;
  }

  int flag = Wire.endTransmission();

  if (flag == 0) { // 成功したか確認
    return 1;
  } else {
    Serial.println(flag);
    return 0;
  }
}

/*-----------GPS関連--------------------
   ここの欄は後で補完
  //こっから下は確認用+使い方、あとで消していいよ
  Serial.println('\n');
  Serial.println('以下gps構造体の中身表示');
  Serial.println(gps.latitude);
  Serial.println(gps.longtitude);
  Serial.println(gps.utc);
  Serial.println(gps.Speed);
  Serial.println(gps.course);
  Serial.println(gps.Direction);
  Serial.println(gps.distance);
  ------------------------------------------*/
// センテンスの解析。
// $GPRMCの場合、引数変数に、緯度、経度を入れ、戻り値 1 を返す。
// $GPRMC以外の場合、戻り値は 0 を返す。
int AnalyzeLineString( char szLineString[], struct GPS* gps) {

  // $GPRMC
  if ( 0 != strncmp( "$GPRMC", szLineString, 6 ) )
  {
    return 0;
  }
  //strtok(char* s1,char* s2):s1文字列内の文字をs2文字列で区切って抽出したトークンの戦闘アドレスを返す
  //2回目以降はs1にNULLを指定して連続
  // $GPRMC,085120.307,A,3541.1493,N,13945.3994,E,000.0,240.3,181211,,,A*6A
  strtok( szLineString, DELIMITER );  // $GPRMCを抽出
  char* psz_utc = strtok( NULL, DELIMITER );  // UTC時刻を抽出
  strtok( NULL, DELIMITER );  // ステータスを抽出
  char* psz_lat = strtok( NULL, DELIMITER ); // 緯度(dddmm.mmmm)
  strtok( NULL, DELIMITER );  // 北緯か南緯か
  char* psz_long = strtok( NULL, DELIMITER );  // 経度(dddmm.mmmm)
  strtok(NULL, DELIMITER);    //東経か西経か
  char* psz_Speed = strtok(NULL, DELIMITER);   //移動速度
  char* psz_course = strtok(NULL, DELIMITER);  //移動の真方位

  if ( NULL == psz_long )
  {
    return 0;
  }
  gps->utc = atof(psz_utc);
  gps->Speed = atof(psz_Speed);
  gps->course = atof(psz_course);

  // dddmm.mmmm → ddd.dddddd
  float temp, deg, min;
  temp = atof(psz_lat);   //char → float
  deg = (int)(temp / 100); //度数
  min = temp - deg * 100;   //分
  gps->latitude = deg + min / 60;   //分→度

  temp = atof(psz_long);
  deg = (int)(temp / 100);
  min = temp - deg * 100;
  gps->longtitude = deg + min / 60;

  return 1;
}
// １行文字列の読み込み
// 0 : 読み取り途中。1 : 読み取り完了。
int ReadLineString( SoftwareSerial& serial,
                    char szReadBuffer[], const int ciReadBufferSize, int& riIndexChar,
                    char szLineString[], const int ciLineStringSize ) {
  while ( 1 )
  {
    char c = serial.read();
    if ( -1 == c )
    {
      break;
    }
    if ( '\r' == c  )
    { // 終端
      szReadBuffer[riIndexChar] = '\0';
      strncpy( szLineString, szReadBuffer, ciLineStringSize - 1 );
      szLineString[ciLineStringSize - 1] = '\0';
      riIndexChar = 0;
      return 1;
    }
    else if ( '\n' == c )
    {
      ; // 何もしない
    }
    else
    { // 途中
      if ( (ciReadBufferSize - 1) > riIndexChar )
      {
        szReadBuffer[riIndexChar] = c;
        riIndexChar++;
      }
    }
  }
  return 0;
}
boolean gps_get(struct GPS* gps) {



  char szLineString[READBUFFERSIZE];
  if ( !ReadLineString( g_gps,
                        g_szReadBuffer, READBUFFERSIZE, g_iIndexChar,
                        szLineString, READBUFFERSIZE ) )
  { // 読み取り途中
    return 0;
  }
  // 読み取り完了

  if ( !AnalyzeLineString( szLineString, gps ) )
  {
    return 0;
  }
  //緯度経度が正常な値にあるか広めに検査

  // 緯度、経度を読み取れた。
  // float to string
  char sz_utc[16];
  char sz_lat[16];
  char sz_long[16];
  //小数点6けたで表示する
  dtostrf(gps->utc, 10, 6, sz_utc);
  dtostrf(gps->latitude, 10, 6, sz_lat);
  dtostrf(gps->longtitude, 10, 6, sz_long);

  Serial.print("utc : ");
  Serial.println(sz_utc);
  Serial.print("latitude : ");
  Serial.println(sz_lat);
  Serial.print("longtitude : ");
  Serial.println(sz_long);
  Serial.print("Speed : ");
  Serial.println(gps->Speed);   //knot表示されます
  Serial.print("Course : ");
  Serial.println(gps->course);
  float LatA = 35.710039, LongA = 139.810726;      //目的地
  float LatB = gps->latitude;       //現在地の緯度経度
  float LongB = gps->longtitude;
  float direct = 0, distance = 0;   //目的地までの距離方角
  //目的地への距離方角算出
  distance = sqrt(pow(LongA - LongB, 2) + pow(LatA - LatB, 2)) * 99096.44, 0;
  direct = (int)(atan2((LongA - LongB) * 1.23, (LatA - LatB)) * 57.3 + 360) % 360;

  Serial.print("Direction = ");                               //目的地Aの方角(°）
  Serial.print(direct);
  Serial.print("deg:Distance = ");                             //目的地A迄の距離(m)
  Serial.print(distance);
  Serial.println("m");
  //以下loop関数に値渡しする
  gps->Direction = direct;
  gps->distance = distance;

  return 1;
}

/*-----------方角算出--------------------
  x, y, z: 各センサで取得した値
  加速度センサの値からオイラー角を計算して座標を変換する。
  ------------------------------------------*/
double get_my_direction(int ac_x, int ac_y, int ac_z, int tm_x, int tm_y, int tm_z) {
  double roll = 0; //ロール角
  double pitch = 0; //ピッチ角
  double dir = 0; //方角
  double ac_xD = double(ac_x); //型変換
  double ac_yD = double(ac_y);
  double ac_zD = double(ac_z);
  double tm_xD = double(tm_x);
  double tm_yD = double(tm_y);
  double tm_zD = double(tm_z);
  double o_x = 0; //オフセット
  double o_y = 0;
  double o_z = 0;
  roll = atan2(ac_yD, ac_zD);
  pitch = atan2(ac_xD * (-1), ac_yD * sin(roll) + ac_zD * cos(roll));
  double numerator = (tm_zD - o_z) * sin(roll) - (tm_yD - o_y) * cos(roll); //分子
  double denominator = (tm_xD - o_x) * cos(pitch) + (tm_yD - o_y) * sin(pitch) * sin(roll) + (tm_zD - o_x) * sin(pitch) * cos(roll);
  dir = atan2(numerator, denominator) * RAD_TO_DEG + 180; //xy平面での向きを角度(°)で表示(北で0°)
  return (dir);
}




/*-----------radからdegに--------------------
  引数はdegree(double)
  ------------------------------------------*/
double rad2deg(double rad) {
  double deg = rad * 180.0 / PI;
  return (deg);
}

/*-----------degからradに--------------------
  引数はrad(double)
  ------------------------------------------*/
double deg2rad(double deg) {
  double rad = deg * PI / 180.0;
  return (rad);
}


/*-----------機体旋回--------------------
   rotate: 回転角
  ------------------------------------------*/
void go_rotate(double rotate) {
  Serial.print("rotate");


  // ピン出力の初期化
  int m1_1 = -1;
  int m1_2 = -1;
  int m2_1 = -1;
  int m2_2 = -1;

  // センサの値の初期化
  int ac_x, ac_y, ac_z = 0; //加速度
  int tm_x, tm_y, tm_z = 0; //地磁気

  int rotate_time = 0; // 開店時間

  // どちら向きに回転するかを判断
  if (0 < rotate) {
    m1_1 = 0;
    m1_2 = 1;
    m2_1 = 1;
    m2_2 = 0;
    rotate_time = (1000 * rotate) / 180;
  } else {
    m1_1 = 1;
    m1_2 = 0;
    m2_1 = 0;
    m2_2 = 1;
    rotate_time = -1 * (1000 * rotate) / 180;

  }
  // 回転を行う
  digitalWrite(M1_1, m1_1);
  digitalWrite(M1_2, m1_2);
  digitalWrite(M2_1, m2_1);
  digitalWrite(M2_2, m2_2);

  delay(rotate_time); /* ここは|rotate|に比例した何か */

  m1_1 = 1;
  m1_2 = 1;
  m2_1 = 1;
  m2_2 = 1;
  // 停止
  digitalWrite(M1_1, m1_1);
  digitalWrite(M1_2, m1_2);
  digitalWrite(M2_1, m2_1);
  digitalWrite(M2_2, m2_2);
  delay(1500);
}


/*-----------機体直進--------------------
   go_time: 進む時間
  ------------------------------------------*/
void go_straight(int go_time) {

  Serial.println("Straight");
  // ピン出力の初期化
  int m1_1 = 0;
  int m1_2 = 1;
  int m2_1 = 0;
  int m2_2 = 1;
  // 直進
  digitalWrite(M1_1, m1_1);
  digitalWrite(M1_2, m1_2);
  digitalWrite(M2_1, m2_1);
  digitalWrite(M2_2, m2_2);
  delay(go_time);

  // 停止
  m1_1 = 1;
  m1_2 = 1;
  m2_1 = 1;
  m2_2 = 1;
  digitalWrite(M1_1, m1_1);
  digitalWrite(M1_2, m1_2);
  digitalWrite(M2_1, m2_1);
  digitalWrite(M2_2, m2_2);
  delay(500);
}

