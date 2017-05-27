//スタックした後の脱出アルゴリズム
/*
 * とりあえず自分の状況を理解するためのやつです
 * 状況がわかったら、またそれに対して適切な処理をしやすくするためflag作っておきましたが、まだ使ってません
 */
int escape(double distance_hold){
  double my_direction[2] = {-1,-1};
  double dif_direction = 0;    // 角度変化
  GPS gps_stack;   //GPSの構造体
  double distance[2] = {-1,-1};
  double dif_distance = 0;

  int flag_direction = 0;
  int flag_distance = 0;
  int j = 0;    //試行回数


  while(1){
    //まずは自分がどういう状況下確認
    //自己位置が変化できるか
     while (!gps_get(&gps_stack)) { //gpsの値が正常になるまで取り続ける
            delay(50);
     }
  
    distance[0] = gps_stack.distance;
  
    //自分の方向が変えられるか
    while(my_direction[0]<0){
      my_direction[0] = get_my_direction();  //自分の向きを確認
    }
    //とりあえず交互に逆に回ってみる

    if (j%2 == 0){
      go_rotate(2000);
    } else {
      go_rotate(-2000);
    }
    while(my_direction[1]<0){
      my_direction[2] = get_my_direction();
    }
  
    dif_direction = fabs(my_direction[0]-my_direction[1]);
    
    if (dif_direction <= 10){//回転できてない
      //砂に埋まるとかしてどうにも身動き取れない状況
    } else {
      //とりあえず回れる
      flag_direction = 1;
    }
    //次にとりあえず走ってみる
    go_straight(4000);
    
    while (!gps_get(&gps_stack)) { //gpsの値が正常になるまで取り続ける
            delay(50);
     }
  
     distance[1] = gps_stack.distance;
  
     dif_distance = fabs(distance[1] - distance[0]);
  
     if(dif_distance <= 1){
      //スタックしたまま
     } else {
      //脱出成功
      flag_distance = 1;
      return 0;
     }
  }
   
  
}

