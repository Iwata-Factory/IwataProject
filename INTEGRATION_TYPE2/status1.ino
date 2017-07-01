int status1(ROVER *rover) {

  if (_S1_ == 0) {
    xbprintf( "skip status1");
    delay(1000);
    return 1;
  }

  return 1;
}

