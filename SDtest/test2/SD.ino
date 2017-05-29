/* SDカード周りの関数を書く

  記録系:write

  読み取る系:read

*/

// 加速度を記録
int write_ac_sd(AC ac) {
  int i = 0; // 試行回数記録用
  Serial.println("加速度を記録します。");
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    File dataFile = SD.open("aclog.txt", FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());
      dataFile.println("*"); // 記録の境目
      dataFile.println(ac.x);
      dataFile.println(ac.y);
      dataFile.println(ac.z);

      dataFile.close();
      return 1; // 成功を返す
    } else {
      Serial.println("ファイルオープンに失敗");
      i += 1;
    }
  }
  return 0; // 失敗を返す
}


// 加速度を記録
int write_tm_sd(TM tm) {
  int i = 0; // 試行回数記録用
  Serial.println("地磁気を記録します。");
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    File dataFile = SD.open("tmlog.txt", FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());
      dataFile.println("*"); // 記録の境目
      dataFile.println(tm.x);
      dataFile.println(tm.y);
      dataFile.println(tm.z);
      dataFile.close();
      return 1; // 成功を返す
    } else {
      Serial.println("ファイルオープンに失敗");
      i += 1;
    }
  }
  return 0; // 失敗を返す
}

// GPSを記録
int write_gps_sd(struct GPS gps) {
  int i = 0; // 試行回数記録用
  Serial.println("GPSを記録します。");
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    File dataFile = SD.open("gpslog.txt", FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());
      dataFile.println("*"); // 記録の境目
      dataFile.println(gps.utc);
      dataFile.println(gps.latitude);
      dataFile.println(gps.longitude);
      dataFile.println(gps.Speed);
      dataFile.println(gps.course);
      dataFile.println(gps.Direction);
      dataFile.println(gps.distance);
      dataFile.close();
      dataFile.close();
      return 1; // 成功を返す
    } else {
      Serial.println("ファイルオープンに失敗");
      i += 1;
    }
  }
  return 0; // 失敗を返す
}

int read_ac_sd(AC ac[100], int num) {

  // 0.0で初期化
  for (int i = 0; i < num; i++) {
    ac[i].x = 0.0;
    ac[i].y = 0.0;
    ac[i].z = 0.0;
  }

  int i = 0; // 試行回数記録用
  Serial.println("加速度を読みます。");
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    File dataFile = SD.open("aclog.txt", FILE_READ);
    if (dataFile) { // ファイルが開けたときの処理
      int now_pos = dataFile.size();
      static int back_num = 1;
      dataFile.seek(now_pos);
      for (int j = 0; j < num; j++) {
        while (1) {
          dataFile.seek(now_pos - back_num);
          String line = dataFile.readStringUntil('\r');
          if (line == "*") {
            String x = dataFile.readStringUntil('\r');
            // Serial.println(x);
            ac[j].x = x.toDouble();
            String y = dataFile.readStringUntil('\r');
            //   Serial.println(y);
            ac[j].y = y.toDouble();
            String z = dataFile.readStringUntil('\r');
            //   Serial.println(z);
            ac[j].z = z.toDouble();
            now_pos -= back_num;
            if (now_pos == 0) {
              dataFile.close();
              return (-1 * (j + 1)); // 途中で終了したがj番目まで取れた
            }
            break;
          } else {
            now_pos -= back_num;
            if (now_pos == 0) {
              dataFile.close();
              return (-1 * j); // 途中で終了したがj番目まで取れた
            }
          }
          continue;
        }
      }
      dataFile.close();
      return 1; // 成功を返す
    } else {
      Serial.println("ファイルオープンに失敗");
      i += 1;
    }
  }
  return 0;
}


