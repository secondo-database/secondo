import java.util.*;

class SegTri_Ops {

    //members

    //constructors

    //methods
    
    public static SegList minus (Segment s, Triangle t) {
	//returns the list of segments resulting from
	//subtracting t from s
	//CAUTION: untested method

	//System.out.println("this is method stops.minus calling...");

	SegList retList = new SegList();
	SegList triSegs;// = t.segments();
	ListIterator lit;
	Segment actSeg;

	//case 1: no intersection
	if (SegTri_Ops.intersects(s,t) == false) {
	    retList.add(s);
	    //System.out.println("case1");
	    return retList;
	}//if
	
	//case 1.b: s lies fully inside of t
	if (inside(s,t)) {
	    //System.out.println("case1b");
	    return retList; }

	triSegs = t.segments();

	//case 2: s lies on border of t
	boolean overlap = false;
	for (int i = 0; i < 3; i++) {
	    if (SegSeg_Ops.overlap(s,(Segment)triSegs.get(i))) {
		try {
		    retList.add(SegSeg_Ops.theOverlap(s,(Segment)triSegs.get(i)));
		}//try
		catch (Exception e) {
		    //this is no problem here
		}//catch
		//System.out.println("case2");
		return retList;
	    }//if
	}//for i

	//case 3: there is a proper intersection
	if (pintersects(s,t)) {
	    //case 3.1: one point is inside and the other one outside of t
	    if ((PointTri_Ops.inside(s.startpoint,t) &&
		 !(PointTri_Ops.inside(s.endpoint,t))) ||
		(PointTri_Ops.inside(s.endpoint,t) &&
		 !(PointTri_Ops.inside(s.startpoint,t)))) {
		//find inside point
		Point ip;// = new Point();
		if (PointTri_Ops.inside(s.startpoint,t)) {
		    ip = s.startpoint; }
		else {
		    ip = s.endpoint; }
		//case 3.1.1: the other point lies on the border
		if (PointTri_Ops.liesOnBorder(s.theOtherOne(ip),t)) {
		    //System.out.println("case3.1.1");
		    return retList; }
		else {
		    //case 3.1.2: the other point lies outside of t
		    //find intersection point
		    Point intP = new Point();
		    for (int i = 0; i < 3; i ++) {
			if (s.pintersects((Segment)triSegs.get(i))) {
			    intP = s.intersection((Segment)triSegs.get(i));
			    break;
			}//if
		    }//for i
		    retList.add(new Segment(s.theOtherOne(ip),intP));
		    //System.out.println("case3.1.2");
		    return retList;
		}//else
	    }//if
	    
	    //System.out.println("ss inside t:"+PointTri_Ops.inside(s.startpoint,t));
	    //System.out.println("se inside t:"+PointTri_Ops.inside(s.endpoint,t));


	    //case 3.3: both of s's endpoints lie outside of t
	    //System.out.println("case3.3");
	    int numCuts = 0;
	    for (int i = 0; i < 3; i++) {
		if (s.pintersects((Segment)triSegs.get(i)) ||
		    PointSeg_Ops.liesOn(((Segment)triSegs.get(i)).startpoint,s) ||
		    PointSeg_Ops.liesOn(((Segment)triSegs.get(i)).endpoint,s)) { numCuts++; }
	    }//for i

	    if ((numCuts == 1) &&
		(PointTri_Ops.liesOnBorder(s.startpoint,t) ||
		 PointTri_Ops.liesOnBorder(s.endpoint,t))) {
		//one point lies on the border, the other lies outside
		//System.out.println("case 3.3.1");
		//find intersection point
		Point intP = new Point();
		lit = triSegs.listIterator(0);
		while (lit.hasNext()) {
		    actSeg = (Segment)lit.next();
		    if (s.pintersects(actSeg)) {
			intP = actSeg.intersection(s);
			break;
		    }//if
		}//while

		if (PointTri_Ops.liesOnBorder(s.startpoint,t)) {
		    retList.add(new Segment(s.endpoint,intP)); }
		else { retList.add(new Segment(s.startpoint,intP)); }
		return retList;
	    }//if
		    

	    if (numCuts == 2) {
		//find intersection points
		boolean found1 = false;
		Point intP1 = new Point();
		Point intP2 = intP1;// = new Point();
		for (int i = 0; i < 3; i++) {
		    if (s.pintersects((Segment)triSegs.get(i))) {
			if (!found1) {
			    intP1 = s.intersection((Segment)triSegs.get(i));
			    found1 = true;
			}//if
			else {
			    intP2 = s.intersection((Segment)triSegs.get(i));
			    break;
			}//else
		    }//if
		}//for i
		//System.out.println("intP1:"); intP1.print();
		//System.out.println("intP2:"); intP2.print();
		//build segments
		
		if (s.startpoint.dist(intP1).less(s.startpoint.dist(intP2))) {
		    retList.add(new Segment(s.startpoint,intP1));
		    retList.add(new Segment(s.endpoint,intP2));
		}//if
		else {
		    retList.add(new Segment(s.startpoint,intP2));
		    retList.add(new Segment(s.endpoint,intP1));
		}//if
		return retList;
	    }//if

	    if (numCuts == 3) {
		//case4
		boolean foundCut = false;
		boolean foundEnd = false;
		Point intP1 = new Point();
		Point intP2 = intP1;
		lit = triSegs.listIterator(0);
		while (lit.hasNext()) {
		    actSeg = (Segment)lit.next();
		    if (!foundCut && s.pintersects(actSeg)) {
			intP1 = s.intersection(actSeg);
		    foundCut = true;
		    }//if
		    if (!foundEnd) {
			if (PointSeg_Ops.liesOn(actSeg.startpoint,s)) {
			    intP2 = actSeg.startpoint;
			    foundEnd = true;
			}//if
			else if (PointSeg_Ops.liesOn(actSeg.endpoint,s)) {
			    intP2 = actSeg.endpoint; 
			    foundEnd = true;
			}//if
			if (foundCut && foundEnd) { break; }
		    }//if
		}//while
		
		//build segments
		if (s.startpoint.dist(intP1).less(s.startpoint.dist(intP2))) {
		    retList.add(new Segment(s.startpoint,intP1));
		    retList.add(new Segment(s.endpoint,intP2));
		}//if
		else {
		    retList.add(new Segment(s.startpoint,intP2));
		    retList.add(new Segment(s.endpoint,intP1));
		}//if
		//System.out.println("case4");
	    }//if
	    
	}//if
	return retList;
    }//end method minus
	    

