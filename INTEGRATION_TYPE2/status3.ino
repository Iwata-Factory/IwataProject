
int status3(ROVER *rover) {  // Status3 降下の関数

  if (_S3_ == 0) {
    xbee_uart( dev, "skip status3\r");
    delay(1000);
    return 1;
  }

  return 1;
}

