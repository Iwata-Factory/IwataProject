int read_dora(byte stat) {
  return EEPROM.read(EEP_CENSOR_STATUS) & stat;
}

//距離センサの死亡判定
int distance_dora() {
  //ありえん電圧を返してしまった時は死亡判定,正常値であれば復活それ以外の死については思いつかない・制御では無理感
  if (SENSOR_STATUS == 0 ) {
    return read_dora(STATUS_DIS);
  }

  //距離センサパラメーター
  int sno = 0;
  double volt = 0;
  int cnt_dead = 0;     //何回パラシュートがあるかの判定をしたかのカウンター
  boolean dead = ((analogRead( DISTANCE ) * 5 / 1023.0  > 2.7) || (analogRead( DISTANCE ) * 5 / 1023.0) < 1.0 );  // 乱高下の扱い？？
  xbprintf("call distance dora");

  while ( sno < 50 ) {
    if ( dead ) { //異常電圧検出
      cnt_dead += 1;
    }
    sno++;
    delay(50);
  }
  if (cnt_dead >= 40 ) { //死亡書き換え
    xbprintf("DISTANCE DEAD\rrenew status...");
    renew_status(STATUS_DIS, 0); //死亡
    return 0;
  }
  if (cnt_dead <= 10 ) {
    xbprintf("DISTANCE REVIVED\rrenew status...");
    renew_status(STATUS_DIS, 1); //復活
    return 1;
  }
  return read_dora( STATUS_DIS);
}

int sd_dora() {
  if (SENSOR_STATUS == 0 ) {
    return read_dora(STATUS_SD);
  }
  xbprintf("call SD dora...");
  int cnt_dead, sno = 0;
  File samplefile = SD.open(SD_FLAG);
  boolean dead;
  while (sno < 50) {
    File samplefile = SD.open(SD_FLAG);
    //    dead = (!digitalRead(SD_INSERT) || (samplefile == false)); //ファイルを開けない・SD挿入YN回路から非挿入。FM以降の機体
    //    dead = (samplefile == false);  // CDR機体用
    //    if ( dead ) {
    if (samplefile == false) {
      cnt_dead += 1;
    }
    samplefile.close();
    sno++;
    delay(5);
  }
  if (cnt_dead >= 40 ) { //死亡書き換え
    renew_status(STATUS_SD, 0); //死亡
    //    xbprintf("SD DEAD\rrenewstatus...");  // ここの関数を有効にするとarduinoが通信しない不具合が発生しています。
    return 0;
  }
  if (cnt_dead <= 10 ) {
    renew_status(STATUS_SD, 1); //復活
    //    xbprintf( "SD REVIVED\rrenew status...");  // ここの関数を有効にするとarduinoが通信しない不具合が発生しています。
    return 1;
  }
  return read_dora(STATUS_SD);
}



//int ac_dora() { //ACKでアドレスが帰ってこない・値が変動しない・値が取れない
//if(SENSOR_STATUS == 0 ){
//    return read_dora(STATUS_AC);
//  }
//  //if ((EEPROM.read(EEP_CENSOR_STATUS) & STATUS_MOTOR) == STATUS_MOTOR ) { //モーターが故障していない場合
//  int cnt_dead, dif_cnt = 0;
//  // 加速度のサンプルを50個取る
//  AC ac;
//  double ac_array[50][3];
//  xbee_uart(dev, "judging life of AC...\r");
//
//  //xbee_uart( dev,"加速度のサンプルを取得します");
//
//  while (dif_cnt < 50) {
//    ac = get_ac(); // 加速度を取得
//    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
//      // 値を取れている
//      // 加速度の大きさを計算
//      ac_array[dif_cnt][0] = ac.x;
//      ac_array[dif_cnt][1] = ac.y;
//      ac_array[dif_cnt][2] = ac.z;
//      delay(30); // サンプリングは0.03秒ごとに。積分すると時間えぐそう
//      dif_cnt += 1;
//    } else {
//      // 加速度を取得に失敗したら3秒待ってもう一度取る
//      cnt_dead++;
//      if (cnt_dead >= 40 ) { //値がやってこない。死亡書き換え
//        xbee_uart(dev, "AC DEAD\rrenew status...\r");
//        renew_status(STATUS_AC, 0); //死亡
//        return 0;
//      }
//      delay(30);  //
//    }
//  }
//  cnt_dead = 0;  //無事サンプリング終了。ロック判定
//
//  for (int i = 0; i < 49; i++) {
//    if ((ac_array[i][1] == ac_array[i + 1][1]) && (ac_array[i][2] == ac_array[i + 1][2]) && (ac_array[i][0] == ac_array[i + 1][0])) {
//      cnt_dead++;
//    }
//  }
//
//
//  if (cnt_dead >= 45 ) { //死亡書き換え
//    xbee_uart(dev, "AC DEAD\rrenew status...\r");
//    renew_status(STATUS_AC, 0); //死亡
//    return 0;
//  }
//
//  if (cnt_dead <= 5 ) {
//    xbee_uart(dev, "AC REVIVED\rrenew status...\r");
//    renew_status(STATUS_AC, 1); //復活
//    return 1;
//  }
//  //}
//  return read_dora(STATUS_AC);
//}


