/*-----------GPS関連--------------------
   ここの欄は後で補完
  AnalyzeLineString  GPSの生データのchar配列を処理、使える形にする
  ReadLineString   GPSのなまでーたを一文字読み取る
  get_data_gps   GPSのデータを処理構造体に入れる
  get_gps   GPS構造体を返す

  xbee_uart( dev,'\n');
  xbee_uart( dev,'以下gps構造体の中身表示');
  xbee_uart( dev,gps.latitude);
  xbee_uart( dev,gps.longitude);
  xbee_uart( dev,gps.utc);
  xbee_uart( dev,gps.Speed);
  xbee_uart( dev,gps.course);
  xbee_uart( dev,gps.Direction);
  xbee_uart( dev,gps.distance);
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
    xbee_uart( dev, "BAD communicatin condition of gps...\r");
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

  float LatA = GOAL_LATITUDE, LongA = GOAL_LONGITUDE;      //目的地

  //  float LatA = 35.710039, LongA = 139.810726;      //目的地
  float LatB = gps->latitude;       //現在地の緯度経度
  float LongB = gps->longitude;
  float direct = 0, distance = 0;   //目的地までの距離方角
  //目的地への距離方角算出
  distance = sqrt(pow(LongA - LongB, 2) + pow(LatA - LatB, 2)) * 99096.44, 0;
  direct = (int)(atan2((LongA - LongB) * 1.23, (LatA - LatB)) * 57.3 + 360) % 360;

  //以下loop関数に値渡しする
  gps->Direction = direct;
  gps->distance = distance;

  return 1;
}

int gps_get(GPS* gps) {
  xbee_uart(dev, "call gps_get\r");
  int t = 0;
  while (1) { //gpsの値が正常になるまで取り続ける
    int gps_flag = 0;   //gps_getの返り値保存
    gps_flag = gps_data_get(gps);
    delay(10);
    t++;
    //gpsの値が取れない間どこで引っかかっているのか識別できるようになりました
    if (gps_flag == 1) { //値が取れたら抜ける

      xbee_uart(dev, "LATI, LONG\r");
      xbee_send_2doubles(gps->latitude, gps->longitude);
      xbee_uart(dev, "DISTANCE, DIRECTION\r");
      xbee_send_2doubles(gps->distance, gps->Direction);

      break;
    }
    if (gps_flag == 2) {
      ;

      //xbee_uart( dev,"cant communicate with gps\r");

    }
    if (gps_flag == 3) {
      ;
      //gpsとの通信はできているが値が変or GPRMCでない

      //xbee_uart( dev, "gps wrong or not GPRMC\r");
    }
    if (gps_flag == 4) {
      ;
      speaker(E_TONE);
      speaker(F_TONE);
      speaker(E_TONE);

      //通信ができて値も解析されたが緯度経度の値がバグってる
      //xbee_uart( dev, "wrong Lat or Long\r");
    }
    if (t >= 10000) {
      //およそ100秒間取れなければ一旦抜ける
      break;
    }
  }
  return 1;
}


/*
   以下GPSの高度関数
*/

// 区切り文字定数
// センテンスの解析。
// $GPRMCの場合、引数変数に、緯度、経度を入れ、戻り値 1 を返す。
// $GPRMC以外の場合、戻り値は 0 を返す。

//static const int READBUFFERSIZE = 256;
//static const char DELIMITER = ",";

