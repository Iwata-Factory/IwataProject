// ターゲットに近いところを目指す
int status5_2(ROVER *rover) {

  if (_S5_ == 0) {
    xbee_uart( dev, "skip status5_2\r");
    delay(1000);
    return 1;
  }

  xbee_uart( dev, "status5_2\r");


  int i = 0; // do-whileの繰り返し数をカウント

  GPS gps;
  DRIVE pid;  // DRIVEの初期化

  do {

    // 一旦停止
    pid.right1 = 0;
    pid.right2 = 0;
    pid.leght1 = 0;
    pid.leght2 = 0;
    rover_analog(pid);


    if (i % 10 == 0) {  // 一定期間ごとにキャリブレーションを実施
      tm_calibration();
    }
    xbee_uart(dev, "get_gps\r");
    // 目的角度を取得
    gps_get(&gps);
    // GPSが取得した値をROVERのステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離

    write_gps_sd(gps);  // 自身の位置をsdに記録

    write_timelog_sd(rover);

    turn_target_direction(rover->Target_Direction, &rover->My_Direction);  // 目的地を向く


    // 偏差の初期化
    double dd_n = 0;
    double dd_n1 = 0;
    double dd_n2 = 0;

    double _mv = 0;  // 概念的な制御量
    int mv = 0;  // 制御量
    xbee_uart(dev, "PID START\r");
    // 加速部
    for (int i = 1; i < 256; i++) {
      pid.right1 = 0;
      pid.right2 = i;
      pid.leght1 = 0;
      pid.leght2 = i;
      rover_analog(pid);
      delay(2);
    }

    for (int j = 0; j < 600; j++) {  // 600 * 200 = 120000(120秒ごと)

      if (j % 150 == 0) {  //30秒に一度ゴール付近にいるか確認
        gps_get(&gps);  // GPSを取得
        if (0 <= gps.distance && gps.distance < 30) {
          pid.right1 = 0;
          pid.right2 = 0;
          pid.leght1 = 0;
          pid.leght2 = 0;
          rover_analog(pid);
          return 1;
        }
      }

      rover_analog(pid);  // 出力を反映
      // 偏差を更新
      dd_n2 = dd_n1;
      dd_n1 = dd_n;
      dd_n = pid_get_control(rover->Target_Direction, &rover->My_Direction);
      _mv = PID_KP * (dd_n - dd_n1) + PID_KI * dd_n + PID_KD * ((dd_n - dd_n1) - (dd_n1 - dd_n2));  // 概念的制御量を求める
      mv = int(_mv + PID_SURPULS);  // 実際のモーターの制御量(正ならば相対的に右側出力が強くなる)
      arrange_motor_input(&pid, mv);  // 出力を調整
      delay(200);

      if (j % 10) {
        char m[sizeof(mv)];
        sprintf(m, "timer: %d\r", mv);
        xbee_uart( dev, m);
      }
    }

  } while (1);
  xbee_uart(dev, "PID END\r");
}


int arrange_motor_input(DRIVE * drive, int mv) {

  mv = arrange_mv(mv);

  // 目標標準出力と現在の出力との差を算出
  int target_r2 = 255;
  int target_l2 = 255;
  int difference_target_r2 = target_r2 - drive->right2;
  int difference_target_l2 = target_l2 - drive->leght2;

  if (0 < mv) {  // 出力の調整

    if (mv <= difference_target_r2) {
      drive->right2 += mv;
    } else {
      drive->right2 = 255;
      drive->leght2 = drive->leght2 + mv - difference_target_r2;
    }
  } else {
    mv = -1 * mv;  // 正の値に直す
    if (mv <= difference_target_l2) {
      drive->leght2 += mv;
    } else {
      drive->leght2 = 255;
      drive->right2 = drive->right2 + mv - difference_target_l2;
    }
  }
  return 1;
}

int arrange_mv(int mv) {  //極端な値を弾く
  if (15 < mv) {
    mv = 15;
  } else if (mv < -15) {
    mv = -15;
  }
  return mv;
}
