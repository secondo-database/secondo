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

package wrapper;

import sj.lang.ListExpr;
import java.io.Serializable;

public interface AlgebraType extends Serializable,Comparable{

   // additionally we need an constructor without any arguments
  /** returns a hashValue depending of the value of this object */
  int getHashValue();
  /** Reads the value of this from  instance */
  boolean loadFrom(ListExpr typeInfo,ListExpr instance);    
  /** Returns the value of this as a nested list */
  ListExpr toListExpr(ListExpr typeInfo);


  /** Checks the type of this. */
  // This method is only needed if composite types are
  // possible 
  //boolean checkType(ListExpr LE);
  
}
