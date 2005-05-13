/*
 * PSPoint.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;

import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;

/**
 * PSPoint stands for plane sweep point and is used to store a point together with some information about it. Hence, it has two int fields
 * to store array indices, one field to store the point itself and to boolean flags to tell, whether the point is a segment's startpoint or
 * intersection point.
 */
public class PSPoint implements ComparableMSE {
    /*
     * fields
     */
    public Point point;
    public int number; //mostly used for storing an index of an arry 
    public int number2; //dito
    public boolean isStartpoint; //isStartpoint of segment
    public boolean isIntPoint; //is intersection point


    /*
     * constructors
     */
    /**
     * Constructs a new PSPoint instance with the passed parameters.
     *
     * @param p the poin
     * @param num the first array index
     * @param num2 the second array index
     * @param m isStartpoint
     * @param i isIntersectionPoint
     */
    public PSPoint(Point p, int num, int num2, boolean m, boolean i) {
	this.point = p;
	this.number = num;
	this.number2 = num2;
	this.isStartpoint = m;
	this.isIntPoint = i;
    }


    /*
     * methods
     */
    /**
     * Compares two PSPoints and returns one of {0, -1, 1}.
     * Compares the fields p.x, p.y, isStartpoint, isIntPoint, number in that order.<p>
     * Returns 0, if both objects are equal.<p>
     * Returns -1, if <i>this</i> is smaller than <i>e</i>.<p>
     * Return 1 otherwise.
     *
     * @param e the object to compare with
     * @throws WrongTypeException if e is not of type PSPoint
     */
    public int compare(ComparableMSE e) throws WrongTypeException {
	//markTSet and inside are NOT used to define a complete order
	if (e instanceof PSPoint) {
	    PSPoint p = (PSPoint)e;
	    int res = this.point.x.comp(p.point.x);
	    if (res == 0) res = this.point.y.comp(p.point.y);
	    if (res != 0) return res;
	    if (res == 0) {
		if (this.isStartpoint && !p.isStartpoint) return -1;
		else if (!this.isStartpoint && p.isStartpoint) return  1; }
	    if (res == 0) {
		if (this.isIntPoint && !p.isIntPoint) return -1;
		else if (!this.isIntPoint && p.isIntPoint) return 1; }
	    if (res == 0) {
		if (this.number < p.number) return -1;
		else return 1; }
	    
	    System.out.println("uncaught Case in PSPoint.compare.");
	    this.print();
	    p.print();
	    System.exit(0);
	    return 0;
	}
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compare


    /**
     * Prints the data of <i>this</i> to standard output.
     */
    public void print () {
	System.out.println("PSPoint: ("+this.point.x+"/"+this.point.y+"), number: "+this.number+", mark: "+isStartpoint+", intpoint: "+isIntPoint);
    }//end method print


    /**
     * Converts the data of <i>this</i> to a String.
     *
     * @return the data as String
     */
    public String toString() {
	return new String("PSPoint: ("+this.point.x+"/"+this.point.y+"), number: "+this.number+", mark: "+isStartpoint+", intpoint: "+isIntPoint);
    }//end method toString
    
}//end class PSPoint
