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

double x;
double y;
double z;


private double readNumeric(ListExpr l) throws NumberFormatException{
   switch(l.atomType()){
    case ListExpr.INT_ATOM: return l.intValue();
    case ListExpr.REAL_ATOM: return l.realValue();
   }
   throw new NumberFormatException();
}

SingleFPoint(){
  x=0;
  y=0;
  z=1.0;
}



public boolean readFromListExpr(ListExpr LE, boolean isPointcloud){
   if (LE==null)
      return false;
   if(LE.listLength()<3){
      Reporter.writeError("wrong listlength"+LE.listLength());
      return false;
   }
   ListExpr LE1 = LE.first();
   ListExpr LE2 = LE.second();
   ListExpr LE3 = LE.third();
   try {
     double tx = readNumeric(LE1);
     double ty = readNumeric(LE2);
     double tz = readNumeric(LE3);
     x = tx;
     y = ty;
     z = tz;
     if(!isPointcloud){
        if(z<0 || z > 1){
           Reporter.writeError("error during reading coordinates:");
           return false;
        }
     }
     return true;
   } catch(NumberFormatException e){
      Reporter.writeError("error during reading coordinates:");
      return false;
     
   }
}

}
