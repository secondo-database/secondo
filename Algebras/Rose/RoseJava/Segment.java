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
import java.io.*;

class Segment extends Element implements Serializable {
    
    //members
    protected Point startpoint; 
    protected Point endpoint; 
    //private double length;
    //private Rect bbox; //bounding box
    
    //constructors
    public Segment() {
	//fill in dummy values
	Point p1 = new Point(0,1);
	Point p2 = new Point(1,0);
	this.startpoint = p1;
	this.endpoint = p2;
	//this.length = compute_length();
	//computeBbox();
    }
    
    public Segment(double x1, double y1, double x2, double y2) {
	this.startpoint = new Point(x1,y1);
	this.endpoint = new Point(x2,y2);
	//this.align();
	//this.length = compute_length(); 
	if (!isSegment()) {
	    System.out.println("Error: Tried to build bad Segment: ("+x1+", "+y1+") - ("+x2+", "+y2+")");
	    System.exit(0);
	}//if
	//computeBbox();
    }
    
    public Segment(Point p1, Point p2) {
	this.startpoint = (Point)p1.copy();
	this.endpoint = (Point)p2.copy();
	//this.align();
	//this.length = compute_length(); 
	if (!isSegment()) {
	    System.out.println("Error: Tried to build bad Segment!");
	    p1.print();
	    p2.print();
	    System.exit(0);
	}//if
	//computeBbox();
    }
    
    public Segment(Rational x1, Rational y1, Rational x2, Rational y2) {
	this.startpoint = new Point(x1,y1);
	this.endpoint = new Point(x2,y2);
	//this.align();
	//this.length = compute_length(); 
	if (!isSegment()) {
	    System.out.println("Error: Tried to build bad Segment!");
	    System.out.println("("+x1+", "+y1+") - ("+x2+", "+y2+")");
	    System.exit(0);
	}//if
	//computeBbox();
    }
    
    //methods
    private boolean isSegment() {
	//returns true if this.length is > 0
	//if (this.length.greater(0)) { return true; }
	if (!this.startpoint.equal(this.endpoint)) { return true; }
	else { return false; }
    }//end method isSegment()
    
    
    private double computeLength(){
	//computes the length of the segment
	//caution: this is not computed precisely
	double l = Mathset.lengthD(Mathset.diff(endpoint,startpoint));
	return l;
    }//end method computeLength
    
    
    public Segment setStartpoint(Point p){
	//sets the segments startpoint to p
	this.startpoint = (Point)p.copy();
	//this.length = compute_length();
	return this;
    }//end method setStartpoint
    
    
    public Segment setEndpoint(Point p){
	//sets the segments endpoint to p
	this.endpoint = (Point)p.copy();
	//this.length = compute_length();
	return this;
    }//end method setEndpoint
    

    public double length(){
	//returns the segments length
	return this.computeLength();
    }//end method get_length
    
    
    public Point getStartpoint(){
	//returns the segments startpoint
	return this.startpoint;
    }//end method getStartpoint
    
    
    public Point getEndpoint(){
	//returns the segments endpoint
	return this.endpoint;
    }//end method get_endpoint


    public PointList endpoints(){
	//returns the endpoints of a segments as a set
	PointList pl = new PointList();
	pl.add(this.startpoint);
	pl.add(this.endpoint);
	return pl;
    }//end method endpoints
    
    
    public Segment set(Point s, Point e){
	//sets startpoint and endpoint to s and e
	this.startpoint = (Point)s.copy();
	this.endpoint = (Point)e.copy();
	//this.length = compute_length();
	//computeBbox();
	return this;
    }//end method set
    
    public Element copy(){
	//returns a copy of the segment
	Segment copy = new Segment(this.startpoint.x,this.startpoint.y,
				   this.endpoint.x,this.endpoint.y);
	//copy.startpoint = (Point)this.startpoint.copy();
	//copy.endpoint = (Point)this.endpoint.copy();
	//copy.length = this.length;
	//copy.computeBbox();
	return copy;
    }//end method copy
    
