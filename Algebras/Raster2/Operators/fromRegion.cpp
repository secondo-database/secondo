/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#include "fromRegion.h"
#include "fromLine.h"
#include "RobustSetOps.h"
#include "AvlTree.h"
#include "AVLSegment.h"

namespace raster2 {



 enum EventKind {START,END,CELL};

 class FromRegionEvent{
   public:
      FromRegionEvent(EventKind _kind, double _x, avlseg::AVLSegment* _segment):
         kind(_kind), x(_x), segment(_segment) {}

      ~FromRegionEvent(){
         if(segment) delete segment;
      }      

      EventKind getKind() const { return kind; } 
      avlseg::AVLSegment* getSegment() const { return segment; }
      double getX() const{ return x; }
      
   private:
     EventKind kind;
     double x;
     avlseg::AVLSegment* segment; 

 };


 void toMiddle(const grid2* grid, double&x, double& y){
    double length = grid->getLength();
    Rectangle<2> cb = grid->getCell(grid->getIndex(x,y));
    x = cb.MinD(0) + length/2;
    y = cb.MinD(1) + length/2;
 }


 class fromRegionES{
   public:
      fromRegionES(Region* _r, grid2* _grid): 
                r(_r), pos(0), firstX(0), length(_grid->getLength()), steps(0){

        Rectangle<2> rbb = r->BoundingBox();
        firstX = rbb.MinD(0);
        double y = rbb.MinD(1);
        toMiddle(_grid,firstX,y);
    }

      bool hasNext(){
        bool hasNext =  pos <  r->Size(); 
        return hasNext;
      }

      FromRegionEvent getNext() {
          HalfSegment hs;
          r->Get(pos,hs);
          double hsx = hs.GetDomPoint().GetX();
          double cx = firstX + steps*length;
          if(hsx< cx){
              EventKind kind = hs.IsLeftDomPoint()?START:END;
              pos++;
              return FromRegionEvent(kind, hsx,
                                     new avlseg::AVLSegment(hs, avlseg::first));
          } else {
             steps++;
             return FromRegionEvent(CELL,cx,0); 
          } 
      }

    private:
       Region * r;
       int pos;
       double firstX;
       double length;
       int steps; 
 };


 void processCell(sbool* raster, grid2* grid, double x, 
                  avltree::AVLTree<avlseg::AVLSegment>& sss){
    
    avltree::AVLTree<avlseg::AVLSegment>::iterator it = sss.begin();
    while(it.hasNext()){
       const avlseg::AVLSegment* first = *it;
       assert(it.hasNext());
       it++;
       const avlseg::AVLSegment* second = *it;
       it++;
       double y1 = first->getY(x);
       double y2 = second->getY(x);
       grid2::index_type i1 = grid->getIndex(x,y1);
       grid2::index_type i2 = grid->getIndex(x,y2);
       grid2::index_type index = i1;
       for(int i = i1[1]; i<=i2[1]; i++){
          index[1] = i;
          raster->set(index,true);
       }
    } 

     
    
 }


  int fromRegionFun
       (Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);
    sbool* res = (sbool*) result.addr;
    Region* r = (Region*) args[0].addr;
    grid2* grid = (grid2*) args[1].addr;

    res->clear();
    
    if(!r->IsDefined() || 
       grid->getLength()<=0){
       res->setDefined(false);
       return 0; 
    } 

    res->setGrid(*grid);
    fromRegionES  es(r,grid);
    avltree::AVLTree<avlseg::AVLSegment> sss;

    while(es.hasNext()){
       FromRegionEvent evt = es.getNext();
       switch(evt.getKind()){
         case START :  sss.insert(*evt.getSegment());break;
         case END   :  sss.remove(*evt.getSegment()); break;
         case CELL  :  processCell(res,grid, evt.getX(),sss);break;
       }
    }    
    return 0;
  }

  

    
    ListExpr fromRegionTypeMap(ListExpr args)
    {
        if(!nl->HasLength(args,2)){
          return listutils::typeError("2 arguments expected");
        }
        if(!Region::checkType(nl->First(args)) ||
           !grid2::checkType(nl->Second(args))){
          return listutils::typeError("region x grid2 expected");
        }
        return listutils::basicSymbol<sbool>();

    }
}
