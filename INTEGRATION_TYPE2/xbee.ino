int xbee_timer() {            //xbeeで現在時刻を送信ただ、millisだと秒で最大通信量に到達？？（未検証）
  int xbtim = millis();
  char timer[sizeof(time)];
  sprintf(timer, "timer: %d\r", time);
  xbee_uart( dev, timer); //millisでは即上限に到達すると思われる。（未検証）
  return xbtim;
}

//６月１日時点でのテストはまだ。
//integrationにてinclude,グローバル,setupを確認のこと。

void xbee_send_3doubles(double first, double second, double third) {
  char send_double[48];
  char xbee_first[16];  //get_gpsと同じ定義をするのは二度手間では。。？
  char xbee_second[16];
  char xbee_third[16];

  //小数点6けたで表示する
  dtostrf(first, 10, 6, xbee_first);
  dtostrf(second, 10, 6, xbee_second);
  dtostrf(third, 10, 6, xbee_third);

  sprintf( send_double, "%s,%s,%s\r", xbee_first, xbee_second, xbee_third );
  xbee_uart( dev, send_double );  //送信
  delay(10);
}

void xbee_send_2doubles(double first, double second) {
  char send_double[32];
  char xbee_first[16];  //get_gpsと同じ定義をするのは二度手間では。。？
  char xbee_second[16];

  //小数点6けたで表示する
  dtostrf(first, 10, 6, xbee_first);
  dtostrf(second, 10, 6, xbee_second);

  sprintf( send_double, "%s,%s\r", xbee_first, xbee_second );
  xbee_uart( dev, send_double );  //送信
  delay(1000);
}

void xbee_send_1double(double first) {
  char send_double[16];  //get_gpsと同じ定義をするのは二度手間では。。？

  //小数点6けたで表示する
  dtostrf(first, 10, 6, send_double);

  sprintf( send_double, "%s\r", send_double);
  xbee_uart( dev, send_double );  //送信
  delay(10);
}

