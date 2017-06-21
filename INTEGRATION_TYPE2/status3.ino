/*
   status3
   降下中の関数。高度差文によって着陸判定
*/

int status3(ROVER *rover) {  // Status3 降下の関数

  speaker(A_TONE);  // 降下フェーズ突入
  int st3_time;  // ログ記録判定用
  int landing_flag = 0;  // 着陸判定を何で行ったか
  int st3_cnt = 0;     // 強制脱出用カウンタ。今後は高度等で強制脱出する感じにして廃止していきたい。。。？
  double st3_alt;

  while (1) {
    st3_time = millis();
    byte sensor = EEPROM.read(EEP_CENSOR_STATUS);  // センサーの状態はどうなっているか


    if ((sensor & STATUS_GPS) == STATUS_GPS) { //GPS生存(高度取得用。後で実装）
      gps_get_al(&st3_alt);
      if (st3_alt < ALT_REGULATION) {
        if (judge_fall()) {
          xbee_uart(dev, "landing ok (judge from GPS)\r" );
          break;
        }
      }

      st3_cnt++;

      if (st3_cnt >= 15) {  //15分経過したら強制脱出。この前にGPS高度で判定を取る段階も作るべき！！

        xbee_uart(dev, "forced to break\r");
        break;

      }
      if ((st3_time % 60000) < 15) { //１分ごとのgpsログ取り一応処理時間を含めて甘めに範囲を取る感じで。あるいはst3_cntを活用？？
        GPS gps;
        gps_get(&gps);      //無限ウープに陥る可能性あり
        if (write_gps_sd(gps)) { // 自身の位置をsdに記録
          xbee_uart(dev, "success!!");
        } else {
          xbee_uart(dev, "fail...");
        }
      }
    }
    else if ((sensor & STATUS_AC) == STATUS_AC ) {   //AC生存
      //      xbee_uart(dev, "only AC alive\rjudging landing...\r");
      GPS st3_gps = eep_gpsget(EEP_GPS_INIT);
      int starttime = st3_gps.utc;   //制御開始時間差分を取るとか海抜変化で判定を取る？？
      if (determine_landing() == 1) {

        xbee_uart(dev, "landing ok (judge from AC)\r");
        break;
      }
      //1ループ1分より適当に15分たったら強制的に着陸したものとする
      st3_cnt++;

      if (st3_cnt >= 15) {

        break;
      }
    }
    else { //割と詰んでる状態
      xbee_uart(dev, "as know as zetsubou\r");//絶望
    }
    delay(10);
  }
  return 1;
}


// 着陸判定関数を書くファイル

/*-----------determine_landing()--------------------
  着陸したか判定を行う
  着陸と判定したら1、そうでなければ0を返す。
  ------------------------------------------*/

int determine_landing() {

  xbee_uart( dev, "judging Landing\r");


  AC ac; // 宣言

  double ac_array[10]; // サンプルを入れる箱

  // 加速度の和と平均
  double ac_sum = 0;
  double ac_ave = 0;

  // 加速度のサンプルを10個取る
  int i = 0;

  //xbee_uart( dev,"加速度のサンプルを取得します");

  while (i < 10) {
    ac = get_ac(); // 加速度を取得
    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
      // 値を取れている
      // 加速度の大きさを計算
      ac_array[i] = sqrt(pow(ac.x, 2) + pow(ac.y, 2) + pow(ac.z, 2));


      //      sprintf(xbee_send, "sample of %d is ", i + 1);  //ac_array bug
      //      xbee_uart(dev, xbee_send);
      //      xbee_send_1double(ac_array[i]);  //ここをコメントアウトしないと再起動する（震え）
      delay(3000); // サンプリングは3秒ごとに
      i += 1;
    } else {
      // 加速度を取得に失敗したら3秒待ってもう一度取る
      delay(3000);
    }
  }

  // 着陸したかの判定
  for (int i = 0; i < 10; ++i) {
    ac_sum += ac_array[i];
  }
  ac_ave = ac_sum / 10;

  //  xbee_uart( dev,"解析結果:");
  //  xbee_uart( dev,ac_ave);
  if (200 <= ac_ave && ac_ave <= 300) {
    xbee_uart( dev, "land ok\r");
    return 1; //着陸判定にパス
  } else {
    xbee_uart( dev, "land not ok\r");
    return 0;
  }
}


/*
   キャリブレーションしていないので地磁気を使う関数の使用原則禁止
   turn_target_direction禁止
*/


int judge_fall() {
  double alt_array[10];
  double alt_dif = 0.0;
  int t_init = millis();

  for (int jf_cnt = 0; jf_cnt < 10; jf_cnt++) {
    if ((millis() - t_init) >= 10000) {
      xbee_uart(dev, "judge_fall: timeout\r");
      return 0;
    }
    gps_get_al(&alt_array[jf_cnt]);
    xbee_send_1double(alt_array[jf_cnt]);  // ここでバグるかもしれない（動作確認まだ）なので注意
    delay(500); 
  }
  
   xbee_uart(dev, "initial alt is\r");
//   sprintf(xbee_send, "%lf\r", alt_array[0]);
//   xbee_uart(dev, xbee_send);          // xbee_send_1doubleでリブートするバグ！！
  
  for (int jf_cnt = 0; jf_cnt < 9; jf_cnt++) {
    alt_dif += alt_array[jf_cnt + 1] - alt_array[jf_cnt];
    if ( alt_dif > 3.0 ) {
      return 0;
    }
  }
  return 1;
}

