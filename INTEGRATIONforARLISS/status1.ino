int status1(ROVER *rover) {

  if (_S1_ == 0) {
    xbee_uart( dev, "skip status1\r");
    delay(1000);
    return 1;
  }


  if (ARLISS_TEST == 1) {


    xbee_uart( dev, "test start\r");
    write_control_sd(F("test start"));
    delay(1000);
    GPS gps0;
    xbee_uart( dev, "gps1\r");
    write_control_sd(F("gps1"));
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    gps_get(&gps0);
    delay(1000);
    gps_switch();
    xbee_uart( dev, "gps2\r");
    write_control_sd(F("gps2"));
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    gps_get(&gps0);
    delay(1000);
    gps_switch();
    xbee_uart( dev, "calibration\r");
    write_control_sd(F("calibration"));
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    delay(1000);
    tm_calibration();
    delay(1000);
    xbee_uart( dev, "turn test\r");
    write_control_sd(F("turn test"));
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    delay(1000);
    double t = 0 ;
    turn_target_direction(0, &t, 0);
    delay(1000);
    turn_target_direction(180, &t, 0);
    delay(1000);
    xbee_uart( dev, "ac test\r");
    write_control_sd(F("ac test"));
    delay(1000);
    xbee_uart( dev, "set rover reverse\r");
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    delay(1000);
    if (judge_invered == 0) {
      xbee_uart( dev, "ac ok\r");
      write_control_sd(F("ac ok"));
    } else {
      xbee_uart( dev, "ac bad\r");
      write_control_sd(F("ac bad"));
    }
    delay(1000);
    xbee_uart( dev, "light test\r");
    write_control_sd(F("light test"));
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    int light_t = 0;
    delay(1000);
    for (int l = 0; l < 5; l++) {
      light_t += digitalRead(LIGHT_PIN);
      xbee_uart( dev, "count\r");
      sprintf(xbee_send, "%d\r", light_t);
      xbee_uart( dev, xbee_send);
      write_control_sd("light count is " + String(light_t, DEC));
      delay(1000);
    }
    xbee_uart( dev, "nicrom test\r");
    write_control_sd(F("nicrom test"));
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    delay(1000);
    cut_nicrom(5000);
    xbee_uart( dev, "motor test\r");
    write_control_sd(F("motor test"));
    xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。
    go_rotate(1000);
    delay(1000);
    go_rotate(-1000);
    delay(1000);
    go_straight(1000);
    delay(1000);
    go_back(1000);
    delay(1000);
    xbee_uart( dev, "all test end\r");
    write_control_sd(F("all test end"));

  }

  write_control_sd(F("start standby"));
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。


  return 1;
}

