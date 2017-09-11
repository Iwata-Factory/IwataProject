
int status3(ROVER *rover) {  // Status3 降下の関数(着陸判定を行う)

  if (_S3_ == 0) {
    xbee_uart( dev, "skip status3\r");
    delay(1000);
    return 1;
  }

  GPS fall_gps;

  if (time_out == 0) {
    xbee_uart( dev, "time_out is 0\r");
    write_control_sd(F("time_out is 0\r"));
    xbee_uart( dev, "delay 1.5hr");
    write_control_sd(F("delay 1.5h\r"));

    unsigned long reset_count_start = millis();  // 開始時刻

    while (millis() - reset_count_start < 5400000) {  // 一時間半待つ

      write_control_sd("1.5h waiting");

      gps_get(&fall_gps);  // GPS送信
      delay(10000);

      unsigned long last_time = 5400000 - millis() + reset_count_start;
      dtostrf(last_time, 10, 6, xbee_send);
      xbprintf("last_time");
      xbprintf(xbee_send);
    }

    return 1;
  }


  if (time_out_flag == 0) {
    xbee_uart( dev, "failed　release decision\r");
    write_control_sd(F("failed　release decision"));

    while (1) {
      xbee_uart( dev, "2hours time-out mode\r");
      write_control_sd(F("2hours time-out mode"));

      gps_get(&fall_gps);  // GPS送信

      delay(10000);

      unsigned long last_time = 7200000 - millis() + time_out;
      dtostrf(last_time, 10, 6, xbee_send);
      xbprintf("last_time");
      xbprintf(xbee_send);

      if (millis() - time_out > 7200000) {
        xbee_uart( dev, "Clear\r");
        write_control_sd(F("Clear"));
        return 1;
      }
    }

  } else {  // 高度を用いてタイムアウトを計算

    write_control_sd("get alt");

    double alt = 0;
    double alt_array[5];
    int i = 0;
    int get_alt_flag = 0;

    for (int j = 0; j < 5; j++) {
      i = 0;
      while (1) {
        gps_get_al(&alt);  // 高度を取得
        if (ALT_VAL == 1) {
          if (2000 < alt && alt < 8000) {
            alt_array[j] = alt;
            break;
          }
        } else {
          if (0 < alt) {
            alt_array[j] = alt;
            break;
          }
        }
        i += 1;
        if (i == 3) {
          alt_array[j] = 0;
          break;
        }
      }
    }

    for (int j = 0; j < 5; j++) {
      if (0 < alt_array[j]) {
        get_alt_flag = 1;
      }
    }

    if (get_alt_flag == 0) {
      xbee_uart( dev, "failed get alt\r");
      write_control_sd(F("failed get alt"));
      while (1) {

        xbee_uart( dev, "2hours time-out mode\r");

        write_control_sd(F("2hours time-out mode"));
        gps_get(&fall_gps);  // GPS送信

        delay(10000);
        unsigned long time_waiting = 0;
        if (LAND_ALT == 1) {
          time_waiting = 7200000;
        } else {
          time_waiting = 100000;
        }


        unsigned long last_time = time_waiting - millis() + time_out;
        dtostrf(last_time, 10, 6, xbee_send);
        xbprintf("last_time");
        xbprintf(xbee_send);

        if (millis() - time_out > time_waiting) {
          xbee_uart( dev, "Clear\r");
          write_control_sd(F("Clear"));
          return 1;
        }
      }
    } else {

      write_control_sd("get max alt");

      //alt_arrayの最大値を出す
      alt = value_max(5, alt_array);
      xbee_uart( dev, "wait start\r");
      write_control_sd(F("wait start"));

      // 秒速5m/sで落下するとし1.2のマージンを取る

      if (1000 < alt) {
        alt = alt - 1000;
      } else {
        ;
      }

      unsigned long wait_time = ((alt * 1.2 * 1000) / 5) + 600000;
      unsigned long fall_count_start = millis();  // 開始時刻
      dtostrf(wait_time, 10, 6, xbee_send);
      xbprintf("wait time");
      xbprintf(xbee_send);

      if (ALT_VAL == 1) {  // 本番は必ずこの時間は待つ
        if (wait_time < 1440000) {
          wait_time = 1440000;
        }
      }

      while (millis() - fall_count_start < wait_time) {

        write_control_sd("waiting");

        gps_get(&fall_gps);  // GPS送信
        delay(10000);

        unsigned long last_time = wait_time - millis() + fall_count_start;
        dtostrf(last_time, 10, 6, xbee_send);
        xbprintf("last_time");
        xbprintf(xbee_send);

      }

      return 1;
    }
  }
}
//
//  int switch_num = 1;  // 判定する素子のスイッチ
//  int landing_flag = 0;
//  int judge_land_counter = 0;
//
//  do {
//    judge_land_counter += 1;
//
//    if (judge_land_counter == 3) {  // 強制降下判定
//      landing_flag = 1;
//    }
//
//    get_censor_status(rover);  // 最新のセンサーの状態を取得
//
//    switch_num = get_switch(rover);
//
//    write_timelog_sd(rover);  //  ログを残す
//
//    switch (switch_num) {
//
//      case 1:
//        landing_flag = judge_landing_by_gps();  // GPS1による着陸判定
//        xbee_uart( dev, "end judge_landing_by_gps\r");
//        delay(3000);
//        break;
//
//      case 2:  /* GPS1と2の振る舞いの違いを実装する必要がありますね by とうま*/
//        landing_flag = judge_landing_by_gps();  // GPS2による着陸判定
//        xbee_uart( dev, "end judge_landing_by_gps\r");
//        delay(3000);
//        break;
//
//      case 3:
//        landing_flag = judge_landing_by_ac();
//        xbee_uart( dev, "end judge_landing_by_ac\r");  // 加速度センサによる着陸判定
//        delay(3000);
//        break;
//
//      case 4:  // 着陸判定にかかるセンサ系の死亡
//        xbee_uart( dev, "sensor death ---> wait\r");
//        delay(30000);  // 30秒待つ
//        // ここで治ったか判定する関数を回します。//
//        check_realive();
//        break;
//    }
//  } while (landing_flag == 0);
//
//  write_timelog_sd(rover);  //  ログを残す
//
//  return 1;
//}
//
//
//// 着陸判定をどのセンサで行うのかを定める関数
//int get_switch(ROVER *rover) {
//  if (rover->ac_arive == 1) {  // 加速度
//    write_control_sd("use ac-censor");
//    return 3;
//  } else if (rover->gps1_arive == 1) { // GPS1
//    write_control_sd("use gps-censor-1");
//    return 1;
//  } else if (rover->gps2_arive == 1) { // GPS2
//    write_control_sd("use gps-censor-2");
//    return 2;
//  } else {
//    write_control_sd("no consor");
//    return 4;  // 詰み(センサ回復を待ちつつ時間で抜ける)
//  }
//
//}
//
//
///*-----------judge_landing_by_gps()--------------------
//  着陸したかGPSの高度情報で判定を行う
//  着陸と判定したら1、そうでなければ0を返す。
//  ------------------------------------------*/
//int judge_landing_by_gps() {
//
//  xbee_uart( dev, "call judge_landing_by_gps\r");
//
//  if (LAND_JUDGE_FLAG == 0) {
//    xbee_uart( dev, "skip judge_landing_by_gps\r");
//    return 1;
//  }
//
//
//  double alt = 100;
//  gps_get_al(&alt);  // 高度を一旦取得
//
//  write_control_sd("altitude is " + String(alt, DEC));
//
//  if (alt < ALT_REGULATION) {
//    write_control_sd("detail check");
//    return (judge_landing_by_gps_detail());
//  } else {
//    write_control_sd("No landing");
//    return 0;
//  }
//}
//
//// GPSによる着陸判定の詳細な処理
//int judge_landing_by_gps_detail() {
//
//  xbee_uart( dev, "call judge_landing_by_gps_detail\r");
//
//  double alt_array[10] = {0.0};
//  double alt_dif = 0.0;
//
//  for (int i = 0; i < 10; i++) {
//    gps_get_al(&alt_array[i]);
//    write_control_sd("altitude[" + String(i, DEC) + "] is " + String(alt_array[i], DEC));
//    delay(1000);
//    if ((0 < i) && ( 3 < (alt_array[i - 1] - alt_array[i]))) {  // 前回-今回
//      write_control_sd(String("difference > 3"));
//      write_control_sd("No landing");
//      return 0;  // まだ降下中である。
//    }
//  }
//  write_control_sd("landing");
//  return 1;  // 高度変化がなくなった
//}
//
//
///*-----------judge_landing_by_ac()--------------------
//  着陸したか加速度で判定を行う
//  着陸と判定したら1、そうでなければ0を返す。
//  ------------------------------------------*/
//
//int judge_landing_by_ac() {
//
//  xbee_uart( dev, "call judge_landing_by_ac\r");
//
//  if (LAND_JUDGE_FLAG == 0) {
//    xbee_uart( dev, "skip judge_landing_by_ac\r");
//    return 1;
//  }
//
//  AC ac; // 宣言
//
//  double ac_array[10]; // サンプルを入れる箱
//
//  // 加速度の和と平均
//  double ac_ave = 0;
//
//  // 加速度のサンプルを10個取る
//  int i = 0;
//
//  //xbee_uart( dev,"加速度のサンプルを取得します");
//
//  while (i < 10) {
//    ac = get_ac(); // 加速度を取得
//    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
//      // 値を取れている
//      // 加速度の大きさを計算
//      ac_array[i] = sqrt(pow(ac.x, 2) + pow(ac.y, 2) + pow(ac.z, 2));
//      write_control_sd("ac[" + String(i, DEC) + "] is " + String(ac_array[i], DEC));
//      delay(3000); // サンプリングは3秒ごとに
//      i += 1;
//    } else {
//      // 加速度を取得に失敗したら3秒待ってもう一度取る
//      delay(3000);
//    }
//  }
//
//  // 着陸したかの判定
//  ac_ave = value_ave(10, ac_array);  // 平均値を取る
//
//  if (200 <= ac_ave && ac_ave <= 300) {  // パラメタは要調整
//    write_control_sd("200 < ac_average < 300 ---> landing");
//    xbee_uart( dev, "success determine_landing\r");
//    return 1; //着陸判定にパス
//  } else {
//    write_control_sd("not (200 < ac_average < 300) ---> No landing");
//    xbee_uart( dev, "false determine_landing\r");
//    return 0;
//  }
//}
//
//// センサーの生死というか、復活を判定(現在実体なし)
//int check_realive() {
//  xbee_uart( dev, "call check_realive\r");
//  if (LAND_JUDGE_FLAG == 0) {
//    xbee_uart( dev, "skip check_realive\r");
//    return 1;
//  }
//  return 0;
//}

