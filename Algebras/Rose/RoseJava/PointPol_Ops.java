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

class PointPol_Ops {

  //variables

  //constructors

  //methods
    /*
  public static boolean lies_on_border(Point poi, Polygon pol){
    //true if p lies on the border of pol, false else
    Segment s = new Segment();
    boolean w = false;
    for (int i = 0; i < pol.border.size(); i++) {
      s = (Segment)pol.border.get(i);
      if (PointSeg_Ops.lies_on(poi,s)){
	w = true;
	break;
      }//if
    }//for
    return w;
  }//end method lies_on_border
    */
    /*
  public static boolean inside(Point poi, Polygon pol) {
    //returns true if poi lies inside one of pol's triangles
    for (int i = 0; i < pol.trilist.size(); i++) {
      if (PointTri_Ops.inside(poi,(Triangle)pol.trilist.get(i))) {
	return true;
      }//if
    }//for i
    return false;
  }//end method inside
    */
}//end class PointPol_Ops
