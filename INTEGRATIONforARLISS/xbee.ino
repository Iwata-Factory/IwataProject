int xbee_timer() {            //xbeeで現在時刻を送信ただ、millisだと秒で最大通信量に到達？？（未検証）
  if (XBEE_SWITCH == 0) {
    return 1;
  }
  int xbtim = millis();
  char timer[sizeof(time)];
  //sprintf(timer, "timer: %d\r", time);
  xbee_uart( dev, timer); //millisでは即上限に到達すると思われる。（未検証）
  return xbtim;
}

//６月１日時点でのテストはまだ。
//integrationにてinclude,グローバル,setupを確認のこと。

int xbee_send_3doubles(double first, double second, double third) {
  if (XBEE_SWITCH == 0) {
    return 0;
  }
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
  delay(2);
  return 1;
}

int xbee_send_2doubles(double first, double second) {
  if (XBEE_SWITCH == 0) {
    return 0;
  }
  char send_double[32];
  char xbee_first[16];  //get_gpsと同じ定義をするのは二度手間では。。？
  char xbee_second[16];

  //小数点6けたで表示する
  dtostrf(first, 10, 6, xbee_first);
  dtostrf(second, 10, 6, xbee_second);

  sprintf( send_double, "%s,%s\r", xbee_first, xbee_second );
  xbee_uart( dev, send_double );  //送信
  delay(2);
  return 1;
}

int xbee_send_1double(double first) {

  if (XBEE_SWITCH == 0) {
    return 0;
  }
  char send_double[16];  //get_gpsと同じ定義をするのは二度手間では。。？

  //小数点6けたで表示する
  dtostrf(first, 10, 6, send_double);

  sprintf( send_double, "%s\r", send_double);
  xbee_uart( dev, send_double );  //送信
  delay(2);
  return 1;

}


void xbprintf(char *fmt, ...) {
  char xb_buf[XBEE_BUFFERSIZE];
  va_list args;
  va_start (args, fmt);
  vsnprintf(xb_buf, XBEE_BUFFERSIZE, fmt, args);
  va_end (args);
  xbee_uart(dev, xb_buf);
  xbee_uart(dev, "\r");
}

int xbee_standby() {
  xbee_uart( dev, "waiting for your command...\r");
  int xb_rxcnt = 0;

  while (1) {  // コマンド受信待機

    AC ac;  // 振動試験用

    ac = get_ac();  // 振動試験用
    write_ac_sd(ac);  // 振動試験用
    delay(100);

    if (xbee_rcv(ENTER)) {
      xbprintf("are you ready?");

      while (1) {
        if (xbee_rcv( O_CAP )) {
          xbprintf("your command finally accepted!");
          return 1;
        }
        xbee_uart(dev, " ");  // これを回さないとxbeeが動かない。。。
      }
    }

    //    xb_rxcnt++;
    xbee_uart(dev, " ");  // これを回さないとxbeeが動かない。。。
    //    if (xb_rxcnt > 100000 ) {  //timeout時間約8000秒
    //      xbprintf("can't receive command...");
    //      break;
    //    }
  }
  return 0;
}

int xbee_rcv(byte character) { // ここの引数は実際のxbeeで送られる16進数をdefineで定義してあります。
  XBEE_RESULT xbee_result;
  xbee_rx_call(&xbee_result);
  if (xbee_result.MODE == MODE_UART) {
    if (xbee_result.DATA[0] == character) {
      return 1;
    }
  }
  else {
    return 0;
  }
}

