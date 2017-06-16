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
  return 1;
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


      sprintf(xbee_send, "sample of %d is %f", i + 1, ac_array[i]);
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
  if (200 <= ac_ave && ac_ave <= 300) {
    xbee_uart( dev, "land ok\r");
    return 1; //着陸判定にパス
  } else {
    xbee_uart( dev, "land not ok\r");
    return 0;
  }
}





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
    if (landing_flag == 0) { //比較的風が弱い
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

  /*ここで反転判定及び復帰シーケンスを必ずできたら入れてください*/
  //ほぼ確実に反転しているので判定なしだとこの後のシーケンスが結構バグります
  judge_invered_revive(); /*←いれましたbyとうま */

  GPS gps;
  //gps_get(&gps);    //ここで取得したデータをSDなりに保管して以後近づかないようにしてください


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
    rover->My_Direction = get_my_direction();  //現在の方角を取得
    direction_hold = rover->My_Direction;      //現在の方角を保持

    xbee_uart( dev, "avoid parashute by opening casing.\r");
    xbee_uart( dev, "Pass this phase in this experiment.\r");

    //0.9~5mくらいなら取れる
    //servoモーターは90°が機体正面としています
    /*
       //サーボモーターで6０～１２０°まで安全確認
       for (angle_servo = 60; angle_servo <= 120; angle_servo += 10 ) {
         servo1.write(angle_servo);    //
         delay(1000);    //回転時間
         volt = analogRead( DISTANCE ) * 5 / 1023.0;
         Serial.println( volt );  //電圧換算表示

         if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
           para_distance = 140.0 / ( volt - 1.10 ) ;
           Serial.print( "success reading! Distance is  " );
           Serial.println( para_distance );
           distance_flag = 1;     //一方向でも危険物があるとパラシュートとみなしアウト
         }
       }*/


    /*
       サーボモーターなしver
    */

    target_flag = 0;
    int i = 0;

    //サーボモーターなしver
    for (i = -3; i <= 3; i++) {
      //うまく回転関数が動かないので一旦同じ位置で複数回と手検査します
      /*
        while (1) {
        rover->My_Direction = get_my_direction();
        target_flag = turn_target_direction((direction_hold + 10 * i), &rover->My_Direction);
        if (target_flag == 1) {
          break;
        } else {
          continue;
        }
        }*/
      volt = analogRead( DISTANCE ) * 5 / 1023.0;
      Serial.println( volt );  //電圧換算表示
      xbee_uart( dev, "volt of distance is " );  //電圧換算表示
      xbee_send_1double(volt);

      if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
        para_distance = 140.0 / ( volt - 1.10 ) ;
        Serial.print( "success reading! Distance is  " );
        Serial.println( para_distance );
        distance_flag = 1;     //一方向でも危険物があるとパラシュートとみなしアウト
      }
      delay(300);
    }


    delay( 500 );

    if (distance_flag == 1) {
      //前方にパラシュートが存在
      //回転する
      //turn_target_direction(rover->My_Direction + 90, &rover->My_Direction);
      //うまく回れないので一旦これで
      go_rotate(1000);
      /*本当は90°直角に回りたいけどいまそこら辺の制御どうなっているかわからないのでとりあえずこれで*/
    } else {
      //前方にパラシュートがない or 近すぎて判別できない
      rover->My_Direction = get_my_direction();  //現在方角を把握
      break;
    }
    //距離センサの以上orパラシュートがかぶさっているなどなんともしがたい状況になっている
    if (count_para >= 10) { //パラシュートがどっかいっちゃってるとかそんな感じ
      break;
    }
  }
  //方角をさっきの方角に戻す
  /*一旦コメントアウト
    rover->My_Direction = get_my_direction();
    target_flag = turn_target_direction((direction_hold ), &rover->My_Direction);
  */
  //先ほど取得した方向へ、しばらく進む

  /*本当は地磁気で角度を取得しながら正確に直進したいがそれが今どうなっているかわからないのでとうまに任せます*/
  /*キャリブレーションしていないので正確に直進はできないかも。禁止エリアに近づかないようにするのが関の山？*/
  go_straight(10000);


  //脱出成功
  return 1;  /*成功は１を返すように統一しようbyとうま*/
}
