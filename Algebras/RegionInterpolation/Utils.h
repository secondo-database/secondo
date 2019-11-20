/*

1 Utils.h

*/

#ifndef UTILS_H_
#define UTILS_H_


namespace RegionInterpol
{
/*

1.1 Class Utils

This class provides some static functions.
 
*/ 
  class Utils
  {
      public:
/*

1.1.1 Static Functions

these functions return the Area of the given polygone. If the polygone is ordered CCW the result is positve otherwise it is negative

*/       
         static double getArea(LineWA linelist[],int linelistlength);
         static double getArea(std::vector<LineWA*>);
         static double getArea(std::vector<CHLine> lines);
         static double getArea(std::vector<LineWA> *lines);
/*

this function uses the Graham-Scan-Algorithm to compute the convex hull of the given polygone
 
*/    
         static std::vector<LineWA*> convexHull(LineWA lt[], int ltlength);
/*

this function compares two LineWAs and gives  a positive result if the angle of the first one is grater then the angle of the second one  
 
*/
      
         static int compareLineWA(const void *line1,const void *line2);
/*

checks if the point p1 is left or right of the line from line1 to line 2
 
*/          
         static double sameSide(LineWA *line1, LineWA *line2, LineWA *p1);
/*

these functions search for a LineWA in a given field. It returns the index.

*/     
         static int indexOf(LineWA *array, int arraylength,  LineWA obj);
         static int indexOf(std::vector<LineWA*> array, LineWA obj);
/*
 
this function computes the Angle between three points

*/    
         static double getAngleRad(double x, double y, 
            double preX, double preY,
            double folX, double folY);
/*

this function reverses the given vector
 
*/       
         static void reverseVector(std::vector<LineWA> *vec);
/*

this function computes the Hausdorff-Distance of two polygones
 
*/       
         static double getHausdorfDistance(std::vector<CHLine> obj1, 
            std::vector<CHLine> obj2);
/*
  
 these functions return the diameter of a given polygone
 
*/       
         static double getDiameter(std::vector<LineWA*> Poly);
         static double getDiameter(std::vector<CHLine> Poly);
/*
 
 this function computes the greatest distance of two Points of the given polygones
 
*/    
         static double getMaxDistance(std::vector<std::vector<LineWA*> > Polys);
/*

these functions convert a vector of LineWAs in a vector of CHLines and otherwise
 
*/       
         static std::vector<LineWA*> 
                        convertCHLine2LineWA(std::vector<CHLine> vec);
         static std::vector<CHLine> *
                        convertLineWA2CHLine(std::vector<LineWA> *vec);
/* 
 
 this function computes the square of the distance between two given LineWAs
      
*/    
         static double getSquareDistance(LineWA *p1,LineWA *p2);
/*
 
 in a field of LineWAs, which represents a polygone, this function adds the angle of each point to its LineWA 
 
*/       
         static void computeLineAngles(std::vector<LineWA> *lines);
/*
 
this function computes the area of the overlap of the two given polygones 

*/       
         static double getOverlap(std::vector<CHLine> *l1,
                                  std::vector<CHLine> *l2);
/*

this function converts a fild of CHLines into a Spatial--Algebra--Region

*/
      
         static Region *convert2Region(std::vector<CHLine> *l1);
         static std::string toString(const int& t);
/*
 
this function computes the distance from a point to a line, if the point is orthogonal to the line 

*/       
         static double getRectangularDistance(LineWA *lineA,
            LineWA *lineB, LineWA *point);
/*
 
this function returns a vector of LineWAs which includes the intersections of a line and a polygone

*/       
         static std::vector<LineWA*> *getIntersections(LineWA *lineA, 
                                                       LineWA *lineB,
            std::vector<CHLine> *poly);
/*

this function computes the interscection between two lines
 
*/                     
         static LineWA *getIntersection(LineWA *line1A, LineWA *line1B, 
            LineWA *line2A, LineWA *line2B);
/*

this function integrates a splitted hale as a concavity in the polygone
 
*/       
         static std::vector<LineWA> *joinLinelists(std::vector<LineWA> *first,
            std::vector<LineWA> *second);
/*
 
this function returns the point on the boundary of the polygon, which lies on the line given by lineA and lineB and ist closest to lineA  

*/       
         static LineWA *getClosestBoundaryPoint(LineWA *lineA, LineWA *lineB, 
            std::vector<LineWA> *poly);
/*
 
 this function checks if at least one of the given points lies on the given line
 
*/       
         static bool PointsOnLine(std::vector<LineWA> *points, 
            LineWA *lineA, LineWA *lineB);
/*

this operator prints a vector of LineWAs on the given stream 

*/       
         friend std::ostream & operator <<(std::ostream & os,
            const std::vector<LineWA> linevector);
      private:
/*

these functions are used by getHausdorffDistance and getDiameter
 
*/       
         static double getSingleHausdorffDistance(LineWA *obj1, 
            std::vector<CHLine> obj2);    
         static int getLongestDistFromPoint(LineWA *from, 
            std::vector<LineWA*> list, int pos);                       
  };
}
#endif 
/*

\pagebreak

*/



