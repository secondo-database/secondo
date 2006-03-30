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

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;


/**
 * A displayclass for the rel-type, the type argument need to be analysed further
 */
public class Dsplrel extends DsplGeneric {

  /**
   * This method is used to analyse the type and value of the relation in NestedList format. * up the intern datastructures for this type. An alphanumeric representation is 
   * neccessary for the displaying this type in the queryresultlist.
   * @param type A relation datatype rel with its type structure 
   * @param value the value of the relation structure
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplrelsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    long startTime=0;
    if(gui.Environment.MEASURE_TIME){
      startTime = System.currentTimeMillis();
    }
    long usedMemory=0;
    if(gui.Environment.MEASURE_MEMORY){
        usedMemory=gui.Environment.usedMemory();
    }
    LEUtils.analyse(type.second(), value, qr);
    if(gui.Environment.MEASURE_TIME){
       Reporter.writeInfo(" Building relation has taken :"+
                          (System.currentTimeMillis()-startTime)+" milliseconds");
    }
    if(gui.Environment.MEASURE_MEMORY){
      Reporter.writeInfo("Memory-Difference :"+ 
                          gui.Environment.formatMemory( gui.Environment.usedMemory()-usedMemory));
    }
    return;
  }
}



