/*
Implementation of geografic distance calculation between two point in degrees, with altitude in meters.
*/
#include <math.h>
#include "geo_distance.h"

double distance(double p1[], double p2[]){
  //point are supposed to be in the format {lat, lon, alt}
  static double lat_distance;
  static double lon_distance;
  static double a;
  static double c;
  static double xy_distance;
  static double height;
  static double distance;

  lat_distance = to_radians(p2[0]-p1[0]);
  lon_distance = to_radians(p2[1]-p1[1]);
  a = sin(lat_distance/2.0)*sin(lat_distance/2.0) + cos(to_radians(p1[0])) *
            cos(to_radians(p2[0])) * sin(lon_distance/2.0)*sin(lon_distance/2.0);
  c = 2.0 * atan2(sqrt(a), sqrt(1.0-a));

  xy_distance = R * c * 1000.0; //convert to meters

  height = p2[2] - p1[2];

  distance = sqrt(pow(xy_distance, 2.0) + pow(height, 2.0));

  return distance;
}

double to_radians(double degrees){
  static double radians;
  radians = degrees * PI/180.0;
  return radians;
}
