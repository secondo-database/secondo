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

class SweepStElem {
    //supportive class for the sweep line status structure of the triangulation method in Polygons
    //this is also used for the sweep line status structure in Triangle.java

  //members
  boolean is_top;
  boolean is_bottom;
  PointList pointChain;
  boolean in; //true if attribute "in polygons" holds
    String col; //may be "red","blue", "both", ""
  Point rimoEl; //rightmost element

  //constructors
  protected SweepStElem() {
    boolean is_top = false;
    boolean is_bottom = false;
    pointChain = new PointList();
    in = false;
    col = "";
    rimoEl = new Point();
  }

    //methods
    protected void print() {
	//prints the elements attributes
	System.out.println("SweepStElem:");
	System.out.println("is_top:"+is_top+", is_bottom:"+is_bottom+", in:"+in+", col:"+col);
	System.out.println("pointChain:"+pointChain.size());
	pointChain.print();
	System.out.println("rimoEl:");
	rimoEl.print();
    }//end method print

}//end class SweepStElem
