int status6(ROVER *rover) {
  
  if (_S6_ == 0) {
    xbee_uart( dev, "skip status6\r");
    delay(1000);
    return 1;
  }

  return 1;
}

