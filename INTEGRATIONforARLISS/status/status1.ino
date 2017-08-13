int status1(ROVER *rover) {

  if (_S1_ == 0) {
    xbee_uart( dev, "skip status1\r");
    delay(1000);
    return 1;
  }

  return 1;
}

