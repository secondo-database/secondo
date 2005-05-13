/*
 * Interval.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;

import twodsack.setelement.*;
import twodsack.util.number.*;

/**
 * An instance of the Interval class represents a (y-) interval in the mathematical sense. So, it has an x field and two borders, left and right.
 * Additionally, it has some fields to store a mark (as String), an element can be references using ref, an interval can have a number and
 * finally, a flag buddyOnSameX can be set.<p>
 * Whereas only x, left and right are used for ordinary intervals, the other fields are used for the storage of extra data for the DAC
 * algorithm implemented in SetOps.overlappingPairs().
 */
public class Interval {
    /*
     * fields
     */
    public Rational left;
    public Rational right;
    public String mark;
    public Rational x;
    public Element ref;
    public int number;
    public boolean buddyOnSameX;    

    /*
     * constructors
     */
    /**
     * Constructs a new instance with the passed paramters
     *
     * @param l left border
     * @param r right border
     * @param m the mark
     * @param x x value of interval
     * @param e referenced element
     * @param n interval number
     * @param buddy sets buddyOnSameX
     */
    public Interval(Rational l, Rational r, String m, Rational x, Element e, int n, boolean buddy) {
	this.left = l;
	this.right = r;
	this.mark = m;
	this.x = x;
	this.ref = e;
	this.number = n;
	this.buddyOnSameX = buddy;
    }//end constructor
    

    /**
     * Returns true, if intervals are equal.
     * Here, only the left and right fields are checked. If they are equal, true is returned. False otherwise.
     *
     * @param in the interval to compare with
     * @return true, if equal, false otherwise
     */
    public boolean equal(Interval in) {
	if ((this.left.equal(in.left)) &&
	    (this.right.equal(in.right))) {
	    return true;
	}//if
	else { return false; }
    }//end method equal

    
    /**
     * Rturns true, if intervals are equal.
     * Return true, if left and right borders are equal AND if their x-coordinate is equal.
     *
     * @param in the interval to compare with
     * @return true, if equal, false otherwise
     */
    public boolean equalX(Interval in) {
	if (this.left.equal(in.left) &&
	    this.right.equal(in.right) &&
	    this.x.equal(in.x)) return true;
	else return false;
    }//end method equalX    


    /**
     * Prints the interval's data to standard output.
     */
    public void print() {
	System.out.print("Interval:");
	System.out.print(" down/left: "+this.left.toString());
	System.out.print(", top/right: "+this.right.toString());
	System.out.print(", mark: "+this.mark);
	System.out.print(", x: "+this.x.toString());
	System.out.print(", buddy: "+this.buddyOnSameX);
	System.out.println(", number: "+this.number);
    }//end method print
  

    /**
     * Compares <i>this</i> and <i>in</i> and returns one of {0, 1, -1}. 
     * Compares the fields x, left, right, number in that order.<p>
     * Returns 0, if both intervals are equal.
     * Returns -1, if <i>this</i> is smaller than <i>in</i>.<p>
     * Returns 1 otherwise.
     * 
     * @param in the interval to compare with
     * @return one of {0, 1, -1} as byte
     */
    public byte comp(Interval in) {
	byte res = this.x.comp(in.x);
	if (res != 0) return res;

	res = this.left.comp(in.left);
	if (res != 0) return res;

	res = this.right.comp(in.right);
	if (res != 0) return res;

	if (this.number < in.number) return -1;
	else return 1;
    }//end method comp


    /**
     * Compares <i>this</i> and <i>in</i> and returns one of {0, 1, -1}.
     * Compares the fields left, right number in that order.<p>
     * Returns 0, if both intervals are equal.<p>
     * Returns -1, if <i>this</i> is smaller than <i>in</i>.<p>
     * Returns 1 otherwise
     *
     * @param in the interval to compare with
     * @return one of {0, 1, -1} as byte
     */
    public byte compY (Interval in) {
	byte res = this.left.comp(in.left);
	if (res != 0) return res;
	
	res = this.right.comp(in.right);
	if (res != 0) return res;
	
	if (this.number < in.number) return -1;
	else return 1;
    }//end method compY

}//end class Interval
