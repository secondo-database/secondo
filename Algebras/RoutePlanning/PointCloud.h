/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef SECONDO_POINTCLOUD_H
#define SECONDO_POINTCLOUD_H

#include "Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "Operator.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "NList.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Stream/Stream.h"


namespace routeplanningalgebra {
    
    /*
    PointCloud object and related types added and maintained
    by Gundula Swidersky, Dec 2017.
    */
    
    /*
    Type Cpoint (x, y, z)
    */
    class Cpoint : public Attribute {
    public:
        // constructor doing nothing
        Cpoint() {}
        // constructor initialzing the object
        Cpoint(const double _x, const double _y, const double _z): 
            Attribute(true),
            x(_x), y(_y),z(_z) {}
        // destructor
        ~Cpoint() {}

        /*
        BasicType
        */
        static const std::string BasicType() { return "cpoint"; }

        /*
        checkType
        */
        static const bool checkType(const ListExpr list){
            return listutils::isSymbol(list, BasicType());
        }
 
        /*
        Getter / Setter and help functions
        */
        double getX() const;
        double getY() const;
        double getZ() const;
  
        /*
        NumOfFlobs
        */
        int NumOfFLOBs() const;
  
        /*
        GetFlob
        */
        Flob* GetFLOB ( const int i );
        
        /*
        Compare
        */
        int Compare (const Attribute * arg ) const;
  
        /*
        Adjacent
        */
        bool Adjacent ( const Attribute * arg) const;
  
        /*
        SizeOf
        */
        size_t Sizeof () const;
  
        /*
        HashValue
        */
        size_t HashValue () const;
  
        /*
        CopyFrom
        */
        void CopyFrom ( const Attribute* arg );
  
        /*
        Clone
        */
        Attribute *Clone () const;

        /*
        Property function
        */
        ListExpr CpointProperty();

        /*
        IN function
        */
        Word InCpoint  (const ListExpr typeinfo, const ListExpr instance, 
                    const int errorPos, ListExpr& errorInfo, bool& correct );

        /*
        OUT function
        */
        ListExpr OutCpoint(ListExpr typeInfo, Word value);

        /*
        Create
        */
        Word CreateCpoint(const ListExpr typeInfo);

        /*
        Delete
        */
        void DeleteCpoint(const ListExpr typeInfo, Word& w);

        /*
        Close
        */
        void CloseCpoint (const ListExpr typeInfo, Word& w );

        /*
        Clone
        */
        Word CloneCpoint( const ListExpr typeInfo, const Word& w);

        /*
        Cast
        */
        void* CastCpoint(void* addr);

        /*
        TypeCheck
        */
        bool CpointTypeCheck(ListExpr type, ListExpr& errorInfo);

        /*
        SizeOfCpoint
        */
        int SizeOfCpoint();
        
        private:
        double x;
        double y;
        double z;
    };

    /*
    Type Cpoints (DBArray of type Cpoint)
    */
    class Cpoints : public Attribute {
        public:
        // constructor doing nothing
        Cpoints() {}
        // constructor initialzing the object
        Cpoints(const int anz, const double *X= 0, const double *Y = 0, 
                const double *Z = 0) : 
                Attribute(true),
                cpoints ( anz ) {
            SetDefined(true);
            Cpoint* cpnt;
            if(anz > 0) {
                for( int i = 0; i < anz; i++) {
                    cpnt = new Cpoint( X[i], Y[i], Z[i] );
                    cpoints.Append( *cpnt );
                } 
            }
        }
        // destructor
        ~Cpoints() {}

        /*
        BasicType
        */    
        static const std::string BasicType() { return "cpoints"; }

        /*
        checkType
        */
        static  const bool checkType(const ListExpr list){
            return listutils::isSymbol(list, BasicType());
        }
        
        /*
        NumOfFlobs
        */
        int NumOfFLOBs() const;

        /*
        GetFlob
        */        
        Flob* GetFLOB ( const int i );
  
        /*
        GetNoCpoints
        */
        int GetNoCpoints();
  
        /*
        GetCpoint
        */
        Cpoint GetCpoint(int i);
  
        /*
        AppendCpoint
        */
        void AppendCpoint(const Cpoint &cpnt);
  
        /*
        Destroy
        */
        void DestroyCpoints();
  
        /*
        Compare
        */
        int Compare (const Attribute * arg ) const;
        
        /*
        Adjacent
        */
        bool Adjacent ( const Attribute * arg) const;
        
        /*
        Sizeof Cpoints
        */
        size_t Sizeof () const;
        
        /*
        HashValue
        */
        size_t HashValue () const;

        /*
        CopyFrom
        */
        void CopyFrom ( const Attribute* arg );
  
