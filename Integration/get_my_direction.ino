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
