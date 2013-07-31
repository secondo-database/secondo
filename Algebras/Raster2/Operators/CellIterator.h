
/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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


#ifndef CELL_ITERATOR_H
#define CELL_ITERATOR_H

#include <utility>
#include <assert.h>
#include "../grid2.h"
#include "../grid3.h"

class CellIterator{

  public:

     CellIterator(const raster2::grid2& _grid, 
                  const double _x1, const double _y1,
                  const double _x2, const double _y2):
       grid(_grid), x1(_x1), y1(_y1), x2(_x2), y2(_y2),
       hasNextX(false), hasNextY(false), nextX(0),
       nextY(0), lastDelta(0), currentDelta(0){
        init();
      }

     ~CellIterator(){}

     bool hasNext(){
        return lastDelta < 1.0;
     }

     pair<double,double> next(){
       pair<double,double> res(lastDelta,currentDelta);
       computeNextDelta();
       return res;
     } 


  private:   

      raster2::grid2 grid;
      double x1;
      double y1;
      double x2;
      double y2;
      bool hasNextX;
      bool hasNextY;
      double nextX;
      double nextY;
      double lastDelta;
      double currentDelta; 
      raster2::grid2::index_type currentCell;
      raster2::grid2::index_type lastCell;
      double deltaX;
      double deltaY;

     void init(){
        currentCell = grid.getIndex(x1,y1);
        lastCell = grid.getIndex(x2,y2);
        if(currentCell==lastCell){
           currentDelta = 1.0;
           return;
        }
        deltaX = x2-x1;
        deltaY = y2-y1;
        computeNextX();
        computeNextY();  
        computeNextDelta();
     }

     void computeNextX(){
       hasNextX = currentCell[0] != lastCell[0];
       if(hasNextX){
          double vertX;
          if(deltaX<0){
            // left border of cell
            vertX = grid.getOriginX() + currentCell[0]* grid.getLength();
          } else {
            // right border of cell
            vertX = grid.getOriginX() + (currentCell[0]+1)* grid.getLength();
          }
          nextX = (vertX - x1)/ deltaX;
       }
     }

     void computeNextY(){
       hasNextY = currentCell[1] != lastCell[1];
       if(hasNextY){
          double horY;
          if(deltaY<0){
            // bottom border of cell
            horY = grid.getOriginY() + currentCell[1]* grid.getLength();
          } else {
            // right border of cell
            horY = grid.getOriginY() + (currentCell[1]+1)* grid.getLength();
          }
          nextY = (horY - y1)/ deltaY;
       }
     }

     void shiftX(){
       if(deltaX>0){
           currentCell[0]++;
       } else if(deltaX < 0){
           currentCell[0]--;
       }
     }

     void shiftY(){
       if(deltaY>0){
           currentCell[1]++;
       } else if(deltaY < 0){
           currentCell[1]--;
       }
     }

     void computeNextDelta(){
        lastDelta = currentDelta;
        if(currentCell==lastCell){
           currentDelta = 1.0;
           return;
        }
        if(hasNextX){
           if(hasNextY){ // x and y deltas are present
              if(nextX==nextY){
                 currentDelta = nextX;
                 shiftX();
                 shiftY();
                 computeNextX();
                 computeNextY();
              } else if(nextX < nextY){
                 // shift in x direction
                 currentDelta = nextX;
                 shiftX();
                 computeNextX();
              } else {
                 // shift in y direction
                 currentDelta = nextY;
                 shiftY();
                 computeNextY(); 
              }
           } else { // only x delta is present
              currentDelta = nextX;
              shiftX();
              computeNextX();
           }
        } else {
          assert(hasNextY);
          currentDelta = nextY;
          shiftY();
          computeNextY(); 
        } 

     }

};


class CellIterator3{

  public:

     CellIterator3(const raster2::grid3& _grid, 
                  const double _x1, const double _y1,
                  const double _x2, const double _y2,
                  const double _t1, const double _t2):
       grid(_grid), x1(_x1), y1(_y1), x2(_x2), y2(_y2),
       t1(_t1), t2(_t2),
       hasNextX(false), hasNextY(false), hasNextT(false),
       nextX(0), nextY(0), nextT(0),
       lastDelta(0), currentDelta(0){
        init();
      }

