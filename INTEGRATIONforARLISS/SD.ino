/* SDカード周りの関数
   write_ac_sd
   write_tm_sd
   write_gps_sd
   read_ac_sd
   read_tm_sd
   read_gps_sd

  記録系:write

  読み取る系:read(使っていない)
*/

/*
   SDに時間を保存
*/
int write_timelog_sd(ROVER *rover) {

  DRIVE go; //DRIVE型の宣言
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

  if (SD_LOG_FLAG == 0) { // スキップ
    return 0;
  }

  xbee_uart(dev, "call write_timelog_sd\r" );
  rover->time_from_start = millis();  // 機体時間を取得
  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    File dataFile = SD.open(LOG_TIME, FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());
      dataFile.println("*"); // 記録の境目
      dataFile.print("time:");
      dataFile.print(rover->time_from_start);
      dataFile.print(",status:");
      dataFile.println(rover->status_number);
      dataFile.close();
      xbee_uart(dev, "success write_timelog_sd\r" );
      return 1; // 成功を返す
    } else {
      i += 1;
    }
  }
  xbee_uart(dev, "false write_timelog_sd\r" );
  return 0; // 失敗を返す
}


// 加速度を記録
int write_ac_sd(AC ac) {

  DRIVE go; //DRIVE型の宣言
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

  if (SD_LOG_FLAG == 0) {  // スキップ
    return 0;
  }

  xbee_uart(dev, "call write_ac_s\r" );
  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    File dataFile = SD.open(LOG_AC, FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());
      dataFile.println("*"); // 記録の境目
      dataFile.println(ac.x);
      dataFile.println(ac.y);
      dataFile.println(ac.z);
      dataFile.close();
      xbee_uart(dev, "success write_ac_sd\r" );
      return 1; // 成功を返す
    } else {
      i += 1;
    }
  }
  xbee_uart(dev, "false write_ac_sd\r" );
  return 0; // 失敗を返す
}


// 地磁気を記録
int write_tm_sd(TM tm) {

  DRIVE go; //DRIVE型の宣言
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

  if (SD_LOG_FLAG == 0) { // スキップ
    return 0;
  }

  xbee_uart(dev, "call write_tm_sd\r" );
  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    File dataFile = SD.open(LOG_TM, FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());
      dataFile.println("*"); // 記録の境目
      dataFile.println(tm.x);
      dataFile.println(tm.y);
      dataFile.println(tm.z);
      dataFile.close();
      xbee_uart(dev, "success write_tm_sd\r" );
      return 1; // 成功を返す
    } else {
      i += 1;
    }
  }
  xbee_uart(dev, "false write_tm_sd\r" );
  return 0; // 失敗を返す
}

// GPSを記録
int write_gps_sd(GPS gps) {

  DRIVE go; //DRIVE型の宣言
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

  if (SD_LOG_FLAG == 0) { // スキップ
    return 0;
  }

  xbee_uart(dev, "call write_gps_sd\r" );
  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める

    File dataFile = SD.open(LOG_GPS, FILE_WRITE);

    if (dataFile) { // ファイルが開けたときの処理

      if (GPS_GET_FLAG == 1) {
        dataFile.seek(dataFile.size());
        dataFile.println("*"); // 記録の境目
        dataFile.println(gps.utc, 4);  // 下6桁
        dataFile.println(gps.latitude, 6);
        dataFile.println(gps.longitude, 6);
        dataFile.println(gps.Speed, 4);
        //        dataFile.println(gps.course, 4);
        dataFile.println(gps.Direction, 6);
        dataFile.println(gps.distance, 6);
        if (use_which_gps == 1) {
          dataFile.println("used gps1");
        } else if (use_which_gps == 2) {
          dataFile.println("used gps2");
        }

      } else {
        dataFile.println("No GPS");
      }

      dataFile.close();
      xbee_uart(dev, "succes write_gps_sd\r" );
      return 1; // 成功を返す
    } else {
      i += 1;
    }
  }
  xbee_uart(dev, "false write_gps_sd\r" );
  return 0; // 失敗を返す
}

