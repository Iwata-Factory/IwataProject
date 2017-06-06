//６月１日時点でのテストはまだ。
//integrationにてinclude,グローバル,setupを確認のこと。

void xbee_send_3doubles(double first, double second, double third){
  char xbee_send[48];
  char xbee_first[16];  //get_gpsと同じ定義をするのは二度手間では。。？
  char xbee_second[16];
  char xbee_third[16];

  //小数点6けたで表示する
  dtostrf(first,10,6,xbee_first);
  dtostrf(second, 10, 6, xbee_second);
  dtostrf(third, 10, 6, xbee_third);
  
  sprintf( xbee_send, "%s,%s,%s\r", xbee_first, xbee_second, xbee_third );
  xbee_uart( dev, xbee_send );  //送信
  delay(1000);
  //ここにdelayを入れるかintegrationの方でdelayを入れるかしてください。1000程度
}

