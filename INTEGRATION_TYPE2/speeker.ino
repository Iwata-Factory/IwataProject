//スピーカーなります
/*pin3.11のpwm出力できなくなります*/
//Ａ～Ｇを引数にとるとその音がなります

/*
 * tone(ピン番号, 音の周波数, 音の長さ)
 */
void speaker(int TONE) {
  tone(TONE_PINNO, TONE, BEAT_LONG);
  delay(BEAT_LONG);
}

