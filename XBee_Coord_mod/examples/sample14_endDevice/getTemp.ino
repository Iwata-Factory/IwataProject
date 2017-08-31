/*******************************************************************************

                             温度計 for Arduino

本ソースリストおよびソフトウェアは、ライセンスフリーです。
個人での利用は自由に行えます。著作権表示の改変は禁止します。

常識・良識の範囲での利用については特に権利を主張しません。
Arduinoが普及するように使っていただけるとありがたいです。

                               Copyright (c) 2011 GarretLab all rights reserved.
                               http://garretlab.web.fc2.com/

                               Copyright (c) 2012-2013 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*******************************************************************************/
/*
参考文献

本ソフトの開発のために下記の文献を参考にしました。

・Arduino Playground - Internal Temperature Sensor
	http://playground.arduino.cc/Main/InternalTemperatureSensor
・Arduinoで遊ぶページ Arduino(ATmega328)内蔵温度センサの実験
    GarretLab 
    http://garretlab.web.fc2.com/arduino/lab/internal_temperature_sensor/
・Arduinoをはじめよう (オライリージャパン)
    Massimo Banzi著 船田功訳
・Arduino 日本語リファレンス
    http://www.musashinodenpa.com/arduino/ref/index.php
*/

// Internal Temperature Sensor
// Example sketch for ATmega328 types.
// 
// April 2012, Arduino 1.0

float getTemp(void){
  unsigned int wADC;
  float t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return (t);
}

#define temp_sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

int analogRead8()
{
  uint8_t low, high;

  ADMUX = 0xc8;
  temp_sbi(ADCSRA, ADSC);
  while (bit_is_set(ADCSRA, ADSC));

  low = ADCL;
  high = ADCH;
  return (high << 8) | low;
}

float getA8Temp(void){
  return( map( (float)analogRead8() * 1100.0 / 1024 , 242, 380, -45, 85) );
}

