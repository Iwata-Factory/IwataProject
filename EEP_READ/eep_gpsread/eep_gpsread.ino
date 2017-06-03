/***
    eeprom_get example.

    This shows how to use the EEPROM.get() method.

    To pre-set the EEPROM data, run the example sketch eeprom_put.
    This sketch will run without it, however, the values shown
    will be shown from what ever is already on the EEPROM.

    This may cause the serial object to print out a large string
    of garbage if there is no null character inside one of the strings
    loaded.

    Written by Christopher Andrews 2015
    Released under MIT licence.
***/

#include <EEPROM.h>

void setup() {

  float f = 0.00f;   //Variable to store data read from EEPROM.
  int eeAddress = 0; //EEPROM address to start reading from

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Read float from EEPROM: ");
  EEPROM.read(0);
  /***
    As get also returns a reference to 'f', you can use it inline.
    E.g: Serial.print( EEPROM.get( eeAddress, f ) );
  ***/

  /***
    Get can be used with custom structures too.
    I have separated this into an extra function.
  ***/

  secondTest(); //Run the next test.
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

void secondTest() {

  struct GPS gps; //Variable to store custom object read from EEPROM.
  EEPROM.get(1, gps);

  Serial.println("\nRead gps from EEPROM: ");
  Serial.println(gps.utc);
  Serial.println(gps.latitude);
  Serial.println(gps.longitude);
  Serial.println(gps.Speed);
  Serial.println(gps.course);
  Serial.println(gps.Direction);
  Serial.println(gps.distance);
}

void loop() {
  /* Empty loop */
}
