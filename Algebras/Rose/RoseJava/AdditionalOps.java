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

import java.lang.reflect.*;

public class AdditionalOps {
    //In this class additional, user-defined operations can be found.
    //Some examples for such operations are given here.

    public static boolean isConnected (SegList sl) {
	//returns true if the segment list sl is connected
	Class c = (new SegSeg_Ops()).getClass();
	Class[] paramList = new Class[2];
	ElemListList ell = new ElemListList();
	paramList[0] = (new Segment()).getClass();
	paramList[1] = (new Segment()).getClass();
	try {
	    Method m = c.getMethod("formALine",paramList);
	    ell = SetOps.group(sl,m);
	}//try
	catch (Exception e) {
	    System.out.println("Error in AdditionalOps.isCoherent(SegList).");
	    System.exit(0);
	}//catch
	
	if (ell.size() == 1) { return true; }
	else { return false; }
    }//end method isConnected

    
    public static boolean isCoherent (TriList tl) {
	//returns tur if the triangle list tl is coherent
	Class c = (new TriTri_Ops()).getClass();
	Class[] paramList = new Class[2];
	ElemListList ell = new ElemListList();
	paramList[0] = (new Triangle()).getClass();
	paramList[1] = (new Triangle()).getClass();
	try {
	    Method m = c.getMethod("adjacent",paramList);
	    ell = SetOps.group(tl,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	if (ell.size() == 1) { return true; }
	else { return false; }
    }//end method isCoherent
	
    
    public static PointList findAccessiblePoints(PointList plIn, SegList slIn) {
	//checks first if slIn is coherent
	//returns the subset of plIn which is accessible through slIn
	//accessible means that a point of plIn is an endpoint of a segment in slIn
	if (!isConnected(slIn)) {
	    System.out.println("Error in AdditionalOps.findAccessiblePoints: Segment list is not connected.");
	    System.exit(0);
	    return (new PointList());
	}//if
	else {
	    PointList segPoints = new PointList();
	    Class c = (new Segment()).getClass();
	    try {
		Method m = c.getMethod("endpoints",null);
		segPoints = PointList.convert(SetOps.map(slIn,m));
		//System.out.println("segPoints.size(): "+segPoints.size());
		//segPoints.print();
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
		 //System.out.println("airp:");
		 //plIn.print();
	    return PointList.convert(SetOps.intersection(plIn,segPoints));
	}//else
    }//end method findAccessiblePoints


    public static SegList flightsOutOfBorder (TriList country, PointList airports) {
	//returns the subset of all direct flights from any possible flights from
	//an airport to another which cross the country's borders

	//compute the set of all possible flights
	SegList flights = new SegList();
	Class c = (new AdditionalOps()).getClass();
	Class[] paramList1 = new Class[2];
	Class[] paramList2 = new Class[2];
	paramList2[0] = (new Point()).getClass();
	paramList2[1] = (new Point()).getClass();
	try {
	    paramList1[0] = Class.forName("Element");
	    paramList1[1] = Class.forName("Element");
	    Method m1 = c.getMethod("unequal",paramList1);
	    Method m2 = c.getMethod("constructSegment",paramList2);
	    flights = SegList.convert(SetOps.map(SetOps.join(airports,airports,m1),m2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	//find flights crossing the border
	c = (new Segment()).getClass();
	Class[] paramList3 = new Class[1];
	SegList retList = new SegList();
	try {
	    paramList3[0] = Class.forName("Element");
	    Method m3 = c.getMethod("intersects",paramList3);
	    retList = SegList.convert(SetOps.rdup(SetOps.proj1(SetOps.join(flights,Algebra.contour(country,true,true),m3))));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return retList;
    }//end method flightsOutOfBorder


    public static boolean unequal (Element el1, Element el2) {
	//returns true
	return (!el1.equal(el2));
    }//end method isTrue

    
    public static Segment constructSegment (Point point1, Point point2) {
	//returns new Segment build from point1,point2
	return (new Segment(point1,point2));
    }//end method constructSegment


}//end class AdditionalOps
