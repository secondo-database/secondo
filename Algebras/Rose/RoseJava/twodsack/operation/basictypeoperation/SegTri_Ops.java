/*
 * SegTri_Ops.java 2004-11-10
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.basictypeoperation;

import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Vector;

/**
 * In <code>SegTri_Ops</code> a set of static methods can be found which all have as parameter types
 * one instance of {@link Segment} and one instance of {@link Triangle}. Therefore, the 
 * class name is <code>SegTri_Ops</code>.
 */
public class SegTri_Ops {

    /*
     * fields
     */
    static final PointComparator POINT_COMPARATOR = new PointComparator();
    static final SegmentComparator SEGMENT_COMPARATOR = new SegmentComparator();


    /**
     * constructors
     */
    /**
     * The standard constructor.
     */
    public SegTri_Ops(){}


    /*
     * methods
     */
   
    /**
     * Returns the list of segments resulting from subtracting a triangle from a segment.<p>
     * For a pair of overlapping objects, one segment <code>s</code> and one triangle <code>t</code>
     * this method computes that part of the segment that is covered by the triangle and 
     * removes that part from the segment. The result may be empty (i.e. the segment fully lies
     * inside of the triangle), may be unchanged (i.e. both objects don't intersect), may consist
     * of one segment (i.e. one half of the segments is covered by the triangle) or may consist of
     * two triangles (i.e. only the middle part of the segment is covered by an edge of the triangle.
     *
     * @param s the segment
     * @param t the triangle
     * @return the (possibly empty) set of segments as <code>SegMultiSet</code>
     */
    public static SegMultiSet minus (Segment s, Triangle t) {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	Iterator lit;
	Segment actSeg;

	//case 1: no intersection
	if (SegTri_Ops.intersects(s,t) == false) {
	    retList.add(s);
	    return retList;
	}//if
	
	//case 1.b: s lies fully inside of t
	if (inside(s,t)) {
	    return retList; }

	Segment[] tArr = t.segmentArray();

	//case 2: s lies on border of t
	boolean overlap = false;
	for (int i = 0; i < 3; i++) {
	    if (SegSeg_Ops.overlap(s,tArr[i])) {
		try {
		    retList.add(SegSeg_Ops.theOverlap(s,tArr[i]));
		}//try
		catch (Exception e) {
		    //this is no problem here
		}//catch
		return retList;
	    }//if
	}//for i

	//case 3: there is a proper intersection
	if (pintersects(s,t)) {
	    //case 3.1: one point is inside and the other one outside of t
	    if ((PointTri_Ops.inside(s.getStartpoint(),t) &&
		 !(PointTri_Ops.inside(s.getEndpoint(),t))) ||
		(PointTri_Ops.inside(s.getEndpoint(),t) &&
		 !(PointTri_Ops.inside(s.getStartpoint(),t)))) {
		//find inside point
		Point ip;
		if (PointTri_Ops.inside(s.getStartpoint(),t)) {
		    ip = s.getStartpoint(); }
		else {
		    ip = s.getEndpoint(); }
		//case 3.1.1: the other point lies on the border
		if (PointTri_Ops.liesOnBorder(s.theOtherOne(ip),t)) {
		    return retList; }
		else {
		    //case 3.1.2: the other point lies outside of t
		    //find intersection point
		    Point intP = new Point();
		    for (int i = 0; i < 3; i ++) {
			if (s.pintersects(tArr[i])) {
			    intP = s.intersection(tArr[i]);
			    break;
			}//if
		    }//for i
		    retList.add(new Segment(s.theOtherOne(ip),intP));
		    return retList;
		}//else
	    }//if
	    
	    //case 3.3: both of s's endpoints lie outside of t
	    int numCuts = 0;
	    for (int i = 0; i < 3; i++) {
		if (s.pintersects(tArr[i]) ||
		    PointSeg_Ops.liesOn(tArr[i].getStartpoint(),s) ||
		    PointSeg_Ops.liesOn(tArr[i].getEndpoint(),s)) { numCuts++; }
	    }//for i

	    if ((numCuts == 1) &&
		(PointTri_Ops.liesOnBorder(s.getStartpoint(),t) ||
		 PointTri_Ops.liesOnBorder(s.getEndpoint(),t))) {
		//one point lies on the border, the other lies outside
	
		//find intersection point
		Point intP = new Point();
		for (int i = 0; i < tArr.length; i++) {
		    if (s.pintersects(tArr[i])) {
			intP = tArr[i].intersection(s);
			break;
		    }//if
		}//while
		
		if (PointTri_Ops.liesOnBorder(s.getStartpoint(),t)) {
		    retList.add(new Segment(s.getEndpoint(),intP)); }
		else { retList.add(new Segment(s.getStartpoint(),intP)); }
		return retList;
	    }//if
		    

	    if (numCuts == 2) {
		//find intersection points
		boolean found1 = false;
		Point intP1 = new Point();
		Point intP2 = intP1;
		for (int i = 0; i < 3; i++) {
		    if (s.pintersects(tArr[i])) {
			if (!found1) {
			    intP1 = s.intersection(tArr[i]);
			    found1 = true;
			}//if
			else {
			    intP2 = s.intersection(tArr[i]);
			    break;
			}//else
		    }//if
		}//for i
		
		//build segments		
		if (s.getStartpoint().dist(intP1).less(s.getStartpoint().dist(intP2))) {
		    retList.add(new Segment(s.getStartpoint(),intP1));
		    retList.add(new Segment(s.getEndpoint(),intP2));
		}//if
		else {
		    retList.add(new Segment(s.getStartpoint(),intP2));
		    retList.add(new Segment(s.getEndpoint(),intP1));
		}//if
		return retList;
	    }//if

	    if (numCuts == 3) {
		//case4
		boolean foundCut = false;
		boolean foundEnd = false;
		Point intP1 = new Point();
		Point intP2 = intP1;
		for (int i = 0; i < tArr.length; i++) {
		    actSeg = tArr[i];
		    if (!foundCut && s.pintersects(actSeg)) {
			intP1 = s.intersection(actSeg);
		    foundCut = true;
		    }//if
		    if (!foundEnd) {
			if (PointSeg_Ops.liesOn(actSeg.getStartpoint(),s)) {
			    intP2 = actSeg.getStartpoint();
			    foundEnd = true;
			}//if
			else if (PointSeg_Ops.liesOn(actSeg.getEndpoint(),s)) {
			    intP2 = actSeg.getEndpoint(); 
			    foundEnd = true;
			}//if
			if (foundCut && foundEnd) { break; }
		    }//if
		}//while
		
		//build segments
		if (s.getStartpoint().dist(intP1).less(s.getStartpoint().dist(intP2))) {
		    retList.add(new Segment(s.getStartpoint(),intP1));
		    retList.add(new Segment(s.getEndpoint(),intP2));
		}//if
		else {
		    retList.add(new Segment(s.getStartpoint(),intP2));
		    retList.add(new Segment(s.getEndpoint(),intP1));
		}//if
		}//if
	    
	}//if
	return retList;
    }//end method minus
	    

