/*
 * Segment.java 2005-05-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.basicdatatype;

import twodsack.operation.basictypeoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.util.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;

import java.util.*;
import java.io.*;


/**
 * The Segment class implements one of the three basic datatypes, a segment.
 * The most interesting (private) fields of this class are 
 * <tt>startpoint</tt> and <tt>endpoint</tt>, which are implemented as {@link Point} instances. Additionally, it has a
 * <tt>bbox</tt> (a bounding box), a <tt>length</tt>
 * and two flags which indicate, whether a value for these fields was already computed (<tt>bboxDefined</tt>, <tt>lengthDefined</tt>). This
 * means, that the <tt>bbox</tt> (or <tt>length</tt>) is not computed when a new instance of this class in constructed and <tt>bboxDefined</tt>
 * (or <tt>lengthDefined</tt>) is initially set to <tt>false</tt>. Only when the <tt>bbox</tt> (or <tt>length</tt>) is requested,
 * it is computed and is set then.<p>
 * A segment can be <i>aligned</i>. An aligned segment's startpoint is alway smaller than its endpoint. For this comparison of
 * points, the <tt>Point.compare()</tt> method is used. A segment is <i>not</i> aligned automatically when constructed.
 */
public class Segment extends Element implements Serializable {
    
    /*
     * fields
     */
    private Point startpoint; 
    private Point endpoint; 
    private boolean bboxDefined;
    private Rect bbox;
    private boolean lengthDefined;
    private double length;
    
    private PointComparator POINT_COMPARATOR = new PointComparator();
    private SegmentComparator SEGMENT_COMPARATOR = new SegmentComparator();

    static Point T1 = new Point(0,0);
    static Point T2 = new Point(0,0);
    static Rational TMP1 = RationalFactory.constRational(0);
    static Rational R1 = RationalFactory.constRational(0);
    static Rational R2 = RationalFactory.constRational(0);
    static Rational R3 = RationalFactory.constRational(0);

    
    /*
     * constructors
     */
    /**
     * The 'empty' constructor. 
     * Sets <tt>startpoint</tt>/<tt>endpoint</tt> to <tt>NULL</tt>.
     */
    public Segment() {
	this.startpoint = null;
	this.endpoint = null;
	this.bboxDefined = false;
	this.lengthDefined = false;
    }
    

    /**
     * Constructs a new segment with the double coordinates for the <tt>startpoint</tt>/<tt>endpoint</tt>.
     *
     * @param x1 x coordinate of the startpoint
     * @param y1 y coordinate of the startpoint
     * @param x2 x coordinate of the endpoint
     * @param y2 y coordinate of the endpoint
     * @throws NotAValidSegmentException
     */
    public Segment(double x1, double y1, double x2, double y2) throws NotAValidSegmentException {
	this.startpoint = new Point(x1,y1);
	this.endpoint = new Point(x2,y2);
	if (!isSegment()) {
	    throw new NotAValidSegmentException("Error in Segment.constructor: Tried to build bad Segment: ("+x1+", "+y1+") - ("+x2+", "+y2+")");
	}//if
	this.bboxDefined = false;
	this.lengthDefined = false;
    }


    /**
     * A new segment is constructed using the passed points as <tt>startpoint</tt>/<tt>endpoint</tt>.
     *
     * @param p1 the startpoint
     * @param p2 the endpoint
     * @throws NotAValidSegmentException
     */
    public Segment(Point p1, Point p2) throws NotAValidSegmentException {
	this.startpoint = (Point)p1.copy();
	this.endpoint = (Point)p2.copy();
	if (!isSegment()) {
	    throw new NotAValidSegmentException("Error in Segment.constructor: Tried to build bad Segment: "+p1+", "+p2);
	}//if
	this.bboxDefined = false;
	this.lengthDefined = false;
    }
    

    /**
     * Constructs a new segment using the given Rational numbers as coordinates.
     *
     * @param x1 the x coordinate of the startpoint
     * @param y1 the y coordinate of the startpoint
     * @param x2 the x coordinate of the endpoint
     * @param y2 the y coordinate of the endpoint
     * @throws NotAValidSegmentException
     */
    public Segment(Rational x1, Rational y1, Rational x2, Rational y2) throws NotAValidSegmentException {
	this.startpoint = new Point(x1,y1);
	this.endpoint = new Point(x2,y2);
	if (!isSegment()) {
	    throw new NotAValidSegmentException("Error in Segment.constructor: Tried to build bad Segment: ("+x1+", "+y1+") - ("+x2+", "+y2+").");
	}//if
	this.bboxDefined = false;
	this.lengthDefined = false;
    }
    

