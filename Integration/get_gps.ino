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

  char g_szReadBuffer[READBUFFERSIZE] = "";
  int  g_iIndexChar = 0;
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
