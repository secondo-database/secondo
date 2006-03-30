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

package viewer.hoese.algebras.periodic;

import viewer.hoese.*;
import sj.lang.ListExpr;
import java.awt.geom.*;
import tools.Reporter;

public class PMRealUnit extends LinearMove{

  public PMRealUnit(){
    a=b=c=0;
    root=false;
    interval = new RelInterval();
    defined = false;
  }

  public String getName(){
    return "pmreal";
  }


  /** returns a Double */
  public Object getObjectAt(Time T){
     if(!defined){
        Reporter.debug("PMRealUnit.getObjectAt called on an undefined instance");
        return null;
     }
     if(!interval.contains(T)){
        return null;
     }
     double p = T.getDouble();
     double res1 = (a*p +b)*p+c;
     if(root){
          if(res1<0){
              Reporter.debug("Trying to compute thr square root of a negative number in PMRealUnit::getObjectAt");
              return null;
           }
           return new Double(Math.sqrt(res1));
     }
     return new Double(res1);
  }

  protected boolean readMap(ListExpr map){
     if(map.listLength()!=4)
          return false;
     Double A = LEUtils.readNumeric(map.first());
     Double B = LEUtils.readNumeric(map.second());
     Double C = LEUtils.readNumeric(map.third());
     if(A==null || B==null || C==null || map.fourth().atomType() !=ListExpr.BOOL_ATOM){
       Reporter.debug("PMRealUnit.readStartEnd : error in map");
       return false;
     }
     a = A.doubleValue();
     b = B.doubleValue();
     c = C.doubleValue();
     root = map.fourth().boolValue();
     isstatic = (a==0 && b==0);
     defined = true;
     return true;
  }

  private double a;
  private double b;
  private double c;
  private boolean root;


}
