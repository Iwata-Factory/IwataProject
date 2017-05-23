
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
  if (0 < rotate) {
    turn.right1 = 0;
    turn.right2 = 1;
    turn.leght1 = 1;
    turn.leght2 = 0;
    rotate_time = (1000 * rotate) / 180;
  } else {
    turn.right1 = 1;
    turn.right2 = 0;
    turn.leght1 = 0;
    turn.leght2 = 1;
    rotate_time = -1 * (1000 * rotate) / 180;
  }

  // 回転を行う
  rover_degital(turn);

  delay(rotate_time);

  turn.right1 = 1;
  turn.right2 = 1;
  turn.leght1 = 1;
  turn.leght2 = 1;

  // 停止
  rover_degital(turn);
  delay(1500);
}




/*-----------機体直進--------------------
   go_time: 進む時間
  ------------------------------------------*/
void go_straight(int go_time) {
  DRIVE go; //DRIVE型の宣言
  // 初期化
  go.right1 = 0;
  go.right2 = 1;
  go.leght1 = 0;
  go.leght2 = 1;
  // 直進
  rover_degital(go);
  delay(go_time);

  // 停止
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  // 停止
  rover_degital(go);
  delay(1000);
}
