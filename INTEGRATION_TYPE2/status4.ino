/*
   放出判定行われたのち
   無風状態でのケーシング展開を目指す。。？
*/


int status4(ROVER *rover) {  // Status4 着陸の関数

  // 加速度とGPSから判断することになりそう
  speaker(A_TONE);
  speaker(B_TONE);
  speaker(A_TONE);
  speaker(B_TONE);
  int landing_flag = 0;   //着地判定を何で行ったか

  //暫定的に前回までのやつにしています
  int st4_cnt = 0;  //時間経過x
  while (1) {
    if ((sensor & STATUS_AC) == STATUS_AC ) {   //AC生存
      //      xbee_uart(dev, "sampling ac...\r");

      if (determine_landing() == 1) {

        xbee_uart(dev, "AC ok\r");
        break;
      }
      //1ループ1分より適当に15分たったら強制的に着陸したものとする
      st4_cnt++;

      if (st4_cnt >= 15) {  //着陸後１５分経過

        break;
      }
    }
    else { //割と詰んでる状態
      xbee_uart(dev, "as know as zetsubou\r");//絶望
    }
    delay(10);
  }

  //以下パラシュートからの脱出

  casing(landing_flag, rover);

// 着陸地点の記録
  xbee_uart(dev, "finish falling\rlanded!!\rlogging landed point!\r");
  GPS gps_land;  //着陸地点を記録（必要？？）
  gps_get(&gps_land);
  if (write_gps_sd_file(gps_land, GPS_SPECIAL)) { // 自身の位置をsdに記録。新規に関数を作成しました。
    xbee_uart(dev, "success!!");
  } else {
    xbee_uart(dev, "fail...");
  }
  return 1;
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
  while (1) {
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


    if (nicrom_count >= 10) {
      //たぶんニクロム線がイカれているとかで異常事態
      break;
      /*将来の冗長性のために判定しておきます*/
    }

    //ケーシングが展開したかの確認シーケンス
    if (landing_flag == 0) { /*比較的風が弱い*/

      //ローバーを回転させ回転できる確認する
      rover->My_Direction = get_my_direction();
      direction_hold = rover->My_Direction;
      /*実験からおよそ180度回転になる値を入れてください*/
      speaker(B_TONE);
      go_rotate(1000);
      rover->My_Direction = get_my_direction();
      //回転した差分をとる
      dif_direction = fabs(rover->My_Direction - direction_hold);

      if (dif_direction <= 20) { //回転できていない
        target_flag = 0;
      } else {
        target_flag = 1;
      }

      if (target_flag == 1) {
        //無事に回転できた＞＞ケーシングが展開している

        break;
      } else {
        //ケーシングが展開していなくて回転できない
        nicrom_count++;
        continue;
      }
    } else {
      //landing_flag == 1の時は風が相当強いので何もいなくても空いていたらケーシングがどっかに行く
      //ローバーを回転させ回転できる確認する
      rover->My_Direction = get_my_direction();
      direction_hold = rover->My_Direction;
      /*実験からおよそ180度回転になる値を入れてください*/
      speaker(A_TONE);
      go_rotate(1000);
      rover->My_Direction = get_my_direction();
      //回転した差分をとる
      dif_direction = fabs(rover->My_Direction - direction_hold);

      if (dif_direction <= 20) { //回転できていない
        target_flag = 0;
      } else {
        target_flag = 1;
      }

      if (target_flag == 1) {
        //無事に回転できた＞＞ケーシングが展開している
        /*風が強いので後述の脱出シーケンスがひょっとしたらいらないかも*/
        break;
      } else {
        //ケーシングが展開していなくて回転できない
        nicrom_count++;
        continue;
      }
    }


  }
  //ここから、パラシュートをよけるプロセス

  /*反転復帰でパラシュートに絡まる恐れあり、反転のままこのシーケンスをやったほうがいいかも*/
  judge_invered_revive();

  //  GPS gps;
  //  gps_get(&gps);    //ここで取得したデータをSDなりに保管して以後近づかないようにしてください
  set_danger_area();


  //ひとまずケーシング展開できたとする
  //ここからは距離センサで目の前にパラシュートがないかを確認する

  //距離センサパラメーター
  double para_distance = 0;  //パラシュートまでの距離を測ります
  double volt = 0;
  int distance_flag = 0;

  double angle_servo = 0;  //servoモーターの角度
  int count_para = 0;     //何回パラシュートがあるかの判定をしたかのカウンター
  double direction_hold = 0;  //方角の保持

  while (1) {
    distance_flag = 0;
    count_para++;


    xbee_uart( dev, "avoid parashute by opening casing.\r");
    xbee_uart( dev, "Pass this phase in this experiment.\r");

    //0.9~5mくらいなら取れる
    //servoモーターは90°が機体正面としています

    /*
       サーボモーターなしver
    */

    target_flag = 0;
    int i = 0;

    //サーボモーターなしver
    for (i = -3; i <= 3; i++) {
      volt = analogRead( DISTANCE ) * 5 / 1023.0;
      Serial.println( volt );  //電圧換算表示
      xbee_uart( dev, "volt of distance is " );  //電圧換算表示
      xbee_send_1double(volt);

      if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
        para_distance = 140.0 / ( volt - 1.10 ) ;
        Serial.print( "success reading! Distance is  " );
        Serial.println( para_distance );
        distance_flag = 1;     //一回でも危険物があるとパラシュートとみなしアウト
      }
      delay(300);
    }


    delay( 500 );

    if (distance_flag == 1) {
      //前方にパラシュートが存在
      //回転する
      go_rotate(1000);

    } else {
      //前方にパラシュートがない or 近すぎて判別できない
      break;
    }
    //距離センサの以上orパラシュートがかぶさっているなどなんともしがたい状況になっている
    if (count_para >= 10) { //パラシュートがどっかいっちゃってるとかそんな感じ
      break;
    }
  }

  //先ほど取得した方向へ、しばらく進む
  go_straight(10000);


  //脱出成功
  return 1;
}
