// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  int i = 0; // do-whileの繰り返し数をカウント

  do {

    if (i % 10 == 0) { // たまにキャリブレーションする
      tm_calibration();
    }

    // GPS情報を取得
    GPS gps;
    gps_get(&gps);
    // GPSが取得した値を自身のステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離

    write_gps_sd(gps);  // 自身の位置をsdに記録
    
    time = millis(); //現在の時間を取得
    rover->time_from_start = time;
    write_timelog_sd(time, 5);

    if (rover->distance < 5) { // 5mまで来たら地上2へ
      return 1;
    }

    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    int turn_result = turn_target_direction(rover->Target_Direction, &rover->My_Direction);

    // 2秒直進
    go_straight(2000);

    i += 1;

  } while (1);
}

