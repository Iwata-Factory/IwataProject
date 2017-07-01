/*
   着陸判定行われたのち
   ケーシング展開、ケーシングからの脱出シーケンス
*/


int status4(ROVER *rover) {  // Status4 着陸の関数

  if (_S4_ == 0) {
    xbprintf( "skip status4");
    delay(1000);
    return 1;
  }

  POINT landing_point;  //着陸座標
  GPS gps;  // GPS
  double lc_difference = 1.0;

  get_rover_point(&landing_point);
  cut_nicrom();  // ニクロム線を切る
  set_danger_area();  // 危険エリアを定義

  int posture_coefficient = set_posture_coefficient();  //  姿勢係数(正しい姿勢なら1、反転なら-1)を取得(反転を直すわけではないことに注意)

  int escape_counter = 0;
  xbprintf( "escape from landing-point");
  do {
    //本当は真っ直ぐ進みたい
    go_straight(10000);   //オフセットの式設定できたら、方向決めて直進できるようなやつに変えてください
    gps_get(&gps);
    lc_difference = distance_get(&gps, &landing_point);
    delay( 500 );
    escape_counter += 1;
  } while ((lc_difference < 10) && (escape_counter < 7));

  xbprintf( "escape complete");

  return 1;
}


/*-----------set_posture_coefficient()--------------------
  姿勢係数を1か-1で返す
  ------------------------------------------*/
int set_posture_coefficient() {
  xbprintf( "call set_posture_coefficient");
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

int cut_nicrom() {

  xbprintf("call cut_nicrom");
  //ニクロム線溶断する
  speaker(C_TONE);
  speaker(E_TONE);
  digitalWrite(NICROM_1, HIGH);
  digitalWrite(NICROM_2, HIGH);
  delay(10000);
  digitalWrite(NICROM_2, LOW);
  digitalWrite(NICROM_1, LOW);
  xbprintf("end cut_nicrom");
  speaker(G_TONE);
  return 1;
  
}