    /**
     * Returns <tt>true</tt> if the intersection of the segment and the triangle again is a segment.<p>
     * This method returns <code>false</code> if the segment overlaps the border of the triangle.
     *
     * @param s the segment
     * @param t the triangle
     * @return {<code>true</code>, <code>false</code>} depending on the mutual position of both objects
     * @see #intersects(Segment,Triangle)
     * @see #isCovered(Segment,Triangle)
     */
    public static boolean pintersects (Segment s, Triangle t) {	
	//does any of the segment's endpoints lie inside of triangle?
	boolean ssInsideT = PointTri_Ops.inside(s.getStartpoint(),t);
	boolean seInsideT = PointTri_Ops.inside(s.getEndpoint(),t);

	if (ssInsideT || seInsideT) {
	    return true;
	}//if

	//no endpoint of segment lies inside of triangle
	//check for intersection with triangle's border
	Segment[] sArr = t.segmentArray();
	for (int i = 0; i < sArr.length; i++) {
	    if (s.pintersects(sArr[i])) {
		return true; 
	    }
	}

	//does segment overlap triangle's border?
	if (overlapsBorder(s,t)) { 
	    return false; }

	//segment lies fully inside of t and both endpoints are on
	//triangle's border
	int count = 0;
	for (int i = 0; i < sArr.length; i++) {
	    count = 0;
	    if (PointSeg_Ops.liesOn(s.getStartpoint(),sArr[i]) ||
		PointSeg_Ops.liesOn(s.getEndpoint(),sArr[i]))
		count++;
	}//for	

	if (count == 2) { 
	    return true; }
	else return false;
    }//end method pintersects