        /*
        Clone
        */
        Attribute *Clone () const;

    private:
        DbArray<Cpoint> cpoints;
    };    
    
    /*
    Type Cpointnode that is a node of an 2D Tree that is the internal
    data structure of the Cpoints stored within a PointCloud object. It
    contains Cpoint coordinates and indexes of left son and right son.    
    */
    class Cpointnode : public Attribute {
        public:
        // constructor doing nothing
        Cpointnode() {}
        // constructor initialzing the object
        Cpointnode(const double _x, const double _y, const double _z, 
                   const int _leftson, const int _rightson): 
                   Attribute(true),
                   x(_x), y(_y),z(_z),leftson(_leftson),rightson(_rightson) {}
        // destructor
        ~Cpointnode() {}

        /*
        BasicType
        */    
        static const std::string BasicType() { return "cpointnode"; }

        /*
        checkType
        */
        static const bool checkType(const ListExpr list){
            return listutils::isSymbol(list, BasicType());
        }

        /*
        Getter, setter and help functions
        */
        double getX() const;
        double getY() const;
        double getZ() const;
        int getLeftSon() const;
        int getRightSon() const;
        void setLeftSon(int lSon);
        void setRightSon(int rSon);
  
        /*
        NumOfFlobs
        */
        int NumOfFLOBs() const;
  
        /*
        GetFlob
        */
        Flob* GetFLOB ( const int i );
        
        /*
        Compare
        */
        int Compare (const Attribute * arg ) const;
  
        /*
        Adjacent
        */
        bool Adjacent ( const Attribute * arg) const;
  
        /*
        Sizeof Cpointnode
        */
        size_t Sizeof () const;
  
        /*
        HashValue
        */
        size_t HashValue () const;
  
        /*
        CopyFrom
        */
        void CopyFrom ( const Attribute* arg );
  
        /*
        Clone
        */
        Attribute *Clone () const; 
        
    private:
        double x;
        double y;
        double z;
        int leftson;
        int rightson;
    };
    
    /*
    Type Pointcloud contains a 2D Tree with Cpoints and
     the x and y-coordinates of the area (bbox) in which the
     Cpoints are located.  
    */
    class PointCloud : public StandardSpatialAttribute<2> {
        public:
        // constructor doing nothing
        PointCloud() {}
        // constructor initialzing the object
        PointCloud(const int anzp, const double *X= 0, const double *Y= 0, 
            const double *Z= 0, const int *Lson= 0, const int *Rson= 0,
            const double Xmin = 0, const double Xmax = 0, 
            const double Ymin = 0, const double Ymax = 0): 
            StandardSpatialAttribute(true),
            cpoint2dtree ( anzp ) {
            SetDefined(true);
            if(anzp > 0) {
                for( int i = 0; i < anzp; i++) {
                    cpoint2dtree.Append(
                        Cpointnode(X[i], Y[i], Z[i], Lson[i], Rson[i]));
                } 
            }
            minX  = Xmin;
            maxX = Xmax;
            minY  = Ymin;
            maxY = Ymax;
        }
        // destructor
        ~PointCloud() {}

        /*
        BasicType
        */    
        static const std::string BasicType() { return "pointcloud"; }

        /*
        checkType
        */
        static  const bool checkType(const ListExpr list){
            return listutils::isSymbol(list, BasicType());
        }
        
        /*
        Getter / Setter and help functions
        */
        double getMinX();
        double getMaxX();
        double getMinY();
        double getMaxY();
        void     setMinX(const double minx);
        void     setMaxX(const double maxx);
        void     setMinY(const double miny);
        void     setMaxY(const double maxy);        
        
        /*
        NumOfFlobs
        */
        int NumOfFLOBs() const;
        
        /*
        GetFlob
        */
        Flob* GetFLOB ( const int i );
        
        /*
        GetNoCpointnodes
        */
        int GetNoCpointnodes();
  
        /*
        Get Cpointnode
        */
        Cpointnode GetCpointnode(int i);

        /*
        SetCpointnode
        */
        void SetCpointnode(int i, Cpointnode* cpnode);
      
        /*
        AppendCpointnode
        */
        void AppendCpointnode(const Cpointnode &cpntnode);
  
        /*
        Destroy
        */
        void DestroyPointCloud();
  
        /*
        Compare
        */
        int Compare (const Attribute * arg ) const;
        
        /*
        Adjacent
        */
        bool Adjacent ( const Attribute * arg) const;
        
        /*
        Sizeof PointCloud
        */
        size_t Sizeof () const;
        
        /*
        HashValue
        */
        size_t HashValue () const;
        
        /*
        CopyFrom
        */
        void CopyFrom ( const Attribute* arg );
  
