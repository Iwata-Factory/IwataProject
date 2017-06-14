void xbee_aclog(AC ac){
  if(write_ac_sd(ac)){   //書き込み成功時に送信
    char xbee_acx[16];
    char xbee_acy[16];
    char xbee_acz[16];
    char xbee_aclog[48];
    
    dtostrf(ac.x,10,6,xbee_acx);
    dtostrf(ac.y,10,6,xbee_acy);
    dtostrf(ac.z,10,6,xbee_acz);  //変換

    sprintf( xbee_aclog, "%s,%s,%s\r",xbee_acx,xbee_acy,xbee_acz );
    xbee_uart( dev, "aclog  " );  //送信
    delay(500);
    xbee_uart( dev, xbee_aclog );
    delay(500);
  }  
}

void xbee_tmlog(TM tm){
  if(write_tm_sd(tm)){
    char xbee_tmx[16];
    char xbee_tmy[16];
    char xbee_tmz[16];
    char xbee_tmlog[48];

    dtostrf(tm.x, 10,6,xbee_tmx);
    dtostrf(tm.y, 10,6,xbee_tmy);
    dtostrf(tm.z, 10,6,xbee_tmz);

    sprintf( xbee_tmlog, "%s,%s,%s\r", xbee_tmx,xbee_tmy,xbee_tmz );
    xbee_uart( dev, "tmlog  " );
    delay(500);
    xbee_uart( dev, xbee_tmlog );
    delay(500);
  }
}

void xbee_gpslog(struct GPS gps){
  if(write_gps_sd(gps)){
    char xbee_utc_sd[16];
    char xbee_lat_sd[16];
    char xbee_long_sd[16];
    char xbee_spd_sd[16];
    char xbee_crs_sd[16];
    char xbee_dir_sd[16];
    char xbee_dis_sd[16];
    char xbee_gpslog1[48];
    char xbee_gpslog2[48];

    dtostrf(gps.utc ,10,6, xbee_utc_sd);
    dtostrf(gps.latitude ,10,6, xbee_lat_sd);
    dtostrf(gps.longitude ,10,6, xbee_long_sd);
    dtostrf(gps.Speed ,10,6, xbee_spd_sd);
    dtostrf(gps.course ,10,6, xbee_crs_sd);
    dtostrf(gps.Direction ,10,6, xbee_dir_sd);
    dtostrf(gps.distance ,10,6, xbee_dis_sd);
    sprintf( xbee_gpslog1, "%s,%s,%s\r", xbee_lat_sd, xbee_long_sd, xbee_spd_sd);
    sprintf( xbee_gpslog2, "%s,%s,%s\r", xbee_crs_sd, xbee_dir_sd, xbee_dis_sd);

    xbee_uart( dev, xbee_utc_sd );
    delay(300);
    xbee_uart( dev, xbee_gpslog1 );
    delay(300);
    xbee_uart( dev, xbee_gpslog2 );
    delay(300);
  }
  
}

