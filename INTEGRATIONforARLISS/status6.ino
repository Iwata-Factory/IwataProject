// 今は2台のGPSでゴールを目指します

int status6(ROVER *rover) {

  if (_S6_ == 0) {
    xbee_uart( dev, "skip status6\r");
    delay(1000);
    return 1;
  }

  int i = 0;  // 最終処理のカウント数

  POINT my_point;
  POINT goal_point;
  goal_point.latitude = GOAL_LATITUDE;
  goal_point.longitude = GOAL_LONGITUDE;

  GPS my_gps_only;

  do {

    write_gps_sd(my_gps_only);
    write_timelog_sd(rover);

    i += 1;

    my_point = gps_get_by_two_module();
    if (my_point.latitude == -1.0 && my_point.longitude == -1.0) {  // GPS2台で取るのは諦め
      ;
    }
    // GPSに緯度と経度だけ入力
    my_gps_only.latitude = my_point.latitude;
    my_gps_only.longitude = my_point.longitude;

    // 距離と方位を取得
    rover->distance = distance_get(&my_gps_only, &goal_point);

    xbee_uart( dev, "last_distance \r");
    dtostrf(rover->distance, 10, 6, xbee_send);
    xbee_uart(dev, xbee_send);

    if (true) {
      if (i < 10) {    //すうかい
        if (get_goal(rover) == 0) {
          //距離センサ関数
        } else {

          return 1;
        }
      } else {
        return 1;
      }

    }

    rover->Target_Direction = direction_get(&my_gps_only, &goal_point);
    rover->My_Direction = get_my_direction();

    // 方向転換
    turn_target_direction(rover->Target_Direction, &rover->My_Direction, 0);
    go_straight(500);

  } while (i < 15);

  xbee_uart( dev, "DANGER DANGER DANGER\r");

  i = 0;
  GPS gps_last;
  do {  // ヤバイ時に走る

    write_gps_sd(gps_last);
    write_timelog_sd(rover);

    i += 1;

    gps_get(&gps_last);

    rover->latitude = gps_last.latitude;  // 緯度
    rover->longitude = gps_last.longitude;  //経度
    rover->Target_Direction = gps_last.Direction;  //ターゲットの方向
    rover->distance = gps_last.distance;  // ターゲットまでの距離

    xbee_uart( dev, "last_distance \r");
    dtostrf(rover->distance, 10, 6, xbee_send);
    xbee_uart(dev, xbee_send);


    if (0 <= rover->distance && rover->distance <= LAST_GOAL_CIRCLE) {  // status6へ
      xbee_uart( dev, "near goal!!!!\r");
      if (i < 10) {    //すうかい
        if (get_goal(rover) == 0) {
          //距離センサ関数
        } else {

          return 1;
        }
      } else {
        return 1;
      }
    }

    turn_target_direction(rover->Target_Direction, &rover->My_Direction, 0);

    go_straight(100);
  } while (i < 20);



  return 1;


}

// 二つのGPSからGPSを平均で取得
POINT gps_get_by_two_module() {

  xbee_uart( dev, "call gps_get_by_two_module\r");

  GPS gps1;
  GPS gps2;
  POINT point1;
  POINT point2;
  POINT success_point;
  POINT false_point;

  int priority_gps = use_which_gps;  // 現在使っているGPS

  gps_get(&gps1);  // GPS1の情報
  gps_switch();  // GPSの変更
  gps_get(&gps2);  // GPS2の情報
  gps_switch();  // GPSの変更(元に戻す)

  point1.latitude = gps1.latitude;
  point1.longitude = gps1.longitude;
  point2.latitude = gps2.latitude;
  point2.longitude = gps2.longitude;

  if (!(0 <= gps1.distance && gps1.distance <= 8 && 0 <= gps2.distance && gps2.distance <= 8)) {  // どちらかのGPSがおかしいかも
    xbee_uart( dev, "false gps_get_by_two_module\r");
    return false_point;
  }

  if (!(0 <= distance_get(&gps1, &point2) && distance_get(&gps1, &point2) <= 16)) {  // どちらかのGPSがおかしいかも
    xbee_uart( dev, "false gps_get_by_two_module\r");
    return false_point;
  }

  // 平均を出す
  success_point.latitude = (point1.latitude + point2.latitude) / 2;
  success_point.longitude = (point1.longitude + point2.longitude) / 2;

  xbee_uart( dev, "success gps_get_by_two_module\r");


  write_control_sd("rover`s point by double gps is (" + String(success_point.latitude, DEC) + ", " + String(success_point.longitude, DEC) + ")");

  return success_point;
}

double get_goal(ROVER *rover) {
  int i = 0;
  int t = 0;
  double para_distance = 0;
  double volt = 0;
  int goal_flag = 0;

  xbee_uart(dev, "get_goal\n");
  DRIVE turn; // DRIVE型の宣言
  //右向き回転
  turn.right1 = 0;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 0;
  rover_degital(turn);
  xbee_uart( dev, "get_goal \r");
  while (t <= 10000) {
    volt = analogRead( DISTANCE ) * 5 / 1023.0;
    xbee_uart( dev, "volt is ");
    dtostrf(volt, 10, 6, xbee_send);
    xbee_uart(dev, xbee_send);
    xbee_uart( dev, "\r");
    if ( 1.4 < volt && volt < 2.7 ) {            //有効測距範囲内

      para_distance = 140.0 / ( volt - 1.10 ) ;
      turn.right1 = 1;
      turn.right2 = 1;
      turn.leght1 = 1;
      turn.leght2 = 1;
      rover_degital(turn);
      rover->My_Direction = get_my_direction();
      delay(1000);
      xbee_uart( dev, "find goal\r");
      xbee_uart( dev, "para_distance is ");
      dtostrf(para_distance, 10, 6, xbee_send);
      xbee_uart(dev, xbee_send);
      xbee_uart( dev, "\r");
      //とりあえず取得した方向に進む
      if (para_distance >= 300) {
        go_straight(500);
        //右向き回転
        turn.right1 = 0;
        turn.right2 = 1;
        turn.leght1 = 1;
        turn.leght2 = 0;
        rover_degital(turn);
        t = 0;
      } else {
        if (para_distance > 100 && para_distance <= 300) {  //一定距離より小さければ、直進して終わり
          go_straight(500);
          goal_flag = 1;
        }
        if (goal_flag == 1) {

          return 1;
        } else {
          xbee_uart( dev, "fail again\r");
          return 0;
        }
      }
      xbee_uart( dev, "again\r");
    }
    xbee_uart( dev, "there is no goal\r");
    delay(50);
    t += 50;
  }


  //停止
  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;
  rover_degital(turn);
  delay(1000);

  //失敗した場合ランダムウォーク
  double r_number = random(0, 11); // 0から10の乱数を生成

  double rotate_random = 40 + 150 * (r_number + 1 / 10);
  write_control_sd("random num: " + String(r_number, DEC));
  go_rotate(rotate_random);
  go_straight(1000);


  return 0;

}



