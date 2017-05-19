//データに統計処理を施す関数群

/*array_numのところには
 * const int　array_num = sizeof value /sizeof value[0];
 * ていうの入れて(配列の要素数)
 */


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
    med = value[(array_num-1) / 2];
  } else {           //偶数
    med = (value[(array_num / 2) -1]+value[(array_num) /2 ]) / 2;
  }
  return med;
}

/*
 * 値の平均値を出す
 */
double value_ave(const int array_num, double* value){
  double ave=0;
  int i = 0;
   
  sort(array_num, value);    //並び替え

  //平均値を出す
  for (i=0; i < array_num, i++){
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
  
}

