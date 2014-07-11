
//This file is part of SECONDO.

//Copyright (C) 2013, University in Hagen, Department of Computer Science, 
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

package viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import tools.Reporter;

/**
 * This class displays place objects.
 */
public class Dsplplace extends DsplGeneric {

  public void init(String name, int nameWidth, int indent, ListExpr type,
                   ListExpr value, QueryResult qr) {
    String T = name;
    T = extendString(T, nameWidth, indent);
    if (isUndefined(value)) {
      qr.addEntry(T + " : undefined");
    }
    else {
      qr.addEntry(T + " : " + SymbolicValues.placeToString(value));
    }
  }
}
