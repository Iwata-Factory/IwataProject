/*
 * １０秒おきに写真を計１０枚撮影
 */
void take_picture()
{
  int pict_cnt = 0;
  while (1) {
    xbprintf("\r\nPress the button to take a picture");
    while (digitalRead(CAM_BUTTON) == LOW);      //wait for buttonPin status to HIGH
    if (digitalRead(CAM_BUTTON) == HIGH) {
      delay(20);                               //Debounce・ボタン式にしないのでいらない。。。？
      if (digitalRead(CAM_BUTTON) == HIGH)
      {
        xbprintf("\nbegin to take picture\n");
        delay(200);
        if (pict_cnt == 0) preCapture();
        Capture();
        cam_GetData();
      }
      xbprintf("Taking pictures success ,number : %d", pict_cnt);
      if(pict_cnt>10){
        break;
      }
      pict_cnt++ ;
      delay(10000);
    }
  }
}
/*********************************************************************/
void clearRxBuf()
{
  while (Serial2.available())
  {
    Serial2.read();
  }
}
/*********************************************************************/
void sendCmd(char cmd[], int cmd_len)
{
  for (char i = 0; i < cmd_len; i++) {
    Serial2.print(cmd[i]);
    xbprintf(cmd[i]);
  }
}
/*********************************************************************/
void cam_initialize()
{
  char cmd[] = {0xaa, 0x0d | cameraAddr, 0x00, 0x00, 0x00, 0x00} ;
  unsigned char resp[6];


  Serial2.setTimeout(500);
  while (1)
  {
    //clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial2.readBytes((char *)resp, 6) != 6)
    {
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0)
    {
      if (Serial2.readBytes((char *)resp, 6) != 6) continue;
      if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break;
    }
  }
  cmd[1] = 0x0e | cameraAddr;
  cmd[2] = 0x0d;
  sendCmd(cmd, 6);
  xbprintf("\nCamera initialization done.");
}
/*********************************************************************/
void preCapture()
{
  char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };
  unsigned char resp[6];


  Serial2.setTimeout(100);
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial2.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
  }
}
void Capture()
{
  char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN >> 8) & 0xff , 0};
  unsigned char resp[6];


  Serial2.setTimeout(100);
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial2.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0;
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial2.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial2.readBytes((char *)resp, 6) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      Serial2.setTimeout(1000);
      if (Serial2.readBytes((char *)resp, 6) != 6)
      {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16);
        xbprintf("picTotalLen:%f", picTotalLen);
        break;
      }
    }
  }


}
/*********************************************************************/
void cam_GetData()
{
  unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6);
  if ((picTotalLen % (PIC_PKT_LEN - 6)) != 0) pktCnt += 1;


  char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };
  unsigned char pkt[PIC_PKT_LEN];


  char picName[] = "pic00.jpg";
  picName[3] = picNameNum / 10 + '0';
  picName[4] = picNameNum % 10 + '0';


  if (SD.exists(picName))
  {
    SD.remove(picName);
  }


  cam_pic = SD.open(picName, FILE_WRITE);
  if (!cam_pic) {
    xbprintf("cam_picture open fail...");
  }
  else {
    Serial2.setTimeout(1000);
    for (unsigned int i = 0; i < pktCnt; i++)
    {
      cmd[4] = i & 0xff;
      cmd[5] = (i >> 8) & 0xff;


      int retry_cnt = 0;
retry:
      delay(10);
      clearRxBuf();
      sendCmd(cmd, 6);
      uint16_t cnt = Serial2.readBytes((char *)pkt, PIC_PKT_LEN);


      unsigned char sum = 0;
      for (int y = 0; y < cnt - 2; y++)
      {
        sum += pkt[y];
      }
      if (sum != pkt[cnt - 2])
      {
        if (++retry_cnt < 100) goto retry;
        else break;
      }


      cam_pic.write((const uint8_t *)&pkt[4], cnt - 6);
      //if (cnt != PIC_PKT_LEN) break;
    }
    cmd[4] = 0xf0;
    cmd[5] = 0xf0;
    sendCmd(cmd, 6);
  }
  cam_pic.close();
  picNameNum ++;
}