int AnalyzeLineString_al( char szLineString[], double* altitude) {

  // $GPGGA
  if ( 0 != strncmp( "$GPGGA", szLineString, 6 ) )
  {
    return 0;
  }
  //strtok(char* s1,char* s2):s1文字列内の文字をs2文字列で区切って抽出したトークンの戦闘アドレスを返す
  //2回目以降はs1にNULLを指定して連続
  // $GPGGA,085120.307,3541.1493,N,13945.3994,E,1,08,1.0,6.9,M,35.9,M,,0000*5E
  //UTC,緯度,北緯南緯,経度,東経西経,位置特定品質,使用衛星数,水平精度低下率,アンテナの海抜高さ,
  strtok( szLineString, DELIMITER );  // $GPGGAを抽出
  strtok( NULL, DELIMITER );  // UTC時刻を抽出
  strtok( NULL, DELIMITER ); // 緯度(dddmm.mmmm)
  strtok( NULL, DELIMITER );  // 北緯か南緯か
  strtok( NULL, DELIMITER );  // 経度(dddmm.mmmm)
  strtok(NULL, DELIMITER);    //東経か西経か
  char* psz_q = strtok(NULL, DELIMITER);   //位置特定品質
  strtok(NULL, DELIMITER);  //仕様衛星数
  strtok(NULL, DELIMITER);   //水平精度低下率
  char* psz_altitude = strtok(NULL, DELIMITER);   //アンテナの海抜高さ

  if ( NULL == psz_altitude )
  {
    return 0;
  }
  /*
     通信ができていても生データが通信状況悪いとVになります
     これが出る場合は屋外とか通信状況よくなるようにしてください
  */
  if ( strncmp(*psz_q, '0', 1 ) == 0) {
    xbee_uart( dev, "BAD communicatin condition of gps...\r");
    return 0;
  }
  *altitude = atof(psz_altitude);

  return 1;
}

int gps_data_get_al(double* altitude) {

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

  if ( !AnalyzeLineString_al( szLineString, altitude ) )
  {
    return 3;
  }

  // 緯度、経度を読み取れた。
  // float to string
  char sz_al[16];
  dtostrf(*altitude, 13, 6, sz_al);

  //xbee送信
  xbee_uart(dev, "get altitude:");
  xbee_uart(dev, sz_al);

  return 1;
}

/*
   gpsの高度を返します
   引数doubleのポインタで渡すとそれに高度を代入します
*/
int gps_get_al(double* altitude) {
  int t = 0;
  while (1) { //gpsの値が正常になるまで取り続ける
    int gps_flag = 0;   //gps_getの返り値保存
    gps_flag = gps_data_get_al(altitude);
    delay(10);
    t++;
    //gpsの値が取れない間どこで引っかかっているのか識別できるようになりました
    if (gps_flag == 1) { //値が取れたら抜ける
      break;
    }
    if (gps_flag == 2) {
      ;

      //xbee_uart( dev,"cant communicate with gps\r");

    }
    if (gps_flag == 3) {
      ;
      //gpsとの通信はできているが値が変or GPRMCでない

      //xbee_uart( dev, "gps wrong or not GPRMC\r");
    }
    if (gps_flag == 4) {
      ;
      speaker(E_TONE);
      speaker(F_TONE);
      speaker(E_TONE);

      //通信ができて値も解析されたが緯度経度の値がバグってる
      //xbee_uart( dev, "wrong Lat or Long\r");
    }
    if (t >= 10000) {
      //およそ100秒間ダメなら向ける
      break;
    }
  }
  return 1;
}


/*-----------get_rover_point(POINT *point)--------------------
   roverの現在位置を取得
  ------------------------------------------*/
int get_rover_point(POINT *point) {

  GPS grp_gps;
  gps_get(&grp_gps);
  point->latitude = grp_gps.latitude;
  point->longitude = grp_gps.longitude;
  return 1;

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

  xbee_uart( dev, " : call get_my_direction() ");

  double my_direction = -1.0;  // 返り値
  double direction_array[10] = { -1.0}; // 自身の方向を10サンプル取得する

  int error_c = 0;  // 何回地磁気取得に失敗したか

  TM tm;  // 地磁気型
  Vector2D tm_v;  // 地磁気ベクトル
  Vector2D s;  // 基準ベクトル

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

      direction_array[i] = tm_degree;  // 外れ値処理のためにradに再変換

      error_c += 1;

      if (error_c == 100) {  // 100回連続で取得失敗したら失敗を返す
        return -1;
      }

    } while (tm.x == 100 || tm.y == 100 || tm.z == 100);
  }

  my_direction = degree_out(10, direction_array);  // 10サンプルから平均を計算

  xbee_uart( dev, " : direction of rover is ");
  xbee_send_1double(my_direction);

  return my_direction;  // 単位はdeg
}


