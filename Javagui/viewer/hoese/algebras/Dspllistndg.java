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

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import viewer.hoese.algebras.jnet.*;


/**
 * A displayclass for lists of NetDistanceGroups ndg, alphanumeric only
 */
public class Dspllistndg extends DsplGeneric {

  public void init (String name, int nameWidth, int indent, ListExpr type,
                    ListExpr value, QueryResult qr)  {
     String T = name;
     T=extendString(T,nameWidth, indent);
     String Entry = T + ": ";
     while(!value.isEmpty()){
       NetDistanceGroup ndg = new NetDistanceGroup(value.first());
       value = value.rest();
       Entry = Entry + ndg.toString();
       if (!value.isEmpty())
         Entry = Entry + ", ";
     }
     qr.addEntry(Entry);
     return;
  }

}



