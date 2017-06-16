
int status4(ROVER *rover) {  // Status4 着陸の関数
  return 1;


  // 加速度とGPSから判断することになりそう

  //暫定的に前回までのやつにしています
  int t = 0;  //時間経過
  while (1) {
    if (determine_landing() == 1) {
      break;
    }
    //1ループ1分より適当に15分たったら強制的に着陸したものとする
    t++;

    if (t >= 15) {
      break;
    }
  }

  //以下パラシュートからの脱出
  //casing();

  return 1;
}

// 着陸判定関数を書くファイル

/*-----------determine_landing()--------------------
  着陸したか判定を行う
  着陸と判定したら1、そうでなければ0を返す。
  ------------------------------------------*/

int determine_landing() {
  xbee_uart( dev,"judging Landing\r");


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
//      xbee_uart( dev,"サンプル");
//      xbee_uart( dev,i + 1);
//      xbee_uart( dev,":");
//      xbee_uart( dev,ac_array[i]);

        sprintf(xbee_send, "sample of %d is %f", i+1, ac_array[i]);
        xbee_uart(dev, xbee_send);
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
//  xbee_uart( dev,"解析結果:");
//  xbee_uart( dev,ac_ave);
  if (225 <= ac_ave && ac_ave <= 245) {
    xbee_uart( dev,"land ok\r");
    return 1; //着陸判定にパス
  } else {
    xbee_uart( dev,"land not ok\r");
    return 0;
  }
}

//ケーシング展開関数
int casing() {

  //ニクロム線溶断する
  //  digitalWrite(NICROM_1, HIGH);
  //  delay(10000);   //10秒間ニクロム線を熱すれば切れるはず
  //  digitalWrite(NICROM_1, LOW);

  AC ac_case;  // 反転判定用
  
  while (1) {
    
    if (ac_case.z < 0) {  // 加速度が負の場合
      speaker(C_TONE);  // 音を鳴らす
      speaker(C_TONE);
      speaker(C_TONE);
      delay(1000);
      go_back(5000);
    } else {
      delay(1000);
      break;
    }
  }

  //ここから、パラシュートをよけるプロセス

  GPS gps;
  gps_get(&gps);    //ここで取得したデータをSDなりに保管して以後近づかないようにしてください

  //とりあえずケーシング展開できていないと困るので動いて刺激与える
  /*本当はケーシングが開いたかの判定を何らかのセンサを用いてしたいけど難しそう*/
  /*いい方法思いついたら教えてください*/
  go_rotate(1000);
  go_rotate(-1000);

  //ひとまずケーシング展開できたとする
  //ここからは距離センサで目の前にパラシュートがないかを確認する

  //距離センサパラメーター
  double para_distance = 0;  //パラシュートまでの距離を測ります
  double volt = 0;
  int distance_flag = 0;
  double my_direction = 0;
  double angle_servo = 0;  //servoモーターの角度
  int count_para = 0;     //何回パラシュートがあるかの判定をしたかのカウンター

  while (1) {
    xbee_uart( dev,"avoid parashute by opening casing.\r");
    xbee_uart( dev,"Pass this phase in this experiment.\r");
    return 1;
    count_para++;
    my_direction = get_my_direction();  //現在の方角を取得
    volt = analogRead( DISTANCE ) * 5 / 1023.0;

    xbee_uart( dev, "volt of distance is " );  //電圧換算表示
    xbee_send_1double(volt);

    //0.9~5mくらいなら取れる
    //servoモーターは90°が機体正面としています

    //サーボモーターで6０～１２０°まで安全確認
    for (angle_servo = 60; angle_servo <= 130; angle_servo += 10 ) {
      servo1.write(angle_servo);    //
      delay(1000);    //回転時間
      if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
        para_distance = 140.0 / ( volt - 1.10 ) ;
        xbee_uart( dev, "success reading! Distance is  \r" );
        xbee_send_1double( para_distance );
        distance_flag = 1;     //一方向でも危険物があるとパラシュートとみなしアウト
      }
    }
    delay( 500 );

    if (distance_flag == 1) {
      //前方にパラシュートが存在
      //回転する
      go_rotate(1000);
      /*本当は90°直角に回りたいけどいまそこら辺の制御どうなっているかわからないのでとりあえずこれで*/
    } else {
      //前方にパラシュートがない or 近すぎて判別できない
      my_direction = get_my_direction();  //現在方角を把握
      break;
    }
    //距離センサの以上orパラシュートがかぶさっているなどなんともしがたい状況になっている
    if (count_para >= 10) { //その時のいい方法も思い浮かばないので一旦運げで走らせることにした
      break;
    }
  }

  //先ほど取得した方向へ、しばらく進む
  /*本当は地磁気で角度を取得しながら正確に直進したいがそれが今どうなっているかわからないのでとうまに任せます*/
  go_straight(10000);


  //脱出成功
  return 0;

}