/*-----------turn_target_direction()--------------------
  ターゲットの方向を向く
  引数
  double target_direction : 目的地の方位
  double *my_direction  : rover.My_Direction 自分の方位のポインタ
  戻り値
  成功:1
  失敗:0
  ------------------------------------------*/

int turn_target_direction(double target_direction, double *my_Direction, int branch = 0) {

  xbee_uart( dev, " : call turn_target_direction() ");

  int i = 0;  // 回転の試行回数をカウントしていく

  do {

    delay(1000);
    i += 1;

    if (target_direction < 0 || 360 <= target_direction) { //target_directionが360以上の場合調整
      target_direction = (360 * 2 + (int)target_direction) % 360;
    }

    double dir_result = get_my_direction(); // 自身の方向を取得(deg)。target_directionもdeg

    if (dir_result != -1) {
      *my_Direction = dir_result;
    } else {
      return 0;
    }

    double rotate_angle = 0;  // 回転量
    double a_difference = *my_Direction - target_direction;

    if (180 <= a_difference) {
      rotate_angle = 360 - a_difference;  // 右回転
    } else if (20 <= a_difference && a_difference < 180) {
      rotate_angle = -a_difference;  // 左回転
    } else if (-20 <= a_difference && a_difference < 20) {
      xbee_uart( dev, " : success turn_target_direction() ");
      rotate_angle = 0;  // 回転しない

      return 1;  // 回転に成功
    } else if (-180 <= a_difference && a_difference < -20) {
      rotate_angle = -a_difference;  // 右回転
    } else {
      rotate_angle = 360 + a_difference;  // 左回転
    }

    xbee_uart(dev, "needed rotation is\r");
    xbee_send_1double(rotate_angle);
    if(branch == 0){
    rotate_angle = rotate_angle * (10 - i) / 10;  // 回転角度を収束させる
    go_rotate(rotate_angle);  // 回転を行う
    } else {//発散ver
      rotate_angle = rotate_angle * (10 * i) / 10;
      go_rotate(rotate_angle);
    }
  } while (i < 5); // 10回回転してもダメだったら失敗
  xbee_uart( dev, " : false turn_target_direction() ");
  return 0;
}

/*-----------tm_calibration()--------------------
   地磁気のキャリブレーション
   戻り値
   1:成功
   0:失敗
  ------------------------------------------*/


