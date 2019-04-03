/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen, 
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


*/
#include "Geoid.h"
#include "Point.h"
#include "SpatialAlgebra.h"
#include "HalfSegment.h"


namespace geodist{


  static const double _d2r = (M_PI / 180);
  static const double PRECISION = 0.1;

  // Haversine Algorithm
  // source: 
  //http://stackoverflow.com/questions/365826/
  // calculate-distance-between-2-gps-coordinates
  double HaversineInKM(const double lat1, const double long1, 
                       const double lat2, const double long2, 
                       const Geoid* geoid) {
     double dlong = (long2 - long1) * _d2r;
     double dlat = (lat2 - lat1) * _d2r;
     double a = pow(sin(dlat / 2), 2) + cos(lat1 * _d2r) * cos(lat2 * _d2r)
              * pow(sin(dlong / 2), 2);
     double c = 2 * atan2(sqrt(a), sqrt(1 - a));
     double d = geoid->getR()* c;
     return d;
  }

  double HaversineInM(const double lat1, const double long1, 
                      const double lat2, const double long2, 
                      const Geoid* geoid) {
        return  (1000 * HaversineInKM(lat1, long1, lat2, long2,geoid));
  }


  double toRadians(double a){
    return _d2r*a;
  }
  
  double toDegrees(double a){
    return a/_d2r;
  }

  // source: http://stackoverflow.com/questions/1185408/
  // converting-from-longitude-latitude-to-cartesian-coordinates
   void toCartsian(const double* coord, double* result,  const Geoid* geoid) {
      double _eQuatorialEarthRadius = geoid->getR();
      result[0] = _eQuatorialEarthRadius * cos(toRadians(coord[0])) 
                  * cos(toRadians(coord[1]));
      result[1] = _eQuatorialEarthRadius * cos(toRadians(coord[0])) 
                  * sin(toRadians(coord[1]));
      result[2] = _eQuatorialEarthRadius * sin(toRadians(coord[0]));
    }
    
    void vectorProduct (const double* a, const double* b, double* result){
        result[0] = a[1] * b[2] - a[2] * b[1];
        result[1] = a[2] * b[0] - a[0] * b[2];
        result[2] = a[0] * b[1] - a[1] * b[0];
    }

    void normalize(double* t) {
        double length = sqrt((t[0] * t[0]) + (t[1] * t[1]) + (t[2] * t[2]));
        t[0] = t[0]/length;
        t[1] = t[1]/length;
        t[2] = t[2]/length;
    }
    

    void multiplyByScalar(double*  v, const double k) {
        v[0] = v[0]*k;
        v[1] = v[1]*k;
        v[2] = v[2]*k;
    }
    

    void fromCartsian(const double* coord, double* result, const Geoid* geoid){
        result[0] = toDegrees(asin(coord[2] / geoid->getR()));
        result[1] = toDegrees(atan2(coord[1], coord[0]));
    }

  // source: http://stackoverflow.com/questions/1299567/
  // how-to-calculate-distance-from-a-point-to-a-line-segment-on-a-sphere
  void  nearestPointGreatCircle(const double* a, const double*  b, 
                                const double*  c, const Geoid* geoid,
                                 double* result) {
        double a_[3];
        double b_[3];
        double c_[3];
        toCartsian(a,a_,geoid);
        toCartsian(b,b_,geoid);
        toCartsian(c,c_,geoid);
        double G[3];
        double F[3];
        double t[3];

        vectorProduct(a_, b_,G);
        vectorProduct(c_, G, F);
        vectorProduct(G, F, t);
        normalize(t);
        multiplyByScalar(t,geoid->getR());
        fromCartsian(t,result, geoid);
   }

   bool onSegment (const double* a, const double* b, const double* t, 
                   const Geoid* geoid) {
         // should be   return distance(a,t)+distance(b,t)==distance(a,b), 
         // but due to rounding errors, we use: 
     return std::abs(   HaversineInKM(a[0], a[1], b[0], b[1], geoid) 
                 - HaversineInKM(a[0], a[1], t[0], t[1], geoid)
                 - HaversineInKM(b[0], b[1], t[0], t[1], geoid)) < PRECISION;
   }

    void nearestPointSegment (const double* a, const double* b, 
                              const double* c, double* result, 
                              const Geoid* geoid)
    {
       
       nearestPointGreatCircle(a,b,c, geoid, result);
       if (onSegment(a,b,result, geoid)) 
          return;
       if (  HaversineInKM(a[0], a[1], c[0], c[1], geoid) 
           < HaversineInKM(b[0], b[1], c[0], c[1], geoid)) {
          result[0] = a[0];
          result[1] = a[1];
       } else {
          result[0] = b[0];
          result[1] = b[1];
       }
    }


    double getDist(const HalfSegment& hs, const Point& p, const Geoid* geoid){
       double p1[] = {hs.GetLeftPoint().GetY(), hs.GetLeftPoint().GetX()};
       double p2[] = {hs.GetRightPoint().GetY(), hs.GetRightPoint().GetX()};
       double p3[] = {p.GetY(), p.GetX()};
       double np[2];
       nearestPointSegment(p1,p2,p3,np,geoid);

       cout << "nearest point is computed as" << np[0] << ", " << np[1] << endl;
       Point npp(true,np[1], np[0]);

       //return HaversineInM(p3[0],p3[1],np[0], np[1], geoid);
       bool valid;
       return p.DistanceOrthodrome(npp, *geoid, valid);
    } 


} // end of namespace geodist

