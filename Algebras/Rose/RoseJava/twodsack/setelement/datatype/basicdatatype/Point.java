/*
 * Point.java 2005-05-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.basicdatatype;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.util.*;
import twodsack.util.number.*;
import java.io.*;


/**
 * The Point class implements one of the three basic datatypes, a point. As fields it has to coordinate values of type
 * {@link Rational}, a bounding box of type {@link Rect} and a flag, whether the bounding box is already defined or not.<p>
 * The fields of this class are all public, because inside of the 2D-SACK package the access to these values is needed
 * very often. However, to change the fields' values, the <tt>set()</tt> method(s) should be used.<p>
 * The bounding box for a Point instance is not constructed automatically, when a constructor is called. It is not constructed
 * until the <tt>rect()</tt> method is called, but then, the bounding box is stored in the object's <tt>bbox</tt> field and only then the
 * <tt>bboxDefined</tt> flag is set.
 */
public class Point extends Element implements Serializable {
    
    /*
     * fields
     */
    static final double DERIV_DOUBLE = RationalFactory.readDerivDouble();
    static final double DERIV_DOUBLE_NEG = RationalFactory.readDerivDoubleNeg();
    static boolean PRECISE;
    static boolean preciseDefined;

    /**
     * The x coordinate.
     */
    public Rational x; //x coordinate

    /**
     * The y coordinate.
     */
    public Rational y; //y coordinate

    /**
     * The bounding box of the point.
     */
    public Rect bbox;

    /**
     * Defines whether the bounding box is actually defined.
     */
    public boolean bboxDefined;
    
    
    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     * The coordinate values are set to NULL.
     */
    public Point() {
	//fill in dummy values
	this.x = null;
	this.y = null;
	this.bboxDefined = false;
    }
    

    /**
     * Constructs a new Point instance from two double values.
     * The double values are used to construct instances of {@link Rational}.
     *
     * @param x the x coordinate
     * @param y the y coordinate
     */
    public Point(double x, double y) {
	this.x = RationalFactory.constRational(x);
	this.y = RationalFactory.constRational(y);
	this.bboxDefined = false;
    }
    

    /**
     * Constructs a new Point instance from two Rational objects.
     *
     * @param x the x coordinate
     * @param y the y coordinate
     */
    public Point(Rational x, Rational y) {
	this.x = x;
	this.y = y;
	this.bboxDefined = false;
    }
    
    /*
     * methods
     */
    /**
     * Sets the coordinates of <i>this</i> to <tt>x</tt>, <tt>y</tt>.
     * Returns the modified point. Sets <tt>bboxDefined = false</tt>.
     *
     * @param x the <tt>x</tt> coordinate
     * @param y the <tt>y</tt> coordinate
     * @return the modified point
     */
    public Point set(double x, double y) {
	this.x = RationalFactory.constRational(x);
	this.y = RationalFactory.constRational(y);
	this.bboxDefined = false;

	return this;
    }//end method set
    

    /**
     * Sets the coordinates of <i>this</i> to <tt>p.x</tt> and <tt>p.y</tt>.
     * Returns the modified point. Sets <tt>bboxDefined = false</tt>.
     *
     * @param p the point with the new coordinates
     * @return the modified point
     */
    public Point set(Point p) {
	this.x = p.x;
	this.y = p.y;
	this.bboxDefined = false;

	return this;
    }//end method set


    /**
     * Sets the coordinates of <i>this</i> to <tt>x</tt>, <tt>y</tt>.
     * Returns the modified point. Sets <tt>bboxDefined = false</tt>;
     * 
     * @param x the <tt>x</tt> coordinate
     * @param y the <tt>y</tt> coordinate
     * @return the modified point
     */
    public Point set(Rational x, Rational y) {
	this.x = x;
	this.y = y;
	this.bboxDefined = false;

	return this;
    }//end method set
    

