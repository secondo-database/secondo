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

class PointList extends ElemList {

  //members

  //constructors

  //methods
  public ElemList copy(){
    PointList copy = new PointList();
    Iterator it = this.listIterator(0);
    while (it.hasNext()) {
	copy.add(((Point)it.next()).copy());
    }//while
    /*
    for (int i = 0; i < this.size(); i++) {
      copy.add(((Point)this.get(i)).copy());
    }//for
    */
    return copy;
  }//end method copy
    
    
    public void print () {
	//prints out all elements
	
	for (int i = 0; i < this.size(); i++) {
	    ((Point)this.get(i)).print();
	}//for i
	if (this.size() == 0) {
	    System.out.println("PointList is empty.");
	}//if
	System.out.println();
    }//end method print


    static public PointList convert(ElemList el) {
	//converts an ElemList to a PointList
	PointList retList = new PointList();
	/*
	for (int i = 0; i < el.size(); i++) {
	    retList.add((Point)el.get(i));
	}//for i
	*/
	retList.addAll(el);
	return retList;
    }//end method convert

    
    public int contains(Point p) {
	//returns the position of p if p element this
	//-1 otherwise
	for (int i = 0; i < this.size(); i ++) {
	    if (p.equal((Point)this.get(i))) {
		return i;
	    }//if
	}//for i
	return -1;
    }//end method contains


}//end class PointList
