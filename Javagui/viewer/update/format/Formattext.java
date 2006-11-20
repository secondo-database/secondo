/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

last update 2006-01-23

*/
package viewer.update.format;

import sj.lang.ListExpr;


public class Formattext implements viewer.update.LEFormatter{

   public String ListExprToString(ListExpr LE){
        return LE.textValue();
   }

   public ListExpr StringToListExpr(String value){
        if(value==null) {
           return null;
        }
        boolean v1 = value.indexOf("'")>0;
        boolean v2 = value.indexOf("</text--->")>0;

        if(v1 && v2){ // cannot be represented
           return null;
        }
        return ListExpr.textAtom(value);
        
   }
} 
