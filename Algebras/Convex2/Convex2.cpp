/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
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



#include "Convex2.h"
#include "Attribute.h"
#include <set>
#include <iostream>
#include "NestedList.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include <tuple>
#include <vector> 
#include <algorithm> 
#include "ListUtils.h"
#include "NList.h"
#include "StandardTypes.h"
#include "Stream.h"
#include "Algebras/FText/FTextAlgebra.h"
#include <cstdio>
#include "AlmostEqual.h"
#include <limits>
#include "math.h"
#include "Algebras/Collection/IntSet.h"
#include "predicates.c"


#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;
using boost::polygon::low;
using boost::polygon::high;
using boost::polygon::construct_voronoi;



/*
 
  1.1 boost definitions needed for constructing the voronoi diag
  
*/
  


namespace boost {
  namespace polygon {


    struct VoronoiPoint {
      double a;
      double b;
      VoronoiPoint(double x, double y) : a(x), b(y) {}
    };   
    

    template<>
    struct geometry_concept<VoronoiPoint>{typedef point_concept type;};

    template <>
    struct point_traits<VoronoiPoint> {
      typedef double coordinate_type;

      static inline coordinate_type get(
          const VoronoiPoint& point, orientation_2d orient) {
        return (orient == HORIZONTAL) ? point.a : point.b;
      }
    };



}}

using namespace std; 


extern NestedList* nl;
using boost::polygon::VoronoiPoint;


namespace convex { 

/*
 
   2.1 constructors
  
*/

Convex::Convex(const Convex& src):Attribute(src),value(0),
  size(src.size),cellId(0){
  if(size>0){
     value = new Point[size];
     memcpy((char*)value, src.value,size*sizeof(Point));
     memcpy(&cellId, &src.cellId, sizeof(int));

  }
}



Convex::Convex(Convex&& src): Attribute(src), 
   value(src.value), size(src.size), cellId(src.cellId) {
   src.size = 0;
   src.value = 0;
   src.cellId = 0;
}

Convex::Convex(const std::vector<std::tuple 
                <double, double>>& src, int id_):Attribute(true), value(0){
   setTo(src, id_);
}

Convex::Convex(const std::vector<std::tuple 
                <double, double>>& src):Attribute(true), value(0){
   setTo(src, -1);
}



Convex& Convex::operator=(const Convex& src) {
   Attribute::operator=(src);
   if(this->size != src.size){
      delete[] value;
      value = 0;
      cellId = 0;
      this->size = src.size;
      if(size > 0){
        value = new Point[size];
      }
   }
   if(size > 0){
     memcpy((char*)value, src.value,size*sizeof(Point));
     memcpy(&cellId, &src.cellId, sizeof(int));
   }
   return *this;
}





Convex& Convex::operator=(Convex&& src) {
   Attribute::operator=(src);
   size = src.size;
   value = src.value;
   cellId = src.cellId;
   src.size = 0;
   src.value = 0;
   src.cellId = 0;
   return *this;
}

void
Convex::setCellId(int cell_id) {
  this->cellId = cell_id;
}

int
Convex::getCellId() {
  return cellId;
}


void 
Convex::setTo(const std::vector<std::tuple <double, double>>& src,
   int id_){
      clear();
      SetDefined(true);
      if(src.size()>0){
         size = src.size();
         value = new Point[size];
         setCellId(id_);
         auto it = src.begin();
         size_t pos = 0;
         std::tuple<double, double> temptup;
         double tempxval;
         double tempyval;
         Point poi;

            
    
         
         
/*
   converting the right tuple vector into point sequence  
      
*/   
         while(it!=src.end()) {
           
         temptup = *it;
         tempxval = std::get<0>(temptup);
         tempyval = std::get<1>(temptup);         
         poi.Set(true, tempxval, tempyval);
         value[pos] = poi;          
              
           it++;
           pos++;
         }


      }
   }






/*
 
  3.1 auxillary  functions
  
*/

bool insideRect(Rectangle<2>* r, std::tuple<double,double> center) {
  double x = get<0>(center);
  double y = get<1>(center);

  if (r->getMinX() <= x && r->getMaxX() >= x 
  && r->getMinY() <= y && r->getMaxY() >= y) 
  {                        
    return true;
  }

 return false;
}


Point getRectCentre(Rectangle<2>* r) 
{
  Point rc;
  double a = (r->getMaxY() - r->getMinY()) / (double)2;
  double b = (r->getMaxX() - r->getMinX()) / (double)2;
  rc.Set(r->getMinX()+ b, r->getMinY() + a);
  return rc;
}

/*
  Returns bounding box of a convex polygon

*/
Rectangle<2> createBBox(Convex* convex) {
  Rectangle<2> bbox;
  double minx = convex->value[0].GetX();  
  double maxx = convex->value[0].GetX();  
  double miny = convex->value[0].GetY();
  double maxy = convex->value[0].GetY();

  for(int c=1; c < (int)convex->size; c++) {
    double x_val = convex->value[c].GetX();
    double y_val = convex->value[c].GetY();
    if(x_val < minx) {
      minx = x_val;
    } else if (x_val > maxx) {
      maxx = x_val;
    }
    if(y_val < miny) {
      miny = y_val;
    } else if(y_val > maxy) {
      maxy = y_val;
    }
  }

  double min[2], max[2];
  min[0] = minx;
  min[1] = miny;
  max[0] = maxx;
  max[1] = maxy;

  bbox = Rectangle<2>(true, min, max);
  return bbox;

}


/*
  Returns true if point is inside of convex polygon

*/
bool PointInPolygon(Point point, Convex* conv) 
{
    std::vector<Point> poly;

    for(int a = 0; a < (int)conv->size; a++) {
      Point p1;
      p1.Set(conv->value[a].GetX(), conv->value[a].GetY());
      poly.insert(poly.begin() + a, p1);
    }

  Rectangle<2> bbox = createBBox(conv);

  /* 
    Precheck: if point is not in bbox of polygon, then it is not in 
    polygon either

  */
  if(!insideRect(&bbox, std::make_tuple(point.GetX(), point.GetY())))
  {
    return false;
  }

  /*
    go through every segment and check 
    on which side point lies

  */
  double detO;
  Point a,b;
  a = poly[0];
  b = poly[1];
  /* 
    P0 (x0,y0) and P1 (x1,y1), another point P (x,y) 
    (y - y0) (x1 - x0) - (x - x0) (y1 - y0)

  */
  detO = (point.GetY() - a.GetY())*(b.GetX() - a.GetX()) 
    - (point.GetX() - a.GetX())*(b.GetY() - a.GetY());
  // if equal to 0, point lies on edge
  if(detO == 0)
  {
    return true;
  }
  for(int e = 1; e < (int)poly.size()-1; e++) {
    Point a,b;
    a = poly[e];
    b = poly[e+1];
    
    double det = (point.GetY() - a.GetY())*(b.GetX() - a.GetX()) 
    - (point.GetX() - a.GetX())*(b.GetY() - a.GetY());

    if(det == 0)
    {
      return true;
    }

    if(signbit(detO) != signbit(det))
    {
      return false;
    }

  }
  return true;
  
}

/*
  Calculates point of intersection between two segments

*/
int getLineIntersection(Point p0, Point p1, 
  Point p2, Point p3, double *i_x, double *i_y)
{

    // Points x and y values
    double p0_x = p0.GetX();
    double p0_y = p0.GetY();

    double p1_x = p1.GetX();
    double p1_y = p1.GetY();

    double p2_x = p2.GetX();
    double p2_y = p2.GetY();

    double p3_x = p3.GetX();
    double p3_y = p3.GetY();

    double xp0p2, yp0p2, xp1p0, yp1p0, xp3p2,
           yp3p2, u_numrator, t_numerator, denominator, t;
    xp1p0 = p1_x - p0_x;
    yp1p0 = p1_y - p0_y;
    xp3p2 = p3_x - p2_x;
    yp3p2 = p3_y - p2_y;

    denominator = xp1p0 * yp3p2 - xp3p2 * yp1p0;
    if (denominator == 0)
        return 0; // parallel
    bool denomPositive = denominator > 0;

    xp0p2 = p0_x - p2_x;
    yp0p2 = p0_y - p2_y;
    u_numrator = xp1p0 * yp0p2 - yp1p0 * xp0p2;
    if ((u_numrator < 0) == denomPositive)
        return 0; // No intersection

    t_numerator = xp3p2 * yp0p2 - yp3p2 * xp0p2;
    if ((t_numerator < 0) == denomPositive)
        return 0; // No intersection

    if (((u_numrator > denominator) == denomPositive) ||
       ((t_numerator > denominator) == denomPositive))
        return 0; // No intersection
    // Intersection
    t = t_numerator / denominator;
    if (i_x != NULL)
        *i_x = p0_x + (t * xp1p0);
    if (i_y != NULL)
        *i_y = p0_y + (t * yp1p0);

    return 1;
}

/*
  Calculates center of a given polygon

*/
std::tuple<double,double> 
calcPolyCentroid(std::vector<std::tuple<double, double>> vertices,
   int vertexNumb)
{
    std::tuple<double,double> centroid = {0, 0};
    double signedArea = 0.0;
    double x0 = 0.0; // current x value
    double y0 = 0.0; // current y value
    double x1 = 0.0; // next x value
    double y1 = 0.0; // next y value
    double a = 0.0;  // area

    int i;
    // Calculate area of polygon with Gaußsche Trapezformel
    for(i=0; i<vertexNumb-1; ++i)
    {
        x0 = get<0>(vertices[i]);
        y0 = get<1>(vertices[i]);
        x1 = get<0>(vertices[i+1]);
        y1 = get<1>(vertices[i+1]);
        a = x0*y1 - x1*y0;
        signedArea += a;
        get<0>(centroid) += (x0 + x1)*a;
        get<1>(centroid) += (y0 + y1)*a;
    }

    // last vertex 
    x0 = get<0>(vertices[i]);
    y0 = get<1>(vertices[i]);
    x1 = get<0>(vertices[0]);
    y1 = get<1>(vertices[0]);
    a = x0*y1 - x1*y0;
    signedArea += a;
    get<0>(centroid) += (x0 + x1)*a;
    get<1>(centroid) += (y0 + y1)*a;

    signedArea *= 0.5;
    // calculate centroid with formula 
    get<0>(centroid) /= (6.0*signedArea);
    get<1>(centroid) /= (6.0*signedArea);

    return centroid;
}

/*
  Returns true if two rectangles (l1, r1) and (l2, r2) overlap 

*/
bool rectOverlap(Point tole1, Point bori1, Point tole2, Point bori2) 
{ 
    // If one rectangle is on left side of other 
    if (tole1.GetX() > bori2.GetX() || tole2.GetX() > bori1.GetX()) 
        return false; 
  
    // If one rectangle is above other 
    if (tole1.GetY() < bori2.GetY() || tole2.GetY() < bori1.GetY()) 
        return false; 
  
    return true; 
} 

/*
   Returns true if two cuboids (l1, r1) and (l2, r2) overlap 

*/   
bool cuboidOverlap(Point3D tolefr1, 
  Point3D boriba1, Point3D tolefr2, Point3D boriba2) 
{ 
    // If one cuboid is on left side of other 
    if (tolefr1.x > boriba2.x || tolefr2.x > boriba1.x) 
        return false; 
  
    // If one cuboid is above other 
    if (tolefr1.y < boriba2.y || tolefr2.y < boriba1.y) 
        return false; 

    if(tolefr1.z > boriba2.z || tolefr2.z > boriba1.z)
        return false;
  
    return true; 
} 



/*
  Returns bounding box of a convex polyhedron

*/
Rectangle<3> createBBox3D(Polyhedron* poly) {
  Rectangle<3> bbox;
  double minx = poly->faces.at(0).at(0).x; 
  double maxx = poly->faces.at(0).at(0).x; 
  double miny = poly->faces.at(0).at(0).y; 
  double maxy = poly->faces.at(0).at(0).y;
  double minz = poly->faces.at(0).at(0).z;
  double maxz = poly->faces.at(0).at(0).z;   

  for(size_t c=1; c < poly->faces.size(); c++) {
    for(size_t v=0; v < poly->faces.at(c).size(); v++)
    {
      double x_val = poly->faces.at(c).at(v).x;
      double y_val = poly->faces.at(c).at(v).y;
      double z_val = poly->faces.at(c).at(v).z;
      if(x_val < minx) {
        minx = x_val;
      } else if (x_val > maxx) {
        maxx = x_val;
      }
      if(y_val < miny) {
        miny = y_val;
      } else if(y_val > maxy) {
        maxy = y_val;
      }

      if(z_val < minz) {
        minz = z_val;
      } else if(z_val > maxz) {
        maxz = z_val;
      }
    }
  }

    double min[3], max[3];
    min[0] = minx;
    min[1] = miny;
    min[2] = minz;
    max[0] = maxx;
    max[1] = maxy;
    max[2] = maxz;
    

    bbox = Rectangle<3>(true, min, max);
    return bbox;
}


/*
  Returns all cellnumbers of cells which intersect with a given rectangle
  Two modes are possible: intersections with the exact cell and intersection
  with the bounding box of a cell

*/
void cellNum(Rectangle<2>* search_window_ptr,
 int mode, std::set<int> *cell_ids) {

  int bbox = 2;
  int precise = 1;
  // Rectangle points
  Point lebo, ribo, leto, rito;
  lebo.Set(search_window_ptr->getMinX(), search_window_ptr->getMinY());
  ribo.Set(search_window_ptr->getMaxX(), search_window_ptr->getMinY());
  leto.Set(search_window_ptr->getMinX(), search_window_ptr->getMaxY());
  rito.Set(search_window_ptr->getMaxX(), search_window_ptr->getMaxY());

  Point center = getRectCentre(search_window_ptr);

  // intersection with exact cell
  if(mode == precise) {
  for(int i = 0; i < (int)voroVec.size(); i++) 
  {
    Convex* tmp = &voroVec[i];
    if(tmp->size > 0) {

    /*
      -1. Precheck if object has an intersection
      with bbox of voronoi cell
      - if yes: go on with further tests
      - if not: go to next cell

    */

    Rectangle<2> bbox = createBBox(tmp);

    // see if bbox intersects with rectangle
    // top left and bottom right of bbox
    Point tole, bori;
    tole.Set(bbox.getMinX(), bbox.getMaxY());
    bori.Set(bbox.getMaxX(), bbox.getMinY());
    if(rectOverlap(tole, bori, leto, ribo)) 
    {
      

      std::vector<std::tuple<double, double>> polygon {};
      std::tuple<double, double> point;

/*
  1. Check if middle of cell intersects with rectangle 
  (get middle of cell, check if it lies in rectangle)
  - if yes: add cell\_id to cell\_ids
  - if no: check if edges intersect with rectangle
  - build edges from points (ordered clockwise), check intersection
  - if one intersect: add cell\_id to cell\_ids
  - if no edge intersects: check if rectangle is small and does
    not intersects with edges or centroid at all, but lies in cell
  - if no intersects: go to next cell, don't add cell\_id

*/

      double firstX = tmp->value[0].GetX();
      double firstY = tmp->value[0].GetY();

      point = std::make_tuple(firstX, firstY);
      polygon.insert(polygon.begin(), point);

      for(int a = 1; a < (int)tmp->size; a++) {
        point = std::make_tuple(tmp->value[a].GetX(), tmp->value[a].GetY());
        polygon.insert(polygon.begin() + a, point);
      }
      
      std::tuple<double,double> centroid = 
      calcPolyCentroid(polygon, polygon.size());

      if(insideRect(search_window_ptr, centroid)){
        cell_ids->insert(tmp->getCellId());
      } else if (PointInPolygon(center, tmp)) {
        cell_ids->insert(tmp->getCellId());
      } else {
        for(int e = 0; e < (int)tmp->size; e++) {
        Point a,b;
        a.Set(tmp->value[e].GetX(), tmp->value[e].GetY());
        if(e == (int)tmp->size-1) {
          b.Set(firstX, firstY);
        } else {
          b.Set(tmp->value[e+1].GetX(), tmp->value[e+1].GetY());

        }
        // Point of intersection i\_x, i\_y w
        double i_x = 0.0;
        double i_y = 0.0;

/* 
call to getlineintersection with 2 points of line
and two points of side of rectangle 
(for each side of rectangle)

*/ 
        if(getLineIntersection(a,b, lebo, ribo, &i_x, &i_y) == 1 
          || getLineIntersection(a,b, lebo, leto, &i_x, &i_y) == 1 
          || getLineIntersection(a,b, ribo, rito, &i_x, &i_y) == 1
          || getLineIntersection(a,b, leto, rito, &i_x, &i_y) == 1) {
          cell_ids->insert(tmp->getCellId());
          break;
        }
        }

      
      }

      if(polygon.size() > 0)
      {
        polygon.clear();
      }
    } 
    }

  }

  
  } else if (mode== bbox) {

    for(int b = 0; b < (int)voroVec.size(); b++) 
    {
      Convex* tmp = &voroVec[b];
      if(tmp->size > 0) {
      Rectangle<2> bbox = createBBox(tmp);

      // see if bbox intersects with rectangle
      // top left and bottom right of bbox
      Point tole, bori;
      tole.Set(bbox.getMinX(), bbox.getMaxY());
      bori.Set(bbox.getMaxX(), bbox.getMinY());
      if(rectOverlap(tole, bori, leto, ribo)) {
        cell_ids->insert(tmp->getCellId());
      }
    }
    }

  } else {
    cell_ids->insert(0);
  }

  return;

} 




bool sortyupxup(const tuple<double, double>& a,  
                   const tuple<double, double>& b) 
{ 
    
    if ( (get<1>(a) < get<1>(b)) ||
          ( ( get<1>(a) == get<1>(b)) && (get<0>(a) <= get<0>(b))) ) 
          return true;
     
     return false;   
    
} 


bool sortyupxdown(const tuple<double, double>& a,  
                   const tuple<double, double>& b) 
{ 
    
    if ( (get<1>(a) < get<1>(b)) ||
          ( ( get<1>(a) == get<1>(b)) && (get<0>(a) >= get<0>(b))) ) 
          return true;
     
     return false;
    
    
    
} 




bool sortxupyup(const tuple<double, double>& a,  
                const tuple<double, double>& b) 
{ 
     if ( (get<0>(a) < get<0>(b)) ||
          ( ( get<0>(a) == get<0>(b)) && (get<1>(a) <= get<1>(b))) ) 
          return true;
     
     return false;
         
} 








bool sortxdownydown (const tuple<double, double>& a,  
                const tuple<double, double>& b) 
{ 
     if ( (get<0>(a) > get<0>(b)) ||
          ( ( get<0>(a) == get<0>(b)) && (get<1>(a) >= get<1>(b))) ) 
          return true;
     
     return false;
         
} 


bool sortxdownyup (const tuple<double, double>& a,  
                const tuple<double, double>& b) 
{ 
     if ( (get<0>(a) > get<0>(b)) ||
          ( ( get<0>(a) == get<0>(b)) && (get<1>(a) <= get<1>(b))) ) 
          return true;
     
     return false;
         
} 

bool sortxupydown (const tuple<double, double>& a,  
                const tuple<double, double>& b) 
{ 
     if ( (get<0>(a) < get<0>(b)) ||
          ( ( get<0>(a) == get<0>(b)) && (get<1>(a) >= get<1>(b))) ) 
          return true;
     
     return false;
         
} 



void rmvredundpoints (std::vector<std::tuple<double, double>>& ppoi) {
   
    std::set <std::tuple<double, double>> setme;
    std::set <std::tuple<double, double>> tempvec;
    
   for (unsigned int i = 0; i < ppoi.size(); i++)
       {
           setme.insert(ppoi[i]);          
    } 
    
    ppoi.assign(setme.begin(), setme.end());
    sort(ppoi.begin(), ppoi.end(), sortxupyup); 
    
}





/*
 
  3.2 checkme function
  
*/
int checkme(std::vector<std::tuple <double, double>>& src, bool checkactive){
      
      
   
   std::vector<std::tuple<double, double>> finalcheckedvec;
   std::vector<std::tuple<double, double>> tmp;
   
   std::vector<std::tuple <double, double>> xysortedvecasc;
   std::vector<std::tuple <double, double>> yxsortedvecasc;
   std::vector<std::tuple <double, double>> xysortedvecdesc;
   std::vector<std::tuple <double, double>> ydownsortedvec;
   
   
   std::tuple<double, double> leftpoint;
   std::tuple<double, double> rightpoint;
   std::tuple<double, double> downpoint;
   std::tuple<double, double> uppoint;  
   std::tuple<double, double> actpoint;  
   std::tuple<double, double> intermpoint;
   
   
   
   std::vector<std::tuple <double, double>> above;
   std::vector<std::tuple <double, double>> below;
   std::vector<std::tuple <double, double>> aboveleft;
   std::vector<std::tuple <double, double>> belowleft;
   std::vector<std::tuple <double, double>> aboveright;
   std::vector<std::tuple <double, double>> belowright;
   std::vector<std::tuple <double, double>> temp1, temp2;
   
   
   
   unsigned int iter, sectorflag;
   unsigned int  aboveleftsize;
   unsigned int  aboverightsize;
   unsigned int  belowleftsize;
   unsigned int  belowrightsize;
   unsigned int  tmpsize;
   
   bool okflag = true;
   bool firstperpend = false;
   bool secondperpend = false;
   bool setfperflag = false;
   bool firstmflag = false;
   
   double m, b, mlast, mnew;
   
  
   
   if (src.size() <= 2) {
       return 0;
       
     }
       
       
      
   tmp = src;
 
   xysortedvecasc = tmp;
   yxsortedvecasc = tmp;
   
   
  
    

/* 
 sorting vecs by x resp. y coord  
 
*/
sort(yxsortedvecasc.begin(), yxsortedvecasc.end(), sortyupxdown);

 
 /*
  eliminate redundant points 
  
 */       
   rmvredundpoints(xysortedvecasc);  
     
 /* 
 get leftpoint and right point of polygon
   
 */
    leftpoint = xysortedvecasc[0];
    rightpoint = xysortedvecasc[xysortedvecasc.size() - 1];
    downpoint = yxsortedvecasc[0];
    uppoint = yxsortedvecasc[yxsortedvecasc.size() - 1];
    
   
   /* 
   get points above and below the imagenary "line" 
   from leftpoint to rightpoint
   
   */   
    m = (get<1>(rightpoint) - get<1>(leftpoint)) / 
        (get<0>(rightpoint) - get<0>(leftpoint));
    
    b =  get<1>(rightpoint) - (m * get<0>(rightpoint));
    
    below.clear();
    above.clear();
   
    
    
    for (unsigned int i = 0; i < xysortedvecasc.size(); i++)  {
        double fuval = (m * get<0>(xysortedvecasc[i])) + b;

        if ( fuval > get<1>(xysortedvecasc[i]) &&
            !(AlmostEqual (fuval, get<1>(xysortedvecasc[i]))))  {
            
        below.push_back (xysortedvecasc[i]);
        
        }
                
        else {
         
        above.push_back (xysortedvecasc[i]);
            
        }
        
        
        } 
       
        
          
/* 
move points above and below to 
aboveleft, aboveright, belowleft and belowright.
using the "line" from downpoint to uppoint
     
get points above and below the imagenary "line" from downpoint to uppoint
    
first: testing if point are perpendicular

*/  
  belowright.clear();
  aboveright.clear();
  belowleft.clear();
  aboveleft.clear();
   
 
 
    
  if (get<0>(uppoint) != get<0>(downpoint) ){   
    
    m = (get<1>(uppoint) - get<1>(downpoint)) / 
        (get<0>(uppoint) - get<0>(downpoint));

    
    b =  get<1>(uppoint) - (m * get<0>(uppoint));


      }
  
    
  
   if (get<0>(uppoint) != get<0>(downpoint)) { 
    
   
       
     for (unsigned int i = 0; i < above.size(); i++)  {
        
        double fval = (m * get<0>(above[i])) + b;
        if ( (fval >  get<1>(above[i]))  && 
           !(AlmostEqual (fval, get<1>(above[i]))))
        {
            
        aboveright.push_back (above[i]);
        
        }
                
        else {     
         
         aboveleft.push_back (above[i]);
            
        }
        
        
        } 
        
        
        
        
     for (unsigned int i = 0; i < below.size(); i++)  {
        
        
        if ( ((m * get<0>(below[i])) + b) >  get<1>(below[i]) ) {
            
            
        belowright.push_back (below[i]);
        
        }
                
        else {
         
         belowleft.push_back (below[i]);
            
        }
        
        
        } 
        
        
     if (m < 0) {
         
         temp1 = aboveright;
         temp2 = belowright;
         
         aboveright = aboveleft;
         belowright = belowleft;
         
         aboveleft = temp1;
         belowleft = temp2;
         
         
         
     }
        
        
     } 
     
   
   
/* 
  uppoint and downpoint are perpendicular 
  
*/
   if (get<0>(uppoint) == get<0>(downpoint) ) {
       
       for (unsigned int i = 0; i < above.size(); i++)  {
        
        
        if ( (get<0>(above[i])) > get<0>(uppoint)) {
            
            
        aboveright.push_back (above[i]);
        
        }
                
        else {
         
         aboveleft.push_back (above[i]);
            
        }
        
        
        }
        
        
        
        
        
       for (unsigned int i = 0; i < below.size(); i++)  {
        
        
        if ( (get<0>(below[i])) >  get<0>(uppoint)) {
            
        belowright.push_back (below[i]);
        
        }
                
        else {
         
         belowleft.push_back (below[i]);
            
        }
        
        
        } 
       
       
   } 
     
     
    
/* 
  sorting aboveleft, ab aboveright, belowright and belowleft... 
    
*/
   
/* 
 aboveleft is already sorted: x up , y up  
    
*/   
   sort(aboveleft.begin(), aboveleft.end(), sortxupyup);
   
/* 
  sorting aboveright: x up, y down 
    
*/ 
  sort(aboveright.begin(), aboveright.end(), sortxupydown); 
       
/* 
  sorting belowright: x down, y down 
   
*/
  sort(belowright.begin(), belowright.end(), sortxdownydown); 
    
/*
  sorting belowleft: x down,  y up  
   
*/
  sort(belowleft.begin(), belowleft.end(), sortxdownyup); 
  
  
   
      
   /*
    constructing the final vector
    
   */
   
   finalcheckedvec.clear();
     
   aboveleftsize  = aboveleft.size();
   aboverightsize = aboveright.size();
   belowleftsize = belowleft.size();
   belowrightsize = belowright.size();
   tmpsize = tmp.size();
   
      
  for (unsigned int i = 0; i < aboveleftsize; i++) {
    
    finalcheckedvec.push_back(aboveleft[i]);
       
   }
  
  
  for (unsigned int i = 0; i < aboverightsize; i++) {
    
    finalcheckedvec.push_back(aboveright[i]);
       
   }
    
  
  
  
 for (unsigned int i = 0; i < belowrightsize; i++) {
    
    finalcheckedvec.push_back(belowright[i]);
       
   }
   
   
 
    
  for (unsigned int i = 0; i < belowleftsize; i++) {
    
    finalcheckedvec.push_back(belowleft[i]);
       
   }
    
 
    
 
 
 if (tmpsize == finalcheckedvec.size()) {
  
     
     
     } 
 
 
  
  if (checkactive == false)
    { 
                
         src = finalcheckedvec;      
        return 2;}
    
   
   
/* 
 put left point at the end of the final vec for testing purposes
  
*/ 
   
   finalcheckedvec.push_back(leftpoint);
   
   actpoint = leftpoint; 
   intermpoint = uppoint;
   sectorflag = 1;
   
   iter = 1;
   
   
   if ((get<0>(leftpoint) == get<0>(finalcheckedvec[iter])) &&
       (get<0>(leftpoint) == get<0>(finalcheckedvec[iter+1]))) {
       
    
    
    ErrorReporter::ReportError(
                "The coordinates do not form a convex polygon");  
   return 0;  
       
       
   }
   
   
   
   
   if ((get<0>(leftpoint) == get<0>(finalcheckedvec[iter])) &&
       (get<0>(leftpoint) != get<0>(finalcheckedvec[iter+1]))) {
       
   firstperpend = true; 
   setfperflag = true;
  
/*
set mlast > mnew as start value
   
*/   
   mlast = 1;
   mnew = 0;   
   
   
   
   }
   
   else {
    
   mlast = (get<1>(finalcheckedvec[iter] ) - get<1>(leftpoint)) /
           (get<0>(finalcheckedvec[iter]) - get<0>(leftpoint));       
       
  
   
   firstmflag = true;
   
   mnew = mlast - 1; 
/*
just a dummy value
     
*/       
  }
   
   
   
   
   
 while ( (iter < finalcheckedvec.size()) && (okflag == true))  {
     
     
     
     
/*
 begin sector 2 case
     
*/
      
       
    
        
    if (actpoint == uppoint)    {
        
        intermpoint = rightpoint;
        sectorflag = 2;
        firstperpend = false; 
        setfperflag = false;
         }    
        
        
        
        
   if (sectorflag == 2) {
       
      
    
      if ( (iter+1 <  finalcheckedvec.size()) &&
           (get<0>(actpoint) == get<0>(finalcheckedvec[iter])) &&
           (get<0>(actpoint) == get<0>(finalcheckedvec[iter+1]))) {
          
                 
                    
                ErrorReporter::ReportError(
                "3The coordinates do not form a convex polygon");  
          return 0;  
      
      }
      
      
       
     if (get<0>(actpoint) == get<0>(finalcheckedvec[iter]) &&
         (actpoint != rightpoint) ) {
        
                 
          ErrorReporter::ReportError(
                "4The coordinates do not form a convex polygon");  
          return 0;;  
      
         
      
        }
   
          
      
      
      if (get<0>(actpoint) != get<0>(finalcheckedvec[iter]) && 
         actpoint == uppoint)
          
             {
    
            
                 
                 
             mlast = (get<1>(finalcheckedvec[iter] ) - get<1>(uppoint)) /
                     (get<0>(finalcheckedvec[iter]) - get<0>(uppoint));       
       
             firstmflag = true;
   
              mnew = mlast - 1; 
              
/* 
just a dummy value
               
*/
       
             }   
    }  
/*
end of sectorflag == 2 case
     
*/
      
       
/* 
 begin sector 3 case 
    
*/
      
     if (actpoint == rightpoint){
        intermpoint = downpoint;
        sectorflag = 3;
        firstperpend = false; 
        setfperflag = false;
        secondperpend = false;
        firstmflag = false;
        
    }
        
        
        
    if (sectorflag == 3) { 
        
        
   
     
    
   if ( (iter+1 <  finalcheckedvec.size()) &&
        (get<0>(actpoint) == get<0>(finalcheckedvec[iter])) &&
        (get<0>(actpoint) == get<0>(finalcheckedvec[iter+1])) &&
        (actpoint == rightpoint) ) {     
          
       
          ErrorReporter::ReportError(
                "4The coordinates do not form a convex polygon");  
          return 0;;  
      
      }
      
        
   
   
   
   
   
  if ((iter+1 <  finalcheckedvec.size()) &&
         (get<0>(rightpoint) == get<0>(finalcheckedvec[iter])) &&
         (get<0>(rightpoint) != get<0>(finalcheckedvec[iter+1])) &&
         (actpoint == rightpoint) )  {
       
   firstperpend = true; 
   setfperflag = true;
  
/* 
set mlast > mnew as start value; 
    
*/  
   mlast = 1;
   mnew = 0;   
   
   
   }
   
   else if (actpoint == rightpoint) {
    
   mlast = (get<1>(finalcheckedvec[iter] ) - get<1>(rightpoint)) /
           (get<0>(finalcheckedvec[iter]) - get<0>(rightpoint));       
       
      
   firstmflag = true;
   
   mnew = mlast - 1; 
/* 
just a dummy value 
    
*/      
  }
   
       
      
    } 
    
/*
end of    sectorflag == 3 case
     
*/
      
      
      
/*
  begin sector 4 case
      
*/
        
        
        
     if ((actpoint == downpoint)  && (downpoint != leftpoint)) {
        intermpoint = leftpoint;
        sectorflag = 4;
       
     
     }
       
    
    
    if ( (sectorflag == 4) && (actpoint != leftpoint) )  {
       
      
    
      if ( (iter+1 <  finalcheckedvec.size()) &&
           (get<0>(actpoint) == get<0>(finalcheckedvec[iter])) &&
           (get<0>(actpoint) == get<0>(finalcheckedvec[iter+1]))) {
          
                               
          ErrorReporter::ReportError(
                "5The coordinates do not form a convex polygon");  
          return 0;;  
      
      }
      
      
       
     if (get<0>(actpoint) == get<0>(finalcheckedvec[iter]) &&
         (actpoint != uppoint) ) {
        
                 
        ErrorReporter::ReportError(
                "6The coordinates do not form a convex polygon");  
          return 0;;  
      
         
          
        }
   
          
      
      
      if (get<0>(actpoint) != get<0>(finalcheckedvec[iter]) && 
          actpoint == downpoint)
          
             {
    
            
                 
                 
             mlast = (get<1>(finalcheckedvec[iter] ) - get<1>(downpoint)) / 
                     (get<0>(finalcheckedvec[iter]) - get<0>(downpoint));       
       
             
             firstmflag = true;
   
              mnew = mlast - 1; 
/* 
just a dummy value 
               
*/       
             }   
             
/*
  end of sector 4 case 
              
*/    
    }
    
    
    
   
       
       
       
    switch (sectorflag) {
        
        case 1: {
              
               
              if  (get<0>(actpoint) == get<0>(finalcheckedvec[iter])&&
                  (setfperflag == false) )
                  
               {secondperpend = true;
                
                
               
                
               }
         
         
              if (! ( (get<0>(actpoint) <= get<0>(finalcheckedvec[iter])) &&
                      (get<1>(actpoint) <= get<1>(finalcheckedvec[iter])) ) ||
                      (setfperflag == false && secondperpend == true) ||
                      mlast <= mnew)
              {
                 
                 okflag = false;    
                 break; }
                 
                 
                 
                 if ((iter+1 <  finalcheckedvec.size()) &&
                     
                     (firstperpend == true) && (setfperflag == true) &&
                     (get<0>(finalcheckedvec[iter+1]) != 
                      get<0>(finalcheckedvec[iter])))   {
                     
                  
                   actpoint = finalcheckedvec[iter];
                   mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                           get<1>(actpoint)) / 
                          (get<0>(finalcheckedvec[iter+1]) - 
                           get<0>(actpoint));     
                          
                   mlast = mnew + 1;
                 
                   setfperflag = false;   
                    
                  }
                 
                 else {
                     
                  
                   actpoint = finalcheckedvec[iter];
                   
                  if ( (iter+1 <  finalcheckedvec.size() ) &&
                      
                      (get<0>(finalcheckedvec[iter+1]) != get<0>(actpoint))&&
                    
                      (firstmflag == false) ) {
                     
                     mlast = mnew; 
                  
                     mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                             get<1>(actpoint)) / 
                            (get<0>(finalcheckedvec[iter+1]) - 
                             get<0>(actpoint));      
                        
                    }
                    
                    else {
                        
/* 
mlast ist the first m value
                       
*/                      
                      mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                              get<1>(actpoint)) / 
                             (get<0>(finalcheckedvec[iter+1]) - 
                              get<0>(actpoint));      
                             
                      firstmflag = false; 
                                            
                    }
                 
                 }
                 
                 break;}
            
        
        case 2:{ 
                
           
            if (! ((get<0>(actpoint) <= get<0>(finalcheckedvec[iter])) &&
                   (get<1>(actpoint) >= get<1>(finalcheckedvec[iter])) ) ||
                     mlast <= mnew ) {
                    
                   
                   okflag = false;    
                   break; }
                
                
               
                if ( (iter+1 <  finalcheckedvec.size() ) &&
                    
                    (get<0>(finalcheckedvec[iter+1]) != get<0>(actpoint)) &&
                     
                                         
                      (firstmflag == true) ) {
                    
                      actpoint = finalcheckedvec[iter];
                      
                      mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                              get<1>(actpoint)) /
                             (get<0>(finalcheckedvec[iter+1]) - 
                              get<0>(actpoint));   
                             
                      firstmflag = false;
                        
                    }  
                
