#include <EEPROM.h>

void setup() {

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Read flag from EEPROM: ");
  Serial.print("rover status:");
  Serial.println(EEPROM.read(0));
  Serial.print("censor status:");
  Serial.println(EEPROM.read(1));
  eep_gpsread(1); //Run the next test.
}

struct GPS { // GPS関連    /* これだけ良くわからなかったのでtypedefしていません */
  double utc = 0.0;       //グリニッジ天文時
  double latitude = 0.0;   //経度
  double longitude = 0.0;   //緯度
  double Speed = 0.0;    //移動速度
  double course = 0.0;    //移動方位
  double Direction = -1.0;   //目的地方位
  double distance = -1.0;     //目的地との距離
  /*Speedとdistanceは小文字が予約語だったのでとりあえず大文字にしてあります*/
};

void loop() {
  /* Empty loop */
}
