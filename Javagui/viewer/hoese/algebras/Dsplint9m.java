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
 * A displayclass for the int9m type 
 */
public class Dsplint9m extends DsplGeneric implements DsplSimple{
  static final String ERROR="wrong list";

  /* returns the string representation for the given value list */
  private String getDisplay(ListExpr value){
     String display = "";
     int at = value.atomType();
     switch(at){
       case ListExpr.INT_ATOM:
          int v = value.intValue();
          int c = 256; 
          for(int i=0;i<9;i++){
             display += (v&c)>0?"1":"0";
             c = c>>1;
          }
          break;
       case ListExpr.NO_ATOM:
           int len = value.listLength();
           if(len!=9){
              display=ERROR;
           }else{
             boolean done=false;
             while(!value.isEmpty() && !done){
               ListExpr f = value.first();
               value = value.rest();
               int atf = f.atomType();
               switch(atf){
                 case ListExpr.INT_ATOM:
                    int i = f.intValue();
                    display += i==0?"0":"1";
                    break;
                case ListExpr.BOOL_ATOM:
                    display += f.boolValue()?"1":"0";
                    break;
                default:
                     display = ERROR;
                     done = true;
               }
             }
           }
           break;
       default: display = ERROR;
     }
     return display;

  }


  /* Sets the entry for the queryresult.
  */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
     qr.addEntry(new String(type.symbolValue() + ":" + getDisplay(value)));
  }

  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getDisplay(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     qr.addEntry(T + " : " + V);
     return;

  }


}