        /*
        Clone
        */
        Attribute *Clone () const;

        /*
        getAllPointsInRange
        */ 
        Cpoints* getAllPointsInRange(const double x1, const double y1, 
                                    const double x2, const double y2)  {
            // Go through the 2D Tree, search the area with help of 
            // intersection of regions built by the points.
            double r2min[2], r2max[2];
            r2min[0] = (x1 < x2)? x1: x2;
            r2max[0] = (x1 < x2)? x2: x1;    
            r2min[1] = (y1 < y2)? y1: y2;
            r2max[1] = (y1 < y2)? y2: y1;    
            Cpoints* rangePoints = new Cpoints(0);
            int chkDim = 1;
            int idx = 0;
            Rectangle<2> rect2 = Rectangle<2>(true, r2min, r2max); 
            if (!this->Intersects(rect2)) {
                cout << "No point within requested area" << endl;
                return 0;
            }
            // go through the 2D Tree
            search2DTree(this->BoundingBox(), rect2, rangePoints, idx, chkDim);
            return rangePoints;
        }

        void search2DTree(Rectangle<2> rect1, Rectangle<2> rect2, 
                                   Cpoints* resultPoints, int idx, int chkDim) {
            if (this->GetNoCpointnodes() < 1) {
                // there are no points within this PC
                // cout << "PC does not contain any points." << endl;
            } else {
                Cpointnode tempnode = this->GetCpointnode(idx);
                Cpointnode* cptnode = &tempnode;
                double curX, curY;
                bool searchLeft = false;
                bool searchRight = false;
                double r3min[2], r3max[2];
                double r4min[2], r4max[2];
                curX = cptnode->getX();
                curY = cptnode->getY();
                // check if current point matches
                if ((curX >= ((Rectangle<2>*) &rect2)->Rectangle<2>::MinD(0)) 
                    && 
                    (curX <= ((Rectangle<2>*) &rect2)->Rectangle<2>::MaxD(0)) 
                    &&
                    (curY >= ((Rectangle<2>*) &rect2)->Rectangle<2>::MinD(1)) 
                    && 
                    (curY <= ((Rectangle<2>*) &rect2)->Rectangle<2>::MaxD(1))) {
                    resultPoints->AppendCpoint(Cpoint(curX, curY, 
                                                      cptnode->getZ()));
                }
                if (chkDim == 1) {
                    // check X
                    // build new rectangles to check for intersection
                    // determine search direction upon result
                    // cout << " Check dim X" << endl;
                    r3min[0] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(0);
                    r3min[1] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(1);
                    r3max[0] = curX;
                    r3max[1] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(1);
                    r4min[0] = curX; 
                    r4min[1] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(1);
                    r4max[0] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(0);
                    r4max[1] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(1); 

                } else {
                    // check y
                    // build new rectangles to check for intersection
                    // determine search direction upon result
                    // cout << " Check dim Y" << endl;
                    r3min[0] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(0);
                    r3min[1] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(1);
                    r3max[0] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(0);
                    r3max[1] = curY;
                    r4min[0] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(0);
                    r4min[1] = curY;
                    r4max[0] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(0);
                    r4max[1] = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(1);
                }
                Rectangle<2> rect3 = Rectangle<2>(true, r3min, r3max);
                Rectangle<2> rect4 = Rectangle<2>(true, r4min, r4max);
                if (intersects2(rect3, rect2)) {
                    searchLeft = true;
                } else {
                    searchLeft = false;
                }
                if (intersects2(rect4, rect2)) {
                    searchRight = true;
                } else {
                    searchRight = false;
                }
                if ((searchLeft) && (cptnode->getLeftSon() != -1)) {
                    search2DTree(rect3, rect2, resultPoints, 
                         cptnode->getLeftSon(), (-1 * chkDim)); 
                }
                if ((searchRight) && (cptnode->getRightSon() != -1)) {
                    search2DTree(rect4, rect2, resultPoints, 
                         cptnode->getRightSon(), (-1 * chkDim)); 
                }
            }
        } 
        
        /*
       BoundingBox (SpatialAttribute)
       */
       const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const {
           double cpmin[2], cpmax[2];
           cpmin[0] = minX;
           cpmin[1] = minY;
           cpmax[0] = maxX;
           cpmax[1] = maxY;
           return Rectangle<2>(true, cpmin, cpmax);
       }
       /*
       getMinMaxZ
       */
       void getMinMaxZ(double& min, double& max) {
         if (IsEmpty()) {
           min = 0;
           max = 0;
           return;
         }
         min = std::numeric_limits<double>::max();
         max = std::numeric_limits<double>::min();
         Cpoints *cps = getAllPointsInRange(minX, minY, maxX, maxY);
         double z;
         cout << "consider " << cps->GetNoCpoints() << " cpoints" << endl;
         for (int i = 0; i < cps->GetNoCpoints(); i++) {
           z = cps->GetCpoint(i).getZ();
           if (z < min) {
             min = z;
           }
           if (z > max) {
             max = z;
           }
         }
       }
       
