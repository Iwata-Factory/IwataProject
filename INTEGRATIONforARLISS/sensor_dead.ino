int read_dora(byte stat) {
  return EEPROM.read(EEP_CENSOR_STATUS) & stat;
}

/*
   いずれの関数も死亡時に０、生存時に１を返すようにしています。
*/

int dis_isdead() {
  //ありえん電圧を返してしまった時は死亡判定,正常値であれば復活それ以外の死については思いつかない・制御では無理感
  if (SENSOR_STATUS == 0 ) {
    return read_dora(STATUS_DIS);
  }

  //距離センサパラメーター
  int sno = 0;
  double volt = 0;
  int cnt_dead = 0;     //何回パラシュートがあるかの判定をしたかのカウンター
  boolean dead = ((analogRead( DISTANCE ) * 5 / 1023.0  > 2.7) || (analogRead( DISTANCE ) * 5 / 1023.0) < 1.0 );  // 乱高下の扱い？？
  xbprintf("call dis_isdead");

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
  return read_dora(STATUS_DIS);
}

int sd_isdead() {
  if (SENSOR_STATUS == 0 ) {
    return read_dora(STATUS_SD);
  }
  xbprintf("call sd_isdead...");
  int cnt_dead, sno = 0;
  File samplefile = SD.open(SD_FLAG);
  boolean dead;
  while (sno < 50) {
    File samplefile = SD.open(SD_FLAG);
    dead = (!digitalRead(SD_INSERT) || (samplefile == false)); //ファイルを開けない・SD挿入YN回路から非挿入。FM以降の機体
    dead = (samplefile == false);  // CDR機体用
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

