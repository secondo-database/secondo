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

import java.util.*;

class TriList extends ElemList {
  
  //members

  //constructors

  //methods
  public ElemList copy(){
    TriList copy = new TriList();
    for (int i = 0; i < this.size(); i++) {
      copy.add(((Triangle)this.get(i)).copy());
    }//for
    return copy;
  }//end method copy

    public void print() {
	//prints out all elements
	for (int i = 0; i < this.size(); i++) {
	    ((Triangle)this.get(i)).print();
	}//for i
	if (this.size() == 0) {
	    System.out.println("TriList is empty.");
	}//if
	System.out.println();
    }//end method print


    public static TriList convert(ElemList el) {
	//converts an ElemList to a TriList
	TriList retList = new TriList();
	/*
	for (int i = 0; i < el.size(); i++) {
	    retList.add((Triangle)el.get(i));
	}//for i
	*/
	retList.addAll(el);
	return retList;
    }//end method convert

}//end class TriList