       /*
       BoundingBox3d
       */
       const Rectangle<3> BoundingBox3d() {
         double min[3], max[3];
         min[0] = minX;
         min[1] = minY;
         max[0] = maxX;
         max[1] = maxY;
         getMinMaxZ(min[2], max[2]);
         cout << min[0] << " " << max[0] << " | " 
              << min[1] << " " << max[1] << " | "
              << min[2] << " " << max[2] << " | " << endl;
         return Rectangle<3>(true, min, max);
       }

       /*
       Distance (SpatialAttribute)
       */        
       double Distance(const Rectangle<2>& rect,
                            const Geoid* geoid=0) const {
           // Remark: Usage of Geoid not implemented.
           // This method calculates the shortest distance
           // between the given two Bboxes
           // 
           if (this->Intersects(rect)) {
               // interesection means distance = 0.0
               return 0.0;
           }
           // Calculate minimum distance between Bboxes
           // position of bboxes to each other not known yet.
           Rectangle<2> checkRect = rect;
           double distX = 0.0;
           double distY = 0.0;
           double minX2 = ((Rectangle<2>*) &checkRect)->Rectangle<2>::MinD(0);
           double maxX2 = ((Rectangle<2>*) &checkRect)->Rectangle<2>::MaxD(0);
           double minY2 = ((Rectangle<2>*) &checkRect)->Rectangle<2>::MinD(1);
           double maxY2 = ((Rectangle<2>*) &checkRect)->Rectangle<2>::MaxD(1); 
           if (maxX2 < minX) {
               distX = minX - maxX2;
           } else {
               if (maxX < minX2) {
                   distX = minX2 - maxX;
               } 
           }
           if (maxY2 < minY) {
               distY = minY - maxY2;
           } else {
               if (maxY < minY2) {
                   distY = minY2 - maxY;
               } 
           }
           if (distX < distY) {
               return distX;
           }
           return distY;
       }
    
       /*
       Intersects (SpatialAttribute)
       */     
       bool Intersects(const Rectangle<2>& rect,
                            const Geoid* geoid=0 ) const {
           double minX2 = ((Rectangle<2>*) &rect)->Rectangle<2>::MinD(0);
           double maxX2 = ((Rectangle<2>*) &rect)->Rectangle<2>::MaxD(0);
           double minY2 = ((Rectangle<2>*) &rect)->Rectangle<2>::MinD(1);
           double maxY2 = ((Rectangle<2>*) &rect)->Rectangle<2>::MaxD(1); 
           bool disjointX, disjointY;     
           disjointX = ((maxX2 < minX) || (maxX < minX2)) ? true : false;
           disjointY = ((maxY2 < minY) || (maxY < minY2)) ? true : false;

           return (!disjointX && !disjointY);
       }

       /*
       IsEmpty (SpatialAttribute)
       */     
       bool IsEmpty() const {
           // check amount of elem in point
           if (this->Sizeof() > 0) {
               return false;
           }
           return true;
       }
        
       bool intersects2(const Rectangle<2> rect1, const Rectangle<2> rect2) {
           // Check wether given rectangles intersect 
           double minX1 = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(0);
           double maxX1 = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(0);
           double minY1 = ((Rectangle<2>*) &rect1)->Rectangle<2>::MinD(1);
           double maxY1 = ((Rectangle<2>*) &rect1)->Rectangle<2>::MaxD(1);
           double minX2 = ((Rectangle<2>*) &rect2)->Rectangle<2>::MinD(0);
           double maxX2 = ((Rectangle<2>*) &rect2)->Rectangle<2>::MaxD(0);
           double minY2 = ((Rectangle<2>*) &rect2)->Rectangle<2>::MinD(1);
           double maxY2 = ((Rectangle<2>*) &rect2)->Rectangle<2>::MaxD(1);
           bool disjointX, disjointY;     
           disjointX = ((maxX2 < minX1) || (maxX1 < minX2)) ? true : false;
           disjointY = ((maxY2 < minY1) || (maxY1 < minY2)) ? true : false;
           return (!disjointX && !disjointY);
       }        

       private:
        DbArray<Cpointnode> cpoint2dtree;
        // Bounding box of pointcloud
        double minX;
        double maxX;
        double minY;
        double maxY;  
    };    

}

#endif //SECONDO_POINTCLOUD_H
