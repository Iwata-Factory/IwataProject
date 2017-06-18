// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  int i = 0; // do-whileの繰り返し数をカウント
  GPS gps;

  DRIVE pid;  // DRIVEの初期化

  // 比例定数
  double kp = 1.0;
  double ki = 1.0;
  double kd = 1.0;


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

    // 目標標準出力
    int target_r2 = 255;
    int target_l2 = 255;
    int difference_target_r1 = pid.right2 - target_r2;
    int difference_target_l1 = pid.leght2 - target_l2;

    // 偏差の初期化
    double dd_n = pid_get_control(rover->Target_Direction, &rover->My_Direction);
    double dd_n1 = dd_n;
    double dd_n2 = dd_n;

    double mv = 0.0;  // 概念的な制御用
    double control_ratio = 0.0;  // モーターの出力比

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

      rover_analog(pid);

      // 偏差を更新
      dd_n2 = dd_n1;
      dd_n1 = dd_n;
      dd_n = pid_get_control(rover->Target_Direction, &rover->My_Direction);

      /* 設計---左右の出力比を調整する（大きい方は255で） */
      mv = kp * (dd_n - dd_n1) + ki * dd_n + kd * ((dd_n - dd_n1) - (dd_n1 - dd_n2));  // 制御量を求める
      control_ratio = mv;  //  ここで制御量の比を求める(0~1)

      if (dd_n <= 0) {  // 右方向に向きたい
        // この場合左側の出力の方が大きくならなければならないので、以下のようにする。

        // 左側出力の決定
        pid.leght2 += 5;
        if (255 < pid.leght2) {  // 255より大きい場合調整する
          pid.leght2 = 255;
        }

        // 右側出力の決定
        double next_right2 = pid.leght2 * control_ratio;
        if (next_right2 <= pid.right2 - 30) {
          next_right2 = pid.right2 - 30;
        } else if ((pid.right2 - 30 < next_right2) && (next_right2 < pid.right2 + 30)) {
          next_right2 = next_right2;
        } else {
          if (pid.right2 + 30 < 256) {
            next_right2 = pid.right2 + 30;
          } else {
            next_right2 = 255;
          }
        }
      }

      // dd>0の時を書く
      delay(200);
    }
    

  } while (1);
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


