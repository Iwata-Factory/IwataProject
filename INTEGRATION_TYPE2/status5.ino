// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  if (_S5_ == 0) {
    xbee_uart( dev, "skip status5\r");
    delay(1000);
    return 1;
  }

  int i = 0; // do-whileの繰り返し数をカウント
  double last_distance = 0;  // スタック判定用

  do {

    if (correct_posture() == 1) {  // 判定修正
      ;
    } else {
      ;
    }

    if (i % 30 == 0) { // たまにキャリブレーションする
      tm_calibration();  // 条件が揃ったらキャリブレーション
    }

    if (5 <= i) {  // 5回目からは危険エリアチェック
      check_danger_area();
    }

    // GPS情報を取得
    GPS gps;
    gps_get(&gps);
    // GPSが取得した値を自身のステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離

    // xbee_printf が問題なければ後で消します
    // xbee_uart( dev, "Latitude");
    // xbee_send_1double(rover->latitude);
    // xbee_uart( dev, "Longitude");
    // xbee_send_1double(rover->longitude);
    // xbee_uart( dev, "Distance");
    // xbee_send_1double(rover->distance);
    // xbee_uart( dev, "Direction");
    // xbee_send_1double(rover->Target_Direction);


    // スタック判定
    if (i == 0) {  // last_distanceの初期値を生成
      last_distance  = gps.distance;
    } else {
      if (fabs(gps.distance - last_distance) < 2.0) {  //Trueでスタック
        stack_check_state(rover);
        continue;
      } else {
        last_distance = gps.distance; // スタックで無かった時はlast_distanceを更新
      }
    }

    if (last_distance < 1.5) {  // status6へ
      break;
    }

    write_gps_sd(gps);
    write_timelog_sd(rover);

    if (0 <= rover->distance && rover->distance < 10) { // 10mまで来たら地上2へ
      xbee_uart( dev, "near goal\r");
      return 1;
    }

    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    turn_target_direction(rover->Target_Direction, &rover->My_Direction, 0);
    if (10 < last_distance) {
      go_straight(6000); // 6秒直進
    } else {
      go_straight(1000); // 3秒直進
    }

    speaker(E_TONE);
    speaker(F_TONE);
    speaker(G_TONE);

    i += 1;

  } while (1);

  return 1;

}

