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

    judge_invered_revive(); //状態復旧

    if (i % 30 == 0) { // たまにキャリブレーションする
      tm_calibration();  // 条件が揃ったらキャリブレーション
    }

    if (4 <= i) {  // 4回目からは危険エリアチェック
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

    // スタック判定
    if (i == 0) {  // last_distanceの初期値を生成
      last_distance  = gps.distance;
    } else {
      delay(3000);
      if (fabs(gps.distance - last_distance) < 3.0) {  //Trueでスタック
        escape_stack(rover);
        delay(3000);
        last_distance = gps.distance;
      } else {
        last_distance = gps.distance;
      }
    }

    write_gps_sd(gps);
    write_timelog_sd(rover);

    if (0 <= rover->distance && rover->distance < 15) { // 15mまで来たら地上2へ
      xbee_uart( dev, "near goal\r");
      return 1;
    }

    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    turn_target_direction(rover->Target_Direction, &rover->My_Direction);
    go_straight(7000); // 7秒直進

    speaker(E_TONE);
    speaker(F_TONE);
    speaker(G_TONE);

    i += 1;

  } while (1);
}


//（砂に埋まった）とかのスタックした後の脱出アルゴリズム
/*
   とりあえず自分の状況を理解するためのやつです
   状況がわかったら、またそれに対して適切な処理をしやすくするためflag作っておきましたが、まだ使ってないやつあります
*/
int escape_stack(ROVER *rover) {

  xbee_uart(dev, "call escape_wadachi\r");

  GPS gps_stack;   //GPSの構造体
  double distance[2] = { -1, -1};
  double dif_distance = 0;

  int flag_direction = 0;
  int flag_distance = 0;
  int j = 0;    //試行回数


  while (1) {
    //まずは自分がどういう状況か確認
    //自己位置が変化できるか
    j += 1;
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

    if (dif_distance <= 2) {
      //スタックしたまま
      flag_distance = 0;
    } else {
      //脱出成功
      flag_distance = 1;
      xbee_uart(dev, "success escape_wadachi\r");
      return 1;
    }
  }

  if ((flag_distance == 0) && (flag_direction == 1)) {
    //回転等はできるが進めない
    //たぶん轍
    if (escape_from_wadachi(rover) == 1) {
      xbee_uart(dev, "success escape_wadachi\r");
      return 1;
    } else {
      xbee_uart(dev, "false escape_wadachi\r");
      return 0;
    }
  }

  if (j == 3) {  // 試行回数3回で終了
    xbee_uart(dev, "false escape_wadachi\r");
    return 0;
  }

}

/*
   轍に沿って移動はできるけど轍から逃げられない
*/
int escape_from_wadachi(ROVER *rover) {

  xbee_uart(dev, "call escape_from_wadachi\r");

  GPS gps;

  double distance_hold = 0;
  double diff_distance = 1000;
  int wadachi_count = 0;

  while (1) {

    rover->My_Direction = get_my_direction();
    distance_hold = gps.distance;   //distance保持
    //基本的に下がっては少し右旋回して直進してまた引っかかったら右旋回とやっていき轍を回避できる場所まで行く
    turn_target_direction(rover->My_Direction + 150, &rover->My_Direction);
    go_straight(4000);
    rover->My_Direction = get_my_direction();
    turn_target_direction(rover->My_Direction - 150, &rover->My_Direction);

    gps_get(&gps);

    distance_hold = gps.distance;   //distance保持
    go_straight(5000);

    gps_get(&gps);

    diff_distance = fabs(distance_hold - gps.distance);    //轍回避行動後に

    if (diff_distance <= 2) {
      //轍に引っかかったままなのでもう一度同じ動き
      wadachi_count++;
    } else {
      //轍の引っ掛かりの回避に成功]
      xbee_uart(dev, "success escape_from_wadachi\r");
      return 1;
    }

    if (wadachi_count % 5 == 0) { //ダメなのが続いたらランダムに進んでみる
      go_rotate(wadachi_count * 200);
      go_straight(5000);
    }
    if (wadachi_count % 11 == 0) { // 失敗
      xbee_uart(dev, "false escape_from_wadachi\r");
      return 0;
    }
  }
}



