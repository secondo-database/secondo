/*
 * SegSeg_Ops.java 2005-05-02
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.basictypeoperation;

import twodsack.io.*;
import twodsack.operation.basictypeoperation.*;
import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;
import twodsack.util.comparator.*;

import java.io.*;

/**
 * The SegSeg_Ops class holds methods with parameter types <code>Segment x Segment</code>. The methods
 * in this class are all static. They are commonly used as parameter functions for set operations of 
 * class {@link twodsack.operation.setoperation.SetOps}. If you don't find a specific method with the parameter types <code>Segment x Segment</code>
 * you should search for it in class {@link Segment}.
 */

public class SegSeg_Ops {
    /*
     * fields
     */
    private static SegmentComparator SEGMENT_COMPARATOR = new SegmentComparator();
    
    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public SegSeg_Ops(){}


    /*
     * methods
     */
    /**
     * Returns <tt>true</tt> if both segments have at least one common point.
     * This method also returns <tt>true</tt> when <tt>s1,s2</tt> are equal, overlap, intersect or if
     * one of the segment's endpoints lies on the other one.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if <tt>s1,s2</tt> have at least one common point
     */
    public static boolean pointsInCommon(Segment s1, Segment s2) {
	if (s1.equal(s2)) { return true; }
	if (s1.pintersects(s2)) { return true; }
	if (PointSeg_Ops.isEndpoint(s1.getStartpoint(),s2) ||
	    PointSeg_Ops.isEndpoint(s1.getEndpoint(),s2) ||
	    PointSeg_Ops.isEndpoint(s2.getStartpoint(),s1) ||
	    PointSeg_Ops.isEndpoint(s2.getEndpoint(),s1)) return true;
	if (overlap(s1,s2)) { return true; }
	if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2)) { return true; }
	if (PointSeg_Ops.liesOn(s1.getEndpoint(),s2)) { return true; }
	if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1)) { return true; }
	if (PointSeg_Ops.liesOn(s2.getEndpoint(),s1)) { return true; }
	    
	return false;
    }//end method pointsInCommon


    /**
     * Returns <tt>true</tt>, if both segments meet.
     * The method name <i>pmeet</i> instead of <i>meet</i> indicates, that both segments 
     * <i>properly</i> meet, i.e. this method returns <tt>true</tt>, if the endpoint of one segment
     * lies on the other segment, but is <u>not</u> an endpoint of that segment. Furthermore,
     * both segments may not overlap.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if <tt>s1,s2</tt> properly meet
     */
    public static boolean pmeet (Segment s1, Segment s2) {
	if (s1.equal(s2)) return false;

	if ((PointSeg_Ops.liesOn(s1.getStartpoint(),s2) ||
	     PointSeg_Ops.liesOn(s1.getEndpoint(),s2) ||
	     PointSeg_Ops.liesOn(s2.getStartpoint(),s1) ||
	     PointSeg_Ops.liesOn(s2.getEndpoint(),s1)) &&
	    !(formALine(s1,s2)) || overlap(s1,s2)) {
	    return true; }
	else {
	    return false; }
    }//end method pmeet


    /**
     * Returns <tt>true</tt>, if both segments form a line.
     * Two segments form a line, if <u>one</u> of their endpoints is equal. The segments don't 
     * need to be collinear to form a line.
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if <tt>s2,s2</tt> form a line
     */
    public static boolean formALine(Segment s1, Segment s2) {
	boolean endpointEQ = false;
	if (s1.equal(s2)) {
	    return false; }
	
	if (PointSeg_Ops.isEndpoint(s1.getStartpoint(),s2) ||
	    PointSeg_Ops.isEndpoint(s1.getEndpoint(),s2) ||
	    PointSeg_Ops.isEndpoint(s2.getStartpoint(),s1) ||
	    PointSeg_Ops.isEndpoint(s2.getEndpoint(),s1)) {
	    endpointEQ = true;
	}//if

	if (endpointEQ) {
	    boolean s1sONs2 = PointSeg_Ops.liesOn(s1.getStartpoint(),s2);
	    boolean s1eONs2 = PointSeg_Ops.liesOn(s1.getEndpoint(),s2);
	    if (s1sONs2 && s1eONs2) {
		return false; }
	    boolean s2sONs1 = PointSeg_Ops.liesOn(s2.getStartpoint(),s1);
	    boolean s2eONs1 = PointSeg_Ops.liesOn(s2.getEndpoint(),s1);
	    if (s2sONs1 && s2eONs1) { 
		return false; }
	    return true;
	}//if
	return false;
    }//end formALine
    

    /**
     * Returns <tt>true</tt>, if both segments form a single segments.
     * Two segments form a segment, if one of their endpoints is equal, if they don't overlap
     * and if they are are collinear.<p>
     * Same as {@link #adjacent(Segment,Segment)}.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if <tt>s1,s2</tt> form a segment
     * @see #adjacent(Segment,Segment)
     */
    public static boolean formASegment(Segment s1, Segment s2) {
	if (formALine(s1,s2) && Mathset.linearly_dependent(s1,s2))
	    return true;
	else return false;
    }//end method formASegment
    

    /**
     * Returns <tt>true</tt>, if both segments are adjacent.
     * Two segments are adjacent, if one of their endpoints is equal, if they don't overlap
     * and if they are collinear.<p>
     * Same as {@link #formASegment(Segment,Segment)}
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if <tt>s1,s2</tt> are adjacent
     * @see #formASegment(Segment,Segment)
     */
    public static boolean adjacent(Segment s1, Segment s2) {
	if (formALine(s1,s2) && 
	    Mathset.linearly_dependent(s1,s2)) { return true; }
	else { return false; }
    }//end method adjacent
    

    /**
     * Returns <tt>true</tt>, if both segments overlap.
     * Two segments overlap, if they are collinear and have an overlapping part that
     * is a segment itself. 
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if <tt>s1,s2</tt> overlap
     */
    public static boolean overlap(Segment s1, Segment s2){
	if (s1.equal(s2))
	    { return true; }
	else {
	    boolean linDep = Mathset.linearly_dependent(s1,s2);
	    boolean poton = pointOnTheOtherOne(s1,s2);
	    if (linDep && poton) {
		return true;
	    }//if
	}//else
	return false;
    }//end method overlap
    

    /**
     * Returns the common part of two segments.
     * The common part of two segments is the overlapping part of two segments that overlap.
     * Returns an 'empty' (or dummy) segment, if the segments don't overlap.<p>
     * Similar to theOverlap but has a different implementation.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return the common part of <tt>s1,s2</tt>
     */
    public static Segment commonPart(Segment s1, Segment s2){
	if (overlap(s1,s2)) {
	    if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2) &&
		PointSeg_Ops.liesOn(s1.getEndpoint(),s2)) {
		return (Segment)s1.copy();
	    }//if
	    if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1) &&
		PointSeg_Ops.liesOn(s2.getEndpoint(),s1)) {
		return (Segment)s2.copy();
	    }//if
	    
	    if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2) &&
		PointSeg_Ops.liesOn(s2.getStartpoint(),s1)) {
		Segment e = new Segment(s2.getStartpoint(),s1.getStartpoint());
		return e;
	    }//if
	    if (PointSeg_Ops.liesOn(s1.getEndpoint(),s2) &&
		PointSeg_Ops.liesOn(s2.getStartpoint(),s1)) {
		Segment e = new Segment(s2.getStartpoint(),s1.getEndpoint());
		return e;
	    }//if
	    if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2) &&
		PointSeg_Ops.liesOn(s2.getEndpoint(),s1)) {
		Segment e = new Segment(s2.getEndpoint(),s1.getStartpoint());
		return e;
	    }//if
	    if (PointSeg_Ops.liesOn(s1.getEndpoint(),s2) &&
		PointSeg_Ops.liesOn(s2.getEndpoint(),s1)) {
		Segment e = new Segment(s2.getEndpoint(),s1.getEndpoint());
		return e;
	    }//if
	}//if
	Segment e = new Segment();
	return e;
    }//end method common_part
    
    
    /**
     * Returns the union of both segments.
     * The union of two segments can be computed only for two overlapping segments. If the segments don't
     * overlap, a <tt>NoOverlapException</tt> is thrown.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return the union of <tt>s1,s2</tt>
     * @throws NoOverlapException if <tt>s1,s2</tt> don't overlap
     */
    public static Segment union (Segment s1, Segment s2) throws NoOverlapException {
	Segment retSeg = null;
	Point p1 = null;
	Point p2 = null;
	
	if (s1.equal(s2)) { return (Segment)s1.copy(); }
	
	if (overlap(s1,s2)) {
	    if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2) &&
		!PointSeg_Ops.liesOn(s1.getEndpoint(),s2)) {
		p1 = s1.getEndpoint(); }//if
	    else {
		if (PointSeg_Ops.liesOn(s1.getEndpoint(),s2) &&
		    !PointSeg_Ops.liesOn(s1.getStartpoint(),s2)) {
		    p1 = s1.getStartpoint(); }//else
		else if ((PointSeg_Ops.liesOn(s1.getStartpoint(),s2)) &&
			 (PointSeg_Ops.liesOn(s1.getEndpoint(),s2))){
		    //both points lie on s2
		    return (Segment)s2.copy();
		}//else
	    }//else
	    if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1) &&
		!PointSeg_Ops.liesOn(s2.getEndpoint(),s1)) {
		p2 = s2.getEndpoint(); }//if
	    else {
		if (PointSeg_Ops.liesOn(s2.getEndpoint(),s1) &&
		    !PointSeg_Ops.liesOn(s2.getStartpoint(),s1)) {
		    p2 = s2.getStartpoint(); }//else
		else if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1) &&
			 PointSeg_Ops.liesOn(s2.getEndpoint(),s1)){
		    //both points lie on s1
		    return (Segment)s1.copy();
		}//else
	    }//else
	    retSeg = new Segment(p1,p2);
	}//if
	else { throw new NoOverlapException(); }

	return retSeg;
    }//union


    /**    
     * Computes the symmetric difference of two segments.<p>
     * The symmetric difference is computed as the union of both segments minus the overlapping part of both.
     * For two equal segments, the resulting set is empty; for two non-overlapping segments <tt>s1,s2</tt> the resulting set
     * is <tt>{s1, s2}</tt>.<p>
     * Note that the result of this method is a SegMultiSet, i.e. a set of segments. If, particularly, both endpoints
     * of a segment lie on the other segment (but are not equal to its endpoints), the resulting set has two elements.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return the symmetric difference of <tt>s1,s2</tt>
     */
    public static SegMultiSet symDiff(Segment s1, Segment s2) {
	SegMultiSet retSeg = new SegMultiSet(SEGMENT_COMPARATOR);
	Point p1 = null;
	Point p2 = null;
	Point p3 = null;
	Point p4 = null;
	
	//if s1,s2 equal return mt
	if (s1.equal(s2)) { 
	    return retSeg; }

	//if s1,s2 overlap...
	if (overlap(s1,s2)) {

	    Segment union = union(s1,s2);
	    Segment ovLap = theOverlap(s1,s2);
	    retSeg = minus(union,ovLap);

	    /*
	    //if s1 fully lies on s2
	    if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2) &&
		PointSeg_Ops.liesOn(s1.getEndpoint(),s2)) {
		//if s1's vertices aren't vertices of s2: we've two segments
		if (!PointSeg_Ops.isEndpoint(s1.getStartpoint(),s2) &&
		    !PointSeg_Ops.isEndpoint(s1.getEndpoint(),s2)) {
		    //use the proper vertices of s2 to build two new segments
		    if ((new Segment(s1.getStartpoint(),s2.getStartpoint()).length()) <
			(new Segment(s1.getStartpoint(),s2.getEndpoint())).length()) {
			retSeg.add(new Segment(s1.getStartpoint(),s2.getStartpoint()));
			retSeg.add(new Segment(s1.getEndpoint(),s2.getEndpoint()));
			//return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s1.getStartpoint(),s2.getEndpoint()));
			retSeg.add(new Segment(s1.getEndpoint(),s2.getStartpoint()));
			//return retSeg;
		    }//if
		}//if

		//s1.startpoint is vertex of s2: build one new segment
		if (PointSeg_Ops.isEndpoint(s1.getStartpoint(),s2)) { 
		    if (s1.getStartpoint().equal(s2.getStartpoint())) {
			retSeg.add(new Segment(s1.getEndpoint(),s2.getEndpoint()));
			//return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s1.getEndpoint(),s2.getStartpoint()));
			//return retSeg;
		    }//else
		}//if
		
		//s1.endpoint is vertex of s2: build one new segment
		if (PointSeg_Ops.isEndpoint(s1.getEndpoint(),s2)) {
		    
		    if (s1.getEndpoint().equal(s2.getStartpoint())) {
			retSeg.add(new Segment(s1.getStartpoint(),s2.getEndpoint()));
			//return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s1.getStartpoint(),s2.getStartpoint()));
			//return retSeg;
		    }//else
		}//if
	    }//if

	    //now do the same checks for s2
	    
	    //if s2 fully lies on s1
	    if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1) &&
		PointSeg_Ops.liesOn(s2.getEndpoint(),s1)) {
		//if s2's vertices aren't vertices of s1: we've two segments
		if (!PointSeg_Ops.isEndpoint(s2.getStartpoint(),s1) &&
		    !PointSeg_Ops.isEndpoint(s2.getEndpoint(),s1)) {
		    //use the proper vertices of s1 to build two new segments
		    if ((new Segment(s2.getStartpoint(),s1.getStartpoint())).length() <
			((new Segment(s2.getStartpoint(),s1.getEndpoint())).length())) {
			retSeg.add(new Segment(s2.getStartpoint(),s1.getStartpoint()));
			retSeg.add(new Segment(s2.getEndpoint(),s1.getEndpoint()));
			//return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s2.getStartpoint(),s1.getEndpoint()));
			retSeg.add(new Segment(s2.getEndpoint(),s1.getStartpoint()));
			//return retSeg;
		    }//if
		}//if

		//s2.startpoint is vertex of s1: build one new segment
		if (PointSeg_Ops.isEndpoint(s2.getStartpoint(),s1)) {
		    if (s2.getStartpoint().equal(s1.getStartpoint())) {
			retSeg.add(new Segment(s2.getEndpoint(),s1.getEndpoint()));
			//return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s2.getEndpoint(),s1.getStartpoint()));
			//return retSeg;
		    }//else
		}//if

		//if s2.endpoint is vertex of s1: build new segment
		if (PointSeg_Ops.isEndpoint(s2.getEndpoint(),s1)) {
		    if (s2.getEndpoint().equal(s1.getStartpoint())) {
			retSeg.add(new Segment(s2.getStartpoint(),s1.getEndpoint()));
			//return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s2.getStartpoint(),s1.getStartpoint()));
			//return retSeg;
		    }//else
		}//if
	    }//if
		    
	    else {
		if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2)) {
		    p1 = s1.getEndpoint();
		    p3 = s1.getStartpoint();
		}//if
		else {
		    p1 = s1.getStartpoint();
		    p3 = s1.getEndpoint();
		}//if
		if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1)) {
		    p2 = s2.getStartpoint();
		    p4 = s2.getEndpoint();
		}//if
		else {
		    p2 = s2.getEndpoint();
		    p4 = s2.getStartpoint();
		}//else
		retSeg.add(new Segment(p1,p2));
		retSeg.add(new Segment(p3,p4));
	    }//if
	    */
	}//if s1,s2 overlap
	else {
	    //s1,s2 don't overlap
	    //put them both in result set
	    retSeg.add(s1);
	    retSeg.add(s2);
	}//else

	return retSeg;
    }//end method symDiff


    /**
     * Returns the overlapping part of two segments.
     * Similar to {@link #commonPart(Segment,Segment)} but has a different implementation.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return the overlapping part of <tt>s1.s2</tt>
     * @throws NoOverlapException
     * @see #commonPart(Segment,Segment)
     */
    public static Segment theOverlap(Segment s1, Segment s2)
	throws NoOverlapException{
	
	if (!overlap(s1,s2)) { throw new NoOverlapException(); }
	else {
	    if (s1.equal(s2)) { return s1; }//if
	    else { return commonPart(s1,s2); }
	}//else
    }//end method theOverlap


    /**
     * Concats two adjacent segments.
     * For two adjacent segments, this method returns the concatenation of both.
     * 
     * @param s1 the first segment
     * @param s2 the second segment
     * @return the concatenation of <tt>s1,s2</tt>
     * @throws NoAdjacentSegmentException
     */
    public static Segment concat (Segment s1, Segment s2)
	throws NoAdjacentSegmentException{
		Point p1;
	Point p2;
	if (!adjacent(s1,s2)) { throw new NoAdjacentSegmentException(); }
	else {
	    if (PointSeg_Ops.isEndpoint(s1.getStartpoint(),s2)) { p1 = s1.getEndpoint(); }
	    else { p1 = s1.getStartpoint(); }
	    if (PointSeg_Ops.isEndpoint(s2.getStartpoint(),s1)) { p2 = s2.getEndpoint(); }
	    else { p2 = s2.getStartpoint(); }
	}//else
	return (new Segment(p1,p2));
    }//end method concat

    
    /**
     * Returns the set of segments emerging from splitting two segments at the 'split' points.<p>
     * Split points of two segments are intersection points and endpoints lying on the other segment, i.e.
     * meeting and overlapping segments are split.
     * As an example, for two segments that meet, that segment which covers one endpoint of the other segment
     * is split in two parts. Then, the resulting set holds three segments in total.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return the set of segments emerging from splitting <tt>s1</tt> and <tt>s2</tt> at their split points.
     */
    public static SegMultiSet split (Segment s1, Segment s2) {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	Point intPoint;
	Point intPoint1;
	Point intPoint2;
	
	//segments are equal
	if (s1.equal(s2)) {
	    retList.add(s1);
	    return retList;
	}//if

	//segments properly intersect
	if (s1.pintersects(s2)) {
	    intPoint = s1.intersection(s2);
	    retList.add(new Segment(s1.getStartpoint(),intPoint));
	    retList.add(new Segment(s1.getEndpoint(),intPoint));
	    retList.add(new Segment(s2.getStartpoint(),intPoint));
	    retList.add(new Segment(s2.getEndpoint(),intPoint));
	    return retList;
	}//if
	
	//segments overlap and form two segments
	if (overlap(s1,s2) &&
	    (PointSeg_Ops.isEndpoint(s2.getStartpoint(),s1) ||
	     PointSeg_Ops.isEndpoint(s2.getEndpoint(),s1) ||
	     PointSeg_Ops.isEndpoint(s1.getStartpoint(),s2) ||
	     PointSeg_Ops.isEndpoint(s1.getEndpoint(),s2))) {
	    try {
		Segment ovLap = theOverlap(s1,s2);
		retList.add(ovLap);
		if (ovLap.equal(s1)) {
		    retList.addAll(symDiff(s2,ovLap));
		}//if
		else if (ovLap.equal(s2)) {
		    retList.addAll(symDiff(s1,ovLap));
		}//if
		else if (overlap(s1,ovLap)) {
		    retList.addAll(symDiff(s1,ovLap));
		}//if
		else {
		    retList.add(symDiff(s2,ovLap));
		}//else
		return retList;
	    }//try
	    catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	    }//catch
	}//if


	//segment overlap and form three segments:
	//one segment fully lies on the other one
	if (overlap(s1,s2) &&
	    (PointSeg_Ops.liesOn(s1.getStartpoint(),s2) &&
	     PointSeg_Ops.liesOn(s1.getEndpoint(),s2)) ||
	    (PointSeg_Ops.liesOn(s2.getStartpoint(),s1) &&
	     PointSeg_Ops.liesOn(s2.getEndpoint(),s1))) {
	    try {
		Segment ov = theOverlap(s1,s2);
		retList.add(ov);
		if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2)) {
		    retList.addAll(symDiff(ov,s2)); }
		else { retList.addAll(symDiff(ov,s1)); }
	    } catch (Exception e) {}
	    return retList;
	}//if

	//segments overlap and form three segments
	if (overlap(s1,s2)) {
	    if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2)) { intPoint1 = s1.getStartpoint(); }
	    else { intPoint1 = s1.getEndpoint(); }
	    if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1)) { intPoint2 = s2.getStartpoint(); }
	    else { intPoint2 = s2.getEndpoint(); }
	    retList.add(new Segment(intPoint1,intPoint2));
	    retList.add(new Segment(s1.theOtherOne(intPoint1),intPoint1));
	    retList.add(new Segment(s2.theOtherOne(intPoint2),intPoint2));
	    return retList;
	}//if

	//segments form a line
	if (formALine(s1,s2)) {;
	    retList.add(s1);
	    retList.add(s2);
	    return retList;
	}//if

	//one endpoint lies on the other segment
	if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2)) {
	    intPoint = s1.getStartpoint();
	    retList.add(s1);
	    retList.add(new Segment(s2.getStartpoint(),intPoint));
	    retList.add(new Segment(s2.getEndpoint(),intPoint));
	    return retList;
	}//if
	else if (PointSeg_Ops.liesOn(s1.getEndpoint(),s2)) {
	    intPoint = s1.getEndpoint();
	    retList.add(s1);
	    retList.add(new Segment(s2.getStartpoint(),intPoint));
	    retList.add(new Segment(s2.getEndpoint(),intPoint));
	    return retList;
	}//if
	else if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1)) {
	    intPoint = s2.getStartpoint();
	    retList.add(s2);
	    retList.add(new Segment(s1.getStartpoint(),intPoint));
	    retList.add(new Segment(s1.getEndpoint(),intPoint));
	    return retList;
	}//if
	
	//At last, we found out that both segments don't even have common points:
	retList.add(s1);
	retList.add(s2);
		
	return retList;
    }//end method split


    /**
     * Subracts the second segment from the first segment and returns the non-overlapping part(s) as SegMultiSet.
     * If both segments don't overlap, the first segment is returned unchanged.
     *
     * @param seg1 the first segment
     * @param seg2 the second segment
     * @return the set of segments resulting from subtracting <tt>s2</tt> from <tt>s1</tt>
     */
    public static SegMultiSet minus (final Segment seg1, final Segment seg2) {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	if (!overlap(seg1,seg2)) {
	    retList.add(seg1);
	    return retList;
	}//if

	//segments are equal
	if (seg1.equal(seg2)) {
	    return retList; }
	
	//one point is equal
	boolean s1s = PointSeg_Ops.isEndpoint(seg1.getStartpoint(),seg2);
	boolean s1e = PointSeg_Ops.isEndpoint(seg1.getEndpoint(),seg2);
	
	if (s1s || s1e) {
	    boolean s2s = PointSeg_Ops.isEndpoint(seg2.getStartpoint(),seg1);
	    boolean s2e = PointSeg_Ops.isEndpoint(seg2.getEndpoint(),seg1);
	    if (s1s && s2s) {
		retList.add(new Segment(seg1.getEndpoint(),seg2.getEndpoint())); }
	    else if (s1s && s2e) {
		retList.add(new Segment(seg1.getEndpoint(),seg2.getStartpoint())); }
	    else if (s1e && s2s) {
		retList.add(new Segment(seg1.getStartpoint(),seg2.getEndpoint())); }
	    else {
		retList.add(new Segment(seg1.getStartpoint(),seg2.getStartpoint())); }
	    return retList;
	}//if

	//no equal points
	//seg1 fully lies on seg2
	if (PointSeg_Ops.liesOn(seg1.getStartpoint(),seg2) &&
	    PointSeg_Ops.liesOn(seg1.getEndpoint(),seg2)) {
	    return retList; }
	//seg2 lies fully on seg1
	if (PointSeg_Ops.liesOn(seg2.getStartpoint(),seg1) &&
	    PointSeg_Ops.liesOn(seg2.getEndpoint(),seg1)) {
	    if (seg1.getStartpoint().dist(seg2.getStartpoint()).less
		(seg1.getStartpoint().dist(seg2.getEndpoint()))) {
		retList.add(new Segment(seg1.getStartpoint(),seg2.getStartpoint()));
		retList.add(new Segment(seg1.getEndpoint(),seg2.getEndpoint()));
		return retList; }
	    else {
		retList.add(new Segment(seg1.getStartpoint(),seg2.getEndpoint()));
		retList.add(new Segment(seg1.getEndpoint(),seg2.getStartpoint()));
		return retList; }
	}//if

	//This point should never be reached.
	if (true) throw new RuntimeException("Reached point of no return in SegSeg_Ops.minus.");
	return retList;
    }//end method minus

			
    /**
     * Returns <tt>true</tt>, if an enpoint of one segment lies on the other segment.
     * If both segments only have one endpoint in common, this method returns <tt>false</tt>.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if an endpoint of one segment lies on the other segment
     */
    public static boolean pointOnTheOtherOne (Segment s1, Segment s2) {
	if (PointSeg_Ops.liesOn(s1.getStartpoint(),s2) &&
	    !(PointSeg_Ops.isEndpoint(s1.getStartpoint(),s2))) return true;
	if (PointSeg_Ops.liesOn(s1.getEndpoint(),s2) &&
	    !(PointSeg_Ops.isEndpoint(s1.getEndpoint(),s2))) return true;
	if (PointSeg_Ops.liesOn(s2.getStartpoint(),s1) &&
	    !(PointSeg_Ops.isEndpoint(s2.getStartpoint(),s1))) return true;
	if (PointSeg_Ops.liesOn(s2.getEndpoint(),s1) &&
	    !(PointSeg_Ops.isEndpoint(s2.getEndpoint(),s1))) return true;
	return false;
    }//end method pointOnTheOtherOne

} //end class SegSeg_Ops
