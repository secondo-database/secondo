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


import de.fernunihagen.dna.hoese.DsplGeneric;
import de.fernunihagen.dna.hoese.QueryResult;
import  sj.lang.ListExpr;


/**
* A displayclass for the date-type, alphanumeric only
*/
public class Dspldate extends DsplGeneric {


public void init (String name, int nameWidth, int indent,
                  ListExpr type, ListExpr value, QueryResult qr)
{  
   String V="";
   if(isUndefined(value)){
      V = "undefined";
   } else{
      if(value.atomType()==ListExpr.STRING_ATOM){
         V = type.stringValue();
      }else{
         V = "<error>";
      }
   }
   String T = name;
   T=extendString(T,nameWidth, indent);
   qr.addEntry(T + " : " + V);
   return;

}


}



