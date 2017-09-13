
// ローバーの移動に関する関数


/*-----------rover_degital(DRIVE)--------------------
  DRIVE型の引数
  degital write　の時
  ------------------------------------------*/
void rover_degital(DRIVE drive) {
  //  write_control_sd("input degital motor(" + String(drive.right1) + " " + String(drive.right2) + " " + String(drive.leght1) + " " + String(drive.leght2) + " " ")");
  digitalWrite(M1_1, drive.right1);
  digitalWrite(M1_2, drive.right2);
  digitalWrite(M2_1, drive.leght1);
  digitalWrite(M2_2, drive.leght2);
}
/*-----------rover_analog(DRIVE)--------------------
  DRIVE型の引数
  analog write　の時
  ------------------------------------------*/
void rover_analog(DRIVE drive) {
  //  write_control_sd("input anarog motor(" + String(drive.right1) + " " + String(drive.right2) + " " + String(drive.leght1) + " " + String(drive.leght2) + " " ")");
  analogWrite(M1_1, drive.right1);
  analogWrite(M1_2, drive.right2);
  analogWrite(M2_1, drive.leght1);
  analogWrite(M2_2, drive.leght2);
}


/*-----------機体旋回--------------------
   rotate: 回転角
  ------------------------------------------*/
void go_rotate(double rotate) {

  DRIVE turn; // DRIVE型の宣言
  // 初期化
  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;

  int rotate_time = 0; // 旋回する時間 /* これを求めるアルゴリズムの用意は必要 */

  // どちら向きに回転するかを判断

  if (MACHINE == 1) { // FMではこのようになるみたいです。実験で発覚
    rotate = -1 * rotate;
  } else if (MACHINE == 2) {
    rotate = -1 * rotate;
  }

  if (0 < rotate) {  // 右
    turn.right1 = 0;
    turn.right2 = 1;
    turn.leght1 = 1;
    turn.leght2 = 0;
    write_control_sd("input degital motor(" + String(turn.right1) + " " + String(turn.right2) + " " + String(turn.leght1) + " " + String(turn.leght2) + " " ")");

    rotate_time = (rotate / 400) * 1000;

  } else {  // 左
    turn.right1 = 1;
    turn.right2 = 0;
    turn.leght1 = 0;
    turn.leght2 = 1;
    write_control_sd("input degital motor(" + String(turn.right1) + " " + String(turn.right2) + " " + String(turn.leght1) + " " + String(turn.leght2) + " " ")");

    rotate_time = -1 * (rotate / 400) * 1000;

  }

  // 回転を行う
  rover_degital(turn);

  if (MACHINE == 1) {  // モデルよる差
    rotate_time = rotate_time * 0.8;
    delay(rotate_time);
  } else if (MACHINE == 2) {
    rotate_time = rotate_time * 0.8;
    delay(rotate_time);
  }

  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;
  write_control_sd("input degital motor(" + String(turn.right1) + " " + String(turn.right2) + " " + String(turn.leght1) + " " + String(turn.leght2) + " " ")");

  // 停止
  rover_degital(turn);
  delay(300);
}


/*-----------機体直進--------------------
   go_time: 進む時間
  ------------------------------------------*/

void go_straight(int go_time) {
  DRIVE go; //DRIVE型の宣言
  // 初期化
  int wait_time = go_time - 1300;
  if (wait_time < 100) {  // 下限を設定
    wait_time = 100;
  }

  write_control_sd(F("motor(0, 1, 0, 1) to motor(0, 255, 0, 255) (increase one by one for every additinal 7 millisecond)"));
  write_control_sd("motor(0, 255, 0, 255) time is " + String(go_time, DEC) + " milliseconds");
  write_control_sd(F("motor(0, 255, 0, 255) to motor(255, 0, 255, 0) (decrease one by one for every additinal 7 millisecond)"));

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;

  // 直進するように調整したパラメタ
  go.right1 = 0;
  go.right2 = PI_RIGHT_DEFAULT;
  go.leght1 = 0;
  go.leght2 = PI_LEGHT_DEFAULT;
  rover_analog(go);

  delay(wait_time);
  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(7);
  }


  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

}

