

void eep_clear() {       //EEPのすべてのアドレスに０書き込み

  int adr_cnt = 0;
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

//古いフラグと比較したのち書き込みを行います。

/*
   読み込みについては
   EEPROM.read(EEP_STATUS);
   を使えば１バイトの形で読み出せます。
*/
boolean EEP_STATUSwrite(int adr, byte oldflag, byte newflag) {
  byte flagread = EEPROM.read(adr);
  if ( oldflag == flagread ) {
    EEPROM.write( adr, newflag );
    Serial.println( "success writing!!" );
    return true;
  }
  else {
    Serial.println("invalid flag" );
    return false;
  }
}


/*
   eep_~~write
   アドレスと書き込む構造体・型を入れて
   次の書き込み先アドレスを返すようにしています。
   前から順に書き込んで行く前提です。
*/
int eep_gpswrite( int adr, GPS gps ) {
  EEPROM.put( adr, gps );
  Serial.println("success writeing gps in EEP.");
  adr += sizeof(gps);   //実験してしまえば必要なさ？？
  return adr + 1;
}

int eep_doublewrite( int adr, double Speed ) {
  EEPROM.put(adr, Speed );
  adr += sizeof(Speed);
  Serial.println("success writing (double)!!");

  return adr + 1;
}

int eep_acwrite( int adr, AC ac ) {
  EEPROM.put(adr, ac);
  adr += sizeof(ac);
  Serial.println("success writing ac!!");

  return adr + 1;
}

int eep_tmwrite( int adr, TM tm ) {
  EEPROM.put(adr, tm);
  adr += sizeof(tm);
  Serial.println("success writing tm!!");

  return adr + 1;
}

//eepの中のGPS構造体を読み込む関数
GPS eep_gpsget(int adr) {
  GPS gps;
  EEPROM.get(adr, gps);
  return gps;

}

boolean trans_phase(int phase ) {
  byte new_flag = EEPROM.read(EEP_STATUS);
  if ( new_flag == flag_phase[phase]) {
    EEPROM.write( EEP_STATUS, flag_phase[phase + 1] );
    return true;
  }
  else return false;
}

/*
   ステータス更新関数
   運用例：GPSが死んだ場合（悲しい）
   renew_status( STATUS_GPS, 0（消去） );
   EEPROM内
   11111101 -> 01111101
   これでEEPROM内のステータスは更新されます。
   statusの読み込みは
   EEPROM.read(EEP_CENSOR_STATUS);
   によってBYTE型で戻ってきます。
*/
byte renew_status( byte stat, int TF ) {
  byte new_status;
  if ( TF == 1 ) { //書き込み
    new_status = stat | EEPROM.read(EEP_CENSOR_STATUS);
    EEPROM.write(EEP_CENSOR_STATUS, new_status );
    return new_status;
  }
  if ( TF == 0) {  //消去 選択されたステータスの部分を0に
    new_status = ~stat & EEPROM.read(EEP_CENSOR_STATUS);
    EEPROM.write(EEP_CENSOR_STATUS, new_status);
    return new_status;
  }
}

