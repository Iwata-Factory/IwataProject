
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
  if (ave >= 0){
    return ave;
  } else {
    ave = ave + 360;
    return ave;
  }
  return ave;
}

Vector2D cordinate_ave(const int array_num, double* value) {
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

  return vector_ave;
}

//角度の外れ値を１ことってその平均をだす(ラジアン)
double degree_out(const int array_num, double* value) {
  int i = 0;
  int j = 0;
  double temp = 0;
  Vector2D vector[array_num];
  Vector2D vector_ave;
  double value_distance[array_num];
  double ave = 0;
  

  vector_ave = cordinate_ave(array_num, value);  //重心のxy座標を取得

  //角度を北から時計回りにとったxy座標上の点にする
  for (i = 0; i < array_num; i++) {
    vector[i].x = cos(value[i]);
    vector[i].y = sin(value[i]);
  }

 

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

