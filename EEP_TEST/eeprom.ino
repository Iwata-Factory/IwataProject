

void eep_clear() {
  int adr_cnt = 0;
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

/*
   flag関係はアドレス0-7を使用します。
*/

//古いフラグと比較したのち書き込みを行います。
boolean eep_flagwrite(byte oldflag, byte newflag) {
  byte flagread = EEPROM.read(EEP_FLAG);
  if ( oldflag == flagread ) {
    EEPROM.write( EEP_FLAG, newflag );
    Serial.println( "success writing!!" );
    return true;
  }
  else {
    Serial.println("invalid flag" );
    return false;
  }
}

int eep_gpswrite( int adr, GPS gps ){
  EEPROM.put( adr, gps );
  Serial.println("success writeing gps in EEP.");
  adr += sizeof(gps);   //必要なさ
  return adr;
}

void eep_speedwrite( int adr, double Speed ){
  EEPROM.put(adr, Speed );
  Serial.println("success writing (double)!!");
}

void eep_acwrite( int adr, AC ac ){
  EEPROM.put(adr, ac);
  Serial.println("success writing ac!!");
}


