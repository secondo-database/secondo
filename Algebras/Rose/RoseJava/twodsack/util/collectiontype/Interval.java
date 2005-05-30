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
 * An instance of the Interval class represents a (y-) interval in the mathematical sense. So, it has an {@link #x} field and two borders,
 * {@link #left} and {@link #right}.
 * Additionally, it has some fields to store a {@link #mark} (as String), an element can be referenced using {@link #ref}, an interval
 * can have a {@link #number} and
 * finally, a flag {@link #buddyOnSameX} can be set.<p>
 * Whereas only {@link #x}, {@link #left} and {@link #right} are used for ordinary intervals, the other fields are used for the
 * storage of extra data for the DAC algorithm implemented in the <tt>overlappingPairs()</tt> method in
 * {@link twodsack.operation.setoperation.SetOps}.
 */
public class Interval {
    /*
     * fields
     */
    /**
     * The left interval border.
     */
    public Rational left;

    /**
     * The right interval border.
     */
    public Rational right;

    /**
     * A mark as String.
     */
    public String mark;

    /**
     * The <tt>x</tt> coordinate of the interval.
     */
    public Rational x;

    /**
     * The referenced object of type <tt>Element</tt>.
     */
    public Element ref;

    /**
     * An integer number for the interval.
     */
    public int number;

    /**
     * A flag that indicates whether the partner of the interval has the same x coordinate.
     */
    public boolean buddyOnSameX;    

    /*
     * constructors
     */
    /**
     * The 'empty' constructor
     */
    public Interval(){};


    /**
     * Constructs a new instance with the passed paramters
     *
     * @param l left border
     * @param r right border
     * @param m the mark
     * @param x <tt>x</tt> value of interval
     * @param e referenced element
     * @param n interval number
     * @param buddy sets <tt>buddyOnSameX</tt>
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
     * Returns <tt>true</tt>, if intervals are equal.
     * Here, only the <tt>left</tt> and <tt>right</tt> fields are checked. If they are equal, <tt>true</tt> is returned. <tt>false</tt> otherwise.
     *
     * @param in the interval to compare with
     * @return <tt>true</tt>, if equal, <tt>false</tt> otherwise
     */
    public boolean equal(Interval in) {
	if ((this.left.equal(in.left)) &&
	    (this.right.equal(in.right))) {
	    return true;
	}//if
	else { return false; }
    }//end method equal

    
    /**
     * Returns <tt>true</tt>, if intervals are equal.
     * Return <tt>true</tt>, if left and right borders are equal AND if their x-coordinate is equal.
     *
     * @param in the interval to compare with
     * @return <tt>true</tt>, if equal, <tt>false</tt> otherwise
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
     * Compares the fields <tt>x</tt>, <tt>left</tt>, <tt>right</tt>, <tt>number</tt> in that order.<p>
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
     * Compares the fields <tt>left</tt>, <tt>right</tt>, <tt>number</tt> in that order.<p>
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
