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
 * A displayclass for the tuple-type. For each attribute in this tuple its displayclass is called
 */
public class Dspltuple extends DsplGeneric {

  /**
   * This method is used to analyse the type and value of this tuple type.
   * For each attribute the helper method displayTuple is called
   * neccessary for the displaying this type in the queryresultlist.
   * @param type datatype tuple with its attribute-types 
   * @param value A listexpr of the attribute-values
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dspltuplesrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    int maxAttribNameLen = maxAttributLength(type.second());
    while (!value.isEmpty()) {
      displayTuple(type.second(), value.first(), maxAttribNameLen, qr);
      value = value.rest();
      if (!value.isEmpty())
        qr.addEntry("---------");
    }
    return;
  }

  /**
   * A method which create an instance of each displayclass that appears as attribute-type, 
   * and calls its init method.
   * @see <a href="Dspltuplesrc.html#displayTuple">Source</a>
   */
  private void displayTuple (ListExpr type, ListExpr value, int maxNameLen, 
      QueryResult qr) {
    int i;
    String s;
    DsplBase dg;
    while (!value.isEmpty()) {
      s = type.first().first().symbolValue();
      dg = LEUtils.getClassFromName(type.first().second().symbolValue());
      if (dg instanceof DsplSimple){
         ((DsplSimple)dg).init(type.first().first(),maxNameLen, value.first(),0, qr);
      }
      else{
         dg.init(type.first().first(), value.first(), qr);
      }
      type = type.rest();
      value = value.rest();
    }
    return;
  }

  /**
   * Calculate the length of the longest attribute name.
   * @param type A ListExpr of the attribute types
   * @return maximal length of attributenames
   * @see <a href="Dspltuplesrc.html#maxAttributLength">Source</a>
   */
  private static final int maxAttributLength (ListExpr type) {
    int max, len;
    String s;
    max = 0;
    while (!type.isEmpty()) {
      s = type.first().first().symbolValue();
      len = s.length();
      if (len > max)
        max = len;
      type = type.rest();
    }
    return  max;
  }
}



