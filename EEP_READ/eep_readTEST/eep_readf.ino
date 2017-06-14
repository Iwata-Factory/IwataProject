
void eep_gpsread(int adr) {

  struct GPS gps; //Variable to store custom object read from EEPROM.
  EEPROM.get(adr, gps);

  Serial.println("\nRead gps from EEPROM: ");
  Serial.println(gps.utc);
  Serial.println(gps.latitude);
  Serial.println(gps.longitude);
  Serial.println(gps.Speed);
  Serial.println(gps.course);
  Serial.println(gps.Direction);
  Serial.println(gps.distance);
}


