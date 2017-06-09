// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  do {

    GPS gps;
    while (1) { //gpsの値が正常になるまで取り続ける
      int gps_flag = 0;   //gps_getの返り値保存
      gps_flag = gps_get(&gps);
      delay(10);
      //gpsの値が取れない間どこで引っかかっているのか識別できるようになりました
      if (gps_flag == 1) { //値が取れたら抜ける
        break;
      }
      if (gps_flag == 2) {
        ;
        //gpsとの通信が来ていない
        //Serial.println("gpsとの通信できていない");
      }
      if (gps_flag == 3) {
        ;
        //gpsとの通信はできているが値が変or GPRMCでない
        //Serial.println("gpsの値がおかしい or GPRMCではない");
      }
      if (gps_flag == 4) {
        ;
        //通信ができて値も解析されたが緯度経度の値がバグってる
        //Serial.println("緯度経度がおかしい");
      }
    }

    // GPSが取得した値を自身のステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離

    if (rover->distance < 5){
      return 1;
    }

    // 目的の方向を目指して回転を行う。rover->My_Directionは書き換えていく。
    int turn_result = turn_target_direction(rover->Target_Direction, &rover->My_Direction);

    // 2秒直進
    go_straight(2000);

  } while (1);
}

