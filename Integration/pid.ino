//PIDの関数
//pid(センサー値, 目標値)
double pid(double sensor_val, double target_val){
  double p = 0;    //PID用比例
  double i = 0;    //微分
  double d = 0;    //積分
  double integral = 0;

  diff[0] = diff[1];  //前回の値を代入
  diff[1] = sensor_val - target_val;   //偏差計算
  integral += (diff[1] + diff[2]) / 2.0 * DELTA_T;  

  p = Kp * diff[1];
  i = Ki * integral;
  d = Kd * (diff[1]-diff[0]) / DELTA_T;

  return (p + i + d);
  
}

