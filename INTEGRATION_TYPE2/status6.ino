int status6(ROVER *rover) {

  if (_S6_ == 0) {
    xbee_uart( dev, "skip status6\r");
    delay(1000);
    return 1;
  }

  POINT first_point;
  POINT check_point[8];   //現在地の周りの緯度経度を設定
  int i = 0;
  int check_flag = 0;
  
  GPS gps;
  gps_get(&gps);
  // GPSが取得した値を自身のステータスに反映する。
  rover->latitude = gps.latitude;  // 緯度
  rover->longitude = gps.longitude;  //経度

  //現在地の周りを設定
  get_rover_point(&first_point);
  set_goal(&first_point, check_point);

  check_flag = check_goal(rover);
  if (check_flag == 1){
    get_goal(rover);
  }


  return 1;
}

/*
   その周囲に何かあるか判定する
   何かある return 1
   何もない return 0
*/
int check_goal(ROVER *rover) {
  double integ = 0;  //距離センサの積分値
  int i = 0;
  double para_distance = 0;
  double volt = 0;
  DRIVE turn; // DRIVE型の宣言
  //右向き回転（ゆっくりメ)
  turn.right1 = 0;
  turn.right2 = 100;
  turn.leght1 = 100;
  turn.leght2 = 0;
  rover_analog(turn);
  for (i = 0; i <= 3000; i++) {
    volt = analogRead( DISTANCE ) * 5 / 1023.0;
    if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
      para_distance = 140.0 / ( volt - 1.10 ) ;
      integ = integ + 1;
    }
    delay(1);
  }
  //停止
  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;
  rover_degital(turn);
  delay(1000);

  //何かあったか
  if (integ >= 10) {
    return 1;
  } else {
    return 0;
  }
  return 0;
}

/*
   何かあった時にその方向を調べる
   方向合わせに成功　return 1
  　　　　　　 失敗　return 0
*/

double get_goal(ROVER *rover) {
  double integ = 0;  //距離センサの積分値
  int i = 0;
  int t = 0;
  double para_distance = 0;
  double volt = 0;

  DRIVE turn; // DRIVE型の宣言
  //右向き回転
  turn.right1 = 0;
  turn.right2 = 50;
  turn.leght1 = 50;
  turn.leght2 = 0;
  rover_analog(turn);
  while (t >= 10000) {
    volt = analogRead( DISTANCE ) * 5 / 1023.0;
    if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
      para_distance = 140.0 / ( volt - 1.10 ) ;
      turn.right1 = 1;
      turn.right2 = 1;
      turn.leght1 = 1;
      turn.leght2 = 1;
      rover_degital(turn);
      delay(1000);
      rover->My_Direction = get_my_direction();
      //とりあえず取得した方向に進む
      go_straight(para_distance*1000);
      return 1;
    }
    delay(1);
  }
  //停止
  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;
  rover_degital(turn);
  delay(1000);

  return 0;

}
/*---------set_goal(POINT *forst_point)----
   status6に入った場所から周囲にチェック緯度経度を設定
   およそ5mの距離の場所にセット
  ------------------------------------------*/
int set_goal(POINT *first_point, POINT *check_point) {
  double dif = 0.00005;  //緯度経度の差（およそ5m離れる）
  double rad = 0;
  int i = 0;

  for (i = 0; i < 8; i++) {
    check_point[i].latitude = first_point->latitude + dif * cos(2 * pi * i / 8);
    check_point[i].longitude = first_point->longitude + dif * sin(2 * pi * i / 8);
  }
  return 1;
}

