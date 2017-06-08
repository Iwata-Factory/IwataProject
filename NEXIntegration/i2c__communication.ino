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
