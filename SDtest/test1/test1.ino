/*
   test1

   SDカードに関するテスト

*/

// include文
#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <SD.h>

#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数
#define PIN_GPS_Rx  53 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_Tx  51 // GPSのシリアル通信でデータを送信するピン
#define SERIAL_BAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(ArduinoとPC)
#define GPSBAUDRATE 9600 //シリアル通信のデータ送信レートを9600bpsに定義するための定数(GPSとArduino)
#define SS 53

const int chipSelect = 4;


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

SoftwareSerial g_gps( PIN_GPS_Rx, PIN_GPS_Tx);

void setup() {
  Wire.begin();           //I2C通信の初期化
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  g_gps.begin(GPSBAUDRATE); //シリアル通信の初期化
  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.print("aa");
    // Serial.println("Card failed, or not present");
    // 失敗、何もしない
    while (1);
  }

  Serial.println("ok");
  Serial.println("setup完了");

  // 日付と時刻を返す関数を登録
  SdFile::dateTimeCallback( &dateTime );

}

void loop() {

  static int i = 0;
  i += 1;
  Serial.println(i);
  if (i == 100) {
    while (1);
  }
  delay(1000);
  struct GPS gps; // 構造体宣言
  gps_get(&gps);
  Serial.println(gps.utc);
  Serial.println(gps.latitude);
  Serial.println(gps.longitude);
  Serial.println(gps.Direction);
  Serial.println(gps.distance);

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    //    int value = analogRead(0);
    dataFile.print("number:::");
    dataFile.println(i);
    dataFile.println(gps.utc);
    dataFile.println(gps.latitude);
    dataFile.println(gps.longitude);
    dataFile.println(gps.Direction);
    dataFile.println(gps.distance);

    Serial.println(dataFile.position());

    dataFile.close();
  } else {
    Serial.println("error opening datalog.txt");
  }
}


void dateTime(uint16_t* date, uint16_t* time)
{
  uint16_t year = 2013;
  uint8_t month = 2, day = 3, hour = 9, minute = 0, second = 0;

  // GPSやRTCから日付と時間を取得
  // FAT_DATEマクロでフィールドを埋めて日付を返す
  *date = FAT_DATE(year, month, day);

  // FAT_TIMEマクロでフィールドを埋めて時間を返す
  *time = FAT_TIME(hour, minute, second);
}
