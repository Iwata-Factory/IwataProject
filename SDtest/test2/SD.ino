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
  Serial.println("加速度を読みとります。");
  while (i < 30) { // 30回SDカードを開けなかったら諦める
    
    File dataFile = SD.open("aclog.txt", FILE_READ);
    
    if (dataFile) { // ファイルが開けたときの処理
      int now_pos = dataFile.size(); // now_pos は自身の位置を表す
      static int back_num = 1; // 操作ごとに調べる位置をいくつ戻すか
      dataFile.seek(now_pos); // 位置移動
      
      for (int j = 0; j < num; j++) { // num回分のデータを取る
        while (1) {
          
          dataFile.seek(now_pos - back_num); // 調べる位置を一つ戻す
          
          String line = dataFile.readStringUntil('\r');
          
          if (line == "*") { // もし*と書いてある行に達したらその下の三行を読む
            String x = dataFile.readStringUntil('\r');
            // Serial.println(x);
            String y = dataFile.readStringUntil('\r');
            //   Serial.println(y);
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

            //   Serial.println(z);
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
      Serial.println("ファイルオープンに失敗");
      i += 1;
    }
  }
  return 0;
}


