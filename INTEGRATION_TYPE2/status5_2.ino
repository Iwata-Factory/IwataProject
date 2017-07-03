// ターゲットに近いところを目指す


//とりあえず一周回りました

//・90度超えて止まるやつを切った


int status5_2(ROVER *rover) {

  if (_S5_ == 0) {
    xbee_uart( dev, "skip status5_2\r");
    delay(1000);
    return 1;
  }

  xbee_uart( dev, "status5_2\r");

  GPS gps;
  DRIVE pid;  // DRIVEの初期化

  if (STACK_EXP == 1) {
    stack_check_state(rover);  // スタックのフラグを立てる
  }


  tm_calibration();  // キャリブレーションの実施(とりあえず最初だけ)

  // double this_my_direction = 0.0;  // 今回の自分の方位
  double this_devision = 0.0;  // 今回の偏差
  double last_devison = 0.0;  // 前回の偏差
  double total_devision = 0.0;  // 偏差の累積値
  double control_amount = 0.0;  // 制御量
  double motor_control = 0.0;  // モーター制御量

  int i = 0; // do-whileの繰り返し数をカウント
  xbee_uart( dev, "(PID) START\r");

  accel();  // ローバースタート

  double last_distance = -1; //スタック判定用
  double fifty_counter = 0;
  double ninety_counter = 0;  // 角度カウンター

  do {

    if (arrange_between(i, &gps, rover, &last_distance) == 0) { // 状態のチェック
      continue;
    }


    do {
      rover->My_Direction = get_my_direction();
      this_devision = get_this_devision(last_devison, rover, i);  // 偏差取得（外れ値除去）(roverはadress)
      fifty_counter += 1;
      if (fifty_counter == 4) {
        fifty_counter = 0;
        break;
      }
    } while (this_devision == 500);


    // write_devision_sd(this_devision, 0);  // 偏差を記録（実験用）


    if (90 < fabs(this_devision) || i == 50) {  // 偏角が90度より大きくなった時かiが1200になったとき

      // ninety_counter += 1;
      ;

      if (ninety_counter == 10 || i == 50) {  // 10連かiが600で発動(ここのカウンタ要調整)

        brake();  // 止まる
        turn_target_direction(rover->Target_Direction, &rover->My_Direction, 0);  // ターゲット方向を向き直す
        accel();
        i = 0;  // iを初期化(0に戻す)
        last_distance = -1;  // last_distanceを初期化
        total_devision = 0;  // 偏差の累積値を初期化
        this_devision = 0;
        last_devison = 0;
        ninety_counter = 0;  // カウンターリセット
        control_amount = 0;
        motor_control = 0;
        continue;
      }

    } else {  // カウンターリセット
      ninety_counter = 0;
    }


    total2zero(&total_devision, i);

    control_amount = get_control(this_devision, total_devision);  // 制御量を取得

    get_motor_control(&pid, control_amount); // DRIVE pid の値を調整
    xbprintf("DRIVE %d, %d, %d, %d", pid.right1, pid.right2, pid.leght1, pid.leght2);
    rover_analog(pid);  // 出力に反映

    total_devision += this_devision;  // 偏差を足していく
    last_devison = this_devision;

    i += 1;  // カウントを足す

    delay(50);


  } while ((rover->distance < 0) || (12 < rover->distance)); // 12m以内に入ったらループを抜ける(status5-2後半へ)

  xbee_uart( dev, "(PID) END\r");

  brake();  // 止まる

  speaker(E_TONE);  // 音鳴らしておく
  speaker(F_TONE);
  speaker(G_TONE);
  speaker(E_TONE);
  speaker(F_TONE);

  xbee_uart( dev, "5_2 NORMAL START\r");

  // ここからはstatus5と同じシーケンスで
  int j = 0;
  do {

    // GPS情報を取得

    gps_get(&gps);
    // GPSが取得した値を自身のステータスに反映する。
    rover->latitude = gps.latitude;  // 緯度
    rover->longitude = gps.longitude;  //経度
    rover->Target_Direction = gps.Direction;  //ターゲットの方向
    rover->distance = gps.distance;  // ターゲットまでの距離

    if ((0 < rover->distance) && (rover->distance) < 3) {  // status6へ
      break;
    }

    write_gps_sd(gps);
    write_timelog_sd(rover);
    turn_target_direction(rover->Target_Direction, &rover->My_Direction, 0);
    go_straight(3000); // 3秒直進

    speaker(E_TONE);
    speaker(F_TONE);
    speaker(G_TONE);

    j += 1;

  } while (1);

  return 1;

}


