/*

1 mLineRep.h

*/

#ifndef MLINEREP_H_
#define MLINEREP_H_


using namespace std;
//#define MLR_DEBUG
namespace RegionInterpol
{
/*

1.1 class mLineRep

This class is used to create MovingSegments of a given Match.
  
*/ 
   class mLineRep
   {
      public:
/*

1.1.1 Constructors and Destructor


*/    
         mLineRep();
         mLineRep(Match *myMatch);
         ~mLineRep();
/*

Returns the calculated triangles as a field of degenerated MovingSegments
 
*/       
         
         vector<temporalalgebra::MSegmentData> getTriangles();
       private:         
/*

Match the given Faces or ConvexHullTreeNodes to create triangles 
 
*/       
         
         void addTrianglesFromFaceFace(Face *face1, Face *face2, int time1, 
            int time2, int time, int facenr);
         void addTrianglesFromCHTCHT(ConvexHullTreeNode *cht1, 
            ConvexHullTreeNode *cht2,int time1,int time2,
            int facenr,int cyclenr);
/*
 
Add the triangles that are created if the givenFace or ConvexHullTreeNode is matches to null 
 
*/
         void addTrianglesFromFaceNull(Face *face, int time, int facenr);
         void addTrianglesFromCHTNull(ConvexHullTreeNode *cht, int time, 
            int facenr, int cyclenr);
/*
 
Add the triangles from a ConvexHullTreeNode to a point given as PointWNL or as three coordinates  
 
*/       
         void addTrianglesFromCHTPoint(ConvexHullTreeNode *chtn, int time,
            double x, double y, int t, int facenr, int cyclenr);
         void addTrianglesFromCHTPoint(ConvexHullTreeNode *chtn, int time,
            PointWNL *p3, int facenr, int cyclenr);

/*

Add the triangle, given by the three points.
  
*/       
         
         void addTriangle(PointWNL *p1, PointWNL *p2, PointWNL *p3, 
            int facenr, int cyclenr);

/*
 
 If a triangle is stored, that has the two given points, hten return the third one.
 
 findIndex helps to find this Point
  
*/       
         PointWNL *getCorrespondingPoint(PointWNL *p1, PointWNL *p2, 
            int facenr, int cyclenr);
         int findIndex(PointWNL *p1, PointWNL *p2, int facenr, int cyclenr);
/*
 
 Removes all triangles that have the two given points. 
 
*/       
         void removeTriangle(PointWNL *p1,PointWNL *p2,int facenr,int cyclenr);
/*
 
Removes all triangles that belong to the trapezoid given by the four points
 
*/       
         void removeTrapezoid(PointWNL *p1, PointWNL *p2, PointWNL *p3,
            PointWNL *p4, int facenr, int cyclenr);
/*
 
 Rotating\_plane is used to calculate triangles between two convex polygons. These are gives as ConvexHullTreeNodes, the timestamps
 are the times of the Polygons and face and cyclenr help to create the MovingSegments.
 
*/       
         void rotaring_pane(ConvexHullTreeNode *chtn1,ConvexHullTreeNode *chtn2,
            int time1, int time2, int facenr, int cyclenr);
/*
 
This Method is a function used  only by rotation\_plane. It finds the Index of a vertex in a polygone that matches the given angle.  
 
*/       
         int findMatchingIndex(vector<LineWA> *s, int j ,double angle, bool ka);
         
      
/*

1.1.1 Attributes  
 
Triangles is the calculated set of MovingSegments, 

myMatch is the Match which is used to create it
  
*/    
         vector<temporalalgebra::MSegmentData> triangles;
         Match *myMatch;
   };
}
#endif 

/*

\pagebreak

*/