                 else {
                    actpoint = finalcheckedvec[iter];
                    
                     mlast = mnew; 
                     
                     mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                             get<1>(actpoint)) / 
                            (get<0>(finalcheckedvec[iter+1]) - 
                             get<0>(actpoint));      
                        
                 }
                
                
                
                break;} 
        
         
        
        
        case 3:{
            
            
            
             if  (get<0>(actpoint) == get<0>(finalcheckedvec[iter])&&
                  (setfperflag == false) )  {
                 
                 secondperpend = true;
                                             
                
               }
         
         
              if (! ( (get<0>(actpoint) >= get<0>(finalcheckedvec[iter])) &&
                      (get<1>(actpoint) >= get<1>(finalcheckedvec[iter])) ) ||
                      (setfperflag == false && secondperpend == true) ||
                      mlast <= mnew)
              {
                
                 okflag = false;    
                 break; }
                 
                 
                 
                 if ((iter+1 < finalcheckedvec.size() ) &&                     
                     
                     (firstperpend == true) && 
                     (setfperflag == true) &&                    
                     
                     (get<0>(finalcheckedvec[iter+1]) != 
                      get<0>(finalcheckedvec[iter])))   {
                     
                   
                   actpoint = finalcheckedvec[iter];
                 
                   mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                           get<1>(actpoint)) / 
                          (get<0>(finalcheckedvec[iter+1]) - 
                           get<0>(actpoint));       
                          
                   mlast = mnew + 1;
                 
                   setfperflag = false;   
                    
                  }
                 
                 else {
                     
                   
                   actpoint = finalcheckedvec[iter];
                   
                  if ((iter+1 < finalcheckedvec.size()) &&                      
                      
                      (get<0>(finalcheckedvec[iter+1]) !=
                       get<0>(actpoint)) &&
                       
                      (firstmflag == false) ) {
                     
                     mlast = mnew; 
                  
                     mnew = (get<1>(finalcheckedvec[iter+1] ) -
                             get<1>(actpoint)) / 
                            (get<0>(finalcheckedvec[iter+1]) - 
                             get<0>(actpoint));      
                     
                    }
                    
                    else {
                        
/*
  mlast ist the first m value 
                       
*/                      
                      if  (get<0>(finalcheckedvec[iter+1]) != 
                           get<0>(actpoint)) {
                          
                      mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                              get<1>(actpoint)) / 
                            (get<0>(finalcheckedvec[iter+1]) - 
                             get<0>(actpoint));   
                      }
                      
                     
                      firstmflag = false; 
                                            
                    }
                 
                 }
                 
                 break;}
            
            
            
      
        
        case 4: {
            
            
            
           
            if (! ((get<0>(actpoint) >= get<0>(finalcheckedvec[iter])) &&
                   (get<1>(actpoint) <= get<1>(finalcheckedvec[iter])) ) ||
                     mlast <= mnew ) {
                    
                  
                   okflag = false;    
                   break; }
                
                
               
                if ( (iter+1 <  finalcheckedvec.size() ) &&
                    
                    (get<0>(finalcheckedvec[iter+1]) != get<0>(actpoint)) &&
                    
                     (firstmflag == true) ) {
                    
                      actpoint = finalcheckedvec[iter];
                     
                      mnew = (get<1>(finalcheckedvec[iter+1] ) -
                              get<1>(actpoint)) / 
                             (get<0>(finalcheckedvec[iter+1]) - 
                              get<0>(actpoint));  
                             
                      
                                 
                             
                      firstmflag = false;
                        
                    }  
                
                 else if (iter+1 <  finalcheckedvec.size())
                 
                 {
                    actpoint = finalcheckedvec[iter];
                    
                     mlast = mnew; 
                     
                     mnew = (get<1>(finalcheckedvec[iter+1] ) - 
                             get<1>(actpoint)) / 
                            (get<0>(finalcheckedvec[iter+1]) -
                             get<0>(actpoint));      
                        
                 }
                
                
                
                break;} 
        
   
   
       
       
   
  
    } 
/*
  end of switch 
     
*/     
   if  (okflag == false) break; 
   iter++; 
   
   }
   
/* 
end of while 
     
*/    
    finalcheckedvec.pop_back();
   
    src = finalcheckedvec;   
   
   

    
    
  if (okflag == true) 
      
      return 1;
      
      else 
          return 0;;
  
    
  
     

 } 
 
/* 
 the end of checkme 
  
*/


/*
 
  4.1   Compare and HasValue 
  
*/
int Convex::Compare(const Convex& arg) const {
   if(!IsDefined()){
      return arg.IsDefined()?-1:0;
   }
   if(!arg.IsDefined()){
      return 1;
   }
   if(size != arg.size){
     return size < arg.size ? -1 : 1;
   }
   for(size_t i=0;i<size;i++){
     if(value[i].GetX() < arg.value[i].GetX()) return -1;
     if(value[i].GetX() > arg.value[i].GetY()) return 1;
     if(value[i].GetX() == arg.value[i].GetX()
        &&
       (value[i].GetY() < arg.value[i].GetY())) return -1;
     
     
   }
   return 0;
}




size_t Convex::HashValue() const {
  if(!IsDefined() ) return 0;
  if(value==nullptr) return 42;

  unsigned int step = size / 2;
  if(step<1) step = 1;
  size_t pos = 0;
  size_t res = 0;
  while(pos < size){
    res = res + value[pos].HashValue();
    pos += step;
  }
  return res;
}



/*
 
  5.1 In functions
  
*/
Word Convex::In(const ListExpr typeInfo, const ListExpr le1,
                const int errorPos, ListExpr& errorInfo, bool& correct) {

   ListExpr le = le1; 
   ListExpr f, fxpoi, fypoi;
   std::vector<std::tuple<double, double>> tmpo;
   
   int checkokflag;
     
   string lexprstr;
    
    Word res = SetWord(Address(0));

    if(listutils::isSymbolUndefined(le)){
      
      
      Convex* co = new Convex(false);
      co->SetDefined(false);
      res.addr = co;      
      correct = true;
      return res;
   }
   
   
  
   
   if(nl->ListLength(le) <= 2){
     Convex* co = new Convex(false);
     correct = true;
       
      co->SetDefined(false);
      res.addr = co;      
     return res;
   }
   
            

   while(!nl->IsEmpty(le)){
     f = nl->First(le);
     le = nl->Rest(le);

    
     
     if(nl->ListLength(f) != 2) 
     
     {
               
      correct = true;
      
      nl->WriteToString(lexprstr, f);
      
      Convex* co = new Convex(false);
   
      co->SetDefined(false);
      res.addr = co;      
     return res;
      
     }    
     
     fxpoi = nl->First(f);
     fypoi = nl->Second(f);
     
      
      
     if ( ( nl->IsAtom(fxpoi) && nl->IsAtom(fypoi))       
         == false)
     
      {
      Convex* co = new Convex(false);
      correct = true;
        
      co->SetDefined(false);
      res.addr = co;      
      return res;
     }    
     
    
     if ( (  (nl->IsAtom(fxpoi) && nl->IsAtom(fypoi)) &&
          (  (nl->AtomType(fxpoi) == RealType) && 
             (nl->AtomType(fypoi) == RealType) ) 
        == false))
     
      {
          
      Convex* co = new Convex(false);
      correct = true;
 
      co->SetDefined(false);
      res.addr = co;      
      return res;
     }    
     
     
      
    
     
     
    
  /* 
   contructing the vektor of tuples  
   
  */   
   tmpo.push_back(std::make_tuple(nl->RealValue(fxpoi), nl->RealValue(fypoi)));
   
   
       
   } 
   
  /*
    end of while 
    
  */
   
   
   
  /*
  CHECKME Call 
    
  */   
   checkokflag = checkme(tmpo, true);
   
  
   
   if (checkokflag == 0) {
    
       
   
    ErrorReporter::ReportError(
                "1The coordinates do not form a convex polygon");  
   
   
   correct = false; 
   return res;
    
   
    }   
      
  else { 
   
   Convex* r = new Convex(tmpo);
   res.addr = r;
   correct = true; 
   //voroVec.push_back(r);
   return res;
   
   }
   
   
}
   
 
/*
 
  5.1 Out function
  
*/   
ListExpr Convex::Out(const ListExpr typeInfo, Word value) {
  
  Convex* is = (Convex*) value.addr;
  if(!is->IsDefined()){
     return listutils::getUndefined();
  }
  
 
    
  if(is->size == 0){
      
     return nl->TheEmptyList();
  }
  
  ListExpr res =  nl->OneElemList( 
                      nl->TwoElemList( nl->RealAtom(is->value[0].GetX()),
                                       nl->RealAtom(is->value[0].GetY())));
  

  
  ListExpr last = res;
  for(unsigned int i=1;i<is->size;i++){
    
    last = nl->Append( last,                       
                        nl->TwoElemList ( nl->RealAtom(is->value[i].GetX()),
                                          nl->RealAtom(is->value[i].GetY()) ) );


  }

  last = nl->Append(last, nl->OneElemList(nl->RealAtom(is->getCellId())));
  
  return res;
}



/*
 
  6.1 Open and save function
  
*/

bool Convex::Open( SmiRecord& valueRecord, size_t& offset, 
           const ListExpr typeInfo, Word& value){

  bool def;
  if(!valueRecord.Read(&def, sizeof(bool), offset)){
    return false;
  } 
  offset += sizeof(bool);
  if(!def){
     value.addr = new Convex(false);
     return true;
  }

  size_t size;  
  if(!valueRecord.Read(&size, sizeof(size_t), offset)){
    return false;
  } 

  offset+=sizeof(size_t);
  if(size==0){
    value.addr = new Convex(0,0,0);
    return true;
  } 
  Point* v = new Point[size];
  if(!valueRecord.Read(v,size*sizeof(Point),offset)){
    return false;
  }
  for(size_t i=0;i<size;i++){
     void* v1 = (void*) &(v[i]);
     new (v1) Point();
  }
  value.addr = new Convex(size,v,0);
  return true;
}




bool Convex::Save(SmiRecord& valueRecord, size_t& offset,
          const ListExpr typeInfo, Word& value) {

   Convex* is = (Convex*) value.addr;
   bool def = is->IsDefined();
   if(!valueRecord.Write(&def, sizeof(bool), offset)){
     return false;
   }
   offset += sizeof(bool);
   if(!def){
     return true;
   }
   size_t size = is->size;
   if(!valueRecord.Write(&size, sizeof(size_t), offset)){
     return false;
   }
   offset += sizeof(size_t);
   if(is->size>0){
      if(!valueRecord.Write(is->value, sizeof(Point) * is->size, offset)){
        return false;
      }
      offset += sizeof(int) * is->size;
   }
   return true;
}




void Convex::Rebuild(char* buffer, size_t sz) {
   if(value!=nullptr){
     delete[] value;
     value = nullptr;
   }
   size = 0;
   bool def;
   size_t offset = 0;
   memcpy(&def,buffer + offset, sizeof(bool));

   offset += sizeof(bool);
   if(!def){
      SetDefined(false);
      return;
   }       
   SetDefined(true);
   memcpy(&size, buffer+offset, sizeof(size_t));
   
   offset += sizeof(size_t);

   if(size > 0){
     value = new Point[size];

     memcpy((char*)value, buffer+offset, size * sizeof(Point));
     offset += size * sizeof(Point);
     
     }
   
   for(size_t i=0;i<size;i++){
         void* v = (void*) &(value[i]);
         new (v) Point();
     }
   
   
   
     memcpy(&cellId, buffer+offset, sizeof(int));

     offset += sizeof(int);
        
   
}



/*
 
  7.1 Auxillary print function 
  
*/
std::ostream& Convex::Print( std::ostream& os ) const {

    if(!IsDefined()){
       os << "undefined";
       return os;
    } 
    os << "{";
    for(size_t i=0;i<size;i++){
      if(i>0) os << ", ";
      os << value[i]; 
    }
    os << "}";
    return os;
}



/*

8 Operator Definitions

*/


/*

8.1 TypeMapping Definitions

*/


/* 
8.1.1 createconvextypemap
 
*/
ListExpr createconvextypemap( ListExpr args)
{ 
  
    
  if(!nl->HasLength(args,2))
   {
    return listutils::typeError("two  arguments expected");
   }
   
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  
  if(!Stream<Point>::checkType(arg1))
    
   {
    return listutils::typeError("first argument must be a stream of points");
   }
   
  
  
  if(!CcBool::checkType(arg2))
    
   {
    return listutils::typeError("second argument must be a Bool Value");
   }
   
  
  
  return nl->SymbolAtom(Convex::BasicType());
      
  
}





/* 
 8.1.2 voronoitypemap
 
*/
ListExpr voronoitypemap ( ListExpr args)

{   
    ListExpr extendconv, stream, namenewattr, attrtype;
    ListExpr second, third, secondname, thirdname;
    string secondnamestr, thirdnamestr;
    int posit1, posit2;
    
   
    if(nl->ListLength(args)!=5){
    ErrorReporter::ReportError("five args expected");
    return nl->TypeError();
  }
    
    
    
     if(nl->AtomType(nl->Second(args))!=SymbolType){
      return listutils::typeError("second arg does not represent a valid "
                                  "attribute name");
    }    
    
    
     if(nl->AtomType(nl->Third(args))!=SymbolType){
      return listutils::typeError("third arg does not represent a valid "
                                  "attribute name");
    }    
    
     
     
   if(!Rectangle<2>::checkType(nl->Fourth(args))){
    return listutils::typeError("fourth arg is not a rectangle");
    
   }
    
   if(!CcBool::checkType(nl->Fifth(args))){
    return listutils::typeError("fifth arg is not a Bool value");
    
   }
   
    second = nl->Second(args); 
    third = nl->Third(args);
    
    
    
   
    
    if ((nl->ListLength(second) != -1 ) 
       || (nl->ListLength(third)  != -1) ) {
  
  
    ErrorReporter::ReportError("two attribute name arguments expected");
    return nl->TypeError();
   }

    
    
    
    if(!IsStreamDescription(nl->First(args))){
    ErrorReporter::ReportError("first argument is not a tuple stream");
    return nl->TypeError();
  }

    
    stream = nl->First(args);
    namenewattr = nl->Third(args);
    
    
 /*
  copy attrlist to newattrlist
      
 */
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn = newAttrList;
  attrList = nl->Rest(attrList);
  
  
  while (!(nl->IsEmpty(attrList)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(attrList));
     attrList = nl->Rest(attrList);
  }

 /* 
   reset attrList 
   
 */
  attrList = nl->Second(nl->Second(stream));
  
  
  secondname =  second;
  secondnamestr = nl->SymbolValue(secondname);
  
  thirdname = third;
  thirdnamestr = nl->SymbolValue(thirdname);
  
  
  
  posit1 = FindAttribute(attrList,secondnamestr,attrtype);
  
  if(posit1==0){
       ErrorReporter::ReportError("Attribute "+ secondnamestr +
                                  " must be a member of the tuple");
       return nl->TypeError();
    }
  
  
  
  posit2 = FindAttribute(attrList,thirdnamestr,attrtype);
 if(posit2!=0){
       ErrorReporter::ReportError("Attribute "+ thirdnamestr +
                                  " is already a member of the tuple");
       return nl->TypeError();
    }
 
  
    
    
    
  extendconv = nl->SymbolAtom(Convex::BasicType());
  
  lastlistn = nl->Append(lastlistn, (nl->TwoElemList(namenewattr, extendconv)));
  
  
  
  return 
   
   nl->ThreeElemList(            
        nl->SymbolAtom(Symbol::APPEND()),
        nl->OneElemList(nl->IntAtom(posit1)),  
        nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                        nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                        newAttrList)));
  
    
}