// クリティカルなログを記録
int write_critical_sd(int flag) {

  DRIVE go; //DRIVE型の宣言
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

  if (SD_LOG_FLAG == 0) { // スキップ
    return 0;
  }

  unsigned long time = millis();  // 機体時間を取得

  GPS gps;
  gps_get(&gps);

  xbee_uart(dev, "call write_critical_sd\r" );

  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める

    File dataFile = SD.open(LOG_CRITICAL, FILE_WRITE);

    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());

      switch (flag) {

        case 0:  // セットアップ時
          dataFile.println("");
          dataFile.println("");
          dataFile.println("");
          dataFile.println("");
          dataFile.println("**end_setup**");
          dataFile.println("arduino-time");
          dataFile.println(time);
          break;


        case 1:

          if (GPS_GET_FLAG == 1) {

            dataFile.println("");
            dataFile.println("**landing**"); // 記録開始
            dataFile.println("arduino-time");
            dataFile.println(time);
            dataFile.println("utc");
            dataFile.println(gps.utc, 6);  // 下4桁
            dataFile.println("lat");
            dataFile.println(gps.latitude, 6);
            dataFile.println("lng");
            dataFile.println(gps.longitude, 6);
            dataFile.println("distance");
            dataFile.println(gps.distance, 6);
            dataFile.println("");
          } else {
            dataFile.println("");
            dataFile.println("**landing**"); // 記録開始
            dataFile.println("No GPS");
            dataFile.println("");
          }
          break;



        case 2:
          if (GPS_GET_FLAG == 1) {

            dataFile.println("");
            dataFile.println("**end-control**"); // 記録開始
            dataFile.println("arduino-time");
            dataFile.println(time);
            dataFile.println("utc");
            dataFile.println(gps.utc, 6);  // 下4桁
            dataFile.println("lat");
            dataFile.println(gps.latitude, 6);
            dataFile.println("lng");
            dataFile.println(gps.longitude, 6);
            dataFile.println("distance");
            dataFile.println(gps.distance, 6);

          } else {
            dataFile.println("");
            dataFile.println("**end-control**"); // 記録開始
            dataFile.println("No GPS");
            dataFile.println("");
          }
          break;


      }

      dataFile.close();
      xbee_uart(dev, "succes write_critical_sd\r" );
      return 1; // 成功を返す
    } else {
      i += 1;
    }
  }
  xbee_uart(dev, "false write_critical_sd\r" );
  return 0; // 失敗を返す
}



// 加速度を読み取る
// 第一引数:値を入れる構造体の配列
// 第二引数:何個の値を入れるか
int read_ac_sd(AC ac[100], int num) {

  // 0.0で初期化
  for (int i = 0; i < num; i++) {
    ac[i].x = 0.0;
    ac[i].y = 0.0;
    ac[i].z = 0.0;
  }

  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める

    File dataFile = SD.open(LOG_AC, FILE_READ);

    if (dataFile) { // ファイルが開けたときの処理
      int now_pos = dataFile.size(); // now_pos は自身の位置を表す
      static const int back_num = 1; // 操作ごとに調べる位置をいくつ戻すか
      dataFile.seek(now_pos); // 位置移動

      for (int j = 0; j < num; j++) { // num回分のデータを取る
        while (1) {

          dataFile.seek(now_pos - back_num); // 調べる位置を一つ戻す

          String line = dataFile.readStringUntil('\r');

          if (line == "*") { // もし*と書いてある行に達したらその下の三行を読む
            String x = dataFile.readStringUntil('\r');
            String y = dataFile.readStringUntil('\r');
            String z = dataFile.readStringUntil('\r');
            if ((x != "*") & (y != "*") & (z != "*")) { // 値が揃っていたなら代入、そうでないなら0のまま
              ac[j].x = x.toDouble();
              ac[j].y = y.toDouble();
              ac[j].z = z.toDouble();
            } else {
              ac[j].x = 0.0;
              ac[j].y = 0.0;
              ac[j].z = 0.0;
            }

            //   xbee_uart( dev,z);
            now_pos -= back_num; //  調べる位置を戻す

            if (now_pos == 0) { // ファイルの先頭に達してしまった場合
              dataFile.close();
              return (-1 * (j + 1)); // 途中で終了したがj+1番目まで取れたことを返す(負の値で)
            }

            break; // j + 1 回目に移行

          } else {
            now_pos -= back_num;
            if (now_pos == 0) {  // ファイルの先頭に達してしまった場合
              dataFile.close();
              return (-1 * j); // 途中で終了したがj番目まで取れたことを返す(負の値で)
            }
          }
          continue;
        }
      }
      dataFile.close();

      return 1; // 要求された処理が完了したことを返す
    } else {
      i += 1;
    }
  }
  return 0;
}


