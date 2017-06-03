// 自身の方向取得関数を書くファイル

/*-----------get_my_direction()--------------------
  自分の向いている方向
  成功したらその値を
  失敗したら負の値を返却
  ------------------------------------------*/

double get_my_direction() {

  double my_direction = -1; //自分の方向(負の値で初期化)

  // 加速度を取得
  AC ac;
  while (1) {
    static int i = 0; // 試行回数をカウント(冗長化に使う)
    ac = get_ac();
    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
      break;
    } else {
      i += 1;
      if (i == 100) {
        return 0;
      }
    }

    Serial.print("ac.x");
    Serial.println(ac.x);

    Serial.print("ac.y");
    Serial.println(ac.y);

    Serial.print("ac.z");
    Serial.println(ac.z);

    // 地磁気を取得
    TM tm;
    while (1) {
      static int i = 0; // 試行回数をカウント
      tm = get_tm();
      if (!(tm.x == 100 && tm.y == 100 && tm.z == 100)) {
        break;
      } else {
        i += 1;
        if (i == 100) {
          return 0;
        }
      }

      Serial.print("tm.x");
      Serial.println(tm.x);

      Serial.print("tm.y");
      Serial.println(tm.y);

      Serial.print("tm.z");
      Serial.println(tm.z);

      // 方角を算出
      double roll = 0; //ロール角
      double pitch = 0; //ピッチ角
      double o_x = 0; //オフセット量
      double o_y = 0;
      double o_z = 0;
      // 基底の変換
      roll = atan2(ac.y, ac.z);
      pitch = atan2(ac.x * (-1), ac.y * sin(roll) + ac.z * cos(roll));
      double numerator = (tm.z - o_z) * sin(roll) - (tm.y - o_y) * cos(roll); //分子
      double denominator = (tm.x - o_x) * cos(pitch) + (tm.y - o_y) * sin(pitch) * sin(roll) + (tm.z - o_x) * sin(pitch) * cos(roll); // 分母
      my_direction = atan2(numerator, denominator) * RAD_TO_DEG + 180; //xy平面での向きを角度(°)で表示(北で0°)
      return my_direction;
    }
  }
}
