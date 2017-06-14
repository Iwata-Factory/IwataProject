// 着陸判定関数を書くファイル

/*-----------determine_landing()--------------------
  着陸したか判定を行う
  着陸と判定したら1、そうでなければ0を返す。
  ------------------------------------------*/

int determine_landing() {

  AC ac; // 宣言

  double ac_array[10]; // サンプルを入れる箱
  
  // 加速度の和と平均
  double ac_sum = 0;
  double ac_ave = 0;

  // 加速度のサンプルを10個取る
  int i = 0;
  while (i < 10) {
    ac = get_ac(); // 加速度を取得
    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)){
      // 値を取れている
      // 加速度の大きさを計算
      ac_array[i] = sqrt(pow(ac.x, 2) + pow(ac.y, 2) + pow(ac.z, 2));
      delay(6000); // サンプリングは6秒ごとに
      i += 1;
    } else {
      // 加速度を取得に失敗したら6秒待ってもう一度取る
      delay(6000);
    }
  }
  // 着陸したかの判定
  // 平均を算出
  for (int i = 0; i < 10; ++i) {
    ac_sum += ac_array[i];
  }
  ac_ave = ac_sum / 10;
  if (9.5 <= ac_ave && ac_ave <= 10.3) {
    return 1; //着陸判定にパス
  } else {
    return 0;
  }
}