    public static boolean pintersects (Segment s, Triangle t) {
	//returns true if the intersection is a line segment
	//this is NOT true if s lies on the border of t
	//CAUTION: untested method

	boolean ssInsideT = PointTri_Ops.inside(s.startpoint,t);
	boolean seInsideT = PointTri_Ops.inside(s.endpoint,t);

	if (ssInsideT || seInsideT) {
	    return true;
	}//if

	//boolean ssOnBorderT = PointTri_Ops.liesOnBorder(s.startpoint,t);
	//boolean seOnBorderT = PointTri_Ops.liesOnBorder(s.endpoint,t);

	if (overlapsBorder(s,t)) { return false; }

	SegList sl = t.segments();
	ListIterator lit = sl.listIterator(0);
	Segment actSeg;
	while (lit.hasNext()) {
	    actSeg = (Segment)lit.next();
	    if (s.pintersects(actSeg)) { return true; }
	    /*
	    if (s.intersects(actSeg) &&
		!SegSeg_Ops.formALine(s,actSeg) &&
		!PointSeg_Ops.liesOn(s.startpoint,actSeg) &&
		!PointSeg_Ops.liesOn(s.endpoint,actSeg)) {
		return true;
	    }//if
	    */	
	}//while
	//last case: s lies fully inside of t and both endpoints are on
	//t's borders
	int count = 0;
	lit = sl.listIterator(0);
	while (lit.hasNext()) {
	    actSeg = (Segment)lit.next();
	    if (PointSeg_Ops.liesOn(s.startpoint,actSeg) ||
		PointSeg_Ops.liesOn(s.endpoint,actSeg)) {
		count++;
	    }//if
	}//while
	
	if (count == 2) { return true; }
	else { return false; }
    }//end method pintersects


