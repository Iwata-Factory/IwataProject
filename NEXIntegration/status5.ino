// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  do {
    // gpsのデータを取る。
    // double Target_Direction = -1.0;  // ターゲットの方角
    // double distance = -1.0;  //ターゲットまでの距離
    // の値を得る
    //
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
    rover->latitude = gps.latitude;
    rover->longitude = gps.longitude;
    rover->Target_Direction = gps.Direction;
    rover->distance = gps.distance;

    // My_directionとTarget_Directionを揃える

    int turn_result = turn_target_direction(rover->Target_Direction, &rover->My_Direction);


    
   
  } while (0 <= rover->distance && rover->distance < 5);
}