    /**
     * Returns that part of the segment, which is covered by the triangle.<p>
     * Prerequisite: The segment and triange <i>must</i> have a common part. Otherwise the result may be wrong.
     *
     * @param s the segment
     * @param t the triangle
     * @return the part of <code>s</code> that is covered by <code>t</code>
     */
    public static Segment intersection (Segment s, Triangle t) {
	/* This case does not really have to be checked. Now it is formulated as prerequisite of this method.
	//case 1: no intersection
	if (!(intersects(s,t))) { 
	    System.out.println("SegTri_OPs.intersection: ERROR! No intersection.");
	    System.exit(0);
	    return new Segment();
	}//if
	*/

	//case 2a: s lies fully inside of t
	if (inside(s,t)) {
	    return s;
	}//if

	//case 2b: s lies fully inside of t, both endpoints lie on t's border
	boolean startOnBorder = false;
	boolean endOnBorder = false;
	Segment[] tArr = t.segmentArray();
	for (int i = 0; i < 3; i ++) {
	    if (!startOnBorder && PointSeg_Ops.liesOn(s.getStartpoint(),tArr[i])) startOnBorder = true;
	    if (!endOnBorder && PointSeg_Ops.liesOn(s.getEndpoint(),tArr[i])) endOnBorder = true;
	}//for i
	if (startOnBorder && endOnBorder) {
	    return s;
	}//if

	
	//case 2.5: s overlaps the border of t
	for (int i = 0; i < tArr.length; i++) 
	    if (SegSeg_Ops.overlap(s,tArr[i])) {
		return SegSeg_Ops.theOverlap(s,tArr[i]);
	    }//if


	//case 3/4: there are intersection points
	Vector intPoints = new Vector(3);
	boolean firstSaved = false;
	Point intPoint;
	boolean isEndPoint;

	for (int i = 0; i < tArr.length; i++) {
	    if (s.intersects(tArr[i])) {
		isEndPoint = false;
		intPoint = s.intersection(tArr[i]);
		if (PointSeg_Ops.isEndpoint(intPoint,tArr[i])) {
		    isEndPoint = true;
		}
		if (!isEndPoint) {
		    intPoints.add(intPoint);
		} else if (!firstSaved) {
		    intPoints.add(intPoint);
		    firstSaved = true;
		}		
	    }//if
	}//for i


	//case 3: one intersection point
	if (intPoints.size() == 1) {
	    if (PointTri_Ops.inside(s.getStartpoint(),t)) {
		return(new Segment(s.getStartpoint(),(Point)intPoints.firstElement()));
	    }//if
	    else {
		return(new Segment(s.getEndpoint(),(Point)intPoints.firstElement()));
	    }//else
	}//if

	//case 4: two intersection points
	if (intPoints.size() == 2) {
	    return (new Segment((Point)intPoints.firstElement(),(Point)intPoints.lastElement()));
	}//if
	
	//This case should never be reached.
	return new Segment();
    }//end method intersection
	

