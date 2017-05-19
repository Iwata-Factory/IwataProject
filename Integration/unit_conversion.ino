// 単位換算関数を書く関数


/*-----------radからdegに--------------------
  引数はdegree(double型)
  ------------------------------------------*/
double rad2deg(double rad) {
  double deg = rad * 180.0 / PI;
  return (deg);
}

/*-----------degからradに--------------------
  引数はrad(double型)
  ------------------------------------------*/
double deg2rad(double deg) {
  double rad = deg * PI / 180.0;
  return (rad);
}



