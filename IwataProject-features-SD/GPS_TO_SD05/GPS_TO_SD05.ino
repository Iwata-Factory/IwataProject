#include <SoftwareSerial.h>
#include <math.h>
#include <SD.h>
#include <SPI.h>
#include <string.h>

#define READBUFFERSIZE  (256)
#define DELIMITER   (",")
#define PIN_GPS_Rx  50               /*GPSのピン番号を変更しました。*/
#define PIN_GPS_Tx  52
#define SERIAL_BAUDRATE 9600
#define GPSBAUDRATE 9600

//以下、このプログラムでの定義
#define GPS_TEXT  ( "GPSlog.txt" )
#define SD_SS  53  // SDカードのピン番号の指定

SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx);  //10,11をシリアル通信用
char g_szReadBuffer[READBUFFERSIZE] = "";
int  g_iIndexChar = 0;

//以下、このプログラム独特の変数
const int chipselect = 4; // SD通信用品
int interval = 10;  //サンプリング間隔
int sno = 0;  //カウンタ
int gps_byte = 0; // gpsのログを入れるファイル内のバイト数
char gps_write[100];

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  g_gps.begin(GPSBAUDRATE);
  Serial.write( "START\n" );

  //SD初期化
  Serial.print("Initializig SD Card...");
  pinMode( SS , OUTPUT );
  SD.begin( SD_SS );
  while (!SD.begin(chipselect)) {                      //SDカードの初期化
    Serial.println("Card Failed, or not present");  //不能なら終わり
  }
  Serial.println("CARD ON");                        //初期化完了

}
//loop関数で使う用のgpsの情報をとっとくやつ
struct GPS {
  float utc = 0.0;       //グリニッジ天文時
  float latitude = 0.0;   //経度
  float longtitude = 0.0;   //緯度
  float Speed = 0.0;    //移動速度
  float course = 0.0;    //移動方位
  float Direction = 0.0;   //目的地方位
  float distance = 0.0;     //目的地との距離
  /*Speedとdistanceは小文字が予約語だったのでとりあえず大文字にしてあります*/
};
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

  Serial.println( "ANALYZE DONE" );

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

      Serial.println( "READ DONE" );

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

void gps_log( struct GPS* gps, int num ) {
  sprintf( gps_write, "%d%s%s%s%s%s%s", num, ",  UTC:  ", gps->utc, ",  Lat:  ", gps->latitude, ",  Long:  ", gps->longtitude );
  delay(50);
  File timeplace = SD.open( GPS_TEXT, FILE_WRITE );
  if (timeplace) {
    timeplace.print(gps_write);
    delay(50);
    Serial.println(gps_write);
    Serial.println( timeplace.position() );
    Serial.println( timeplace.size() );
    Serial.println( "FILE OPENED" );
  }
  timeplace.close();
}


void loop() {
  struct GPS gps;

  while ( !gps_get(&gps) ) {
    delay(25);     //ここのdelayを長くとると構造体が不完全になって帰ってくる
    Serial.println( "meanless data..." );
  }
  //こっから下は確認用+使い方、あとで消していいよ
  gps_log( &gps, sno );

  Serial.println("以下gps構造体の中身表示");
  Serial.println( sno );
  Serial.println(gps.latitude);
  Serial.println(gps.longtitude);
  Serial.println(gps.utc);
  Serial.println(gps.Speed);
  Serial.println(gps.course);
  Serial.println(gps.Direction);
  Serial.println(gps.distance);
  Serial.println( "\n" );
  sno++;
  delay( 5000 );
}

