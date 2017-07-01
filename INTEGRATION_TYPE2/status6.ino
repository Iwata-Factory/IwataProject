int status6(ROVER *rover) {

  if (_S6_ == 0) {
    xbprintf( "skip status6");
    delay(1000);
    return 1;
  }

  return 1;
}

