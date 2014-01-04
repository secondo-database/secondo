
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
package de.fernunihagen.dna.hoese.algebras;

import de.fernunihagen.dna.hoese.DsplBase;
import de.fernunihagen.dna.hoese.DsplGeneric;
import de.fernunihagen.dna.hoese.LEUtils;
import de.fernunihagen.dna.hoese.QueryResult;
import  sj.lang.ListExpr;


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
public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
  int maxAttribNameLen = maxAttributLength(type.second());
  while (!value.isEmpty()) {
    displayTuple(type.second(), value.first(), maxAttribNameLen, indent, qr);
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
private void displayTuple (ListExpr type, ListExpr value, int maxNameLen, int indent, QueryResult qr) {
  int i;
  String s;
  DsplBase dg;
  while (!value.isEmpty()) {
    s = type.first().first().symbolValue();
    ListExpr subType = type.first().second();


    while(subType.atomType()!=ListExpr.SYMBOL_ATOM){
       subType = subType.first();
    }

    dg = LEUtils.getClassFromName(subType.symbolValue());
    // ensure to add exactly one entry per attribute
//    int oldnum = qr.getModel().getSize();
    String name = type.first().first().symbolValue();

    subType = type.first().second();
    dg.init(name, maxNameLen, indent, subType, value.first(), qr);
//    int newnum = qr.getModel().getSize();
    int diff = 1; // newnum-oldnum;
    if(diff<1){
       tools.Reporter.writeError("missing entry for attribute "+s);
       tools.Reporter.writeError("check the implementation of the class " + dg.getClass());
       qr.addEntry("error");
       
    }
    if(diff>1){
       tools.Reporter.writeError("to many entries for attribute "+s+
                           "\n please check the implementation of the "+dg.getClass() + " class");
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



