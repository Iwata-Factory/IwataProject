// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  int i = 0; // do-whileの繰り返し数をカウント

  do {


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

//（砂に埋まった）とかのスタックした後の脱出アルゴリズム
/*
   とりあえず自分の状況を理解するためのやつです
   状況がわかったら、またそれに対して適切な処理をしやすくするためflag作っておきましたが、まだ使ってないやつあります
*/
int escape(double distance_hold, ROVER *rover) {  /* こっちの統合ではdistance_holdをまだ定義してなかったね */

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
int wadachi(ROVER *rover) {
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


