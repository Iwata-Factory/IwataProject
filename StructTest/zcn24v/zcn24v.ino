#include <Wire.h>
#include <SoftwareSerial.h>
#define SERIAL_BAUDRATE 9600
//XBEE関連
#define XBEE_BUFFERSIZE 63
#define ENTER 0x0D
#define O_CAP 0x4F
byte dev[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xED, 0x61};  // XBEE親機アドレス（６月３０日改造版）
char xbee_send[XBEE_BUFFERSIZE];  //とりあえずのxbee送信用配列
#define XBEE_SWITCH 1  // 0にするとxbeeを全て無視します(今は動作不安定っぽい)
//
//#define M1_1 4
//#define M1_2 5
//#define M2_1 7
//#define M2_2 6

#define M1_1 7
#define M1_2 6
#define M2_1 4
#define M2_2 5
#include <xbee.h>  //このライブラリはslackを参照して各自PCに入れてください。

typedef struct {
  int right1 = 0;
  int right2 = 0;
  int leght1 = 0;
  int leght2 = 0;
} DRIVE;
void rover_degital(DRIVE drive) {
  digitalWrite(M1_1, drive.right1);
  digitalWrite(M1_2, drive.right2);
  digitalWrite(M2_1, drive.leght1);
  digitalWrite(M2_2, drive.leght2);
}
void rover_analog(DRIVE drive) {
  analogWrite(M1_1, drive.right1);
  analogWrite(M1_2, drive.right2);
  analogWrite(M2_1, drive.leght1);
  analogWrite(M2_2, drive.leght2);
}
void  setup() {
  Serial.begin(SERIAL_BAUDRATE);


  //  xbee関連
  if (XBEE_SWITCH == 1) {
    xbee_init(0);  //初期化
    xbee_atcb(4);  //ネットワーク初期化
    xbee_atnj(0);  //孫機のジョイン拒否
    while (xbee_atai() > 0x01) { //ネットワーク参加状況を確認
      delay(3000);
      xbee_atcb(1);  //ネットワーク参加ボタン押下
    }
  }

  pinMode(M1_1, OUTPUT);
  pinMode(M1_2, OUTPUT);
  pinMode(M2_1, OUTPUT);
  pinMode(M2_2, OUTPUT);


}
void loop() {


  DRIVE go;
  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;

  xbee_uart( dev, "255 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 255;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);



  xbee_uart( dev, "250 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 250;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);

  xbee_uart( dev, "245 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 245;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);

  xbee_uart( dev, "240 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 240;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);

  xbee_uart( dev, "235 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 235;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);


  xbee_uart( dev, "230 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 230;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);


  xbee_uart( dev, "225 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 225;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);

  xbee_uart( dev, "220 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 220;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);


  xbee_uart( dev, "215 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 215;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);

  xbee_uart( dev, "210 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 210;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);



  xbee_uart( dev, "205 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 205;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);

  xbee_uart( dev, "200 255\r");
  xbee_standby();  // 現状enter押下したのちに大文字のOを入力することによって脱出します。

  go.right1 = 0;
  go.right2 = 200;
  go.leght1 = 0;
  go.leght2 = 255;
  rover_analog(go);

  delay(5000);

  for (int i = 255; i > 0; i--) {
    go.right1 = 0;
    go.right2 = i;
    go.leght1 = 0;
    go.leght2 = i;
    rover_analog(go);
    delay(5);
  }

  go.right1 = 1;
  go.right2 = 1;
  go.leght1 = 1;
  go.leght2 = 1;
  rover_degital(go);
  delay(2500);


}


int xbee_standby() {
  xbee_uart( dev, "waiting for your command...\r");
  int xb_rxcnt = 0;

  while (1) {  // コマンド受信待機

    if (xbee_rcv(ENTER)) {
      xbprintf("are you ready?");
      while (1) {
        if (xbee_rcv( O_CAP )) {
          xbprintf("your command finally accepted!");
          return 1;
        }
        xbee_uart(dev, " ");  // これを回さないとxbeeが動かない。。。
      }
    }

    //    xb_rxcnt++;
    xbee_uart(dev, " ");  // これを回さないとxbeeが動かない。。。
    //    if (xb_rxcnt > 100000 ) {  //timeout時間約8000秒
    //      xbprintf("can't receive command...");
    //      break;
    //    }
  }
  return 0;
}

int xbee_rcv(byte character) { // ここの引数は実際のxbeeで送られる16進数をdefineで定義してあります。
  XBEE_RESULT xbee_result;
  xbee_rx_call(&xbee_result);
  if (xbee_result.MODE == MODE_UART) {
    if (xbee_result.DATA[0] == character) {
      return 1;
    }
  }
  else {
    return 0;
  }
}



void xbprintf(char *fmt, ...) {
  char xb_buf[XBEE_BUFFERSIZE];
  va_list args;
  va_start (args, fmt);
  vsnprintf(xb_buf, XBEE_BUFFERSIZE, fmt, args);
  va_end (args);
  xbee_uart(dev, xb_buf);
  xbee_uart(dev, "\r");
}