    /**
     * Returns <code>true</code> if the segment is completely covered by the segment.
     * This method also returns <code>true</code> if one (or two) of the segment's endpoints
     * lies on the triangle's border.
     *
     * @param s the segment
     * @param t the triangle
     * @return {<code>true</code>, <code>false</code>} depending on the mutual position of both objects
     */
    public static boolean inside (Segment s, Triangle t) {
	boolean retVal = false;
	
	if ((PointTri_Ops.inside(s.getStartpoint(),t) ||
	     PointTri_Ops.liesOnBorder(s.getStartpoint(),t)) &&
	    (PointTri_Ops.inside(s.getEndpoint(),t) ||
	     PointTri_Ops.liesOnBorder(s.getEndpoint(),t))) {
	    retVal = true;
	}//if
	else {
	    retVal = false;
	}//else
	return retVal;
    }//end method inside
	

    /**
     * Returns <code>true</code> if the segment and the triangle have common points.
     * Particularly, this is <code>true</code> when the segment overlaps the triangle's border
     * or <i>meets</i> the border in one point. This method is based on <code>Segment.intersects</code>.
     *
     * @param s the segment
     * @param t the triangle
     * @return {<code>true</code>, <code>false</code>} depending on the mutual position of both objects
     * @see #pintersects(Segment,Triangle)
     * @see #isCovered(Segment,Triangle)
     */
    public static boolean intersects (Segment s, Triangle t) {
	if (PointTri_Ops.inside(s.getStartpoint(),t) ||
	    PointTri_Ops.inside(s.getEndpoint(),t)) {
	    return true; }//if
	else {
	    Segment[] tArr = t.segmentArray();
	    for (int i = 0; i < 3; i++) {
		if (s.intersects(tArr[i])) {
		    return true; }//if
	    }//for i
	}//else
	return false;
    }//end method intersects


    /**
     * Returns the shortest distance between the segment and the triangle.
     *
     * @param s the segment
     * @param t the triangle
     * @return the distance as a <code>Rational</code>
     */
    public static Rational dist (Segment s, Triangle t) {
	if (intersects(s,t)) { return (RationalFactory.constRational(0)); }
	else {
	    LinkedList distlist = new LinkedList();
	    
	    Segment[] tArr = t.segmentArray();;
	    for (int i = 0; i < 3; i++) {
		distlist.add(s.dist(tArr[i]));
	    }//for i
	    Rational min = (Rational)distlist.getFirst();
	    for (int i = 1; i < 3; i++) {
		if (((Rational)distlist.get(i)).less(min)) {
		    min = (Rational)distlist.get(i); }//if
	    }//for i
	    return min.copy();
	}//else
    }//end method dist


    /**
     * Returns <code>true</code> if the segments overlaps one of the triangle's border segments.
     *
     * @param s the segment
     * @param t the triangle
     * @return {<code>true</code>, <code>false</code>} depending on the mutual position of both objects     
     */
    public static boolean overlapsBorder (Segment s, Triangle t) {
	Segment[] tArr = t.segmentArray(); 
	for (int i = 0; i < tArr.length; i++) {
	    if (SegSeg_Ops.overlap(s,tArr[i])) { 
		return true;
	    }//if
	}//for i
	return false;
    }//end method overlapsBorder


    /**
     * Returns <code>true</code> if the segments is covered by the triangle.
     * Particularly, a (part of the)segment is covered, if it lies on the boundary of the triangle.
     * It is <i>not</i> covered, if only one of its endpoints lies on the triangles boundary.
     *
     * @param s the segment
     * @param t the triangle
     * @return {<code>true</code>, <code>false</code>} depending on the mutual position of both objects
     * @see #intersects(Segment,Triangle)
     * @see #pintersects(Segment,Triangle)
     */
    public static boolean isCovered (Segment s, Triangle t) {
	if (overlapsBorder(s,t)) return true;
	return pintersects(s,t);
    }//end method isCovered
        
}//end class SegTri_Ops