    /*
     * methods
     */
    /**
     * Returns <tt>true</tt>, if <i>this</i> is a segment.
     *
     * @return <tt>true</tt>, if <tt>startpoint</tt>/<tt>endpoint</tt> are not equal
     */
    private boolean isSegment() {
	if (!this.startpoint.equal(this.endpoint)) { return true; }
	else { return false; }
    }//end method isSegment()
    
    
    /**
     * Computes the length of <i>this</i> and sets <i>this</i>.<tt>length</tt>.
     *
     * @return the length as double
     */
    private double computeLength(){
	double l = Mathset.lengthD(Mathset.diff(endpoint,startpoint));
	this.lengthDefined = true;
	return l;
    }//end method computeLength
    
    
    /**
     * Sets the startpoint of <i>this</i> to <tt>p</tt>.<p>
     * Also sets <tt>bboxDefined</tt> = <tt>false</tt> and <tt>lengthDefined</tt> = <tt>false</tt>.
     * 
     * @param p the new startpoint
     * @return the changed segment
     */
    public Segment setStartpoint(Point p){
	this.startpoint = p;
	this.bboxDefined = false;
	this.lengthDefined = false;
	return this;
    }//end method setStartpoint
    

    /**
     * Sets the endpoint of <i>this</i> to <tt>p</tt>.
     * Also sets <tt>bboxDefines</tt> = <tt>false</tt> and <tt>lengthDefined</tt> = <tt>false</tt>.
     *
     * @param p the new endpoint
     * @return the changed segment
     */
    public Segment setEndpoint(Point p){
	this.endpoint = p;
	this.bboxDefined = false;
	this.lengthDefined = false;
	return this;
    }//end method setEndpoint
    

    /**
     * Returns the length of <i>this</i>.
     * Computes it, if not already stored; stores it then.
     *
     * @return the length as double
     */
    public double length(){
	if (this.lengthDefined) return this.length;
	else {
	    this.length = this.computeLength();
	    this.lengthDefined = true;
	    return this.length;
	}//else
    }//end method get_length
    

    /**
     * Returns the startpoint of <i>this</i>.
     * Changes on the returned point directly affect the segment.
     *
     * @return the startpoint
     */
    public Point getStartpoint(){
	return this.startpoint;
    }//end method getStartpoint
    

    /**
     * Returns the endpoint of <i>this</i>.
     * Changes on the returned point directly affect the segment.
     */
    public Point getEndpoint(){
	return this.endpoint;
    }//end method get_endpoint


    /**
     * Returns both endpoints stored in a {@link PointMultiSet}.
     *
     * @return the endpoints in a PointMultiSet
     */
    public PointMultiSet endpoints(){
	PointMultiSet pl = new PointMultiSet(POINT_COMPARATOR);
	pl.add(this.startpoint);
	pl.add(this.endpoint);
	return pl;
    }//end method endpoints


    /**    
     * Sets the segment's <tt>startpoint</tt>/<tt>endpoint</tt> to <tt>s,e</tt>.
     * Also sets <tt>bboxDefined</tt> = <tt>false</tt> and <tt>lengthDefined</tt> = <tt>false</tt>.
     *
     * @param s the new startpoint
     * @param e the new endpoint
     * @return the new segment
     */
    public Segment set(Point s, Point e){
	this.startpoint = s;
	this.endpoint = e;
	this.bboxDefined = false;
	this.lengthDefined = false;
	return this;
    }//end method set


    /**
     * Sets the coordinates of the segment's <tt>startpoint</tt>/<tt>endpoint</tt> to new coordinates.   
     * Also sets <tt>bboxDefined</tt> = <tt>false</tt> and <tt>lengthDefined</tt> = <tt>false</tt>.
     *
     * @param x1 the new x coordinate for the startpoint
     * @param y1 the new y coordinate for the startpoint
     * @param x2 the new x coordinate for the endpoint
     * @param y2 the new y coordinate for the endpoint
     */
    public void set(double x1, double y1, double x2, double y2) {
	Point start = new Point(RationalFactory.constRational(x1),
				RationalFactory.constRational(y1));
	Point end = new Point(RationalFactory.constRational(x2),
			      RationalFactory.constRational(y2));
	this.startpoint = start;
	this.endpoint = end;
	this.bboxDefined = false;
	this.lengthDefined = false;
    }//end method set


