//defineについてはいntegrationを参照のこと。
//距離センサからの電圧をデータシートの関数に基づいて距離に変換しています。
//測定範囲外の値についての扱いは適当です。。

double get_distance() {
  float distance;       //cmで距離を表示
  float volt;            //電圧表示
  double dis_data[DIS_SAMPLE];  //サンプル格納
  int cnt_dis = 0;  //ただのカウンタ
  
  Serial.println("sampling start");
  while ( cnt_dis < DIS_SAMPLE ) {
    volt = analogRead( DISTANCE ) * 5 / 1023.0;
    dis_data[cnt_dis] = volt;
    Serial.println( volt );  //電圧換算表示
    cnt_dis++;
    delay(10);
  }

/*
 * 以下、データ処理を行います。
 * データの精度としては１〜1.5mで１０cm、1.5〜2.5mで３０cm
 * 2.5m以上で５０cm程度の誤差が生じます。
 * 存在あるなしの判断は３m程度かなという感じです。
 * 
 * 精度を上げるには実験して経験式を得たほうがよさそう。
 */
  volt = value_ave( DIS_SAMPLE, dis_data );
  Serial.print("average is  ");
  Serial.println(volt);
  if ( 1.40  < volt & volt < 2.7 ) {            //有効測距範囲内
    distance = 140.0 / ( volt - 1.10 ) ;
    Serial.print( "success reading! Distance is  " );
    Serial.println( distance );
    return distance;
  }
  else{
    return 5.00;  //このreturnは適当です。
  }
}
