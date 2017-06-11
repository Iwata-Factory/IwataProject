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
   機体ステータスが少なくともなんらかの段階にある
   かどうかを判定する関数。
   機体ステータスを９段階以上に分けることになったら変えます。
   戻り値は
   １：機体は何かしらのミッション段階にある
   ０：読み取ったデータが破損している
   ２：データの読み取りに失敗
   
*/
int phasein_YN(int eep_sd)  //引数が０ならeep,１ならsd内の期待ステータスを読み込み
{
  byte bit8;
  switch (eep_sd) {
    case 0:
      bit8 = EEPROM.read(EEP_FLAG);
      break;
    case 1:
      File sd_flag = SD.open(SD_FLAG, FILE_READ);
      sd_flag.seek(0);
      bit8 = sd_flag.peek();
      sd_flag.close();
      break;
  }

  // 8 bits 限定アルゴリズムを利用している。
  //参照：https://www16.atwiki.jp/projectpn/pages/35.html
  if ( 1 == sizeof( bit8 ) ) {

    bit8 = ( ((bit8 & 0xAA) >> 1) + (bit8 & 0x55) );
    bit8 = ( ((bit8 & 0xCC) >> 2) + (bit8 & 0x33) );
    bit8 = ( ((bit8 & 0xF0) >> 4) + (bit8 & 0x0F) );  //ここでbit8は1byteの中で1になっている桁の数になっている。

    if( bit8 == 1){
      return 1;  //機体ステータスはなんらかのステータスにあることを示している。
    }
    else{
      return 0;  //機体ステータスのデータが破損してしまっている！！
    }
  }
  else {
    return 2;
  }
}

/*
 * センサーのステータスが正しいデータになっているかを判定する関数
 */
