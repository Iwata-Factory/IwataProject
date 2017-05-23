//データに統計処理を施す関数群

/*これってヘッダファイルではもう一度宣言しなくてもいいっけ？一応しときます*/
#include <math.h>

/*array_numのところには
 * const int　array_num = sizeof value /sizeof value[0];
 * ていうの入れて(配列の要素数)
 */

//角度計算用のベクトル
struct rad_vector{
  double x=0.0;
  double y=0.0;
};


/*
 * 受け取った配列を昇順に並び替える
 */
double sort(const int array_num, double* value ){
  
  int i=0;
  int j=0;
  double temp=0;   //値の一時保持

  //データの並び替え
  for (i=1; i<array_num; i++){
    for (j=0; j<array_num; j++){
      if (value[j]>value[j+1]){
        temp=value[j];
        value[j]=value[j+1];
        value[j+1]=temp;
      }
    }
  }
}
/*
 * 値の中央値をdoubleで返す
 */
double value_median(const int array_num, double* value){
  double median=0;

  sort(array_num, value);  //並び替え

  //中央値を出す
  if (array_num%2 == 1){   //データの数が奇数
    median = value[(array_num-1) / 2];
  } else {           //偶数
    median = (value[(array_num / 2) -1]+value[(array_num) /2 ]) / 2;
  }
  return median;
}

/*
 * 値の平均値を出す
 */
double value_ave(const int array_num, double* value){
  double ave=0;
  int i = 0;
   
  sort(array_num, value);    //並び替え

  //平均値を出す
  for (i=0; i < array_num; i++){
     ave += value[i]; 
  }
  ave = ave / array_num;

  return ave;
}

/*
 * 角度の平均値を出す
 */
double rad_ave(const int array_num, double* value){
  double ave=0;
  int i=0;

  struct rad_vector vector[array_num];
  struct rad_vector vector_sum;
  struct rad_vector vector_ave;

  //角度を北から時計回りにとったxy座標上の点にする
  for (i=0; i<array_num; i++){
    vector[i].x = cos(value[i]);
    vector[i].y = sin(value[i]);
  }


  //点の重心をとる(平均みたいになります)
  for (i=0; i<array_num; i++){
    vector_sum.x += vector[i].x;
    vector_sum.y += vector[i].y;
  }
  vector_ave.x = vector_sum.x / array_num;  
  vector_ave.y = vector_sum.y / array_num;

  //重心の角度を北から時計回りで出します
  ave = atan2(vector_ave.x, vector_ave.y);  //東から
  ave = ((int)ave) % 360;

  return ave;
}


//角度の外れ値を１ことってその平均をだす
double rad_out(const int array_num, double* value){
  int i=0;
  int j=0;  
  double temp=0;
  struct rad_vector vector[array_num];
  struct rad_vector vector_ave;
  double value_distance[5];
  double ave=0;
  
  
  ave = rad_ave(array_num, value);  //まずはすべての平均角度をとる

   //角度を北から時計回りにとったxy座標上の点にする
  for (i=0; i<array_num; i++){
    vector[i].x = cos(value[i]);
    vector[i].y = sin(value[i]);
  }

  vector_ave.x = cos(ave);
  vector_ave.y = sin(ave); 
  
  for (i=0; i<array_num; i++){
    value_distance[i] = sqrt(pow((vector[i].x - vector_ave.x), 2)+pow((vector[i].y - vector_ave.y), 2));
  }

  //距離を比較し一番長いのを外れ値として省く
  for (i=1; i<array_num; i++){
    if (value_distance[i] > value_distance[i-1]){
      //何もしない
    } else {  //平均ちからの距離を昇順でだし、一番大きい点を最後に来るように並び替える
      temp = value_distance[i];
      value_distance[i] = value_distance[i-1];
      value_distance[i-1] = temp; 

      temp = value[i];
      value[i] = value[i-1];
      value[i-1] = temp; 
  
     
    }   
  }

  ave = rad_ave(array_num-1, value);   //一番距離の大きい点を省いた平均を出す

  return ave;
}