/*
 8.1.3 cellNumber Typemapping

*/
ListExpr cellnumvoronoitypemap ( ListExpr args)
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if(Stream<Convex>::checkType(first) && CcInt::checkType(third) 
      && Rectangle<2>::checkType(second)) {
      return nl->SymbolAtom(collection::IntSet::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " Stream<Convex> x rect x int";

  return  listutils::typeError(errMsg);

}


/*

 8.1.4 smallest common cellnumber typemapping

*/
ListExpr sccvoronoitypemap (ListExpr args)
{
  if(nl->HasLength(args, 4)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);
    
    if(Stream<Convex>::checkType(first) && Rectangle<2>::checkType(second)
     && Rectangle<2>::checkType(third)
     && CcInt::checkType(fourth)) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }

  const std::string errMsg = "The following four arguments are expected:"
      " Stream<Convex> x rect x rect x int";

  return  listutils::typeError(errMsg);

}

/*

 8.1.5 getcell typemapping

*/
ListExpr getcellvoronoitypemap ( ListExpr args)
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if(Stream<Convex>::checkType(first)
      && CcInt::checkType(second)) {
      return nl->SymbolAtom(Rectangle<2>::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " Stream<Convex> x int";

  return  listutils::typeError(errMsg);

}

/* 
  8.1.6 voronoi3dtypemap
 
*/
ListExpr voronoi3dtypemap( ListExpr args )
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (Stream<Rectangle<3>>::checkType(first)
      && Rectangle<3>::checkType(second)) {
        return nl->SymbolAtom(Convex3D::BasicType());
      }
  } 
  
  const string errMsg = "The following two arguments are expected:"
      " stream(rect3) x rect3";

  return  listutils::typeError(errMsg);
}

/* 
  8.1.7 cellnum 3d typemapping
 
*/
ListExpr cellnumvoronoi3dtypemap ( ListExpr args)
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (Convex3D::checkType(first) && CcInt::checkType(third)
       && Rectangle<3>::checkType(second)) {
      return nl->SymbolAtom(collection::IntSet::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " Convex3D x rect3 x int";

  return  listutils::typeError(errMsg);

}


int voronoi3DCreateSelect( ListExpr args )
{
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args); 

  if (Stream<Rectangle<3>>::checkType(first) 
    && Rectangle<3>::checkType(second)) {
    return 0;
  }
  return -1;
}


/*
 8.1.8 smallest common cellnumber typemapping

*/
ListExpr sccvoronoi3dtypemap (ListExpr args)
{
  if(nl->HasLength(args, 4)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);
    
    if(Convex3D::checkType(first) 
    && Rectangle<3>::checkType(second) 
    && Rectangle<3>::checkType(third)
     && CcInt::checkType(fourth)) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " Convex3D x rect3 x rect3 x int";

  return  listutils::typeError(errMsg);

}

/*
 8.1.9 getcell 3d typemapping

*/
ListExpr getcellvoronoi3dtypemap ( ListExpr args)
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if(Convex3D::checkType(first) && CcInt::checkType(second)) {
      return nl->SymbolAtom(Rectangle<3>::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " Convex3D x int";

  return  listutils::typeError(errMsg);

}



/*
8.2 ValueMapping Definitions

*/


/*
8.2.1 cellNumVM

*/
int cellNumVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  if(voroVec.size() == 0) {
    Stream<Convex> input_convex(args[0]);
    input_convex.open();
    Convex* next = input_convex.request();

    while(next != 0){
      voroVec.push_back(*next);
      // free memory
      delete next;
      next = input_convex.request();
    }

    input_convex.close();
  }
  Rectangle<2> *search_window_ptr
    = static_cast<Rectangle<2>*>( args[1].addr );

  /* 
    mode == 1 => precise allocation; 
    mode == 2 => use Bboxes of convex cells
    In case the user did not specify 
    the mode of allocation, use Bboxes

  */
  int mode = ((CcInt*)args[2].addr)->GetIntval();
  std::set<int> cell_ids;
  int bbox = 2;
  int precise = 1;
  result = qp->ResultStorage(s);
  collection::IntSet* res = (collection::IntSet*) result.addr;

  if(search_window_ptr == nullptr) {
    return 1;
  }

  if(mode > bbox || mode < precise) {
    mode = bbox;
  }

  cellNum(search_window_ptr, mode, &cell_ids);
  res->setTo(cell_ids);

  
  if(voroVec.size() > 0) {
    voroVec.clear();
  }

  return 0;

}

/*
8.2.2 smallestCommonCellnumVM

*/
int smallestCommonCellnumVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  Stream<Convex> input_convex(args[0]);
  input_convex.open();
  Convex* next = input_convex.request();

  while(next != 0){
    voroVec.push_back(*next);
    delete next;
    next = input_convex.request();
  }

  input_convex.close();

  Rectangle<2> *search_window_ptr
    = static_cast<Rectangle<2>*>( args[1].addr );

  Rectangle<2> *search_window_ptr_2
    = static_cast<Rectangle<2>*>( args[2].addr );

  CcInt* cellno_ptr = static_cast<CcInt*>(args[3].addr);
  int cellno = cellno_ptr->GetIntval();

  if(search_window_ptr == nullptr 
    || search_window_ptr_2 == nullptr) {
    return -1;
  }

  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  bool boolval = false;

  std::set<int> intsetRect1;
  std::set<int> intsetRect2;


  cellNum(search_window_ptr, 1, &intsetRect1);
  if (intsetRect1.find(cellno) != intsetRect1.end()) 
  {

    cellNum(search_window_ptr_2, 1, &intsetRect2);


    std::vector<int> v(sizeof(intsetRect1)+ sizeof(intsetRect2));
    std::vector<int>::iterator it;

    it=std::set_intersection (intsetRect1.begin(), intsetRect1.end(),
                 intsetRect2.begin(), intsetRect2.end(), v.begin());
    v.resize(it-v.begin());                      
  
    if(v.empty()) { 
      //no intersection between rectangles
        res->Set( true, boolval);
        return 0;

    }
      
    if(v[0] == cellno)
    {
        boolval = true;
    }

    if(v.size() > 0)
    {
      v.clear();
    }
  }

    if(intsetRect1.size() > 0)
    {
      intsetRect1.clear();
    }
    if(intsetRect2.size() > 0)
    {
      intsetRect2.clear();
    }

    if(voroVec.size() > 0) 
    {
      voroVec.clear();
    }
  
      
    res->Set( true, boolval);

    return 0;

}


/*
8.2.3 getCellVM

*/
int getcellvoronoiVM(Word* args, Word& result, int message,
  Word& local, Supplier s)
{
  Stream<Convex> input_convex(args[0]);
  input_convex.open();
  Convex* next = input_convex.request();

  while(next != 0){
    voroVec.push_back(*next);
    delete next;
    next = input_convex.request();
  }

  input_convex.close();

  CcInt* cellno_ptr = static_cast<CcInt*>(args[1].addr);
  int cellno = cellno_ptr->GetIntval();

  if (voroVec.size() > 0 )
  {
    result = qp->ResultStorage( s );
    Rectangle<2> *res = (Rectangle<2>*) result.addr;

    for(size_t i = 0; i < voroVec.size(); i++)
    {
      if(voroVec[i].getCellId() == cellno)
      {
        double min[2], max[2];
        Rectangle<2> bbox = createBBox(&voroVec[i]);

        min[0] = bbox.getMinX();
        min[1] = bbox.getMinY();
        max[0] = bbox.getMaxX();
        max[1] = bbox.getMaxY();
        res->Set(true, min, max);
        return 0;
      }
    }
  }


  return -1;

}


/*

8.2.4  createconvexVM

*/
int createconvexVM (Word* args, Word& result,
                   int message, Word& local, Supplier s) 


