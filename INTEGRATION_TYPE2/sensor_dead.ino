//距離センサの死亡判定
void distance_dora() {
  //ありえん電圧を返してしまった時は死亡判定,正常値であれば復活それ以外の死については思いつかない・制御では無理感

  //距離センサパラメーター
  int sno = 0;
  double volt = 0;
  int cnt_dead = 0;     //何回パラシュートがあるかの判定をしたかのカウンター
  boolean dead = ((analogRead( DISTANCE ) * 5 / 1023.0  > 2.7) || (analogRead( DISTANCE ) * 5 / 1023.0) < 1.0);

  while ( sno < 50 ) {
    if ( dead ) { //異常電圧検出
      cnt_dead += 1;
    }
    sno++;
    delay(50);
  }
  if (cnt_dead >= 40 ) { //死亡書き換え
    xbee_uart(dev, "DISTANCE DEAD\rrenew status...\r");
    renew_status(STATUS_DIS, 0); //死亡
  }
  if (cnt_dead == 0 ) {
    xbee_uart(dev, "DISTANCE REVIVED\rrenew status...\r");
    renew_status(STATUS_DIS, 1); //復活
  }
}

void sd_dora() {
  int cnt_dead, sno = 0;
  File samplefile = SD.open(SD_FLAG);
  boolean dead;
  while (sno < 50) {
    File samplefile = SD.open(SD_FLAG);
    //    dead = (digitalRead(SD_INSERT)) || (samplefile == false) ;  //ファイルを開けない・SD挿入YN回路から火挿入を返される（回路未実装）
    dead = (samplefile == false );  // 現状はこれで。
    if ( dead ) {
      cnt_dead += 1;
    }
    samplefile.close();
    sno++;
    delay(50);
  }
  if (cnt_dead >= 40 ) { //死亡書き換え
    xbee_uart(dev, "SD DEAD\rrenewstatus...\r");
    renew_status(STATUS_SD, 0); //死亡
  }
  if (cnt_dead == 0 ) {
    xbee_uart(dev, "SD REVIVED\rrenew status...\r");
    renew_status(STATUS_SD, 1); //復活
  }
}


void ac_dora() { //ACKでアドレスが帰ってこない・値が変動しない・値が取れない
  //if ((EEPROM.read(EEP_CENSOR_STATUS) & STATUS_MOTOR) == STATUS_MOTOR ) { //モーターが故障していない場合
  int cnt_dead, dif_cnt = 0;
  // 加速度のサンプルを50個取る
  AC ac;
  double ac_array[50][3];
  xbee_uart(dev, "judging life of AC...\r");

  //xbee_uart( dev,"加速度のサンプルを取得します");

  while (dif_cnt < 50) {
    ac = get_ac(); // 加速度を取得
    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
      // 値を取れている
      // 加速度の大きさを計算
      ac_array[dif_cnt][0] = ac.x;
      ac_array[dif_cnt][1] = ac.y;
      ac_array[dif_cnt][2] = ac.z;
      delay(300); // サンプリングは0.3秒ごとに。積分すると時間えぐそう
      dif_cnt += 1;
    } else {
      // 加速度を取得に失敗したら3秒待ってもう一度取る
      cnt_dead++;
      if (cnt_dead >= 40 ) { //値がやってこない。死亡書き換え
        xbee_uart(dev, "AC DEAD\rrenew status...\r");
        renew_status(STATUS_AC, 0); //死亡
      }
      delay(300);  //
    }
  }
  cnt_dead = 0;  //無事サンプリング終了。ロック判定

  for (int i = 0; i < 49; i++) {
    if ((ac_array[i][1] == ac_array[i + 1][1]) && (ac_array[i][2] == ac_array[i + 1][2]) && (ac_array[i][0] == ac_array[i + 1][0])) {
      cnt_dead++;
    }
  }


  if (cnt_dead >= 45 ) { //死亡書き換え
    xbee_uart(dev, "AC DEAD\rrenew status...\r");
    renew_status(STATUS_AC, 0); //死亡
  }

  if (cnt_dead == 0 ) {
    xbee_uart(dev, "AC REVIVED\rrenew status...\r");
    renew_status(STATUS_AC, 1); //復活
  }
  //}
}


void tm_dora() { //acと同じ不具合が考えられる
  //if ((EEPROM.read(EEP_CENSOR_STATUS) & STATUS_MOTOR) == STATUS_MOTOR ) { //モーターが故障していない場合
  int cnt_dead, dif_cnt = 0;
  // 加速度のサンプルを50個取る
  TM tm;
  double tm_array[50][3];
  xbee_uart(dev, "judging life of TM...\r");

  //xbee_uart( dev,"Sampling TM...\r");

  while (dif_cnt < 50) {
    tm = get_tm(); // 加速度を取得
    if (!(tm.x == 100 && tm.y == 100 && tm.z == 100)) {
      // 値を取れている
      // 加速度の大きさを計算
      tm_array[dif_cnt][0] = tm.x;
      tm_array[dif_cnt][1] = tm.y;
      tm_array[dif_cnt][2] = tm.z;
      delay(300); // サンプリングは0.3秒ごとに。積分すると時間えぐそう
      dif_cnt += 1;
    } else {
      // 加速度を取得に失敗したら3秒待ってもう一度取る
      cnt_dead++;
      if (cnt_dead >= 40 ) { //値がやってこない。死亡書き換え
        xbee_uart(dev, "TM DEAD\rrenew status...\r");
        renew_status(STATUS_TM, 0); //死亡
      }
      delay(300);  //
    }
  }
  cnt_dead = 0;  //無事サンプリング終了。ロック判定

  for (int i = 0; i < 49; i++) {
    if ((tm_array[i][1] == tm_array[i + 1][1]) && (tm_array[i][2] == tm_array[i + 1][2]) && (tm_array[i][0] == tm_array[i + 1][0])) {
      cnt_dead++;
    }
  }


  if (cnt_dead >= 45 ) { //死亡書き換え
    xbee_uart(dev, "TM DEAD\rrenew status...\r");
    renew_status(STATUS_TM, 0); //死亡
  }

  if (cnt_dead == 0 ) {
    xbee_uart(dev, "TM REVIVED\rrenew status...\r");
    renew_status(STATUS_TM, 1); //復活
  }
  //}
}

//motor  //加速度変化がない？？
//light  //現状単一のセンサから１・０が来ているので死亡判定は出せないと思われる。放出判定はタイマーで済ませてその後のステータスにおいて明を返さない場合に死亡判定を出す程度しか不能では
//xbee  // xbee_ataiが0x00でない
//gps  //gpsについてはfuncs内のget_gpsの中に故障判定を追加しました。

//boolean sensing_dora(int sensor) { //sensor = 0:1:2:3:
//
//}

