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




#pragma once

#include <set>
#include <string>
#include <iostream>
#include <assert.h>
#include "Stream.h"

#include "Attribute.h" 
#include "GenericTC.h"
#include "Algebras/Spatial/SpatialAlgebra.h"



/*
Class ~Convex~


This class represents a set of point values forming a convex polygon.  


*/



namespace convex{


class Convex: public Attribute{
  public:
     Convex() {}  // use in cast only
     Convex(const bool def): Attribute(def), value(0),size(0){}
     Convex(const Convex& src);
     Convex(Convex&& src);
     Convex(const std::vector<std::tuple<double, double>>& src, int id);
     Convex(const std::vector<std::tuple<double, double>>& src);

     Convex& operator=(const Convex& src);
     Convex& operator=(Convex&& src);
     ~Convex(){
       if(value){ delete[] value; }
     }

    std::vector<Convex> getVoronoiVector();
    void setVoronoiVector(std::vector<Convex> voronoi_vect);


    // auxiliary functions
    static const std::string BasicType(){ return "convex"; }
    static const bool checkType(ListExpr type){
      return listutils::isSymbol(type,BasicType());
    }
    // attribute related functions
    inline int NumOfFLOBs() const{
      return 0;
    }
    inline virtual Flob* GetFLOB(const int i){
       return 0;
    }
    int Compare(const Convex&  arg)const;

    int Compare(const Attribute* arg)const{
      return Compare( * ((Convex*)arg));
    }

    bool Adjacent(const Attribute* arg) const{
      return false;
    }
    size_t Sizeof() const{
       return sizeof(*this);
    }
    size_t HashValue() const;
  
    std::ostream& Print( std::ostream& os ) const;

    void CopyFrom(const Attribute* arg){
       *this = *((Convex*) arg);
    }   
    Attribute* Clone() const{
       
        
       return new Convex(*this);
    }
   //functions supporting the embedding into secondo
    static ListExpr Property(){
        return gentc::GenProperty( " -> DATA",
                                   BasicType(),
                                   "((x1, y1) (x2 y2) (x3 y3) ...)",
                                   "((10.2 2.2) (11.1 4.2) (1.3 3.7))");
    }
    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
        return checkType(type);
    }

    static Word In(const ListExpr typeInfo, const ListExpr le,
                const int errorPos, ListExpr& errorInfo, bool& correct);


    static ListExpr Out(const ListExpr typeInfo, Word value);

    static Word Create(const ListExpr typeInfo){
       Word res( new Convex(false));
       return res;
    }   

    static void Delete(const ListExpr typeInfo, Word& v){
       delete (Convex*) v.addr;
       v.addr = 0;
    }

    static bool Open( SmiRecord& valueRecord, size_t& offset, 
                       const ListExpr typeInfo, Word& value);

   static bool Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);


   static void Close(const ListExpr typeInfo, Word& w){
     delete (Convex*) w.addr;
     w.addr = 0;
   }
   static Word Clone(const ListExpr typeInfo, const Word& w){
     
      Convex* v = (Convex*) w.addr;
      Word res;
      res.addr = new Convex(*v);
      return res;
   }
   
   
   
   

   static int Size() {
        return 256; // ??
   }

   static void* Cast(void* addr){
       return addr;
   }

   static bool TypeCheck(ListExpr type, ListExpr& errorInfo){
      return checkType(type);
   }

   inline virtual StorageType GetStorageType() const{
      return Extension;
   }

   inline virtual size_t SerializedSize() const{
       if(!IsDefined()) return sizeof(bool);


       size_t res =  sizeof(bool) + sizeof(size_t) 
              + size * sizeof(Point) + sizeof(int); 
       return res;
   }
    
  inline virtual void Serialize(char* buffer, size_t sz, 
                                 size_t offset) const{
      bool def = IsDefined();
      memcpy(buffer+offset, &def, sizeof(bool));
      offset += sizeof(bool);
      if(!def){
        return;
      }
      memcpy(buffer+offset, &size, sizeof(size_t));
      offset += sizeof(size_t);
      if(size > 0){
        memcpy(buffer+offset, value, size*sizeof(Point));
        offset += size*sizeof(Point);
      }
      for(size_t i=0;i<size;i++){
         void* v = (void*) &(value[i]);
         new (v) Point();
      }

      memcpy(buffer+offset, &cellId, sizeof(int));
        offset += sizeof(int);
   }

   inline virtual void Rebuild(char* buffer, size_t sz);
   
   
   
   
   

   virtual size_t GetMemSize() {
     return sizeof(*this) + size * sizeof(Point) + sizeof(int);
   }

   void clear(){
     size = 0;
     cellId = 0;
     if(value!=nullptr){
        delete[] value;
        value = nullptr;
     }
   }

   void setTo(const std::vector<std::tuple<double, double>>& src, int id);
   int getCellId();
   void setCellId(int id);

  
  //private:
     Point* value;   
     size_t size;
     int cellId;

     Convex(size_t size, Point* value, int id): Attribute(true), 
            value(value),size(size),cellId(id) {}


};
  // used for cellnos
   std::vector<Convex> voroVec {};