{ 
 qp->DeleteResultStorage(s);
 qp->ReInitResultStorage(s);
 result = qp->ResultStorage(s);
 
 
 Stream<Point> stream(args[0]);
 Convex* res = static_cast<Convex*>(result.addr); 
 Point* elem; 
 vector<Point> points;
 std::vector<std::tuple<double, double>> temp;
 int checkgood;
 CcBool* checkonp = ((CcBool*)args[1].addr);
   
 if (!checkonp->IsDefined()) {
      res->SetDefined(false);
     return 0;
     
    }
 
 bool checkon = checkonp->GetBoolval(); 
  
 stream.open();
 
     
 
 while ( (elem = stream.request() ) ){
     
  
     
   if (!elem->IsDefined()) {
     res->SetDefined(false);
     return 0;
   }
   
  
        
/* 
contructing the vektor of tuples
    
*/  
   temp.push_back(std::make_tuple(elem->GetX(), elem->GetY()));
   
  }

  
 if (checkon == true) {
      
    
  checkgood = checkme(temp, true);  

 }

 else {
     
  checkgood = checkme(temp, false);
 
 }
 
 
 

if ((checkgood == 1) || (checkgood == 2)) {

 
 
 res -> setTo(temp, -1);; 
 
 stream.close(); 
 return 0;    
 }

 
 
 
 else {
 stream.close();
 return 0;    
 }
      
}    
    

    
/*

8.2.3  voronoiVM

*/   
struct voronoiInfo {
 

map<std::tuple<double, double>, 
std::vector<std::tuple<double, double>> > center2vorpoi;

   
TupleBuffer *rel;

GenericRelationIterator *reliter;

int attrcount;

int pointposition;

bool checkon;

int cellId;

};
   
     







    
int voronoiVM (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word tee;
  
  Tuple* tup;
        
  voronoiInfo*  localInfo = (voronoiInfo*) qp->GetLocal2(s).addr;
  
  std::vector<VoronoiPoint> voropoints; 
  
  voronoi_diagram<double> vorodiag;
   
  double xval, yval;

  

  
  
  
switch (message)
  
{
/*
voronoiVM-open 
        
*/   
      case OPEN: {         
           
       localInfo = new voronoiInfo;
       qp->GetLocal2(s).addr = localInfo;    
          
       ListExpr resultType = GetTupleResultType(s);
       TupleType *tupleType = new TupleType(nl->Second(resultType));
       local.addr = tupleType;
    
       ListExpr attrsave = qp->GetType(qp->GetSon(s,0));
       ListExpr attrsave2 = nl->Second(nl->Second(attrsave));
       
       
       int counter = 0; 
       int counttup = 0;
             
       localInfo->reliter = 0;
       
              
       int pointpos = ((CcInt*)args[5].addr)->GetIntval();     
       Rectangle<2>* rectme = (Rectangle<2>*) args[3].addr;
       
      double belowleftx,belowlefty, belowrightx, belowrighty, aboveleftx, 
             abovelefty, aboverightx, aboverighty, miny, minx, maxx, maxy;
             
             
      double scale = 50;
      double stretchx = 0; 
      double stretchy = 0;
      double diaglenght;
      double testpointstretch;
      
      bool scalereset = false;
        
       
       std::tuple<double, double> center;
       std::tuple<double, double> testcenter;
       std::vector<std::tuple<double, double>> voropoi;
       std::vector<std::tuple<double, double>> allpoints;
       std::vector<std::tuple<double, double>> infedgecenter;      
       std::vector<std::tuple<double, double>> voropoitemp;
       std::vector<std::tuple<double, double>> infedgepoints;   
       std::vector<std::tuple<double, double>> normaledgepoints;   
                         
          
          
       
       map<std::tuple<double, double>, 
           std::tuple<double, double>>  infcenter2firstinfedgepoi;
          
          
       map<std::tuple<double, double>, 
           std::tuple<double, double>>  infcenter2secondinfedgepoi;
           
       
       map<std::tuple<double, double>, 
           std::vector<std::tuple<double, double>> > infedgepoi2lineparam;
           
           
       map<std::tuple<double, double>, 
           std::vector<std::tuple<double, double>> > infcenter2neighborcenters; 
           
           
      map<std::tuple<double, double>, 
           std::vector<std::tuple<double, double>>>
                    normaledgepoints2normalcenters; 
           
               
       map<std::tuple<double, double>, 
           std::vector<std::tuple<double, double>> > infedgepoints2centers;   
           
           
       std::tuple<double, double>  aboveleft, aboveright, belowleft, belowright;
       std::tuple<double, double> testpoint1, testpoint2;
       std::tuple<double, double> newpoint1, newpoint2;
       
       
       map<std::tuple<double, double>,  
       std::vector<std::tuple<double, double>>>::
       iterator iterone = normaledgepoints2normalcenters.begin();   
   
       
             
       map<std::tuple<double, double>,  
       std::vector<std::tuple<double, double>>>::
       iterator itedge = infedgepoints2centers.begin();   
       
       map<std::tuple<double, double>,  
       std::vector<std::tuple<double, double>>>::
       iterator itlineparam = infedgepoi2lineparam.begin();   
       
    
   
   
       
/*
  storing pointpos in localInfo
    
*/   
   localInfo->pointposition = pointpos;
   
   
   if (!((CcBool*)args[4].addr)->IsDefined()) {
       localInfo->checkon = true;
         
    }
   
   else {
   
/*
storing bool in localInfo
     
*/
     localInfo->checkon = ((CcBool*)args[4].addr)->GetBoolval(); 
   }
       
  
  
  
  while (!(nl->IsEmpty(attrsave2)))
     {
     
     attrsave2 = nl->Rest(attrsave2);
     
/*
counting the attributes
      
*/
     counter++;
     }
     
/*
  Init localInfo 
      
*/
     localInfo->attrcount = counter;     
     
           
    
     qp->Open(args[0].addr);
     qp->Request(args[0].addr, tee);
     

     
     if(qp->Received(args[0].addr))
      {
        localInfo->rel = new TupleBuffer( );
      }
      else
      {
        localInfo->rel = 0;
      }
      
      
   while(qp->Received(args[0].addr))
      { 
 
/*
  storing tuples in localInfo
         
*/
        Tuple *tup2 = (Tuple*)tee.addr;
        localInfo->rel->AppendTuple( tup2 );
        
        counttup++;
        
 /*
 Setting up the VoroPoint vector 
         
 */
     Point* pointval = static_cast<Point*>(tup2->GetAttribute(pointpos-1));
        
     if(pointval->IsDefined()) {
         
        xval = pointval->GetX(); 
        yval = pointval->GetY();
        voropoints.push_back(VoronoiPoint(xval, yval));
        allpoints.push_back(std::make_tuple(xval, yval));
        
       }
        
        tup2->DeleteIfAllowed();
        qp->Request(args[0].addr, tee);
      }

   
    
        
  
     if( localInfo->rel)
      {
        localInfo->reliter = localInfo->rel->MakeScan();
      }
      else
      {
        localInfo->reliter = 0;
      }
       
       
       
 /*
 constructing the voronoi diagramm using the boost library
      
 */
     construct_voronoi(voropoints.begin(), voropoints.end(), &vorodiag);
     
     
         
     
    int contzell = 0;   
  
    unsigned int cell_index = 0;
    for (voronoi_diagram<double>::const_cell_iterator it =
         vorodiag.cells().begin();
         it != vorodiag.cells().end(); ++it) {
        
        contzell++;  
              
      if (it->contains_point()) {
          
        if (it->source_category() ==
            boost::polygon::SOURCE_CATEGORY_SINGLE_POINT) {
          std::size_t index = it->source_index();
          VoronoiPoint p = voropoints[index];

        
                         
          center =  std::make_tuple(x(p), y(p));
          voropoitemp.push_back ( std::make_tuple(x(p), y(p)));
        
          
          const voronoi_diagram<double>::cell_type& cell = *it;
          const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();


  /*
    iterate edges around voronoi cell
     
  */  
  do {
      if (true) {

      
      

      if( (edge->vertex0() != NULL) && 
          (edge->vertex1() != NULL))  {
          
          
       std::tuple<double, double>  
       edgepoitemp1 = std::make_tuple((edge->vertex0())->x(),
                                      (edge->vertex0())->y());
        
        
       std::tuple<double, double> 
       edgepoitemp2  = std::make_tuple((edge->vertex1())->x(),
                                       (edge->vertex1())->y());
       
       
        
       voropoi.push_back(edgepoitemp1);
       voropoi.push_back(edgepoitemp2);
 
       allpoints.push_back(edgepoitemp1);
       allpoints.push_back(edgepoitemp2);
       
       
       std::tuple<double, double> edgecentertemp = std::make_tuple(x(p), y(p));
       
       std::vector<std::tuple<double, double>> vectemp;
       
                
        
       vectemp.push_back (edgecentertemp);
       
      
   
       normaledgepoints.push_back(edgepoitemp1);
       normaledgepoints.push_back(edgepoitemp2);
       
        iterone = normaledgepoints2normalcenters.find(edgepoitemp1);
      
        
        
   /*
     edgepoint not set yet
         
   */
        if (iterone == normaledgepoints2normalcenters.end())   {
            
                    
        normaledgepoints2normalcenters.insert
        (pair<std::tuple<double, double>, 
        std::vector<std::tuple<double, double>>>
        (edgepoitemp1, vectemp));
        
                    
            
          }
             
   /* 
    edgepoint entry exists already
         
   */        
           else {
               
           normaledgepoints2normalcenters.at(edgepoitemp1).push_back(
               edgecentertemp);
           
           rmvredundpoints(normaledgepoints2normalcenters.at(edgepoitemp1));
               
                  }
        
        
        
        iterone = normaledgepoints2normalcenters.find(edgepoitemp2);
      
   /*
     edgepoint not set yet
          
   */
        if (iterone == normaledgepoints2normalcenters.end())   {
            
                    
        normaledgepoints2normalcenters.insert
        (pair<std::tuple<double, double>,
        std::vector<std::tuple<double, double>>>
        (edgepoitemp2, vectemp));
        
                    
            
          }
             
   /*
     edgepoint entry exists already
         
    */        
           else {
               
           normaledgepoints2normalcenters.at(edgepoitemp2).push_back
           (edgecentertemp);
           
           rmvredundpoints(normaledgepoints2normalcenters.at(edgepoitemp2));
               
                  }
        
        
        
        
       
      }


      if ( (edge->vertex0() != NULL) && 
           (edge->vertex1() == NULL))  {
       
       std::tuple<double, double> 
       edgepoitemp4 = std::make_tuple((edge->vertex0())->x(),
                                      (edge->vertex0())->y());
       
       
       std::vector<std::tuple<double, double>> vectemp3;
     
       std::tuple<double, double>  twincenter; 
      
      
      allpoints.push_back(edgepoitemp4);
       
       infedgecenter.push_back(center);
       
       infedgepoints.push_back(edgepoitemp4);
       
       voropoi.push_back(edgepoitemp4);
       
             
       
        
       if ( ((edge->twin())->cell())->contains_point()) {
          
        if (((edge->twin())->cell())->source_category() ==
            boost::polygon::SOURCE_CATEGORY_SINGLE_POINT) {
          std::size_t index = ((edge->twin())->cell())->source_index();
          VoronoiPoint q = voropoints[index];
          
          twincenter = std::make_tuple(x(q), y(q));
          
        
        }
       }
       
       
       
          
        
        
        
        itedge = infedgepoints2centers.find(edgepoitemp4);
      
        
        
   /*
     edgepoint not set yet
         
   */
        if (itedge == infedgepoints2centers.end())   {
         
         vectemp3.push_back(center);   
         vectemp3.push_back(twincenter);   
         
         
         
        infedgepoints2centers.insert
        (pair<std::tuple<double, double>, 
        std::vector<std::tuple<double, double>>>
        (edgepoitemp4, vectemp3));
        
                    
            
          }
             
   /*
     edgepoint entry exists already
         
   */        
           else {
               
           infedgepoints2centers.at(edgepoitemp4).push_back(center);
           infedgepoints2centers.at(edgepoitemp4).push_back(twincenter);
           
               
                  }
        
     
               
           
       }

 
      if( (edge->vertex1() != NULL) && 
          (edge->vertex0() == NULL))  {

     
          
          
         allpoints.push_back(
             std::make_tuple((edge->vertex1())->x(), 
                             (edge->vertex1())->y()));
               
         infedgecenter.push_back(std::make_tuple(x(p), y(p)));
         
         
         infedgepoints.push_back(
             std::make_tuple((edge->vertex1())->x(), 
                             (edge->vertex1())->y()));
         
         
         
         voropoi.push_back(std::make_tuple((edge->vertex1())->x(), 
                             (edge->vertex1())->y()));
         
         
      }


      

      }
   
      edge = edge->next();
     
     


    } while (edge != cell.incident_edge());

  

    }



      ++cell_index;
    }
    
   
   rmvredundpoints(voropoi);
   
   
   
   
   localInfo->center2vorpoi.insert
   (pair<std::tuple<double, double>, 
   std::vector<std::tuple<double, double>>>
   (center, voropoi));
   
     voropoi.clear();    
 /*
   cleared for the next cell

 */     
    }  
   
  
 /*
   setting up border handling of the voronoi cells  
      
 */  
     
     
 /*
   remove redundant points
      
 */   
    rmvredundpoints(allpoints);
      
    rmvredundpoints(infedgecenter);
        
    rmvredundpoints(infedgepoints);
    
    
    
     
    
    
     
 /*
     compute minx, maxx, miny and maxy from allpoints
     
 */   
    sort(allpoints.begin(), allpoints.end(), sortyupxup); 
    
    miny = get<1>(allpoints[0]);
    maxy = get<1>(allpoints[allpoints.size()-1]);
    
    
    sort(allpoints.begin(), allpoints.end(), sortxupyup); 
      
    minx = get<0>(allpoints[0]);
    maxx = get<0>(allpoints[allpoints.size()-1]);
    
   
    
    
 /* 
   Voronoi Border handling
   
 */   
   
    
 /*
    testing if box is fitting and rectangle value defined
   
 */ 
    
   if ( (rectme->IsDefined()) &&  
       
       ( (rectme->MinD(0) < minx) && (rectme->MaxD(0) > maxx) &&
         (rectme->MinD(1) < miny) && (rectme->MaxD(1) > maxy) )  ) {
       
     
 /*
   receiving box points from rectangle
   
 */           
     belowleftx = rectme->MinD(0);
     belowlefty = rectme->MinD(1);
   
     belowrightx = rectme->MaxD(0);
     belowrighty = rectme->MinD(1);
   
     aboveleftx = rectme->MinD(0);
     abovelefty = rectme->MaxD(1);
   
     aboverightx = rectme->MaxD(0);
     aboverighty = rectme->MaxD(1);
     
     
   }
     
/*
  use the dafault box if box is not fitting  or 
  if rectangle value is undefined 
        
*/      
    else {
        
     stretchx = (abs(maxx - minx)) / 4;
     stretchy = (abs(maxy - miny)) / 4;
        
        
     belowleftx = minx - stretchx;
     belowlefty = miny - stretchy;
     
     belowrightx = maxx + stretchx;
     belowrighty = miny - stretchy;
   
     aboveleftx = minx - stretchx;
     abovelefty = maxy + stretchy;
   
     aboverightx = maxx + stretchx; 
     aboverighty = maxy + stretchy;
        
           
    }
      
      
   
    
    
 /* 
   construct box points
   
 */  
  aboveleft = std::make_tuple(aboveleftx, abovelefty);
  aboveright = std::make_tuple(aboverightx, aboverighty);
  belowright = std::make_tuple(belowrightx, belowrighty);
  belowleft = std::make_tuple(belowleftx, belowlefty);
  
  
  
 diaglenght = sqrt (pow (get<0>(belowleft) - get<0>(aboveright), 2) +
                    pow (get<1>(belowleft) - get<1>(aboveright), 2));
 
               
                  
  
          
    
 /* 
   constructing a point and the corresponding 
   straight line equation for infinite edges
    
 */   
    for (unsigned int i = 0; i <  infedgepoints.size(); i++)  {
        
              
        itedge = infedgepoints2centers.find(infedgepoints[i]);
        
      if (itedge != infedgepoints2centers.end()) { 
      
      std::vector<std::tuple<double, double>> pairvec =
          infedgepoints2centers.at(infedgepoints[i]);
      
      if ((pairvec.size() % 2) == 0) {
          
        
      for (unsigned int j = 0; j < pairvec.size(); j=j+2)   {
          
                
         
        std::tuple<double, double> infovec5 = pairvec[j];
        
                               
        std::tuple<double, double> infovec6 = pairvec[j+1];
        
        
                   
        
        double xfirst = get<0>(infovec5);
        double xsecond = get<0>(infovec6);
        double yfirst = get<1>(infovec5);
        double ysecond = get<1>(infovec6);
         
          
/*
  The result point lies on the corresponding 
  straight line equation of the infinite edge
         
*/        
        double resultx = (xfirst + xsecond) / 2;
        double resulty = (yfirst + ysecond) / 2;
        
        
     
              
        if ( !(xfirst == xsecond) &&
             !(AlmostEqual(xfirst, xsecond)) &&
             
            !(yfirst == ysecond) &&
            !(AlmostEqual(yfirst, ysecond)) )  {      
          
/*
   m and b are the straight line equation parameters for the line
   that goes through the 2 center points. 
   We only need the gradient m here.   
   
*/ 

  double  m = (ysecond - yfirst) / (xsecond - xfirst);
    
       
        
/*
   mm and bb are the straight line equation parameters  for the line
   that goes through the infinte edge between the 2 centers
   The two straight lines are always perpendicular to each other, 
   so you can determine  mm from m.  
         
*/              
       double mm = - 1/m;
       double bb = resulty - (mm * resultx);
        
        
       
         
        
        std::vector<std::tuple<double, double>> paramvec; 
         std::tuple<double, double> param = std::make_tuple(mm ,bb);
        
        itlineparam = infedgepoi2lineparam.find(infedgepoints[i]);

        
/*
   edgepointto param not set
        
*/
        if (itlineparam == infedgepoi2lineparam.end())   {
         
         paramvec.push_back(param);   
          
                    
        infedgepoi2lineparam.insert
        (pair<std::tuple<double, double>, 
        std::vector<std::tuple<double, double>>>
        (infedgepoints[i], paramvec));
        
                    
            
          }
             
/*
  edgepoint entry exists already
         
*/        
           else {
               
           infedgepoi2lineparam.at(infedgepoints[i]).push_back(param);
           
               
                  }
        
     
               
        
        
        
        
        
         
        
        }   
/*
   the 2 centers form a straight line with gradient 0 
             
*/                      
         else if ( (yfirst == ysecond) || (AlmostEqual(yfirst, ysecond)) ) {
         
        
         double dummy = std::numeric_limits<double>::infinity();
         
         
         
         std::tuple<double, double> dummytup = std::make_tuple(dummy ,dummy);
          std::vector<std::tuple<double, double>> paramvec2; 
             
/*
  just pushing an infinty value for identification purposes
          
*/         
        itlineparam = infedgepoi2lineparam.find(infedgepoints[i]);

        
/*
   edgepointto param not set
          
*/
        if (itlineparam == infedgepoi2lineparam.end())   {
         
         paramvec2.push_back(dummytup);   
          
                    
        infedgepoi2lineparam.insert
        (pair<std::tuple<double, double>, 
        std::vector<std::tuple<double, double>>>
        (infedgepoints[i], paramvec2));
        
                    
            
          }
             
/*
  edgepoint entry exists already
         
*/        
           else {
               
           infedgepoi2lineparam.at(infedgepoints[i]).push_back(dummytup);
           
               
                  }
        
     
               
             
             
        }
         
         else {
         
                         
/*
 the two centers are perpendicular
          
*/         
         double mmm = 0;
         double bbb = resulty - (mmm * resultx);
         
         
                  
          std::vector<std::tuple<double, double>> paramvec3; 
         std::tuple<double, double> para = std::make_tuple(mmm ,bbb);
         
         itlineparam = infedgepoi2lineparam.find(infedgepoints[i]);
         
         
/* 
 edgepointto param not set
           
*/       
         if (itlineparam == infedgepoi2lineparam.end())   {
         
         paramvec3.push_back(para);   
          
                    
                     
        infedgepoi2lineparam.insert
        (pair<std::tuple<double, double>,
        std::vector<std::tuple<double, double>>>
        (infedgepoints[i], paramvec3));
        
        
                    
            
          }
             
/* 
 edgepoint entry exists already
         
*/        
           else {
               
           infedgepoi2lineparam.at(infedgepoints[i]).push_back(para);
           
               
            }
        
                
           
               
           }
      
          
         
         }
/*
   end of second for
         
*/             
       }
 /* 
   nothing has to be done here 
        
 */        
       }
       
 /* 
    no entry found, should not happen 
         
 */
        
    }
    
/* 
  end of first for
        
*/ 
        
        
    
/* 
 Calculating points on the border lines 
 and the correpondending cells
       
*/  
    for (unsigned int i = 0; i <  infedgepoints.size(); i++)  {
       
               
        
       itlineparam = infedgepoi2lineparam.find(infedgepoints[i]);
        
      if (itlineparam != infedgepoi2lineparam.end()) { 
      
      std::vector<std::tuple<double, double>> paramvector =
          infedgepoi2lineparam.at(infedgepoints[i]);
      
      
        
      for (unsigned int j = 0; j <  paramvector.size(); j++)   {
          
        
                
        std::tuple<double, double> lineinfo = 
        paramvector[j];
        
        double m = get<0>(lineinfo);
        double t = get<1>(lineinfo);
        
        
            
       std::vector<std::tuple<double, double>> vec =
           infedgepoints2centers.at(infedgepoints[i]);
           
/* 
 get center coords
       
*/      
        std::tuple<double, double> center1 = vec[2*j];
        
        std::tuple<double, double> center2 = vec[(2*j) + 1];       
               
        double xfirst = get<0>(center1);
        double xsecond = get<0>(center2);
        double yfirst = get<1>(center1);
        double ysecond = get<1>(center2);
         
             
        double resultx = (xfirst + xsecond) / 2;
        double resulty = (yfirst + ysecond) / 2;
        
        
        
        std::tuple<double, double> testp1 =  std::make_tuple(resultx, resulty);
        
        std::tuple<double, double> testp2 = infedgepoints[i];
        std::tuple<double, double> testpfinal =  
        std::make_tuple ((resultx + get<0>(testp2)) / 2,
                         (resulty + get<1>(testp2)) / 2);
        
        
        
       
 /*
   normaledgepoints2normalcenters output
        
 */        
    iterone = normaledgepoints2normalcenters.find(infedgepoints[i]); 
    
   if (iterone !=  normaledgepoints2normalcenters.end()) {
    
    std::vector<std::tuple<double, double>> centerinfovec =
    normaledgepoints2normalcenters.at(infedgepoints[i]);
        
    for (unsigned int j = 0; j < centerinfovec.size(); j++) {
        
        if ((centerinfovec[j] != center1) &&  (centerinfovec[j] != center2)) {
            
             std::tuple<double, double> testc = centerinfovec[j];
           
             testcenter = testc;
             
            
                
         }
     
     }
            
  }    
  
  

  bool cond1 = ( ((get<0>(testcenter) >= get<0>(testp1)) && 
                 (get<0>(testcenter) <= get<0>(testpfinal)) &&      
                 (get<0>(testp1) <= get<0>(testpfinal))) ||
       
       ((get<0>(testcenter) <= get<0>(testp1)) && 
       (get<0>(testcenter) >= get<0>(testpfinal)) &&      
       (get<0>(testp1) >= get<0>(testpfinal))) ||
       
       ((get<1>(testcenter) >= get<1>(testp1)) &&
       (get<1>(testcenter) <= get<1>(testpfinal)) &&      
       (get<1>(testp1) <= get<1>(testpfinal))) ||
       
       ((get<1>(testcenter) <= get<1>(testp1)) &&
       (get<1>(testcenter) >= get<1>(testpfinal)) &&      
       (get<1>(testp1) >= get<1>(testpfinal))) ); 
  
  
 /*
   change testpfinal if necessary 
   
 */   
  if (cond1 == true) {
                  
      testpfinal = testp1;
      scale = 200;
      scalereset = true;
      
     }
       
       
  if ( ((cond1 != true) && (get<0>(testcenter) > get<0>(testp1)) && 
       (get<1>(testcenter) > get<1>(testp1))) ||
        
       ((cond1 != true) && (get<0>(testcenter) < get<0>(testp1)) && 
       (get<1>(testcenter) < get<1>(testp1))) ) {
       
       testpfinal = testp1;
       scale = 30;
       scalereset = true;     
       }
        
             
        testpointstretch = diaglenght/scale;
       
       
        if (scalereset == true) scale = 50;
        
        
        
        if (isinf(m) && isinf(t)) {
            
         testpoint1 =  std::make_tuple(get<0>( testpfinal),
                                       get<1>( testpfinal) + 
                                       testpointstretch);
         
         testpoint2 =  std::make_tuple(get<0>( testpfinal), 
                                       get<1>( testpfinal) - 
                                       testpointstretch);
        
        }
        
        
        else if (m==0) {
           
                testpoint1 =  std::make_tuple(get<0>( testpfinal) + 
                              testpointstretch, get<1>( testpfinal));
                
                testpoint2 =  std::make_tuple(get<0>( testpfinal) - 
                              testpointstretch, get<1>( testpfinal));
           }
        
          else {
              
              testpoint1 =  std::make_tuple(get<0>( testpfinal) + 
                                            testpointstretch, 
                                            m * (get<0>( testpfinal) + 
                                            testpointstretch) + t);
              
              testpoint2 =  std::make_tuple(get<0>( testpfinal) - 
                                            testpointstretch, 
                                            m * (get<0>( testpfinal) - 
                                            testpointstretch) + t); 
              
               }
       
        
        
/*
  Euklidien distance to  testcenter
         
*/        
        double distinfedgpoitotestcenter = sqrt 
                                           (pow (get<0>(testpfinal) - 
                                            get<0>(testcenter), 2) +
                                            pow (get<1>(testpfinal) - 
                                            get<1>(testcenter), 2));
        
       
              
        
         double distinfedgpoitotestpoint1 = sqrt
                                            (pow (get<0>(testpoint1) - 
                                            get<0>(testcenter), 2) +
                                            pow (get<1>(testpoint1) - 
                                            get<1>(testcenter), 2));
        
        
         double distinfedgpoitotestpoint2 = sqrt 
                                            (pow (get<0>(testpoint2) - 
                                            get<0>(testcenter), 2) +
                                            pow (get<1>(testpoint2) - 
                                            get<1>(testcenter), 2));
        
           
               
        
              
        
/*
  looking for points added to center voropoints
         
*/        
      if ( (distinfedgpoitotestpoint1 >  distinfedgpoitotestcenter)  && 
           ( (distinfedgpoitotestpoint2 <=  distinfedgpoitotestcenter) ||
             (distinfedgpoitotestpoint1 >  distinfedgpoitotestpoint2)))   {
          
         
          if (isinf(m) && isinf(t)) {
             
             newpoint1 =  std::make_tuple(get<0>(infedgepoints[i]), 
                                          aboverighty);
           }
           
           
            else if (m==0) {
                
                
                 newpoint1 =  std::make_tuple(belowrightx,
                                              get<1>(infedgepoints[i]));
                        
             }
            
/*
 case m<0
               
*/              
              else if (m<0)   {
                  
                   
                    std::tuple<double, double> newpoint1cand1;
                    std::tuple<double, double> newpoint1cand2;
                    double newxval = (belowrighty - t) / m;
                    double newyval = (m * belowrightx) + t;
                     
                      newpoint1cand1 = std::make_tuple(belowrightx, newyval);
                      newpoint1cand2 = std::make_tuple(newxval, belowrighty);
                      
                      
                      if ( ((belowrightx ==  newxval) && 
                            (belowrighty == newyval)) ||
                           (AlmostEqual (belowrightx, newxval) && 
                            AlmostEqual (belowrighty, newyval)) ) {
                          
                          newpoint1 = std::make_tuple (belowrightx,
                                                       belowrighty);
                                          
                       }
                       
                      
                      if ((newxval > belowrightx) && (newyval > belowrighty) &&
                          (newyval < aboverighty) ) {
                          
                          newpoint1 =  newpoint1cand1;
                                      
                         }
                         
                         
                       else if   ((newyval < belowrighty) && 
                                  (newxval < belowrightx) &&
                                  (newxval > belowleftx) ) {
                          
                                    newpoint1 =  newpoint1cand2;
                                      
                                     }
                                     
                                     
                 }              
                                     
                   
/* 
case m>0 
                 
*/          
                else 
                    
                {            
                    std::tuple<double, double> newpoint1cand1;
                    std::tuple<double, double> newpoint1cand2;
                    double newxval = (aboverighty - t) / m;
                    double newyval = (m * aboverightx) + t;
                     
                      newpoint1cand1 = std::make_tuple(aboverightx, newyval);
                      newpoint1cand2 = std::make_tuple(newxval, aboverighty);
                      
                      
                      if ( ((aboverightx ==  newxval) && 
                            (aboverighty == newyval)) ||
                           (AlmostEqual (aboverightx, newxval) && 
                            AlmostEqual (aboverighty, newyval)) ) {
                          
                          newpoint1 = std::make_tuple (aboverightx, 
                                                       aboverighty);
                                          
                       }
                       
                      
                      if ((newxval > aboverightx) && 
                          (newyval > belowrighty) &&
                          (newyval < aboverighty) ) {
                          
                          newpoint1 =  newpoint1cand1;
                                      
                         }
                         
                         
                       else if   ((newyval > aboverighty) && 
                                  (newxval < belowrightx) &&
                                  (newxval > belowleftx) ) {
                          
                                    newpoint1 =  newpoint1cand2;
                                      
                                     }
                                     
                        
                }
                  
                 
            
         localInfo->center2vorpoi.at(center1).push_back (newpoint1);
         localInfo->center2vorpoi.at(center2).push_back (newpoint1);
         
          
         
         }   
         
         
         
         
         
/*
 case m=0 
                 
*/
       else if  ( (distinfedgpoitotestpoint2 >  distinfedgpoitotestcenter)  && 
           ( (distinfedgpoitotestpoint1 <=  distinfedgpoitotestcenter) ||
             (distinfedgpoitotestpoint2 >  distinfedgpoitotestpoint1)))    {
            
                            
               
               if (isinf(m) && isinf(t)) {
                  
                  newpoint2 =  std::make_tuple(get<0>(infedgepoints[i]), 
                                               belowrighty);
                }
                
               
                 else if (m==0) {                
                       
                       newpoint2 =  std::make_tuple(belowleftx, 
                                                    get<1>(infedgepoints[i]));
                        
                     }
/* 
 case m<0 
                 
*/           
                    else if (m<0) {                        
                        
                      std::tuple<double, double> newpoint1cand1;
                      std::tuple<double, double> newpoint1cand2;
                      double newxval = (abovelefty - t) / m;
                      double newyval = (m * aboveleftx) + t;
                     
                      newpoint1cand1 = std::make_tuple(aboveleftx, newyval);
                      newpoint1cand2 = std::make_tuple(newxval, abovelefty);
                      
                      
                      if ( ((aboveleftx ==  newxval) && 
                            (abovelefty == newyval)) ||
                           (AlmostEqual (aboveleftx, newxval) && 
                            AlmostEqual (abovelefty, newyval)) ) {
                          
                          newpoint2 = std::make_tuple (aboveleftx, abovelefty);
                                          
                       }
                       
                      
                      if ((newxval < aboveleftx) && (newyval > belowlefty) &&
                          (newyval < abovelefty) ) {
                          
                          newpoint2 =  newpoint1cand1;
                                      
                         }
                         
                         
                       else if   ((newyval > abovelefty) && 
                                  (newxval < belowrightx) &&
                                  (newxval > belowleftx) ) {
                          
                                    newpoint2 =  newpoint1cand2;
                                      
                                     } 
                        
                        
                        

                        
                        
                        
                       
                        
                    }
                
/* 
  case m>0 
                 
*/                
                else {
                   
                    std::tuple<double, double> newpoint1cand1;
                    std::tuple<double, double> newpoint1cand2;
                    double newxval = (belowlefty - t) / m;
                    double newyval = (m * belowleftx) + t;
                     
                      newpoint1cand1 = std::make_tuple(belowleftx, newyval);
                      newpoint1cand2 = std::make_tuple(newxval, belowlefty);
                      
                      
                      if ( ((belowleftx ==  newxval) && 
                           (belowlefty == newyval)) ||
                           (AlmostEqual (belowleftx, newxval) && 
                            AlmostEqual (belowlefty, newyval)) ) {
                          
                          newpoint2 = std::make_tuple (belowleftx, belowlefty);
                                          
                       }
                       
                      
                      if ((newxval < belowleftx) && 
                          (newyval > belowlefty) &&
                          (newyval < abovelefty) ) {
                          
                          newpoint2 =  newpoint1cand1;
                                      
                         }
                         
                         
                       else if   ((newyval < belowlefty) && 
                                  (newxval < belowrightx) &&
                                  (newxval > belowleftx) ) {
                          
                                    newpoint2 =  newpoint1cand2;
                                      
                                     }
                  }
                
                
            
         localInfo->center2vorpoi.at(center1).push_back (newpoint2);
         localInfo->center2vorpoi.at(center2).push_back (newpoint2);
                
                
                        
           }
                
                
/*
   nothing is done here 
       
*/           
      }
      
/*
 end of second for
      
*/
          
    } 
/*
 end of first if
    
*/   
    } 
        
/*
  end of for
      
*/
             
      
/* 
 distribute the 4 box edge points to the right voronoicells
     
*/   
    for (unsigned int i = 0; i < infedgecenter.size(); i++)  {
         int countymaxvals = 0; 
         int countyminvals = 0;
         int countxminvals = 0;
         int countxmaxvals = 0;
        
        
         std::vector<std::tuple<double, double>> infovec =
         localInfo->center2vorpoi.at(infedgecenter[i]);
         
         
         
/* 
   counting the cases 
         
*/      
       for (unsigned int j = 0; j < infovec.size(); j++) {
       
       
       
         if (get<0>(infovec[j]) == belowleftx) countxminvals++;
         
         if (get<0>(infovec[j]) == belowrightx) countxmaxvals++;      
         
         if (get<1>(infovec[j]) == belowlefty) countyminvals++;
         
         if (get<1>(infovec[j]) == abovelefty) countymaxvals++;    
        
            
         }
         
        
/* 
  distribute the new points 
         
*/   
         
         
         if ((countxminvals == 1) && (countyminvals == 1)) {
             
          localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                      (belowleft);
          
         }
          
         else if ((countxminvals == 1) && (countymaxvals == 1)) {
                        
             localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                         (aboveleft);
             
              }
         
         else if ((countxmaxvals == 1) && (countymaxvals == 1)) {
                
              localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                         (aboveright);         
             
               }
             
         
         else if ((countxmaxvals == 1) && (countyminvals == 1)) {
             
               localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (belowright);
             
                }
         
         
         else if ((countyminvals == 1) && (countymaxvals == 1)) {
             
               localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (belowright);
                                                        
                                                        
               localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (aboveright);
                                                        
              }                                      
                                                        
/* 
special case
         
*/                                               
          else if ( (countyminvals == 1) && (countymaxvals == 1) &&
                    (get<0>(infedgecenter[i]) > 
                    ((belowleftx + belowrightx) / 2)) ) {
               
                  localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                             (belowright);
                                                        
                                                        
                  localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                             (aboveright);
                                                        
               }                                      
       
       
              
/* 
 special case
         
*/                                                            
           else if ( (countyminvals == 1) && (countymaxvals == 1) &&
                     (get<0>(infedgecenter[i]) < 
                     ((belowleftx + belowrightx) / 2)) )   {
             
                localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (belowleft);
                                                        
                                                        
                localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (aboveleft);
                                                        
               }               
               
               
 /* 
   special case
      
 */             
             else if ( (countxminvals == 1) && (countxmaxvals == 1) &&
                     (get<1>(infedgecenter[i]) < 
                     ((belowlefty + abovelefty) / 2)) )   {
             
                localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (belowleft);
                                                        
                                                        
                localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (belowright);
                                                        
               }                 
               
               
/*
  special case
         
*/              
             else if ( (countxminvals == 1) && (countxmaxvals == 1) &&
                     (get<1>(infedgecenter[i]) > 
                     ((belowlefty + abovelefty) / 2)) )   {
             
                localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (aboveleft);
                                                        
                                                        
                localInfo->center2vorpoi.at(infedgecenter[i]).push_back
                                                        (aboveright);
                                                        
               }                 
               
         
         
            }     
    
    
            
      return 0;
      
    }
    
    
    
/* 
  voronoiVM-request 
        
*/  
 case REQUEST: {    
       
   TupleType *tupleType = (TupleType*)local.addr;
   Tuple *newTuple = new Tuple( tupleType );
   
   map<std::tuple<double, double>,  std::vector<std::tuple<double, double>>>::
   iterator iter=localInfo->center2vorpoi.begin();   
   
   int maxattrpos = 0;
   std::vector<std::tuple<double, double>> tmpo;
   std::vector<std::tuple<double, double>> dummy;
   
   int checkokflag;
   
   double xv, yv;
   
   int pointposit = localInfo->pointposition;

   std::tuple<double, double> search;
   
 /*
    calculate max attribute position
    
 */
  maxattrpos = localInfo->attrcount - 1;
    
    
 /*    
   get point coords
    
 */   
  if ((tup = localInfo->reliter->GetNextTuple()) != 0 ) {
     
   Point* pointval = static_cast<Point*>(tup->GetAttribute(pointposit-1));

   // Convex cell id corresponds to tupleId 
   int cell_Id = (int)tup->GetTupleId();        
     if(pointval->IsDefined()) {
         
        xv = pointval->GetX(); 
        yv = pointval->GetY();
     }
     
     
    
     search =  std::make_tuple(xv, yv);     
     iter = localInfo->center2vorpoi.find(search);
      
    

   if (iter !=localInfo->center2vorpoi.end())   {
       
        
/*
  setting up the new tuple
    
*/   
   for( int i = 0; i < tup->GetNoAttributes(); i++ ) {
   
        newTuple->CopyAttribute( i, tup, i );
       }
   
    
   tmpo = localInfo->center2vorpoi.at(search);   
   
   
  checkokflag = checkme(tmpo, localInfo->checkon);   
   if ((checkokflag == 1) || (checkokflag == 2)) {

   Convex* conv = new Convex(tmpo, cell_Id);
   voroVec.push_back(*conv);
   newTuple->PutAttribute(maxattrpos+1, conv);
   result = SetWord(newTuple);
      
   }
   
     else { 
      
    Convex* conv = new Convex(false);
    newTuple->PutAttribute(maxattrpos+1, conv);        
    result = SetWord(newTuple);


    
   
     
    }
   
   return YIELD;
   
   }
   
   else { 
       
 /* 
  not possible
   
 */   
   Convex* conv2 = new Convex(false);
   
   for( int i = 0; i < tup->GetNoAttributes(); i++ ) {
   
        newTuple->CopyAttribute( i, tup, i );
       }
        
    
    newTuple->PutAttribute(maxattrpos+1, conv2);        
  
    result = SetWord(newTuple);    
    
    
  
   return YIELD;
   
   }
   
   
   return YIELD; 
   
 /*
  never happens
   
 */    
   }
   
   else {
        delete newTuple;
        return CANCEL; 
   }
       
       
   
   return 0;
 }
    
/* 
 voronoiVM-close
        
*/    
 case CLOSE : {
    
    if(localInfo){
        if( localInfo->reliter != 0 )
          delete localInfo->reliter;
        
         if( localInfo->rel )  {
          localInfo->rel->Clear();
          delete localInfo->rel;
           }
        delete localInfo;
         qp->GetLocal2(s).addr=0;
      }
        
      qp->Close(args[0].addr);
      
      if (local.addr)
      {
        ((TupleType*)local.addr)->DeleteIfAllowed();
        local.setAddr(0);
      
      }
     
 return 0;    
 }
 
       
 } 
 
 return 0;
}



/*

3D Convex

*/

/*
  Class Polyhedron for result of voronoi3d

*/

Polyhedron::Polyhedron() {}

void Polyhedron::setPolyId(int id_) {
  this->polyhedronId = id_;
}

int Polyhedron::getPolyId() {
  return polyhedronId;
}

Polyhedron::~Polyhedron() {}


/*
  3D Convex constructor

*/
Convex3D::Convex3D() {
  boundingBox = nullptr;
}

Convex3D::Convex3D(const Convex3D& g) {
  boundingBox = g.boundingBox;
}

Convex3D::Convex3D(Rectangle<3> &bounding_box) {
  boundingBox = &bounding_box;
}

Convex3D::Convex3D(std::vector<Polyhedron> &poly_vec) {
  polyhedronvec = poly_vec;
}

void Convex3D::Set(Stream<Rectangle<3>> rStream) {
    CreateVoronoi3D(rStream);
}

void Convex3D::CreateVoronoi3D(Stream<Rectangle<3>> rStream) {
  // create voronoi diagram
  buildVoronoi3D(rStream);

}

void
Convex3D::setTetrahedronVector(std::vector<Tetrahedron*> tetra_vec) {
  this->tetravec = tetra_vec;
}

std::vector<Tetrahedron*>&
Convex3D::getTetrahedronVector() {
  return this->tetravec;
}

void
Convex3D::setPolyhedronVector(std::vector<Polyhedron> poly_vec) {
  this->polyhedronvec = poly_vec;
}

std::vector<Polyhedron>&
Convex3D::getPolyhedronVector() {
  return this->polyhedronvec;
}

std::vector<Point3D>&
Convex3D::getPointsVector() {
  return this->points;
}
    
void 
Convex3D::setPointsVector(std::vector<Point3D> pointsvec) {
  this->points = pointsvec;
}

std::vector<Point3D>& 
Convex3D::getVerticeVector() {
  return this->vertices;
}

void 
Convex3D::setVerticeVector(std::vector<Point3D> verticevec) {
  this->vertices = verticevec;
}

std::vector<std::vector<int>>&
Convex3D::getFacesVector() {
  return this->faces;
}

void 
Convex3D::setFacesVector(std::vector<std::vector<int>> facevec) {
  this->faces = facevec;
}
    
std::vector<std::vector<int>>&
Convex3D::getV2TVector(){
  return this->v2t;
}
    
void 
Convex3D::setV2TVector(std::vector<std::vector<int>> v2tvec){
  this->v2t = v2tvec;
}
    
std::vector<std::vector<int>>&
Convex3D::getF2TVector(){
  return this->f2t;
}
    
void 
Convex3D::setF2TVector(std::vector<std::vector<int>> f2tvec){
  this->f2t = f2tvec;
}


void
Convex3D::setCellId(int cell_id) {
  this->polyhedronId = cell_id;
}

int
Convex3D::getCellId() {
  return polyhedronId;
}

