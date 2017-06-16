
int status2(ROVER *rover) {  // Status2 打ち上げの関数
  return 1;
  return (judge_release());
}


int judge_release() {
  xbee_uart( dev, "judging housyutu\r");
  int i = 0;
  int light = 0; //照度センサのカウンタ
  unsigned long t = 0; //一定時間たったら勝手に照度センサ起動
  //照度センサの値を5回連続でhighになるまで続ける
  /*判定周期とか厳しさは要実験*/
  while (1) {

    for (i = 0; i < 5; i++) {
      light += digitalRead(LIGHT_PIN);
      xbee_uart( dev, "count");
      xbee_uart( dev, light);
      delay(1000);
      t++;
    }

    if (light = 5) {
      break;
    }
    i = 0;
    light = 0;

    if ( t >= 1000) { //仮に1000秒たったら強制で投下されたと判断する
      return 1;
    }

  }
  return 1;
}



