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
 * The displayclass of Rose algebras coordinate-type .
 */
public class Dsplcoordinate extends DsplGeneric {
  double koord;

  /**
   * Scans the numeric representation of a coordinate
   * @param v the numeric value
   * @return true if no errors occur
   * @see sj.lang.ListExpr
   * @see <a href="Dsplcoordinatesrc.html#ScanValue">Source</a>
   */
  private boolean ScanValue (ListExpr v) {
    //System.out.println(v.writeListExprToString());
    if (v.isAtom()) {
      if (v.atomType() != ListExpr.INT_ATOM) {
        System.out.println("Error: No correct coordinate expression: rat or int-type needed");
        return  false;
      }
      koord = (double)v.intValue();
      return  true;
    } 
    else {
      if (v.listLength() != 5) {
        System.out.println("Error: No correct rat expression: 5 elements needed");
        return  false;
      }
      if ((v.first().atomType() != ListExpr.SYMBOL_ATOM) || (v.second().atomType()
          != ListExpr.INT_ATOM) || (v.third().atomType() != ListExpr.INT_ATOM)
          || (v.fourth().atomType() != ListExpr.SYMBOL_ATOM) || (v.fifth().atomType()
          != ListExpr.INT_ATOM)) {
        System.out.println("Error: No correct rat expression: wrong types");
        return  false;
      }
      if ((!v.first().symbolValue().equals("rat")) || (!v.fourth().symbolValue().equals("/"))) {
        System.out.println("Error: No correct rat expression: wrong symbols");
        return  false;
      }
      koord = (double)v.second().intValue() + (double)v.third().intValue()/(double)v.fifth().intValue();
    }
    return  true;
  }

  /**
   * Init. the Dsplcoordinate instance.
   * @param type The symbol coordinate
   * @param value The numeric value of a coordinate.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplcoordinatesrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    if (!ScanValue(value))
      qr.addEntry("(" + type.symbolValue() + ":" + koord + ")"); 
    else 
      qr.addEntry(type.symbolValue() + ":" + koord);
    return;
  }
}



