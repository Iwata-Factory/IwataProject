
/*
   受け取った配列を昇順に並び替える
*/

double descending_sort(const int array_num, double* value ) {
  int i = 0;
  int j = 0;
  double temp = 0; //値の一時保持
  //データの並び替え
  for (i = 0; i < array_num; i++) {
    for (j = 0; j < (array_num - 1); j++) {
      if (value[j] > value[j + 1]) {
        temp = value[j];
        value[j] = value[j + 1];
        value[j + 1] = temp;
      }
    }
  }
}

/*
 * 値の最大値を返す
 */
double value_max(const int array_num, double* value){
  double maximum = 0;   //最大値

  descending_sort(array_num, value);  //並び替え
  return value[array_num - 1];   //最大値を返す
}


/*
   値の中央値をdoubleで返す
   array_num:要素数
*/
double value_median(const int array_num, double* value) {
  double median = 0;

  descending_sort(array_num, value);  //並び替え

  //中央値を出す
  if (array_num % 2 == 1) { //データの数が奇数
    median = value[(array_num - 1) / 2];
  } else {           //偶数
    median = (value[(array_num / 2) - 1] + value[(array_num) / 2 ]) / 2;
  }
  return median;
}



/*
   値の平均値を出す
*/
double value_ave(const int array_num, double* value) {

  descending_sort(array_num, value);    //並び替え

  int i = 0;
  double ave = 0;

  //平均値を出す
  for (i = 0; i < array_num; i++) {
    ave += value[i];
  }
  ave = ave / array_num;

  return ave;
}



/*
   角度の平均値を出す
   引数　deg
   戻り値 deg
*/
double degree_out(const int array_num, double* value) {
  double ave = 0;
  int i = 0;

  Vector2D vector[array_num];
  Vector2D vector_sum;
  Vector2D vector_ave;

  //角度を北から時計回りにとったxy座標上の点にする
  for (i = 0; i < array_num; i++) {
    vector[i].x = cos(deg2rad(value[i]));
    vector[i].y = sin(deg2rad(value[i]));
  }

  //点の重心をとる(平均みたいになります)
  for (i = 0; i < array_num; i++) {
    vector_sum.x += vector[i].x;
    vector_sum.y += vector[i].y;
  }
  vector_ave.x = vector_sum.x / array_num;
  vector_ave.y = vector_sum.y / array_num;

  //重心の角度を北から時計回りで出します
  ave = atan2(vector_ave.y, vector_ave.x);  //東から
  /*これってはatan2だからはじめからradだからrad2degで逆だよね？*/

  ave = rad2deg(ave);
  ave = (((int)ave + 360) % 360);
  return ave;
}




/*
   角度の平均値を出す
   引数　deg
   戻り値 deg
*/
double rad_ave(const int array_num, double* value) {
  double ave = 0;
  int i = 0;

  Vector2D vector[array_num];
  Vector2D vector_sum;
  Vector2D vector_ave;

  //角度を北から時計回りにとったxy座標上の点にする
  for (i = 0; i < array_num; i++) {
    vector[i].x = cos(deg2rad(value[i]));
    vector[i].y = sin(deg2rad(value[i]));
  }

  //点の重心をとる(平均みたいになります)
  for (i = 0; i < array_num; i++) {
    vector_sum.x += vector[i].x;
    vector_sum.y += vector[i].y;
  }
  vector_ave.x = vector_sum.x / array_num;
  vector_ave.y = vector_sum.y / array_num;

  //重心の角度を北から時計回りで出します
  ave = atan2(vector_ave.y, vector_ave.x);  //東から
  /*これってはatan2だからはじめからradだからrad2degで逆だよね？*/

  ave = rad2deg(ave);
  ave = (((int)ave + 360) % 360);
  return ave;
}

//角度の外れ値を１ことってその平均をだす(ラジアン)
/*
   引数 deg
   戻り値　deg
*/
double rad_out(const int array_num, double* value) {
  int i = 0;
  int j = 0;
  double temp = 0;
  Vector2D vector[array_num];
  Vector2D vector_ave;
  double value_distance[5];
  double ave = 0;


  ave = degree_out(array_num, value);  //まずはすべての平均角度をとる

  //角度を北から時計回りにとったxy座標上の点にする
  for (i = 0; i < array_num; i++) {
    vector[i].x = cos(value[i]);
    vector[i].y = sin(value[i]);
  }

  vector_ave.x = cos(ave);
  vector_ave.y = sin(ave);

  for (i = 0; i < array_num; i++) {
    value_distance[i] = sqrt(pow((vector[i].x - vector_ave.x), 2) + pow((vector[i].y - vector_ave.y), 2));
  }

  //距離を比較し一番長いのを外れ値として省く
  for (i = 1; i < array_num; i++) {
    if (value_distance[i] > value_distance[i - 1]) {
      //何もしない
    } else {  //平均ちからの距離を昇順でだし、一番大きい点を最後に来るように並び替える
      temp = value_distance[i];
      value_distance[i] = value_distance[i - 1];
      value_distance[i - 1] = temp;

      temp = value[i];
      value[i] = value[i - 1];
      value[i - 1] = temp;


    }
  }

  ave = rad_ave(array_num - 1, value); //一番距離の大きい点を省いた平均を出す

  return ave;
}



