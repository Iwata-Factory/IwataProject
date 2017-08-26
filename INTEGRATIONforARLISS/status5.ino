// ターゲットに近いところを目指す
int status5(ROVER *rover) {

  if (_S5_ == 0) {
    xbee_uart( dev, "skip status5\r");
    delay(1000);
    return 1;
  }

  int i = 0; // do-whileの繰り返し数をカウント
  double last_distance = -1;  // スタック判定用

  if (STACK_EXP == 1) {
    stack_check_state(rover);  // スタックのフラグを立てる
  }

  int do_stack_check = 0;  // スタック判定するかのフラグ

  GPS gps1;
  GPS gps2;
  gps_get(&gps1);

  float d_lat = 0;
  float d_lng = 0;
  float d_time = 0;
  
  
  do {

    //ローバー駆動
    delay(500);
    gps2 = gps1;
    gps_get(&gps1);

    d_lat = gps1.latitude - gps2.latitude;
    d_lng = gps1.longitude - gps2.longitude;
    d_time = gps1.utc - gps2.utc;
    Vector2D vct;
    Vector2D north_vct;
    vct.x = d_lat;
    vct.y = d_lng;
    north_vct.x = 0;
    north_vct.y = 1;
    double deg =  rad2deg(acos(vector2d_inner(vct, north_vct)));  // 0~π

    // 自己方位の確定
    if (0 <= vct.x){
      deg = deg;
    } else {
      deg = -deg;
    }
    double angle = get_angle_devision_gps(deg, gps1.Direction);  // ここで出てくるのが必要な回転角的なやつ

    // 出力調整部分
    

  } while (1);
}


double get_angle_devision_gps(double my_Direction, double target_direction) {


  double a_difference = my_Direction - target_direction;

  if (180 <= a_difference) {
    return (360 - a_difference);  // 右回転
  } else if (0 <= a_difference && a_difference < 180) {
    return (-a_difference);  // 左回転
  } else if (-180 <= a_difference && a_difference < 0) {
    return (-a_difference);  // 右回転
  } else {
    return (-1 * (360 + a_difference));  // 左回転
  }
}
