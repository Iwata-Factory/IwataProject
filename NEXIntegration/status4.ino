/*
 * status4をコンパイルするようです
 * 後で消してください
 */
int status4() {  // Status4 着陸の関数
  // 加速度とGPSから判断することになりそう

  //暫定的に前回までのやつにしています
  int t = 0;  //時間経過
  while (1) {
    if (determine_landing() == 1) {
      break;
    }
    //1週1分より適当に15分たったら強制的に着陸したものとする
    t++;

    if (t >= 15) {
      break;
    }
  }

  //以下パラシュートからの脱出
  casing();

  return 1;
}

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
    if (!(ac.x == 100 && ac.y == 100 && ac.z == 100)) {
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

//ケーシング展開関数
int casing() {
  //ニクロム線溶断する
  digitalWrite(NICROM_1, HIGH);
  delay(10000);   //10秒間ニクロム線を熱すれば切れるはず
  digitalWrite(NICROM_1, LOW);

  //ここから、パラシュートをよけるプロセス
  struct GPS gps;
  gps_get(&gps);    //ここで取得したデータをSDなりに保管して以後近づかないようにしてください

  //とりあえずケーシング展開できていないと困るので動いて刺激与える
  /*本当はケーシングが開いたかの判定を何らかのセンサを用いてしたいけど難しそう*/
  go_rotate(1000);
  go_rotate(-1000);

  //ひとまずケーシング展開できたとする
  //ここからは距離センサで目の前にパラシュートがないかを確認する

  //距離センサパラメーター
  double para_distance = 0;  //パラシュートまでの距離を測ります
  double volt = 0;
  int distance_flag = 0;
  double my_direction = 0;
  while (1) {
    volt = analogRead( DISTANCE ) * 5 / 1023.0;
    Serial.println( volt );  //電圧換算表示

    //0.9~5mくらいなら取れる
    if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
      para_distance = 140.0 / ( volt - 1.10 ) ;
      Serial.print( "success reading! Distance is  " );
      Serial.println( para_distance );
      distance_flag = 1;
    }
    delay( 500 );

    if (distance_flag == 1) {
      //前方にパラシュートが存在
      //回転する
      go_rotate(1000);

    } else {
      //前方にパラシュートがない or 近すぎて判別できない
      my_direction = get_my_direction();  //現在位置を把握
      break;
    }
  }

  //先ほど取得した方向へ、しばらく進む
  /*本当は地磁気で角度を取得しながら正確に奏功したいがそれが今どうなっているかわからないのでとうまに任せます*/
  go_straight(10000);


  //脱出成功
  return 0;

}