int tm_calibration() {

  xbee_uart( dev, " : call tm_calibration");

  AC ac_calib;  // キャリブレーション時の水平判定用
  int count_calib = 0;  // 非水平カウント用

  while (1) {

    ac_calib = get_ac();

    if ((fabs(ac_calib.x) < 50 && fabs(ac_calib.y) < 50 && 200 < ac_calib.z) || count_calib == 5) {  // 水平な感じの場所にいるならキャリブレーション。試行回数過多でもキャリブレーション
      speaker(C_TONE);
      speaker(D_TONE);

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

      delay(10000);  // 周辺の掃除

      for (int i = 0; i < 1500; i++) {

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

      xbee_uart( dev, " : x_def, y_def ");
      xbee_send_2doubles(x_def, y_def);
      xbee_uart( dev, " : tm_x_offset, tm_y_offset ");
      xbee_send_2doubles(tm_x_offset, tm_y_offset);
      delay(500);
      xbee_uart( dev, " : success tm_calibration");
      return 1;

    } else {
      count_calib += 1;
      go_straight(1500);  //水平な場所を目指す
    }
  }
}


/*-----------pid_get_control()--------------------
  ターゲットとの差を取得
  ------------------------------------------*/

double pid_get_control(double target_direction, double *my_Direction) {

  int i = 0;  // 回転の試行回数をカウントしていく


  i += 1;

  if (target_direction < 0 || 360 <= target_direction) { //target_directionが360以上の場合調整
    target_direction = (int)target_direction % 360;
  }

  double dir_result = get_my_direction(); // 自身の方向を取得(deg)。target_directionもdeg

  if (dir_result != -1) {  // 正しく角度が取れたのか確認
    *my_Direction = dir_result;
  } else {
    return 0.0;  // ダメだった場合は回転角度0
  }

  double rotate_angle = 0;  // 回転量
  double a_difference = *my_Direction - target_direction;

  if (180 <= a_difference) {
    rotate_angle = 360 - a_difference;  // 右回転
  } else if (0 <= a_difference < 180) {
    rotate_angle = -rotate_angle;  // 左回転
  } else if (-180 <= a_difference < 0) {
    rotate_angle = -rotate_angle;  // 右回転
  } else {
    rotate_angle = 360 + a_difference;  // 左回転
  }

  rotate_angle = -rotate_angle;
  return rotate_angle;  // 基準値との差を返す
}

int correct_posture() {
  xbee_uart( dev, "call correct_posture\r");

  for (int ji = 0; ji < 5; ji++) {  // 状態復旧
    if (judge_invered() == 1) {
      xbee_uart( dev, "success correct_posture\r");
      return 1;  // 問題なし
    } else {
      go_straight(5000);
    }
  }
  xbee_uart( dev, "failed correct_posture\r");
  return 0;  // 反転したまま
}


/*-----------judge_invered()--------------------
   戻り値
   1:異常なし
   0:ひっくりかえっってる
  ------------------------------------------*/

int judge_invered() {

  xbee_uart( dev, "call judge_invered\r");

  AC ac[10];  // サンプル数は10個とする
  double z[10] = {1.0};

  for (int i = 0; i < 10; i++) {
    ac[i] = get_ac();  // 加速度を取得
    z[i] = ac[i].z;  // zの値に着目
    delay(50);
  }
  double ac_z_ave =  value_ave(10, z);

  if (ac_z_ave < -1.0) {  // この式が真なら反転している。
    xbee_uart( dev, "revive ---> go straighr\r");
    return 0;
  } else {
    xbee_uart( dev, "success judge_invered_revive\r");
    return 1; // 問題なし
  }
}

/*-----------set_danger_area()--------------------
   引数の周囲10mを立ち入り禁止エリアに
   戻り値
   1:設定完了
   0:引数おかしい
  ------------------------------------------*/

int set_danger_area() {

  xbee_uart( dev, "call set_danger_area\r");

  /* GPSとれなかったら死ぬからそのままでも良いけどgps_getの無限ループは避けたいbyとうま */
  GPS danger_gps;
  gps_get(&danger_gps);

  for (int i = 0; i < 10; i++) {
    if ((danger_area_points[i].latitude == -1.0 && danger_area_points[i].longitude == -1.0)) {
      danger_area_points[i].latitude = danger_gps.latitude;
      danger_area_points[i].longitude = danger_gps.longitude;
      xbee_uart( dev, "success set_danger_area\r");
      return 1;  // 登録完了
    }
  }
  xbee_uart( dev, "false set_danger_area\r");
  return 0;  // 登録が10箇所埋まっている
}


/*-----------check_danger_area()--------------------
   引数の周囲10mを立ち入り禁止エリアに
   戻り値
   1:問題なし
   2:問題ありだったが退避完了
   0:問題あり且つ解決していない
  ------------------------------------------*/

int check_danger_area() {

  xbee_uart( dev, "call check_danger_are\r");

  GPS check_gps;
  gps_get(&check_gps);

  int escape_count = 0;

  for (int i = 0; i < 10; i++) {  // 各禁止エリアについて

    if (!(danger_area_points[i].latitude == -1.0 && danger_area_points[i].longitude == -1.0)) {
      // 禁止エリアまでの距離算出
      double danger_distance = get_distance(&check_gps, &danger_area_points[i]);

      if (danger_distance < 7) {  // 7m以内に居たらやばい

        xbee_uart( dev, "DANGER ---> try escape\r");

        escape_count += 1;

        int escape_result = escape_danger_area(&check_gps, &danger_area_points[i]);

        if (escape_result == 1) {
          continue;  // 脱出出来たから次の危険エリアに引っかかって居ないかチェック
        } else {
          xbee_uart( dev, "false check_danger_are\r");
          return 0;  // 危険エリアから脱出できなかった
        }
      }
    }
  }

  if (escape_count == 0) {
    xbee_uart( dev, "success1 check_danger_are\r");
    return 1; // 何も問題が起きなかった
  } else {
    xbee_uart( dev, "success2 check_danger_are\r");
    return 2;
  }
}


/*-----------escape_danger_area()--------------------
   引数の周囲10mを立ち入り禁止エリアに
   戻り値
   1:成功
   0:失敗
  ------------------------------------------*/

int escape_danger_area(GPS *gps, POINT *point) {

  xbee_uart( dev, "call escape_danger_area\r");

  double escape_direction = get_direction(gps, point) + 180.0;  // 危険エリアの中心とは真逆の角度を指定
  double escape_my_direction = get_my_direction();  // 自身の角度を取得
  double danger_distance = 0.0;
  int escape_count = 0;

  do {

    int turn_result = turn_target_direction(escape_direction, &escape_my_direction);  //危険エリアの真逆を向く
    go_straight(4000);  // 4秒直進
    danger_distance = get_distance(gps, point);  //再度距離を取る

    escape_count += 1;

    if (escape_count == 5) {
      xbee_uart( dev, "false escape_danger_area\r");
      return 0;  //  上手く離れることができなかった
    }

  }  while (10 < danger_distance); // GPS上で十分に離れるか試行回数十分
  xbee_uart( dev, "success escape_danger_area\r");
  return 1;  // 成功を返す
}


/*stack_check_state()
 * スタックしたら呼び出す
   とりあえず自分がどうなっているか把握する
   go_straight_flag: 直進出来るか
   rotate_flag: 回転できるか

*/
int stack_check_state(ROVER *rover) {

  xbee_uart(dev, "call stack_check_state\r");


  int go_straight_flag = 0;  // フラグ定義
  int rotate_flag = 0;

  GPS gps_scs;
  POINT point_scs;
  get_rover_point(&point_scs);  // 位置を記録

  go_straight(10000);  // とりあえず10秒進んで見る
  gps_get(&gps_scs);  // GPS位置を取得

  if (get_distance(&gps_scs, &point_scs) < 3) {
    go_straight_flag = 0;
  } else {
    go_straight_flag = 1;
  }

  rover->My_Direction = get_my_direction();
  if (turn_target_direction(rover->My_Direction + 90, &rover->My_Direction) == 1) {
    rotate_flag = 1;
  } else {
    rotate_flag = 0;
  }

  int result = choose_behavior(rover, go_straight_flag, go_straight_flag);  // 何をするべきか決定する

  xbee_uart(dev, "end stack_check_state\r");
  return result;

}

/*choose_behavior()
   現状を受けて行動を選択
*/

int choose_behavior(ROVER *rover, int go_straight_flag, int rotate_flag) {
  if (go_straight_flag == 1) {  // 問題なし
    return 1;
  } else if ((go_straight_flag = 0) && (rotate_flag = 1)) {  // 回転は出来るが進めない(恐らく轍の中にいる)
    return escape_from_wadachi(rover);
  } else {   // 死亡(ランダムにする？)
    return 0;
  }
}


/*escape_from_wadachi()
  轍からの脱出

  走る→進めないなら少し後退→右旋回→

*/
int escape_from_wadachi(ROVER *rover) {
  xbee_uart(dev, "call escape_from_wadachi\r");

  POINT point_efw;
  GPS gps_efw;
  int try_counter = 0;
  boolean turn_flag = 0;

  get_rover_point(&point_efw);  // 初期位置を記録

  do {

    go_straight(10000);  // 10秒進む
    gps_get(&gps_efw);  // GPSを取得

    if (get_distance(&gps_efw, &point_efw) < 5) {
      go_back(4000);  // 少し下がる
      rover->My_Direction = get_my_direction();
      turn_flag = turn_target_direction(rover->My_Direction + 120, &rover->My_Direction, try_counter);  // 120度回転
      go_straight(5000);
      rover->My_Direction = get_my_direction();
      turn_target_direction(rover->My_Direction - 100, &rover->My_Direction, ~turn_flag);
    } else {
      ;  // 何もしない
    }

    try_counter += 1;

    if (10 <=try_counter) {
      xbee_uart(dev, "false escape_from_wadachi\r");
      return 0;
    }

  } while (15 < get_distance(&gps_efw, &point_efw));
  xbee_uart(dev, "success escape_from_wadachi\r");
  return 1;
}