// pi制御による直進
// go_time:実際の時間と一致しないので注意
int go_straight_control(int go_time, double target_direction) {

  int a = 0;

  write_control_sd("go_straight_control (argument is" + String(go_time, DEC)  + ")");


  DRIVE go; //DRIVE型の宣言
  // 初期化
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;

  write_control_sd("input degital motor(" + String(go.right1) + " " + String(go.right2) + " " + String(go.leght1) + " " + String(go.leght2) + " " ")");

  // 直進するように調整したパラメタ
  go.right1 = 0;
  go.right2 = PI_RIGHT_DEFAULT;
  go.leght1 = 0;
  go.leght2 = PI_LEGHT_DEFAULT;
  rover_analog(go);

  write_control_sd("input analog motor(" + String(go.right1) + " " + String(go.right2) + " " + String(go.leght1) + " " + String(go.leght2) + " " ")");

  int get_control_counter = int(go_time / PID_STREIGHT_BETWEEN);  // サイクル数を決定
  double my_direction = target_direction;  // 自身の方向
  double d_direction = 0;  // 偏差
  int integral_counter = 0;  // 積分のカウント数
  double integral = 0;  // 積分の足し合わせ
  int error_count = 0;

  POINT point;
  double dis;

  for (int i = 0; i < get_control_counter + 1 ; i++) {

    GPS gps;

    if (i == 0) {  // 初期位置を生成
      gps_get(&gps);
      point.latitude = gps.latitude;
      point.longitude = gps.longitude;
    }

    a += 1;

    if ((5 < a) && (a % 5 == 0)) {
      gps_get(&gps);
      dis = distance_get(&gps, &point);

      if ((0.0 < dis) && (dis < 1.0)) {
        return 0;
      } else {
        point.latitude = gps.latitude;
        point.longitude = gps.longitude;
      }
    }


    go.right1 = 0;
    go.right2 = PI_RIGHT_DEFAULT;
    go.leght1 = 0;
    go.leght2 = PI_LEGHT_DEFAULT;
    delay(PID_STREIGHT_BETWEEN);

    my_direction = get_my_direction();  // 新しい角度
    d_direction = get_angle_devision(my_direction, target_direction);  // 偏差を求める

    if (90 < fabs(d_direction)) {  // 角度がバグっている時は止まる
      error_count += 1;
      if (error_count == 2) {
        xbee_uart(dev, "false pi control\r");
        brake();
        return;
      }
    } else {
      error_count = 0;
    }

    integral += d_direction;  // 積分の増加、リセット等を行う
    integral_counter += 1;
    integral = integral * integral_riset(integral_counter);

    xbee_uart(dev, "integ:");  // double型だからバグる？
    dtostrf(integral, 10, 6, xbee_send);
    xbprintf(xbee_send);

    go = get_drive_input(go, d_direction, integral);   // 入力を得る

    xbee_uart(dev, "right:");
    xbprintf("%d", go.right2);
    xbee_uart(dev, "leght:");
    xbprintf("%d", go.leght2);

    rover_analog(go);  // 入力の反映

    write_control_sd("input analog motor(" + String(go.right1) + " " + String(go.right2) + " " + String(go.leght1) + " " + String(go.leght2) + " " ")");
  }
  brake();

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

  write_control_sd("input analog motor(" + String(go.right1) + " " + String(go.right2) + " " + String(go.leght1) + " " + String(go.leght2) + " " ")");

  return 1;
}


int integral_riset(int counter) {
  if (counter % PI_INTEGRAL_RISET == 0) {
    xbee_uart(dev, "reset\r");
    return 0;
  } else {
    return 1;
  }
}

//DRIVE get_drive_input(DRIVE drive, double d, double i) {
//
//  if (0 <= d) {  // 右方向によりたい（右を落とす）
//
//    drive.right2 -= fabs(d * PI_KP);  /* ここのfabs(d)ってfabs(i)ですねbyとうま */
//
//
//
//    if (i > 0) {
//      drive.right2 -= fabs(i * PI_KI);
//    } else {
//      drive.leght2 -= fabs(i * PI_KI);
//    }
//
//    if (drive.right2 < PI_MIN) {
//      drive.right2 = PI_MIN;
//    } else if (PI_MAX < drive.right2) {
//      drive.right2 = PI_MAX;
//    }
//    if (drive.leght2 < PI_MIN) {
//      drive.leght2 = PI_MIN;
//    } else if (PI_MAX < drive.leght2) {
//      drive.leght2 = PI_MAX;
//    }
//
//  } else {
//
//    d = -1 * d;
//    i = -1 * i;
//
//    drive.leght2 -= fabs(d * PI_KP);
//
//
//    if (i > 0) {
//      drive.right2 -= fabs(i * PI_KI);
//    } else {
//      drive.leght2 -= fabs(i * PI_KI);
//    }
//
//    if (drive.right2 < PI_MIN) {
//      drive.right2 = PI_MIN;
//    } else if (PI_MAX < drive.right2) {
//      drive.right2 = PI_MAX;
//    }
//
//    if (drive.leght2 < PI_MIN) {
//      drive.leght2 = PI_MIN;
//    } else if (PI_MAX < drive.leght2) {
//      drive.leght2 = PI_MAX;
//    }
//  }
//  return drive;
//
//}

