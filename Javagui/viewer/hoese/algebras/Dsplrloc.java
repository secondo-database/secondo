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
 * A displayclass for the string-type, alphanumeric only
 */
public class Dsplrloc extends DsplGeneric implements LabelAttribute{

   String label;
   RouteLocation rloc;

  public void init (String name, int nameWidth, int indent, ListExpr type,
                    ListExpr value, QueryResult qr)  {
     String T = name;
     rloc = new RouteLocation(value);
     String V = rloc.toString();
     label = V;
     T=extendString(T,nameWidth, indent);
     qr.addEntry(T + " : " + V);
     return;
  }

  public String getLabel(double time){
     return label;
  }


}



