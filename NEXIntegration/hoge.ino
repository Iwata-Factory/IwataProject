/*
 * status4をコンパイルするようです
 * あとで消してください
 */



// 自身の方向取得関数を書くファイル

/*-----------get_my_direction()--------------------
  自分の向いている方向
  成功したらその値を
  失敗したら負の値を返却
  ------------------------------------------*/


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


double get_my_direction() {

  double my_direction = -1; //自分の方向(負の値で初期化)

  // 加速度を取得
  AC ac;
  int ac_counter = 0; // 試行回数をカウント(冗長化に使う)
  while (1) {
    ac = get_ac();
    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
      break;
    } else {
      ac_counter += 1;
      delay(30);
      if (ac_counter == 100) {
        return -1;
      }
    }
  }

  Serial.print("ac.x:");
  Serial.println(ac.x);

  Serial.print("ac.y:");
  Serial.println(ac.y);

  Serial.print("ac.z:");
  Serial.println(ac.z);

  // 地磁気を取得
  TM tm;
  int tm_counter = 0; // 試行回数をカウント(冗長化に使う)
  while (1) {
    tm = get_tm();
    if (!(tm.x == 100 && tm.y == 100 && tm.z == 100)) {
      break;
    } else {
      tm_counter += 1;
      delay(30);
      if (tm_counter == 100) {
        return -2;
      }
    }
  }

  Serial.print("tm.x:");
  Serial.println(tm.x);

  Serial.print("tm.y:");
  Serial.println(tm.y);

  Serial.print("tm.z:");
  Serial.println(tm.z);

  // 方角を算出
  double roll = 0; //ロール角
  double pitch = 0; //ピッチ角
  double o_x = 0; //オフセット量
  double o_y = 0;
  double o_z = 0;
  // 基底の変換
  roll = atan2(ac.y, ac.z);
  pitch = atan2(ac.x * (-1), ac.y * sin(roll) + ac.z * cos(roll));
  double numerator = (tm.z - o_z) * sin(roll) - (tm.y - o_y) * cos(roll); //分子
  double denominator = (tm.x - o_x) * cos(pitch) + (tm.y - o_y) * sin(pitch) * sin(roll) + (tm.z - o_x) * sin(pitch) * cos(roll); // 分母
  my_direction = atan2(numerator, denominator) * RAD_TO_DEG + 180; //xy平面での向きを角度(°)で表示(北で0°)
  return my_direction;
}

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

int gps_get(struct GPS* gps) {

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


// ローバーの移動に関する関数


/*-----------rover_degital(DRIVE)--------------------
  DRIVE型の引数
  degital write　の時
  ------------------------------------------*/
void rover_degital(DRIVE drive) {
  digitalWrite(M1_1, drive.right1);
  digitalWrite(M1_2, drive.right2);
  digitalWrite(M2_1, drive.leght1);
  digitalWrite(M2_2, drive.leght2);
}

/*-----------rover_analog(DRIVE)--------------------
  DRIVE型の引数
  analog write　の時
  ------------------------------------------*/
void rover_analog(DRIVE drive) {
  analogWrite(M1_1, drive.right1);
  analogWrite(M1_2, drive.right2);
  analogWrite(M2_1, drive.leght1);
  analogWrite(M2_2, drive.leght2);
}


/*-----------機体旋回--------------------
   rotate: 回転角
  ------------------------------------------*/
void go_rotate(double rotate) {

  DRIVE turn; // DRIVE型の宣言
  // 初期化
  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;

  int rotate_time = 0; // 旋回する時間 /* これを求めるアルゴリズムの用意は必要 */

  // どちら向きに回転するかを判断
  if (0 < rotate) {
    turn.right1 = 0;
    turn.right2 = 1;
    turn.leght1 = 1;
    turn.leght2 = 0;

    rotate_time = (rotate / 400) * 1000 + 120;

  } else {
    turn.right1 = 1;
    turn.right2 = 0;
    turn.leght1 = 0;
    turn.leght2 = 1;

    rotate_time = -1 * (rotate / 400) * 1000 + 120;

  }

  // 回転を行う
  rover_degital(turn);

  delay(rotate_time);

  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;

  // 停止
  rover_degital(turn);
  delay(1500);
}




/*-----------機体直進--------------------
   go_time: 進む時間
  ------------------------------------------*/

void go_straight(int go_time) {
  DRIVE go; //DRIVE型の宣言
  // 初期化
  int wait_time = go_time - 1024;
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  for (int i = 1; i < 256; i++) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(2);
  }
  go.right1 = 0;
  go.right2 = 1;
  go.leght1 = 0;
  go.leght2 = 1;
  rover_degital(go);
  delay(wait_time);
  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(2);
  }
}
//void go_straight(int go_time) {
//  DRIVE go; //DRIVE型の宣言
//  // 初期化
//  go.right1 = 0;
//  go.right2 = 1;
//  go.leght1 = 0;
//  go.leght2 = 1;
//  // 直進
//  rover_degital(go);
//  delay(go_time);
//
//  // 停止
//  go.right1 = 1;
//  go.right2 = 1;
//  go.leght1 = 1;
//  go.leght2 = 1;
//  // 停止
//  rover_degital(go);
//  delay(1000);
//}


