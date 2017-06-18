boolean trans_phase(int phase ) {
  byte new_flag = EEPROM.read(EEP_FLAG);
  if ( new_flag == flag_phase[phase]) {
    EEPROM.write( EEP_FLAG, flag_phase[phase + 1] );
    return true;
  }
  else return false;
}

/*
 * ステータス更新関数
 * 運用例：GPSが死んだ場合（悲しい）
 * renew_status( STATUS_GPS, 0（消去） );
 * EEPROM内
 * 11111101 -> 01111101
 * これでEEPROM内のステータスは更新されます。
 * statusの読み込みは
 * EEPROM.read(EEP_STATUS);
 * によってBYTE型で戻ってきます。
 */
byte renew_status( byte stat, int TF ) {
  byte new_status;
  if ( TF == 1 ) { //書き込み
    new_status = stat | EEPROM.read(EEP_STATUS);
    EEPROM.write(EEP_STATUS, new_status );
    return new_status;
  }
  if ( TF == 0) {  //消去 選択されたステータスの部分を0に
    new_status = ~stat & EEPROM.read(EEP_STATUS);
    EEPROM.write(EEP_STATUS, new_status);
    return new_status;
  }
}

