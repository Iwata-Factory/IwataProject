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
    renew_status(STATUS_DIS, 0); //死亡
  }
  if (cnt_dead == 0 ) {
    renew_status(STATUS_DIS, 1); //復活
  }
}

boolean sd_dora() {
  int cnt_dead, sno = 0;
  File samplefile = SD.open(SD_FLAG);
  boolean dead;
  while (sno < 50) {
    File samplefile = SD.open(SD_FLAG);
    dead = (digitalRead(SD_INSERT)) || (samplefile == false) ;  //ファイルを開けない・SD挿入YN回路から火挿入を返される（回路未実装）
    if ( dead ) {
      cnt_dead += 1;
    }
    samplefile.close();
    sno++;
    delay(50);
  }
  if (cnt_dead >= 40 ) { //死亡書き換え
    renew_status(STATUS_SD, 0); //死亡
  }
  if (cnt_dead == 0 ) {
    renew_status(STATUS_SD, 1); //復活
  }
}


//ac  //ACKでアドレスが帰ってこない・値が変動しない
//tm  //acと同じ不具合が考えられる
//motor  //加速度変化がない？？
//light  //現状単一のセンサから１・０が来ているので死亡判定は出せないと思われる。放出判定はタイマーで済ませてその後のステータスにおいて明を返さない場合に死亡判定を出す程度しか不能では
//xbee
//gps  //
