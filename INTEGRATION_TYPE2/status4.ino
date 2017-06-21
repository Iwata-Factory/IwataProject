/*
   放出判定行われたのち
   着地判定とケーシング展開、ケーシングからの脱出シーケンス
*/


int status4(ROVER *rover) {  // Status4 着陸の関数

  if (_S4_ == 0) {
    xbee_uart( dev, "skip status4\r");
    delay(1000);
    return 1;
  }

  int landing_flag = get_landing_flag();   //着地判定を何で行ったか(0:加速語　1:時間)

  POINT landing_point;  //着陸座標
  GPS gps;  // GPS
  double lc_difference = -1.0;

  get_rover_point(&landing_point);


  cut_nicrom();  // ニクロム線を切る
  set_danger_area();  // 危険エリアを定義

  int posture_coefficient = set_posture_coefficient();  //  姿勢係数(正しい姿勢なら1、反転なら-1)を取得

  int escape_counter = 0;
  do {
    //本当は真っ直ぐ進みたい
    go_straight(10000);   //オフセットの式設定できたら、方向決めて直進できるようなやつに変えてください
    lc_difference = get_distance(&gps, &landing_point);
    delay( 500 );
    escape_counter += 1;
  } while ((0 < lc_difference && lc_difference < 10) || escape_counter == 5);

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

/*-----------get_landing_flag()--------------------
  landing_flagを取得
*/
int get_landing_flag() {
  //暫定的に前回までのやつにしています
  int t = 0;  //時間経過
  while (1) {
    if (determine_landing() == 1) {
      return 0;
    }
    t++;
    if (t >= 15) {
      return 1;
    }
  }
}

/*-----------determine_landing()--------------------
  着陸したか判定を行う
  着陸と判定したら1、そうでなければ0を返す。
  ------------------------------------------*/

int determine_landing() {

  xbee_uart( dev, "call determine_landing\r");

  AC ac; // 宣言

  double ac_array[10]; // サンプルを入れる箱

  // 加速度の和と平均
  double ac_sum = 0;
  double ac_ave = 0;

  // 加速度のサンプルを10個取る
  int i = 0;

  //xbee_uart( dev,"加速度のサンプルを取得します");

  while (i < 10) {
    ac = get_ac(); // 加速度を取得
    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
      // 値を取れている
      // 加速度の大きさを計算
      ac_array[i] = sqrt(pow(ac.x, 2) + pow(ac.y, 2) + pow(ac.z, 2));
      delay(3000); // サンプリングは3秒ごとに
      i += 1;
    } else {
      // 加速度を取得に失敗したら3秒待ってもう一度取る
      delay(3000);
    }
  }

  // 着陸したかの判定
  for (int i = 0; i < 10; ++i) {
    ac_sum += ac_array[i];
  }
  ac_ave = ac_sum / 10;

  if (200 <= ac_ave && ac_ave <= 300) {
    xbee_uart( dev, "success determine_landing\r");
    return 1; //着陸判定にパス
  } else {
    xbee_uart( dev, "false determine_landing\r");
    return 0;
  }
}


/*
   キャリブレーションしていないので地磁気を使う関数の使用原則禁止
   turn_target_direction禁止
*/

int cut_nicrom() {
  xbee_uart(dev, "call cut_nicrom\n");

  //ニクロム線溶断する
  speaker(C_TONE);
  speaker(E_TONE);
  digitalWrite(NICROM_1, HIGH);
  digitalWrite(NICROM_2, HIGH);
  delay(1000);
  digitalWrite(NICROM_2, LOW);
  digitalWrite(NICROM_1, LOW);
  xbee_uart(dev, "end cut_nicrom\n");
  speaker(G_TONE);
  return 1;
}
