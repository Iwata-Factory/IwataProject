
int status2(ROVER *rover) {  // Status2 打ち上げの関数

  if (_S2_ == 0) {
    xbee_uart( dev, "skip status2\r");
    delay(1000);
    return 1;
  }

  return (judge_release());
}

int judge_release() {
  int i = 0;
  int light = 0; //照度センサのカウンタ
  unsigned long t = 0; //一定時間たったら勝手に照度センサ起動
  //照度センサの値を5回連続でhighになるまで続ける
  /*判定周期とか厳しさは要実験*/
  while (1) {

    light = 0;

    for (i = 0; i < 5; i++) {
      light += digitalRead(LIGHT_PIN);
      xbee_uart( dev, "count\r");
      sprintf(xbee_send, "%d\r", i);
      xbee_uart( dev, xbee_send);
      write_control_sd("light count is " + String(i, DEC));
      delay(1000);
      t++;
    }

    if (light == 5) {
      break;
    }
    i = 0;
    light = 0;

    if ( t >= 300) { //仮に300秒たったら強制で投下されたと判断する
      write_control_sd("timeout");
      return 1;
    }

  }
  return 1;
}



