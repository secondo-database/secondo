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

import java.io.*;

class ElemPair implements Serializable {
  //members
  public Element first;
  public Element second;

  //constructors
  ElemPair(){
      first = null;
      second = null;
  };

  ElemPair(Element e1, Element e2) {
    first = (Element)e1.copy();
    second = (Element)e2.copy();
  }
  
  //methods
  public ElemPair copy(){
    //returns a copy of ElemPair
    ElemPair copy = new ElemPair();
    copy.first = (Element)first.copy();
    copy.second = (Element)second.copy();
    return copy;
  }//end method copy

    public boolean equal(ElemPair inEl) {
	//returns true if both elements are equal
	if (this.first.getClass() != inEl.first.getClass() ||
	    this.second.getClass() != inEl.second.getClass()) {
	    return false;
	}//if
	else {
	    if (this.first.equal(inEl.first) &&
		this.second.equal(inEl.second)) {
		return true;
	    }//if
	}//else
	return false;
    }//end method equal

    public boolean equalOrInvertedEqual (ElemPair inEl) {
	//returns true if both elements are equal or if
	//equal, when first/second are inverted
	if (this.equal(inEl)) {
	    return true; }
	else {
	    if (this.first.getClass() != inEl.second.getClass() ||
		this.second.getClass() != inEl.first.getClass()) {
		return false; 
	    }//if
	    else {
		if (this.first.equal(inEl.second) &&
		    this.second.equal(inEl.first)) {
		    return true;
		}//if
	    }//else
	}//else
	return false;
    }//end method equalOrInvertedEqual

    
    public void print () {
	System.out.println("first:");
	first.print();
	System.out.println("second:");
	second.print();
    }//end method print    


    public byte compX (ElemPair inPair) throws WrongTypeException {
	//uses the compX method of Element
	byte res = this.first.compX(inPair.first);
	if (res == 0) {
	    res = this.second.compX(inPair.second); }
	return res;
    }//end method compX

    public byte compY (ElemPair inPair) throws WrongTypeException {
	//uses the compY method of Element
	byte res = this.first.compY(inPair.first);
	if (res == 0) {
	    res = this.second.compY(inPair.second); }
	return res;
    }//end method compY

    public byte compare (ElemPair inPair) throws WrongTypeException {
	//...
	byte res = this.first.compare(inPair.first);
	if (res == 0) res = this.second.compare(inPair.second);
	return res;
    }//end method compare

    public void twist () {
	//twists first and second of this
	Element swap = this.first;
	this.first = this.second;
	this.second = swap;
    }//end method twist

}//end class ElemPair
