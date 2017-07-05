
// ローバーの移動に関する関数


/*-----------rover_degital(DRIVE)--------------------
  DRIVE型の引数
  degital write　の時
  ------------------------------------------*/
void rover_degital(DRIVE drive) {
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

  if (YOUR_MODEL == 1) { // FMではこのようになるみたいです。実験で発覚
    rotate = -1 * rotate;
  }

  if (0 < rotate) {  // 右
    turn.right1 = 0;
    turn.right2 = 1;
    turn.leght1 = 1;
    turn.leght2 = 0;

    rotate_time = (rotate / 400) * 1000;

  } else {  // 左
    turn.right1 = 1;
    turn.right2 = 0;
    turn.leght1 = 0;
    turn.leght2 = 1;

    rotate_time = -1 * (rotate / 400) * 1000;

  }

  // 回転を行う
  rover_degital(turn);

  if (YOUR_MODEL == 1) {  // モデルよる差
    rotate_time = rotate_time * 0.8;
    delay(rotate_time);
  } else {
    delay(rotate_time);
  }

  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;

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
  if (wait_time < 500) {  // 下限を設定
    wait_time = 500;
  }


  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;

  if (YOUR_MODEL == 0) { // EM
    for (int i = 1; i < 256; i++) {
      go.right1 = 0;
      go.right2 = i;
      go.leght1 = 0;
      go.leght2 = i;
      rover_analog(go);
      delay(2);
    }
    go.right1 = 0;
    go.right2 = 1;
    go.leght1 = 0;
    go.leght2 = 1;
    rover_degital(go);
    delay(wait_time);
    for (int i = 255; i > 0; i--) {
      go.right1 = 0;
      go.right2 = i;
      go.leght1 = 0;
      go.leght2 = i;
      rover_analog(go);
      delay(7);
    }
  } else if (YOUR_MODEL == 1) {  // FM(直進出来るように調整する)

    // 直進するように調整したパラメタ
    go.right1 = 0;
    go.right2 = 200;
    go.leght1 = 0;
    go.leght2 = 255;
    rover_analog(go);

    delay(wait_time);
    for (int i = 255; i - 30 > 0; i--) {
      go.right1 = 0;
      go.right2 = i - 30;
      go.leght1 = 0;
      go.leght2 = i;
      rover_analog(go);
      delay(7);
    }
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

}


void go_straight_control(int go_time, double target_direction) {
  DRIVE go; //DRIVE型の宣言
  // 初期化
  int wait_time = go_time - 1300;
  if (wait_time < 1100) {  // 下限を設定
    wait_time = 1100;
  }


  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;

  if (YOUR_MODEL == 0) { // EM
    for (int i = 1; i < 256; i++) {
      go.right1 = 0;
      go.right2 = i;
      go.leght1 = 0;
      go.leght2 = i;
      rover_analog(go);
      delay(2);
    }
    go.right1 = 0;
    go.right2 = 1;
    go.leght1 = 0;
    go.leght2 = 1;
    rover_degital(go);
    delay(wait_time);
    for (int i = 255; i > 0; i--) {
      go.right1 = 0;
      go.right2 = i;
      go.leght1 = 0;
      go.leght2 = i;
      rover_analog(go);
      delay(7);
    }
  } else if (YOUR_MODEL == 1) {  // FM(直進出来るように調整する)

    // 直進するように調整したパラメタ
    go.right1 = 0;
    go.right2 = 225;
    go.leght1 = 0;
    go.leght2 = 255;
    rover_analog(go);
    int get_control_counter = int(wait_time / PID_STREIGHT_BETWEEN);
    double my_direction = target_direction;
    double d_direction = 0;  // 差
    int ki_counter = 0;
    int ki = 0;

    for (int i = 0; i < get_control_counter + 1 ; i++) {

      go.right1 = 0;
      go.right2 = 200;
      go.leght1 = 0;
      go.leght2 = 255;

      delay(PID_STREIGHT_BETWEEN);

      my_direction = get_my_direction();  // 新しい角度
      d_direction = get_angle_devision(my_direction, target_direction);  // 偏差

      ki += d_direction;
      ki_counter += 1;
      if (ki_counter % 10 == 0) {
        xbee_uart(dev, "reset\r");
        ki = 0;
      }

      xbee_uart(dev, "integ:");
      xbprintf("%d", ki);

      if (0 <= d_direction) {  // 右方向によりたい（右を落とす）
        go.right2 -= (d_direction * PID_MAGNIFICATION + fabs(d_direction) * PID_INTEG);
        if (go.right2 < 80) {
          go.right2 = 80;
        } else if (255 < go.right2){
          go.right2 = 255;
        }
      } else {
        d_direction = -1 * d_direction;
        go.leght2 -= (d_direction * PID_MAGNIFICATION + fabs(d_direction) * PID_INTEG);
        if (go.leght2 < 80) {
          go.leght2 = 80;
        } else if (255 < go.leght2){
          go.leght2 = 255;
        }
      }

      xbee_uart(dev, "right:");
      xbprintf("%d", go.right2);
      xbee_uart(dev, "leght:");
      xbprintf("%d", go.leght2);

      rover_analog(go);
    }

    for (int i = 255; i - 30 > 0; i--) {
      go.right1 = 0;
      go.right2 = i - 30;
      go.leght1 = 0;
      go.leght2 = i;
      rover_analog(go);
      delay(7);
    }
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

}


void go_back(int go_time) {
  DRIVE go; //DRIVE型の宣言
  // 初期化

  if (go_time < 100) {  // waitタイムに下限を設定
    go_time = 100;
  }

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
  go.right1 = 180;
  go.right2 = 0;
  go.leght1 = 180;
  go.leght2 = 0;
  rover_analog(go);
  delay(go_time);
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

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(4);
  }
}

/*-----------急発進→ブレーキをかける--------------------
  ------------------------------------------*/
void go_suddenly_brake(int times) {
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
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);

}








