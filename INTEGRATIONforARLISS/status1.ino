int status1(ROVER *rover) {

  if (_S1_ == 0) {
    xbee_uart( dev, "skip status1\r");
    delay(1000);
    return 1;
  }
  write_control_sd(F("start standby"));
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。


  return 1;
}

