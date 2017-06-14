//（砂に埋まった）とかのスタックした後の脱出アルゴリズム
/*
   とりあえず自分の状況を理解するためのやつです
   状況がわかったら、またそれに対して適切な処理をしやすくするためflag作っておきましたが、まだ使ってません
*/
int escape(double distance_hold) {
  double my_direction[2] = { -1, -1};
  double dif_direction = 0;    // 角度変化
  struct GPS gps_stack;   //GPSの構造体
  double distance[2] = { -1, -1};
  double dif_distance = 0;

  int flag_direction = 0;
  int flag_distance = 0;
  int j = 0;    //試行回数


  while (1) {
    //まずは自分がどういう状況下確認
    //自己位置が変化できるか
    while (1) { //gpsの値が正常になるまで取り続ける
      int gps_flag = 0;   //gps_getの返り値保存
      gps_flag = gps_get(&gps_stack);
      delay(10);
      //gpsの値が取れない間どこで引っかかっているのか識別できるようになりました
      if (gps_flag == 1) { //値が取れたら抜ける
        break;
      }
      if (gps_flag == 2) {
        ;
        //gpsとの通信が来ていない
        //Serial.println("gpsとの通信できていない");
      }
      if (gps_flag == 3) {
        ;
        //gpsとの通信はできているが値が変or GPRMCでない
        //Serial.println("gpsの値がおかしい or GPRMCではない");
      }
      if (gps_flag == 4) {
        ;
        //通信ができて値も解析されたが緯度経度の値がバグってる
        //Serial.println("緯度経度がおかしい");
      }
    }

    distance[0] = gps_stack.distance;

    //自分の方向が変えられるか
    while (my_direction[0] < 0) {
      my_direction[0] = get_my_direction();  //自分の向きを確認
    }
    //とりあえず交互に逆に回ってみる

    if (j % 2 == 0) {
      go_rotate(2000);
    } else {
      go_rotate(-2000);
    }
    while (my_direction[1] < 0) {
      my_direction[1] = get_my_direction();
    }

    dif_direction = fabs(my_direction[0] - my_direction[1]);

    if (dif_direction <= 10) { //回転できてない
      //砂に埋まるとかしてどうにも身動き取れない状況
    } else {
      //とりあえず回れる
      flag_direction = 1;
    }
    //次にとりあえず走ってみる
    go_straight(4000);

    gps_get(&gps_stack);

    distance[1] = gps_stack.distance;

    dif_distance = fabs(distance[1] - distance[0]);

    if (dif_distance <= 1) {
      //スタックしたまま
      
    } else {
      //脱出成功
      flag_distance = 1;
      return 0;
    }
  }

  if ((flag_distance == 0) && (flag_direction == 1)){
    //回転等はできるが進めない
    //たぶん轍
    wadachi();
    return 0;
  }
}

/*
   轍に沿って移動はできるけど轍から逃げられない
*/
int wadachi() {
  struct GPS gps;
  double distance_hold = 0;
  double diff_distance = 1000;
  int wadachi_count = 0;

  distance_hold = gps.distance;   //distance保持
  //基本的に下がっては少し右旋回して直進してまた引っかかったら右旋回とやっていき轍を回避できる場所まで行く
  go_back(2000);
  go_rotate(1000);     /*ここは本当は右に90度みたいに絶対的に角度変化がわかるようにしたいけど、一旦1000msとしておきました*/
  go_straight(5000);
  go_rotate(1000);

  
  gps_get(&gps);

  distance_hold = gps.distance;   //distance保持
  go_straight(5000);

  gps_get(&gps);

  diff_distance = fabs(distance_hold - gps.distance);    //轍回避行動後に

  if (diff_distance <= 2) {
    //轍に引っかかったままなのでもう一度同じ動き
  } else {
    //轍の引っ掛かりの回避に成功
    return 0;
  }

  diff_distance = fabs(distance_hold - gps.distance);

  if (diff_distance <= 3) {
    //轍から回避できてない、もう一回同じことをする
    wadachi_count++;
  } else {
    //轍の乗り越えに成功
    return 0;
  }

}

