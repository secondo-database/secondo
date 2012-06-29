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

//2012, June Simone Jandt

package viewer.hoese.algebras.jnet;

import java.util.*;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;



/**
 * JRoute
 * List of section ids belonging to the route with id.
 */
public class JRoute{

   private Integer rid;
   private Vector<Integer> sids= new Vector<Integer>();


  public JRoute(ListExpr value){
    if (value.listLength() == 4){
      if(value.first().atomType()==ListExpr.INT_ATOM)
        rid = value.first().intValue();
      ListExpr sectList = value.third();
      while (!sectList.isEmpty()){
        sids.add(sectList.first().intValue());
        sectList = sectList.rest();
      }
    }
  }

  public Vector<Integer> getSids(){
    return sids;
  }

  public int getRid(){
    return rid;
  }


}