Convex3D::~Convex3D() {}


/* 
  Calculates cross product of two vectors and returns vector

*/
std::vector<double> crossProduct(std::vector<double> va,
          std::vector<double> vb) 
{
  
  std::vector<double> res {va[1]*vb[2] - va[2]*vb[1],
   va[2]*vb[0] - va[0]*vb[2], va[0]*vb[1] - va[1]*vb[0]};
  return res;
}

/*
  Calculates dot product of two vectors and returns value

*/
double dotProduct(std::vector<double> va, std::vector<double> vb)
{
   double product = va[0]*vb[0] + va[1]*vb[1] + va[2]*vb[2];
   return product;

}

/*
  Calculates centre of a cuboid

*/
Point3D getCentre(Rectangle<3>* r) {
  double a = (r->getMaxY() - r->getMinY()) / (double)2;
  double b = (r->getMaxX() - r->getMinX()) / (double)2;
  double c = (r->getMaxZ() - r->getMinZ()) / (double)2;

  Point3D r_c;
  r_c.x = r->getMinX() + b;
  r_c.y = r->getMinY() + a;
  r_c.z = r->getMinZ() + c;

  return r_c;
}

/*
  Returns true if Point p is in given polyhedron

*/
bool isInPolyhedron(Point3D p, Polyhedron poly, Point3D pi)
{    
  for(size_t i = 1; i < poly.faces.size(); i++)
  {
    // calculate first orient3d to have a comparetive value
    std::vector<Point3D> face = poly.faces[i];
    double pa[3] = {face[0].x, face[0].y, face[0].z};
    double pb[3] = {face[1].x, face[1].y, face[1].z};
    double pc[3] = {face[2].x, face[2].y, face[2].z};
    // take point pi (which lies definitly in cell) as a reference
    double pd[3] = {pi.x, pi.y, pi.z};


    double d0 = orient3d(pa, pb, pc, pd);
    double paC[3] = {face[0].x, face[0].y, face[0].z};
    double pbC[3] = {face[1].x, face[1].y, face[1].z};
    double pcC[3] = {face[2].x, face[2].y, face[2].z};
    double pp[3] = {p.x, p.y, p.z};
    double d1 = orient3d(paC, pbC, pcC, pp);

/* 
  if values for determinants (d0,d1) 
  have different signs,
  the points pi and p cannot be on the same side
  of the checked face. In this case p cannot be
  inside of the polyhedron

*/
    if(signbit(d0) != signbit(d1)) 
    {
      return false;
    }
  }

  return true;
}

/* 
  Returns true if point lies in cuboid

*/
bool insideCuboid(Rectangle<3>* r, Point3D center) {
  double x = center.x;
  double y = center.y;
  double z = center.z;

  if (r->getMinX() <= x && r->getMaxX() >= x
      && r->getMinY() <= y && r->getMaxY() >= y
      && r->getMinZ() <= z && r->getMaxZ() >= z) {                        

    return true;
  }

 return false;
}

/* 
  Returns true if one of the points
  of the given polyhedron lies in cuboid

*/
bool insideCuboid(Rectangle<3>* r, Polyhedron poly) {

  for(size_t t = 0; t < poly.faces.size(); t++)
  {
    std::vector<Point3D> face = poly.faces[t];
    for(size_t f = 0; f < face.size(); f++)
    {
        double x = face[f].x;
        double y = face[f].y;
        double z = face[f].z;
        if (r->getMinX() <= x && r->getMaxX() >= x
        && r->getMinY() <= y && r->getMaxY() >= y
        && r->getMinZ() <= z && r->getMaxZ() >= z) {                        

        return true;
    } 
    }
  }

 return false;
}


bool sortbyX(Point3D a, Point3D b)
{
    return a.x < b.x;
}

bool sortbyY(Point3D a, Point3D b)
{
    return a.y < b.y;
}

bool sortbyZ(Point3D a, Point3D b)
{
    return a.z < b.z;
}


ListExpr
Convex3D::PropertyConvex3D()
{
    ListExpr desclst = nl->TextAtom();
    nl->AppendText(desclst,
    "A polyhedron id "
    "followed by list of faces.\n"
    "A face consists of a list of 3d points\n"
    "(<x value> <y value> <z value>). ");

  ListExpr formatlst = nl->TextAtom();
    nl->AppendText(formatlst,
    "(1 (((1.0 2.0 1.0)(1.0 3.0 1.0)(4.0 3.0 1.0)"
    "(1.0 1.5 2.0)(2.0 1.5 2.0))))");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(Convex3D::BasicType()),
               desclst,
               formatlst)));
}

/* 
  Out function

*/
ListExpr
Convex3D::OutConvex3D( ListExpr typeInfo, Word value ) {

  Convex3D* conv = static_cast<Convex3D*>( value.addr );

    if(conv != nullptr)
    {
      std::vector<Polyhedron> polyhedronvec = conv->getPolyhedronVector();
      
    ListExpr polyExpr = nl->Empty();
    ListExpr lastPolyLstExpr;
    for(size_t test=0; test < polyhedronvec.size(); test++)
    {
      ListExpr facesLstExpr;
      ListExpr lastFacesLstExpr;
      Polyhedron poly = polyhedronvec.at(test);
      if(test > 0) {

        lastPolyLstExpr = nl->Append(lastPolyLstExpr,
                          nl->IntAtom(poly.getPolyId()));
      } else {
        polyExpr = nl->OneElemList(nl->IntAtom(poly.getPolyId()));
        lastPolyLstExpr = polyExpr;
      }
      
      for(size_t fac = 0; fac < poly.faces.size(); fac++)
      {
        ListExpr faceLstExpr;
        ListExpr lastFaceLstExpr;
        if(fac > 0)
        {
          lastFacesLstExpr = nl->Append(lastFacesLstExpr, nl->Empty());
        } else {
          facesLstExpr = nl->OneElemList(nl->Empty());
          lastFacesLstExpr = facesLstExpr;
        }
        std::vector<Point3D> facc = poly.faces.at(fac);
        for(size_t poii=0; poii < facc.size(); poii++)
        {
          
          if(poii > 0)
          {
            lastFaceLstExpr = nl->Append(lastFaceLstExpr,
                              nl->ThreeElemList(nl->RealAtom(facc.at(poii).x), 
                              nl->RealAtom(facc.at(poii).y),
                              nl->RealAtom(facc.at(poii).z)));
          } else {
            faceLstExpr = nl->OneElemList(nl->ThreeElemList(
                              nl->RealAtom(facc.at(poii).x), 
                              nl->RealAtom(facc.at(poii).y),
                              nl->RealAtom(facc.at(poii).z)));
            lastFaceLstExpr = faceLstExpr;
          }
        }

        lastFacesLstExpr = nl->Append(lastFacesLstExpr,
             faceLstExpr);
        
      }
      
      lastPolyLstExpr = nl->Append(lastPolyLstExpr, facesLstExpr);
    }
    return polyExpr;
    } else {
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  }
}

/* 
  In function

*/
Word
Convex3D::InConvex3D( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct ) {

  Word w = SetWord(Address(0));
  try {
  Polyhedron polyhedron; 
  std::vector<Point3D> face;
  std::vector<Polyhedron>  poly_vec;
  std::vector<std::vector<Point3D>> facesPoints;
   
   ListExpr polyExpr = instance;

   int polyId;
   if (nl->ListLength( polyExpr ) > 0) {
     while(nl->ListLength(polyExpr) > 1) {
       ListExpr lstElem = nl->First(polyExpr);
       if(nl->IsAtom(lstElem))
       {
         polyhedron = Polyhedron();
         polyId = nl->IntValue(lstElem);
         polyhedron.setPolyId(polyId);
       }

       ListExpr facesLst = nl->Second(polyExpr); 
/*
  list of faces

*/
       if(nl->ListLength(facesLst) > 1) { 
          while(nl->ListLength(facesLst) > 1) {
            if(nl->ListLength(facesLst) > 1) {
            ListExpr faceElems = nl->Second(facesLst);
            if(nl->ListLength(faceElems) > 0) {
              face.clear();
              while(!nl->IsEmpty(faceElems)) {

              ListExpr vertex = nl->First(faceElems);
              ListExpr first = nl->First(vertex);
              ListExpr second = nl->Second(vertex);
              ListExpr third = nl->Third(vertex);

              
              if(nl->IsAtom(first) && nl->AtomType(first) == RealType
              && nl->IsAtom(second) && nl->AtomType(second) == RealType
              && nl->IsAtom(third) && nl->AtomType(third) == RealType)
              {
                double x = nl->RealValue(first);
                double y = nl->RealValue(second);
                double z = nl->RealValue(third);
                Point3D p {x,y,z};
                face.push_back(p);
              }
              
              faceElems = nl->Rest(faceElems);
              }
            }
            polyhedron.faces.push_back(face);
            facesLst = nl->Rest(facesLst);
            facesLst = nl->Rest(facesLst);
            }
          }
       }
      poly_vec.push_back(polyhedron);
      polyExpr = nl->Rest(polyExpr);
      polyExpr = nl->Rest(polyExpr);
     }


   }

    Convex3D* conv3d = new Convex3D();
    conv3d->setPolyhedronVector(poly_vec);

    w.addr = conv3d;
    correct = true;
    
    return w;
  } catch (int e) {
    correct = false;
    cmsg.inFunError("Expecting a 3dvoronoi list representation. Exit code "
        + std::to_string(e));

    return w;
  }
}

// This function checks whether the type constructor is applied correctly.
bool
Convex3D::KindCheckConvex3D( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Convex3D::BasicType() ));
}

// Clone function
Word
Convex3D::CloneConvex3D( const ListExpr typeInfo, const Word& w )
{
  Convex3D *g = new Convex3D( *((Convex3D *)w.addr) );
  return SetWord( g );
}

// Create function
Word
Convex3D::CreateConvex3D( const ListExpr typeInfo )
{
  return SetWord( new Convex3D() );
}

// Delete function
void
Convex3D::DeleteConvex3D( const ListExpr typeInfo, Word& w )
{
  delete (Convex3D *)w.addr;
  w.addr = 0;
}

// SizeOf function
int
Convex3D::SizeOfConvex3D()
{
  return sizeof(Convex3D);
}

/*
  Returns true if points are equal

*/
bool comparePoints3D(Point3D p1, Point3D p2) 
{
  bool res = (p1.x == p2.x && p1.y == p2.y && p1.z == p2.z); 
    return res;
}

/*
  Normalizes a vector

*/
Point3D normalize(double x, double y, double z) 
{
        const float s = 1.0f / sqrtf(x*x + y*y + z*z);
        Point3D p {x*s, y*s, z*s};
        return p;

}

/*
  Returns the index of a point in a vector of points

*/
int getIndex(Point3D t1, std::vector<Point3D> v2) 
{
  for(int i=0; i < (int)v2.size(); i++)
  {
    Point3D t2 = v2.at(i);
    if(t1.x == t2.x && t1.y == t2.y && t1.z == t2.z) {
      return i;
    }
  }

  return -1;

}

/*
  Returns the centre of the sphere around a tetrahedron
  see
  Weisstein, Eric W. 'Circumsphere.' 
  From MathWorld--A Wolfram Web Resource.  
  for explanation

*/
Point3D getCircumcenter(Point3D a, Point3D b, Point3D c, Point3D d)
{
  // calculate middle of Tetrahedron with circumsphere
  double fina[3] = {a.x, a.y, a.z};
  double finb[3] = {b.x, b.y, b.z};
  double finc[3] = {c.x, c.y, c.z};
  double find[3] = {d.x, d.y, d.z};

  float ax = orient3d(fina, finb, finc, find);

/*
  build new points for Dx, Dy and Dz 
  by discarding column xi (or yi or zi)

*/
  float square_1 = a.x*a.x + a.y*a.y + a.z*a.z;
  float square_2 = b.x*b.x + b.y*b.y + b.z*b.z;
  float square_3 = c.x*c.x + c.y*c.y + c.z*c.z;
  float square_4 = d.x*d.x + d.y*d.y + d.z*d.z;

  double dx_1[3] = {square_1, a.y, a.z};
  double dx_2[3] = {square_2, b.y, b.z};
  double dx_3[3] = {square_3, c.y, c.z};
  double dx_4[3] = {square_4, d.y, d.z};

  float dx = orient3d(dx_1, dx_2, dx_3, dx_4);

  double dy_1[3] = {square_1, a.x, a.z};
  double dy_2[3] = {square_2, b.x, b.z};
  double dy_3[3] = {square_3, c.x, c.z};
  double dy_4[3] = {square_4, d.x, d.z};

  float dy = -orient3d(dy_1, dy_2, dy_3, dy_4);

  double dz_1[3] = {square_1, a.x, a.y};
  double dz_2[3] = {square_2, b.x, b.y};
  double dz_3[3] = {square_3, c.x, c.y};
  double dz_4[3] = {square_4, d.x, d.y};

  float dz = orient3d(dz_1, dz_2, dz_3, dz_4);

  if(ax < 0)
  {
    ax = orient3d(fina, finc, finb, find);
    dx = orient3d(dx_1, dx_3, dx_2, dx_4);
    dy = -orient3d(dy_1, dy_3, dy_2, dy_4);
    dz = orient3d(dz_1, dz_3, dz_2, dz_4);

  }

  Point3D circumcenter;
  circumcenter.x = (dx / (2*ax));
  circumcenter.y = (dy / (2*ax));
  circumcenter.z = (dz / (2*ax));

  return circumcenter;
}


/*
  Returns true if line p1 to p2 is a line in tet (two tetrahedrons share a line)

*/
bool sameline(Point3D p1, Point3D p2, Tetrahedron tet)
{
    std::vector<Point3D> tmp {tet.a, tet.b, tet.c, tet.d};
    bool p1in = false;
    bool p2in = false;
    for(int i = 0; i < (int)tmp.size(); i++)
    {
      p1in = comparePoints3D(p1, tmp.at(i));
      if(p1in)
      {
        for(int z=0; z<(int)tmp.size(); z++)
        {
          if(z!=i)
          {
            p2in = comparePoints3D(p2,tmp.at(z));
            if(p2in)
            {
              return true;
            }
          }
        }
      }
    }

  return false;

}

bool sameline(int p1, int p2, std::vector<int> tet)
{
    bool p1in = false;
    bool p2in = false;
    for (const auto &value:tet)
    {
      if(value == p1)
        p1in = true;

      if(value == p2)
        p2in = true;


      if(p1in && p2in)
      {
        return true;
      }
    }

  return false;

}

/*
  Returns true if a pair of points is 
  in a vector of pairs of points

*/
bool insidevec(Point3D p1, Point3D p2,
   std::vector<std::tuple<Point3D, Point3D>> pairs)
{
  for(int o=0; o < (int)pairs.size(); o++)
  {
    Point3D t1 = std::get<0>(pairs.at(o));
    Point3D t2 = std::get<1>(pairs.at(o));

    if(comparePoints3D(t1, p1) && comparePoints3D(t2, p2))
    {
      return true;
    }

  }


  return false;
}

/*
  Returns true if a pair of points is 
  in a vector of pairs of points

  using indices

*/
bool insidevec(int p1, int p2,
   std::vector<std::tuple<int, int>> pairs)
{
  for(int o=0; o < (int)pairs.size(); o++)
  {
    int t1 = std::get<0>(pairs.at(o));
    int t2 = std::get<1>(pairs.at(o));

    if((t1 == p1 && t2 == p2) || (t1 == p2 && t2 == p1))
    {
      return true;
    }

  }


  return false;
}

/*
  Returns true if a segment pd intersects with triangle abc
  See Moeller-Trumbore intersection algorithm 
  for further explanation
  Paper: Fast MinimumStorage RayTriangle Intersection

*/
bool segmentIntersectsArea(Point3D p, Point3D d, Point3D a,
Point3D b, Point3D c)
{ 
  double epsilon = 0.000001f;
  Point3D e0 {b.x-a.x, b.y-a.y, b.z-a.z};
  Point3D e1 {c.x-a.x, c.y-a.y, c.z-a.z};

  Point3D dir {d.x-p.x, d.y-p.y, d.z-p.z};
  Point3D dir_norm = normalize(dir.x, dir.y, dir.z);

  std::vector<double> dir_norm_vec {dir_norm.x, dir_norm.y, dir_norm.z};
  std::vector<double> dir_vec{dir.x, dir.y, dir.z};
  std::vector<double> e1_vec {e1.x, e1.y, e1.z};
  std::vector<double> e0_vec {e0.x, e0.y, e0.z};

/*
The equation that needs to be solved is (6)
(page 3 from source above)
The first step is to check whether the part
(dir\_norm\_vec x e1\_vec) \* e0\_vec
(in the paper this would be (D x E2)\*E1) is non-zero.

If the equation is not solvable we can stop.

In the paper h is called P.

*/
  std::vector<double> h = crossProduct(dir_norm_vec, e1_vec);
  double dp = dotProduct(e0_vec, h);

  if (dp > -epsilon && dp < epsilon) {
      return false;
  }

/*
  The next step is to check the value of u
  which is calculated as 1 / (P * E1) * (P * T)
  In this notation:
  h: P
  dp: (P * E1)
  s\_vec: T
  So:
  u = (1 / dp) * (s\_vec * h) 

*/
  Point3D s {p.x-a.x, p.y-a.y, p.z-a.z};
  std::vector<double> s_vec {s.x, s.y, s.z};
  const float f = 1.0f / dp;
  const float u = f * dotProduct(s_vec, h);

/*
  u has to be between 0 and 1 as we are 
  looking at a unit triangle in
  y and z and a ray direction aligned with x.
  If u is not between 0 and 1 we can stop.

*/
  if (u < 0.0f || u > 1.0f) {
      return false;
  }

/*
  Now we look at the v in equation (6) in the paper.
  Here v = 1 / (P * E1) * ((T x E1) * D)
  In this notation:
  v = f * ((s\_vec x e0\_vec) * dir\_norm\_vec)

*/
  std::vector<double> q = crossProduct(s_vec, e0_vec);
  const float v = f * dotProduct(dir_norm_vec, q);

/*
  v has to be positive and u + v cannot exceed 1 to lie
  in the unit triangle

*/
  if (v < 0.0f || u + v > 1.0f) {
      return false;
  }

/*
  Now the last value t is calculated accordingly as
  t = 1 / (P * E1) * ((T x E1) * E2)
  In this notation:
  t = f * (q * e1\_vec)

*/
  const float t = f * dotProduct(e1_vec, q);

/*
  segment intersection:
  the distance variable t must be between 0 and the length
  of the direction vector dir\_vec for the segment between the two
  points p and d to intersect the triangle.

*/
  if (t > epsilon && t < sqrtf(dotProduct(dir_vec, dir_vec))) { 
      return true;
  }

  return false;

}

