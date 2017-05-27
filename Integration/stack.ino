//スタック判定をする関数

/*前回とのgpsの値を比較して距離が変化しなければスタックしたと判断する*/
/*冗長化として地磁気とか加速度とか使うのものちのちいれるといいかも*/

int stack(double last_distance, double distance_hold){
  double difference_distance=0;



  //distanceの値がまだ取れてないときはエラー
  if(last_distance == -1  ||  distance_hold == -1){
    return 0;
  }
  
  difference_distance = fabs(last_distance - distance_hold);  //差の絶対値をとる
  //difference_distanceが一定より小さければスタックしたと判定する
  if (difference_distance <= 1){  //1m以内
    Serial.println("stack");
    //ここにスタックぬける関数おいてください
  } else {
    Serial.println("no stack");
  }


  

  return 0;
}

