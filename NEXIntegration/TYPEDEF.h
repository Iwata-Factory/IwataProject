#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

// 構造体を宣言

typedef struct {  // 機体自身の情報を定義
  
  double latitude = -1.0;  // 自身の緯度
  double longitude = -1.0;  // 自身の経度
  double My_Direction = -1.0;  // 自身が向いているの方角
  
  double Target_Direction = -1.0;  // ターゲットの方角
  double distance = -1.0;  //ターゲットまでの距離
  
  long time_from_start = 0.0;  // 機体の現在時刻
  
  int status_number = 0;  // 自分の現在のシーケンスを把握
  
  int gpa1_arive = 1;  // 1個目のgpsが生きているか（1:生存、0:死亡）
  int gps2_arive = 1;
  int tm_arive = 1;
  int ac_arive = 1;
  int ill_arive = 1;
  int distance_arive = 1;
  
} ROVER;

typedef struct { // 2次元のベクトル
  double x = 0.0; //2次元ベクトルのx座標
  double y = 0.0; //2次元ベクトルのy座標
} Vector2D;

typedef struct { // 3次元のベクトル
  double x = 0.0; //3次元ベクトルのx座標
  double y = 0.0; //3次元ベクトルのy座標
  double z = 0.0; //3次元ベクトルのz座標
} Vector3D;

typedef struct{ // GPS関連    
  double utc = 0.0;       //グリニッジ天文時
  double latitude = 0.0;   //経度
  double longitude = 0.0;   //緯度
  double Speed = 0.0;    //移動速度
  double course = 0.0;    //移動方位
  double Direction = -1.0;   //目的地方位
  double distance = -1.0;     //目的地との距離
  /*Speedとdistanceは小文字が予約語だったのでとりあえず大文字にしてあります*/
} GPS;

typedef struct { // 加速度センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
} AC;

typedef struct { // 地磁気センサ
  double x = 0.0; // x軸方向
  double y = 0.0; // y軸方向
  double z = 0.0; // z軸方向
} TM;

typedef struct { // モーター制御
  int right1 = 0; // 8番ピン対応
  int right2 = 0; // 9番ピン対応
  int leght1 = 0; // 10番ピン対応
  int leght2 = 0; // 11番ピン対応
} DRIVE;

#endif
