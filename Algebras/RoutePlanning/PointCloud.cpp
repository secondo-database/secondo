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

#include "PointCloud.h"

namespace routeplanningalgebra {

    /*
    PointCloud object and related types added and maintained
    by Gundula Swidersky, Dec 2017.
    */
    
    /*
    Begin Cpoint
    */

    /*
    NumOfFlobs
    */
    int Cpoint::NumOfFLOBs() const {
        return 0;
    }
  
    /*
    GetFlob
    */
    Flob* Cpoint::GetFLOB ( const int i ) {
        assert(false);
    }

    /*
    Compare
    */
    int Cpoint::Compare (const Attribute * arg ) const {
        // first compare defined flags
        if(!IsDefined()) {
            return arg->IsDefined()?-1:0;
        }
        if(!arg->IsDefined()) {
            return 1;
        }
        Cpoint* p = (Cpoint*) arg;
        if (x < p->x) return -1;
        if (x > p->x) return 1;
        if (y < p->y) return -1;
        if (y > p->y) return 1;
        return 0;
    }
  
    /*
    Adjacent
    */
    bool Cpoint::Adjacent ( const Attribute * arg) const {
         return false;
    }

    /*
    Sizeof Cpoint
    */
    size_t Cpoint::Sizeof () const {
        return sizeof(*this);
    }
  
    /*
    HashValue
    */
    size_t Cpoint::HashValue () const {
        if(!IsDefined()) {
            return 0;
        }
        return (size_t) (x+y);
    }
  
    /*
    CopyFrom
    */
    void Cpoint::CopyFrom ( const Attribute* arg )  {
      if(!arg->IsDefined()) {
        SetDefined(false);
        return;
      }
      *this = (*(Cpoint*) arg);
      SetDefined(true);
   }
    
    /*
    Clone
    */
    Attribute *Cpoint::Clone () const {
        Cpoint* res = new Cpoint(x,y,z);
        if(!this->IsDefined()) {
            res->SetDefined(false);
        }
        return res;
    }

    /*
    Getter / Setter and help functions
    */
    double Cpoint::getX() const { return x; }
    double Cpoint::getY() const { return y; }
    double Cpoint::getZ() const { return z; }

    /*
    Begin Cpoints (DBArray of type Cpoint)
    */

    /*
    NumOfFlobs
    */
    int Cpoints::NumOfFLOBs() const {
        return 1;
    }

    /*
    GetFlob
    */
    Flob* Cpoints::GetFLOB ( const int i ) {
        if (i == 0) {
            return &cpoints;
        }
        return 0;
        // assert(false);
    }

    /*
    GetNoCpoints
    */
    int Cpoints::GetNoCpoints() {
        return (cpoints.Size());
    }

    /*
    GetCpoint
    */
    Cpoint Cpoints::GetCpoint(int i) {
        Cpoint cpelem;
        cpoints.Get(i, &cpelem);
        return cpelem;
    }
  
    /*
    AppendCpoints
    */
    void Cpoints::AppendCpoint(const Cpoint &cpnt) {
        cpoints.Append(cpnt);
    }
  
    /*
    Destroy
    */
    void Cpoints::DestroyCpoints() {
        cpoints.Destroy();
    }

    /*
    Compare
    */
    int Cpoints::Compare (const Attribute * arg ) const {
        // first compare defined flags
        if(!IsDefined()) {
            return arg->IsDefined()?-1:0;
        }
        if(!arg->IsDefined()) {
            return 1;
        }
        // Cpoints* s = (Cpoints*) arg;
        // TODO
        // if (*this->Size() < s->Size()) return -1;
        // if (*this->Size() > s->Size()) return 1;
        return 0;
    }

    /*
    Adjacent
    */
    bool Cpoints::Adjacent ( const Attribute * arg) const {
        return false;
    }

    /*
    Sizeof
    */    
    size_t Cpoints::Sizeof () const {
        return sizeof(*this);
    }

