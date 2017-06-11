/*-----------GPS関連--------------------
   ここの欄は後で補完
  //こっから下は確認用+使い方、あとで消していいよ
  Serial.println('\n');
  Serial.println('以下gps構造体の中身表示');
  Serial.println(gps.latitude);
  Serial.println(gps.longitude);
  Serial.println(gps.utc);
  Serial.println(gps.Speed);
  Serial.println(gps.course);
  Serial.println(gps.Direction);
  Serial.println(gps.distance);
  ------------------------------------------*/

// 区切り文字定数
// センテンスの解析。
// $GPRMCの場合、引数変数に、緯度、経度を入れ、戻り値 1 を返す。
// $GPRMC以外の場合、戻り値は 0 を返す。

//static const int READBUFFERSIZE = 256;
//static const char DELIMITER = ",";

int AnalyzeLineString( char szLineString[], GPS* gps) {

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
  char* gps_status = strtok( NULL, DELIMITER );  // ステータスを抽出
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
  /*
     通信ができていても生データが通信状況悪いとVになります
     これが出る場合は屋外とか通信状況よくなるようにしてください
  */
  if ( strncmp(*gps_status, 'V', 1 ) == 0) {
    Serial.println("通信状況が悪いから歩こう");
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
  gps->longitude = deg + min / 60;

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

int gps_data_get(GPS* gps) {

  //  char g_szReadBuffer[READBUFFERSIZE] = "";
  //  int  g_iIndexChar = 0;
  char szLineString[READBUFFERSIZE];

  if ( !ReadLineString( g_gps,
                        g_szReadBuffer, READBUFFERSIZE, g_iIndexChar,
                        szLineString, READBUFFERSIZE ) )
  { // 読み取り途中
    return 2;
  }
  // 読み取り完了

  if ( !AnalyzeLineString( szLineString, gps ) )
  {
    return 3;
  }

  //緯度経度が明らかにおかしい場合はじく
  if (LATITUDE_MINIMUM < (gps->latitude) && LATITUDE_MAXIMUM > (gps->latitude)) { //緯度の検査域にいるか
    if (  LONGITUDE_MINIMUM < (gps->longitude) && LONGITUDE_MAXIMUM > (gps->longitude)) { //経度の検査域にいるか
    } else {
      return 4;
    }
  } else {
    return 4;
  }
  // 緯度、経度を読み取れた。
  // float to string
  char sz_utc[16];
  char sz_lat[16];
  char sz_long[16];
  //小数点6けたで表示する
  dtostrf(gps->utc, 10, 6, sz_utc);
  dtostrf(gps->latitude, 10, 6, sz_lat);
  dtostrf(gps->longitude, 10, 6, sz_long);

  Serial.print("utc : ");
  Serial.println(sz_utc);
  Serial.print("latitude : ");
  Serial.println(sz_lat);
  Serial.print("longitude : ");
  Serial.println(sz_long);
  Serial.print("Speed : ");
  Serial.println(gps->Speed);   //knot表示されます
  Serial.print("Course : ");
  Serial.println(gps->course);
  float LatA = GOAL_LATITUDE, LongA = GOAL_LONGITUDE;      //目的地
  //  float LatA = 35.710039, LongA = 139.810726;      //目的地
  float LatB = gps->latitude;       //現在地の緯度経度
  float LongB = gps->longitude;
  float direct = 0, distance = 0;   //目的地までの距離方角
  //目的地への距離方角算出
  distance = sqrt(pow(LongA - LongB, 2) + pow(LatA - LatB, 2)) * 99096.44, 0;
  direct = (int)(atan2((LongA - LongB) * 1.23, (LatA - LatB)) * 57.3 + 360) % 360;

  Serial.print("Direction = ");                               //目的地Aの方角(°）
  Serial.print(direct);
  Serial.print("Distance = ");                             //目的地A迄の距離(m)
  Serial.print(distance);
  Serial.println("m");
  //以下loop関数に値渡しする
  gps->Direction = direct;
  gps->distance = distance;

  return 1;
}

int gps_get(GPS* gps) {
  while (1) { //gpsの値が正常になるまで取り続ける
    int gps_flag = 0;   //gps_getの返り値保存
    gps_flag = gps_data_get(gps);
    delay(10);
    //gpsの値が取れない間どこで引っかかっているのか識別できるようになりました
    if (gps_flag == 1) { //値が取れたら抜ける
      break;
    }
    if (gps_flag == 2) {
      ;
      //      gpsとの通信が来ていない
      Serial.println("gpsとの通信できていない");
    }
    if (gps_flag == 3) {
      ;
      //gpsとの通信はできているが値が変or GPRMCでない
      Serial.println("gpsの値がおかしい or GPRMCではない");
    }
    if (gps_flag == 4) {
      ;
      //通信ができて値も解析されたが緯度経度の値がバグってる
      Serial.println("緯度経度がおかしい");
    }
  }

}

/*-----------get_ac()--------------------
   加速度センサーの値を取得
   返り値はAC型
  ------------------------------------------*/
AC get_ac() {
  byte ac_axis_buff[6]; //ADXL345のデータ格納バッファ(各軸あたり2つずつ)
  AC ac; // 初期化
  ac.x = 100; // 失敗時は(100, 100, 100)を返す
  ac.y = 100;
  ac.z = 100;

  if (readI2c(0x32, length, ac_axis_buff, ADXL345)) { //ADXL345のデータ(加速度)取得
    ac.x = double((((int)ac_axis_buff[1]) << 8) | ac_axis_buff[0]);     //MSBとLSBの順番も逆になっている
    ac.y = double((((int)ac_axis_buff[3]) << 8) | ac_axis_buff[2]);
    ac.z = double((((int)ac_axis_buff[5]) << 8) | ac_axis_buff[4]);
  } else {
    ; // 何もしない
  }
  return ac;
}

// 自身の方向取得関数を書くファイル
/*-----------get_tm()--------------------
   地磁気センサーの値を取得
   返り値はTM型
  ------------------------------------------*/
TM get_tm() {
  byte tm_axis_buff[6]; //HMC5883Lのデータ格納バッファ(各軸あたり2つずつ)
  TM tm; //初期化
  tm.x = 100; // 失敗時は(100, 100, 100)を返す
  tm.y = 100;
  tm.z = 100;

  if (readI2c(0x03, length, tm_axis_buff, HMC5883L)) {   //HMC5883Lのデータ(地磁気取得)
    tm.x = (((int)tm_axis_buff[0]) << 8) | tm_axis_buff[1];  //x軸のデータは2バイト分であるMSBは8ビット左シフトさせる
    tm.z = (((int)tm_axis_buff[2]) << 8) | tm_axis_buff[3];  //z軸
    tm.y = (((int)tm_axis_buff[4]) << 8) | tm_axis_buff[5];  //y軸
  } else {
    ; //何もしない
  }
  return tm;
}

/*-----------get_my_direction()--------------------
  自分の向いている方向
  成功したらその値を
  失敗したら負の値を返却
  ------------------------------------------*/

double get_my_direction() {

  double my_direction = -1.0;  // 返り値
  double direction_array[10] = { -1.0}; // 自身の方向を10サンプル取得する

  int error_c = 0;  // 何回地磁気取得に失敗したか

  TM tm;  // 地磁気型
  Vector2D tm_v;  // 地磁気ベクトル
  Vector2D s;  // 基準ベクトル

  Serial.println("自身の方向のサンプルを取得します");
  for (int i = 0; i < 10; i++) {
    error_c = 0;
    do {
      delay(200);

      s.x = 1.0;
      s.y = 0.0;

      tm.x = 100; tm.y = 100; tm.z = 100;
      tm = get_tm();

      tm_v.x = 2 * (tm.x - tm_x_offset) / x_def;
      tm_v.y = 2 * (tm.y - tm_y_offset) / y_def;

      double tm_v_size = vector2d_size(tm_v);

      tm_v.x = tm_v.x / tm_v_size;  // tm_vの大きさは1
      tm_v.y = tm_v.y / tm_v_size;

      double inner_product = vector2d_inner(tm_v, s);  // 内積を取る
      double tm_degree = rad2deg(acos(inner_product));  // 角度を得る(0~π)

      // 角度が0~360で出るように調整
      if (tm_v.y < 0) {
        tm_degree = int(360 - tm_degree + TM_DIFFERENCE) % 360;
      } else {
        tm_degree = int(tm_degree + TM_DIFFERENCE) % 360;
      }
      if (tm_degree < 90) {
        tm_degree = tm_degree - 90 + 360;
      } else {
        tm_degree =  tm_degree - 90;
      }
      Serial.print("サンプル");
      Serial.print(i + 1);
      Serial.print(":");
      Serial.println(tm_degree);

      direction_array[i] = tm_degree;  // 外れ値処理のためにradに再変換

      error_c += 1;

      if (error_c == 100) {  // 100回連続で取得失敗したら失敗を返す
        return -1;
      }

    } while (tm.x == 100 || tm.y == 100 || tm.z == 100);
  }
  Serial.println("解析します。");
  my_direction = degree_out(10, direction_array);  // 10サンプルから平均を計算
  //my_direction = rad2deg(my_direction);  // radからdegへ
  Serial.print("機体の方向は");
  Serial.println(my_direction);

  return my_direction;  // 単位はdeg
}


/*-----------turn_target_direction()--------------------
  ターゲットの方向を向く
  戻り値
  成功:1
  失敗:0
  ------------------------------------------*/

int turn_target_direction(double target_direction, double *my_Direction) {

  int i = 0;  // 回転の試行回数をカウントしていく

  do {

    delay(1000);
    i += 1;
    Serial.println("自身の方向を取得します。");
    double dir_result = get_my_direction(); // 自身の方向を取得(deg)。target_directionもdeg

    if (dir_result != -1) {
      *my_Direction = dir_result;
    } else {
      return 0;
    }

    double rotate_angle = 0;  // 回転量
    double a_difference = *my_Direction - target_direction;

    Serial.print("a_difference");
    Serial.println(a_difference);

    if (180 <= a_difference) {
      rotate_angle = 360 - a_difference;  // 右回転
    } else if (30 <= a_difference && a_difference < 180) {
      rotate_angle = -a_difference;  // 左回転
    } else if (-30 <= a_difference && a_difference < 30) {
      rotate_angle = 0;  // 回転しない
      Serial.println("機体方向が許容範囲内にあります。");
      return 1;  // 回転に成功
    } else if (-180 <= a_difference && a_difference < -30) {
      rotate_angle = -a_difference;  // 右回転
    } else {
      rotate_angle = 360 + a_difference;  // 左回転
    }

    Serial.print("必要な回転量は。");
    Serial.println(rotate_angle);
    Serial.println("回転します。");
    go_rotate(rotate_angle);  // 回転を行う

  } while (i < 10); // 10回回転してもダメだったら失敗
  return 0;
}

/*-----------tm_calibration()--------------------
   地磁気のキャリブレーション
   戻り値
   1:成功
   0:失敗
  ------------------------------------------*/


int tm_calibration() {

  Serial.println("キャリブレーションを行います。");

  delay(500);

  // 値を定義しておく
  DRIVE turn; // DRIVE型の宣言
  // 初期化
  turn.right1 = 0;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 0;

  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double min_z;
  double max_z;

  TM tm;

  rover_degital(turn); // 回転開始
  Serial.println("サンプル取得開始");

  for (int i = 0; i < 2500; i++) {

    delay(10);

    tm = get_tm();

    if (i == 0) {
      min_x = tm.x;
      max_x = tm.x;
      min_y = tm.y;
      max_y = tm.y;
      min_z = tm.z;
      max_z = tm.z;
    } else {
      if (tm.x < min_x) {
        min_x = tm.x;
      } else if (max_x < tm.x) {
        max_x = tm.x;
      }
      if (tm.y < min_y) {
        min_y = tm.y;
      } else if (max_y < tm.y) {
        max_y = tm.y;
      }
      if (tm.z < min_z) {
        min_z = tm.z;
      } else if (max_z < tm.z) {
        max_z = tm.z;
      }
    }
  }

  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;

  rover_degital(turn); // 回転終了

  // 最大値と最小値の差を求める
  x_def = max_x - min_x;
  y_def = max_y - min_y;
  // オフセットを計算
  tm_x_offset = (max_x + min_x) / 2;
  tm_y_offset = (max_y + min_y) / 2;

  delay(500);
  Serial.print("x_def:");
  Serial.println(x_def);
  Serial.print("y_def:");
  Serial.println(y_def);
  Serial.print("tm_x_offset:");
  Serial.println(tm_x_offset);
  Serial.print("tm_y_offset:");
  Serial.println(tm_y_offset);


  Serial.println("キャリブレーション完了。");


  return 1;
}