    /**
     * Returns a <i>deep</i> copy of this.
     * This means, that any changes in the copy doesn't affect the original point.
     *
     * @return the copy
     */
    public Element copy(){
	Point copy = new Point(this.x,this.y);
	copy.bboxDefined = this.bboxDefined;
	copy.bbox = this.bbox;

	return copy;
    }//end method copy
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> and <tt>e</tt> are equal.
     * Throws a WrongTypeException if <tt>e</tt> is not of type Point.
     *
     * @param e the point to compare with
     * @return <tt>true</tt>, if both objects are of type Point and have the same coordinates
     * @throws WrongTypeException if <tt>e</tt> is not of type Point
     */
    public boolean equal(Element e) throws WrongTypeException {
	if (e instanceof Point) {
	    Point p = (Point)e;
	    
	    if (!this.preciseDefined) {
		this.PRECISE = RationalFactory.readPrecise();
		this.preciseDefined = true;
	    }//if
	    
	    if (this.PRECISE) {
	    //PRECISE == true
		if ((this.x.equal(p.x)) && (this.y.equal(p.y))) { return true; }
		else { return false; }
		}//if precise
	    else {
		//PRECISE == false
		double tx = this.x.getDouble();
		double ty = this.y.getDouble();
		double px = p.x.getDouble();
		double py = p.y.getDouble();
		
		/*
		double diffx = tx - px;
		double diffy = ty - py;
		
		if ((diffx < DERIV_DOUBLE && diffx > DERIV_DOUBLE_NEG) &&
		    (diffy < DERIV_DOUBLE && diffy > DERIV_DOUBLE_NEG))
		    return true;
		else return false;
		*/
		double diffx = Math.abs(tx-px);
		double diffy = Math.abs(ty-py);
		if (diffx < DERIV_DOUBLE && diffy < DERIV_DOUBLE)
		    return true;
		else 
		    return false;
	    }//else
	    
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method equal
    

    /**
     * Compares the <tt>y</tt> coordinates of both objects and returns one of {0, 1, -1}.<p>
     * Returns 0, if the <tt>y</tt> coordinates are equal.<p>
     * Returns 1, if <tt>e</tt> has the smaller <tt>y</tt> coordinate.<p>
     * Returns -1 otherwise.
     * 
     * @param e the object to compare with
     * @return {0, 1, -1} as int
     * @throws WrongTypeException if <tt>e</tt> is not of type Point
     */
    public byte compY(Element e) throws WrongTypeException {
	if (e instanceof Point) {
	    Point p = (Point)e;
	    if (!this.preciseDefined) {
		this.PRECISE = RationalFactory.readPrecise();
		this.preciseDefined = true;
	    }//if
	    if (this.PRECISE)
		return this.y.comp(p.y);
	    else {
		double thisy = this.y.getDouble();
		double py = p.y.getDouble();
		if (thisy > py) return 1;
		if (thisy < py) return -1;
		return 0;
	    }//else
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compY
    

    /**
     * Compares the <tt>x</tt> coordinates of both objects and returns one of {0, 1, -1}.<p>
     * Returns 0, if the <tt>x</tt> coordinates are equal.<p>
     * Returns 1, if <tt>e</tt> has the smaller <tt>x</tt> coordinate.<p>
     * Returns -1 otherwise.
     *
     * @param e the object to compare with
     * @return {0, 1, -1} as byte
     * @throws WrongTypeException if <tt>e</tt> is not of type Point
     */
    public byte compX(Element e) throws WrongTypeException {
	if (e instanceof Point) {
	    Point p = (Point)e;
	    if (!this.preciseDefined) {
		this.PRECISE = RationalFactory.readPrecise();
		this.preciseDefined = true;
	    }//if
	    if (this.PRECISE) 
		return this.x.comp(p.x);
	    else {
		double thisx = this.x.getDouble();
		double px = p.x.getDouble();
		if (thisx > px) return 1;
		if (thisx < px) return -1;
		return 0;
	    }//else
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compX
    
    
    /**
     * First, compares the x coordinates and then the y coordinates of both objects. Returns one of {0, 1, -1}.
     * If the comparison of the <tt>x</tt> coordinate results in 0, the <tt>y</tt> coordinates are checked. If both results are 0,
     * <tt>this.equal(e) == true</tt>.<p>
     * A point is considered as 'smaller', if it has smaller <tt>x</tt> coordinates and/or <tt>y</tt> coordinates than the other point.
     *
     * @param e the object to compare with
     * @return {0, 1, -1} as int
     * @throws WrongTypeException if <tt>e</tt> is not of type Point
     */
    public int compare(ComparableMSE e) throws WrongTypeException {
	if (e instanceof Point) {
	    Point p = (Point)e;
	    int res = this.x.comp(p.x);
	    if (res == 0) res = this.y.comp(p.y);
	    return res;	
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compare


    /**
     * Computes and sets the bounding box for <i>this</i> and sets the <tt>bboxDefined</tt> flag.
     *
     * @return the bounding box
     */
    private Rect computeBbox() {
	this.bbox = new Rect(this.x,this.y,this.x,this.y);
	this.bboxDefined = true;
	return this.bbox;
    }//end computeBbox
    
    
    /**
     * Prints the Point's data to the standard output.
     */
    public void print() {
	System.out.println("Point: ("+this.x.toString()+", "+this.y.toString()+")");
    }//end method print
    

    /**
     * Returns <tt>true</tt>, if both objects have common points.
     * In this special case, <tt>true</tt> is returned if both objects are equal.
     *
     * @param e the object which is checked for intersection
     * @return <tt>true</tt>, if both Point objects are equal
     * @throws WrongTypeException if <tt>e</tt> is not of type Point
     */
    public boolean intersects(Element e) throws WrongTypeException {
	if (e instanceof Point) {
	    Point p = (Point)e;
	    if ((this.x.equal(p.x)) && (this.y.equal(p.y))) { return true; }
	    else { return false; }
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method intersects
    

    /**
     * Returns the bounding box of <i>this</i>.
     * If the bounding box was not already computed, that is done at this occasion. The <tt>bboxFlag</tt> is set, too.
     *
     * @return the bounding box
     */
    public Rect rect() {
	if (bboxDefined) return this.bbox;
	else {
	    computeBbox();
	    return this.bbox;
	}//else
    }//end method rect


    /**
     * The Euclidean distance between <i>this</i> and e is returned.
     * Note, that the computation of the distance may be unprecise due to a neccessary double/Rational conversion. <p>
     * If both objects are equal, the result is 0.
     * 
     * @param e the 'distant' object
     * @return the distance as Rational
     */
    public Rational dist(Element e) throws WrongTypeException {
	if (e instanceof Point) {
	    Point p = (Point)e;
	    return RationalFactory.constRational(Mathset.lengthD(Mathset.diff(this,p)));
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method dist
    

    /**
     * Multiplies the coordinates of <i>this</i> with fact.
     *
     * @param fact the number which is mulitplied with the coordinate values
     */
    public void zoom (Rational fact) {
	this.x = this.x.times(fact);
	this.y = this.y.times(fact);
	this.bboxDefined = false;
    }//end method zoom
    

    /**
     * For two points, returns the smaller one.
     * {@link #compare(ComparableMSE)} is used in the implementation of this method.
     *
     * @param p1 the first point
     * @param p2 the second point
     * @return the 'smaller' point
     */
    public static Point min (Point p1, Point p2) {
	if (p1.compare(p2) <= 0) return p1;
	else return p2;
    }//end method min


    /**
     * For two points, returns the greater one.
     * {@link #compare(ComparableMSE)} is used in the implementation of this method.
     */
    public static Point max (Point p1, Point p2) {
	//System.out.println("max: p1.compare(p2): "+p1.compare(p2));
	if (p1.compare(p2) > 0) return p1;
	else return p2;
    }//end method max


    /**
     * Returns the hashcode for <i>this</i>.
     * The code is computed as the sum of its coordinates.
     *
     * @return the hashcode as int
     */
    public int hashCode() {
	return (int)(this.x.getDouble() + this.y.getDouble());
    }//end method hashCode


    /**
     * Returns <tt>true</tt>, if both objects are equal.
     *
     * @param o the object to compare with
     * @return true, if <i>this</i> and o are equal
     */
    public boolean equals(Object o) {
	return (this.equal((Element)o));
    }//end method equals
    

    /**
     * Converts the Point object data to a string.
     * Useful for pretty-printing.
     *
     * @return the point data as string
     */
    public String toString() {
	return new String("Point("+this.x+"/"+this.y+")");
    }//end method toString

}//end class Point
