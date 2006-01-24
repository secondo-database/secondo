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

last change 2006-01-23


*/

package viewer.update;

import sj.lang.*;

/**
This is the formatter for all unknow types. It just converts 
a nested list into its string representation and vice versa.

**/

public class StandardFormatter implements LEFormatter{

public String ListExprToString(ListExpr LE){
    return LE.writeListExprToString();
}

public ListExpr StringToListExpr(String value){
    ListExpr result = new ListExpr();
    if(result.readFromString(value)!=0)
        return null;
    else
        return result;

}


}