/*
  Returns all cellnumbers of cells which intersect with a given rectangle
  Two modes are possible: intersections with the exact cell and intersection
  with the bounding box of a cell

*/
std::set<int> cellNum3D(Convex3D* convex3d, 
Rectangle<3>* search_window_ptr, int mode) 
{
  // get polyvec
  std::vector<Polyhedron> polyvec = convex3d->getPolyhedronVector();
  std::vector<Point3D> inputPoints = convex3d->getPointsVector();
  if(inputPoints.size() < polyvec.size())
  {
/* 
  calculate center of polyeder
  1. calculate bbox of polyeder
  2. calculate center of this bbox

*/
    for(size_t p = 0; p < polyvec.size(); p++)
    {
      if(polyvec[p].faces.size() > 2) {
        Rectangle<3> bboxP = createBBox3D(&polyvec[p]);
        Point3D bc = getCentre(&bboxP);
        inputPoints.push_back(bc);
      } else {
        polyvec.erase(polyvec.begin()+p);
      }
    }
  }

  std::set<int> cell_ids;
  int bbox = 2;
  int precise = 1;
  // Rectangle points
  Point3D lebofr, ribofr, letofr, ritofr, leboba, riboba, letoba, ritoba;
  lebofr = {search_window_ptr->getMinX(), 
    search_window_ptr->getMinY(), search_window_ptr->getMinZ()};
  ribofr = {search_window_ptr->getMaxX(), 
    search_window_ptr->getMinY(), search_window_ptr->getMinZ()};
  letofr = {search_window_ptr->getMinX(), 
    search_window_ptr->getMaxY(), search_window_ptr->getMinZ()};
  ritofr = {search_window_ptr->getMaxX(), 
    search_window_ptr->getMaxY(), search_window_ptr->getMinZ()};
  leboba = {search_window_ptr->getMinX(), 
    search_window_ptr->getMinY(), search_window_ptr->getMaxZ()};
  riboba = {search_window_ptr->getMaxX(), 
    search_window_ptr->getMinY(), search_window_ptr->getMaxZ()};
  letoba = {search_window_ptr->getMinX(), 
    search_window_ptr->getMaxY(), search_window_ptr->getMaxZ()};
  ritoba = {search_window_ptr->getMaxX(), 
    search_window_ptr->getMaxY(), search_window_ptr->getMaxZ()};

  Point3D center = getCentre(search_window_ptr);

  // intersection with exact cell
  if(mode == precise) {
  for(size_t i = 0; i < polyvec.size(); i++) 
  {
    Polyhedron tmp = polyvec[i];
    if(!tmp.faces.empty()) {

/*
  1. Check if any point (taken from points) of polyhedron 
    lies in cuboid: add cell id (easy, saves time)
  2. Check if middle of cuboid intersects with polyhedron 
    cell (get middle of cuboid, check if it lies in cell)
  - if yes: add cell\_id to cell\_ids
  - if no: check if edges of cuboid intersect with faces of polyhedron
    - get face and points of cuboid, check intersection
    - if one intersect: add cell\_id to cell\_ids
    - if no intersects: go to next cell, don't add cell\_id

*/
    if(insideCuboid(search_window_ptr, inputPoints.at(i)))
    {
      cell_ids.insert(tmp.getPolyId());
    }
    else if(isInPolyhedron(center, tmp, inputPoints.at(i)))
    {
        cell_ids.insert(tmp.getPolyId());
    } 
    else
    {
      // check intersection for each face
      for(size_t f = 0; f < tmp.faces.size(); f++)
      {
        std::vector<Point3D> face = tmp.faces.at(f);
        if(face.size() > 2) {
        for(size_t n=0; n < face.size()-2; n++)
        {
/* 
  polygon consists of x-2 (x number of vertices) triangles
  triangle consists of vertex v[0], v[n+1], v[n+2]
  n = number of triangles already formed
  first triangle: v[0], v[1], v[2] with n = 0

*/
        if(segmentIntersectsArea(lebofr, leboba, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(ribofr, riboba, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(lebofr, ribofr, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(leboba, riboba, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(lebofr, letofr, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(ribofr, ritofr, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(leboba, letoba, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(riboba, ritoba, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(letofr, letoba, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(ritofr, ritoba, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(letofr, ritofr, face[0], face[n+1], face[n+2])
        || segmentIntersectsArea(letoba, ritoba, face[0], face[n+1], face[n+2]))
        {
          cell_ids.insert(tmp.getPolyId());
          break;
        }
        }
        }
      }
    }

    }
  }
  
  } else if (mode== bbox) {

    for(size_t b = 0; b < polyvec.size(); b++) 
    {
      Polyhedron pol = polyvec.at(b);
      if(!pol.faces.empty()) {
      Rectangle<3> bbox = createBBox3D(&pol);

/*
  see if bbox intersects with rectangle
  top left and bottom right of bbox

*/
      Point3D tolefr, boriba;
      tolefr = {bbox.getMinX(), bbox.getMaxY(), bbox.getMinZ()};
      boriba = {bbox.getMaxX(), bbox.getMinY(), bbox.getMaxZ()};
      if(cuboidOverlap(tolefr, boriba, letofr, riboba)) {
        cell_ids.insert(pol.getPolyId());
      }
      }
    }

  } else {
    cell_ids.insert(0);
  }

  return cell_ids;

}

/*
  checks if id of face is already in vector
  if yes: returns position

*/
bool Convex3D::faceIn(std::vector<int> face, int* pos)
{
  std::sort(face.begin(), face.end());
  for(int a = 0; a < (int)faces.size(); a++)
  {
    std::sort(faces[a].begin(), faces[a].end());
    if(faces[a] == face)
    { 
      *pos = a;
      return true;
    }
  }
  return false;
}

/*
  check if tetrahedron exists in vector v2t

*/
bool Convex3D::tetExists(int a, int b, int p, int d, int* pos)
{
  std::vector<int> tetr {a, b, p, d};
  sort(tetr.begin(), tetr.end());
  for(int v=0; v < (int)v2t.size(); v++)
  {
    sort(v2t[v].begin(), v2t[v].end());
    if(v2t[v] == tetr)
    {
      *pos = v;
      return true;
    }

  }
  return false;
}



/*
  Create Delaunay diagram 
  Input: Set of points
  Output: Delaunay diagram

  Further explanation in paper of Hugo Ledoux
  Computing the 3D Voronoi Diagram Robustly: 
  An Easy Explanation

*/
void
Convex3D::createDelaunay (std::vector<Point3D> points,
         std::vector<Tetrahedron>* tetravec) 
{
  // 1. Get min and max x,y,z
  std::vector<Point3D> pointsSortedbyX = points;
  std::vector<Point3D> pointsSortedbyY = points;
  std::vector<Point3D> pointsSortedbyZ = points;

  // Sort Points by x-value
  std::sort(pointsSortedbyX.begin(), pointsSortedbyX.end(), sortbyX);
  double xmin = pointsSortedbyX.front().x;
  double xmax = pointsSortedbyX[pointsSortedbyX.size() -1].x;

  // Sort points by y-value
  std::sort(pointsSortedbyY.begin(), pointsSortedbyY.end(), sortbyY);
  double ymin = pointsSortedbyY.front().y;
  double ymax = pointsSortedbyY[pointsSortedbyY.size()-1].y;

  // Sort points by z-value - check if it works to get z-value
  std::sort(pointsSortedbyZ.begin(), pointsSortedbyZ.end(), sortbyZ);
  double zmin = pointsSortedbyZ.front().z;
  double zmax = pointsSortedbyZ[pointsSortedbyZ.size()-1].z;

  if(pointsSortedbyX.size() > 0)
  {
    pointsSortedbyX.clear();
  }
  if(pointsSortedbyY.size() > 0)
  {
    pointsSortedbyY.clear();
  }
  if(pointsSortedbyZ.size() > 0)
  {
    pointsSortedbyZ.clear();
  }

/*
  2. Build tetrahedron with min and max values
  2.1 Calculate bounding box of points with min and max values
  2.2 Build triangle around upper rectangle of cuboid (bounding box)
  2.3 Build tetrahedron with point above triangle
  2.4 Extend sides from point to triangle until it reaches 
      height of lower rectangle of cuboid
  2.5 result is the start tetrahedron  

*/
  double min[3], max[3];
  min[0] = xmin;
  min[1] = ymin;
  max[0] = xmax;
  max[1] = ymax;
  min[2] = zmin;
  max[2] = zmax;

  Rectangle<3> bbox (true, min, max);

  double x_len = (xmax-xmin);
  double z_len = (zmax-zmin);
  // triangle points:
  Point3D t1 {xmin-x_len, ymax, zmin-1};
  Point3D t2 {xmin+x_len, ymax, ((z_len+1)*2)};
  Point3D t3 {xmax+x_len, ymax, zmin-1};

  // upper tetrahedron:
  double y_len = (ymax-ymin);
  // upper point of tetrahedron
  Point3D t4 {xmin+(x_len/2), ymax+y_len, zmin+(z_len/2)};

  Point3D t5 {t1.x-y_len, ymin-1, zmin-1};
  Point3D t6 {t2.x, ymin-1, t2.z+y_len};
  Point3D t7 {t3.x+y_len, ymin-1, zmin-1};

  // adding vertices t4-t7 to vertice vector
  vertices.push_back(t4);
  vertices.push_back(t5);
  vertices.push_back(t6);
  vertices.push_back(t7);

/*
  adding faces to faces vector
  use indices of vertice vector

*/
  std::vector<int> face1 {0, 1, 2};
  std::vector<int> face2 {0, 2, 3};
  std::vector<int> face3 {0, 1, 3};
  std::vector<int> face4 {1, 2, 3};

  faces.push_back(face1);
  faces.push_back(face2);
  faces.push_back(face3);
  faces.push_back(face4);

  if(face1.size() > 0)
    face1.clear();
  if(face2.size() > 0)
    face2.clear();
  if(face3.size() > 0)
    face3.clear();
  if(face4.size() > 0)
    face4.clear();

/* 
  adding indices to tetrahedron vectors
  tetrahedron is build from four vertices
  vertices2tetrahedron (v2t)

*/
  double dz[3] {t4.x, t4.y, t4.z};
  double dx[3] {t5.x, t5.y, t5.z};
  double dy[3] {t6.x, t6.y, t6.z};
  double dv[3] {t7.x, t7.y, t7.z};

  if(orient3d(dz, dx, dy, dv) > 0)
  {
    std::vector<int> vert2tet {0,1,2,3};
    v2t.push_back(vert2tet);
    vert2tet.clear();
  } else {
    std::vector<int> vert2tet {0,2,1,3};
    v2t.push_back(vert2tet);
    vert2tet.clear();
  }

/*
  faces2tetrahedron (f2t)
  vector which has indices to faces
  of each tetrahedron that is inside
  this vector

*/
  std::vector<int> face2tet {0,1,2,3};
  f2t.push_back(face2tet);
  face2tet.clear();

  std::vector<std::vector<int>> tetfaces;
  std::vector<std::vector<int>> tetverts;

/* 
  push start tetrahedron in vector
  Insert point by point in initial tetrahedron

*/
  for(int i = 0; i < (int)points.size(); i++)
  {
    Point3D p = points.at(i);

/*
  find tetrahedron T containing p
  with visibility walk through diagram

*/
    bool found_tet = false;
    int n = 0;
    int start = 0;
    std::vector<int> tetStart =f2t[n];
    std::vector<int> faceStart = faces[tetStart[start]];

    std::vector<int> tStart = v2t[n];
    double pit[3] = {p.x, p.y, p.z};

    int a = 0;

/*
  visited elements in v2t

*/
    std::vector<int> visited {};
    std::vector<int>::iterator it;

    while(found_tet == false)
    {
      start = 0;
      a++;

      it = std::find(visited.begin(), visited.end(), n); 

/* 
  to take another starting point 
  (and not to get stuck in visibility walk)

*/
      if (it != visited.end()) 
      { 
        // random start 
        n = std::rand() % v2t.size();
        tStart = v2t[n];
      }

      // first points at position 0,1,2
      double ait[3] = {vertices[tStart[0]].x, 
      vertices[tStart[0]].y, vertices[tStart[0]].z} ;
      double bit[3] = {vertices[tStart[1]].x, 
      vertices[tStart[1]].y, vertices[tStart[1]].z};
      double cit[3] = {vertices[tStart[2]].x, 
      vertices[tStart[2]].y, vertices[tStart[2]].z};
      double dit[3] = {vertices[tStart[3]].x, 
      vertices[tStart[3]].y, vertices[tStart[3]].z};

      double detTet = orient3d(ait, bit, cit, dit);

      if(signbit(orient3d(ait, bit, cit, pit)) 
        != signbit(detTet))
      {
        visited.push_back(n);
          // face
        std::vector<int> fa {tStart[0], tStart[1], tStart[2]};
        int index = -1;
        std::vector<int>::iterator it;
        if(faceIn(fa, &index)) 
        {
          
/*
  is index in another tet in f2t

*/
          for(int ii=0; ii < (int)f2t.size(); ii++)
          { 
            if(ii != n) {
              it = find (f2t[ii].begin(), f2t[ii].end(), index);
              if (it != f2t[ii].end()) 
              {
                n = ii;
                tStart = v2t[n];
                break;
              }
            } 
          }
        }

      } else {
        if(signbit(orient3d(ait, bit, pit, dit)) 
            != signbit(detTet))
        {
          visited.push_back(n);
          std::vector<int> fa {tStart[0], tStart[1], tStart[3]};
          int index = -1;
          std::vector<int>::iterator it;
          if(faceIn(fa, &index)) 
          {
            
/*
  is index in another tet in f2t

*/
            for(int ii=0; ii < (int)f2t.size(); ii++)
            { 
              if(ii != n) {
                it = find (f2t[ii].begin(), f2t[ii].end(), index);
                if (it != f2t[ii].end()) 
                {
                  n = ii;
                  tStart = v2t[n];
                  break;
                }
              } 
            }
          }
        } else {
          if(signbit(orient3d(ait, pit, cit, dit)) 
                != signbit(detTet))
          {
            visited.push_back(n);
            std::vector<int> fa {tStart[0], tStart[2], tStart[3]};
            int index = -1;
            std::vector<int>::iterator it;
            if(faceIn(fa, &index)) 
            {

/*
  is index in another tet in f2t

*/
              for(int ii=0; ii < (int)f2t.size(); ii++)
              {  
                if(ii != n) {
                  it = find (f2t[ii].begin(), f2t[ii].end(), index);
                  if (it != f2t[ii].end()) 
                  {
                    n = ii;
                    tStart = v2t[n];
                    break;
                  }  
                } 
              }
            }
          } else {
            if(signbit(orient3d(pit, bit, cit, dit)) 
                != signbit(detTet))
            {
              visited.push_back(n);
              std::vector<int> fa {tStart[1], tStart[2], tStart[3]};
              int index = -1;
              std::vector<int>::iterator it;
              if(faceIn(fa, &index)) 
              { 

/* 
  is index in another tet in f2t

*/
                for(int ii=0; ii < (int)f2t.size(); ii++)
                {  
                  if(ii != n) {
                    it = find (f2t[ii].begin(), f2t[ii].end(), index);
                    if (it != f2t[ii].end()) 
                    {
                      n = ii;
                      tStart = v2t[n];
                      break;
                    }  
                  } 
                }
              }
            } else {
                
/* 
  point is inside

*/
                found_tet = true;
                if(found_tet) {
                  a = (int)v2t.size();
                }
              }
            }
          }

        }

        if(a > ((int)v2t.size()*4))
        {
/*
  cannot find tetrahedron
  remove point

*/
          points.erase(points.begin()+i);
          break;
        }

    } // end of while

    if(visited.size() > 0){
      visited.clear();
    }


/*
  check if new point lies directly on one of
  the already existing vertices

*/
    if(comparePoints3D(p, vertices[v2t[n][0]]) 
    || comparePoints3D(p, vertices[v2t[n][1]]) 
    || comparePoints3D(p, vertices[v2t[n][2]]) 
    || comparePoints3D(p, vertices[v2t[n][3]]))
    {

/* 
  do not insert point at all 

*/
    } else {

      // insert point p to vertices
      vertices.push_back(p);

/* 
  add new v2t
  get current tetrahedron and 
  remove it from vector to insert
  4 new ones

*/
      std::vector<int> tetcrr = v2t[n];
      std::vector<int> facecrr = f2t[n];
      v2t.erase(v2t.begin()+n);
      // (0,1,3,4), (1,2,3,4), (0,1,2,4), (0,2,3,4)
      double ux[3] ={vertices[tetcrr[0]].x,
        vertices[tetcrr[0]].y, vertices[tetcrr[0]].z};
      double uy[3] ={vertices[tetcrr[1]].x,
        vertices[tetcrr[1]].y, vertices[tetcrr[1]].z};
      double uv[3] ={vertices[tetcrr[2]].x,
        vertices[tetcrr[2]].y, vertices[tetcrr[2]].z};
      double uw[3] ={vertices[tetcrr[3]].x,
        vertices[tetcrr[3]].y, vertices[tetcrr[3]].z};
      double ur[3] ={vertices[vertices.size()-1].x,
      vertices[vertices.size()-1].y, vertices[vertices.size()-1].z};

/*
  check orientation of vertices
    before inserting

*/
      if(orient3d(ux, uy, uw, ur) > 0) {
        std::vector<int> tetnew1 {tetcrr[0],
          tetcrr[1], tetcrr[3], (int)vertices.size()-1};
          v2t.push_back(tetnew1);
          tetverts.push_back(tetnew1);

      } else {
        std::vector<int> tetnew1 {tetcrr[0],
          tetcrr[3], tetcrr[1], (int)vertices.size()-1};
          v2t.push_back(tetnew1);
          tetverts.push_back(tetnew1);

      }
      if(orient3d(uy, uv, uw, ur) > 0) {
        std::vector<int> tetnew2 {tetcrr[1],
          tetcrr[2], tetcrr[3], (int)vertices.size()-1};
          v2t.push_back(tetnew2);
          tetverts.push_back(tetnew2);

      } else {
        std::vector<int> tetnew2 {tetcrr[1],
          tetcrr[3], tetcrr[2], (int)vertices.size()-1};
          v2t.push_back(tetnew2);
          tetverts.push_back(tetnew2);

      }
      if(orient3d(ux, uy, uv, ur) > 0) {
        std::vector<int> tetnew3 {tetcrr[0],
          tetcrr[1], tetcrr[2], (int)vertices.size()-1};
          v2t.push_back(tetnew3);
          tetverts.push_back(tetnew3);

      } else {            
        std::vector<int> tetnew3 {tetcrr[0],
          tetcrr[2], tetcrr[1], (int)vertices.size()-1};
          v2t.push_back(tetnew3);
          tetverts.push_back(tetnew3);

      }
      if(orient3d(ux, uv, uw, ur) > 0) {
        std::vector<int> tetnew4 {tetcrr[0],
          tetcrr[2], tetcrr[3], (int)vertices.size()-1};
          v2t.push_back(tetnew4);
          tetverts.push_back(tetnew4);

      } else {
        std::vector<int> tetnew4 {tetcrr[0],
          tetcrr[3], tetcrr[2], (int)vertices.size()-1};
          v2t.push_back(tetnew4);
          tetverts.push_back(tetnew4);

      }

/*
  add new faces
  (numbers behind for orientation)

*/
      std::vector<int> facenew1_1 {tetcrr[0],
        tetcrr[1], tetcrr[3] }; //013
      std::vector<int> facenew1_2 {tetcrr[0],
        tetcrr[1], (int)vertices.size()-1 }; //014
      std::vector<int> facenew1_3 {tetcrr[0],
        tetcrr[3], (int)vertices.size()-1 }; //034
      std::vector<int> facenew1_4 {tetcrr[1],
        tetcrr[3], (int)vertices.size()-1 }; //134

      std::vector<int> facenew2_1 {tetcrr[1],
        tetcrr[2], (int)vertices.size()-1 }; //124
      std::vector<int> facenew2_2 {tetcrr[1],
        tetcrr[2], tetcrr[3] }; //123
      // 134 s.o.: 1-4
      std::vector<int> facenew2_4 {tetcrr[2],
        tetcrr[3], (int)vertices.size()-1 }; //234

      std::vector<int> facenew3_1 {tetcrr[0],
        tetcrr[1], tetcrr[2] }; //012     
      //014 s.o.: 1-2
      std::vector<int> facenew3_3 {tetcrr[0],
        tetcrr[2], (int)vertices.size()-1 }; //024  
      std::vector<int> facenew3_4 {tetcrr[1],
        tetcrr[2], (int)vertices.size()-1 }; //124

      //024 s.o.: 3-3
      //034 s.o.: 1-3
      std::vector<int> facenew4_3 {tetcrr[0],
        tetcrr[2], tetcrr[3] }; //023 
      std::vector<int> facenew4_4 {tetcrr[2],
        tetcrr[3], (int)vertices.size()-1 }; //234


      int pos1 = 0;
      std::vector<int> tetfacenew1;
      std::vector<int> tetfacenew2;
      std::vector<int> tetfacenew3;
      std::vector<int> tetfacenew4;
          
/*
  create faces
  - check if face exists
  - take position if it does

*/
      if(!faceIn(facenew1_1, &pos1)) {
        faces.push_back(facenew1_1);
        tetfacenew1.push_back(faces.size()-1);
      } else {
        tetfacenew1.push_back(pos1);
      }
      if(!faceIn(facenew1_2, &pos1)) {
        faces.push_back(facenew1_2);
        tetfacenew1.push_back(faces.size()-1);
      } else {
        tetfacenew1.push_back(pos1);
      }
      if(!faceIn(facenew1_3, &pos1)) {
        faces.push_back(facenew1_3);
        tetfacenew1.push_back(faces.size()-1);
      } else {
        tetfacenew1.push_back(pos1);
      }
      if(!faceIn(facenew1_4, &pos1)) {
        faces.push_back(facenew1_4);
        tetfacenew1.push_back(faces.size()-1);
      } else {
        tetfacenew1.push_back(pos1);
      }

/*
  faces for tetrahedron 2

*/  
      if(!faceIn(facenew2_1, &pos1)) {
        faces.push_back(facenew2_1);
        tetfacenew2.push_back(faces.size()-1);
      } else {
        tetfacenew2.push_back(pos1);
      }
      if(!faceIn(facenew2_2, &pos1)) {
        faces.push_back(facenew2_2);
        tetfacenew2.push_back(faces.size()-1);
      } else {
        tetfacenew2.push_back(pos1);
      }
      if(!faceIn(facenew1_4, &pos1)) {
        faces.push_back(facenew1_4);
        tetfacenew2.push_back(faces.size()-1);
      } else {
        tetfacenew2.push_back(pos1);
      }
      if(!faceIn(facenew2_4, &pos1)) {
        faces.push_back(facenew2_4);
        tetfacenew2.push_back(faces.size()-1);
      } else {
        tetfacenew2.push_back(pos1);
      }

/*
  faces for tetrahedron 3

*/ 
      if(!faceIn(facenew3_1, &pos1)) {
        faces.push_back(facenew3_1);
        tetfacenew3.push_back(faces.size()-1);
      } else {
        tetfacenew3.push_back(pos1);
      }
      if(!faceIn(facenew1_2, &pos1)) {
        faces.push_back(facenew1_2);
        tetfacenew3.push_back(faces.size()-1);
      } else {
        tetfacenew3.push_back(pos1);
      }
      if(!faceIn(facenew3_3, &pos1)) {
        faces.push_back(facenew3_3);
        tetfacenew3.push_back(faces.size()-1);
      } else {
        tetfacenew3.push_back(pos1);
      }
      if(!faceIn(facenew3_4, &pos1)) {
        faces.push_back(facenew3_4);
        tetfacenew3.push_back(faces.size()-1);
      } else {
        tetfacenew3.push_back(pos1);
      }

/*
  faces for tetrahedron 4

*/ 
      if(!faceIn(facenew3_3, &pos1)) {
        faces.push_back(facenew3_3);
        tetfacenew4.push_back(faces.size()-1);
      } else {
        tetfacenew4.push_back(pos1);
      }
      if(!faceIn(facenew1_3, &pos1)) {
        faces.push_back(facenew1_3);
        tetfacenew4.push_back(faces.size()-1);
      } else {
        tetfacenew4.push_back(pos1);
      }
      if(!faceIn(facenew4_3, &pos1)) {
        faces.push_back(facenew4_3);
        tetfacenew4.push_back(faces.size()-1);
      } else {
        tetfacenew4.push_back(pos1);
      }
      if(!faceIn(facenew4_4, &pos1)) {
        faces.push_back(facenew4_4);
        tetfacenew4.push_back(faces.size()-1);
      } else {
        tetfacenew4.push_back(pos1);
      }
          
/*
  push tetrahedrons in vector f2t
  and same tetrahedrons in 
  temporary vector tetfaces
  to check delaunay criteria

*/
      f2t.erase(f2t.begin()+n);
      f2t.push_back(tetfacenew1);
      f2t.push_back(tetfacenew2);
      f2t.push_back(tetfacenew3);
      f2t.push_back(tetfacenew4);
      tetfaces.push_back(tetfacenew1);
      tetfaces.push_back(tetfacenew2);
      tetfaces.push_back(tetfacenew3);
      tetfaces.push_back(tetfacenew4);

      if(tetfacenew1.size() > 0) {
        tetfacenew1.clear();
      }
      if(tetfacenew2.size() > 0) {
        tetfacenew2.clear();
      }
      if(tetfacenew3.size() > 0) {
        tetfacenew3.clear();
      }
      if(tetfacenew4.size() > 0) {
        tetfacenew4.clear();
      }
      if(facenew1_1.size() > 0) {
        facenew1_1.clear();
      }
      if(facenew1_2.size() > 0) {
        facenew1_2.clear();
      }
      if(facenew1_3.size() > 0) {
        facenew1_3.clear();
      }
      if(facenew1_4.size() > 0) {
        facenew1_4.clear();
      }
      if(facenew2_1.size() > 0) {
        facenew2_1.clear();
      }
      if(facenew2_2.size() > 0) {
        facenew2_2.clear();
      }
      if(facenew2_4.size() > 0) {
        facenew2_4.clear();
      }
      if(facenew3_1.size() > 0) {
        facenew3_1.clear();
      }
      if(facenew3_3.size() > 0) {
        facenew3_3.clear();
      }
      if(facenew3_4.size() > 0) {
        facenew3_4.clear();
      }
      if(facenew4_3.size() > 0) {
        facenew4_3.clear();
      }
      if(facenew4_4.size() > 0) {
        facenew4_4.clear();
      }

    }

    int f2tsize = (int)f2t.size();

/*
  check if the new tetrahedrons 
  meet delaunay critera

*/
    while((int)tetverts.size() > 0) {
      int siz = (int)tetverts.size();
      std::vector<int> ttt = tetverts.at(siz-1);
      tetverts.pop_back();
      f2tsize--;

/*
  get tetrahedron with same face opposite of p

*/
      int index_p = getIndex(p, vertices);
      int faceIndx;
      for(int gf=0; gf < (int)tetfaces[siz-1].size(); gf++)
      {

        auto it = std::find(std::begin(faces[tetfaces[siz-1][gf]]),
          std::end(faces[tetfaces[siz-1][gf]]), index_p);
        if(it == std::end(faces[tetfaces[siz-1][gf]])) {
          faceIndx = tetfaces[siz-1][gf];
          break;
        }
      }
      bool foundtet = false;
      int ind_tet2;
      int ind_d;
      for(int ai=0; ai < (int)f2t.size(); ai++)
      {
        auto it = std::find(std::begin(f2t[ai]),
          std::end(f2t[ai]), faceIndx);
        if(it != std::end(f2t[ai])) {
          if(ai != f2tsize) {
            ind_tet2 = ai;

            for(int zz=0; zz < (int)v2t[ai].size(); zz++)
            {
              if(faces[faceIndx][0] != v2t[ai][zz] 
              && faces[faceIndx][1] != v2t[ai][zz]
              && faces[faceIndx][2] != v2t[ai][zz]) {
                  ind_d = zz;
                  foundtet = true;
                  break;
                
              }
            }

            break;
          }
        }
      }

      tetfaces.pop_back();

/*
  found tet with point d

*/
      if(foundtet) {
        std::vector<int> tettt = v2t[ind_tet2];
        double pti[3] {vertices[ttt[3]].x,
          vertices[ttt[3]].y, vertices[ttt[3]].z};
        double ati[3] {vertices[ttt[0]].x,
          vertices[ttt[0]].y, vertices[ttt[0]].z};
        double bti[3] {vertices[ttt[1]].x,
          vertices[ttt[1]].y, vertices[ttt[1]].z};
        double cti[3] {vertices[ttt[2]].x,
          vertices[ttt[2]].y, vertices[ttt[2]].z};
        double dti[3] {vertices[tettt[ind_d]].x,
          vertices[tettt[ind_d]].y, vertices[tettt[ind_d]].z};

/* 
  Before insphere check if orientation
  of points is correct

*/
        float orient_1 = orient3d(ati, bti, cti, pti);
        float detInSphere;
        if(orient_1 < 0.0)
        {
          detInSphere = insphere(ati, cti, bti, pti, dti);
                      
        } else {
          detInSphere = insphere(ati, bti, cti, pti, dti);

        }

/*
  if point d lies inside sphere of tetrahedron abcp,
  check if it fulfills one of four other cases.
  If so, change ordering of points corresponding to
  the face

*/
        if(detInSphere > 0.0) 
        { 
/* 
  case 1: pd intersects area abc 
  perform Flip23

*/
          if(signbit(orient3d(ati, bti, cti, pti)) 
            != signbit(orient3d(ati,bti,cti, dti)))
          {
/*
  Flip23:
  get 'old' tetrahedrons and push three new one in vector

  find ttt and tettt 
  in v2t/f2t

*/
            std::sort(tettt.begin(), tettt.end());
            for(int comp1=0; comp1 < (int) v2t.size(); comp1++)
            {
              std::sort(v2t[comp1].begin(), v2t[comp1].end());
              if(v2t[comp1] == tettt)
              {
                v2t.erase(v2t.begin()+comp1);
                f2t.erase(f2t.begin()+comp1);

              }
            }

            std::sort(ttt.begin(), ttt.end());
            for(int comp=0; comp < (int)v2t.size(); comp++)
            {
              std::sort(v2t[comp].begin(), v2t[comp].end());
              if(v2t[comp] == ttt) {

                v2t.erase(v2t.begin()+comp);
                f2t.erase(f2t.begin()+comp);

                break;
              }
            }

/*
remove tettt from tetfaces and tetverts

*/
            std::sort(tettt.begin(), tettt.end());
            for(int au=0; au < (int)tetfaces.size(); au++)
            {
              std::sort(tetfaces[au].begin(), tetfaces[au].end());
              if(tetfaces[au] == tettt) {
                tetfaces.erase(tetfaces.begin()+au);
                tetverts.erase(tetverts.begin()+au);
              }
              
            }

/* 
  build new ones
  p = ttt[3], a = ttt[0], b = ttt[1],
  c = ttt[2], d = tettt[3]

*/
          double dx1[3] {vertices[ttt[0]].x,
            vertices[ttt[0]].y, vertices[ttt[0]].z};
          double dx2[3] {vertices[ttt[1]].x,
            vertices[ttt[1]].y, vertices[ttt[1]].z};
          double dx3[3] {vertices[ttt[2]].x,
            vertices[ttt[2]].y, vertices[ttt[2]].z};
          double dx4[3] {vertices[ttt[3]].x,
            vertices[ttt[3]].y, vertices[ttt[3]].z};
          double dx5[3] {vertices[tettt[ind_d]].x,
            vertices[tettt[ind_d]].y, vertices[tettt[ind_d]].z};

/*
  abdp

*/
          if(orient3d(dx1, dx2, dx5, dx4) > 0) {
            std::vector<int> pnew1 {ttt[0], ttt[1], tettt[ind_d], ttt[3]};
            v2t.push_back(pnew1);
            tetverts.push_back(pnew1);
            pnew1.clear();
          } else {
            std::vector<int> pnew1 {ttt[0], tettt[ind_d], ttt[1], ttt[3]};
            v2t.push_back(pnew1);
            tetverts.push_back(pnew1);
            pnew1.clear();
          }

/* 
  acdp

*/
          if(orient3d(dx1, dx3, dx5, dx4) > 0) {
            std::vector<int> pnew2 {ttt[0], ttt[2], tettt[ind_d], ttt[3]};
            v2t.push_back(pnew2);
            tetverts.push_back(pnew2);
            pnew2.clear();
          } else {
            std::vector<int> pnew2 {ttt[0], tettt[ind_d], ttt[2], ttt[3]};
            v2t.push_back(pnew2);
            tetverts.push_back(pnew2);
            pnew2.clear();
          }

/* 
  bcdp

*/
          if(orient3d(dx2, dx3, dx5, dx4) > 0) {
            std::vector<int> pnew3 {ttt[1], ttt[2], tettt[ind_d], ttt[3]};
            v2t.push_back(pnew3);
            tetverts.push_back(pnew3);
            pnew3.clear();
          } else {
            std::vector<int> pnew3 {ttt[1], tettt[ind_d], ttt[2], ttt[3]};
            v2t.push_back(pnew3);
            tetverts.push_back(pnew3);
            pnew3.clear();
          }

/*
  first tet with faces
  pac, acd, pcd, pad

*/
          std::vector<int> fnew1 {ttt[3], ttt[0], ttt[2]};
          std::vector<int> fnew2 {ttt[0], ttt[2], tettt[ind_d]};
          std::vector<int> fnew3 {ttt[3], ttt[2], tettt[ind_d]};
          std::vector<int> fnew4 {ttt[3], ttt[0], tettt[ind_d]};
          // pbc, bcd, pcd, pbd
          std::vector<int> fnew5 {ttt[3], ttt[1], ttt[2]};
          std::vector<int> fnew6 {ttt[1], ttt[2], tettt[ind_d]};
          // pdc s.o.: fnew3
          std::vector<int> fnew7 {ttt[3], ttt[1], tettt[ind_d]};
          // pab, abd, pad, pbd
          std::vector<int> fnew8 {ttt[3], ttt[0], ttt[1]};
          std::vector<int> fnew9 {ttt[0], ttt[1], tettt[ind_d]};
          // pad s.o.: fnew4
          // pbd s.o.: fnew7

          int pos1 = 0;
          std::vector<int> ftet;
          std::vector<int> ftet2;
          std::vector<int> ftet3;

/*
  build new faces
  check if combination already
  exists

  1. faces for tet 1

*/
          if(!faceIn(fnew1, &pos1)) {
            faces.push_back(fnew1);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);
          }
          if(!faceIn(fnew2, &pos1)) {
            faces.push_back(fnew2);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);

          }
          if(!faceIn(fnew3, &pos1)) {
            faces.push_back(fnew3);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);
          }
          if(!faceIn(fnew4, &pos1)) {
            faces.push_back(fnew4);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);
          }

/*
  faces for tet 2

*/
          if(!faceIn(fnew5, &pos1)) {
            faces.push_back(fnew5);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);
          }
          if(!faceIn(fnew6, &pos1)) {
            faces.push_back(fnew6);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);
          }
          if(!faceIn(fnew3, &pos1)) {
            faces.push_back(fnew3);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);
          }
          if(!faceIn(fnew7, &pos1)){
            faces.push_back(fnew7);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);
          }

/*
  faces for tet 3

*/
          if(!faceIn(fnew8, &pos1)){
            faces.push_back(fnew8);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);
          }
          if(!faceIn(fnew9, &pos1)){
            faces.push_back(fnew9);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);
          }
          if(!faceIn(fnew7, &pos1)){
            faces.push_back(fnew7);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);
          }
          if(!faceIn(fnew4, &pos1)) {
            faces.push_back(fnew4);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);
          }

/* 
  push in vectors
  and update f2tsize

*/
          f2t.push_back(ftet3);
          f2t.push_back(ftet);
          f2t.push_back(ftet2);

          f2tsize = (int)f2t.size();

          tetfaces.push_back(ftet3);
          tetfaces.push_back(ftet);
          tetfaces.push_back(ftet2);

          if(ftet.size() > 0) {
            ftet.clear();
          }
          if(ftet2.size() > 0) {
            ftet2.clear();
          }
          if(ftet3.size() > 0) {
            ftet3.clear();
          }

          }  else 
          {
              
/*
  case 2: pd does not intersect area abc and there is 
  a third tetrahedron abpd, 
  so all tetrahedrons share edge ab: Flip32

*/
          int pos;
          if(tetExists(ttt[0], ttt[1], ttt[3], tettt[ind_d], &pos)) {
            std::vector<int> tetabpd = v2t[pos];

            v2t.erase(v2t.begin() + pos);
            f2t.erase(f2t.begin() + pos);

/*
  find ttt in v2t/f2t

*/
            std::sort(ttt.begin(), ttt.end());
            for(int comp=0; comp < (int)v2t.size(); comp++)
            {
              std::sort(v2t[comp].begin(), v2t[comp].end());
              if(v2t[comp] == ttt) {

              // position is comp
                v2t.erase(v2t.begin()+comp);
                f2t.erase(f2t.begin()+comp);
                break;
              }
            }

/* 
  find tettt in v2t/f2t

*/
            std::sort(tettt.begin(), tettt.end());
            for(int comp=0; comp < (int)v2t.size(); comp++)
            {
              std::sort(v2t[comp].begin(), v2t[comp].end());
              if(v2t[comp] == tettt) {

              // position is comp
                v2t.erase(v2t.begin()+comp);
                f2t.erase(f2t.begin()+comp);
                break;
              }
            }

/*
  perform flip32
  build two new tetrahedrons from
  three old ones

*/
            double dx1[3] {vertices[ttt[0]].x,
              vertices[ttt[0]].y, vertices[ttt[0]].z};
            double dx2[3] {vertices[ttt[1]].x,
              vertices[ttt[1]].y, vertices[ttt[1]].z};
            double dx3[3] {vertices[ttt[2]].x,
              vertices[ttt[2]].y, vertices[ttt[2]].z};
            double dx4[3] {vertices[ttt[3]].x,
              vertices[ttt[3]].y, vertices[ttt[3]].z};
            double dx5[3] {vertices[tettt[ind_d]].x,
              vertices[tettt[ind_d]].y, vertices[tettt[ind_d]].z};

          if(orient3d(dx1, dx2, dx3, dx4) > 0) {
            std::vector<int> pnew1 {ttt[0], ttt[1], ttt[2], ttt[3]};
            v2t.push_back(pnew1);
            tetverts.push_back(pnew1);
            pnew1.clear();
          } else {
            std::vector<int> pnew1 {ttt[0], ttt[2], ttt[1], ttt[3]};
            v2t.push_back(pnew1);
            tetverts.push_back(pnew1);
            pnew1.clear();
          }
          if(orient3d(dx1, dx2, dx3, dx5) > 0) {
            std::vector<int> pnew2 {ttt[0], ttt[1], ttt[2], tettt[ind_d]};
            v2t.push_back(pnew2);
            tetverts.push_back(pnew2);
            pnew2.clear();
          } else {
            std::vector<int> pnew2 {ttt[0], ttt[2], ttt[1], tettt[ind_d]};
            v2t.push_back(pnew2);
            tetverts.push_back(pnew2);
            pnew2.clear();
          }

            std::vector<int> newf1 {ttt[3], ttt[0], ttt[2]};
            std::vector<int> newf2 {ttt[3], ttt[1], ttt[2]};
            std::vector<int> newf3 {ttt[3], ttt[0], ttt[1]};
            std::vector<int> newf4 {ttt[0], ttt[1], ttt[2]}; // in both

            std::vector<int> newf5 {ttt[0], ttt[2], tettt[ind_d]};
            std::vector<int> newf6 {ttt[1], ttt[2], tettt[ind_d]};
            std::vector<int> newf7 {ttt[0], ttt[1], tettt[ind_d]};

            int pos2;
            
/*
  check if face already exists
  if yes: take position of needed face
  if no: create new one

*/
            std::vector<int> ftet32_1;
            std::vector<int> ftet32_2;
            if(faceIn(newf1, &pos2)) {
              ftet32_1.push_back(pos2);
            } else {
              faces.push_back(newf1);
              ftet32_1.push_back(faces.size()-1);
            }
            if(faceIn(newf2, &pos2)) {
              ftet32_1.push_back(pos2);
            } else {
              faces.push_back(newf2);
              ftet32_1.push_back(faces.size()-1);
            }
            if(faceIn(newf3, &pos2)) {
              ftet32_1.push_back(pos2);
            } else {
              faces.push_back(newf3);
              ftet32_1.push_back(faces.size()-1);
            }
            if(faceIn(newf4, &pos2)) {
              ftet32_1.push_back(pos2);
            } else {
              faces.push_back(newf4);
              ftet32_1.push_back(faces.size()-1);
            }


            if(faceIn(newf4, &pos2)) {
              ftet32_2.push_back(pos2);
            } else {
              faces.push_back(newf4);
              ftet32_2.push_back(faces.size()-1);
            }
            if(faceIn(newf5, &pos2)) {
              ftet32_2.push_back(pos2);
            } else {
              faces.push_back(newf5);
              ftet32_2.push_back(faces.size()-1);
            }
            if(faceIn(newf6, &pos2)) {
              ftet32_2.push_back(pos2);
            } else {
              faces.push_back(newf6);
              ftet32_2.push_back(faces.size()-1);
            }
            if(faceIn(newf7, &pos2)) {
              ftet32_2.push_back(pos2);
            } else {
              faces.push_back(newf7);
              ftet32_2.push_back(faces.size()-1);
            }

/*
  push new builded tetrahedrons
  from faces in vectors

*/
            f2t.push_back(ftet32_1);
            f2t.push_back(ftet32_2);

            // update f2tsize
            f2tsize = (int)f2t.size();

            tetfaces.push_back(ftet32_1);
            tetfaces.push_back(ftet32_2);

            if(ftet32_1.size() > 0) {
              ftet32_1.clear();
            }
            if(ftet32_2.size() > 0) {
              ftet32_2.clear();
            }

            if(newf1.size() > 0) {
              newf1.clear();
            }
            if(newf2.size() > 0) {
              newf2.clear();
            }
            if(newf3.size() > 0) {
              newf3.clear();
            }
            if(newf4.size() > 0) {
              newf4.clear();
            }
            if(newf5.size() >  0) {
              newf5.clear();
            }
            if(newf6.size() > 0) {
              newf6.clear();
            }
            if(newf7.size() > 0) {
              newf7.clear();
            }
          }
          else if(orient3d(pti, ati, bti, dti) == 0.00 
          || orient3d(pti, bti, cti, dti) == 0.00
          || orient3d(pti, ati, cti, dti) == 0.00)
          {

/*
  case 3: line pd intersects edge of abc and there exist 
  two other tetrahedrons which share edges: flip44

  1. sorting the vectors

*/
            sort(ttt.begin(), ttt.end());
            sort(tettt.begin(), tettt.end());

/*
  declaring result vector to
  store the common elements

*/
            vector<int> v3(ttt.size() + tettt.size());

/*
  iterator to store return type

*/
            vector<int>::iterator it, end;

            end = set_intersection(
                ttt.begin(), ttt.end(),
                tettt.begin(), tettt.end(),
                v3.begin());

            std::vector<int> samep;
            for (it = v3.begin(); it != end; it++)
            {  
              if(*it != ttt[3] && *it != tettt[ind_d])
              {
                  samep.push_back(*it);
              } 
            }
            
            if((int)samep.size() == 2)
            {
              sort(samep.begin(), samep.end());
              std::vector<int> vv(4);
              int first = -1;
              int second = -1;

/*
  find two other tetrahedrons 
  with same points

*/
              for(int r=0; r < (int)v2t.size(); r++)
              {
                std::vector<int> v = v2t[r];
                sort(v.begin(), v.end());
                
                  std::set_intersection(v.begin(), v.end(),
                      samep.begin(), samep.end(),
                      std::back_inserter(vv));
                if(vv.size() == 2) {
                  first = r;
                  vv.clear();
                  for(int rr=r; rr < (int)v2t.size(); rr++)
                  {
                    std::vector<int> v = v2t[rr];
                    sort(v.begin(), v.end());
                
                    std::set_intersection(v.begin(), v.end(),
                        samep.begin(), samep.end(),
                        std::back_inserter(vv));
                    if(vv.size() == 2) {
                      second = rr;
                      break;
                    }

                  }
                  break;
                }
              }

/*
  found two other tetrahedrons

*/
              if(first >= 0 && second >= 0) 
              {
                std::vector<int> thitet = v2t[first];
                std::vector<int> foutet = v2t[second];

                if(first > second) {
                  v2t.erase(v2t.begin() + first);
                  f2t.erase(f2t.begin() + first);
                  v2t.erase(v2t.begin() + second);
                  f2t.erase(f2t.begin() + second);
                } else {
                  v2t.erase(v2t.begin() + second);
                  f2t.erase(f2t.begin() + second);
                  v2t.erase(v2t.begin() + first);
                  f2t.erase(f2t.begin() + first);
                }
                
/*
  find ttt in v2t/f2t
  find tettt in v2t/f2t

*/
                for(int comp=0; comp < (int)v2t.size(); comp++)
                {
                  if(v2t[comp] == ttt) {
                    v2t.erase(v2t.begin()+comp);
                    f2t.erase(f2t.begin()+comp);
                    break;
                  }
                }
                
                for(int comp=0; comp < (int)v2t.size(); comp++)
                {
                  if(v2t[comp] == tettt) {
                    v2t.erase(v2t.begin()+comp);
                    f2t.erase(f2t.begin()+comp);
                    break;
                  }
                }
              
                
/* 
  build 4 new tetrahedrons
  1 curr.a, curr.b, curr.c, t1.d
  2 curr.a, curr.b, curr.d, t1.d
  3 d, curr.b, curr.c, t1.d
  4 d, curr.b, curr.d, t1.d

*/
              double ux[3] ={vertices[ttt[0]].x,
                vertices[ttt[0]].y, vertices[ttt[0]].z};
              double uy[3] ={vertices[ttt[1]].x,
                vertices[ttt[1]].y, vertices[ttt[1]].z};
              double uv[3] ={vertices[ttt[2]].x,
                vertices[ttt[2]].y, vertices[ttt[2]].z};
              double uw[3] ={vertices[ttt[3]].x,
                vertices[ttt[3]].y, vertices[ttt[3]].z};
              double ur[3] ={vertices[tettt[ind_d]].x,
                vertices[tettt[ind_d]].y, vertices[tettt[ind_d]].z};
              double ut[3] ={vertices[thitet[0]].x,
                vertices[thitet[0]].y, vertices[thitet[0]].z};

/*
  build new tetrahedrons
  check order of vertices before push to vector

*/
              if(orient3d(ux, uy, ut, uw) > 0) {
                std::vector<int> newt1 {ttt[0], ttt[1],
                  thitet[0], ttt[3]};
                v2t.push_back(newt1);
                tetverts.push_back(newt1);
                newt1.clear();
              } else {
                std::vector<int> newt1 {ttt[0], thitet[0], ttt[1], ttt[3]};
                v2t.push_back(newt1);
                tetverts.push_back(newt1);
                newt1.clear();
              }
              if(orient3d(ur, ux, uy, ut) > 0) {
                std::vector<int> newt2 {tettt[ind_d], ttt[0],
                  ttt[1], thitet[0]};
                v2t.push_back(newt2);
                tetverts.push_back(newt2);
                newt2.clear();
              } else {
                std::vector<int> newt2 {tettt[ind_d], ttt[1],
                  ttt[0], thitet[0]};
                v2t.push_back(newt2);
                tetverts.push_back(newt2);
                newt2.clear();
              }
              if(orient3d(ux, uv, ut, uw) > 0) {
                std::vector<int> newt3 {ttt[0],
                  ttt[2], thitet[0], ttt[3]};
                v2t.push_back(newt3);
                tetverts.push_back(newt3);
                newt3.clear();
              } else {
                std::vector<int> newt3 {ttt[0],
                  thitet[0], ttt[2], ttt[3]};
                v2t.push_back(newt3);
                tetverts.push_back(newt3);
                newt3.clear();
              }
              if(orient3d(ur, ux, uv, ut) > 0) {
                std::vector<int> newt4 {tettt[ind_d],
                  ttt[0], ttt[2], thitet[0]};
                  v2t.push_back(newt4);
                  tetverts.push_back(newt4);
                  newt4.clear();
              } else {
                std::vector<int> newt4 {tettt[ind_d],
                  ttt[2], ttt[0], thitet[0]};
                  v2t.push_back(newt4);
                  tetverts.push_back(newt4);
                  newt4.clear();
              }

/*
  Build faces and push
  to vector

*/
              std::vector<int> newf1 {ttt[3], ttt[0], ttt[1]};
              std::vector<int> newf2 {ttt[3], ttt[1], thitet[0]};
              std::vector<int> newf3 {ttt[0], ttt[1], thitet[0]};
              std::vector<int> newf4 {ttt[3], ttt[0], thitet[0]};

              std::vector<int> newf5 {tettt[ind_d], ttt[0], ttt[1]};
              std::vector<int> newf6 {tettt[ind_d], ttt[1], thitet[0]};
              std::vector<int> newf7 {ttt[0], ttt[1], thitet[0]};
              std::vector<int> newf8 {tettt[ind_d], ttt[0], thitet[0]};

              std::vector<int> newf9 {ttt[3], ttt[0], ttt[2]};
              std::vector<int> newf10 {ttt[3], ttt[2], thitet[0]};
              std::vector<int> newf11 {ttt[0], ttt[3], thitet[0]};
              std::vector<int> newf12 {ttt[0], ttt[2], thitet[0]};

              std::vector<int> newf13 {tettt[ind_d], ttt[2], thitet[0]};
              std::vector<int> newf14 {tettt[ind_d], ttt[0], ttt[2]};
              std::vector<int> newf15 {tettt[ind_d], ttt[0], thitet[0]};
              std::vector<int> newf16 {ttt[0], ttt[2], thitet[0]};

              int pos2 = 0;
              std::vector<int> ftet44_1;
              std::vector<int> ftet44_2;
              std::vector<int> ftet44_3;
              std::vector<int> ftet44_4;

/*
  faces for tetrahedron 44-1

*/  
              if(!faceIn(newf1, &pos2)) {
                faces.push_back(newf1);
                ftet44_1.push_back(faces.size()-1);
              } else {
                ftet44_1.push_back(pos2);
              }
              if(!faceIn(newf2, &pos2)) {
                faces.push_back(newf2);
                ftet44_1.push_back(faces.size()-1);
              } else {
                ftet44_1.push_back(pos2);
              }
              if(!faceIn(newf3, &pos2)) {
                faces.push_back(newf3);
                ftet44_1.push_back(faces.size()-1);
              } else {
                ftet44_1.push_back(pos2);
              }
              if(!faceIn(newf4, &pos2)) {
                faces.push_back(newf4);
                ftet44_1.push_back(faces.size()-1);
              } else {
                ftet44_1.push_back(pos2);
              }

/*
  faces for tetrahedron 44-2

*/  
              if(!faceIn(newf5, &pos2)) {
                faces.push_back(newf5);
                ftet44_2.push_back(faces.size()-1);
              } else {
                ftet44_2.push_back(pos2);
              }
              if(!faceIn(newf6, &pos2)) {
                faces.push_back(newf6);
                ftet44_2.push_back(faces.size()-1);
              } else {
                ftet44_2.push_back(pos2);
              }
              if(!faceIn(newf7, &pos2)) {
                faces.push_back(newf7);
                ftet44_2.push_back(faces.size()-1);
              } else {
                ftet44_2.push_back(pos2);
              }
              if(!faceIn(newf8, &pos2)) {
                faces.push_back(newf8);
                ftet44_2.push_back(faces.size()-1);
              } else {
                ftet44_2.push_back(pos2);
              }

/*
  faces for tetrahedron 44-3

*/  
              if(!faceIn(newf9, &pos2)) {
                faces.push_back(newf8);
                ftet44_3.push_back(faces.size()-1);
              } else {
                ftet44_3.push_back(pos2);
              }
              if(!faceIn(newf10, &pos2)) {
                faces.push_back(newf10);
                ftet44_3.push_back(faces.size()-1);
              } else {
                ftet44_3.push_back(pos2);
              }
              if(!faceIn(newf11, &pos2)) {
                faces.push_back(newf11);
                ftet44_3.push_back(faces.size()-1);
              } else {
                ftet44_3.push_back(pos2);
              }
              if(!faceIn(newf12, &pos2)) {
                faces.push_back(newf12);
                ftet44_3.push_back(faces.size()-1);
              } else {
                ftet44_3.push_back(pos2);
              }

/*
  faces for tetrahedron 44-4

*/  
              if(!faceIn(newf13, &pos2)) {
                faces.push_back(newf13);
                ftet44_4.push_back(faces.size()-1);
              } else {
                ftet44_4.push_back(pos2);
              }
              if(!faceIn(newf14, &pos2)) {
                faces.push_back(newf14);
                ftet44_4.push_back(faces.size()-1);
              } else {
                ftet44_4.push_back(pos2);
              }
              if(!faceIn(newf15, &pos2)) {
                faces.push_back(newf15);
                ftet44_4.push_back(faces.size()-1);
              } else {
                ftet44_4.push_back(pos2);
              }
              if(!faceIn(newf16, &pos2)) {
                faces.push_back(newf16);
                ftet44_4.push_back(faces.size()-1);
              } else {
                ftet44_4.push_back(pos2);
              }
                    

              f2t.push_back(ftet44_1);
              f2t.push_back(ftet44_2);
              f2t.push_back(ftet44_3);
              f2t.push_back(ftet44_4);

/*
  update f2tsize

*/
              f2tsize = (int)f2t.size();

              tetfaces.push_back(ftet44_1);
              tetfaces.push_back(ftet44_2);
              tetfaces.push_back(ftet44_3);
              tetfaces.push_back(ftet44_4);

/*
  clear vectors
  which have been used to build faces

*/
              if(newf1.size() > 0) {
                newf1.clear();
              }
              if(newf2.size() > 0) {
                newf2.clear();
              }
              if(newf3.size() > 0) {
                newf3.clear();
              }
              if(newf4.size() > 0) {
                newf4.clear();
              }
              if(newf5.size() >  0) {
                newf5.clear();
              }
              if(newf6.size() > 0) {
                newf6.clear();
              }
              if(newf7.size() > 0) {
                newf7.clear();
              }
              if(newf8.size() > 0) {
                newf8.clear();
              }
              if(newf9.size() > 0) {
                newf9.clear();
              }
              if(newf10.size() >  0) {
                newf10.clear();
              }
              if(newf11.size() > 0) {
                newf11.clear();
              }
              if(newf12.size() > 0) {
                newf12.clear();
              }
              if(newf13.size() > 0) {
                newf13.clear();
              }
              if(newf14.size() > 0) {
                newf14.clear();
              }
              if(newf15.size() > 0) {
                newf15.clear();
              }
              if(newf16.size() > 0) {
                newf16.clear();
              }

              if(ftet44_1.size() > 0) {
                ftet44_1.clear();
              }
              if(ftet44_2.size() > 0) {
                ftet44_2.clear();
              }
              if(ftet44_3.size() > 0) {
                ftet44_3.clear();
              }
              if(ftet44_4.size() > 0) {
                ftet44_4.clear();
              }

                }
              }


          } else if(orient3d(ati, bti, cti, dti) == 0.00) 
          {
            
/*
  case 4: tetrahedron is flat
  perform Flip23
  get 'old' tetrahedrons and push three new one in vector

*/
            std::sort(tettt.begin(), tettt.end());
            for(int comp1=0; comp1 < (int) v2t.size(); comp1++)
            {
              std::sort(v2t[comp1].begin(), v2t[comp1].end());
              if(v2t[comp1] == tettt)
              {
                v2t.erase(v2t.begin()+comp1);
                f2t.erase(f2t.begin()+comp1);

              }
            }
            // find ttt in v2t/f2t
            std::sort(ttt.begin(), ttt.end());
            for(int comp=0; comp < (int)v2t.size(); comp++)
            {
              std::sort(v2t[comp].begin(), v2t[comp].end());
              if(v2t[comp] == ttt) {

                v2t.erase(v2t.begin()+comp);
                f2t.erase(f2t.begin()+comp);

                break;
              }
            }

            // remove tettt from tetfaces and tetverts
            std::sort(tettt.begin(), tettt.end());
            for(int au=0; au < (int)tetfaces.size(); au++)
            {
              std::sort(tetfaces[au].begin(), tetfaces[au].end());
              if(tetfaces[au] == tettt) {
                tetfaces.erase(tetfaces.begin()+au);
                tetverts.erase(tetverts.begin()+au);
              }
              
            }

/*
  build new ones
  p = ttt[3], a = ttt[0], b = ttt[1], c = ttt[2], d = tettt[3]

*/
          double dx1[3] {vertices[ttt[0]].x,
            vertices[ttt[0]].y, vertices[ttt[0]].z};
          double dx2[3] {vertices[ttt[1]].x,
            vertices[ttt[1]].y, vertices[ttt[1]].z};
          double dx3[3] {vertices[ttt[2]].x,
            vertices[ttt[2]].y, vertices[ttt[2]].z};
          double dx4[3] {vertices[ttt[3]].x,
            vertices[ttt[3]].y, vertices[ttt[3]].z};
          double dx5[3] {vertices[tettt[ind_d]].x,
            vertices[tettt[ind_d]].y, vertices[tettt[ind_d]].z};

          // abdp
          if(orient3d(dx1, dx2, dx5, dx4) > 0) {
            std::vector<int> pnew1 {ttt[0], ttt[1], tettt[ind_d], ttt[3]};
            v2t.push_back(pnew1);
            tetverts.push_back(pnew1);
            pnew1.clear();
          } else {
            std::vector<int> pnew1 {ttt[0], tettt[ind_d], ttt[1], ttt[3]};
            v2t.push_back(pnew1);
            tetverts.push_back(pnew1);
            pnew1.clear();
          }
          // acdp
          if(orient3d(dx1, dx3, dx5, dx4) > 0) {
            std::vector<int> pnew2 {ttt[0], ttt[2], tettt[ind_d], ttt[3]};
            v2t.push_back(pnew2);
            tetverts.push_back(pnew2);
            pnew2.clear();
          } else {
            std::vector<int> pnew2 {ttt[0], tettt[ind_d], ttt[2], ttt[3]};
            v2t.push_back(pnew2);
            tetverts.push_back(pnew2);
            pnew2.clear();
          }
          // bcdp
          if(orient3d(dx2, dx3, dx5, dx4) > 0) {
            std::vector<int> pnew3 {ttt[1], ttt[2], tettt[ind_d], ttt[3]};
            v2t.push_back(pnew3);
            tetverts.push_back(pnew3);
            pnew3.clear();
          } else {
            std::vector<int> pnew3 {ttt[1], tettt[ind_d], ttt[2], ttt[3]};
            v2t.push_back(pnew3);
            tetverts.push_back(pnew3);
            pnew3.clear();
          }

/* 
  first tet with faces
  pac, acd, pcd, pad

*/
          std::vector<int> fnew1 {ttt[3], ttt[0], ttt[2]};
          std::vector<int> fnew2 {ttt[0], ttt[2], tettt[ind_d]};
          std::vector<int> fnew3 {ttt[3], ttt[2], tettt[ind_d]};
          std::vector<int> fnew4 {ttt[3], ttt[0], tettt[ind_d]};
          // pbc, bcd, pcd, pbd
          std::vector<int> fnew5 {ttt[3], ttt[1], ttt[2]};
          std::vector<int> fnew6 {ttt[1], ttt[2], tettt[ind_d]};
          // pdc s.o.: fnew3
          std::vector<int> fnew7 {ttt[3], ttt[1], tettt[ind_d]};
          // pab, abd, pad, pbd
          std::vector<int> fnew8 {ttt[3], ttt[0], ttt[1]};
          std::vector<int> fnew9 {ttt[0], ttt[1], tettt[ind_d]};
          // pad s.o.: fnew4
          // pbd s.o.: fnew7

          int pos1 = 0;
          std::vector<int> ftet;
          std::vector<int> ftet2;
          std::vector<int> ftet3;
          
/*
  build faces

*/
          if(!faceIn(fnew1, &pos1)) {
            faces.push_back(fnew1);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);
          }
          if(!faceIn(fnew2, &pos1)) {
            faces.push_back(fnew2);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);

          }
          if(!faceIn(fnew3, &pos1)) {
            faces.push_back(fnew3);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);

          }
          if(!faceIn(fnew4, &pos1)) {
            faces.push_back(fnew4);
            ftet.push_back(faces.size()-1);
          } else {
            ftet.push_back(pos1);

          }


          if(!faceIn(fnew5, &pos1)) {
            faces.push_back(fnew5);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);

          }
          if(!faceIn(fnew6, &pos1)) {
            faces.push_back(fnew6);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);

          }
          if(!faceIn(fnew3, &pos1)) {
            faces.push_back(fnew3);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);

          }
          if(!faceIn(fnew7, &pos1)){
            faces.push_back(fnew7);
            ftet2.push_back(faces.size()-1);
          } else {
            ftet2.push_back(pos1);

          }


          if(!faceIn(fnew8, &pos1)){
            faces.push_back(fnew8);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);

          }
          if(!faceIn(fnew9, &pos1)){
            faces.push_back(fnew9);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);

          }
          if(!faceIn(fnew7, &pos1)){
            faces.push_back(fnew7);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);

          }
          if(!faceIn(fnew4, &pos1)) {
            faces.push_back(fnew4);
            ftet3.push_back(faces.size()-1);
          } else {
            ftet3.push_back(pos1);

          }

