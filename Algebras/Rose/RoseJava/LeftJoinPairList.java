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

class LeftJoinPairList extends ElemList {

    //members

    //constructors

    //methods
    public ElemList copy(){
	LeftJoinPairList copy = new LeftJoinPairList();
	for (int i = 0; i < this.size(); i++) {
	    copy.add(((LeftJoinPair)this.get(i)).copy());
	}//for
	return copy;
    }//end method copy

    public void print(){
	//prints out the list's elements
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("\nElement "+i+":");
	    ((LeftJoinPair)this.get(i)).element.print();
	    if (((LeftJoinPair)this.get(i)).elemList.isEmpty()) {
		System.out.println("list is empty"); }
	    else {
		for (int j = 0; j < ((LeftJoinPair)this.get(i)).elemList.size(); j++) {
		    System.out.println("Element "+j+" of list, size:"+((LeftJoinPair)this.get(i)).elemList.size());
		    ((Element)((LeftJoinPair)this.get(i)).elemList.get(j)).print();
		}//for j
	    }//else
	}//for i
    }//end method print

}//end class LeftJoinPairList
