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
import java.io.*;

class PairList extends LinkedList implements Serializable {
  
  //members

  //constructors

  //methods
    public PairList copy() {
	PairList copy = new PairList();
	for (int i = 0; i < this.size(); i++) {
	    copy.add(((ElemPair)this.get(i)).copy());
	}//for
	return copy;
    }//end method copy

    public void print() {
	if (this.isEmpty()) { System.out.println("list is empty"); }
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("Element "+i);
	    if (((ElemPair)this.get(i)).first != null) ((ElemPair)this.get(i)).first.print();
	    else System.out.println("element is NULL");
	    if (((ElemPair)this.get(i)).second != null)((ElemPair)this.get(i)).second.print();
	    else System.out.println("element is NULL");
	}//for i
    }//end method print


    protected void twistElements() throws WrongTypeException {
	//checks every ElemPair whether the first or
	//second Element is smaller regarding compX
	//CAUTION: works only if both Elements are of the same type
	byte result;
	for (int i = 0; i < this.size(); i++) {
	    Element el1 = ((ElemPair)this.get(i)).first;
	    Element el2 = ((ElemPair)this.get(i)).second;
	    result = el1.compX(el2);
	    if (result == 0) {
		result = el1.compY(el2);
	    }//if
	    switch (result) {
	    case -1 : break;
	    case 0 : break;
	    case 1 : this.set(i,new ElemPair(el2,el1));
	    }//switch
	}//for i
    }//end method twistElements

}//end class PairList
