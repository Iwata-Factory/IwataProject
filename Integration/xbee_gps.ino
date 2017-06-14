//６月１日時点でのテストはまだ。
//integrationにてinclude,グローバル,setupを確認のこと。

void send_xbee(struct GPS* gps){
  char xbee_gps[48];
  char xbee_utc[16];  //get_gpsと同じ定義をするのは二度手間では。。？
  char xbee_lat[16];
  char xbee_long[16];

  //小数点6けたで表示する
  dtostrf(gps->utc,10,6,xbee_utc);
  dtostrf(gps->latitude, 10, 6, xbee_lat);
  dtostrf(gps->longitude, 10, 6, xbee_long);
  
  sprintf( xbee_gps, "%s,%s,%s\r", xbee_utc, xbee_lat, xbee_long );
  xbee_uart( dev, xbee_gps );  //送信
  //ここにdelayを入れるかintegrationの方でdelayを入れるかしてください。1000程度
}

