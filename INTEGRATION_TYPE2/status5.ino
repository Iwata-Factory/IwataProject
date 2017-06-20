// ターゲットに近いところを目指す
int status5(ROVER *rover) {

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

    // 目的角を取得
    gps_get(&gps);
    // GPSが取得した値をROVERのステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離

    write_gps_sd(gps);  // 自身の位置をsdに記録

    rover->time_from_start = millis();
    write_timelog_sd(millis(), 5);

    turn_target_direction(rover->Target_Direction, &rover->My_Direction);  // 目的地を向く


    // 偏差の初期化
    double dd_n = 0;
    double dd_n1 = 0;
    double dd_n2 = 0;

    double _mv = 0;  // 概念的な制御量
    int mv = 0;  // 制御量

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

      rover_analog(pid);  // 出力を反映
      // 偏差を更新
      dd_n2 = dd_n1;
      dd_n1 = dd_n;
      dd_n = pid_get_control(rover->Target_Direction, &rover->My_Direction);
      _mv = PID_KP * (dd_n - dd_n1) + PID_KI * dd_n + PID_KD * ((dd_n - dd_n1) - (dd_n1 - dd_n2));  // 概念的制御量を求める
      mv = int(_mv + PID_SURPULS);  // 実際のモーターの制御量(正ならば相対的に右側出力が強くなる)
      arrange_motor_input(&pid, mv);  // 出力を調整
      delay(200);
    }
  } while (1);
}

int arrange_motor_input(DRIVE *drive, int mv) {

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




//（砂に埋まった）とかのスタックした後の脱出アルゴリズム
/*
   とりあえず自分の状況を理解するためのやつです
   状況がわかったら、またそれに対して適切な処理をしやすくするためflag作っておきましたが、まだ使ってないやつあります
*/
int escape_wadachi(double distance_hold, ROVER * rover) { /* こっちの統合ではdistance_holdをまだ定義してなかったね */

  GPS gps_stack;   //GPSの構造体
  double distance[2] = { -1, -1};
  double dif_distance = 0;

  int flag_direction = 0;
  int flag_distance = 0;
  int j = 0;    //試行回数


  while (1) {
    //まずは自分がどういう状況か確認
    //自己位置が変化できるか

    gps_get(&gps_stack);
    distance[0] = gps_stack.distance;
    rover->My_Direction = get_my_direction();
    if (turn_target_direction(rover->My_Direction + 90, &rover->My_Direction) == 1) {
      //回転できる
      flag_direction = 1;
    } else {
      //回転できない
      flag_direction = 0;
    }
    //次にとりあえず走ってみる
    go_straight(4000);

    gps_get(&gps_stack);

    distance[1] = gps_stack.distance;

    dif_distance = fabs(distance[1] - distance[0]);

    if (dif_distance <= 3) {
      //スタックしたまま
      flag_distance = 0;
    } else {
      //脱出成功
      flag_distance = 1;
      return 1;
    }
  }

  if ((flag_distance == 0) && (flag_direction == 1)) {
    //回転等はできるが進めない
    //たぶん轍
    wadachi(rover);
    return 1;
  }
}

/*
   轍に沿って移動はできるけど轍から逃げられない
*/
int wadachi(ROVER * rover) {

  GPS gps;

  double distance_hold = 0;
  double diff_distance = 1000;
  int wadachi_count = 0;

  rover->My_Direction = get_my_direction();
  distance_hold = gps.distance;   //distance保持
  //基本的に下がっては少し右旋回して直進してまた引っかかったら右旋回とやっていき轍を回避できる場所まで行く
  go_back(3000);
  turn_target_direction(rover->My_Direction + 60, &rover->My_Direction);
  go_straight(3000);
  turn_target_direction(rover->My_Direction - 60, &rover->My_Direction);


  gps_get(&gps);

  distance_hold = gps.distance;   //distance保持
  go_straight(5000);

  gps_get(&gps);

  diff_distance = fabs(distance_hold - gps.distance);    //轍回避行動後に

  if (diff_distance <= 2) {
    //轍に引っかかったままなのでもう一度同じ動き
    wadachi_count++;
  } else {
    //轍の引っ掛かりの回避に成功
    return 0;
  }

  if (wadachi_count % 5 == 0) { //ダメなのが続いたらランダムに進んでみる
    go_rotate(wadachi_count * 200);
    go_straight(5000);
  }

}


