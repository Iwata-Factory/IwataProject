#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#define SD_SS  10      //SDのSSに割り当てられているソケット番号
#define ADXL345 0x53  //slave address

const int chipSelect = 4;
int interval = 20;          //サンプリング間隔
byte ac_axis_buff[6];       //ADXL345のデータ格納バッファ(各軸あたり2つずつ)
int sno = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);             //加速度センサのセットアップ
  Wire.begin();
  Serial.write("init");
  writeI2c(0x31, 0x00,ADXL345);
  writeI2c(0x2d, 0x08,ADXL345);
  
  Serial.print("Initializig SD Card...");
  pinMode( SS , OUTPUT );

  SD.begin( SD_SS );

  if(!SD.begin(chipSelect)){                        //SDカードの初期化
    Serial.println("Card Failed, or not present");  //不能なら終わり
    while(1);
  }
  
  Serial.println("CARD ON");                        //初期化完了
  
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t length = 6;                                         //xyzそれぞれ2バイトずつの出力のため

  if (millis() % (1000*interval)) {                            //interval秒ごとにサンプリング

    int ac_x, ac_y, ac_z = 0;
    float g_x, g_y, g_z;

    sno++;
    readI2c(0x32, length, ac_axis_buff, ADXL345);               //ADXL345のデータ取得
    ac_x = (((int)ac_axis_buff[1]) << 8) | ac_axis_buff[0];     //MSBとLSBの順番も逆になっている
    ac_y = (((int)ac_axis_buff[3]) << 8) | ac_axis_buff[2]; 
    ac_z = (((int)ac_axis_buff[5]) << 8) | ac_axis_buff[4];
    
    Serial.print(" ac_x: ");
    Serial.print(ac_x);
    Serial.print("ac_y: ");
    Serial.print(ac_y);
    Serial.print("ac_z: ");
    Serial.println(ac_z);

    g_x = ac_x / 255.0;
    g_y = ac_y / 255.0;
    g_z = ac_z / 255.0;

    File dataFile = SD.open("aclog.txt", FILE_WRITE);
    if(dataFile){
      dataFile.print(sno);
      dataFile.print(" ac_x = ");
      dataFile.print(g_x);
      dataFile.print("g  ,ac_y = ");
      dataFile.print(g_y);
      dataFile.print("g  ,ac_z = ");
      dataFile.print(g_z);
      dataFile.println("g");
      dataFile.close();
    }
    else{
      dataFile.println("Error opening file");                  //エラー表示
      dataFile.close();
    }
  }
  if(sno == 50){                                               //50回計測したら終了
    while(1);
  }
}


/*------------I2C書き込み-------------------------------
  register_addr:センサ内部のアドレス番地
  value:register_addrで指定したアドレスに書き込むデータ
  slave_addr:対象とするセンサのスレーブアドレス
  ------------------------------------------------------*/
void writeI2c(byte register_addr, byte value, byte slave_addr){   
  Wire.beginTransmission(slave_addr);
  Wire.write(register_addr);
  Wire.write(value);
  Wire.endTransmission();
}


/*------------I2C読み出し-------------------------------
  register_addr:センサ内部のアドレス番地
  num:読み出しを行うバイト数(通常はxyz2つずつで計6つ
  buffer[]:読み出したデータを格納するためのバッファ
  slave_addr:対象とするセンサのスレーブアドレス
  ------------------------------------------------------*/
void readI2c(byte register_addr, int num, byte buffer[], byte slave_addr){
  Wire.beginTransmission(slave_addr);
  Wire.write(register_addr);
  Wire.endTransmission();

  Wire.beginTransmission(slave_addr);
  Wire.requestFrom(slave_addr,num );
  
  int i=0;
  while(Wire.available()){
    buffer[i]=Wire.read();
    i++;
  }
  Wire.endTransmission();
}
