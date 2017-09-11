// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  if (_S5_ == 0) {
    xbee_uart( dev, "skip status5\r");
    delay(1000);
    return 1;
  }

  int c_r = check_danger_area2();  // 1~3
  int try_via_c = 0;

  int i = 0; // do-whileの繰り返し数をカウント
  double last_distance = -1;  // スタック判定用

  if (STACK_EXP == 1) {
    stack_check_state(rover);  // スタックのフラグを立てる
  }

  int do_stack_check = 0;  // スタック判定するかのフラグ

  GPS gps;
  POINT last_point;

  do {

    if (c_r != 1) {
      try_via_c += 1;
    }

    if (try_via_c == 30) {
      c_r = 1;
    }

    if (correct_posture() == 1) {  // 判定修正
      delay(10);
    } else {
      delay(10);
    }

    if ((i + 24) % 30 == 0) { // たまにキャリブレーションする
      tm_calibration();  // 条件が揃ったらキャリブレーション
    }

    if ((i + 9) % 10 == 0) {  // 定期的に反転修正
      go_suddenly_brake(2500);
    }

    if (5 <= i) {  // 5回目からは危険エリアチェック
      check_danger_area();
    }

    // GPS情報を取得
    if (c_r == 1) {
      gps_get(&gps);
    } else if (c_r == 2) {
      gps_get(&gps);
      gps_get_l(&gps);
    } else {
      gps_get(&gps);
      gps_get_r(&gps);

    }


    // GPSが取得した値を自身のステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離

    if (check_gps_jump(&gps, &last_point) == 0) { // GPSのジャンプのチェック
      write_control_sd("gps difference > " + String(GPS_JUMP_DISTANCE, DEC) + "---> gps jump");
      continue;
    }

    if (0 < rover->distance && rover->distance < 15 && NEAR_GOAL_STACK_EXP != 1 && c_r == 1) {
      do_stack_check = 0;
    }

    // スタック判定
    if (do_stack_check == 1) {
      if (i == 0) {  // last_distanceの初期値を生成
        last_distance  = rover->distance;
      } else {
        if ((fabs(rover->distance - last_distance) < 2) && (0 < last_distance)) {  //Trueでスタック
          write_control_sd(F("distance difference < 2 ---> stack"));
          int scs_result = stack_check_state(rover);
          if (scs_result != 1) {
            continue;
          }
        } else {
          last_distance = rover->distance; // スタックで無かった時はlast_distanceを更新
        }
      }
    }

    if (c_r == 1) {
      if (0 <= rover->distance && rover->distance <= GOAL_CIRCLE) {  // status6へ
        xbee_uart( dev, "near goal\r");
        break;
      }
    } else if (c_r == 2) {
      if (0 <= rover->distance && rover->distance <= VIA_CIRCLE) {  // status6へ
        xbee_uart( dev, "via goal\r");
        c_r = 1;
      }
    } else {
      if (0 <= rover->distance && rover->distance <= VIA_CIRCLE) {  // status6へ
        xbee_uart( dev, "via goal\r");
        c_r = 1;
      }
    }


    write_gps_sd(gps);
    write_timelog_sd(rover);

    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    turn_target_direction(rover->Target_Direction, &rover->My_Direction, 0);

    int arg = get_go_argument(rover->distance);

    if (PI_FLAG == 1) {
      if (go_straight_control(arg, rover->Target_Direction) == 0) {
        xbee_uart( dev, "pi stop ---> stack\r");
        write_control_sd(F("pi stop ---> stack"));

        int scs_result2 = stack_check_state(rover);
        if (scs_result2 != 1) {
          continue;
        }
      }
    } else {
      if (500 < arg) {  // 出力調整
        arg = 10000;
      } else if (250 < arg && arg <= 500) {
        arg = 8000;
      } else {
        arg = 6000;
      }
      go_straight(arg);
    }

    i += 1;

    do_stack_check = 1;  // スタック判定をon

  } while (1);

  return 1;

}

// 直進関数の引数を決める
int get_go_argument (double last_distance) {
  if (last_distance < 0 || 300 < last_distance) {
    return 3000;
  } else if (50 < last_distance && last_distance <= 300) {
    return 1000;
  } else if (25 < last_distance && last_distance <= 50) {
    return 600;
  } else if (15 < last_distance && last_distance <= 25) {
    return 300;
  } else if (5 < last_distance && last_distance <= 15) {
    return 50;
  } else {
    return 15;
  }
}


int check_gps_jump(GPS * gps, POINT * point) {

  if (point->latitude == -1.0 && point->longitude == -1.0) {  // 初期化
    point->latitude = gps->latitude;
    point->longitude = gps->longitude;
    return 1;
  } else {
    double gps_difference = distance_get(gps, point);  // GPSのジャンプを計算
    if (gps_difference < GPS_JUMP_DISTANCE) {  // 正常
      point->latitude = gps->latitude;
      point->longitude = gps->longitude;
      xbee_uart( dev, "no jump gps ---> ok\r");
      return 1;
    } else {
      gps_switch();  // GPS切り替え
      point->latitude = -1.0;  // pointの初期化
      point->longitude = -1.0;
      xbee_uart( dev, "jump gps ---> switch\r");
      return 0;
    }
  }
}

