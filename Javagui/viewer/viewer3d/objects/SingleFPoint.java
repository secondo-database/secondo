//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.viewer3d.objects;

import sj.lang.ListExpr;
import tools.Reporter;

class SingleFPoint{

int x;
int y;
double z;

SingleFPoint(){
  x=0;
  y=0;
  z=1.0;
}



public boolean readFromListExpr(ListExpr LE){
   if (LE==null)
      return false;
   if(LE.listLength()!=3){
      Reporter.writeError("wrong listlength"+LE.listLength());
      return false;
   }
   ListExpr LE1 = LE.first();
   ListExpr LE2 = LE.second();
   ListExpr LE3 = LE.third();
   int tx,ty;
   double tz;

   if(LE1.isAtom() && LE1.atomType()==ListExpr.INT_ATOM)
      tx = LE1.intValue();
   else{
      Reporter.writeError("error reading x:");
      return false;
   }

   if(LE2.isAtom() && LE2.atomType()==ListExpr.INT_ATOM)
      ty = LE2.intValue();
    else{
      Reporter.writeError("error reading y");
      return false;
    }

    if(LE3.isAtom() && ( LE3.atomType()==ListExpr.INT_ATOM | LE3.atomType()==ListExpr.REAL_ATOM))
       if (LE3.atomType()==ListExpr.INT_ATOM)
          tz=LE3.intValue();
       else
          tz=LE3.realValue();
    else{
       Reporter.writeError("error reading z");
       return false;
    }

    if(tz<0 | tz>1){
       Reporter.writeError("wrong z :"+z);
       return false;
    }

    this.x = tx;
    this.y = ty;
    this.z = tz;
    return true;
}

}
