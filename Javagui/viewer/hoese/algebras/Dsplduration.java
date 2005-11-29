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


package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;


/**
 * A displayclass for the duration type 
 */
public class Dsplduration extends DsplGeneric implements DsplSimple{

  /** Converts a duration given in nested list format into 
    * a string representing the same value. If the list don't
    * is a valid representation for a durationtype (day ms), null 
    * is returned. 
    **/
  private String getString(ListExpr value){
    // first, extract day and millisecond infro from value
    if(value.listLength()!=2)
        return null;
    ListExpr f = value.first();
    ListExpr s = value.second();
    if(f.atomType()!=ListExpr.INT_ATOM || s.atomType()!=ListExpr.INT_ATOM)
       return null;
    int d = f.intValue();
    int ms = s.intValue();
    // very simple output !
    if(d==0){ 
      return ms +" ms";  
    } else{
      return d+"days " + ms+" ms";
    }
  }

  
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    String v = getString(value);
    if(v==null)
       v = "invalid list rep";
    qr.addEntry(new String(type.symbolValue() + ":" + v));
    return;
  }

  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getString(value);
     if(V==null)
       V = "invalid list rep";
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     qr.addEntry(T + " : " + V);
     return;
  }


}



