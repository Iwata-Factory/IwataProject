// 統計的処理を書く

/*
   受け取った配列を昇順に並び替える
*/

double descending_sort(const int array_num, double* value ) {
  int i = 0;
  int j = 0;
  
  double temp = 0; //値の一時保持

  //データの並び替え
  for (i = 1; i < array_num; i++) {
    for (j = 0; j < array_num; j++) {
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

  double ave = 0;
  int i = 0;

  descending_sort(array_num, value);    //並び替え

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

//  struct rad_vector vector[array_num];
//  struct rad_vector vector_sum;
//  struct rad_vector vector_ave;
  
  Vector2D vector[array_num];
  Vector2D vector_sum;
  Vector2D vector_ave;

  //角度を北から時計回りにとったxy座標上の点にする
  for (i = 0; i < array_num; i++) {
    vector[i].x = cos(rad2deg(value[i]));
    vector[i].y = sin(rad2deg(value[i]));
  }

  //点の重心をとる(平均みたいになります)
  for (i = 0; i < array_num; i++) {
    vector_sum.x += vector[i].x;
    vector_sum.y += vector[i].y;
  }
  vector_ave.x = vector_sum.x / array_num;
  vector_ave.y = vector_sum.y / array_num;

  //重心の角度を北から時計回りで出します
  ave = atan2(vector_ave.x, vector_ave.y);  //東から
  ave = deg2rad(((int)ave) % 360);

  return ave;
}




