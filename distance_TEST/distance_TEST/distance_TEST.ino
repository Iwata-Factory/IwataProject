#define DISTANCE A0

float distance;       //cmで距離を表示
float volt;            //電圧表示

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.write( "START\n" );
  pinMode( DISTANCE, INPUT );
}

void loop() {
  // put your main code here, to run repeatedly:
  volt = analogRead( DISTANCE ) * 5 / 1023.0;
  Serial.println( volt );  //電圧換算表示
  if ( 1.35 < volt & volt < 2.7 ) {            //有効測距範囲内
    distance = 140.0 / ( volt - 1.10 ) ;
    Serial.print( "success reading! Distance is  " );
    Serial.println( distance );
  }
  delay( 500 );
}
