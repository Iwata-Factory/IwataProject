// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  if (_S5_ == 0) {
    xbee_uart( dev, "skip status5\r");
    delay(1000);
    return 1;
  }

  int i = 0; // do-whileの繰り返し数をカウント
  double last_distance = -1;  // スタック判定用

  if (STACK_EXP == 1) {
    stack_check_state(rover);  // スタックのフラグを立てる
  }

  int do_stack_check = 1;  // スタック判定するかのフラグ

  do {

    if (correct_posture() == 1) {  // 判定修正
      delay(10);
    } else {
      delay(10);
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

    if (0 < rover->distance && rover->distance < 10 && NEAR_GOAL_STACK_EXP != 1) {
      do_stack_check = 0;
    }

    // スタック判定
    if (do_stack_check == 1) {
      if (i == 0) {  // last_distanceの初期値を生成
        last_distance  = rover->distance;
      } else {
        if ((fabs(rover->distance - last_distance) < 2.5) && (0 < last_distance)) {  //Trueでスタック
          int scs_result = stack_check_state(rover);
          if (scs_result != 1) {
            continue;
          }
        } else {
          last_distance = rover->distance; // スタックで無かった時はlast_distanceを更新
        }
      }
    }

    if (0 <= rover->distance && rover->distance <= 1.0) {  // status6へ
      xbee_uart( dev, "near goal\r");
      break;
    }

    write_gps_sd(gps);
    write_timelog_sd(rover);

    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    turn_target_direction(90, &rover->My_Direction, 0);

    if (rover->distance < 0 || 10 < rover->distance) {
      //      go_straight(10000); // 10秒直進
      go_straight_control(10000, 90);

    } else {
      go_straight(2000); //  短距離直進
    }

    i += 1;

  } while (1);

  return 1;

}

