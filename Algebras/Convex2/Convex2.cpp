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

/*std::vector<Convex>
Convex::getVoronoiVector() {
  return this->voroVec;
}
    
void 
Convex::setVoronoiVector(std::vector<Convex> voro_vec) {
  this->voroVec = voro_vec;
}*/



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

bool PointInPolygon(Point point, Convex* conv) 
{
    std::vector<Point> poly;

    for(int a = 0; a < (int)conv->size; a++) {
      Point p1;
      p1.Set(conv->value[a].GetX(), conv->value[a].GetY());
      poly.insert(poly.begin() + a, p1);
    }

  Rectangle<2> bbox = createBBox(conv);

  // Precheck: if point is not in bbox of polygon, then it is not in 
  // polygon either
  if(!insideRect(&bbox, std::make_tuple(point.GetX(), point.GetY())))
  {
    return false;
  }

  int i, j = 0;
  int nvert = (int)poly.size();
  bool c = false;

  for(i = 0, j = nvert - 1; i < nvert; j = i++) {
    double i_y = poly[i].GetY();
    double j_y = poly[j].GetY();
    double i_x = poly[i].GetX();
    double j_x = poly[j].GetX();

    if( ( (i_y > point.GetY() ) != (j_y > point.GetY()) ) &&
        (point.GetX() < (j_x - i_x) * (point.GetY() - i_y) / (j_y - i_y) + i_x)
      ) {
        c = !c;
      }
  }

  return c;
}


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

    /*
      L1 = (x0,y0) + t*(x1-x0, y1-y0)
      L2 = (x2,y2) + u*(x3-x2, y3-y2)

      t = (x0-x2)*(y2-y3)-(y0-y2)*(x2-x3) / (x0-x1)*(y2-y3)-(y0-y1)*(x2-x3)
      u = (x0-x1)*(y0-y2)-(y0-y1)*(x0-x2) / (x0-x1)*(y2-y3)-(y0-y1)*(x2-x3)

      Px,Py = x0+t(x1-x0), y0 + t(y1-y0)
      Px,Py = x2+u(x3-x2), y2 + u(y3-y2)

      parallel or coincident => denom is zero
      (x0-x1)*(y2-y3) - (y0-y1)*(x2-x3) = 0

    */

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
computePolygonCentroid(std::vector<std::tuple<double, double>> vertices,
   int vertexCount)
{
    std::tuple<double,double> centroid = {0, 0};
    double signedArea = 0.0;
    double x0 = 0.0; // Current vertex X
    double y0 = 0.0; // Current vertex Y
    double x1 = 0.0; // Next vertex X
    double y1 = 0.0; // Next vertex Y
    double a = 0.0;  // Partial signed area

    // For all vertices except last
    int i=0;
    for (i=0; i<vertexCount-1; ++i)
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
    // If one rectangle is on left side of other 
    if (tolefr1.x > boriba2.x || tolefr2.x > boriba1.x) 
        return false; 
  
    // If one rectangle is above other 
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
    Convex tmp = voroVec[i];

    std::vector<std::tuple<double, double>> polygon {};
    std::tuple<double, double> point;

    /*
      1. Check if middle of cell intersects with rectangle 
      (get middle of cell, check if it lies in rectangle)
      -> if yes: add cell_id to cell_ids -> done
      -> if no: check if edges intersect with rectangle
        -> build edges from points (ordered clockwise), check intersection
        -> if one intersect: add cell_id to cell_ids -> done
        -> if no edge intersects: check if rectangle is small and does
          not intersects with edges or centroid at all, but lies in cell
        -> if no intersects: go to next cell, don't add cell_id
    */

    double firstX = tmp.value[0].GetX();
    double firstY = tmp.value[0].GetY();

    point = std::make_tuple(firstX, firstY);
    polygon.insert(polygon.begin(), point);

    for(int a = 1; a < (int)tmp.size; a++) {
      point = std::make_tuple(tmp.value[a].GetX(), tmp.value[a].GetY());
      polygon.insert(polygon.begin() + a, point);
    }
    size_t vertexCount = polygon.size();
    std::tuple<double,double> centroid = 
      computePolygonCentroid(polygon, vertexCount);

    int in = 0;
    if(insideRect(search_window_ptr, centroid)){
      cell_ids->insert(tmp.getCellId());
      in += 1;
    } else if (PointInPolygon(center, &tmp)) {
      cell_ids->insert(tmp.getCellId());
      in += 1;
    } else {
      for(int e = 0; e < (int)tmp.size; e++) {
      Point a,b;
      a.Set(tmp.value[e].GetX(), tmp.value[e].GetY());
      if(e == (int)tmp.size-1) {
        b.Set(firstX, firstY);
      } else {
        b.Set(tmp.value[e+1].GetX(), tmp.value[e+1].GetY());

      }
      // Point of intersection i_x, i_y
      double i_x = 0.0;
      double i_y = 0.0;

      /* call to getlineintersection with 2 points of line
       and two points of side of rectangle 
       (for each side of rectangle)
      */ 
      if(getLineIntersection(a,b, lebo, ribo, &i_x, &i_y) == 1 
        || getLineIntersection(a,b, lebo, leto, &i_x, &i_y) == 1 
        || getLineIntersection(a,b, ribo, rito, &i_x, &i_y) == 1
        || getLineIntersection(a,b, leto, rito, &i_x, &i_y) == 1) {
        cell_ids->insert(tmp.getCellId());
        in += 1;
        break;
      }
      }

      
    }

  }

  
  } else if (mode== bbox) {

    for(int b = 0; b < (int)voroVec.size(); b++) 
    {
      Convex* tmp = &voroVec[b];

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
       // cellId
     if(nl->ListLength(f) == 1)
     {
       Convex* co = new Convex(false);
      correct = true;
      co->cellId = nl->RealValue(f);
 
      co->SetDefined(false);
      res.addr = co;      
      return res;


     } else {
               
      correct = true;
      
      nl->WriteToString(lexprstr, f);
      
      Convex* co = new Convex(false);
   
      co->SetDefined(false);
      res.addr = co;      
     return res;
     }
      
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
   voroVec.push_back(r);
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

 8.1.5 toprightclass typemapping

*/
ListExpr trcvoronoitypemap (ListExpr args)
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    
    if(Rectangle<2>::checkType(first) && Rectangle<2>::checkType(second)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " rect x rect";

  return  listutils::typeError(errMsg);

}

ListExpr trcCellIdvoronoitypemap (ListExpr args)
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if(Stream<Convex>::checkType(first) && Rectangle<2>::checkType(second) 
      && Rectangle<2>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " Stream<Convex> x rect x rect";

  return  listutils::typeError(errMsg);

}

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
 8.1.2 voronoi3dtypemap
 
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

ListExpr cellnumvoronoi3dtypemap ( ListExpr args)
{
  if(nl->HasLength(args, 3)) {
    //ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (CcInt::checkType(third) && Rectangle<3>::checkType(second)) {
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
  return -1; // should never occur
}


/*

 8.1.9 smallest common cellnumber typemapping

*/
ListExpr sccvoronoi3dtypemap (ListExpr args)
{
  if(nl->HasLength(args, 4)) {
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);
    
    if(Rectangle<3>::checkType(second) && Rectangle<3>::checkType(third)
     && CcInt::checkType(fourth)) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " Convex3D x rect3 x rect3";

  return  listutils::typeError(errMsg);

}

/*

 8.1.10 toprightclass typemapping

*/
ListExpr trcCellIdvoronoi3dtypemap (ListExpr args)
{
  if(nl->HasLength(args, 3)) {
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    
    if(Rectangle<3>::checkType(second) && Rectangle<3>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " Convex3D x rect3 x rect3";

  return  listutils::typeError(errMsg);

}

ListExpr trcvoronoi3dtypemap (ListExpr args)
{
  if(nl->HasLength(args, 2)) {
    //ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    // check for polyhedron is missing
    if(Rectangle<3>::checkType(second)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " Polyhedron x rect3";

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
    Stream<Convex> input_convex(args[0]);
    input_convex.open();
    Convex* next = input_convex.request();

    while(next != 0){
      voroVec.push_back(*next);
      next = input_convex.request();
    }

    input_convex.close();

  Rectangle<2> *search_window_ptr
    = static_cast<Rectangle<2>*>( args[1].addr );

  /* mode == 1 => precise allocation; mode == 2 => use Bboxes of convex cells
    In case the user did not specify the mode of allocation, use Bboxes
  */
  int mode = ((CcInt*)args[2].addr)->GetIntval();
  std::set<int> cell_ids;
  int bbox = 2;
  int precise = 1;
  result = qp->ResultStorage(s);
  collection::IntSet* res = (collection::IntSet*) result.addr;

  if(search_window_ptr == nullptr /*|| convex == nullptr*/) {
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
    return 0;
  }

  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  bool boolval = false;

  std::set<int> intsetRect1;
  std::set<int> intsetRect2;


  cellNum(search_window_ptr, 1, &intsetRect1);
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
        res->Set( true, boolval);
        return 0;
  }
      
  res->Set( true, boolval);
  
  if(voroVec.size() > 0) {
    voroVec.clear();
  }

  return 0;

}

/*
8.2.2 TopRightClass returns cellId of cell which should be reported

*/
int trcCellIdvorononoiVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  Stream<Convex> input_convex(args[0]);
  input_convex.open();
  Convex* next = input_convex.request();

  while(next != 0){
    voroVec.push_back(*next);
    next = input_convex.request();
  }

  input_convex.close();

  Rectangle<2> *search_window_ptr
    = static_cast<Rectangle<2>*>( args[1].addr );

  Rectangle<2> *search_window_ptr_2
    = static_cast<Rectangle<2>*>( args[2].addr );

  if(search_window_ptr == nullptr 
    || search_window_ptr_2 == nullptr) {
    return 1;
  }

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;

  std::set<int> intsetRect1;
  std::set<int> intsetRect2;

  cellNum(search_window_ptr, 2, &intsetRect1);
  cellNum(search_window_ptr_2, 2, &intsetRect2);
  
  for (std::set<int>::iterator it=intsetRect1.begin(); 
    it!=intsetRect1.end(); ++it)
    std::cout << ' ' << *it;
  for (std::set<int>::iterator ip=intsetRect2.begin(); 
    ip!=intsetRect2.end(); ++ip)
    std::cout << ' ' << *ip;
  


  std::vector<int> v(sizeof(intsetRect1)+ sizeof(intsetRect2));
  std::vector<int>::iterator it;

  it=std::set_intersection (intsetRect1.begin(), intsetRect1.end(),
                 intsetRect2.begin(), intsetRect2.end(), v.begin());
  v.resize(it-v.begin());                      
  
  if(v.empty()) { 
    //no intersection between rectangles
    res->Set(0);
    if(voroVec.size() > 0) {
      voroVec.clear();
    }
    return -1;

  }

  for (it=v.begin(); it!=v.end(); ++it) {
    for(int c = 0; c < (int)voroVec.size(); c++) {
      if(voroVec[c].getCellId() == *it) {
        // get TRC Values
        int value_rect1 = 0;
        int value_rect2 = 0;

        // create bbox of voronoi cell
        Convex* tmp = &voroVec[c];

        Rectangle<2> bbox = createBBox(tmp);
        if ( search_window_ptr->getMaxX() >= bbox.getMaxX() )
          {value_rect1++;}
        if ( search_window_ptr->getMaxY() >= bbox.getMaxY() ) {
          value_rect1 += 2;}
        

        if ( search_window_ptr_2->getMaxX() >= bbox.getMaxX() )
          {value_rect2++;}
        if ( search_window_ptr_2->getMaxY() >= bbox.getMaxY() ) {
          value_rect2 += 2;}
        
        int value = value_rect1 & value_rect2;
        if (value == 0)
        {
          res->Set( *it );
          if(voroVec.size() > 0) {
            voroVec.clear();
          }
          return 0;

        }
    }
  }
  }
  

  if(voroVec.size() > 0) {
    voroVec.clear();
  }
  res->Set(0);
  return 0;

}

/*
8.2.2 TopRightClass returns toprightclassvalue

*/
int trcvorononoiVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  Rectangle<2> *search_window_ptr_C
    = static_cast<Rectangle<2>*>( args[0].addr );

  Rectangle<2> *search_window_ptr
    = static_cast<Rectangle<2>*>( args[1].addr );

  if(search_window_ptr == nullptr 
  || search_window_ptr_C == nullptr) {
    return -1;
  }

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;

  // get TRC Value
  int value_rect1 = 0;

  if ( search_window_ptr->getMaxX() >= search_window_ptr_C->getMaxX() )
    {value_rect1++;}
  if ( search_window_ptr->getMaxY() >= search_window_ptr_C->getMaxY() ) {
    value_rect1 += 2;}


  res->Set(value_rect1);

  return 0;    

}

int getcellvoronoiVM(Word* args, Word& result, int message,
  Word& local, Supplier s)
{
  Stream<Convex> input_convex(args[0]);
  input_convex.open();
  Convex* next = input_convex.request();

  while(next != 0){
    voroVec.push_back(*next);
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

8.2.3  createconvexVM

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

void
Polyhedron::setNeighbor(Polyhedron* neighbor_) {
  this->neighbor = neighbor_;
}

Polyhedron*
Polyhedron::getNeighbor() {
  return neighbor;
}

// cellId
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

Convex3D::Convex3D() {}

Convex3D::Convex3D(const Convex3D& g) {
  boundingBox = g.boundingBox;
}

Convex3D::Convex3D(Rectangle<3> &bounding_box) {
  boundingBox = &bounding_box;
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
Point3D
getCentre(Rectangle<3>* r) {
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
  calculates normal of three points

*/
std::vector<double> normal(std::vector<Point3D> pp)
{
  if(pp.size() > 2)
  {
    std::vector<double> dir1 {pp.at(1).x - pp.at(0).x,
       pp.at(1).y - pp.at(0).y, pp.at(1).z - pp.at(0).z };
    std::vector<double> dir2 {pp.at(2).x - pp.at(0).x,
       pp.at(2).y - pp.at(0).y, pp.at(2).z - pp.at(0).z };
    std::vector<double> cross = crossProduct(dir1, dir2);
    double d = std::sqrt(cross.at(0)*cross.at(0) + 
      cross.at(1)*cross.at(1) + cross.at(2)*cross.at(2));

    std::vector<double> res {cross.at(0)/d, cross.at(1)/d, cross.at(2)/d};
    return res;
  }

  return {0,0,0};
}

/*
  Returns true if Point p is in given polyhedron

*/
bool isInPolyhedron(Point3D p, Polyhedron poly)
{
  for(std::vector<Point3D> pp : poly.faces)
  {
    std::vector<double> p_pp {pp.at(0).x - p.x,
       pp.at(0).y - p.y, pp.at(0).z - p.z};
    //Point3D normal = normalize(pp);
    double dot = dotProduct(p_pp, normal(pp));
    dot /= std::sqrt(p_pp.at(0)*p_pp.at(0) 
      + p_pp.at(1)*p_pp.at(1) + p_pp.at(2)*p_pp.at(2));

    double bound = -1e-15;
    if(dot < bound)
      return false;
  }
  return true;
}


/*
  Returns true if points lie on the same side

*/
bool sameside(Point3D p1, Point3D p2, Point3D p3, Point3D p4, Point3D p)
{
  std::vector<double> subv2_v1 {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
  std::vector<double> subv3_v1 {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
  std::vector<double> subv4_v1 {p4.x - p1.x, p4.y - p1.y, p4.z - p1.z};
  std::vector<double> subp_v1 {p.x - p1.x, p.y - p1.y, p.z - p1.z};

  std::vector<double> normal = crossProduct(subv2_v1, subv3_v1);
  double dotproductV4 = dotProduct(normal, subv4_v1);
  double dotproductP = dotProduct(normal, subp_v1);

  // isgreater > 0: points are on the same side
  bool isgreater = (dotproductV4 * dotproductP) > 0;
  return isgreater;

}

/*
  Calculates determinant of 4x4 matrix

*/
float determinant4x4(Point3D a, Point3D b, Point3D c, Point3D x)
{
  float det = 0.0;
  /*
  calculate 4x4 matrix
  ax ay az 1
  bx by bz 1
  cx cy cz 1
  xx xy xz 1

  */

  det = 
  a.x * (b.y*(c.z*1 - 1*x.z) - b.z*(c.y*1 - 1*x.y) + 1*(c.y*x.z - c.z*x.y)) - 
  b.x * (a.y*(c.z*1 - 1*x.z) - a.z*(c.y*1 - 1*x.y) +1*(c.y*x.z - c.z*x.y)) + 
  c.x * (a.y*(b.z*1 - 1*x.z) - a.z*(b.y*1 - 1*x.y) +1*(b.y*x.z - b.z*x.y)) -
  x.x * (a.y*(b.z*1 - 1*c.z) - a.z*(b.y*1 - 1*c.y) +1*(b.y*c.z - b.z*c.y));

  return det;

}

/*
 Calculates if a point p lies inside a sphere (returns value > 0), outside a 
 sphere (returns value < 0) or directly on a sphere (returns value == 0)

*/
float insphereP(Point3D a, Point3D b, Point3D c, Point3D d, Point3D p) 
{
    float determinant = 0.0;

    // calculate px²+py²+pz²
    float a_square = a.x*a.x + a.y*a.y + a.z*a.z;
    float b_square = b.x*b.x + b.y*b.y + b.z*b.z;
    float c_square = c.x*c.x + c.y*c.y + c.z*c.z;
    float d_square = d.x*d.x + d.y*d.y + d.z*d.z;
    float p_square = p.x*p.x + p.y*p.y + p.z*p.z;


    // calculate 5x5 matrix
    determinant = 
    a.x* (b.y*(c.z*d_square*1 + c_square*1*p.z + 1*d.z*p_square
       - p.z*d_square*1 - p_square*1*c.z - 1*d.z*c_square)  
        - c.y*(b.z*d_square*1 + b_square*1*p.z + 1*d.z*p_square
           - p.z*d_square*1 - p_square*1*b.z - 1*d.z*b_square)  
        + d.y*(b.z*c_square*1 + b_square*1*p.z + 1*c.z*p_square
           - p.z*c_square*1 - p_square*1*b.z - 1*c.z*b_square)  
        - p.y*(b.z*c_square*1 + b_square*1*d.z + 1*c.z*d_square
           - d.z*c_square*1 - d_square*1*b.z - 1*c.z*b_square))
    -b.x* (a.y*(c.z*d_square*1 + c_square*1*p.z + 1*d.z*p_square
       - p.z*d_square*1 - p_square*1*c.z - 1*d.z*c_square)  
        - c.y*(a.z*d_square*1 + a_square*1*p.z + 1*d.z*p_square
           - p.z*d_square*1 - p_square*1*a.z - 1*d.z*a_square) 
        + d.y*(a.z*c_square*1 + a_square*1*p.z + 1*c.z*p_square
           - p.z*c_square*1 - p_square*1*a.z - 1*c.z*a_square) 
        - p.y*(a.z*c_square*1 + a_square*1*d.z + 1*c.z*d_square
           - d.z*c_square*1 - d_square*1*a.z - 1*c.z*a_square))
    +c.x* (a.y*(b.z*d_square*1 + b_square*1*p.z + 1*d.z*p_square
       - p.z*d_square*1 - p_square*1*b.z - 1*d.z*b_square)  
        - b.y*(a.z*d_square*1 + a_square*1*p.z + 1*d.z*p_square
           - p.z*d_square*1 - p_square*1*a.z - 1*d.z*a_square)  
        + d.y*(a.z*b_square*1 + a_square*1*p.z + 1*b.z*p_square
           - p.z*b_square*1 - p_square*1*a.z - 1*b.z*a_square)  
        - p.y*(a.z*b_square*1 + a_square*1*d.z + 1*b.z*d_square
           - d.z*b_square*1 - d_square*1*a.z - 1*b.z*a_square))
    -d.x* (a.y*(b.z*c_square*1 + b_square*1*p.z + 1*c.z*p_square
       - p.z*c_square*1 - p_square*1*b.z - 1*c.z*b_square) 
        - b.y*(a.z*c_square*1 + a_square*1*p.z + 1*c.z*p_square
           - p.z*c_square*1 - p_square*1*a.z - 1*c.z*a_square)
        + c.y*(a.z*b_square*1 + a_square*1*p.z + 1*b.z*p_square
           - p.z*b_square*1 - p_square*1*a.z - 1*b.z*a_square)  
        - p.y*(a.z*b_square*1 + a_square*1*c.z + 1*b.z*c_square
           - c.z*b_square*1 - c_square*1*a.z - 1*b.z*a_square))
    + p.x*(a.y*(b.z*c_square*1 + b_square*1*d.z + 1*c.z*d_square
       - d.z*c_square*1 -d_square*1*b.z - 1*c.z*b_square)  
        - b.y*(a.z*c_square*1 + a_square*1*d.z + 1*c.z*d_square
           - d.z*c_square*1 - d_square*1*a.z - 1*c.z*a_square)  
        + c.y*(a.z*b_square*1 + b_square*1*d.z + 1*b.z*d_square
           - d.z*b_square*1 - d_square*1*a.z - 1*b.z*d_square)  
        - d.y*(a.z*b_square*1 + a_square*1*c.z + 1*b.z*c_square
           - c.z*b_square*1 - c_square*1*a.z - 1*b.z*c_square));



    return determinant;

}

/*
  Calculates determinat of a 3x3 matrix

*/
double det3x3(Point3D b, Point3D c, Point3D d)
{
  double det = b.x*c.y*d.z + c.x*d.y*b.z + d.x*b.y*c.z
     - d.x*c.y*b.z - c.x*b.y*d.z - b.x*d.y*c.z;
  return det;
}

/*
  Returns true if a point is inside of a given tetrahedron

*/
bool inTetrahedron(Point3D p1, Point3D p2, Point3D p3, Point3D p4, Point3D newP)
{

  double p11[3];
  p11[0] = p1.x;
  p11[1] = p1.y;
  p11[2] = p1.z;
  double p22[3];
  p22[0] = p2.x;
  p22[1] = p2.y;
  p22[2] = p2.z;
  double p33[3];
  p33[0] = p3.x;
  p33[1] = p3.y;
  p33[2] = p3.z;
  double p44[3];
  p44[0] = p4.x;
  p44[1] = p4.y;
  p44[2] = p4.z;
  double newpp[3];
  newpp[0] = newP.x;
  newpp[1] = newP.y;
  newpp[2] = newP.z;
  double d0 = orient3d(p11, p22, p33, p44);
  double d1 = orient3d(newpp, p22, p33, p44);
  double d2 = orient3d(p11, newpp, p33, p44);
  double d3 = orient3d(p11, p22, newpp, p44);
  double d4 = orient3d(p11, p22, p33, newpp);
  


  if(d0 == 0) // tetrahedron is coplanar
  {
    return false;
  }

  if(signbit(d0) == signbit(d1) && signbit(d0) == signbit(d2) &&
  signbit(d0) == signbit(d3) && signbit(d0) == signbit(d4))
  {
    return true;
  }

  return false;

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

void
createconvex3D (std::vector<Point> points) 
{
  /*
    Create Convex in 3D
    Input: Set of points
    Output: Convex

  */

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
    "(1.0 ('Face' ((1.0 2.0 1.0)(1.0 3.0 1.0)(4.0 3.0 1.0)"
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

// Out function
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
          lastFacesLstExpr = nl->Append(lastFacesLstExpr, nl->IntAtom(fac+1));
          //lastFacesLstExpr = nl->Append(lastFacesLstExpr, nl->Empty());
        } else {
          facesLstExpr = nl->OneElemList(nl->IntAtom(fac+1));
          //facesLstExpr = nl->OneElemList(nl->Empty());
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
             nl->OneElemList(faceLstExpr));
        
      }
      
      
      //facesLstExpr = nl->OneElemList(nl->IntAtom(poly.faces.size()));
      lastPolyLstExpr = nl->Append(lastPolyLstExpr, facesLstExpr);

    }
    return polyExpr;
    } else {
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  }
}

// In function
Word
Convex3D::InConvex3D( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct ) {

  Word w = SetWord(Address(0));
  try {
    Rectangle<3>* bbox;

    ListExpr bboxLstExpr;

    if ( nl->ListLength( instance ) == 2 ) {
      bboxLstExpr = nl->First(instance);
    } else {
      throw 1;
    }

    // fetch bounding box information from input
    if (nl->ListLength( bboxLstExpr ) == 6 ) {
      ListExpr left = nl->First(bboxLstExpr);
      ListExpr right = nl->Second(bboxLstExpr);
      ListExpr bottom = nl->Third(bboxLstExpr);
      ListExpr top = nl->Fourth(bboxLstExpr);
      ListExpr front = nl->Fifth(bboxLstExpr);
      ListExpr back = nl->Sixth(bboxLstExpr);

      if ( nl->IsAtom(left) && nl->AtomType(left) == RealType
          && nl->IsAtom(right) && nl->AtomType(right) == RealType
          && nl->IsAtom(bottom) && nl->AtomType(bottom) == RealType
          && nl->IsAtom(top) && nl->AtomType(top) == RealType
          && nl->IsAtom(front) && nl->AtomType(front) == RealType
          && nl->IsAtom(back) && nl->AtomType(back) == RealType) {

        double min[3], max[3];
        min[0] = nl->RealValue(left);
        min[1] = nl->RealValue(bottom);
        max[0] = nl->RealValue(right);
        max[1] = nl->RealValue(top);
        min[2] = nl->RealValue(front);
        max[2] = nl->RealValue(back);

        bbox = new Rectangle<3>(true, min, max);
      } else {
        throw 3;
      }

    } else {
      throw 2;
    }

    // temporary support structures
    std::map<int, int> cellRef;

    std::vector<Convex3D> cell_vec {};
    
    std::vector<Point3D> points_vec {};

    correct = true;
    Convex3D* conv = new Convex3D(*bbox);
    conv->setPointsVector(points_vec);

    w.addr = conv;
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

*/
Point3D getCircumcenter(Tetrahedron fin)
{
  // calculate middle of Tetrahedron with circumsphere
  double fina[3];
  double finb[3];
  double finc[3];
  double find[3];
  fina[0] = fin.a.x;
  fina[1] = fin.a.y;
  fina[2] = fin.a.z;
  finb[0] = fin.b.x;
  finb[1] = fin.b.y;
  finb[2] = fin.b.z;
  finc[0] = fin.c.x;
  finc[1] = fin.c.y;
  finc[2] = fin.c.z;
  find[0] = fin.d.x;
  find[1] = fin.d.y;
  find[2] = fin.d.z;

    float a = orient3d(fina, finb, finc, find);
    //build new points for Dx, Dy and Dz by discarding column xi (or yi or zi)
    float square_1 = fin.a.x*fin.a.x + fin.a.y*fin.a.y + fin.a.z*fin.a.z;
    float square_2 = fin.b.x*fin.b.x + fin.b.y*fin.b.y + fin.b.z*fin.b.z;
    float square_3 = fin.c.x*fin.c.x + fin.c.y*fin.c.y + fin.c.z*fin.c.z;
    float square_4 = fin.d.x*fin.d.x + fin.d.y*fin.d.y + fin.d.z*fin.d.z;

    double dx_1[3];
    dx_1[0] = square_1;
    dx_1[1] = fin.a.y;
    dx_1[2] = fin.a.z;

    double dx_2[3];
    dx_2[0] = square_2;
    dx_2[1] = fin.b.y;
    dx_2[2] = fin.b.z;

    double dx_3[3];
    dx_3[0] = square_3;
    dx_3[1] = fin.c.y;
    dx_3[2] = fin.c.z;

    double dx_4[3];
    dx_4[0] = square_4;
    dx_4[1] = fin.d.y;
    dx_4[2] = fin.d.z;

    float dx = orient3d(dx_1, dx_2, dx_3, dx_4);

    double dy_1[3];
    dy_1[0] = square_1;
    dy_1[1] = fin.a.x;
    dy_1[2] = fin.a.z;

    double dy_2[3];
    dy_2[0] = square_2;
    dy_2[1] = fin.b.x;
    dy_2[2] = fin.b.z;

    double dy_3[3];
    dy_3[0] = square_3;
    dy_3[1] = fin.c.x;
    dy_3[2] = fin.c.z;

    double dy_4[3];
    dy_4[0] = square_4;
    dy_4[1] = fin.d.x;
    dy_4[2] = fin.d.z;

    float dy = -orient3d(dy_1, dy_2, dy_3, dy_4);

    double dz_1[3];
    double dz_2[3];
    double dz_3[3];
    double dz_4[3];
    dz_1[0] = square_1;
    dz_1[1] = fin.a.x;
    dz_1[2] = fin.a.y;
    dz_2[0] = square_2;
    dz_2[1] = fin.b.x;
    dz_2[2] = fin.b.y;
    dz_3[0] = square_3;
    dz_3[1] = fin.c.x;
    dz_3[2] = fin.c.y;
    dz_4[0] = square_4;
    dz_4[1] = fin.d.x;
    dz_4[2] = fin.d.y;

    float dz = orient3d(dz_1, dz_2, dz_3, dz_4);

    if(a < 0)
    {
      a = orient3d(fina, finc, finb, find);
      dx = orient3d(dx_1, dx_3, dx_2, dx_4);
      dy = -orient3d(dy_1, dy_3, dy_2, dy_4);
      dz = orient3d(dz_1, dz_2, dz_3, dz_4);

    }

    Point3D circumcenter;
    circumcenter.x = (dx / (2*a));
    circumcenter.y = (dy / (2*a));
    circumcenter.z = (dz / (2*a));

    return circumcenter;
}


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
  Returns true if a tetrahedron t shares the face abc of another tetrahedron

*/
bool shareSameFace(Point3D a, Point3D b, Point3D c, Tetrahedron t)
{
    std::vector<Point3D> faceP {a, b, c};
    std::vector<Point3D> points3d_vec_tmp {t.a, t.b, t.c, t.d};
    std::vector<Point3D> points3d_vec_tmp_2 {t.a, t.b, t.c, t.d};
    for(int fa = 0; fa < (int)faceP.size(); fa++)
    {
      for(int v = 0; v < (int)points3d_vec_tmp.size(); v++)
      {
        if(comparePoints3D(points3d_vec_tmp.at(v), faceP.at(fa)))
        {
          // get position of point points3d_vec_tmp.at(v) in points3d_vec_tmp_2
          int i = getIndex(points3d_vec_tmp.at(v), points3d_vec_tmp_2);
          points3d_vec_tmp_2.erase(points3d_vec_tmp_2.begin()+i);
          break;
        }
      }
      
      if((int)points3d_vec_tmp_2.size() == 1)
      {
        return true;
      }
    }
    
    return false;

}

/*
  Returns true if two lines intersect

*/
bool linesInt3D(Point3D p0, Point3D p1, Point3D q0, Point3D q1)
{
  std::vector<double> dp {p1.x-p0.x, p1.y-p0.y, p1.z-p0.z}; // first line
  std::vector<double> dq {q1.x-q0.x, q1.y-q0.y, q1.z-q0.z}; // second line
  // difference
  std::vector<double> pq {q0.x-p0.x, q0.y-p0.y,q0.z-p0.z};

  double a = dotProduct(dp, dp);
  double b = dotProduct(dp, dq);
  double c = dotProduct(dq, dq);
  double d = dotProduct(dp, pq);
  double e = dotProduct(dq, pq);

  // find discriminant
  double dd = (a*c)-(b*b);

  if(dd == 0) //parallel
  {
    return false;
  } 
  else  // find parameters for the closest points on lines
  {
    double tt = (b*e - c*d) / dd;
    double uu = (a*e - b*d) / dd;

    // if parameters are not between 0 and 1, lines do not intersect
    if(tt > 1 || tt < 0 || uu > 1 || uu <0)
    {
      return false;
    }
    else 
    {
      // find distance between points
      std::vector<double> p_tt {p0.x+(tt*dp.at(0)),
           p0.y+(tt*dp.at(1)), p0.z+(tt*dp.at(2))};
      std::vector<double> q_uu {q0.x+(uu*dq.at(0)),
           q0.y+(uu*dq.at(1)), q0.z+(uu*dq.at(2))};
      double dist = ((q_uu.at(0)-p_tt.at(0))*(q_uu.at(0)-p_tt.at(0)) + 
        (q_uu.at(1)-p_tt.at(1))*(q_uu.at(1)-p_tt.at(1)) + 
        (q_uu.at(2)-p_tt.at(2))*(q_uu.at(2)-p_tt.at(2)));

      // if distance is == 0 or nearly 0: lines intersect
      if(dist < 0.000001f)
      {
        return true;
      }
    }
  }

  return false;
}

/*
  Returns true if two lines in 3d space intersect
  if yes: calculates point of intersection

*/
bool linesIntersect(Point3D p0, Point3D p1,
   Point3D q0, Point3D q1, std::vector<Point3D> *line)
{
  std::vector<double> dp {p1.x-p0.x, p1.y-p0.y, p1.z-p0.z};
  std::vector<double> dq {q1.x-q0.x, q1.y-q0.y, q1.z-q0.z};
  // difference
  std::vector<double> pq {q0.x-p0.x, q0.y-p0.y,q0.z-p0.z};

  double a = dotProduct(dp, dp);
  double b = dotProduct(dp, dq);
  double c = dotProduct(dq, dq);
  double d = dotProduct(dp, pq);
  double e = dotProduct(dq, pq);

  // find discriminant
  double dd = (a*c)-(b*b);

  if(dd == 0) //parallel
  {
    return false;
  } 
  else  // find parameters for the closest points on lines
  {
    double tt = (b*e - c*d) / dd;
    double uu = (a*e - b*d) / dd;

    // if parameters are not between 0 and 1, lines do not intersect
    if(tt > 1 || tt < 0 || uu > 1 || uu <0)
    {
      return false;
    }
    else 
    {
      // find distance between points
      std::vector<double> p_tt {p0.x+(tt*dp.at(0)),
           p0.y+(tt*dp.at(1)), p0.z+(tt*dp.at(2))};
      std::vector<double> q_uu {q0.x+(uu*dq.at(0)),
           q0.y+(uu*dq.at(1)), q0.z+(uu*dq.at(2))};
      double dist = ((q_uu.at(0)-p_tt.at(0))*(q_uu.at(0)-p_tt.at(0)) 
        + (q_uu.at(1)-p_tt.at(1))*(q_uu.at(1)-p_tt.at(1)) 
        + (q_uu.at(2)-p_tt.at(2))*(q_uu.at(2)-p_tt.at(2)));

      // if distance is == 0 or nearly 0: lines intersect
      if(dist < 0.000001f)
      {
        line->push_back(q1);
        line->push_back(q0);
        return true;
      }
    }
  }

  return false;
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
  Returns true if a pair of points is in a vector of point pairs

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
  Returns the distance between a 3d point and a line defined by 3d points l1 and l2

*/
double pointLineDist( Point3D P, Point3D L1, Point3D L2)
{
     std::vector<double> v {L2.x - L1.x, L2.y-L1.y, L2.z-L1.z};
     std::vector<double> w {P.x - L1.x, P.y -L1.y, P.z-L1.z};

     double c1 = dotProduct(w,v);
     double c2 = dotProduct(v,v);
     double b = c1 / c2;

     Point3D Pb;
     Pb.x = L1.x + b * v.at(0);
     Pb.y = L1.y + b * v.at(1);
     Pb.z = L1.z + b * v.at(2);
     
     double dist = (P.x-Pb.x)*(P.x -Pb.x) + (P.y-Pb.y)*(P.y-Pb.y)
                   + (P.z-Pb.z)*(P.z-Pb.z); 
     return dist;
}

/*
  Returns true if there is a tetrahedron in vector tetravec which shares the line ab with
  another tetrahedron
  if yes position of tetrahedron in vector is returned

*/
bool shareSameLine(Point3D a, Point3D b, Point3D c,
   Point3D p, std::vector<Tetrahedron> tetravec, int* pos_tet)
{
  std::vector<Point3D> line {a, b};
  for(int f=0; f < (int)tetravec.size(); f++)
  {
    // current tetrahedron from all tetrahedros in tetravec
    Tetrahedron t = tetravec[f];
    // two temporary vectors with points of current tetrahedron
    std::vector<Point3D> points3d_vec_tmp {t.a, t.b, t.c, t.d};
    std::vector<Point3D> points3d_vec_tmp_2 {t.a, t.b, t.c, t.d};

    // check for every point in faceP (points to proof) 
    // if they are equal to points of current tetrahedron
    for(int fa = 0; fa < (int)line.size(); fa++)
    {
      for(int v = 0; v < (int)points3d_vec_tmp.size(); v++)
      {
        if(comparePoints3D(points3d_vec_tmp.at(v), line.at(fa)))
        {
          // if the point is equal, remove it from vector 
          // (check if empty == done => tetrahedron has same values)
          int i = getIndex(points3d_vec_tmp.at(v), points3d_vec_tmp_2);
          points3d_vec_tmp_2.erase(points3d_vec_tmp_2.begin()+i);
          if(points3d_vec_tmp_2.size() == 2)
          {
            if((!comparePoints3D(points3d_vec_tmp_2.at(0), c) 
                && !comparePoints3D(points3d_vec_tmp_2.at(1), c)) &&
                (!comparePoints3D(points3d_vec_tmp_2.at(0), p) 
                && !comparePoints3D(points3d_vec_tmp_2.at(1), p)))
            {
              *pos_tet = f;
              return true;
            }
          }
        }
      }
    }            
  }
  return false;
}


/*
  Returns index of the tetrahedron in vector with the given 3d points

*/
int getTetrahedron(Point3D p, Point3D d, Point3D a, 
  Point3D b, std::vector<Tetrahedron> vectorTet)
{

  bool equal = false;
  std::vector<Point3D> faceP {p, a, b, d};
  for(int f=0; f < (int)vectorTet.size(); f++)
  {
      // current tetrahedron from all tetrahedros in tetravec
      Tetrahedron t = vectorTet[f];
      // two temporary vectors with points of current tetrahedron
      std::vector<Point3D> points3d_vec_tmp {t.a, t.b, t.c, t.d};
      std::vector<Point3D> points3d_vec_tmp_2 {t.a, t.b, t.c, t.d};
  
      // check for every point in faceP (points to proof)
      // if they are equal to points of current tetrahedron
      for(int fa = 0; fa < (int)faceP.size(); fa++)
      {
        for(int v = 0; v < (int)points3d_vec_tmp.size(); v++)
        {
          if(comparePoints3D(points3d_vec_tmp.at(v), faceP.at(fa)))
          {
            // if the point is equal, remove it from vector 
            // (check if empty == done => tetrahedron has same values)

            int i = getIndex(points3d_vec_tmp.at(v), points3d_vec_tmp_2);
            points3d_vec_tmp_2.erase(points3d_vec_tmp_2.begin()+i);
            equal = true;
            if((int)points3d_vec_tmp_2.size() == 0)
            {
              return f;
            }
          }
        }
          if(!equal)
          { //whole set of points must be equal, 
            //so if one is not, step to the next tetrahedron
            break;
          }
      }
     
  }
  return -1;

}

/*
  Returns true if a segment pd intersects with an area abc
  in case of intersection: returns point of intersection

*/
#define EPSILON 0.000001f
bool segmentIntersectsArea(Point3D p, Point3D d, Point3D a,
     Point3D b, Point3D c, Point3D *point)
{ 
    Point3D e0 {b.x-a.x, b.y-a.y, b.z-a.z};
    Point3D e1 {c.x-a.x, c.y-a.y, c.z-a.z};

    Point3D dir {d.x-p.x, d.y-p.y, d.z-p.z};
    Point3D dir_norm = normalize(dir.x, dir.y, dir.z);

    std::vector<double> dir_norm_vec {dir_norm.x, dir_norm.y, dir_norm.z};
    std::vector<double> dir_vec{dir.x, dir.y, dir.z};
    std::vector<double> e1_vec {e1.x, e1.y, e1.z};
    std::vector<double> e0_vec {e0.x, e0.y, e0.z};

    std::vector<double> h = crossProduct(dir_norm_vec, e1_vec);
    double dp = dotProduct(e0_vec, h);

    if (dp > -EPSILON && dp < EPSILON) {
        return false;
    }

    Point3D s {p.x-a.x, p.y-a.y, p.z-a.z};
    std::vector<double> s_vec {s.x, s.y, s.z};
    const float f = 1.0f / dp;
    const float u = f * dotProduct(s_vec, h);

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    std::vector<double> q = crossProduct(s_vec, e0_vec);
    const float v = f * dotProduct(dir_norm_vec, q);

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    const float t = f * dotProduct(e1_vec, q);

    // segment intersection
    if (t > EPSILON && t < sqrtf(dotProduct(dir_vec, dir_vec))) { 
        if (point) {
            point->x = p.x + dir_norm.x*t;
            point->y = p.y + dir_norm.y*t;
            point->z = p.z + dir_norm.z*t;
          }
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


    /*
      1. Check if any point (take from points) of polyhedron 
        lies in cuboid -> add cell id (easy, saves time)
      2. Check if middle of cuboid intersects with polyhedron 
        cell (get middle of cuboid, check if it lies in cell)
      -> if yes: add cell_id to cell_ids -> done
      -> if no: check if edges of cuboid intersect with edges of polyhedron
        -> build edges from points (ordered clockwise) (points of 
          polyhedron faces and points of cuboid), check intersection
        -> if one intersect: add cell_id to cell_ids -> done
        -> if no intersects: go to next cell, don't add cell_id
    */

    if(insideCuboid(search_window_ptr, inputPoints.at(i)))
    {
      cell_ids.insert(tmp.getPolyId());
    }
    // check if center lies in polyhedron
    else if(isInPolyhedron(center, tmp))
    {
        cell_ids.insert(tmp.getPolyId());
    } 
    else
    {

      // get faces
      for(size_t f = 0; f < tmp.faces.size(); f++)
      {

        std::vector<std::tuple<double, double>> polygon {};
        std::tuple<double, double> point;
        std::vector<Point3D> face = tmp.faces.at(f);
        Point3D firstP = face.at(0);
        for(size_t e; e < face.size(); e++)
        {
          Point3D a,b;
          a.x = face.at(e).x;
          a.y = face.at(e).y;
          a.z = face.at(e).z;
          if(e == face.size() -1)
          {
            b = firstP;
          } else {
            b = {face.at(e+1).x, face.at(e+1).y, face.at(e+1).z};
          }

          //check for every point combination (=line) of cuboid 
          // if it intersects with line ab from polygon (polyhedron face)
          if(linesInt3D(a, b, lebofr, leboba) 
            || linesInt3D(a, b, ribofr, riboba) 
            || linesInt3D(a, b, lebofr, ribofr) 
            || linesInt3D(a, b, leboba, riboba) 
            || linesInt3D(a, b, lebofr, letofr) 
            || linesInt3D(a, b, ribofr, ritofr) 
            || linesInt3D(a, b, leboba, letoba) 
            || linesInt3D(a, b, riboba, ritoba) 
            || linesInt3D(a, b, letofr, letoba) 
            || linesInt3D(a, b, ritofr, ritoba) 
            || linesInt3D(a, b, letofr, ritofr) 
            || linesInt3D(a, b, letoba, ritoba)) 
            {

            cell_ids.insert(tmp.getPolyId());
            break;
          }

        }
     

      }

    }

    }
  
  } else if (mode== bbox) {

    for(size_t b = 0; b < polyvec.size(); b++) 
    {
      Polyhedron pol = polyvec.at(b);
      Rectangle<3> bbox = createBBox3D(&pol);

      // see if bbox intersects with rectangle
      // top left and bottom right of bbox
      Point3D tolefr, boriba;
      tolefr = {bbox.getMinX(), bbox.getMaxY(), bbox.getMinZ()};
      boriba = {bbox.getMaxX(), bbox.getMinY(), bbox.getMaxZ()};
      if(cuboidOverlap(tolefr, boriba, letofr, riboba)) {
        cell_ids.insert(pol.getPolyId());
      }
    }

  } else {
    cell_ids.insert(0);
  }

  return cell_ids;

}

bool Convex3D::faceIn(std::vector<int> face, int* pos)
{
  std::sort(face.begin(), face.end());
  for(int a = 0; a < (int)faces.size(); a++)
  {
    std::sort(faces[a].begin(), faces[a].end());
    if(faces[a] == face)
    {
      printf("\n SAME: %d, %d, %d | %d %d %d",
      faces[a][0], faces[a][1], faces[a][2],
      face[0], face[1], face[2]);
      
      *pos = a;
      return true;
    }
  }
  return false;
}

bool Convex3D::tetExists(int a, int b, int p, int d, int* pos)
{
  std::vector<int> tetr {a, b, p, d};
  sort(tetr.begin(), tetr.end());
  for(int v=0; v < (int)v2t.size(); v++)
  {
    sort(v2t[v].begin(), v2t[v].end());
    if(v2t[v] == tetr)
    {
      printf("\n in tetexists");
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

  // Sort points by z-value ---> check if it works to get z-value
  std::sort(pointsSortedbyZ.begin(), pointsSortedbyZ.end(), sortbyZ);
  double zmin = pointsSortedbyZ.front().z;
  double zmax = pointsSortedbyZ[pointsSortedbyZ.size()-1].z;

  /* 2. Build tetrahedron with min and max values
    2.1 Calculate bounding box of points with min and max values
    2.2 Build triangle around upper rectangle of cuboid (bounding box)
    2.3 Build tetrahedron with point above triangle
    2.4 Extend sides from point to triangle until it reaches 
        height of lower rectangle of cuboid
    2.5 result is the start tetrahedron  
  */
  // Tetrahedron has 4 edges a, b, c, d
  //Tetrahedron tet = Tetrahedron();

  // Build tetrahedron with initial rectangle around bounding box of points
  // get min, max values of each dimension
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

  /*tet.a.x = t4.x;
  tet.a.y = t4.y;
  tet.a.z = t4.z;

  tet.b.x = t5.x;
  tet.b.y = t5.y;
  tet.b.z = t5.z;

  tet.c.x = t6.x;
  tet.c.y = t6.y,
  tet.c.z = t6.z;

  tet.d.x = t7.x;
  tet.d.y = t7.y;
  tet.d.z = t7.z;*/

  // adding vertices t4-t7 to vertice vector
  vertices.push_back(t4);
  vertices.push_back(t5);
  vertices.push_back(t6);
  vertices.push_back(t7);

  // adding faces to faces vector
  // use indices of vertice vector
  std::vector<int> face1 {0, 1, 2};
  std::vector<int> face2 {0, 2, 3};
  std::vector<int> face3 {0, 1, 3};
  std::vector<int> face4 {1, 2, 3};

  faces.push_back(face1);
  faces.push_back(face2);
  faces.push_back(face3);
  faces.push_back(face4);

  // adding indices to tetrahedron vectors
  // vertices2tetrahedron
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
  //faces2tetrahedron
  std::vector<int> face2tet {0,1,2,3};
  f2t.push_back(face2tet);
  face2tet.clear();

  //std::vector<Tetrahedron> tempvec;
  //std::vector<Tetrahedron> flip14vec;

  std::vector<std::vector<int>> tetfaces;
  std::vector<std::vector<int>> tetverts;

  //tetverts.push_back(vert2tet);
  //tetfaces.push_back(face2tet);
  
  /*int count = 0;
  int counti = 0;
  int count3 = 0;
  int count4 = 0;*/

  // push start tetrahedron in vector
  //tetravec->push_back(tet);
  // Create Delaunay with tet and points
  // Insert point by point in initial tetrahedron
  for(int i = 0; i < (int)points.size(); i++)
  {
    Point3D p = points.at(i);
    //Tetrahedron tmptet;
    // find tetrahedron T containing p
    // visibility walk through diagram
    bool found_tet = false;
    int n = 0;
    int start = 0;
    std::vector<int> tetStart =f2t[n];
    std::vector<int> faceStart = faces[tetStart[start]];

    std::vector<int> tStart = v2t[n];
    double pit[3] = {p.x, p.y, p.z};

    printf("\n size v2t: %d", (int)v2t.size());
    printf("\n size f2t: %d", (int)f2t.size());

    printf("\n ");

          for(int z=0; z < (int)vertices.size(); z++)
          {
            printf(" (%.2f, %.2f, %.2f )",
             vertices[z].x, vertices[z].y, vertices[z].z);
          }

          printf("\n ");
          for(int t=0; t < (int)faces.size(); t++)
          {
            printf("t: %d (%d, %d, %d) ",
            t, faces[t][0], faces[t][1], faces[t][2]);
          }
                    printf("\n TET Vertices and Faces: ");

          for(int e=0; e < (int)v2t.size(); e++)
          {
            printf("\n V: %d, %d, %d, %d",
             v2t[e][0], v2t[e][1], v2t[e][2], v2t[e][3]);
            printf("\n F: %d, %d, %d, %d",
             f2t[e][0], f2t[e][1], f2t[e][2], f2t[e][3]);
          }

    int a = 0;
    std::vector<int> visited {}; // visited elements in v2t
    std::vector<int>::iterator it;

    while(found_tet == false)
    {
      //printf("\n in while");
      start = 0;
      a++;

      it = std::find(visited.begin(), visited.end(), n); 
      if (it != visited.end()) 
      { 
        // random start
        n = std::rand() % v2t.size();
        tStart = v2t[n];
      }

      printf("\n Vertices: %d %d %d %d",
       tStart[0], tStart[1], tStart[2], tStart[3]);
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
          printf("\n not same side -check #1-");
          visited.push_back(n);
          std::vector<int> fa {tStart[0], tStart[1], tStart[2]};
          int index = -1;
          std::vector<int>::iterator it;
          if(faceIn(fa, &index)) { //is index in another tet in f2t
            for(int ii=0; ii < (int)f2t.size(); ii++)
            { 
              if(ii != n) {
               it = find (f2t[ii].begin(), f2t[ii].end(), index);
               if (it != f2t[ii].end()) 
               {
                 n = ii;
                 printf("\n n: %d", n);
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
              printf("\n not same side -check2-");
              visited.push_back(n);
              std::vector<int> fa {tStart[0], tStart[1], tStart[3]};
              int index = -1;
              std::vector<int>::iterator it;
              if(faceIn(fa, &index)) { //is index in another tet in f2t
              for(int ii=0; ii < (int)f2t.size(); ii++)
              { 
                if(ii != n) {
                  it = find (f2t[ii].begin(), f2t[ii].end(), index);
                  if (it != f2t[ii].end()) 
                  {
                    n = ii;
                                     printf("\n n: %d", n);

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
                printf("\n not same side -check3-");
                visited.push_back(n);
                std::vector<int> fa {tStart[0], tStart[2], tStart[3]};
                int index = -1;
                std::vector<int>::iterator it;
                if(faceIn(fa, &index)) { //is index in another tet in f2t
                for(int ii=0; ii < (int)f2t.size(); ii++)
                {  
                  if(ii != n) {
                    it = find (f2t[ii].begin(), f2t[ii].end(), index);
                    if (it != f2t[ii].end()) 
                    {
                      n = ii;
                                       printf("\n n: %d", n);

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
                printf("\n not same side -check4-");
                visited.push_back(n);
                std::vector<int> fa {tStart[1], tStart[2], tStart[3]};
                int index = -1;
                std::vector<int>::iterator it;
                if(faceIn(fa, &index)) { //is index in another tet in f2t
                for(int ii=0; ii < (int)f2t.size(); ii++)
                {  
                  if(ii != n) {
                    it = find (f2t[ii].begin(), f2t[ii].end(), index);
                    if (it != f2t[ii].end()) 
                    {
                      n = ii;
                                       printf("\n n: %d", n);

                      tStart = v2t[n];
                      break;
                    }  
                  } 
                }
                }
              } else {
                // point is inside
                found_tet = true;
                printf("\n found inside tet!");
                if(found_tet) {
                  a = (int)v2t.size();
                }
              }
              }
            }

        }

        if(a > ((int)v2t.size()*2))
        {
          // cannot find tetrahedron
          // remove point
          printf("\n point %d removed", i);
          points.erase(points.begin()+i);

        }

    } // end of while

    if(visited.size() > 0){
      visited.clear();
    }
    

    printf("\n Punkt: %.2f %.2f %.2f", p.x, p.y, p.z);
    printf("\n Tet new version: %.2f %.2f %.2f, %.2f %.2f " 
    " %.2f, %.2f %.2f %.2f, %.2f %.2f, %.2f",
     vertices[v2t[n][0]].x, vertices[v2t[n][0]].y, 
         vertices[v2t[n][0]].z, vertices[v2t[n][1]].x,
          vertices[v2t[n][1]].y, vertices[v2t[n][1]].z,
           vertices[v2t[n][2]].x, vertices[v2t[n][2]].y,
         vertices[v2t[n][2]].z, vertices[v2t[n][3]].x,
          vertices[v2t[n][3]].y, vertices[v2t[n][3]].z);

    if(comparePoints3D(p, vertices[v2t[n][0]]) 
    || comparePoints3D(p, vertices[v2t[n][1]]) 
    || comparePoints3D(p, vertices[v2t[n][2]]) 
    || comparePoints3D(p, vertices[v2t[n][3]]))
    {
            // do not insert point at all
            printf("\n point is equal to vertex");

    } else {
          // insert point p to vertices
          vertices.push_back(p);
          // add new v2t
          // get current tet
          std::vector<int> tetcrr = v2t[n];
          std::vector<int> facecrr = f2t[n];
          //printf("\n size v2t: %d, n: %d", (int)v2t.size(), n);
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
          // add new faces
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
          // 134 s.o => 1_4
          std::vector<int> facenew2_4 {tetcrr[2],
           tetcrr[3], (int)vertices.size()-1 }; //234

          std::vector<int> facenew3_1 {tetcrr[0],
           tetcrr[1], tetcrr[2] }; //012     
          //014 s.o. => 1_2
          std::vector<int> facenew3_3 {tetcrr[0],
           tetcrr[2], (int)vertices.size()-1 }; //024  
          std::vector<int> facenew3_4 {tetcrr[1],
           tetcrr[2], (int)vertices.size()-1 }; //124

          //024 s.o. => 3_3
          //034 s.o. => 1_3
          std::vector<int> facenew4_3 {tetcrr[0],
           tetcrr[2], tetcrr[3] }; //023 
          std::vector<int> facenew4_4 {tetcrr[2],
           tetcrr[3], (int)vertices.size()-1 }; //234

          
         

          int pos1 = 0;
          std::vector<int> tetfacenew1;
          std::vector<int> tetfacenew2;
          std::vector<int> tetfacenew3;
          std::vector<int> tetfacenew4;

          if(!faceIn(facenew1_1, &pos1)) {
            faces.push_back(facenew1_1);
            tetfacenew1.push_back(faces.size()-1);
          } else {
            //printf("\n pos: %d",pos1);
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
          

          /*faces.push_back(facenew1); //size-6
          faces.push_back(facenew2); //size-5
          faces.push_back(facenew3); //size-4
          faces.push_back(facenew4); //size-3
          faces.push_back(facenew5); //size-2
          faces.push_back(facenew6); //size-1
          */
          // new faces tet
          /*std::vector<int> tetfacenew1 {facecrr[2],
           (int)faces.size()-6, (int)faces.size()-3,
            (int)faces.size()-1};
          std::vector<int> tetfacenew2 {facecrr[3],
           (int)faces.size()-1, (int)faces.size()-4,
            (int)faces.size()-2};
          std::vector<int> tetfacenew3 {facecrr[0],
           (int)faces.size()-6, (int)faces.size()-5,
            (int)faces.size()-4};
          std::vector<int> tetfacenew4 {facecrr[0],
           (int)faces.size()-5, (int)faces.size()-3,
            (int)faces.size()-2};
          */
          // remove tet from v2t and f2t and add new ones
          

          f2t.erase(f2t.begin()+n);
          f2t.push_back(tetfacenew1);
          f2t.push_back(tetfacenew2);
          f2t.push_back(tetfacenew3);
          f2t.push_back(tetfacenew4);
          tetfaces.push_back(tetfacenew1);
          tetfaces.push_back(tetfacenew2);
          tetfaces.push_back(tetfacenew3);
          tetfaces.push_back(tetfacenew4);

          for(int e=0; e < (int)f2t.size(); e++)
          {
            printf("\n F: %d, %d, %d, %d",
             f2t[e][0], f2t[e][1], f2t[e][2], f2t[e][3]);
          }

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

        while((int)tetverts.size() > 0) {
          int siz = (int)tetverts.size();
          //printf("\n tetverts size: %d", siz);
          std::vector<int> ttt = tetverts.at(siz-1);
          tetverts.pop_back();

          printf("\n ttt: %d, %d, %d, %d ",
           ttt[0], ttt[1], ttt[2], ttt[3]);
          printf("\n p: %.2f %.2f %.2f", p.x, p.y, p.z);
          // get tetrahedron with same face opposite of p
          int index_p = getIndex(p, vertices);
          printf("\n indexP: %d", index_p);
          printf("\n tetfaces: %d, %d, %d, %d,",
           tetfaces[siz-1][0], tetfaces[siz-1][1],
            tetfaces[siz-1][2], tetfaces[siz-1][3]);
          printf("\n ");
          for(int t=0; t < (int)faces.size(); t++)
          {
            printf("t: %d (%d, %d, %d) ",t,
             faces[t][0], faces[t][1], faces[t][2]);
          }
          int faceIndx;
          for(int gf=0; gf < (int)tetfaces[siz-1].size(); gf++)
          {
            //printf("\n p index index_p: %d", index_p);

            /*printf("\n Tetfaces[siz-1]: %d %d %d",
             faces[tetfaces[siz-1][gf]][0], faces[tetfaces[siz-1][gf]][1],
            faces[tetfaces[siz-1][gf]][2]);*/

            auto it = std::find(std::begin(faces[tetfaces[siz-1][gf]]),
             std::end(faces[tetfaces[siz-1][gf]]), index_p);
            if(it == std::end(faces[tetfaces[siz-1][gf]])) {
              faceIndx = tetfaces[siz-1][gf];
              printf("\n faceindx: %d", faceIndx);
              break;
            }
          }
          //int faceIndx = tetfaces[siz-1][0]; // it is the first one
          bool foundtet = false;
          int ind_tet2;
          int ind_d;
          //std::sort(tetfaces[siz-1].begin(), tetfaces[siz-1].end());
          for(int ai=0; ai < (int)f2t.size(); ai++)
          {
            auto it = std::find(std::begin(f2t[ai]),
             std::end(f2t[ai]), faceIndx);
            if(it != std::end(f2t[ai])) {
              //std::sort(f2t[ai].begin(), f2t[ai].end());
              //printf("\n F2tAI: %d %d %d %d", f2t[ai][0],
              // f2t[ai][1],f2t[ai][2],f2t[ai][3]);
              //printf("\n Tetfaces[siz-1]: %d %d %d %d",
              // tetfaces[siz-1][0], tetfaces[siz-1][1],
              // tetfaces[siz-1][2],tetfaces[siz-1][3]);
              if(f2t[ai] != tetfaces[siz-1]) {
                ind_tet2 = ai;
                  /*printf("\n v2t-ai: %d", v2t[ai][0]);
                  printf("\n v2t-ai: %d", v2t[ai][1]);
                  printf("\n v2t-ai: %d", v2t[ai][2]);
                  printf("\n v2t-ai: %d", v2t[ai][3]);*/

                for(int zz=0; zz < (int)v2t[ai].size(); zz++)
                {
                  //printf("\n f2t-ai: %d %d %d",
                  // faces[faceIndx][0], faces[faceIndx][1],
                  // faces[faceIndx][2]);
                  //printf("\n v2t-ai: %d", v2t[ai][zz]);

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
          if(foundtet) {
            printf("\n found tet");
            std::vector<int> tettt = v2t[ind_tet2];
            printf("\tettt: %d %d %d %d", tettt[0],
             tettt[1], tettt[2], tettt[3]);
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

            printf("\n P: %.2f %.2f %.2f",
             vertices[ttt[3]].x, vertices[ttt[3]].y, vertices[ttt[3]].z);
            printf("\n A: %.2f %.2f %.2f",
             vertices[ttt[0]].x, vertices[ttt[0]].y, vertices[ttt[0]].z);
            printf("\n B: %.2f %.2f %.2f",
             vertices[ttt[1]].x, vertices[ttt[1]].y, vertices[ttt[1]].z);
            printf("\n C: %.2f %.2f %.2f",
             vertices[ttt[2]].x, vertices[ttt[2]].y, vertices[ttt[2]].z);
            printf("\n D: %.2f %.2f %.2f",
             vertices[tettt[ind_d]].x, vertices[tettt[ind_d]].y,
              vertices[tettt[ind_d]].z);
            // Before insphere check if orientation of points is correct
            float orient_1 = orient3d(ati, bti, cti, pti);
            float detInSphere;
            if(orient_1 < 0.0)
            {
              detInSphere = insphere(ati, cti, bti, pti, dti);
                          
            } else {
              detInSphere = insphere(ati, bti, cti, pti, dti);

            }
            printf("\n DETINSPHERE: %.2f", detInSphere);

            if(detInSphere > 0.0) 
              // inside sphere or on it (on it also ok?! ==> no)
            { // #case 1: pd intersects area abc => Flip23
             
              printf("\n in detinsphere");
             if(signbit(orient3d(ati, bti, cti, pti)) 
                != signbit(orient3d(ati,bti,cti, dti))) 
             {
               for(int e=0; e < (int)f2t.size(); e++)
          {
            printf("\n V: %d, %d, %d, %d",
             v2t[e][0], v2t[e][1], v2t[e][2], v2t[e][3]);
            printf("\n F: %d, %d, %d, %d",
             f2t[e][0], f2t[e][1], f2t[e][2], f2t[e][3]);
          }
               // perform Flip23
               // get "old" tetrahedrons and push three new one in vector
               printf("\n in first ");
               printf("\n size before: %d, %d",
                (int)v2t.size(), (int)f2t.size());
              /* printf("\n v2t[a] : %d %d %d %d", v2t[a-1][0],
                v2t[a-1][1], v2t[a-1][2], v2t[a-1][3]);
               v2t.erase(v2t.begin()+a-1);
               f2t.erase(f2t.begin()+a-1);
               printf("\n size after: %d, %d",
                (int)v2t.size(), (int)f2t.size());
  */
               std::sort(tettt.begin(), tettt.end());
               for(int comp1=0; comp1 < (int) v2t.size(); comp1++)
               {
                 std::sort(v2t[comp1].begin(), v2t[comp1].end());
                 if(v2t[comp1] == tettt)
                 {
                   v2t.erase(v2t.begin()+comp1);
                   f2t.erase(f2t.begin()+comp1);
                  printf("\n removed 1");

                   printf("\n size after: %d, %d",
                    (int)v2t.size(), (int)f2t.size());

                 }
               }
               // find ttt in v2t/f2t
               std::sort(ttt.begin(), ttt.end());
               for(int comp=0; comp < (int)v2t.size(); comp++)
               {
                 printf("\n COMP: %d, %d, %d, %d",
                  v2t[comp][0], v2t[comp][1], v2t[comp][2], v2t[comp][3]);

                 std::sort(v2t[comp].begin(), v2t[comp].end());
                 if(v2t[comp] == ttt) {
                    printf("\n removed 2");

                  // position is comp
                  printf("\n size before: %d, %d",
                   (int)v2t.size(), (int)f2t.size());

                   v2t.erase(v2t.begin()+comp);
                   f2t.erase(f2t.begin()+comp);
                   printf("\n size after: %d, %d",
                    (int)v2t.size(), (int)f2t.size());

                   break;
                 }
               }

               // remove tettt from tetfaces & tetverts
               std::sort(tettt.begin(), tettt.end());
               for(int au=0; au < (int)tetfaces.size(); au++)
               {
                 std::sort(tetfaces[au].begin(), tetfaces[au].end());
                 if(tetfaces[au] == tettt) {
                   printf("\n removed from tetfaces/tetvertices");

                   tetfaces.erase(tetfaces.begin()+au);
                   tetverts.erase(tetverts.begin()+au);
                 }
                 
               }

               // build new ones
               // p = ttt[3], a = ttt[0], b = ttt[1], c = ttt[2], d = tettt[3]
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

              printf("\n TETS: %d, %d, %d, %d, %d",
               ttt[0], ttt[1], ttt[2], ttt[3], tettt[ind_d]);

              if(orient3d(dx4, dx1, dx2, dx5) > 0) {
                std::vector<int> pnew1 {ttt[3], ttt[0], ttt[1], tettt[ind_d]};
                v2t.push_back(pnew1);
                tetverts.push_back(pnew1);
                pnew1.clear();
              } else {
                std::vector<int> pnew1 {ttt[3], ttt[1], ttt[0], tettt[ind_d]};
                v2t.push_back(pnew1);
                tetverts.push_back(pnew1);
                pnew1.clear();
              }
              if(orient3d(dx4, dx1, dx3, dx5) > 0) {
                std::vector<int> pnew2 {ttt[3], ttt[0], ttt[2], tettt[ind_d]};
                v2t.push_back(pnew2);
                tetverts.push_back(pnew2);
                pnew2.clear();
              } else {
                std::vector<int> pnew2 {ttt[3], ttt[2], ttt[0], tettt[ind_d]};
                v2t.push_back(pnew2);
                tetverts.push_back(pnew2);
                pnew2.clear();
              }
              if(orient3d(dx4, dx2, dx3, dx5) > 0) {
                std::vector<int> pnew3 {ttt[3], ttt[1], ttt[2], tettt[ind_d]};
                v2t.push_back(pnew3);
                tetverts.push_back(pnew3);
                pnew3.clear();
              } else {
                std::vector<int> pnew3 {ttt[3], ttt[2], ttt[1], tettt[ind_d]};
                v2t.push_back(pnew3);
                tetverts.push_back(pnew3);
                pnew3.clear();
              }

              // first tet with faces
              // pac, acd, pcd, pad
              std::vector<int> fnew1 {ttt[3], ttt[0], ttt[2]};
              std::vector<int> fnew2 {ttt[0], ttt[2], tettt[ind_d]};
              std::vector<int> fnew3 {ttt[3], ttt[2], tettt[ind_d]};
              std::vector<int> fnew4 {ttt[3], ttt[0], tettt[ind_d]};
              // pbc, bcd, pcd, pbd
              std::vector<int> fnew5 {ttt[3], ttt[1], ttt[2]};
              std::vector<int> fnew6 {ttt[1], ttt[2], tettt[ind_d]};
              // pdc s.o. => fnew3
              std::vector<int> fnew7 {ttt[3], ttt[1], tettt[ind_d]};
              // pab, abd, pad, pbd
              std::vector<int> fnew8 {ttt[3], ttt[0], ttt[1]};
              std::vector<int> fnew9 {ttt[0], ttt[1], tettt[ind_d]};
              // pad s.o. => fnew4
              // pbd s.o. => fnew7
              int pos1 = 0;
              std::vector<int> ftet;
              std::vector<int> ftet2;
              std::vector<int> ftet3;
              if(!faceIn(fnew1, &pos1)) {
                faces.push_back(fnew1);
                ftet.push_back(faces.size()-1);
              } else {
                ftet.push_back(pos1);
                printf("\n pos: %d", pos1);
              }
              if(!faceIn(fnew2, &pos1)) {
                faces.push_back(fnew2);
                ftet.push_back(faces.size()-1);
              } else {
                ftet.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew3, &pos1)) {
                faces.push_back(fnew3);
                ftet.push_back(faces.size()-1);
              } else {
                ftet.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew4, &pos1)) {
                faces.push_back(fnew4);
                ftet.push_back(faces.size()-1);
              } else {
                ftet.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }


              if(!faceIn(fnew5, &pos1)) {
                faces.push_back(fnew5);
                ftet2.push_back(faces.size()-1);
              } else {
                ftet2.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew6, &pos1)) {
                faces.push_back(fnew6);
                ftet2.push_back(faces.size()-1);
              } else {
                ftet2.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew3, &pos1)) {
                faces.push_back(fnew3);
                ftet2.push_back(faces.size()-1);
              } else {
                ftet2.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew7, &pos1)){
                faces.push_back(fnew7);
                ftet2.push_back(faces.size()-1);
              } else {
                ftet2.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }


              if(!faceIn(fnew8, &pos1)){
                faces.push_back(fnew8);
                ftet3.push_back(faces.size()-1);
              } else {
                ftet3.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew9, &pos1)){
                faces.push_back(fnew9);
                ftet3.push_back(faces.size()-1);
              } else {
                ftet3.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew7, &pos1)){
                faces.push_back(fnew7);
                ftet3.push_back(faces.size()-1);
              } else {
                ftet3.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }
              if(!faceIn(fnew4, &pos1)) {
                faces.push_back(fnew4);
                ftet3.push_back(faces.size()-1);
              } else {
                ftet3.push_back(pos1);
                                printf("\n pos: %d", pos1);

              }

              // push in vectorss
              f2t.push_back(ftet3);
              f2t.push_back(ftet);
              f2t.push_back(ftet2);

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

             }  else {
               //#case 2: pd does not intersect area abc and there is 
              // a third tetrahedron abpd, 
              // so all tetrahedrons share ab => Flip32
              int pos;
              if(tetExists(ttt[0], ttt[1], ttt[3], tettt[ind_d], &pos)) {
                printf("\n in secondo case");
                std::vector<int> tetabpd = v2t[pos];
                                  printf("\n removed 1");

                v2t.erase(v2t.begin() + pos);
                f2t.erase(f2t.begin() + pos);
                printf("\n in second case,a: %d, pos: %d", a, pos);

                // find ttt in v2t/f2t
                std::sort(ttt.begin(), ttt.end());
                for(int comp=0; comp < (int)v2t.size(); comp++)
                {
                  std::sort(v2t[comp].begin(), v2t[comp].end());
                  if(v2t[comp] == ttt) {
                                      printf("\n removed 2");

                  // position is comp
                   v2t.erase(v2t.begin()+comp);
                   f2t.erase(f2t.begin()+comp);
                   break;
                 }
                }
                // find tettt in v2t/f2t
                std::sort(tettt.begin(), tettt.end());
                for(int comp=0; comp < (int)v2t.size(); comp++)
                {
                  std::sort(v2t[comp].begin(), v2t[comp].end());
                  if(v2t[comp] == tettt) {
                                      printf("\n removed 3");

                  // position is comp
                   v2t.erase(v2t.begin()+comp);
                   f2t.erase(f2t.begin()+comp);
                   break;
                 }
                }
                


                //flip32:
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

              if(orient3d(dx4, dx1, dx2, dx3) > 0) {
                std::vector<int> pnew1 {ttt[3], ttt[0], ttt[1], ttt[2]};
                v2t.push_back(pnew1);
                tetverts.push_back(pnew1);
                pnew1.clear();
              } else {
                std::vector<int> pnew1 {ttt[3], ttt[1], ttt[0], ttt[2]};
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
                std::vector<int> ftet32_1;
                std::vector<int> ftet32_2;
                if(faceIn(newf1, &pos2)) {
                  //printf("\n posi: %d", pos2);
                  ftet32_1.push_back(pos2);
                } else {
                  faces.push_back(newf1);
                  ftet32_1.push_back(faces.size()-1);
                }
                if(faceIn(newf2, &pos2)) {
                  //printf("\n posi: %d", pos2);
                  ftet32_1.push_back(pos2);
                } else {
                  faces.push_back(newf2);
                  ftet32_1.push_back(faces.size()-1);
                }
                if(faceIn(newf3, &pos2)) {
                  //printf("\n posi: %d", pos2);
                  ftet32_1.push_back(pos2);
                } else {
                  faces.push_back(newf3);
                  ftet32_1.push_back(faces.size()-1);
                }
                if(faceIn(newf4, &pos2)) {
                  //printf("\n posi: %d", pos2);
                  ftet32_1.push_back(pos2);
                } else {
                  faces.push_back(newf4);
                  ftet32_1.push_back(faces.size()-1);
                }


                if(faceIn(newf4, &pos2)) {
                  //printf("\n posi: %d", pos2);
                  ftet32_2.push_back(pos2);
                } else {
                  faces.push_back(newf4);
                  ftet32_2.push_back(faces.size()-1);
                }
                if(faceIn(newf5, &pos2)) {
                                    //printf("\n posi: %d", pos2);

                  ftet32_2.push_back(pos2);
                } else {
                  faces.push_back(newf5);
                  ftet32_2.push_back(faces.size()-1);
                }
                if(faceIn(newf6, &pos2)) {
                                    //printf("\n posi: %d", pos2);

                  ftet32_2.push_back(pos2);
                } else {
                  faces.push_back(newf6);
                  ftet32_2.push_back(faces.size()-1);
                }
                if(faceIn(newf7, &pos2)) {
                                    //printf("\n posi: %d", pos2);

                  ftet32_2.push_back(pos2);
                } else {
                  faces.push_back(newf7);
                  ftet32_2.push_back(faces.size()-1);
                }

                f2t.push_back(ftet32_1);
                f2t.push_back(ftet32_2);

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
                printf("\n in case 3");
                //#case 3: line pd intersects edge of abc and there exist 
                // two other tetrahedrons which share edges => flip44
                // 1. find common edge => share same points (ab, bc, or ac ?)
                //vectors

                //sorting the vectors
                sort(ttt.begin(), ttt.end());
                sort(tettt.begin(), tettt.end());

                //declaring result vector to
                //store the common elements
                vector<int> v3(ttt.size() + tettt.size());

                //iterator to store return type
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
                  // find two other tetrahedrons with same points
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
                  if(first >= 0 && second >= 0) {
                    std::vector<int> thitet = v2t[first];
                    std::vector<int> foutet = v2t[second];

                    if(first > second) {
                      printf("\n removed 1 and 2");

                      v2t.erase(v2t.begin() + first);
                      f2t.erase(f2t.begin() + first);
                      v2t.erase(v2t.begin() + second);
                      f2t.erase(f2t.begin() + second);
                    } else {
                       printf("\n removed 1 and 2");

                      v2t.erase(v2t.begin() + second);
                      f2t.erase(f2t.begin() + second);
                      v2t.erase(v2t.begin() + first);
                      f2t.erase(f2t.begin() + first);
                    }
                    
                    // find ttt in v2t/f2t
                    for(int comp=0; comp < (int)v2t.size(); comp++)
                    {
                      if(v2t[comp] == ttt) {
                      printf("\n removed 3");

                      // position is comp
                      v2t.erase(v2t.begin()+comp);
                      f2t.erase(f2t.begin()+comp);
                      break;
                      }
                    }
                   // find tettt in v2t/f2t
                    for(int comp=0; comp < (int)v2t.size(); comp++)
                    {
                      if(v2t[comp] == tettt) {
                      printf("\n removed 4");

                      // position is comp
                      v2t.erase(v2t.begin()+comp);
                      f2t.erase(f2t.begin()+comp);
                      break;
                      }
                    }
                    
                    // remove old tets, add 4 new ones => flip44
                    // build 4 new tetrahedrons
                    // 1: curr.a, curr.b, curr.c, t1.d
                    // 2: curr.a, curr.b, curr.d, t1.d
                    // 3: d, curr.b, curr.c, t1.d
                    // 4: d, curr.b, curr.d, t1.d

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

                    if(orient3d(uw, ux, uy, ut) > 0) {
                      std::vector<int> newt1 {ttt[3],
                       ttt[0], ttt[1], thitet[0]};
                      v2t.push_back(newt1);
                      tetverts.push_back(newt1);
                      newt1.clear();
                    } else {
                      std::vector<int> newt1 {ttt[3],
                       ttt[1], ttt[0], thitet[0]};
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
                    if(orient3d(uw, ux, uv, ut) > 0) {
                      std::vector<int> newt3 {ttt[3], ttt[0],
                       ttt[2], thitet[0]};
                      v2t.push_back(newt3);
                      tetverts.push_back(newt3);
                      newt3.clear();
                    } else {
                      std::vector<int> newt3 {ttt[3],
                       ttt[2], ttt[0], thitet[0]};
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

                    tetfaces.push_back(ftet44_1);
                    tetfaces.push_back(ftet44_2);
                    tetfaces.push_back(ftet44_3);
                    tetfaces.push_back(ftet44_4);

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


              } else if(orient3d(ati, bti, cti, pti) == 0.00) {
                printf("\n in case 4");
                //#case 4: tetrahedron is flat
                // perform flip32

                // remove ttt and tettt
                // find ttt in v2t/f2t
                std::sort(ttt.begin(), ttt.end());
                for(int comp=0; comp < (int)v2t.size(); comp++)
                {
                  std::sort(v2t[comp].begin(), v2t[comp].end());
                  if(v2t[comp] == ttt) {
                  // position is comp
                  printf("\n removed 1");
                  v2t.erase(v2t.begin()+comp);
                  f2t.erase(f2t.begin()+comp);
                  break;
                  }
                }
                // find tettt in v2t/f2t
                std::sort(tettt.begin(), tettt.end());
                for(int comp=0; comp < (int)v2t.size(); comp++)
                {
                  std::sort(v2t[comp].begin(), v2t[comp].end());
                  if(v2t[comp] == tettt) {
                                      printf("\n removed 2");

                  // position is comp
                  v2t.erase(v2t.begin()+comp);
                  f2t.erase(f2t.begin()+comp);
                  break;
                  }
                }


              // build new ones
               // p = ttt[3], a = ttt[0], b = ttt[1], c = ttt[2], d = tettt[3]
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

              if(orient3d(dx4, dx1, dx2, dx5) > 0) {
                std::vector<int> pnew1 {ttt[3], ttt[0], ttt[1], tettt[ind_d]};
                v2t.push_back(pnew1);
                tetverts.push_back(pnew1);
                pnew1.clear();
              } else {
                std::vector<int> pnew1 {ttt[3], ttt[1], ttt[0], tettt[ind_d]};
                v2t.push_back(pnew1);
                tetverts.push_back(pnew1);
                pnew1.clear();
              }
              if(orient3d(dx4, dx1, dx3, dx5) > 0) {
                std::vector<int> pnew2 {ttt[3], ttt[0], ttt[2], tettt[ind_d]};
                v2t.push_back(pnew2);
                tetverts.push_back(pnew2);
                pnew2.clear();
              } else {
                std::vector<int> pnew2 {ttt[3], ttt[2], ttt[0], tettt[ind_d]};
                v2t.push_back(pnew2);
                tetverts.push_back(pnew2);
                pnew2.clear();
              }
              if(orient3d(dx4, dx2, dx3, dx5) > 0) {
                std::vector<int> pnew3 {ttt[3], ttt[1], ttt[2], tettt[ind_d]};
                v2t.push_back(pnew3);
                tetverts.push_back(pnew3);
                pnew3.clear();
              } else {
                std::vector<int> pnew3 {ttt[3], ttt[2], ttt[1], tettt[ind_d]};
                v2t.push_back(pnew3);
                tetverts.push_back(pnew3);
                pnew3.clear();
              }

              // first tet with faces
              // pac, acd, pcd, pad
              std::vector<int> fnew1 {ttt[3], ttt[0], ttt[2]};
              std::vector<int> fnew2 {ttt[0], ttt[2], tettt[ind_d]};
              std::vector<int> fnew3 {ttt[3], ttt[2], tettt[ind_d]};
              std::vector<int> fnew4 {ttt[3], ttt[0], tettt[ind_d]};
              // pbc, bcd, pcd, pbd
              std::vector<int> fnew5 {ttt[3], ttt[1], ttt[2]};
              std::vector<int> fnew6 {ttt[1], ttt[2], tettt[ind_d]};
              // pdc s.o.
              std::vector<int> fnew7 {ttt[3], ttt[1], tettt[ind_d]};
              // pab, abd, pad, pbd
              std::vector<int> fnew8 {ttt[3], ttt[0], ttt[1]};
              std::vector<int> fnew9 {ttt[0], ttt[1], tettt[ind_d]};
              // pad s.o.
              // pbd s.o.
              int pos1 = 0;
              std::vector<int> ftet;
              std::vector<int> ftet2;
              std::vector<int> ftet3;
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
                faces.push_back(fnew6);
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

              // push in vectorss
              f2t.push_back(ftet);
              f2t.push_back(ftet2);
              f2t.push_back(ftet3);

              tetfaces.push_back(ftet);
              tetfaces.push_back(ftet2);
              tetfaces.push_back(ftet3);

              if(ftet.size() > 0) {
                ftet.clear();
              }
              if(ftet2.size() > 0) {
                ftet2.clear();
              }
              if(ftet3.size() > 0) {
                ftet3.clear();
              }

              if(fnew1.size() > 0) {
                fnew1.clear();
              }
              if(fnew2.size() > 0) {
                fnew2.clear();
              }
              if(fnew3.size() > 0) {
                fnew3.clear();
              }
              if(fnew4.size() > 0) {
                fnew4.clear();
              }
              if(fnew5.size() > 0) {
                fnew5.clear();
              }
              if(fnew6.size() > 0) {
                fnew6.clear();
              }
              if(fnew7.size() > 0) {
                fnew7.clear();
              }
              if(fnew8.size() > 0) {
                fnew8.clear();
              }
              if(fnew9.size() > 0) {
                fnew9.clear();
              }
              }
             }
            }
          }
        }

        for(size_t u=0; u < v2t.size(); u++)
        {
          double pa[3] = {vertices[v2t[u][0]].x,
           vertices[v2t[u][0]].y, vertices[v2t[u][0]].z};
          double pb[3] = {vertices[v2t[u][1]].x,
           vertices[v2t[u][1]].y, vertices[v2t[u][1]].z};
          double pc[3] = {vertices[v2t[u][2]].x,
           vertices[v2t[u][2]].y, vertices[v2t[u][2]].z};
          double pd[3] = {vertices[v2t[u][3]].x,
           vertices[v2t[u][3]].y, vertices[v2t[u][3]].z};
          double det = orient3d(pa, pb, pc, pd);
          printf("\n T: %.2f %.2f %.2f, %.2f %.2f %.2f, "
          " %.2f %.2f %.2f, %.2f %.2f %.2f",
           vertices[v2t[u][0]].x, vertices[v2t[u][0]].y,
          vertices[v2t[u][0]].z, vertices[v2t[u][1]].x,
           vertices[v2t[u][1]].y, vertices[v2t[u][1]].z,
            vertices[v2t[u][2]].x, 
          vertices[v2t[u][2]].y, vertices[v2t[u][2]].z,
           vertices[v2t[u][3]].x, vertices[v2t[u][3]].y,
            vertices[v2t[u][3]].z);
          printf("\n DET: %.2f", det);
        }



         /* 
          while stack is not empty
           τ = { p, a, b, c } ← pop from stack
           τ a = { a, b, c, d } ← get adjacent tetrahedron 
           of τ having the edge abc as a face 

          */
       /*  while((int)tempvec.size() != 0) {
           int size_vec = (int)tempvec.size();
           // first element von curr_tet.a => p
           Tetrahedron curr_tet = tempvec.at(size_vec-1); 
           tempvec.pop_back(); // remove first one

           // get a, b, c of t (first point p is already known)
           Point3D a = curr_tet.b;
           Point3D b = curr_tet.c;
           Point3D c = curr_tet.d;

          pt[0] = curr_tet.a.x;
          pt[1] = curr_tet.a.y;
          pt[2] = curr_tet.a.z;
          at[0] = curr_tet.b.x;
          at[1] = curr_tet.b.y;
          at[2] = curr_tet.b.z;
          bt[0] = curr_tet.c.x;
          bt[1] = curr_tet.c.y;
          bt[2] = curr_tet.c.z;
          ct[0] = curr_tet.d.x;
          ct[1] = curr_tet.d.y;
          ct[2] = curr_tet.d.z;

          

           std::vector<Point3D> faceP {a, b, c};

           Point3D d;
           bool found = false;
           // find tetrahedron which contains points a,b,c as a face
           for(int f=0; f < (int)tetravec->size(); f++)
           {
             found = false;
             Tetrahedron t = tetravec->at(f);
             std::vector<Point3D> points3d_vec_tmp {t.a, t.b, t.c, t.d};
             std::vector<Point3D> points3d_vec_tmp_2 {t.a, t.b, t.c, t.d};

             for(int fa = 0; fa < (int)faceP.size(); fa++)
             {
              for(int v = 0; v < (int)points3d_vec_tmp.size(); v++)
              {
                if(comparePoints3D(points3d_vec_tmp.at(v), faceP.at(fa)))
                {
                  // get position of point points3d_vec_tmp.at(v) 
                  // in points3d_vec_tmp_2
                  int i = getIndex(points3d_vec_tmp.at(v), points3d_vec_tmp_2);
                  points3d_vec_tmp_2.erase(points3d_vec_tmp_2.begin()+i);
                  break;
                }
              }
              if((int)points3d_vec_tmp_2.size() == 1)
              {
                 d = points3d_vec_tmp_2.at(0);
                 //not the same as input tetra
                 if(!comparePoints3D(d, curr_tet.a)) { 
                  found = true;
                  dt[0] = d.x;
                  dt[1] = d.y;
                  dt[2] = d.z;
                  break;
                 }
              }
             }

             if(found)
             {
               break;
             }

             
           } */

         // if(found) { //found d
           /*
            Check if d lies in circumsphere of p a b c
            with InSphere -> calculate determinant

            ax ay az ax²+ay²+az² 1
            bx by bz bx²+by²+bz² 1
            cx cy cz cx²+cy²+cz² 1
            dx dy dz dx²+dy²+dz² 1
            px py pz px²+py²+pz² 1

            5x5 Matrix
            
           */

          // Before insphere check if orientation of points is correct
         /* float orient_1 = orient3d(pt, at, bt, ct);
          float detInSphere = 0.0;
          if(orient_1 < 0.0)
          {
            detInSphere = insphere(pt, bt, at, ct, dt);
          } else {
            detInSphere = insphere(pt, at, bt, ct, dt);
          }
           if(detInSphere > 0.0) 
              // inside sphere or on it (on it also ok?! ==> no)
            { // #case 1: pd intersects area abc => Flip23
             std::vector<Point3D> intersectlines;
             int pos_tet32_1;
             int pos_tet32_2;
             int pos_tet32_3;

             if(signbit(orient3d(pt, at, bt, ct)) 
                != signbit(orient3d(dt,at,bt,ct))) 
             {
               // perform Flip23
               // get "old" tetrahedrons and push three new one in vector
               // get position of first tetrahedron to delete
                int pos_1 = getTetrahedron(curr_tet.a, curr_tet.b,
                               curr_tet.c, curr_tet.d, *tetravec);
                if(pos_1 >= 0)
                {
                     tetravec->erase(tetravec->begin() + pos_1);
                }
                
               // get position of second tetrahedron in vector to delete
               int pos_2 = getTetrahedron(curr_tet.b, curr_tet.c,
                                       curr_tet.d, d, *tetravec);
               if(pos_2 >= 0) 
               {
                  tetravec->erase(tetravec->begin() + pos_2); 
               }

               // create new tetrahedrons and save in right orientation
               Tetrahedron tet_flip23_1;
               float orient_23 = orient3d(pt, at, ct, dt);
               if(orient_23 > 0.00) {
                  tet_flip23_1.a = curr_tet.a;
                  tet_flip23_1.b = curr_tet.b;
                  tet_flip23_1.c = curr_tet.d;
                  tet_flip23_1.d = d;
                  tempvec.push_back(tet_flip23_1);
                  tetravec->push_back(tet_flip23_1);
               } else {
                  tet_flip23_1.a = curr_tet.a;
                  tet_flip23_1.b = curr_tet.d;
                  tet_flip23_1.c = curr_tet.b;
                  tet_flip23_1.d = d; 
                  tempvec.push_back(tet_flip23_1);
                  tetravec->push_back(tet_flip23_1);
               }

               Tetrahedron tet_flip23_2;
               orient_23 = orient3d(pt, bt, ct, dt);
               if(orient_23 > 0.00)
               {
                 tet_flip23_2.a = curr_tet.a;
                 tet_flip23_2.b = curr_tet.c;
                 tet_flip23_2.c = curr_tet.d;
                 tet_flip23_2.d = d;
                 tempvec.push_back(tet_flip23_2);
                 tetravec->push_back(tet_flip23_2);
               } else {
                   tet_flip23_2.a = curr_tet.a;
                   tet_flip23_2.b = curr_tet.d;
                   tet_flip23_2.c = curr_tet.c;
                   tet_flip23_2.d = d;
                   tempvec.push_back(tet_flip23_2);
                   tetravec->push_back(tet_flip23_2);
               }
               
               Tetrahedron tet_flip23_3;
               orient_23 = orient3d(pt, at, bt, dt);
               if(orient_23 > 0.00)
               {
                  tet_flip23_3.a = curr_tet.a;
                  tet_flip23_3.b = curr_tet.b;
                  tet_flip23_3.c = curr_tet.c;
                  tet_flip23_3.d = d;
                  tempvec.push_back(tet_flip23_3);
                  tetravec->push_back(tet_flip23_3);

               } else {
                  tet_flip23_3.a = curr_tet.a;
                  tet_flip23_3.b = curr_tet.c;
                  tet_flip23_3.c = curr_tet.b;
                  tet_flip23_3.d = d;
                  tempvec.push_back(tet_flip23_3);
                  tetravec->push_back(tet_flip23_3);
               }
               
               count++;

             } else {
              //#case 2: pd does not intersect area abc and there is 
              // a third tetrahedron abpd, 
              // so all tetrahedrons share ab => Flip32
              if (shareSameLine(curr_tet.b, curr_tet.c, curr_tet.d, 
                  curr_tet.a, *tetravec, &pos_tet32_1)) { 
                                
                // delete first tetrahedron
                tetravec->erase(tetravec->begin()+ pos_tet32_1);
                //get the other two tetrahedrons
                  pos_tet32_2 = getTetrahedron(curr_tet.a, curr_tet.b,
                     curr_tet.c, curr_tet.d, *tetravec);
                  if(pos_tet32_2 >= 0)
                  {
                     tetravec->erase(tetravec->begin() + pos_tet32_2);
                  }
                
                pos_tet32_3 = getTetrahedron(curr_tet.b, curr_tet.c,
                   curr_tet.d, d, *tetravec);
                if(pos_tet32_3 >= 0) 
                {
                  tetravec->erase(tetravec->begin()+ pos_tet32_3);
                }
                // sharing ab:
                
                // 2 new Tetrahedron:
                Tetrahedron tet_flip32_1;
                Tetrahedron tet_flip32_2;
                float orient_32 = orient3d(pt, at, ct, dt);
                if(orient_32 > 0.00) 
                {
                  tet_flip32_1.a = curr_tet.a;
                  tet_flip32_1.b = curr_tet.b;
                  tet_flip32_1.c = curr_tet.d;
                  tet_flip32_1.d = d;
                  tempvec.push_back(tet_flip32_1);
                  tetravec->push_back(tet_flip32_1);

                } else {
                    tet_flip32_1.a = curr_tet.a;
                    tet_flip32_1.b = curr_tet.d;
                    tet_flip32_1.c = curr_tet.b;
                    tet_flip32_1.d = d;
                    tempvec.push_back(tet_flip32_1);
                    tetravec->push_back(tet_flip32_1);
                }

                orient_32 = orient3d(pt, bt, ct, dt);
                if(orient_32 > 0.00)
                {
                    tet_flip32_1.a = curr_tet.a;
                    tet_flip32_1.b = curr_tet.c;
                    tet_flip32_1.c = curr_tet.d;
                    tet_flip32_1.d = d;
                    tempvec.push_back(tet_flip32_2);
                    tetravec->push_back(tet_flip32_2);

                } else {
                    tet_flip32_1.a = curr_tet.a;
                    tet_flip32_1.b = curr_tet.d;
                    tet_flip32_1.c = curr_tet.c;
                    tet_flip32_1.d = d;
                    tempvec.push_back(tet_flip32_2);
                    tetravec->push_back(tet_flip32_2);
                }
                
                
                counti++;
              
              
             }*/ /*else if (linesIntersect(curr_tet.a, d, curr_tet.b,
               curr_tet.c, &intersectlines) 
             || linesIntersect(curr_tet.a, d, curr_tet.b, 
                curr_tet.d, &intersectlines) 
             || linesIntersect(curr_tet.a, d, curr_tet.c, 
                curr_tet.d, &intersectlines))
             
             */
             /*else if(determinant4x4(curr_tet.a, curr_tet.b, 
                curr_tet.c, d) == 0.00 
             || determinant4x4(curr_tet.a, curr_tet.c, 
                curr_tet.d, d) == 0.00
             || determinant4x4(curr_tet.a, curr_tet.b, 
                curr_tet.d, d) == 0.00)
             
             */
            /*  else if(orient3d(pt, at, bt, dt) == 0.00 
              || orient3d(pt, bt, ct, dt) == 0.00
              || orient3d(pt, at, ct, dt) == 0.00)
             {
              //#case 3: line pd intersects edge of abc and there exist 
              // two other tetrahedrons which share edges => flip44

              // TODO
              Point3D q1;// = intersectlines.at(0);
              Point3D q2;// = intersectlines.at(1);
              bool found1 = false;
              bool found2 = false;
              Tetrahedron t1;
              int pos_t1;
              Tetrahedron t2;
              int pos_t2;
              for(int g=0; g < (int)tetravec->size(); g++)
              { 
                //find other tetrahedrons which share edge q1q2
                if(!found1 && shareSameFace(curr_tet.a, q1,
                   q2, tetravec->at(g)))
                {
                    found1 = true;
                    t1 = tetravec->at(g);
                    pos_t1 = g;
                }
                if(!found2 && shareSameFace(d, q1, q2, tetravec->at(g)))
                {
                    found2 = true;
                    t2 = tetravec->at(g);
                    pos_t2 = g;
                } 

              }

              // found two other tetrahedrons, prepare flip44
              if(found1 && found2)
              {
                // FLIP44
                // get 4 "old" tetrahedrons and push 4 new one in vector
               int pos_2 = getTetrahedron(curr_tet.b, curr_tet.c,
                                          curr_tet.d, d, *tetravec); 
               if(pos_2 >= 0) {
                tetravec->erase(tetravec->begin() + pos_2); }
               if(pos_t1 >= 0)
               {
                 tetravec->erase(tetravec->begin() + pos_t1);
               }
               if(pos_t2 >= 0)
               {
                 tetravec->erase(tetravec->begin() + pos_t2);
               }
               
               // build 4 new tetrahedrons
               // 1: curr.a, curr.b, curr.c, t1.d
               // 2: curr.a, curr.b, curr.d, t1.d
               // 3: d, curr.b, curr.c, t1.d
               // 4: d, curr.b, curr.d, t1.d

               Tetrahedron tet_flip44_1;
               float orient_44 = determinant4x4(curr_tet.a,
                 curr_tet.b, curr_tet.c, t1.d);
               if(orient_44 > 0.00) {
                  tet_flip44_1.a = curr_tet.a;
                  tet_flip44_1.b = curr_tet.b;
                  tet_flip44_1.c = curr_tet.c;
                  tet_flip44_1.d = t1.d;
                  tempvec.push_back(tet_flip44_1);
                  tetravec->push_back(tet_flip44_1);
               } else {
                  tet_flip44_1.a = curr_tet.a;
                  tet_flip44_1.b = curr_tet.c;
                  tet_flip44_1.c = curr_tet.b;
                  tet_flip44_1.d = t1.d; 
                  tempvec.push_back(tet_flip44_1);
                  tetravec->push_back(tet_flip44_1);
               }

               Tetrahedron tet_flip44_2;
               orient_44 = determinant4x4(curr_tet.a,
                 curr_tet.b, curr_tet.d, t1.d);
               if(orient_44 > 0.00)
               {
                 tet_flip44_2.a = curr_tet.a;
                 tet_flip44_2.b = curr_tet.b;
                 tet_flip44_2.c = curr_tet.d;
                 tet_flip44_2.d = t1.d;
                 tempvec.push_back(tet_flip44_2);
                 tetravec->push_back(tet_flip44_2);
               } else {
                   tet_flip44_2.a = curr_tet.a;
                   tet_flip44_2.b = curr_tet.d;
                   tet_flip44_2.c = curr_tet.b;
                   tet_flip44_2.d = t1.d;
                   tempvec.push_back(tet_flip44_2);
                   tetravec->push_back(tet_flip44_2);
               }
               
               Tetrahedron tet_flip44_3;
               orient_44 = determinant4x4(d, curr_tet.b, curr_tet.c, t1.d);
               if(orient_44 > 0.00)
               {
                  tet_flip44_3.a = d;
                  tet_flip44_3.b = curr_tet.b;
                  tet_flip44_3.c = curr_tet.c;
                  tet_flip44_3.d = t1.d;
                  tempvec.push_back(tet_flip44_3);
                  tetravec->push_back(tet_flip44_3);
               } else {
                    tet_flip44_3.a = d;
                    tet_flip44_3.b = curr_tet.c;
                    tet_flip44_3.c = curr_tet.b;
                    tet_flip44_3.d = t1.d;
                    tempvec.push_back(tet_flip44_3);
                    tetravec->push_back(tet_flip44_3);
               }

              Tetrahedron tet_flip44_4;
               orient_44 = determinant4x4(d, curr_tet.b, curr_tet.d, t1.d);
               if(orient_44 > 0.00)
               {
                  tet_flip44_3.a = d;
                  tet_flip44_3.b = curr_tet.b;
                  tet_flip44_3.c = curr_tet.d;
                  tet_flip44_3.d = t1.d;
                  tempvec.push_back(tet_flip44_4);
                  tetravec->push_back(tet_flip44_4);
               } else {
                    tet_flip44_3.a = d;
                    tet_flip44_3.b = curr_tet.d;
                    tet_flip44_3.c = curr_tet.b;
                    tet_flip44_3.d = t1.d;
                    tempvec.push_back(tet_flip44_4);
                    tetravec->push_back(tet_flip44_4);
               }
              }

                count3++;
             } 
             // tetrahedron is flat
             else if(orient3d(at, bt, dt, pt) == 0.00)
             {
               //#case 4: p lies exactly on edge of abc => 
               // Flip23 == same as orient== 0 ?!
               // get "old" tetrahedrons and push three new one in vector
               int pos_1 = getTetrahedron(curr_tet.a, curr_tet.b,
                 curr_tet.c, curr_tet.d, *tetravec);
               if(pos_1 >= 0) {
                tetravec->erase(tetravec->begin() + pos_1); }
               int pos_2 = getTetrahedron(curr_tet.b, curr_tet.c,
                 curr_tet.d, d, *tetravec);
               if(pos_2 >= 0) {
                tetravec->erase(tetravec->begin() + pos_2); }

               Tetrahedron tet_flip23_1;
                float orient_23 = orient3d(pt, at, ct, dt);
               if(orient_23 > 0.00) {
                  tet_flip23_1.a = curr_tet.a;
                  tet_flip23_1.b = curr_tet.b;
                  tet_flip23_1.c = curr_tet.d;
                  tet_flip23_1.d = d;
                  tempvec.push_back(tet_flip23_1);
                  tetravec->push_back(tet_flip23_1);
               } else {
                  tet_flip23_1.a = curr_tet.a;
                  tet_flip23_1.b = curr_tet.d;
                  tet_flip23_1.c = curr_tet.b;
                  tet_flip23_1.d = d; 
                  tempvec.push_back(tet_flip23_1);
                  tetravec->push_back(tet_flip23_1);
               }

               Tetrahedron tet_flip23_2;
               orient_23 = orient3d(pt, bt, ct, dt);
               if(orient_23 > 0.00)
               {
                 tet_flip23_2.a = curr_tet.a;
                 tet_flip23_2.b = curr_tet.c;
                 tet_flip23_2.c = curr_tet.d;
                 tet_flip23_2.d = d;
                 tempvec.push_back(tet_flip23_2);
                 tetravec->push_back(tet_flip23_2);
               } else {
                   tet_flip23_2.a = curr_tet.a;
                   tet_flip23_2.b = curr_tet.d;
                   tet_flip23_2.c = curr_tet.c;
                   tet_flip23_2.d = d;
                   tempvec.push_back(tet_flip23_2);
                   tetravec->push_back(tet_flip23_2);
               }
               
               Tetrahedron tet_flip23_3;
               orient_23 = orient3d(pt, at, bt, dt);
               if(orient_23 > 0.00)
               {
                  tet_flip23_3.a = curr_tet.a;
                  tet_flip23_3.b = curr_tet.b;
                  tet_flip23_3.c = curr_tet.c;
                  tet_flip23_3.d = d;
                  tempvec.push_back(tet_flip23_3);
                  tetravec->push_back(tet_flip23_3);

               } else {
                    tet_flip23_3.a = curr_tet.a;
                    tet_flip23_3.b = curr_tet.c;
                    tet_flip23_3.c = curr_tet.b;
                    tet_flip23_3.d = d;
                    tetravec->push_back(tet_flip23_3);
                    tempvec.push_back(tet_flip23_3);
               }

              count4++;

             }

           } // end of second if
          }  // else do nothing


          } // end of dfound

        }  // end of while-loop
      */
  }

        for(size_t u=0; u < v2t.size(); u++)
        {
          printf("\n Tetraeder %d", (int)u+1);
          printf("\n dreieck(%.2f|%.2f|%.2f "
          " %.2f|%.2f|%.2f %.2f|%.2f|%.2f)",
           vertices[v2t[u][0]].x,vertices[v2t[u][0]].y, vertices[v2t[u][0]].z,
           vertices[v2t[u][1]].x, vertices[v2t[u][1]].y, vertices[v2t[u][1]].z,
            vertices[v2t[u][2]].x, vertices[v2t[u][2]].y,
             vertices[v2t[u][2]].z);
          printf("\n dreieck(%.2f|%.2f|%.2f "
          " %.2f|%.2f|%.2f %.2f|%.2f|%.2f)",
           vertices[v2t[u][0]].x,vertices[v2t[u][0]].y, vertices[v2t[u][0]].z,
           vertices[v2t[u][1]].x, vertices[v2t[u][1]].y, vertices[v2t[u][1]].z,
            vertices[v2t[u][3]].x, vertices[v2t[u][3]].y,
             vertices[v2t[u][3]].z);
          printf("\n dreieck(%.2f|%.2f|%.2f "
          " %.2f|%.2f|%.2f %.2f|%.2f|%.2f)",
            vertices[v2t[u][0]].x,vertices[v2t[u][0]].y, vertices[v2t[u][0]].z,
          vertices[v2t[u][2]].x, vertices[v2t[u][2]].y, vertices[v2t[u][2]].z,
           vertices[v2t[u][3]].x, vertices[v2t[u][3]].y, vertices[v2t[u][3]].z);
           printf("\n dreieck(%.2f|%.2f|%.2f "
           " %.2f|%.2f|%.2f %.2f|%.2f|%.2f)",
             vertices[v2t[u][1]].x, vertices[v2t[u][1]].y,
              vertices[v2t[u][1]].z,
            vertices[v2t[u][2]].x, vertices[v2t[u][2]].y, vertices[v2t[u][2]].z,
             vertices[v2t[u][3]].x, vertices[v2t[u][3]].y,
              vertices[v2t[u][3]].z);
        }
   
   
}

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

void
Convex3D::buildVoronoi3D(Stream<Rectangle<3>> rStream) {

    // get all points from input cuboids
    rStream.open();
    Rectangle<3>* next = rStream.request();
    while(next != 0){
      Point3D p = getCentre(next);
      if(!duplicateP(p)) {
        points.push_back(getCentre(next));
      }
      next = rStream.request();
    }

    Convex3D* conv3d = new Convex3D();
    rStream.close();

    std::vector<Tetrahedron> tetravec;
    // create delaunay diagram with points
    createDelaunay(points, &tetravec);
    std::vector<std::vector<Point3D>> facesPoints;
    std::vector<std::tuple<int, int>> lines;
    std::vector<std::vector<int>> temp;
    std::vector<Point3D> circumcenters;
    std::vector<Polyhedron> poly_vec;

    // create voronoi diagram from delaunay diagram
    // find all lines including point p from points
    for(int t=0; t < (int)points.size(); t++)
    {
        //Point3D p = points.at(t);
        int p_index;
        for(size_t id=0; id < vertices.size(); id++)
        {
          if(comparePoints3D(vertices[id], points[t]))
          {
            p_index = id;
            printf("\n p_index: %d", p_index);
            break;
          }
        }

        for(int verti=0; verti< (int)v2t.size(); verti++)
        {
          if(p_index == v2t[verti][0] || p_index == v2t[verti][1] 
          || p_index == v2t[verti][2]
          || p_index == v2t[verti][3])
          {
            printf("\n in v2t: %d", verti+1);
            printf("\n %d, %d, %d, %d",
             v2t[verti][0], v2t[verti][1], v2t[verti][2], v2t[verti][3]);
            Point3D vertex = getCircumcenter(vertices[v2t[verti][0]],
             vertices[v2t[verti][1]], 
            vertices[v2t[verti][2]], vertices[v2t[verti][3]]);
            printf("\n %.2f %.2f %.2f", vertex.x, vertex.y, vertex.z);
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

        for(int n=0; n < (int)lines.size(); n++)
        {
              std::tuple<int, int> tup = lines.at(n); 
              int p1 = std::get<0>(tup);
              int p2 = std::get<1>(tup);
              //printf("\n %d, %d", p1, p2);
              for(int b=0; b < (int)v2t.size(); b++)
              {
                //printf("\n %d %d %d %d", v2t[b][0],
                // v2t[b][1],  v2t[b][2],  v2t[b][3]);
                if(sameline(p1, p2, v2t[b]))
                {    
                  //printf(" => same"); 
                  temp.push_back(v2t[b]);
                }
              }
                // all tetrahedrons in vector which share line point_tup[n]
                // get circumcenter
                for(int m=0; m< (int)temp.size(); m++)
                {
                  Point3D vertex = getCircumcenter(vertices[temp[m][0]],
                   vertices[temp[m][1]], 
                  vertices[temp[m][2]], vertices[temp[m][3]]);
                  circumcenters.push_back(vertex);
                }
                  facesPoints.push_back(circumcenters);
                  temp.clear();
                  circumcenters.clear();
        }

    /*for(int t=0; t < (int)points.size(); t++)
    {
      Point3D p = points.at(t);
        lines.clear();

        for(int z=0; z<(int)tetravec.size(); z++)
        {
          Tetrahedron fin = tetravec.at(z);
          if(comparePoints3D(fin.a, p))
          {
             if(!insidevec(fin.a, fin.b, lines)) {
             lines.push_back(std::make_tuple(fin.a, fin.b)); }
             if(!insidevec(fin.a, fin.c, lines)) {
             lines.push_back(std::make_tuple(fin.a, fin.c)); }
             if(!insidevec(fin.a, fin.d, lines)) {
             lines.push_back(std::make_tuple(fin.a, fin.d));
             }
          }
          else if(comparePoints3D(fin.b, p)) 
          {
            if(!insidevec(fin.b, fin.a, lines)) {
             lines.push_back(std::make_tuple(fin.b, fin.a)); }
             if(!insidevec(fin.b, fin.c, lines)) {
             lines.push_back(std::make_tuple(fin.b, fin.c)); }
             if(!insidevec(fin.b, fin.d, lines)) {
             lines.push_back(std::make_tuple(fin.b, fin.d)); 
            }
          }

          else if(comparePoints3D(fin.c, p))
          {
            if(!insidevec(fin.c, fin.a, lines)) {
              lines.push_back(std::make_tuple(fin.c, fin.a)); }
              if(!insidevec(fin.c, fin.b, lines)) {
              lines.push_back(std::make_tuple(fin.c, fin.b)); }
              if(!insidevec(fin.c, fin.d, lines)) {
              lines.push_back(std::make_tuple(fin.c, fin.d));
            }
          }

          else if(comparePoints3D(fin.d, p))
          {
            if(!insidevec(fin.d, fin.a, lines)) {
             lines.push_back(std::make_tuple(fin.d, fin.a)); }
             if(!insidevec(fin.d, fin.b, lines)) {
             lines.push_back(std::make_tuple(fin.d, fin.b)); }
             if(!insidevec(fin.d, fin.c, lines)) {
             lines.push_back(std::make_tuple(fin.d, fin.c)); 
            }
          }
          

          
        }*/
         /*   for(int n=0; n < (int)lines.size(); n++)
            {
              std::tuple<Point3D, Point3D> tup = lines.at(n); 
              Point3D p1 = std::get<0>(tup);
              Point3D p2 = std::get<1>(tup);
              for(int b=0; b < (int)tetravec.size(); b++)
              {
                if(sameline(p1, p2, tetravec.at(b)))
                {     
                  temp.push_back(tetravec.at(b));
                }
              }
                // all tetrahedrons in vector which share line point_tup[n]
                // get circumcenter
                for(int m=0; m< (int)temp.size(); m++)
                {
                  Point3D vertex = getCircumcenter(temp.at(m));
                  circumcenters.push_back(vertex);
                }
                  facesPoints.push_back(circumcenters);
                  temp.clear();
                  circumcenters.clear();
            }
*/
      // create polyhedron
      // Convex3d: vector<faces> => 
      // face: vector<points> => 
      // Convex3D = std::vector<std::vector<Point3D>>
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
        polyhedron.faces.push_back(face);
      }
      // save polyhedron in vector
      polyhedronvec.push_back(polyhedron);
      facesPoints.clear();
    }

    for(int pol=0; pol < (int)polyhedronvec.size(); pol++)
    {
      Polyhedron polyhe = polyhedronvec[pol];
      printf("\n Polygon %d", pol+1);
      for(int fa=0; fa < (int)polyhe.faces.size(); fa++)
      {
        std::vector<Point3D> face = polyhe.faces[fa];
        printf("\n polygon(");
        for(int po=0; po < (int)face.size(); po++)
        {
          printf("%.2f|%.2f|%.2f ", face[po].x, face[po].y, face[po].z);
        }
        printf(")");
      }
    }

    conv3d->setPolyhedronVector(polyhedronvec);

    if(faces.size() > 0)
    {
      faces.clear();
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
    return 1;
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
    return 1;
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
    return -1;
  }
      
  if(v[0] == cellno)
  {
    boolval = true;
    res->Set( true, boolval);
    return 0;
  }
      
  res->Set( true, boolval);

  /*if(convex->voroVec.size() > 0) {
    convex->voroVec.clear();
  }*/

  return 0;

}

/*
8.2.2 TopRightClassVoronoiVM
*/

int trcCellIdvorononoi3DVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  Convex3D *convex3d = static_cast<Convex3D*>(args[0].addr);
  std::vector<Polyhedron> polyvec = convex3d->getPolyhedronVector();

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  Rectangle<3> *search_window_ptr_2
    = static_cast<Rectangle<3>*>( args[2].addr );

  if(search_window_ptr == nullptr 
    || search_window_ptr_2 == nullptr 
    || convex3d == nullptr) {
    return 1;
  }

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;

  std::set<int> intsetRect1 = cellNum3D(convex3d,
                              search_window_ptr, 2);
  std::set<int> intsetRect2 = cellNum3D(convex3d, 
                              search_window_ptr_2, 2);
  
  /*
  for (std::set<int>::iterator it=intsetRect1.begin();
   it!=intsetRect1.end(); ++it)
    std::cout << ' ' << *it;
  for (std::set<int>::iterator ip=intsetRect2.begin();
   ip!=intsetRect2.end(); ++ip)
    std::cout << ' ' << *ip;

   */ 
  


  std::vector<int> v(sizeof(intsetRect1)+ sizeof(intsetRect2));
  std::vector<int>::iterator it;

  it=std::set_intersection (intsetRect1.begin(), intsetRect1.end(),
     intsetRect2.begin(), intsetRect2.end(), v.begin());
  v.resize(it-v.begin());                      
  
  if(v.empty()) { 
    //no intersection between rectangles
    res->Set(0);
    return -1;

  }

  for (it=v.begin(); it!=v.end(); ++it) {
    for(size_t c = 0; c < polyvec.size(); c++) {
      if(polyvec[c].getPolyId() == *it) {
        // get TRC Values
        int value_rect1 = 0;
        int value_rect2 = 0;

        // create bbox of polyhedron cell
        Polyhedron* tmp = &polyvec[c];

        Rectangle<3> bbox = createBBox3D(tmp);
        if ( search_window_ptr->getMaxX() >= bbox.getMaxX() )
          {value_rect1++;}
        if ( search_window_ptr->getMaxY() >= bbox.getMaxY() ) {
          value_rect1 += 2;}
        

        if ( search_window_ptr_2->getMaxX() >= bbox.getMaxX() )
          {value_rect2++;}
        if ( search_window_ptr_2->getMaxY() >= bbox.getMaxY() ) {
          value_rect2 += 2;}
        
        int value = value_rect1 & value_rect2;
        if (value == 0)
        {
          res->Set( *it );
          return 0;
        }
   }
  }
  }

  res->Set(0);
  return 0;

}

int trcvorononoi3DVM( Word* args, Word& result, int message,
      Word& local, Supplier s ) 
{
  Polyhedron *convex3d = static_cast<Polyhedron*>(args[0].addr);

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );


  if(search_window_ptr == nullptr || convex3d == nullptr) {
    return 1;
  }

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;

        // get TRC Values
        int value_rect1 = 0;

        // create bbox of polyhedron cell
        Rectangle<3> bbox = createBBox3D(convex3d);
        if ( search_window_ptr->getMaxX() >= bbox.getMaxX() )
          {value_rect1++;}
        if ( search_window_ptr->getMaxY() >= bbox.getMaxY() ) {
          value_rect1 += 2;}


  res->Set(value_rect1);
  return 0;

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


const string toprightclassCellSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> Stream<Convex> x rect2 x rect2"
    "-> int </text--->"
    "<text>_ trcvoronoi [ list ] </text--->"
    "<text>Returns cell id in which "
    "  two rectangles should "
    " be reported. </text--->"    
    "<text> query trcvoronoi((convtest feed voronoi "
    " [Punkt, Conv, [const rect value (-10 10 -10 10)], FALSE] "
    " project[Conv] consume) feed projecttransformstream[Conv],"
    " rectangle2(-1.0, 4.0, 0.0, 2.0), " 
    " rectangle2(5.0, 7.0, -0.5, 0.5)) </text--->" 
    ") )";

const string toprightclassSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> rect2 x rect2"
    "-> int </text--->"
    "<text>_ trcvoronoi [ list ] </text--->"
    "<text>Returns the toprightclass value "
    " of two rectangles to show if they should "
    " be be reported now or later </text--->"    
    "<text> query trcvoronoi((convtest feed voronoi "
    " [Punkt, Conv, [const rect value (-10 10 -10 10)], FALSE] "
    " project[Conv] consume), rectangle2(-1.0, 4.0, 0.0, 2.0), " 
    " rectangle2(5.0, 7.0, -0.5, 0.5)) </text--->" 
    ") )";

const string getcellvoronoiSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> Stream<Convex> x int "
    "-> rectangle<2> </text--->"
    "<text>_ getcellvoro [ ] </text--->"
    "<text>Returns the bbox of a cell to "
    " the given cellid </text--->"    
    "<text> query trcvoronoi((convtest feed voronoi "
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


const string toprightclass3DSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>((stream (tuple (..., (ak1 Tk1),...))) x ak1 "
    "x ak2 x rect x bool x rect2 x rect2"
    "-> int </text--->"
    "<text>_ trcvoronoi3d [ list ] </text--->"
    "<text>Returns the toprightclass value "
    " of two rectangles to show if they should "
    " be be reported now or later </text--->"    
    "<text> query trc_voronoi3d([const rect3 value(-1 2 -3 6 0 2)]"
    " feed voronoi3d "
    " [const rect value (-10 10 -10 10 -10 10)], "
    " rectangle3(-1.0, 4.0, 0.0, 2.0, 1.0, 2.0), "
    " rectangle3(-1.0, 2.0, 1.0, 2.0, 1.0, 2.0)) </text--->" 
    ") )";

const string toprightclassCell3DSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>((stream (tuple (..., (ak1 Tk1),...))) x ak1"
    " x ak2 x rect x bool x rect2 x rect2"
    "-> int </text--->"
    "<text>_ trcvoronoi3d [ list ] </text--->"
    "<text>Returns the cell id of the cell in which "
    " two rectangles should "
    " be reported. </text--->"    
    "<text> query trc_voronoi3d([const rect3 value(-1 2 -3 6 0 2)]"
    " feed voronoi3d "
    " [const rect value (-10 10 -10 10 -10 10)], "
    " rectangle3(-1.0, 4.0, 0.0, 2.0, 1.0, 2.0), "
    " rectangle3(-1.0, 2.0, 1.0, 2.0, 1.0, 2.0)) </text--->" 
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


Operator trcvoronoi ("trcvoronoi",
                      toprightclassSpec,
                      trcvorononoiVM,
                      Operator::SimpleSelect,
                      trcvoronoitypemap );

Operator trccellvoronoi ("trccellvoronoi",
                      toprightclassCellSpec,
                      trcCellIdvorononoiVM,
                      Operator::SimpleSelect,
                      trcCellIdvoronoitypemap );

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

Operator trcvoronoi3d ("trcvoronoi3d",
                      toprightclass3DSpec,
                      trcvorononoi3DVM,
                      Operator::SimpleSelect,
                      trcvoronoi3dtypemap );  

Operator trccellvoronoi3d ("trccellvoronoi3d",
                  toprightclassCell3DSpec,
                  trcCellIdvorononoi3DVM,
                  Operator::SimpleSelect,
                  trcCellIdvoronoi3dtypemap
                  );                                         
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
    AddOperator( &trcvoronoi);
    AddOperator( &trccellvoronoi );
    AddOperator( &getcellvoronoi );
    AddOperator( &voronoi3d );
    AddOperator( &cellnum3d );
    AddOperator( &sccvoronoi3d );
    AddOperator( &trcvoronoi3d );
    AddOperator( &trccellvoronoi3d );
 
    
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