DRIVE get_drive_input(DRIVE drive, double d, double i) {

  if (0 >= d) {  // 右方向によりたい（右を落とす）

    drive.leght2 -= fabs(d * PI_KP);  /* ここのfabs(d)ってfabs(i)ですねbyとうま */



    if (i > 0) {
      drive.right2 -= fabs(i * PI_KI);
    } else {
      drive.leght2 -= fabs(i * PI_KI);
    }

    if (drive.right2 < PI_MIN) {
      drive.right2 = PI_MIN;
    } else if (PI_MAX < drive.right2) {
      drive.right2 = PI_MAX;
    }
    if (drive.leght2 < PI_MIN) {
      drive.leght2 = PI_MIN;
    } else if (PI_MAX < drive.leght2) {
      drive.leght2 = PI_MAX;
    }

  } else {

    d = -1 * d;
    i = -1 * i;

    drive.right2 -= fabs(d * PI_KP);


    if (i < 0) {
      drive.right2 -= fabs(i * PI_KI);
    } else {
      drive.leght2 -= fabs(i * PI_KI);
    }

    if (drive.right2 < PI_MIN) {
      drive.right2 = PI_MIN;
    } else if (PI_MAX < drive.right2) {
      drive.right2 = PI_MAX;
    }

    if (drive.leght2 < PI_MIN) {
      drive.leght2 = PI_MIN;
    } else if (PI_MAX < drive.leght2) {
      drive.leght2 = PI_MAX;
    }
  }
  return drive;

}

void go_back(int go_time) {
  DRIVE go; //DRIVE型の宣言
  // 初期化

  if (go_time < 100) {  // waitタイムに下限を設定
    go_time = 100;
  }

  write_control_sd(F("motor(1, 1, 1, 1) to motor(100, 0, 100, 0) (increase one by one for every additinal 8 millisecond)"));

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  for (int i = 1; i < 100; i++) {  // ゆっくりと下がる
    go.right1 = i;
    go.right2 = 0;
    go.leght1 = i;
    go.leght2 = 0;
    rover_analog(go);
    delay(8);
  }

  write_control_sd("motor(180, 0, 180, 0) time is " + String(go_time, DEC) + " milliseconds");

  go.right1 = 180;
  go.right2 = 0;
  go.leght1 = 180;
  go.leght2 = 0;
  rover_analog(go);
  delay(go_time);
  write_control_sd(F("motor(1, 1, 1, 1)"));

  go.right1 = 1;  // バック時は急停止
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
}


/*-----------加速を行う--------------------
  ------------------------------------------*/

void accel() {
  DRIVE go; //DRIVE型の宣言
  // 初期化
  go.right1 = 0;
  go.right2 = 0;
  go.leght1 = 0;
  go.leght2 = 0;
  for (int i = 1; i < 256; i++) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(10);
  }
}

/*-----------ブレーキをかける--------------------
  ------------------------------------------*/
void brake() {

  DRIVE go; //DRIVE型の宣言
  // 初期化
  go.right1 = 0;
  go.right2 = 0;
  go.leght1 = 0;
  go.leght2 = 0;

  for (int i = 230; i > 0; i--) {  // 減速（回転しないように調整）
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(7);
  }

  // 明示的な停止

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

  //  for (int i = 255; i > 0; i--) {
  //    go.right1 = 0;
  //    go.right2 = i;
  //    go.leght1 = 0;
  //    go.leght2 = i;
  //    rover_analog(go);
  //    delay(4);
  //  }
}

/*-----------急発進→ブレーキをかける--------------------
  ------------------------------------------*/
void go_suddenly_brake(int times) {
  write_control_sd(F("motor(0, 1, 0, 1) to motor(0, 255, 0, 255) (increase one by one for every additinal 4 millisecond)"));
  write_control_sd("motor(0, 255, 0, 255) time is " + String(times, DEC) + " milliseconds");
  DRIVE go; //DRIVE型の宣言
  // 初期化
  go.right1 = 0;
  go.right2 = 1;
  go.leght1 = 0;
  go.leght2 = 1;
  rover_degital(go);
  delay(times);
  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(4);
  }
  write_control_sd(F("motor(0, 255, 0, 255) ---> motor(1, 1, 1, 1)"));
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

}








