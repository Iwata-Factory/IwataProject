boolean trans_phase(int phase ) {
  byte new_flag = EEPROM.read(EEP_FLAG);
  if ( new_flag == flag_phase[phase]) {
    EEPROM.write( EEP_FLAG, flag_phase[phase + 1] );
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
   EEPROM.read(EEP_STATUS);
   によってBYTE型で戻ってきます。
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


/*
   1byte内に存在する1の数をカウント
   機体ステータスが少なくともなんらかの段階にあるかどうか判定可能
   （現状では各ミッション段階に対して特定桁が1になるようにしているので）
   例）00000010 (機体ステータスLAUNCH)
   戻り値　１
   00010110
   戻り値　３
*/
int // 立っている ビットの数
bitcount8
(
  unsigned char b8 // 8 bits 値
)
{
  // 8 bits 限定アルゴリズムを利用している
  c_assert( 8 == (CHAR_BIT * sizeof( b8 )) );

  b8 = (unsigned char)( ((b8 & 0xAA) >> 1) + (b8 & 0x55) );
  b8 = (unsigned char)( ((b8 & 0xCC) >> 2) + (b8 & 0x33) );
  b8 = (unsigned char)( ((b8 & 0xF0) >> 4) + (b8 & 0x0F) );

  return b8;
}//bitcount8

