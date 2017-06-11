

void eep_clear() {       //EEPのすべてのアドレスに０書き込み

  int adr_cnt = 0;
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

//古いフラグと比較したのち書き込みを行います。

/*
   読み込みについては
   EEPROM.read(EEP_FLAG);
   を使えば１バイトの形で読み出せます。
*/
boolean eep_flagwrite(int adr, byte oldflag, byte newflag) {
  if ( phasein_YN == 1) {
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
  else{
    Serial.println("flagdata broken!!");
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
  struct GPS gps;
  EEPROM.get(adr, gps);
  return gps;

}