int check_danger_area2() {

  xbee_uart( dev, "check danger area\r");
  write_control_sd(F("check danger area"));

  delay(1000);


  GPS cda_gps;
  gps_get(&cda_gps);

  double c_lat = cda_gps.latitude;
  double c_lng = cda_gps.longitude;

  //2.00969x - y + 36.9663169 = 0
  //0.49758918x + y - 139.438782 = 0

  double c1 = 2.00969 * c_lat - c_lng + 36.9663169;
  double c2 = 0.49758918 * c_lat + c_lng - 139.438782;

  //A 負正
  //B 正正
  //C 正負
  //D 負負

  int area = 0; //1~4
  if (c1 < 0) {
    if (c2 < 0) {
      area = 4;
      xbee_uart( dev, "my area is 4\r");
      write_control_sd(F("my area is 4"));
    } else {
      area = 1;
      xbee_uart( dev, "my area is 1\r");
      write_control_sd(F("my area is 1"));
    }
  } else {
    if (c2 < 0) {
      area = 3;
      xbee_uart( dev, "my area is 3\r");
      write_control_sd(F("my area is 3"));
    } else {
      area = 2;
      xbee_uart( dev, "my area is 2\r");
      write_control_sd(F("my area is 2"));
    }
  }

  if ((area == 1) || (area == 2)) {
    xbee_uart( dev, "don`t set via point\r");
    write_control_sd(F("don`t set via point"));
    return 1;
  } else if (area == 3) {
    xbee_uart( dev, "set via point L\r");
    write_control_sd(F("set via point L"));
    return 2;
  } else {
    xbee_uart( dev, "set via point R\r");
    write_control_sd(F("set via point R"));
    return 3;
  }
}

//int go_point(double via_l, double via_r) {
//
//  ROVER rover;
//
//  int i = 0; // do-whileの繰り返し数をカウント
//  double last_distance = -1;  // スタック判定用
//
//  if (STACK_EXP == 1) {
//    stack_check_state(&rover);  // スタックのフラグを立てる
//  }
//
//  int do_stack_check = 0;  // スタック判定するかのフラグ
//
//  GPS gps;
//  POINT last_point;
//
//  do {
//
//    if (correct_posture() == 1) {  // 判定修正
//      delay(10);
//    } else {
//      delay(10);
//    }
//
//    if ((i + 24) % 30 == 0) { // たまにキャリブレーションする
//      tm_calibration();  // 条件が揃ったらキャリブレーション
//    }
//
//    if ((i + 9) % 10 == 0) {  // 定期的に反転修正
//      go_suddenly_brake(2500);
//    }
//
//    if (5 <= i) {  // 5回目からは危険エリアチェック
//      check_danger_area();
//    }
//
//    // GPS情報を取得
//    gps_get(&gps);
//
//
//    // GPSが取得した値を自身のステータスに反映する。
//    rover.latitude = gps.latitude;  // 緯度
//    rover.longitude = gps.longitude;  //経度
//    rover.Target_Direction = gps.Direction;  //ターゲットの方向
//    rover.distance = gps.distance;  // ターゲットまでの距離
//
//    if (check_gps_jump(&gps, &last_point) == 0) { // GPSのジャンプのチェック
//      write_control_sd("gps difference > " + String(GPS_JUMP_DISTANCE, DEC) + "---> gps jump");
//      continue;
//    }
//
//    if (0 < &rover.distance && &rover.distance < 15 && NEAR_GOAL_STACK_EXP != 1) {
//      do_stack_check = 0;
//    }
//
//    // スタック判定
//    if (do_stack_check == 1) {
//      if (i == 0) {  // last_distanceの初期値を生成
//        last_distance  = &rover.distance;
//      } else {
//        if ((fabs(&rover.distance - last_distance) < 2) && (0 < last_distance)) {  //Trueでスタック
//          write_control_sd(F("distance difference < 2 ---> stack"));
//          int scs_result = stack_check_state(&rover);
//          if (scs_result != 1) {
//            continue;
//          }
//        } else {
//          last_distance = &rover.distance; // スタックで無かった時はlast_distanceを更新
//        }
//      }
//    }
//
//    if (0 <= &rover.distance && &rover.distance <= GOAL_CIRCLE) {  // status6へ
//      xbee_uart( dev, "near goal\r");
//      break;
//    }
//
//    write_gps_sd(gps);
//    write_timelog_sd(rover);
//
//    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
//    turn_target_direction(*rover.Target_Direction, rover.My_Direction, 0);
//
//    int arg = get_go_argument(*rover.distance);
//
//    if (PI_FLAG == 1) {
//      if (go_straight_control(arg, *rover.Target_Direction) == 0) {
//        xbee_uart( dev, "pi stop ---> stack\r");
//        write_control_sd(F("pi stop ---> stack"));
//
//        int scs_result2 = stack_check_state(&rover);
//        if (scs_result2 != 1) {
//          continue;
//        }
//      }
//    } else {
//      if (500 < arg) {  // 出力調整
//        arg = 10000;
//      } else if (250 < arg && arg <= 500) {
//        arg = 8000;
//      } else {
//        arg = 6000;
//      }
//      go_straight(arg);
//    }
//
//    i += 1;
//
//    do_stack_check = 1;  // スタック判定をon
//
//  } while (1);
//
//
//}

