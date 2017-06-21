/*
   放出判定行われたのち
   着地判定とケーシング展開、ケーシングからの脱出シーケンス
*/


int status4(ROVER *rover) {  // Status4 着陸の関数

  // 加速度とGPSから判断することになりそう
  speaker(A_TONE);
  int landing_flag = 0;   //着地判定を何で行ったか

  //暫定的に前回までのやつにしています
  int t = 0;  //時間経過
  while (1) {
    if (determine_landing() == 1) {

      landing_flag = 0;

      break;
    }
    //1ループ1分より適当に15分たったら強制的に着陸したものとする
    t++;

    if (t >= 15) {


      landing_flag = 1;

      break;
    }
  }

  //以下パラシュートからの脱出

  casing(landing_flag, rover);


  return 1;
}

// 着陸判定関数を書くファイル

/*-----------determine_landing()--------------------
  着陸したか判定を行う
  着陸と判定したら1、そうでなければ0を返す。
  ------------------------------------------*/

int determine_landing() {

  xbee_uart( dev, "judging Landing\r");


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


      //      sprintf(xbee_send, "sample of %d is ", i + 1);  //ac_array bug
      //      xbee_uart(dev, xbee_send);
      //      xbee_send_1double(ac_array[i]);  //ここをコメントアウトしないと再起動する（震え）
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

  //  xbee_uart( dev,"解析結果:");
  //  xbee_uart( dev,ac_ave);
  if (200 <= ac_ave && ac_ave <= 300) {
    xbee_uart( dev, "land ok\r");
    return 1; //着陸判定にパス
  } else {
    xbee_uart( dev, "land not ok\r");
    return 0;
  }
}


/*
   キャリブレーションしていないので地磁気を使う関数の使用原則禁止
   turn_target_direction禁止
*/


//ケーシング展開関数
int casing(int landing_flag, ROVER * rover) {

  int target_flag = 0;
  int nicrom_count = 0;

  //ニクロム線溶断する

  double direction_hold = 0;   //方角保持
  double dif_direction = 0;   //directionの差をとる
  speaker(C_TONE);
  speaker(E_TONE);
  xbee_uart(dev, "nicrom hotten\n");
  digitalWrite(NICROM_1, HIGH);
  digitalWrite(NICROM_2, HIGH);
  delay(1000);
  digitalWrite(NICROM_2, LOW);
  digitalWrite(NICROM_1, LOW);
  xbee_uart(dev, "nicrom end\n");
  speaker(G_TONE);


  //ここから、パラシュートをよけるプロセス

  /*反転復帰でパラシュートに絡まる恐れあり、反転のままこのシーケンスをやったほうがいいかも*/
  judge_invered_revive();

  //  GPS gps;
  //  gps_get(&gps);    //ここで取得したデータをSDなりに保管して以後近づかないようにしてください
  set_danger_area();


  go_straight(10000);   //オフセットの式設定できたら、方向決めて直進できるようなやつに変えてください


  delay( 500 );
}

//先ほど取得した方向へ、しばらく進む
go_straight(10000);


//脱出成功
return 1;
}
