// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  int i = 0; // do-whileの繰り返し数をカウント

  do {
    
    AC ac_going;  // 反転判定用
    
    while (1) {  // 判定していないかチェック

      if (ac_going.z < 0) {  // 加速度が負の場合
        speaker(C_TONE);  // 音を鳴らす
        speaker(C_TONE);
        speaker(C_TONE);
        delay(1000);
        go_back(5000);
      } else {
        delay(1000);
        break;
      }
    }

    if (i % 30 == 0) { // たまにキャリブレーションする

      AC ac_calib;  // キャリブレーション時の水平判定用
      int count_calib = 0;  // 非水平カウント用

      while (1) {

        ac_calib = get_ac();

        if ((fabs(ac_calib.x) < 50 && fabs(ac_calib.y) < 50 && 200 < ac_calib.z) || count_calib == 5) {  // 水平な感じの場所にいるならキャリブレーション。試行回数過多でもキャリブレーション
          speaker(C_TONE);
          speaker(D_TONE);
          tm_calibration();  // 条件が揃ったらキャリブレーション
          break;
        } else {
          count_calib += 1;
          go_straight(1500);  //水平な場所を目指す
        }
      }

    }

    xbee_uart( dev,"get gps\r");
    // GPS情報を取得
    GPS gps;
    gps_get(&gps);
    // GPSが取得した値を自身のステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離
    xbee_uart( dev,"logging");
    if (write_gps_sd(gps)) { // 自身の位置をsdに記録
      xbee_uart( dev,"success\r");
    } else {
      xbee_uart( dev,"fail\r");
    }

    rover->time_from_start =  millis();//現在の時間を取得
    write_timelog_sd(time, 5);

    sprintf(xbee_send, "distance to goal is %f\r", gps.distance);
    xbee_uart(dev, xbee_send);
//    xbee_uart( dev,"ゴールまでの距離は");
//    xbee_uart( dev,gps.distance);

    if (0 <= rover->distance && rover->distance < 15) { // 15mまで来たら地上2へ
      xbee_uart( dev,"near goal\r");
      return 1;
    }

    xbee_uart( dev,"balancing rover\r");
    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    int turn_result = turn_target_direction(rover->Target_Direction, &rover->My_Direction);


    if (turn_result == 0) {
      xbee_uart( dev,"give up!!!\r");
    }

    xbee_uart( dev,"go straight\r");
    // 3秒直進
    go_straight(3000);

    speaker(E_TONE);
    speaker(F_TONE);
    speaker(G_TONE);



    i += 1;

  } while (1);
}

