
/*
   降下中のログを１分ごとに測定
   もし加速度センサzが急変したら脱出  ！！パラシュートによる変化との差別化！！！！風に煽られた時の変化！！
   ー＞海抜取得byGPSの$GPGGA
   加速度センサが死んだようならステータス更新
   加速度が死んでいる時はEEPGPSからのスタート時間からの差分
   GPSすら死んでしまったらタイマー
*/

int status3(ROVER *rover) {  // Status3 降下の関数
  int st3_time;

  while (1) {
    st3_time = millis();       //時刻取得
    byte sensor = EEPROM.read(EEP_CENSOR_STATUS );

    if ((sensor & STATUS_AC) == STATUS_AC ) {   //AC生存
      if ((sensor & STATUS_GPS) == STATUS_GPS) { //かつGPS生存(高度取得用。後で実装）
        if (determine_landing() == 1) {
          break;
        }
        if ((st3_time % 60000) < 15) { //１分ごとのgpsログ取り一応処理時間を含めて甘めに範囲を取る感じで
          GPS gps;
          gps_get(&gps);      //無限ウープに陥る可能性あり
          if (write_gps_sd(gps)) { // 自身の位置をsdに記録
            Serial.println("success!!");
          } else {
            Serial.println("fail...");
          }
        }
      }


    }
    else if ((sensor & STATUS_GPS) == STATUS_GPS) {
      Serial.println("only GPS alive");
      GPS st3_gps = eep_gpsget(INITIAL_GPS);
      int starttime = st3_gps.utc;   //この後滅茶苦茶差分をとった(fin)
    }
    else { //割と詰んでる状態
      //絶望
    }
    delay(10);
  }
  Serial.println("finish falling\rlanded!!\rlogging landed point!");
  GPS gps_land;  //着陸地点を記録（必要？？）
  gps_get(&gps_land);
  if (write_gps_sd_file(gps_land, GPS_SPECIAL)) { // 自身の位置をsdに記録。新規に関数を作成しました。
    Serial.println("success!!");
  } else {
    Serial.println("fail...");
  }
  return 1;
}

