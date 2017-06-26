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
  double turn_angle = 0;
  int t = 0;

  GPS gps;
  gps_get(&gps);
  // GPSが取得した値を自身のステータスに反映する。
  rover->latitude = gps.latitude;  // 緯度
  rover->longitude = gps.longitude;  //経度

  //現在地の周りを設定
  get_rover_point(&first_point);
  xbee_uart(dev, "set_goal\n");
  set_goal(&first_point, check_point);

  check_flag = check_goal(rover);
  if (check_flag == 1) {
    get_goal(rover);
    xbee_uart(dev, "end\n");
    return 1;
  }

  for (i = 0; i <= 8; i++) {
  /*ここにcheck_pointに緯度経度に向かう関数入れる*/
  reach_point(rover, &check_point[i]);
  check_flag = check_goal(rover);
    if (check_flag == 1) {
      get_goal(rover);
      xbee_uart(dev, "end\n");
      return 1;
    }
  }

  reach_point(rover, &first_point);


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
  //右向き回転（ゆっくりめ)
  turn.right1 = 0;
  turn.right2 = 150;
  turn.leght1 = 150;
  turn.leght2 = 0;
  rover_analog(turn);

  for (i = 0; i <= 5000; i++) {
    volt = analogRead( DISTANCE ) * 5 / 1023.0;
    if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
      para_distance = 140.0 / ( volt - 1.10 ) ;
      integ = integ + 1;
    }
    delay(1);
  }
  xbee_uart( dev, "check_goal\n");
  xbee_send_1double(integ);
  //停止
  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;
  rover_degital(turn);
  delay(1000);

  //何かあったか
  if (integ >= 10) {
    xbee_uart(dev, "goal is near\n");
    return 1;
  } else {
    xbee_uart(dev, "there is no goal.\n");
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

  xbee_uart(dev, "get_goal\n");
  DRIVE turn; // DRIVE型の宣言
  //右向き回転
  turn.right1 = 0;
  turn.right2 = 50;
  turn.leght1 = 50;
  turn.leght2 = 0;
  rover_analog(turn);
  while (t <= 10000) {
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
      if (para_distance >= 3) {
        go_straight(1000);
      } else {
        go_straight(1000);
        return 1;
      }

    }
    delay(1);
    t++;
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

/*---------reach_point()-------
   入力した目的地に向かう関数
   成功 return 1
   失敗 return 0
  ------------------------------*/
int reach_point(ROVER* rover, POINT* point) {
  int i = 0; // do-whileの繰り返し数をカウント
  double last_distance = 0;  // スタック判定用

  do {

    if (correct_posture() == 1) {
      ;
    } else {
      ;
    }

    if (i % 30 == 15) { // たまにキャリブレーションする
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
      delay(3000);
      if (fabs(gps.distance - last_distance) < 2.0) {  //Trueでスタック
        stack_check_state(rover);
        delay(3000);
        last_distance = gps.distance;
      } else {
        last_distance = gps.distance;
      }
    }

    write_gps_sd(gps);
    write_timelog_sd(rover);

    if (0 <= rover->distance && rover->distance < 5) { // 5mまで来たら地上2へ
      xbee_uart( dev, "near goal\r");
      return 1;
    }

    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    turn_target_direction(rover->Target_Direction, &rover->My_Direction, 0);
    if (10 < last_distance) {
      go_straight(6000); // 6秒直進
    } else {
      go_straight(3000); // 3秒直進
    }


    speaker(E_TONE);
    speaker(F_TONE);
    speaker(G_TONE);

    i += 1;

  } while (1);
}



int gps_data_get_goal(GPS* gps, POINT* point) {

  //  char g_szReadBuffer[READBUFFERSIZE] = "";
  //  int  g_iIndexChar = 0;
  char szLineString[READBUFFERSIZE];

  if ( !ReadLineString( g_gps,
                        g_szReadBuffer, READBUFFERSIZE, g_iIndexChar,
                        szLineString, READBUFFERSIZE ) )
  { // 読み取り途中
    return 2;
  }
  // 読み取り完了

  if ( !AnalyzeLineString( szLineString, gps ) )
  {
    return 3;
  }

  //緯度経度が明らかにおかしい場合はじく
  if (LATITUDE_MINIMUM < (gps->latitude) && LATITUDE_MAXIMUM > (gps->latitude)) { //緯度の検査域にいるか
    if (  LONGITUDE_MINIMUM < (gps->longitude) && LONGITUDE_MAXIMUM > (gps->longitude)) { //経度の検査域にいるか
    } else {
      return 4;
    }
  } else {
    return 4;
  }
  // 緯度、経度を読み取れた。
  // float to string
  char sz_utc[16];
  char sz_lat[16];
  char sz_long[16];
  //小数点6けたで表示する
  dtostrf(gps->utc, 10, 6, sz_utc);
  dtostrf(gps->latitude, 10, 6, sz_lat);
  dtostrf(gps->longitude, 10, 6, sz_long);

  float LatA = point->latitude, LongA = point->longitude;      //目的地

  //  float LatA = 35.710039, LongA = 139.810726;      //目的地
  float LatB = gps->latitude;       //現在地の緯度経度
  float LongB = gps->longitude;
  float direct = 0, distance = 0;   //目的地までの距離方角
  //目的地への距離方角算出
  distance = sqrt(pow(LongA - LongB, 2) + pow(LatA - LatB, 2)) * 99096.44, 0;
  direct = (int)(atan2((LongA - LongB) * 1.23, (LatA - LatB)) * 57.3 + 360) % 360;

  //以下loop関数に値渡しする
  gps->Direction = direct;
  gps->distance = distance;

  return 1;
}

int gps_get_goal(GPS* gps, POINT* point) {
  xbee_uart(dev, "call gps_get\r");
  int t = 0;
  while (1) { //gpsの値が正常になるまで取り続ける
    int gps_flag = 0;   //gps_getの返り値保存
    gps_flag = gps_data_get_goal(gps, point);
    delay(10);
    t++;
    //gpsの値が取れない間どこで引っかかっているのか識別できるようになりました
    if (gps_flag == 1) { //値が取れたら抜ける
      xbprintf("LAT: %f, LONG: %f", gps->latitude, gps->longitude);
      xbprintf("DISTANCE: %f,DIRECTION: %f", gps->distance, gps->Direction);
      break;
    }
    if (gps_flag == 2) {
      ;

      //      xbee_uart( dev, "cant communicate with gps\r");

    }
    if (gps_flag == 3) {
      ;
      //gpsとの通信はできているが値が変or GPRMCでない

      //      xbee_uart( dev, "gps wrong or not GPRMC\r");

    }
    if (gps_flag == 4) {
      ;
      speaker(E_TONE);
      speaker(F_TONE);
      speaker(E_TONE);

      //通信ができて値も解析されたが緯度経度の値がバグってる
      //      xbee_uart( dev, "wrong Lat or Long\r");

    }
    if (t >= 10000) {
      //およそ100秒間取れなければ一旦抜ける
      break;
    }
  }
  return 1;
}