    /*
    HashValue
    */    
    size_t Cpoints::HashValue () const  {
        if(!IsDefined()) {
            return 0;
        }
        double sumHash = 0.0;
        Cpoint cpelem;
        Cpoint* pcpelem;
        for (int i = 0; i < cpoints.Size(); i++) {
            cpoints.Get(i, cpelem);
            pcpelem = &cpelem;
            sumHash = sumHash + 
            (pcpelem)->getX() +
            (pcpelem)->getY() +            
            (pcpelem)->getZ();
        }
        return (size_t) sumHash; 
    }

    /*
    CopyFrom
    */
    void Cpoints::CopyFrom ( const Attribute* arg )  {
        if(!arg->IsDefined()) {
            SetDefined(false);
            return;
        }
        *this = (*(Cpoints*) arg);
        SetDefined(true);
   }
  
    /*
    Clone
    */
    Attribute *Cpoints::Clone () const {
       Cpoints* res = new Cpoints(0);
       res->SetDefined(true);
       Cpoint cpelem;
       if(!this->IsDefined()) {
           res->SetDefined(false);
       }
       for (int i = 0; i < cpoints.Size(); i++) {
           cpoints.Get(i, cpelem);
           res->AppendCpoint(cpelem);
       }
       return res;
   }

    /*
    Begin Cpointnode
    */

    /*
    Getter / Setter and help functions
    */
    double Cpointnode::getX() const { return x; }
    double Cpointnode::getY() const { return y; }
    double Cpointnode::getZ() const { return z; }
    int Cpointnode::getLeftSon() const { return leftson; }
    int Cpointnode::getRightSon() const { return rightson; }

    void Cpointnode::setLeftSon(int lSon) { 
        leftson = (lSon < 0) ? -1 : lSon; 
    }

    void Cpointnode::setRightSon(int rSon) {
        rightson = (rSon < 0) ? -1 : rSon;
    }

    /*
    NumOfFlobs
    */
    int Cpointnode::NumOfFLOBs() const {
        return 0;
    }
 
    /*
    GetFlob
    */
    Flob* Cpointnode::GetFLOB ( const int i ) {
        assert(false);
    }

    /*
    Compare
    */    
    int Cpointnode::Compare (const Attribute * arg ) const {
        // first compare defined flags
        if(!IsDefined()) {
            return arg->IsDefined()?-1:0;
        }
        if(!arg->IsDefined()) {
            return 1;
        }
        Cpointnode* p = (Cpointnode*) arg;
        if (x < p->x) return -1;
        if (x > p->x) return 1;
        if (y < p->y) return -1;
        if (y > p->y) return 1;
        return 0;
    }
  
    /*
    Adjacent
    */
    bool Cpointnode::Adjacent ( const Attribute * arg) const {
         return false;
    }
 
    /*
    Sizeof
    */
    size_t Cpointnode::Sizeof () const {
        return sizeof(*this);
    }

    /*
    HashValue
    */
    size_t Cpointnode::HashValue () const {
        if(!IsDefined()) {
            return 0;
        }
        return (size_t) (x+y);
    }
  
    /*
    CopyFrom
    */
    void Cpointnode::CopyFrom ( const Attribute* arg )  {
        if(!arg->IsDefined()) {
            SetDefined(false);
            return;
        }
        *this = (*(Cpointnode*) arg);
        SetDefined(true);
     }

    /*
    Clone
    */
    Attribute *Cpointnode::Clone () const {
        Cpointnode* res = new Cpointnode(x,y,z,leftson,rightson);
        if(!this->IsDefined()) {
            res->SetDefined(false);
        }
        return res;
    }
    
    /*
    Begin PointCloud 
    */

    /*
    Getter / Setter and help functions
    */
    double PointCloud::getMinX() { return minX; }
    double PointCloud::getMaxX() { return maxX; }
    double PointCloud::getMinY() { return minY; }
    double PointCloud::getMaxY() { return maxY; }

    void     PointCloud::setMinX(const double minx) {
        minX = minx;
    }

    void     PointCloud::setMaxX(const double maxx) {
        maxX = maxx;
    }

    void     PointCloud::setMinY(const double miny) {
        minY = miny;
    }

    void     PointCloud::setMaxY(const double maxy) {
        maxY = maxy;
    }        
        
    /*
    NumOfFlobs
    */
    int PointCloud::NumOfFLOBs() const {
        return 1;
    }
       