// 単位換算関数を書く関数


/*-----------radからdegに--------------------
  引数はrad(double型)
  ------------------------------------------*/
double rad2deg(double rad) {
  double deg = rad * 180.0 / PI;
  return (deg);
}

/*-----------degからradに--------------------
  引数はdeg(double型)
  ------------------------------------------*/
double deg2rad(double deg) {
  double rad = deg * PI / 180.0;
  return (rad);
}



/*-----------Vector2Dのサイズを得る--------------------
  ------------------------------------------*/
double vector2d_size(Vector2D v) {
  double v_size = sqrt(pow(v.x, 2) + pow(v.y, 2));
  return v_size;
}

/*----------- Vector2Dの内積を得る--------------------
  ------------------------------------------*/
double vector2d_inner(Vector2D v1, Vector2D v2) {
  double inner_product = v1.x * v2.x + v1.y * v2.y;
  return inner_product;
}

// get_distanceとget_distance_by_sphereの呼び出し元
double distance_get(GPS* gps, POINT* point) {
  xbee_uart(dev, "call distance_get\r");
  double d = 0;
  if (SPHERE_FLAG == 0) {
    d = get_distance(gps, point);
  } else {
    d = get_distance_by_sphere(gps, point);
  }
  char sz_d[16];
  dtostrf(d, 10, 6, sz_d);
  xbee_uart(dev, "between_distance:");
  xbee_uart(dev, sz_d);
  xbee_uart(dev, "\r");
  return d;
}


/*
   緯度経度から指定した地点までの距離を測定する
*/
double get_distance(GPS* gps, POINT* point) {
  double distance = 0;
  distance = sqrt(pow(point->longitude - gps->longitude, 2) + pow(point->latitude - gps->latitude, 2)) * 99096.44, 0;
  return distance;
}

/*
   緯度経度から指定した地点までの距離を測定する（hyubeniの公式を使用）
*/
double get_distance_by_sphere(GPS* gps, POINT* point) {

  // WGS84 (GPS)より
  double A = 6378137.000;  // 長半径
  double B = 6356752.314245;  // 短半径
  double E2 = 0.00669437999019758;  // 第一離心率のべき乗

  double d_lat = deg2rad(point->latitude - gps->latitude) ;  // 二地点の緯度の差
  double d_lng = deg2rad(point->longitude - gps->longitude);  // 経度の差
  double ave_lat = deg2rad((point->latitude + gps->latitude) / 2); // 緯度の平均 (ラジアンで)
  // 公式を適用
  double w = sqrt(1 - E2 * pow((sin(ave_lat)), 2));
  double m = (A * (1 - E2)) / pow(w, 3);
  double n = A / w;
  double distance = sqrt(pow((d_lat * m), 2) + pow(d_lng * n * cos(ave_lat), 2));

  return distance;
}


// get_directionとget_direction_by_sphereの呼び出し元
double direction_get(GPS* gps, POINT* point) {
  xbee_uart(dev, "call direction_get\r");
  double dire = 0;
  if (SPHERE_FLAG == 0) {
    dire = get_direction(gps, point);
  } else {
    dire = get_direction_by_sphere(gps, point);
  }
  char sz_dire[16];
  dtostrf(dire, 10, 6, sz_dire);
  xbee_uart(dev, "between_direction:");
  xbee_uart(dev, sz_dire);
  xbee_uart(dev, "\r");
  return dire;
}

/*
   緯度経度から指定した地点までの方角を測定する
*/
double get_direction(GPS* gps, POINT* point) {
  double direct = 0;
  direct = (int)(atan2((point->longitude - gps->longitude) * 1.23, (point->latitude - gps->latitude)) * 57.3 + 360) % 360;

  return direct;
}

/*
   緯度経度から指定した地点までの方角を測定する(球面三角法)
*/
double get_direction_by_sphere(GPS* gps, POINT* point) {

  double y = cos(deg2rad(point->latitude)) * sin(deg2rad(point->longitude - gps->longitude));
  double x1 = cos(deg2rad(gps->latitude)) * sin(deg2rad(point->latitude));
  double x2 = sin(deg2rad(gps->latitude)) * cos(deg2rad(point->latitude)) * cos(deg2rad(point->longitude - gps->longitude));
  double x = x1 - x2;
  double direct = rad2deg(atan2(y, x));  // 逆正接を計算

  //0~360に直す
  if (direct < 0) {
    direct = direct + 360.0;
  } else {
    ;
  }
  return direct;
}


