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
 * A displayclass for the string-type, alphanumeric only
 */
public class Dspltext extends DsplGeneric implements DsplSimple{

  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is
   * neccessary for the displaying this type in the queryresultlist.
   * @param type A ListExpr of the datatype string
   * @param value A string in a listexpr
   * @param qr The queryresultlist to add alphanumeric representation
   * @see QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplstringsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
     if (value.listLength()==1)
     value = value.first();
     if(value.atomType()!=ListExpr.TEXT_ATOM)
        qr.addEntry(new String(type.symbolValue()) + ": error in value ");
     else
        qr.addEntry(new String(type.symbolValue() + ":" + value.textValue()));
     return;
  }

  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V;

     if (value.listLength()==1)
     value = value.first();
     if(value.atomType()!=ListExpr.TEXT_ATOM)
        V =  "error in value ";
     else
        V =  value.textValue();
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     qr.addEntry(T + " : " + V);
     return;

  }



}