/*
  push in vectors

*/
          f2t.push_back(ftet3);
          f2t.push_back(ftet);
          f2t.push_back(ftet2);

          // update f2tsize
          f2tsize = (int)f2t.size();

          tetfaces.push_back(ftet3);
          tetfaces.push_back(ftet);
          tetfaces.push_back(ftet2);

          if(ftet.size() > 0) {
            ftet.clear();
          }
          if(ftet2.size() > 0) {
            ftet2.clear();
          }
          if(ftet3.size() > 0) {
            ftet3.clear();
          }
          }
          }
        }
      }
    }
  }

  if(tetfaces.size() > 0)
  {
    tetfaces.clear();
  }
  if(tetverts.size() > 0)
  {
    tetverts.clear();
  }
   
}

/*
  check if point p is already 
  in vector points

*/
bool
Convex3D::duplicateP(Point3D p) 
{
  for(size_t dup = 0; dup < points.size(); dup++)
  {
    if(points[dup].x == p.x && points[dup].y == p.y
     && points[dup].z == p.z) {
       return true;
     }
  }
  return false;
}

/*
  build voronoi diagramm 3d

*/
void
Convex3D::buildVoronoi3D(Stream<Rectangle<3>> rStream) {

    // get all points from input cuboids
    rStream.open();
    Rectangle<3>* next = rStream.request();
    while(next != 0){
      Point3D p = getCentre(next);
      // check for duplicates
      if(!duplicateP(p)) {
        points.push_back(getCentre(next));
      }
       // free memory
      delete next;
      next = rStream.request();
    }

    rStream.close();

    if(points.size() < 2)
    {
      const string errmsg = "Expected at least two points";
      return;
    }
    Convex3D* conv3d = new Convex3D();

    std::vector<Tetrahedron> tetravec;
    // create delaunay diagram with points
    createDelaunay(points, &tetravec);

    std::vector<std::vector<Point3D>> facesPoints;
    std::vector<std::tuple<int, int>> lines;
    std::vector<std::vector<int>> temp;
    std::vector<std::vector<int>> tempF;
    std::vector<Point3D> circumcenters;

/* 
  create voronoi diagram from delaunay diagram
  find all lines including point p from points

*/
    for(int t=0; t < (int)points.size(); t++)
    {
        int p_index;
        for(size_t id=0; id < vertices.size(); id++)
        {
          if(comparePoints3D(vertices[id], points[t]))
          {
            p_index = id;
            break;
          }
        }

        lines.clear();

        for(int z=0; z<(int)v2t.size(); z++)
        {
          std::vector<int> fin = v2t[z];
          if(fin[0] == p_index)
          {
             if(!insidevec(fin[0], fin[1], lines)) {
             lines.push_back(std::make_tuple(fin[0], fin[1])); }
             if(!insidevec(fin[0], fin[2], lines)) {
             lines.push_back(std::make_tuple(fin[0], fin[2])); }
             if(!insidevec(fin[0], fin[3], lines)) {
             lines.push_back(std::make_tuple(fin[0], fin[3]));
             }
          }
          else if(fin[1] == p_index) 
          {
            if(!insidevec(fin[1], fin[0], lines)) {
             lines.push_back(std::make_tuple(fin[1], fin[0])); }
             if(!insidevec(fin[1], fin[2], lines)) {
             lines.push_back(std::make_tuple(fin[1], fin[2])); }
             if(!insidevec(fin[1], fin[3], lines)) {
             lines.push_back(std::make_tuple(fin[1], fin[3])); 
            }
          }

          else if(fin[2] == p_index)
          {
            if(!insidevec(fin[2], fin[0], lines)) {
              lines.push_back(std::make_tuple(fin[2], fin[0])); }
              if(!insidevec(fin[2], fin[1], lines)) {
              lines.push_back(std::make_tuple(fin[2], fin[1])); }
              if(!insidevec(fin[2], fin[3], lines)) {
              lines.push_back(std::make_tuple(fin[2], fin[3]));
            }
          }

          else if(fin[3] == p_index)
          {
            if(!insidevec(fin[3], fin[0], lines)) {
             lines.push_back(std::make_tuple(fin[3], fin[0])); }
             if(!insidevec(fin[3], fin[1], lines)) {
             lines.push_back(std::make_tuple(fin[3], fin[1])); }
             if(!insidevec(fin[3], fin[2], lines)) {
             lines.push_back(std::make_tuple(fin[3], fin[2])); 
            }
          }

        }

        // find tetrahedrons to lines
        for(int n=0; n < (int)lines.size(); n++)
        {
              std::tuple<int, int> tup = lines.at(n); 
              int p1 = std::get<0>(tup);
              int p2 = std::get<1>(tup);
              for(int b=0; b < (int)v2t.size(); b++)
              {
                if(sameline(p1, p2, v2t[b]))
                {    
                  temp.push_back(v2t[b]);
                  tempF.push_back(f2t[b]);
                }
              }
              // sort tetrahedrons of temp
              // face to face
              std::vector<std::vector<int>> tempSorted;
              std::vector<int> faces = tempF[temp.size()-1];
              tempSorted.push_back(temp[temp.size()-1]);
              int x = temp.size()-1;
              while(temp.size() > 0)
              {
                bool foundface = false;
                faces = tempF[x];
                temp.erase(temp.begin()+x);
                tempF.erase(tempF.begin()+x);

                for(size_t st=0; st < faces.size(); st++)
                {
                  int face = faces[st];

                  for(int ai=0; ai < (int)tempF.size(); ai++)
                  {       
                      auto it = std::find(std::begin(tempF[ai]),
                      std::end(tempF[ai]), face);
                      if(it != std::end(tempF[ai])) {
                        tempSorted.push_back(temp[ai]);
                        foundface = true;
                        x = ai;
                        break;
                      }
                  }
                  if(foundface) { break; }  
                }

              }

/* 
  all tetrahedrons in vector which share line point\_tup[n]
  get circumcenter which builds vertex of face of polyhedron

*/
                for(int m=0; m< (int)tempSorted.size(); m++)
                {
                  Point3D vertex = getCircumcenter(vertices[tempSorted[m][0]],
                   vertices[tempSorted[m][1]], 
                  vertices[tempSorted[m][2]], vertices[tempSorted[m][3]]);
                  if(isfinite(vertex.x) && isfinite(vertex.y) 
                  && isfinite(vertex.z))
                  {
                    circumcenters.push_back(vertex);
                  }
                }
                  facesPoints.push_back(circumcenters);
                  tempSorted.clear();
                  circumcenters.clear();
        }

    
      // create polyhedron
      Polyhedron polyhedron = Polyhedron();
      std::vector<Point3D> face;
      polyhedron.setPolyId(t+1);
      for(int s=0; s< (int)facesPoints.size(); s++)
      {
        face.clear();
        
        for(int g=0; g<(int)facesPoints.at(s).size(); g++)
        {
          face.push_back(facesPoints.at(s).at(g));
        }
        if(!face.empty()) {
          polyhedron.faces.push_back(face);
        }
      }
      // save polyhedron in vector
      polyhedronvec.push_back(polyhedron);
      facesPoints.clear();
    }

    conv3d->setPolyhedronVector(polyhedronvec);
    delete conv3d;

    if(faces.size() > 0)
    {
      faces.clear();
    }
    if(lines.size() > 0)
    {
      lines.clear();
    }
    if(vertices.size() > 0) {
      vertices.clear();
    }
    if(v2t.size() > 0) {
      v2t.clear();
    }
    if(f2t.size() > 0) {
      f2t.clear();
    }
    if(tetravec.size() > 0)
    {
      tetravec.clear();
    }

}