/*
 Class Convex3D

*/
struct Point3D {
   double x;
   double y;
   double z;
};

struct Tetrahedron {
  Point3D a;
  Point3D b;
  Point3D c;
  Point3D d;
};

class Polyhedron {
  public:
    Polyhedron();
    ~Polyhedron();

    Polyhedron* getNeighbor();
    int getPolyId();

    // Auxiliary functions for In or/and Value mapping functions
    void setNeighbor(Polyhedron* neighbor_);
    void setPolyId(int id_);
    std::vector<std::vector<Point3D>> faces {};

    friend class Convex3D;

  private:
    Polyhedron* neighbor;
    int polyhedronId;
};


class Convex3D {
   public:

   // The first constructor.
   Convex3D(Rectangle<3> &bounding_box);

   // copy constructor
   Convex3D( const Convex3D& g );

    // Auxiliary functions for In or/and Value mapping functions
    void Set(Stream<Rectangle<3>> rStream);
    int getCellId();
    void setCellId(int cell_id);
    std::vector<Tetrahedron*> &getTetrahedronVector();
    void setTetrahedronVector(std::vector<Tetrahedron*> tetravec);
    std::vector<Polyhedron> &getPolyhedronVector();
    void setPolyhedronVector(std::vector<Polyhedron> polyvec);

    std::vector<Point3D> &getPointsVector();
    void setPointsVector(std::vector<Point3D> pointsvec);

    std::vector<Point3D> &getVerticeVector();
    void setVerticeVector(std::vector<Point3D> verticevec);
    std::vector<std::vector<int>> &getFacesVector();
    void setFacesVector(std::vector<std::vector<int>> facevec);
    std::vector<std::vector<int>> &getV2TVector();
    void setV2TVector(std::vector<std::vector<int>> v2tvec);
    std::vector<std::vector<int>> &getF2TVector();
    void setF2TVector(std::vector<std::vector<int>> f2tvec);
    

   // Algebra supporting functions

    static ListExpr PropertyConvex3D();

    static ListExpr OutConvex3D(ListExpr typeInfo, Word value);

    static Word InConvex3D( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct);

    static Word CreateConvex3D( const ListExpr typeInfo);

    static void DeleteConvex3D( const ListExpr typeInfo, Word& w);

    static void Close(const ListExpr typeInfo, Word& w){
     delete (Convex3D*) w.addr;
     w.addr = 0;
    }
    static Word CloneConvex3D( const ListExpr typeInfo, const Word& w);

    static int SizeOfConvex3D();

    static bool KindCheckConvex3D (ListExpr type, ListExpr& errorInfo);

    static const std::string BasicType() {
      printf("\n in basictype");
      return "convex3d";
    }

    static const bool checkType(const ListExpr type) {
      return listutils::isSymbol(type, BasicType());
    }

    ~Convex3D();

    private: 
    Convex3D();

    // vector with points of input cuboids
    std::vector<Point3D> points {};

    // Data structure for delaunay
    // Vector of vertices
    std::vector<Point3D> vertices {};
    // Vector of faces with indices to points included by face
    // 3 points per face
    std::vector<std::vector<int>> faces {};
    // vector tetrahedrons from vertices
    // with indices to vertices
    std::vector<std::vector<int>> v2t {};
    // vector tetrahedrons from faces 
    // with indices to faces
    std::vector<std::vector<int>> f2t {};


    void CreateVoronoi3D(Stream<Rectangle<3>> rStream);
    void buildVoronoi3D(Stream<Rectangle<3>> rStream);
    void createDelaunay (std::vector<Point3D> points,
         std::vector<Tetrahedron>* tetravec);
    bool faceIn(std::vector<int> face, int* pos);
    bool tetExists(int a, int b, int p, int d, int* pos);
    bool duplicateP(Point3D p);
    
    // cellId => polyhedronId
    int polyhedronId;
    Rectangle<3> *boundingBox;
    std::vector<Tetrahedron*> tetravec;
    std::vector<Polyhedron> polyhedronvec {};
};



/*struct CellInfoConvex3D {
  int cellId;
  Convex3D* cell;

  CellInfoConvex3D (int c_id,
    double left, double right, double bottom, double top, double front, double back) {
    cellId = c_id;

    double min[3], max[3];
    min[0] = left;
    min[1] = bottom;
    min[2] = front;
    max[0] = right;
    max[1] = top;
    max[2] = back;

    cell = new Convex3D;
  }
};*/

} //end of namespace
