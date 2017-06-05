// include文
#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <avr/sleep.h>


// 定数の定義
#define READBUFFERSIZE  (256)
#define DELIMITER   (",")  // 区切り文字定数
// GPSのピンはSDと干渉しないように
#define PIN_GPS_Rx  53 // GPSのシリアル通信でデータを受信するピン
#define PIN_GPS_Tx  51 // GPSのシリアル通信でデータを送信するピン
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
#define TONE_PINNO 12   // 

typedef struct { // 加速度センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
} AC;

static const uint8_t length = 6;   //読み出しデータの個数

void setup() {
  Wire.begin();           //I2C通信の初期化
  Serial.begin(SERIAL_BAUDRATE); //シリアル通信の初期化
  writeI2c(0x02, 0x00, HMC5883L); //HMC5883Lの初期設定0x02レジスタに0x00書き込み
  writeI2c(0x31, 0x00, ADXL345);  //上と同様
  writeI2c(0x2d, 0x08, ADXL345);  //上と同
  Serial.println("setup完了");

}

void loop() {

  byte ac_axis_buff[6]; //ADXL345のデータ格納バッファ(各軸あたり2つずつ)
  AC ac; // 初期化
  ac.x = 100; // 失敗時は(100, 100, 100)を返す
  ac.y = 100;
  ac.z = 100;

  if (readI2c(0x32, length, ac_axis_buff, ADXL345)) { //ADXL345のデータ(加速度)取得
    ac.x = double((((int)ac_axis_buff[1]) << 8) | ac_axis_buff[0]) * 9.8 / 255;     //MSBとLSBの順番も逆になっている
    ac.y = double((((int)ac_axis_buff[3]) << 8) | ac_axis_buff[2]) * 9.8 / 255;
    ac.z = double((((int)ac_axis_buff[5]) << 8) | ac_axis_buff[4]) * 9.8 / 255;
  } else {
    ; // 何もしない
  }
  Serial.print("x:");
  Serial.println(ac.x);
  Serial.print("y:");
  Serial.println(ac.y);
  Serial.print("z:");
  Serial.println(ac.z);
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");

  delay(1000);


}



// I2c関連の関数を書くファイル

/*------------I2C書き込み-------------------
   register_addr:HMC5883L内のレジスタ番地
   value:レジスタに書き込むデータ
   slave_addr:通信したいスレーブのアドレス
   　
   成功した場合は1
   失敗した場合は0
   を返す
  ------------------------------------------*/

int writeI2c(byte register_addr, byte value, byte slave_addr) {
  Wire.beginTransmission(slave_addr);
  Wire.write(register_addr);
  Wire.write(value);
  int flag = Wire.endTransmission();

  if (flag == 0) { // 成功したか確認
    return 1;
  } else {
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
    return 0;
  }
}