    /*
    GetFlob
    */
    Flob* PointCloud::GetFLOB ( const int i ) {
        if (i == 0) {
            return &cpoint2dtree;
        }
        return 0;
    }
  
    /*
    GetNoCpointnodes
    */
    int PointCloud::GetNoCpointnodes() {
        return (cpoint2dtree.Size());
    }
  
    /*
    GetCpointnode
    */
    Cpointnode PointCloud::GetCpointnode(int i) {
        Cpointnode cpelem;
        cpoint2dtree.Get(i, &cpelem);
        return cpelem;
    }

        
    /*
    AppendCpointnode
    */
    /*
    void PointCloud::AppendCpointnode(const Cpointnode &cpntnode) {
        cpoint2dtree.Append(cpntnode);
        double z = cpntnode.getZ();
        if(z<minZ) minZ = z;
        if(z>maxZ) maxZ = z;
    }
    */
  
    /*
    DestroyPointCloud
    */
    void PointCloud::DestroyPointCloud() {
        cpoint2dtree.Destroy();
    }
  
    /*
    Compare
    */
    int PointCloud::Compare (const Attribute * arg ) const {
        // first compare defined flags
        if(!IsDefined()) {
            return arg->IsDefined()?-1:0;
        }
        if(!arg->IsDefined()) {
            return 1;
        }
        // TODO define criteria to be compared e.g. like below
        // PointCloud* s = (PointCloud*) arg;
        // if (point2dtree.Size() < s.Size()) return -1;
        // if (point2dtree.Size() > s.Size()) return 1;
        return 0;
    }

    /*
    Adjacent
    */
    bool PointCloud::Adjacent ( const Attribute * arg) const {
        return false;
    }
        
    /*
    Sizeof
    */
    size_t PointCloud::Sizeof () const {
        return sizeof(*this);
    }
        
    /*
    HashValue
    */
    size_t PointCloud::HashValue () const  {
        if(!IsDefined()) {
            return 0;
        }
        double sumHash = 0.0;
        Cpointnode cpelem;
        Cpointnode* pcpelem;
        for (int i = 0; i < cpoint2dtree.Size(); i++) {
            cpoint2dtree.Get(i, cpelem);
            pcpelem = &cpelem;
            sumHash = sumHash + 
            (pcpelem)->getX() +
            (pcpelem)->getY() +            
            (pcpelem)->getZ();
        }
        return (size_t) sumHash; 
    }
        
    /*
    CopyFrom
    */
    void PointCloud::CopyFrom ( const Attribute* arg )  {
        if(!arg->IsDefined()) {
            SetDefined(false);
            return;
        }
        *this = (*(PointCloud*) arg);
        SetDefined(true);
    }
  
    /*
    Clone
    */
    Attribute *PointCloud::Clone () const {
        return new PointCloud(*this);
    }
   

    bool PointCloud::insert(const double x, const double y, const double z){
      if(cpoint2dtree.Size()==0){
        cpoint2dtree.Append(Cpointnode(x,y,z,-1,-1)); // first node
        minZ = maxZ = z;
        return true;
      }
      // correct z dimension
      if(z<minZ) minZ = z;
      if(z>maxZ) maxZ = z;

      // search insertion position
      int idx = 0;
      Cpointnode node;
      bool xdim = true;
      cpoint2dtree.Get(idx,node);
      bool done = false;
      bool left;
      do{
        if(xdim){
          left = x < node.getX();
        } else {
           left = y < node.getY();
        }
        int son = left?node.getLeftSon():node.getRightSon();
        done = son==-1;
        if(!done){
          xdim = !xdim;
          idx = son;
          cpoint2dtree.Get(idx,node);
        }
      } while(!done);

      int pos =  cpoint2dtree.Size();
      if(left){
         node.setLeftSon(pos);
      } else {
         node.setRightSon(pos);         
      }
      cpoint2dtree.Put(idx,node); // store corrected son
      cpoint2dtree.Append(Cpointnode(x,y,z,-1,-1));
      return true; 
    } 

    
}


std::ostream& operator<<(std::ostream& out, 
                         const routeplanningalgebra::PointCloud& pc){
   return pc.print(out); 
}

