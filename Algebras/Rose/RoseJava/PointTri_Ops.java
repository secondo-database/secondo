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

public class PointTri_Ops {
  
  //members

  //variables
    
  //methods
    public static boolean inside (Point p, Triangle t) {
	//returns true, if p lies inside of t
	//to do this shoot a line from p to the outside and count the cuts
	//shoot two lines, because there is the possibility that the line
	//cuts through a corner and the intersection is not found properly

	//System.out.println();
	//System.out.println("\nentering PTO.inside...");
	//p.print();
	
	
	//does p lie on the border of t?
	if (liesOnBorder(p,t)) {
	    //System.out.print("ptops.inside:1...");
	    return false;
	}//if
	
	//find the highest x-value of t
	//and find the lowest x-value of t
	
	Rational hxv = RationalFactory.constRational(t.rect().lr.x);
	Rational lxv = RationalFactory.constRational(t.rect().ll.x);
	
	//build cutting segment
	Rational help = RationalFactory.constRational(1);
	if ((hxv.plus(help)).equal(p.x)) { help = help.plus(1); }
	Segment cut = new Segment(p,new Point(hxv.plus(help),p.y));
	SegList sl = t.segments();
	help = RationalFactory.constRational(1);
	if ((lxv.minus(help)).equal(p.x)) { help = help.plus(1); }
	Segment cut2 = new Segment(p,new Point(lxv.minus(help),p.y));
	
	//do 'cut' and one of the triangle's segments overlap?
	if (SegSeg_Ops.overlap((Segment)sl.get(0),cut) ||
	    SegSeg_Ops.overlap((Segment)sl.get(1),cut) ||
	    SegSeg_Ops.overlap((Segment)sl.get(2),cut)) {
	    //System.out.println("ptops.inside:2...");
	    return false;
	}//if
	
	int noOfCuts = 0;
	int noOfCuts2 = 0;
	for (int i = 0; i < 3; i++) {
	    Segment actSeg = (Segment)sl.get(i);
	    if (cut.pintersects(actSeg)) { noOfCuts++; }
	    if (cut2.pintersects(actSeg)) { noOfCuts2++; }
	    //if cut/cut2 go through endpoints this must be added, too
	    if (PointSeg_Ops.liesOn(actSeg.startpoint,cut) ||
		PointSeg_Ops.liesOn(actSeg.endpoint,cut)) { noOfCuts++; }
	    if (PointSeg_Ops.liesOn(actSeg.startpoint,cut2) ||
		PointSeg_Ops.liesOn(actSeg.endpoint,cut2)) { noOfCuts2++; }
	}//for i
	
	
	//System.out.print("cut: "); cut.print();
	//System.out.print("cut2: "); cut2.print();
	//System.out.print("triangle: "); t.print();
	//System.out.print("point: "); p.print();
	//System.out.println("...noOfCuts: "+noOfCuts);
	//System.out.println("...noOfCuts2: "+noOfCuts2);
	
	if ((noOfCuts == 1) || (noOfCuts2 == 1)) {
	    if (((p.compX(new Point(t.rect().ll.x,t.rect().ll.y)) <= 0) &&
		 (cut.endpoint.compX(new Point(t.rect().lr.x,t.rect().lr.y)) >=0)) ||
		((p.compX(new Point(t.rect().lr.x,t.rect().lr.y)) >= 0) &&
		 (cut2.startpoint.compX(new Point(t.rect().ll.x,t.rect().ll.y)) <= 0))) {
		//System.out.println("ptops.inside: return false");
		return false;
	    }//if
	    else {
		//System.out.println("ptops.inside: return true!");
		return true;
	    }//else
	}//if
	else {
	    //System.out.println("ptops.inside: return false!");
	    return false; }
    }//end method inside

  
  public static boolean liesOnBorder (Point p, Triangle t) {
    //returns true if p lies on one of t's segments or is equal to
    //one of t's vertices
      //System.out.println("entering PTO.liesOnBorder...");

      if (t.vertices[0].equal(p) ||
	  t.vertices[1].equal(p) ||
	  t.vertices[2].equal(p)) {
	  //System.out.println("PTO.lOB: case1");
	  return true;
      }//if
      
    SegList sl = t.segments();
    if (PointSeg_Ops.liesOn(p,(Segment)sl.get(0)) ||
	PointSeg_Ops.liesOn(p,(Segment)sl.get(1)) ||
	PointSeg_Ops.liesOn(p,(Segment)sl.get(2))) {
	//System.out.println("PTO.lOB: case2");
      return true;
    }//if
    
    //System.out.println("PTO.lOB: case 3");
    return false;
  }//end method liesOnBorder

    public static boolean isVertex (Point p, Triangle t) {
	//returns true if p is vertice of t
	//false else
   
	if (t.vertices[0].equal(p) ||
	    t.vertices[1].equal(p) ||
	    t.vertices[2].equal(p)) {
	    return true; }
	else { return false; }
    }//end method is_vertice

    public static Rational dist(Point p, Triangle t) {
	//returns the distance between p and t
	LinkedList distList = new LinkedList();
	
	if (inside(p,t)) return (RationalFactory.constRational(0));
	for (int i = 0; i < 3; i++) { distList.add(PointSeg_Ops.dist(p,(Segment)t.segments().get(i))); }
	Rational min = (Rational)distList.getFirst();
	for (int i = 1; i < 3; i++) {
	    if (((Rational)distList.get(i)).less(min)) {
		min = (Rational)distList.get(i); }//if
	}//for i
	return min.copy();
    }//end method dist

}//end class PointTri_Ops