    /**
     * Returns a 'deep' copy of <i>this</i>.
     * This means, that no changes on the copy affect the original segment.
     *
     * @return the copy
     */
    public Element copy(){
	Segment copy = new Segment(this.startpoint.x,this.startpoint.y,
				   this.endpoint.x,this.endpoint.y);
	copy.lengthDefined = this.lengthDefined;
	copy.length = this.length;
	copy.bbox = this.bbox;
	copy.bboxDefined = this.bboxDefined;
	return copy;
    }//end method copy
    

    /**
     * Return that point of <i>this</i> which is not equal to <tt>p</tt>.
     * Make sure, that <tt>p</tt> is really a point of <i>this</i>. Otherwise the result may be not correct.
     * @param p a point of <i>this</i>
     * @return the other point of <i>this</i>
     */
    public Point theOtherOne(Point p){
	if (this.startpoint.equal(p)) { return this.endpoint; }
	else { return this.startpoint; }
    }//end method theOtherOne
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> and <tt>segin</tt> are equal.
     * Equality of two segments is computed by comparing their endpoints. Note, that <tt>(a,b) == (b,a)</tt>.
     *
     * @param segin the 'in' segment
     * @return <tt>true</tt>, if both segments are equal
     * @throws WrongTypeException if <tt>segin</tt> is not of type Segment
     */
    public boolean equal(Element segin) throws WrongTypeException {
	if (segin instanceof Segment) {
	    Segment sin = (Segment)segin;
	    
	    if ((this.startpoint.equal(sin.startpoint) &&
		 this.endpoint.equal(sin.endpoint)) ||
		(this.startpoint.equal(sin.endpoint) &&
		 this.endpoint.equal(sin.startpoint))) {
		return true; }
	    else { return false; }
	}//if
	
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+segin.getClass()); }
    }//end equal
    

    /**
     * Compares the x coordinates of the smallest endpoints of both segments and returns one of {0, 1, -1}.<p>
     * Returns 0, if the x coordinates are equal.<p>
     * Returns 1, if <tt>sin</tt> has the smaller x coordinate.<p>
     * Returns -1 otherwise.
     *
     * @param sin the object to compare with
     * @return {0, 1, -1} as byte
     * @throws WrongTypeException if <tt>sin</tt> is not of type Segment
     */
    public byte compX(Element sin) throws WrongTypeException {
	if (sin instanceof Segment) {
	    Segment seg = (Segment)sin;
	    Point lemoSeg;
	    Point lemoThis;
	    if (this.startpoint.compX(this.endpoint) == -1) {
		lemoThis = this.startpoint; }
	    else { lemoThis = this.endpoint; }
	    if (seg.startpoint.compX(seg.endpoint) == -1) {
		lemoSeg = seg.startpoint; }
	    else { lemoSeg = seg.endpoint; }
	    
	    byte res = lemoThis.compX(lemoSeg);
	    if (res == 0) { res = this.theOtherOne(lemoThis).compX(seg.theOtherOne(lemoSeg)); }    
	    return res;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+sin.getClass()); }
    }//end method compX
    

    /**
     * Compares the y coordinates of the smallest endpoints of both segments and returns one of {0, 1, -1}.<p>
     * Returns 0, if the y coordinates are equal.<p>
     * Returns 1, if <tt>sin</tt> has the smaller y coordinate.<p>
     * Returns -1 otherwise.
     *
     * @param sin the object to compare with
     * @return {0, 1, -1} as byte
     * @throws WrongTypeException if <tt>sin</tt> is not of type Segment
     */
    public byte compY(Element sin) throws WrongTypeException {
	if (sin instanceof Segment) {
	    Segment seg = (Segment)sin;
	    Point upmoSeg;
	    Point upmoThis;
	    
	    if (this.startpoint.compY(this.endpoint) == -1) {
		upmoThis = this.startpoint; }
	    else { upmoThis = this.endpoint; }
	    if (seg.startpoint.compY(seg.endpoint) == -1) {
		upmoSeg = seg.startpoint; }
	    else { upmoSeg = seg.endpoint; }
	    
	    byte res = upmoThis.compY(upmoSeg);
	    if (res == 0) { res = this.theOtherOne(upmoThis).compY(seg.theOtherOne(upmoSeg)); }
	    return res;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+sin.getClass()); }
    }//end method compY


    /**
     * Compares the coordinates of the smallest points of both segments and returns one of {0, 1, -1}.<p>
     * Returns 0, if both segments are equal.<p>
     * Returns 1, if <i>this</i> is greater.<p>
     * Returns -1 otherwise.<p>
     * Note, that both segments must be <u>aligned</u>. Otherwise, this method will not work correctly.
     */
    public int compare (ComparableMSE sin) throws WrongTypeException {
	if (sin instanceof Segment) {
	    Segment seg = (Segment)sin;
	    int res = this.startpoint.compare(seg.startpoint);
	    if (res == 0) res = this.endpoint.compare(seg.endpoint);
	    return res;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+sin.getClass()); }
    }//compare


    /**
     * Computes and stores the bounding box for <i>this</i>.
     *
     * @return the bounding box
     */
    private Rect computeBbox() {
	Rational hxv;//highest x value
	Rational lxv;//lowest x value
	Rational hyv;//highest y value
	Rational lyv;//lowest y value
	if (startpoint.x.less(endpoint.x)) {
	    hxv = endpoint.x;
	    lxv = startpoint.x;
	}//if
	else {
	    hxv = startpoint.x;
	    lxv = endpoint.x;
	}//else
	if (startpoint.y.less(endpoint.y)) {
	    hyv = endpoint.y;
	    lyv = startpoint.y;
	}//if
	else {
	    hyv = startpoint.y;
	    lyv = endpoint.y;
	}//else
	
	//build rectangle, start with upper left corner
	Rect bbox = new Rect(lxv,hyv,hxv,lyv);
	this.bboxDefined = true;
	return bbox;
    }//end method computeBbox
    
    
    /**
     * Prints the segment's data to standard output.
     */
    public void print(){
	System.out.println("Segment:");
	System.out.println(" startpoint: ("+startpoint.x.toString()+", "+startpoint.y.toString()+")");
	System.out.println(" endpoint: ("+endpoint.x.toString()+", "+endpoint.y.toString()+")");
    }//end method print
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> and <tt>e</tt> have common points.
     *
     * @param e the 'in' segment
     * @return <tt>true</tt>, if both segments have common points
     * @throws WrongTypeException if <tt>e</tt> is not of type Segment
     */
    public boolean intersects (Element e) throws WrongTypeException {
	if (e instanceof Segment) {
	    Segment s;
	    s = (Segment)e;
	    if (SegSeg_Ops.pointsInCommon(this,s)) {
		return true;
	    }//if
	}//if
	else {
	    throw new WrongTypeException();
	}//else
	return false;
    }//end method intersects
    
    
    /**
     * Returns <tt>true</tt>, if <i>this</i> and <tt>e</tt> have an intersection point that is not an endpoint of them.
     *
     * @param e the 'in' segment
     * @return <tt>true</tt>, if both segments properly intersect
     * @throws WrongTypeException if <tt>e</tt> is not of type Segment
     */
    public boolean pintersects(Element e) throws WrongTypeException {
	if (e instanceof Segment) {
	    Segment s = (Segment)e;
	    if (PointSeg_Ops.liesOn(this.startpoint,s) ||
		PointSeg_Ops.liesOn(this.endpoint,s) ||
		PointSeg_Ops.liesOn(s.startpoint,this) ||
		PointSeg_Ops.liesOn(s.endpoint,this)) {
		return false;
	    }//if

	    if (SegSeg_Ops.formALine(this,s) || SegSeg_Ops.formASegment(this,s)) { return false; }
	    
	    //if this.object's endpoints lie on different sides of s and
	    //s's endpoints lie on different sides of this.object then return <tt>true</tt>
	    int thiss;//this.object startpoint position regarding s
	    int thise;//this.object endpoint position regarding s
	    int ss;//dito
	    int se;//dito
	    
	    thiss = Mathset.pointPosition(s.startpoint,s.endpoint,this.startpoint);
	    thise = Mathset.pointPosition(s.startpoint,s.endpoint,this.endpoint);
	    ss = Mathset.pointPosition(this.startpoint,this.endpoint,s.startpoint);
	    se = Mathset.pointPosition(this.startpoint,this.endpoint,s.endpoint);

	    if ((((thiss == 1) && (thise == -1)) || ((thiss == -1) && (thise == 1))) &&
		(((ss == 1) && (se == -1)) || ((ss == -1) && (se == 1)))) {
		return true;
	    }//if
	    else {
		return false;
	    }//else
	    
	}//if
	else {
	    throw new WrongTypeException();
	}//else
    }//end method pintersects
    

    /**
     * Returns the intersection point of <code>this</code> and <tt>inseg</tt>.
     * Prerequisite: Both segments <i>must</i> have an intersection point. Otherwise a SegmentsDontIntersectException
     * is thrown.
     *
     * @param inseg the second segment
     * @return the intersection point
     * @throws SegmentsDontIntersectException if segments don't intersect
     */
    public Point intersection(Segment inseg) throws SegmentsDontIntersectException {
	Point retPoint = new Point();
	
	try {
	    //convert both segments to parametric form
	    Point p1;
	    Point p2;
	    Rational r;//parameter
	    Rational s;//parameter
	    double tsx,tsy,tex,tey,isx,isy,iex,iey;
	    double t1x = 0;
	    double t1y = 0;
	    double t2x = 0;
	    double t2y = 0;
	    
	    p1 = this.startpoint;
	    p2 = inseg.startpoint;
	    boolean t1yEQ0,t2yEQ0;
	    
	    T1.set(Mathset.diff(this.endpoint,this.startpoint,T1)); 
	    T2.set(Mathset.diff(inseg.endpoint,inseg.startpoint,T2)); 
	    
	    t1yEQ0 = T1.y.equal(0);
	    t2yEQ0 = T2.y.equal(0);
		
	    //compute r
	    if (!(t2yEQ0) && !(T1.x.minus(T1.y.times(T2.x.dividedby(T2.y,TMP1),TMP1),TMP1).equal(0))) {
		r = ((p2.x.minus(p1.x,R1)).minus((p2.y.minus(p1.y,R2)).times(T2.x.dividedby(T2.y,R3),R3),R3)).dividedby(T1.x.minus(T1.y.times(T2.x.dividedby(T2.y,R1),R1),R1),R1);
		
		retPoint.x = p1.x.plus(r.times(T1.x));
		retPoint.y = p1.y.plus(r.times(T1.y));
	    }//if
	    else {
		//compute s
		s = ((p2.x.minus(p1.x,R1)).minus((p2.y.minus(p1.y,R2)).times(T1.x.dividedby(T1.y,R3),R3),R3)).dividedby(((T2.x.times(-1)).minus(T2.y.times(T1.x.dividedby(T1.y,R1),R1),R1)));
		
		retPoint.x = p2.x.plus(s.times(T2.x,R1));
		retPoint.y = p2.y.plus(s.times(T2.y,R1));
	    }//else   
	} catch (Exception e) {
	    e.printStackTrace();
	    throw new SegmentsDontIntersectException("Segment.intersection: Segments (probably) don't intersect. "+this+", "+inseg);
	}//catch
	return retPoint;
    }//end method intersection


    /**
     * Returns the gradient/slope of <i>this</i>.
     *
     * @return the gradient as Rational
     * @throws InfiniteGradientException if <i>this</i> is a vertical segment
     */
    public Rational gradient() throws InfiniteGradientException{
	Rational r;
	if (!((this.startpoint.x.minus(this.endpoint.x)).equal(0))) {
	    r = ((this.startpoint.y.minus(this.endpoint.y)).dividedby((this.startpoint.x.minus(this.endpoint.x)))); }//if
	else {
	    //deny division by zero
	    throw new InfiniteGradientException();
	}//else
	return r;
    }//end method gradient

    
    /**
     * Returns the bounding box of <i>this</i>.
     * If the bounding box is not already stored, it is computed and stored afterwards.
     *
     * @return the bounding box
     */
    public Rect rect() {
	if (this.bboxDefined) return this.bbox;
	else {
	    this.bbox = this.computeBbox();
	    return this.bbox;
	}//else
    }//end method bbox
    

    /**
     * Returns the Euclidean distance of the two segments.
     *
     * @return the distance as Rational
     * @throws WrongTypeException if e is not of type Segment
     */
    public Rational dist(Element e) throws WrongTypeException {
	if (e instanceof Segment) {
	    Segment s = (Segment)e;
	    LinkedList distlist = new LinkedList();
	    Rational min = RationalFactory.constRational(0);//must be initialized
	    if ((!s.intersects(this)) &&
		(!s.equal(this)) &&
		(!SegSeg_Ops.formALine(s,this))) {
		Point ProjSThisStart = Mathset.projectionPointLine(this.startpoint,s.startpoint,s.endpoint);
		if (PointSeg_Ops.liesOn(ProjSThisStart,s)) {
		    distlist.add(ProjSThisStart.dist(this.startpoint));
		}//if
		
		Point ProjSThisEnd = Mathset.projectionPointLine(this.endpoint,s.startpoint,s.endpoint);
		if (PointSeg_Ops.liesOn(ProjSThisEnd,s)) {
		    distlist.add(ProjSThisEnd.dist(this.endpoint));
		}//if
		
		Point ProjThisSStart = Mathset.projectionPointLine(s.startpoint,this.startpoint,this.endpoint);
		if (PointSeg_Ops.liesOn(ProjThisSStart,this)) {
		    distlist.add(ProjThisSStart.dist(s.startpoint));
		}//if
		
		Point ProjThisSEnd = Mathset.projectionPointLine(s.endpoint,this.startpoint,this.endpoint);
		if (PointSeg_Ops.liesOn(ProjThisSEnd,this)) {
		    distlist.add(ProjThisSEnd.dist(s.endpoint));
		}//if
		
		if (distlist.size() > 0) {
		    //there is a valid solder that can be used for distance measurement
		    min = (Rational)distlist.get(0);
		    for (int i = 0; i < distlist.size(); i++) {
			if (((Rational)distlist.get(i)).less(min)) {
			    min = (Rational)distlist.get(i); }//if
		    }//for i
		}// if
		else {
		    //there is no valid solder
		    //use the point distances
		    		    distlist.add(s.startpoint.dist(this.startpoint));
		    distlist.add(s.startpoint.dist(this.endpoint));
		    distlist.add(s.endpoint.dist(this.startpoint));
		    distlist.add(s.endpoint.dist(this.endpoint));
		    min = (Rational)distlist.get(0);
		    for (int i = 0; i < distlist.size(); i++) {
			if (((Rational)distlist.get(i)).less(min)) {
			    min = (Rational)distlist.get(i); }//if
		    }//for i
		}//else
	    }//if 
	    return min.copy();
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method dist


    /**
     * Turns the segment.
     * A segment <tt>(a,b)</tt> is <tt>(b,a)</tt> after having turned it.
     *
     * @return the turned segment
     */
    public Segment turn() {
	Point help = this.startpoint;
	this.startpoint = this.endpoint;
	this.endpoint = help;
	return this;
    }//end method turn
    
    
    /**
     * Multiplies the coordinates of <i>this</i> with fact.
     *
     * @param fact the number which is multiplied with the coordinate values
     */
    public void zoom (Rational fact) {
	this.startpoint.zoom(fact);
	this.endpoint.zoom(fact);
	this.lengthDefined = false;
	this.bboxDefined = false;
    }//end method zoom


    /**
     * Aligns the segment.
     * To align a segments means to turn it in the case that the point stored as endpoint is smaller than the startpoint.
     */
    public void align () {
	byte res = this.startpoint.compX(this.endpoint);
	if (res == 0) res = this.startpoint.compY(this.endpoint);
	if (res == 1) this.turn();
    }//align

    
    /**
     * Converts the segment's data to a String.
     * Useful for pretty-printing.
     *
     * @return the segment data as String
     */
    public String toString() {
	return "segment ("+this.startpoint.x+"/"+this.startpoint.y+" "+this.endpoint.x+"/"+this.endpoint.y+")";
    }//end method toString


    /**
     * Removes from <i>this</i> all overlapping segment parts in <tt>inSet</tt>.<p>
     * The {@link twodsack.set.ElemMultiSet} <tt>inSet</tt> may only contain Segment instances which overlap <i>this</i> to guarantee a correct result. 
     * Then, all of the segments
     * contained in that set are subtracted from <i>this</i>.<p>
     * Example: For a point <tt>P</tt> on segment <tt>A</tt> (not an endpoint of <tt>A</tt>) <tt>B</tt> and <tt>C</tt> 
     * are the segments (<tt>A.start</tt>,<tt>P</tt>) and (<tt>P</tt>,<tt>A.end</tt>). Then, <i>this</i>.<tt>minus({B,C})</tt>
     * would return the empty set.<p>
     * As a side effect, the segments in <tt>inSet</tt> are aligned afterwards.
     *
     * @param inSet the set of overlapping segments
     * @return <i>this</i> minus <tt>inSet</tt> as SegMultiSet
     */
    public SegMultiSet minus (ElemMultiSet inSet) {
	SegMultiSet retSet = new SegMultiSet(SEGMENT_COMPARATOR);

	if (inSet == null || inSet.isEmpty()) {
	    retSet.add(this);
	    return retSet;
	}//if

	//The segments in inSet are already sorted.
	this.align();
	Iterator it = inSet.iterator();
	while (it.hasNext())
	    ((Segment)((MultiSetEntry)it.next()).value).align();

	it = inSet.iterator();
	Segment actSeg;
	Point actPoint = this.startpoint;

	while (it.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)it.next()).value;
	    
	    if (actSeg.startpoint.compare(actPoint) == 1)
		retSet.add(new Segment(actPoint,actSeg.startpoint));
	    actPoint = actSeg.endpoint;
	    if (actPoint.compare(this.endpoint) != -1) break;
	}//while
	if (actPoint.compare(this.endpoint) == -1)
	    retSet.add(new Segment(actPoint,this.endpoint));

	return retSet;
    }//end method minus


    /**
     * Returns <i>this</i> plus all segments of <tt>inSet</tt>.<p>
     * The {@link ElemMultiSet} <tt>inSet</tt> may only contain Segment instances which overlap <i>this</i> to guarantee a correct result.
     * Then, all the segments contained in <tt>inSet</tt> and <i>this</i> are joined to a single segment.<p>
     * The resulting set always contains only one single segment. However, when using a set as return type this method
     * is more compatible to the set operations in {@link twodsack.operation.setoperation.SetOps} and its implemented similar to {@link #minus(ElemMultiSet)}
     *
     * @param inSet the overlapping segments
     * @return the set with the joint segment
     */
    public SegMultiSet plus (ElemMultiSet inSet) {
	SegMultiSet retSet = new SegMultiSet(SEGMENT_COMPARATOR);
	Segment retSeg = (Segment)this;
	
	if (inSet == null || inSet.isEmpty()) {
	    retSet.add(retSeg);
	    return retSet;
	}//if
	
	//align THIS and inSet
	this.align();
	Iterator it = inSet.iterator();
	while (it.hasNext()) 
	    ((Segment)((MultiSetEntry)it.next()).value).align();

	//traverse inSet and adapt startpoint/endpoint for retSet
	it = inSet.iterator();
	Segment actSeg;
	Point minStart = retSeg.startpoint;
	Point maxEnd = retSeg.endpoint;
	while (it.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)it.next()).value;
	    if (actSeg.startpoint.compare(minStart) == -1) 
		minStart = actSeg.startpoint;
	    if (actSeg.endpoint.compare(maxEnd) == 1)
		maxEnd = actSeg.endpoint;
	}//while

	retSeg.set(minStart,maxEnd);
	retSet.add(retSeg);
	return retSet;
    }//end method plus


    /**
     * Returns the hashcode for <i>this</i>.
     * All of the point coordinates are added up.
     *
     * @return the hashcode as int
     */
    public int hashCode() {
	return (int)(this.startpoint.x.getDouble() + this.startpoint.y.getDouble() +
		     this.endpoint.x.getDouble() + this.endpoint.y.getDouble());
    }//end method hashCode


    /**
     * Returns <tt>true</tt>, if both objects are equal.
     *
     * @param o the object to compare with
     * @return <tt>true</tt>, if <i>this</i> and <tt>o</tt> area equal
     */
    public boolean equals(Object o) {
	//Neccessary to be used with hashing.
	return (this.equal((Element)o));
    }//end method equals


}//end class segment
