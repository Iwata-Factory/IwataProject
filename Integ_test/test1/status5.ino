// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  int i = 0; // do-whileの繰り返し数をカウント

  do {

    if (i % 20 == 0) { // たまにキャリブレーションする
      tm_calibration();
    }

    Serial.println("GPSを取得します");
    // GPS情報を取得
    GPS gps;
    gps_get(&gps);
    // GPSが取得した値を自身のステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離
    Serial.println("取得した値をSDに記録");
    if (write_gps_sd(gps)) { // 自身の位置をsdに記録
      Serial.println("成功");
    } else {
      Serial.println("失敗");
    }

    time = millis(); //現在の時間を取得
    rover->time_from_start = time;
    write_timelog_sd(time, 5);

    Serial.print("ゴールまでの距離は");
    Serial.println(gps.distance);

    if (0 <= rover->distance && rover->distance < 15) { // 15mまで来たら地上2へ
      Serial.println("ゴール付近へ到達");
      return 1;
    }

    Serial.println("機体方向を調整します");
    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    int turn_result = turn_target_direction(rover->Target_Direction, &rover->My_Direction);


    if (turn_result == 0) {
      Serial.println("10回調整に失敗したので諦めます。");
    }

    Serial.println("直進します");
    // 2秒直進
    go_straight(2000);

    i += 1;

  } while (1);
}