/*
  8.2.1 cellNumVM

*/
int cellNum3DVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  Convex3D *convex3d = static_cast<Convex3D*>(args[0].addr);

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

/*
  mode == 1 => precise allocation; 
  mode == 2 => use Bboxes of convex3d cells
  In case the user did not specify the mode of 
  allocation, use Bboxes 

*/

  int mode = ((CcInt*)args[2].addr)->GetIntval();
  std::set<int> cell_ids;
  int bbox = 2;
  int precise = 1;
  result = qp->ResultStorage(s);
  collection::IntSet* res = (collection::IntSet*) result.addr;

  if(search_window_ptr == nullptr || convex3d == nullptr) {
    return 0;
  }

  if(mode > bbox || mode < precise) {
    mode = bbox;
  }

  cell_ids = cellNum3D(convex3d, search_window_ptr, mode);
  res->setTo(cell_ids);

  return 0;

}

/*
  8.2.2 smallestCommonCellnumVM

*/
int smallestCommonCellnum3DVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  Convex3D *convex3d = static_cast<Convex3D*>(args[0].addr);

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  Rectangle<3> *search_window_ptr_2
    = static_cast<Rectangle<3>*>( args[2].addr );

  CcInt* cellno_ptr = static_cast<CcInt*>(args[3].addr);
  int cellno = cellno_ptr->GetIntval();

  if(search_window_ptr == nullptr 
    || search_window_ptr_2 == nullptr 
    || convex3d == nullptr) {
    return -1;
  }

  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  bool boolval = false;

  std::set<int> intsetRect1 = cellNum3D(convex3d, search_window_ptr, 1);
  std::set<int> intsetRect2 = cellNum3D(convex3d, search_window_ptr_2, 1);


  std::vector<int> v(sizeof(intsetRect1)+ sizeof(intsetRect2));
  std::vector<int>::iterator it;

  it=std::set_intersection (intsetRect1.begin(), intsetRect1.end(),
     intsetRect2.begin(), intsetRect2.end(), v.begin());
  v.resize(it-v.begin());                      
  
  if(v.empty()) { 
    //no intersection between rectangles
    res->Set( true, boolval);
    return 0;
  }
      
  if(v[0] == cellno)
  {
    boolval = true;
    res->Set( true, boolval);
    return 0;
  }
      
  res->Set( true, boolval);

  return 0;

}

/*
  8.2.3 getcellvoronoi3DVM

*/
int getcellvoronoi3DVM(Word* args, Word& result, int message,
  Word& local, Supplier s)
{
  Convex3D *convex3d = static_cast<Convex3D*>(args[0].addr);
  
  CcInt* cellno_ptr = static_cast<CcInt*>(args[1].addr);
  int cellno = cellno_ptr->GetIntval();

  result = qp->ResultStorage( s );
  Rectangle<3> *res = (Rectangle<3>*) result.addr;

  std::vector<Polyhedron> polyvec = convex3d->getPolyhedronVector();

  for(size_t i = 0; i < polyvec.size(); i++)
  {
      if(polyvec[i].getPolyId() == cellno)
      {
        double min[3], max[3];
        Rectangle<3> bbox = createBBox3D(&polyvec[i]);

        min[0] = bbox.getMinX();
        min[1] = bbox.getMinY();
        min[2] = bbox.getMinZ();
        max[0] = bbox.getMaxX();
        max[1] = bbox.getMaxY();
        max[2] = bbox.getMaxZ();
        res->Set(true, min, max);
        return 0;
      }
  }
  


  return -1;

}

/*
  Valuemapping function for create 3D-Tree

*/
int voronoi3dVM( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {
    Stream<Rectangle<3>> input_rect_ptr(args[0]);
    result = qp->ResultStorage(s);
      ((Convex3D*)result.addr)->Set(
        input_rect_ptr);
      return 0;
}




 
   
/*

8.3 Specifications

*/


/*

createconvexSpec

*/
const string createconvexSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(points) x bool -> convex </text--->"
    "<text>_ createconvex </text--->"
    "<text>Creates a convex polygon in form of a point sequence "
    "starting from the left point in clockwise order."    
    "The input point sequence does not have to be ordered in that way. "
    "Returns the polygon "
    "if the input point sequence form a convex polygon. "
    "Otherwise undef is returned either if the point stream "
    "does not form a convex polygon or " 
    "any other error occurs. With the bool parameter, you can switch on/off "
    " the convex test. Be sure that your data form convex polygons when "
    "you switch it to FALSE. With TRUE the test is enabled </text--->"    
    "<text> query Kinos feed head [3] projecttransformstream[GeoData] "
    "createconvex[TRUE] </text--->"
    ") )";
    

/*

voronoiSpec

*/    
 const string voronoiSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>((stream (tuple (..., (ak1 Tk1),...))) x"
    " ak1 x ak2 x rect x bool"
    "-> (stream (tuple (..., (ak1 Tk1),..., "
    "(ak2 Tk2))))) </text--->"
    "<text>_ voronoi [ list ] </text--->"
    "<text>Expands each tuple of the relation with a convex polygon "
    " representing the voronoi cell that belongs to the "
    "centerpoint specified "    
    " with point value Tk1 of the attribute ak1. The complete voronoi "
    " diagramm is set up with "
    " all points of ak1 from all tuples, in other words with "
    "all points of the "
    " ak1 column of the relation. "
    "The value of ak2 ist the type convex forming a convex polygon "
    "representing the voronoi region "
    "that is corresponding to the point value of ak1. "     
    "The rect paramater specifies a rectangle "
    " forming the bounding box of the voronoi diagram. If the diagram fits "
    "in the box the rectangle values are used. The sequence of parameters "
    "for the rectangle values must be (minx, maxx, miny, maxy). "
    " If the diagram do not fit in the rectangle box, a default box is used "
    " derived from the expansion of the voronoi diagram." 
    " With the last parameter you can switch on/off the convex polygon test. "
    " When TRUE is set the contructed polygons are explicit tested "
    " if they are convex. Choose FALSE, when you are sure about "
    "that property and "
    " you want to leave out the test.</text--->"    
    "<text> query testrel feed voronoi [p, conv, "
    "[const rect value (-10 9 2 )], TRUE] </text--->" 
    ") )";
    

/*
cellNumSpec

*/
const string cellNumSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> Stream<Convex> x rect2 x int"
    "-> intset </text--->"
    "<text>_ cellnum[ list ] </text--->"
    "<text> Returns all cellnumbers of the created voronoi"
    " diagram which intersect with the given "
    " rectangle. </text--->"    
    "<text> query cellnum((convtest feed voronoi "
    " [Punkt, Conv, [const rect value (-10 10 -10 10)], "
    " FALSE] project[Conv] consume) feed "
    " projecttransformstream[Conv], "
    " rectangle2(-1.0, 4.0, 0.0, 2.0), 1) </text--->" 
    ") )";


const string smallestCommCellnumSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> Stream<Convex> x rect2 x rect2 x int"
    "-> bool </text--->"
    "<text>_ sccvoronoi [ list ] </text--->"
    "<text> Returns true if int equals "
    " the smallest common cellnumber "
    " of two rectangles in a voronoi diagram. First "
    " the voronoi diagram is created as in voronoi(). </text--->"    
    "<text> query sccvoronoi((convtest feed voronoi "
    " [Punkt, Conv, [const rect value (-10 10 -10 10)], FALSE] "
    " project[Conv] consume) feed projecttransformstream[Conv],"
    " rectangle2(-1.0, 4.0, 0.0, 2.0), "
    " rectangle2(5.0, 7.0, -0.5, 0.5), 5) </text--->" 
    ") )";

const string getcellvoronoiSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> Stream<Convex> x int "
    "-> rectangle<2> </text--->"
    "<text>_ getcellvoro [ ] </text--->"
    "<text>Returns the bbox of a cell to "
    " the given cellid </text--->"    
    "<text> query getcellvoro((convtest feed voronoi "
    " [Punkt, Conv, [const rect value (-10 10 -10 10)], FALSE] "
    " project[Conv] consume) feed projecttransformstream[Conv],"
    " 2  </text--->" 
    ") )";

const string voronoi3dSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> stream(rect3) x rect3"
    "-> (stream (convex3d) </text--->"
    "<text>_ voronoi3d [] </text--->"
    "<text> It calculates the centre of the cuboids "
    " and builds the voronoi diagram to this set of points. "
    " Returns a list of cell ids with the number of faces "
    " belonging to this cell ids." 
    " </text--->"    
    "<text> query [const rect3 value(-1 2 -3 6 0 2)] feed voronoi3d [ "
    "[const rect3 value (-10  10 9 20 2 10)]] </text--->" 
    ") )";    

const string cellNum3DSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>((stream (rect3) x rect3"
    "-> intset </text--->"
    "<text>_ cellnum3d[ list ] </text--->"
    "<text> Returns all cellnumbers of the created voronoi"
    " diagram which intersect with the given "
    " cuboid.  </text--->"    
    "<text> query cellnum3d([const rect3 value(-1 2 -3 6 0 2)]"
    " feed voronoi3d "
    " [const rect value (-10 10 -10 10 -10 10)], "
    " rectangle3(-1.0, 4.0, 0.0, 2.0, 1.0, 2.0)) </text--->" 
    ") )";

const string smallestCommCellnum3DSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>((stream(rect3) x rect3 x rect3 x int"
    "-> bool </text--->"
    "<text>_ sccvoronoi3d [ list ] </text--->"
    "<text> Returns true if int equals "
    " the smallest common cellnumber "
    " of two cuboids in a voronoi diagram. First "
    " the voronoi diagram is created as in voronoi(). </text--->"    
    "<text> query ssc_voronoi3d([const rect3 value(-1 2 -3 6 0 2)]"
    " feed voronoi3d "
    " [const rect value (-10 10 -10 10 -10 10)], "
    " rectangle3(-1.0, 4.0, 0.0, 2.0, 1.0, 2.0), "
    " rectangle3(-1.0, 2.0, 1.0, 2.0, 1.0, 2.0), 5) </text--->" 
    ") )";

const string getcellvoronoi3DSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> Stream<Convex3D> x int "
    "-> rectangle<3> </text--->"
    "<text>_ getcellvoro3d [ ] </text--->"
    "<text>Returns the bbox of a cell to "
    " the given cellid </text--->"    
    "<text> query getcellvoro3d((convtest feed voronoi "
    " [Punkt, Conv, [const rect value (-10 10 -10 10)], FALSE] "
    " project[Conv] consume) feed projecttransformstream[Conv],"
    " 2  </text--->" 
    ") )";


/*
 
10 Registration an initialization

*/   
    
    
/*
 
10.1 Registration of Types

*/

TypeConstructor Conv3DTC(
  // name of the type in SECONDO
  Convex3D:: BasicType(),
  // property function describing signature
  Convex3D::PropertyConvex3D,
  // Out and In functions
  Convex3D::OutConvex3D, Convex3D::InConvex3D,
  // SaveToList, RestoreFromList functions
  0,0,
  // object creation and deletion
  Convex3D::CreateConvex3D, Convex3D::DeleteConvex3D,
  // object open, save
  0, 0,
  // object close and clone
  Convex3D::Close, Convex3D::CloneConvex3D,
  // cast function
  0,
  // sizeof function
  Convex3D::SizeOfConvex3D,
  // kind checking function
  Convex3D::KindCheckConvex3D 
);


TypeConstructor ConvexTC(
  Convex::BasicType(),
  Convex::Property, Convex::Out, Convex::In, 0,0,
  Convex::Create, Convex::Delete,
  Convex::Open, Convex::Save, Convex::Close, Convex::Clone,
  Convex::Cast, Convex::Size, Convex::TypeCheck
);


/*
  
10.2 Operator instances

*/
Operator createconvex ( "createconvex",
                   createconvexSpec,
                   createconvexVM,
                   Operator::SimpleSelect,
                   createconvextypemap );

Operator voronoi  ( "voronoi",
                   voronoiSpec,
                   voronoiVM,
                   Operator::SimpleSelect,
                   voronoitypemap );
         
Operator cellnum ("cellnum",
                  cellNumSpec,
                  cellNumVM,
                  Operator::SimpleSelect,
                  cellnumvoronoitypemap );

Operator sccvoronoi ("sccvoronoi",
                        smallestCommCellnumSpec,
                        smallestCommonCellnumVM,
                        Operator::SimpleSelect,
                        sccvoronoitypemap );

Operator getcellvoronoi ( "getcellvoronoi",
                          getcellvoronoiSpec,
                          getcellvoronoiVM,
                          Operator::SimpleSelect, 
                          getcellvoronoitypemap);

Operator voronoi3d ( "voronoi3d",
                      voronoi3dSpec,
                      voronoi3dVM,  
                      Operator::SimpleSelect,
                      voronoi3dtypemap );  

Operator cellnum3d ("cellnum3d",
                  cellNum3DSpec,
                  cellNum3DVM,
                  Operator::SimpleSelect,
                  cellnumvoronoi3dtypemap );

Operator sccvoronoi3d ("sccvoronoi3d",
                        smallestCommCellnum3DSpec,
                        smallestCommonCellnum3DVM,
                        Operator::SimpleSelect,
                        sccvoronoi3dtypemap );  

Operator getcellvoronoi3d ( "getcellvoronoi3d",
                          getcellvoronoi3DSpec,
                          getcellvoronoi3DVM,
                          Operator::SimpleSelect, 
                          getcellvoronoi3dtypemap);
                                    
/*
  
10.3 Creating the Algebra 

*/
class ConvexAlgebra : public Algebra 
{
 public:
  ConvexAlgebra() : Algebra()
   

/*
  Registration of Types 
   
*/

  
  
  {
    AddTypeConstructor(&ConvexTC);
    ConvexTC.AssociateKind(Kind::DATA() );  

    AddTypeConstructor ( &Conv3DTC );
    Conv3DTC.AssociateKind(Kind::SIMPLE() ); 
   

/* 
 Registration of operators 
   
*/ 
    AddOperator( &createconvex);
    AddOperator( &voronoi);
    AddOperator( &cellnum);
    AddOperator( &sccvoronoi);
    AddOperator( &getcellvoronoi );
    AddOperator( &voronoi3d );
    AddOperator( &cellnum3d );
    AddOperator( &sccvoronoi3d );
    AddOperator( &getcellvoronoi3d );

 
    
  }
  
  ~ConvexAlgebra() {};
};    


    
}

/*
 end of namespace 
  
*/


/*
  
10.4 Initialization

*/ 

extern "C"
Algebra*
InitializeConvex2Algebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return new convex::ConvexAlgebra;
}