// iが400で割れる時に色々やる
int arrange_between(int i, GPS *gps, ROVER *rover, double *last_distance) {

  if (i % 200 == 0) {  // delayないし200回くらいごとにGPS更新

    xbee_uart( dev, "call arrange_between_fifty\r");

    xbee_uart( dev, "(PID) GET GPS NEW\r");

    gps_get(gps);  // GPSを取る
    // GPSが取得した値をROVERのステータスに反映する。
    rover->latitude = gps->latitude;  // 緯度
    rover->longitude = gps->longitude;  //経度
    rover->Target_Direction = gps->Direction;  //ターゲットの方向
    rover->distance = gps->distance;  // ターゲットまでの距離
    write_gps_sd(*gps);  // 自身の位置をsdに記録
    write_timelog_sd(rover);

    if (i == 0) {
      *last_distance = rover->distance;  // 前回距離を生成
      return 1;
    } else {
      xbee_uart( dev, "check stack\r");
      if (fabs(rover->distance - *last_distance) < 3 && (0 < *last_distance)) {
        return stack_check_state(rover);
      } else {
        xbee_uart( dev, "no problem\r");
        *last_distance = rover->distance; // スタックで無かった時はlast_distanceを更新
        return 1;
      }
    }
  }

  return 1;
}


// 制御量を取得(正負どちらの値もありえる)
double get_control(double this_d, double total_d) {
  double amount_control = PID_KP * (this_d + PID_KI * total_d);
  return (amount_control);
}

// モーターの出力を弄る関数
int get_motor_control(DRIVE *pid_drive, double control_amount) {

  // 初期の量
  pid_drive->right1 = 0;
  pid_drive->right2 = 255;
  pid_drive->leght1 = 0;
  pid_drive->leght2 = 255;

  if (control_amount <= 0) {  // 左側の出力を弱くすれば良い
    pid_drive->leght2 -= fabs(control_amount);
    if (pid_drive->leght2 < 0) {
      pid_drive->leght2 = 0;
    }
  } else {  // 右側の出力を弱くすれば良い
    pid_drive->right2 -= fabs(control_amount);
    if (pid_drive->right2 < 0) {
      pid_drive->right2 = 0;
    }
  }
  return 1;
}

double get_this_devision(double last_devison, ROVER *rover, int i) {

  xbee_uart( dev, "call get_this_devision\r");


  double this_devision = get_angle_devision(rover->My_Direction, rover->Target_Direction);  // 自分から見た偏差を取得

  return this_devision;

  // ここコメントアウトしたら出来ました
  // if (i == 0) {
  //   return this_devision;
  // } else {
  //   if (50 < fabs(this_devision - last_devison)) {
  //   return 500;  // 失敗を返す(繰り返しを命令)
  //   } else {
  //   return this_devision;
  //   }
  // }
}


// 偏差が小さい時もしくは入力切り替え30回ごとに偏差の累積値を0にする関数
int total2zero(double *total_devision, int i) {
  if ((fabs(*total_devision) < 15) || i % 30 == 0) { // 方位が合ってきたら累積値を0にする
    xbee_uart( dev, "(PID) TOTAL ---> 0.0\r");
    *total_devision = 0.0;
    return 1;
  } else {
    return 0;
  }
}


// PIDでの偏差を記録していく
/*実験用の関数であとで消しますbyとうま*/
int write_devision_sd(double devision, int flag) {

  if (SD_LOG_FLAG == 0) { // スキップ
    return 0;
  }

  int i = 0; // 試行回数記録用
  while (i < 5) { // 5回SDカードを開けなかったら諦める
    File dataFile = SD.open("devlog.txt", FILE_WRITE);
    if (dataFile) { // ファイルが開けたときの処理
      dataFile.seek(dataFile.size());
      if (flag == 0) {
        dataFile.println(devision);
      } else {
        dataFile.println("start sample");
      }
      dataFile.close();
      return 1; // 成功を返す
    } else {
      i += 1;
    }
  }
  return 0; // 失敗を返す
}