    public static Segment intersection (Segment s, Triangle t) {
	//returns the part of s which intersects t
	//CAUTION: untested method

	//System.out.println("this is method 'sto.intersection' calling...");
	//s.print();
	//t.print();
	//System.out.println();

	//case 1: no intersection
	if (!(intersects(s,t))) { 
	    System.out.println("SegTri_OPs.intersection: ERROR! No intersection.");
	    System.exit(0);
	    return new Segment();
	}//if

	//case 2a: s lies fully inside of t
	if (inside(s,t)) {
	    //System.out.println("case2");
	    return (Segment)s.copy();
	}//if

	//case 2b: s lies fully inside of t, both endpoints lie on t's border
	int noPointsOnBorder = 0;
	SegList triSegs = t.segments();
	for (int i = 0; i < 3; i ++) {
	    if (PointSeg_Ops.liesOn(s.startpoint,(Segment)triSegs.get(i))) {
		noPointsOnBorder++;
	    }//if
	    if (PointSeg_Ops.liesOn(s.endpoint,(Segment)triSegs.get(i))) {
		noPointsOnBorder++;
	    }//if
	}//for i
	if (noPointsOnBorder == 2) {
	    return (Segment)s.copy();
	}//if

	//case 3/4: there are intersection points
	PointList intPoints = new PointList();

	for (int i = 0; i < triSegs.size(); i++) {
	    if (s.intersects((Segment)triSegs.get(i))) {
		intPoints.add(s.intersection((Segment)triSegs.get(i)));		
	    }//if
	}//for i
	//it may happen that the segment cuts through the edges of
	//the triangle. so remove duplicates!
	if (intPoints.size() >= 2) {
	    intPoints = (PointList)SetOps.rdup(intPoints); }
	//System.out.println("noIntersectionPoints:"+intPoints.size()); intPoints.print();

	//remove intersection points from list, that lie
	//on the triangle's border
	//for (int i = 0; i < intPoints.size(); i++) {
	//  if (PointTri_Ops.liesOnBorder((Point)intPoints.get(i),t)) {
	//intPoints.remove(i);
	//i--;
	//  }//if
	//}//for

	//System.out.println("noIntersectionPoints(after deletion):"+intPoints.size());

	//case 3: one intersection point
	if (intPoints.size() == 1) {
	    //System.out.println("case3");
	    if (PointTri_Ops.inside(s.startpoint,t)) {
		return(new Segment(s.startpoint,(Point)intPoints.getFirst()));
	    }//if
	    else {
		return(new Segment(s.endpoint,(Point)intPoints.getFirst()));
	    }//else
	}//if

	//case 4: two intersection points
	if (intPoints.size() == 2) {
	    //System.out.println("case4");
	    return (new Segment((Point)intPoints.getFirst(),(Point)intPoints.getLast()));
	}//if

	System.out.println("ERROR(SegTri_Ops.intersection):undefined intersection");
	System.exit(0);
	return new Segment();
    }//end method intersection
	

    public static boolean inside (Segment s, Triangle t) {
	//returns true if s lies fully inside of t
	//CAUTION: untested method
	
	boolean retVal = false;
	
	if ((PointTri_Ops.inside(s.startpoint,t) ||
	     PointTri_Ops.liesOnBorder(s.startpoint,t)) &&
	    (PointTri_Ops.inside(s.endpoint,t) ||
	     PointTri_Ops.liesOnBorder(s.endpoint,t))) {
	    retVal = true;
	}//if
	else {
	    retVal = false;
	}//else
	return retVal;
    }//end method inside
	

    public static boolean intersects (Segment s, Triangle t) {
	//returns true if s and t intersect
	//System.out.println();
	//System.out.println("This is method 'SegTri_OPs.intersects' calling...");
	//t.print();
	//s.print();
	if (PointTri_Ops.inside(s.startpoint,t) ||
	    PointTri_Ops.inside(s.endpoint,t)) {
	    //System.out.println("sto.intersects:true(case1)");
	    return true; }//if
	else {
	    for (int i = 0; i < 3; i++) {
		if (s.intersects((Segment)t.segments().get(i))) {
		    //System.out.println("sto.intersects:true(case2)");
		    return true; }//if
	    }//for i
	}//else
	//System.out.println("sto.intersects:false");
	return false;
    }//end method intersects

    /*
      protected static boolean fakeIntersects (Segment s, Triangle t) {
      //returns true if s intersects any
      //segment of t
      //false else
      SegList tsegs = t.segments();
      
      for (int i = 0; i < 3; i++) {
      if (s.intersects(tsegs.get(i))) {
      return true;
      }//if
      }//for i
      return false;
      }//end method fakeIntersects
    */

    public static Rational dist (Segment s, Triangle t) {
	//returns the distance between s and t
	if (intersects(s,t)) { return (new Rational(0)); }
	else {
	    LinkedList distlist = new LinkedList();
	    
	    for (int i = 0; i < 3; i++) {
		distlist.add(s.dist((Segment)t.segments().get(i)));
	    }//for i
	    Rational min = (Rational)distlist.getFirst();
	    for (int i = 1; i < 3; i++) {
		if (((Rational)distlist.get(i)).less(min)) {
		    min = (Rational)distlist.get(i); }//if
	    }//for i
	    return min.copy();
	}//else
    }//end method dist


    protected static boolean overlapsBorder (Segment seg, Triangle tri) {
	//returns true if seg overlaps one of tri's segments
	SegList segs = tri.segments();
	for (int i = 0; i < segs.size(); i++) {
	    if (SegSeg_Ops.overlap(seg,(Segment)segs.get(i))) {
		return true;
	    }//if
	}//for i
	return false;
    }//end method overlapsBorder

	    
}//end class SegTri_Ops