    protected Point theOtherOne(Point p){
	//returns that Point of this.Segment which is not equal to p
	if (this.startpoint.equal(p)) { return this.endpoint; }
	else { return this.startpoint; }
    }//end method theOtherOne
    

    public boolean equal(Element segin) throws WrongTypeException {
	//returns true, if this.segment is equal to sin
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
    

    public void update() {
	//updates this.length
	//this.length = compute_length();
	//computeBbox();
    }//end method update
    
    public byte compX(Element sin) throws WrongTypeException {
	//compares the x-coordinates of the leftmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller x-coordinate
	//0, if the x-coordinates are equal
	//+1, if THIS.object has a greater x-coordinate
	//if the x coordinates of the leftmost points are equal
	//then sort by gradient
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
    
    public byte compY(Element sin) throws WrongTypeException {
	//compares the y-coordinates of the upmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller y-coordinate
	//0, if the y-coordinates are equal
	//+1, if THIS.object has a greater y-coordinate
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

    
    public byte compare (Element sin) throws WrongTypeException {
	//...
	//expects that both segments are aligned
	if (sin instanceof Segment) {
	    Segment seg = (Segment)sin;
	    byte res = this.startpoint.compare(seg.startpoint);
	    if (res == 0) res = this.endpoint.compare(seg.endpoint);
	    return res;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+sin.getClass()); }
    }//compare

    
    public Rect computeBbox() {
	//computes and sets bbox
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
	return bbox;
    }//end method computeBbox
    
    
    public void print(){
	//prints the object's data
	System.out.println("Segment:");
	System.out.println(" startpoint: ("+startpoint.x.toString()+", "+startpoint.y.toString()+")");
	System.out.println(" endpoint: ("+endpoint.x.toString()+", "+endpoint.y.toString()+")");
    }//end method print
    
    
    public boolean intersects (Element e) {
	//returns true if this and e have common points
	if (e instanceof Segment) {
	    Segment s;// = new Segment();
	    s = (Segment)e;
	    if (SegSeg_Ops.pointsInCommon(this,s)) {
		return true;
	    }//if
	}//if
	else {
	    System.out.println("Error(class Segment): Wrong type.");
	    System.exit(0);
	}//else
	return false;
    }//end method intersects
    
    /*
    public boolean pintersects(Element e) throws WrongTypeException {
	Point retPoint = new Point();
	if (e instanceof Segment) {
	    Segment inseg = (Segment)e;
	    if (this.equal(e)) { return false; }
	    if (Mathset.linearly_dependent(this,inseg)) { return false; }
	    else {
		//convert both segments to parametric form
		Point p1;
		Point t1;
		Point p2;
		Point t2;
		Rational r;
		Rational s;
		
		p1 = this.startpoint;
		t1 = Mathset.diff(this.endpoint,this.startpoint);
		p2 = inseg.startpoint;
		t2 = Mathset.diff(inseg.endpoint,inseg.startpoint);
		
		if ((t2.y.equal(0) && t1.y.equal(0)) ||
		    ((!(t2.y.equal(0)) && (t1.x.minus(t1.y.times(t2.x.dividedby(t2.y)))).equal(0)) &&
		     (!(t1.y.equal(0)) && (t2.x.times(-1)).minus(t2.y.times(t1.x.dividedby(t1.y))).equal(0))))
		    {
			System.out.println("ERROR! Can't deny division by zero! (Segment.class)");
			System.exit(0);
		    }//if
		
		//compute r
		if (!(t2.y.equal(0)) && !(t1.x.minus(t1.y.times(t2.x.dividedby(t2.y))).equal(0))) {
		    r = ((p2.x.minus(p1.x)).minus((p2.y.minus(p1.y)).times(t2.x.dividedby(t2.y)))).dividedby(t1.x.minus(t1.y.times(t2.x.dividedby(t2.y))));
		    retPoint.x = p1.x.plus(r.times(t1.x));
		    retPoint.y = p1.y.plus(r.times(t1.y));
		}//if
		else {
		    //compute s
		    s = ((p2.x.minus(p1.x)).minus((p2.y.minus(p1.y)).times(t1.x.dividedby(t1.y)))).dividedby(((t2.x.times(-1)).minus(t2.y.times(t1.x.dividedby(t1.y)))));
		    retPoint.x = p2.x.plus(s.times(t2.x));
		    retPoint.y = p2.y.plus(s.times(t2.y));
		}//else   
	    }//if	
	    //System.out.println("intersection point:"); retPoint.print();

	    //now check whether retPoint lies on both segments
	    if (PointSeg_Ops.isEndpoint(retPoint,this) ||
		PointSeg_Ops.isEndpoint(retPoint,inseg)) {
		//System.out.println("case1");
		return false; }
	    if (PointSeg_Ops.liesOn(retPoint,this) &&
		PointSeg_Ops.liesOn(retPoint,inseg)) {
		//System.out.println("case2");
		return true; }
	    else { return false; }
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method pintersects	
    */


    
    public boolean pintersects(Element e) {
	//returns true if this.object and e intersect
	//note: if this.object and e are linearly dependent they do not intersect
	//note: returns FALSE if one of the segment's endpoints lie on the other segment
	
	//if (gradient(s1) == gradient(s2)) { return false; }
	//System.out.println();
	//System.out.println("entering pintersects..:");
	//System.out.println("segment1:"); this.print();
	//System.out.println("segment2:"); e.print();
	
	
	if (e instanceof Segment) {
	    //Segment s = new Segment();
	    Segment s = (Segment)e;
	    if (PointSeg_Ops.liesOn(this.startpoint,s) ||
		PointSeg_Ops.liesOn(this.endpoint,s) ||
		PointSeg_Ops.liesOn(s.startpoint,this) ||
		PointSeg_Ops.liesOn(s.endpoint,this)) {
		return false;
	    }//if

	    if (SegSeg_Ops.formALine(this,s) || SegSeg_Ops.formASegment(this,s)) { return false; }
	    
	    //if this.object's endpoints lie on different sides of s and
	    //s's endpoints lie on different sides of this.object then return true
	    int thiss;//this.object startpoint position regarding s
	    int thise;//this.object endpoint position regarding s
	    int ss;//dito
	    int se;//dito
	    
	    thiss = Mathset.pointPosition(s.startpoint,s.endpoint,this.startpoint);
	    thise = Mathset.pointPosition(s.startpoint,s.endpoint,this.endpoint);
	    ss = Mathset.pointPosition(this.startpoint,this.endpoint,s.startpoint);
	    se = Mathset.pointPosition(this.startpoint,this.endpoint,s.endpoint);

	    //System.out.println("thiss:"+thiss+", thise:"+thise+", ss:"+ss+", se:"+se);
	    
	    if ((((thiss == 1) && (thise == -1)) || ((thiss == -1) && (thise == 1))) &&
		(((ss == 1) && (se == -1)) || ((ss == -1) && (se == 1)))) {
		//System.out.println("leaving S.pintersects:true");
		return true;
	    }//if
	    else {
		//System.out.println("leaving S.pintersects:false");
		return false;
	    }//else
	    
	}//if
	else {
	    System.out.println("Error(class Segment): Wrong type.");
	    System.exit(0);
	    return false;
	}//else
    }//end method pintersects
    

    public Point intersection(Segment inseg) throws SegmentsDontIntersectException {
	//returns the intersection point of this and inseg 
	Point retPoint = new Point();
	
	if (this.intersects(inseg)) {
	    //convert both segments to parametric form
	    Point p1;// = new Point();
	    Point t1;// = new Point();
	    Point p2;// = new Point();
	    Point t2;// = new Point();
	    Rational r;// = new Rational(0);//parameter
	    Rational s;// = new Rational(0);//parameter
	    
	    //System.out.println();
	    //System.out.println("thisseg: "); this.print();
	    //System.out.println("inseg: "); inseg.print();
	    //p1 = (Point)this.startpoint.copy(); //System.out.print("p1: ");p1.print();
	    p1 = this.startpoint;
	    t1 = Mathset.diff(this.endpoint,this.startpoint); //System.out.print("t1: ");t1.print();
	    //p2 = (Point)inseg.startpoint.copy(); //System.out.print("p2: ");p2.print();
	    p2 = inseg.startpoint;
	    t2 = Mathset.diff(inseg.endpoint,inseg.startpoint); //System.out.print("t2: ");t2.print();
	    
	    //System.out.println("p1: "); p1.print();
	    //System.out.println("t1: "); t1.print();
	    //System.out.println("p2: "); p2.print();
	    //System.out.println("t2: "); t2.print();

	    boolean t1yEQ0 = t1.y.equal(0);
	    boolean t2yEQ0 = t2.y.equal(0);

	    if ((t1yEQ0 && t2yEQ0) ||
		((!(t2yEQ0) && (t1.x.minus(t1.y.times(t2.x.dividedby(t2.y)))).equal(0)) &&
		 (!(t1yEQ0) && (t2.x.times(-1)).minus(t2.y.times(t1.x.dividedby(t1.y))).equal(0))))
		{
		    System.out.println("Segment.intersection: Can't deny division by zero!");
		    System.out.println("Segments:");
		    this.print();
		    inseg.print();
		    System.out.println("intersects(s1,s2) = "+this.pintersects(inseg));
		    System.exit(0);
		}//if
	    
	    //compute r
	    if (!(t2yEQ0) && !(t1.x.minus(t1.y.times(t2.x.dividedby(t2.y))).equal(0))) {
		r = ((p2.x.minus(p1.x)).minus((p2.y.minus(p1.y)).times(t2.x.dividedby(t2.y)))).dividedby(t1.x.minus(t1.y.times(t2.x.dividedby(t2.y))));
		//System.out.println("r: "+r);
		retPoint.x = p1.x.plus(r.times(t1.x));
		retPoint.y = p1.y.plus(r.times(t1.y));
	    }//if
	    else {
		//compute s
		s = ((p2.x.minus(p1.x)).minus((p2.y.minus(p1.y)).times(t1.x.dividedby(t1.y)))).dividedby(((t2.x.times(-1)).minus(t2.y.times(t1.x.dividedby(t1.y)))));
		//System.out.println("s: "+s);
		retPoint.x = p2.x.plus(s.times(t2.x));
		retPoint.y = p2.y.plus(s.times(t2.y));
	    }//else   
	}//if
	else {
	    this.print();
	    inseg.print();
	    throw new SegmentsDontIntersectException("Segment.intersection: Segments don't intersect.");
	}//else
	return retPoint;
    }//end method intersection


    public Rational gradient() throws InfiniteGradientException{
	//returns the gradient of this.object
	Rational r;// = new Rational(0);
	if (!((this.startpoint.x.minus(this.endpoint.x)).equal(0))) {
	    r = ((this.startpoint.y.minus(this.endpoint.y)).dividedby((this.startpoint.x.minus(this.endpoint.x)))); }//if
	else {
	    //deny division by zero
	    throw new InfiniteGradientException();
	}//else
	return r;
    }//end method gradient

    
    public Rect rect() {
	//returns the bounding box of this.object
	return this.computeBbox();
    }//end method bbox
    

    public Rational dist(Element e) throws WrongTypeException {
	//returns the distance between this.object and e
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


    public Segment turn() {
	//turns this
	Point help = this.startpoint;
	this.startpoint = this.endpoint;
	this.endpoint = help;
	return this;
    }//end method turn
    
    
    protected void zoom (Rational fact) {
	//multiplies this.coordinates with fact
	this.startpoint.zoom(fact);
	this.endpoint.zoom(fact);
	this.update();
    }//end method zoom


    public void align () {
	//aligns this
	byte res = this.startpoint.compX(this.endpoint);
	if (res == 0) res = this.startpoint.compY(this.endpoint);
	if (res == 1) this.turn();
    }//align

}//end class segment
