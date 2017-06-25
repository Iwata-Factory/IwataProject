// ターゲットに近いところを目指す
int status5_2(ROVER *rover) {

  if (_S5_ == 0) {
    xbee_uart( dev, "skip status5_2\r");
    delay(1000);
    return 1;
  }

  xbee_uart( dev, "status5_2\r");

  GPS gps;
  DRIVE pid;  // DRIVEの初期化

  tm_calibration();  // キャリブレーションの実施

  double this_my_direction = 0.0;  // 今回の自分の方位
  double this_devision = 0.0;  // 偏差
  double total_devision = 0.0;  // 偏差の累積値
  double control_amount = 0.0;  // 制御量
  double motor_control = 0.0;  // モーター制御量

  int i = 0; // do-whileの繰り返し数をカウント
  xbee_uart( dev, "(PID) START\r");
  accel();  // ローバースタート

  // 実験用に区切り文字を書き込む
  write_devision_sd(0.0, 1);

  do {

    if (i % 50 == 0) {  // delayないし50回くらいごとにGPS更新

      xbee_uart( dev, "(PID) GET GPS NEW\r");

      gps_get(&gps);  // GPSを取る
      // GPSが取得した値をROVERのステータスに反映する。
      rover->latitude = gps.latitude;  // 緯度
      rover->longitude = gps.longitude;  //経度
      rover->Target_Direction = gps.Direction;  //ターゲットの方向
      rover->distance = gps.distance;  // ターゲットまでの距離
      write_gps_sd(gps);  // 自身の位置をsdに記録

    }

    write_timelog_sd(rover);

    this_my_direction = get_my_direction();
    this_devision = get_angle_devision(this_my_direction, rover->Target_Direction);  // 自分から見た偏差を取得

    write_devision_sd(this_devision, 0);  // 偏差を記録（実験用）

    total2zero(&total_devision);

    control_amount = get_control(this_devision, total_devision);  // 制御量を取得

    get_motor_control(&pid, control_amount); // DRIVE pid の値を調整
    rover_analog(pid);  // 出力に反映

    total_devision += this_devision;  // 偏差を足していく

  } while (10 < rover->distance); // とりあえず今はこれで

  xbee_uart( dev, "(PID) END\r");

  brake();  // 止まる

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
  pid_drive->right1 = 255;
  pid_drive->right2 = 0;
  pid_drive->leght1 = 255;
  pid_drive->leght2 = 0;

  if (control_amount <= 0) {  // 左側の出力を弱くすれば良い
    pid_drive->leght1 -= fabs(control_amount);
    if (pid_drive->leght1 < 0) {
      pid_drive->leght1 = 0;
    }
  } else {  // 右側の出力を弱くすれば良い
    pid_drive->right1 -= fabs(control_amount);
    if (pid_drive->right1 < 0) {
      pid_drive->right1 = 0;
    }
  }
  return 1;
}


// 偏差が小さい時は偏差の累積値を0にする関数
int total2zero(double *this_d) {
  if (fabs(*this_d) < 15) { // 方位が合ってきたら累積値を0にする
    xbee_uart( dev, "(PID) TOTAL ---> 0.0\r");
    *this_d = 0.0;
    return 1;
  } else {
    return 0;
  }
}



// PIDでの偏差を記録していく
/*実験用の関数であとで消しますbyとうま*/
int write_devision_sd(double devision, int flag) {
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