// 地磁気を読み取る
// 第一引数:値を入れる構造体の配列
// 第二引数:何個の値を入れるか
int read_tm_sd(TM tm[100], int num) {

  // 0.0で初期化
  for (int i = 0; i < num; i++) {
    tm[i].x = 0.0;
    tm[i].y = 0.0;
    tm[i].z = 0.0;
  }

  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める

    File dataFile = SD.open(LOG_TM, FILE_READ);

    if (dataFile) { // ファイルが開けたときの処理
      int now_pos = dataFile.size(); // now_pos は自身の位置を表す
      static const int back_num = 1; // 操作ごとに調べる位置をいくつ戻すか
      dataFile.seek(now_pos); // 位置移動

      for (int j = 0; j < num; j++) { // num回分のデータを取る
        while (1) {

          dataFile.seek(now_pos - back_num); // 調べる位置を一つ戻す

          String line = dataFile.readStringUntil('\r');

          if (line == "*") { // もし*と書いてある行に達したらその下の三行を読む
            String x = dataFile.readStringUntil('\r');
            // xbee_uart( dev,x);
            String y = dataFile.readStringUntil('\r');
            //   xbee_uart( dev,y);
            String z = dataFile.readStringUntil('\r');
            if ((x != "*") & (y != "*") & (z != "*")) { // 値が揃っていたなら代入、そうでないなら0のまま
              tm[j].x = x.toDouble();
              tm[j].y = y.toDouble();
              tm[j].z = z.toDouble();
            } else {
              tm[j].x = 0.0;
              tm[j].y = 0.0;
              tm[j].z = 0.0;
            }

            //   xbee_uart( dev,z);
            now_pos -= back_num; //  調べる位置を戻す

            if (now_pos == 0) { // ファイルの先頭に達してしまった場合
              dataFile.close();
              return (-1 * (j + 1)); // 途中で終了したがj+1番目まで取れたことを返す(負の値で)
            }

            break; // j + 1 回目に移行

          } else {
            now_pos -= back_num;
            if (now_pos == 0) {  // ファイルの先頭に達してしまった場合
              dataFile.close();
              return (-1 * j); // 途中で終了したがj番目まで取れたことを返す(負の値で)
            }
          }
          continue;
        }
      }
      dataFile.close();

      return 1; // 要求された処理が完了したことを返す
    } else {
      i += 1;
    }
  }
  return 0;
}


/* struct GPS gps[100];的に宣言かな*/
// GPSを読み取る
// 第一引数:値を入れる構造体の配列
// 第二引数:何個の値を入れるか
int read_gps_sd(GPS gps[100], int num) {

  // 0.0で初期化
  for (int i = 0; i < num; i++) {
    gps[i].utc = 0.0;
    gps[i].latitude = 0.0;
    gps[i].longitude = 0.0;
    gps[i].Speed = 0.0;
    gps[i].course = 0.0;
    gps[i].Direction = 0.0;
    gps[i].distance = 0.0;
  }

  int i = 0; // 試行回数記録用
  while (i < 30) { // 30回SDカードを開けなかったら諦める

    File dataFile = SD.open(LOG_GPS, FILE_READ);

    if (dataFile) { // ファイルが開けたときの処理
      int now_pos = dataFile.size(); // now_pos は自身の位置を表す
      static const int back_num = 1; // 操作ごとに調べる位置をいくつ戻すか
      dataFile.seek(now_pos); // 位置移動

      for (int j = 0; j < num; j++) { // num回分のデータを取る
        while (1) {

          dataFile.seek(now_pos - back_num); // 調べる位置を一つ戻す

          String line = dataFile.readStringUntil('\r');

          if (line == "*") { // もし*と書いてある行に達したらその下の三行を読む
            String utc = dataFile.readStringUntil('\r');
            String latitude = dataFile.readStringUntil('\r');
            String longitude = dataFile.readStringUntil('\r');
            String Speed = dataFile.readStringUntil('\r');
            String course = dataFile.readStringUntil('\r');
            String Direction = dataFile.readStringUntil('\r');
            String distance = dataFile.readStringUntil('\r');

            if ((utc != "*") & (latitude != "*") & (longitude != "*") & (Speed != "*") & (course != "*") & (Direction != "*") & (distance != "*")) { // 値が揃っていたなら代入、そうでないなら0のまま
              gps[j].utc = utc.toDouble();;
              gps[j].latitude = latitude.toDouble();
              gps[j].longitude = longitude.toDouble();
              gps[j].Speed = Speed.toDouble();
              gps[j].course = course.toDouble();
              gps[j].Direction = Direction.toDouble();
              gps[j].distance = distance.toDouble();
            } else {
              gps[j].utc = 0.0;
              gps[j].latitude = 0.0;
              gps[j].longitude = 0.0;
              gps[j].Speed = 0.0;
              gps[j].course = 0.0;
              gps[j].Direction = 0.0;
              gps[j].distance = 0.0;
            }

            //   xbee_uart( dev,z);
            now_pos -= back_num; //  調べる位置を戻す

            if (now_pos == 0) { // ファイルの先頭に達してしまった場合
              dataFile.close();
              return (-1 * (j + 1)); // 途中で終了したがj+1番目まで取れたことを返す(負の値で)
            }

            break; // j + 1 回目に移行

          } else {
            now_pos -= back_num;
            if (now_pos == 0) {  // ファイルの先頭に達してしまった場合
              dataFile.close();
              return (-1 * j); // 途中で終了したがj番目まで取れたことを返す(負の値で)
            }
          }
          continue;
        }
      }
      dataFile.close();

      return 1; // 要求された処理が完了したことを返す
    } else {
      i += 1;
    }
  }
  return 0;
}