     ~CellIterator3(){}

     bool hasNext(){
        return lastDelta < 1.0;
     }

     pair<double,double> next(){
       pair<double,double> res(lastDelta,currentDelta);
       computeNextDelta();
       return res;
     } 


  private:   

      raster2::grid3 grid;
      double x1;
      double y1;
      double x2;
      double y2;
      double t1;
      double t2;
      bool hasNextX;
      bool hasNextY;
      bool hasNextT;
      double nextX;
      double nextY;
      double nextT;
      double lastDelta;
      double currentDelta; 
      raster2::grid3::index_type currentCell;
      raster2::grid3::index_type lastCell;
      double deltaX;
      double deltaY;
      double deltaT;

     void init(){
        currentCell = grid.getIndex(x1,y1,t1);
        lastCell = grid.getIndex(x2,y2,t2);
        if(currentCell==lastCell){
           currentDelta = 1.0;
           return;
        }
        deltaX = x2-x1;
        deltaY = y2-y1;
        deltaT = t2-t1;
        computeNextX();
        computeNextY();
        computeNextT();  
        computeNextDelta();
     }

     void computeNextX(){
       hasNextX = currentCell[0] != lastCell[0];
       if(hasNextX){
          double vertX;
          if(deltaX<0){
            // left border of cell
            vertX = grid.getOriginX() + currentCell[0]* grid.getLength();
          } else {
            // right border of cell
            vertX = grid.getOriginX() + (currentCell[0]+1)* grid.getLength();
          }
          nextX = (vertX - x1)/ deltaX;
       }
     }

     void computeNextY(){
       hasNextY = currentCell[1] != lastCell[1];
       if(hasNextY){
          double horY;
          if(deltaY<0){
            // bottom border of cell
            horY = grid.getOriginY() + currentCell[1]* grid.getLength();
          } else {
            // right border of cell
            horY = grid.getOriginY() + (currentCell[1]+1)* grid.getLength();
          }
          nextY = (horY - y1)/ deltaY;
       }
     }

      
     void computeNextT(){
       hasNextT = currentCell[2] != lastCell[2];
       if(hasNextT){
          double horT;
          if(deltaT<0){
            // bottom border of cell
            horT = currentCell[2]* grid.getDuration().ToDouble();
          } else {
            // right border of cell
            horT = (currentCell[2]+1)* grid.getDuration().ToDouble();
          }
          nextT = (horT - t1)/ deltaT;
       }
     }



     void shiftX(){
       if(deltaX>0){
           currentCell[0]++;
       } else if(deltaX < 0){
           currentCell[0]--;
       }
     }

     void shiftY(){
       if(deltaY>0){
           currentCell[1]++;
       } else if(deltaY < 0){
           currentCell[1]--;
       }
     }

     void shiftT(){
        if(deltaT>0){
           currentCell[2]++;
        } else if(deltaT < 0){
           currentCell[2]--;  
        }
     }

     double getDV(int index){
        switch(index){
          case 0 : return nextX;
          case 1 : return nextY;
          case 2 : return nextT;
          default: assert(false);
        }
        return -1;
     }

     void computeNextDelta(){
        lastDelta = currentDelta;
        if(currentCell==lastCell){
           currentDelta = 1.0;
           return;
        }
        bool cand[3];
        assert(hasNextX || hasNextY || hasNextT);
        cand[0] = hasNextX;
        cand[1] = hasNextY;
        cand[2] = hasNextT;
        double min;
        bool first = true;
        for(int i=0;i<3;i++){
           if(cand[i]){
              if(first){
                 first = false;
                 min = getDV(i); 
              } else {
                 double dv = getDV(i);
                 if(dv<min){ // new minimum found
                    min = dv;
                    for(int j=0;j<i;j++){
                      cand[j] = false;
                    }
                 }
              }
           }
        }
        currentDelta = min;
        if(cand[0]){
          shiftX();
          computeNextX();
        }
        if(cand[1]){
           shiftY();
           computeNextY();
        }
        if(cand[2]){
           shiftT();
           computeNextT();
        }
     }

};


#endif


