/*
   着陸判定行われたのち
   ケーシング展開、ケーシングからの脱出シーケンス
*/


int status4(ROVER *rover) {  // Status4 着陸の関数

  if (_S4_ == 0) {
    xbee_uart( dev, "skip status4\r");
    delay(1000);
    return 1;
  }

  write_critical_sd(1);  // 着陸終了
  POINT landing_point;  //着陸座標
  GPS gps;  // GPS
  double lc_difference = 1.0;
  get_rover_point(&landing_point);
  write_control_sd("landing point is (" + String(landing_point.latitude, DEC) + ", " + String(landing_point.longitude, DEC) + " )");
  cut_nicrom(10000);  // ニクロム線を切る
  set_danger_area();  // 危険エリアを定義

  int posture_coefficient = set_posture_coefficient();  //  姿勢係数(正しい姿勢なら1、反転なら-1)を取得(反転を直すわけではないことに注意)
  int escape_counter = 0;
  xbee_uart( dev, "escape from landing-point\r");
  write_control_sd(F("start to escape from landing-point"));
  do {
    //本当は真っ直ぐ進みたい
    go_straight(5000);   //直進
    gps_get(&gps);
    lc_difference = distance_get(&gps, &landing_point);
    write_control_sd("diffrence is " + String(lc_difference, DEC));
    delay(500);
    escape_counter += 1;

  } while ((lc_difference < 7) && (escape_counter < 7));
  if (escape_counter == 7) {
    write_control_sd(F("count out(7 times)"));
    cut_nicrom(10000);  // ニクロム線を切る
  } else {
    write_control_sd(F("get sufficient distance"));

  }
  xbee_uart( dev, "escape complete\r");

  return 1;
}


/*-----------set_posture_coefficient()--------------------
  姿勢係数を1か-1で返す
  ------------------------------------------*/
int set_posture_coefficient() {
  xbee_uart( dev, "call set_posture_coefficient\r");
  if (judge_invered() == 1) {
    return 1;
  } else {
    return -1;
  }
}


/*
   キャリブレーションしていないので地磁気を使う関数の使用原則禁止
   turn_target_direction禁止
*/

int cut_nicrom(int cut_time) {
  write_control_sd(F("try to cut nicrom"));
  xbee_uart(dev, "call cut_nicrom\n");
  //ニクロム線溶断する
  //speaker(C_TONE);
  //speaker(E_TONE);
  digitalWrite(NICROM_1, HIGH);
  digitalWrite(NICROM_2, HIGH);
  delay(cut_time);
  digitalWrite(NICROM_2, LOW);
  digitalWrite(NICROM_1, LOW);
  xbee_uart(dev, "end cut_nicrom\n");
  //speaker(G_TONE);
  return 1;

}
