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
import java.lang.reflect.*;

class TriTri_Ops {

  //variables

  //constructors

  //methods
    public static boolean inside(Triangle t1, Triangle t2) {
	//return true if t1 inside t2
	//System.out.println("\nentering TTO_inside...");
	PointList t1verts = t1.vertexlist();
	for (int i = 0; i < t1verts.size(); i++) {
	    if (!(PointTri_Ops.inside((Point)t1verts.get(i),t2) ||
		  PointTri_Ops.liesOnBorder((Point)t1verts.get(i),t2))) {
		//System.out.println("TTO.i: return false");
		return false;
	    }//if
	}//for i
	//System.out.println("TTO.i: return true");
	return true;
    }//end method inside


    public static boolean adjacent(Triangle t1, Triangle t2) {
	//returns true if t1,t2 are area_disjoint but have common boundaries
	boolean t1IntersectsT2 = false;
	
	t1IntersectsT2 = t1.intersects(t2);
	if (t1IntersectsT2) { return false; }
	else {
	    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		    if (SegSeg_Ops.overlap((Segment)t1.segments().get(i),(Segment)t2.segments().get(j))) {
			return true;
		    }//if
		}//for j
	    }//for i

	    for (int i = 0; i < 3;i++) {
		for (int j = 0; j < 3; j++) {
		    if (PointSeg_Ops.liesOn(t1.vertices[i],(Segment)t2.segments().get(j))) {
			return true;
		    }//if

		    if (PointSeg_Ops.liesOn(t2.vertices[i],(Segment)t1.segments().get(j))) {
			return true;
		    }//if
		}//for j
	    }//for i
	}//else
	return false;
    }//end method adjacent

}//end class TriTri_Ops