int tm_dora() { //acと同じ不具合が考えられる
  if (SENSOR_STATUS == 0 ) {
    return read_dora(STATUS_TM);
  }

  //if ((EEPROM.read(EEP_CENSOR_STATUS) & STATUS_MOTOR) == STATUS_MOTOR ) { //モーターが故障していない場合
  int cnt_dead, dif_cnt = 0;
  // 地磁気のサンプルを50個取る
  TM tm;
  double tm_array[50][3];
  xbee_uart(dev, "judging life of TM...\r");

  //xbee_uart( dev,"Sampling TM...\r");
  DRIVE turn;

  turn.right1 = 0;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 0;

  rover_degital(turn);
  delay(700);

  while (dif_cnt < 50) {
    tm = get_tm(); // 加速度を取得

    if (!(tm.x == 100 && tm.y == 100 && tm.z == 100)) {
      // 値を取れている
      // 加速度の大きさを計算
      tm_array[dif_cnt][0] = tm.x;
      tm_array[dif_cnt][1] = tm.y;
      tm_array[dif_cnt][2] = tm.z;
      //      xbee_send_3doubles(tm.x, tm.y, tm.z);  // 生死状態でも擾乱のため値はぶれるので本当にロックされないと発動しない
      delay(50); // サンプリングは0.05秒ごとに。積分すると時間えぐそう
      dif_cnt += 1;
    } else {
      // 加速度を取得に失敗したら3秒待ってもう一度取る
      cnt_dead++;
      if (cnt_dead >= 40 ) { //値がやってこない。死亡書き換え
        xbee_uart(dev, "TM DEAD\rrenew status...\r");
        renew_status(STATUS_TM, 0); //死亡
        return 0;
      }
      delay(50);  //
    }
  }

  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;

  cnt_dead = 0;  //無事サンプリング終了。ロック判定

  for (int i = 0; i < 49; i++) {
    if ((tm_array[i][1] == tm_array[i + 1][1]) && (tm_array[i][2] == tm_array[i + 1][2]) && (tm_array[i][0] == tm_array[i + 1][0])) {
      cnt_dead++;
    }
  }


  if (cnt_dead >= 45 ) { //死亡書き換え
    xbee_uart(dev, "TM DEAD\rrenew status...\r");
    renew_status(STATUS_TM, 0); //死亡
    return 0;
  }

  if (cnt_dead <= 5 ) {
    xbee_uart(dev, "TM REVIVED\rrenew status...\r");
    renew_status(STATUS_TM, 1); //復活
    return 1;
  }
  //}
  return read_dora( STATUS_TM );
}


int xbee_dora() {  // いつも接続を施工し続けるべきだからむしろ定期的にネットワーク参加を試行するプログラムにするべき。。？
  if (SENSOR_STATUS == 0 ) {
    return read_dora(STATUS_XBEE);
  }

  xbprintf("judging life of XBEE...");
  int cnt_dead = 0;

  for (int i = 0; i < 50 ; i++) {
    if ( xbee_atai > 0x01) {
      cnt_dead ++;
    }
    delay(2);
  }
  if (cnt_dead > 48) {  // ネットワークに接続していないので参加を試す　
    xbee_atcb(1);
    delay(500);
    for (int i = 0; i < 50 ; i++) {
      if ( xbee_atai > 0x01) {
        cnt_dead ++;
      }
      delay(2);
    }
    if (cnt_dead > 48) {  //ネットワークに参加できなかった
      xbprintf("XBEE DEAD renew status...");
      renew_status(STATUS_XBEE, 0);  //死亡
      return 0;
    }
    else {
      xbprintf("XBEE REVIVED renew status...");
      renew_status(STATUS_XBEE, 1);  //生存
      return 1;
    }
  }
  else {
    xbprintf("XBEE REVIVED renew status...");
    renew_status(STATUS_XBEE, 1);  //生存
    return 1;
  }
  return read_dora(STATUS_XBEE);
}


