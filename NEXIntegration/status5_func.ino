/*-----------GPS関連--------------------
  ------------------------------------------*/

// 区切り文字定数
// センテンスの解析。
// $GPRMCの場合、引数変数に、緯度、経度を入れ、戻り値 1 を返す。
// $GPRMC以外の場合、戻り値は 0 を返す。

//static const int READBUFFERSIZE = 256;
//static const char DELIMITER = ",";

int AnalyzeLineString( char szLineString[], GPS *gps) {

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

int gps_get(GPS *gps) {

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
  float LatA = 35.713860, LongA = 139.759570;      //目的地
  //  float LatA = 35.710039, LongA = 139.810726;      //目的地
  float LatB = gps->latitude;       //現在地の緯度経度
  float LongB = gps->longitude;
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
    tm.x = (((int)tm_axis_buff[0]) << 8) | tm_axis_buff[1] + 215;  //x軸のデータは2バイト分であるMSBは8ビット左シフトさせる
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

  TM tm;

  for (int i = 0; i < 10; i++) {
    error_c = 0;
    while (tm.x == 100 || tm.y == 100 || tm.z == 100) {
      tm.x = 100; tm.y = 100; tm.z = 100;
      tm = get_tm();
      direction_array[i] = atan((tm.y - tm_y_offset) / (tm.x - tm_x_offset)); // 自身の方向を計算
      error_c += 1;
      if (error_c = 100) {  // 100回連続で取得失敗したら失敗を返す
        return -1;
      }
    }
  }
  my_direction = rad_out(10, direction_array);  // 10サンプルから平均を計算
  my_direction = rad2deg(my_direction);  // radからdegへ
  return my_direction;
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
    i += 1;
    
    *my_Direction = get_my_direction(); // 自身の方向を取得(deg)。target_directionもdeg

    if (*my_Direction == -1) {
      return 0; // 方向が取得出来ていないときの処理
    }

    double rotate_angle = 0;  // 回転量
    double a_difference = *my_Direction - target_direction;

    if (180 <= a_difference) {
      rotate_angle = 360 - a_difference;  // 右回転
    } else if (10 <= a_difference < 180) {
      rotate_angle = -rotate_angle;  // 左回転
    } else if (-10 <= a_difference && a_difference < 10) {
      rotate_angle = 0;  // 回転しない
      return 1;  // 回転に成功
    } else if (-180 <= a_difference < 10) {
      rotate_angle = -rotate_angle;  // 右回転
    } else {
      rotate_angle = 360 + a_difference;  // 左回転
    }

    go_rotate(rotate_angle);  // 回転を行う

  } while (i < 10); // 10回回転してもダメだったら失敗
  return 0;
}

