/*
 * LineComparator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import twodsack.util.number.*;
import java.util.*;
import java.io.*;


/**
 * A LineComparator is used to sort ojects of type Line and is usually passed to a constructor of a collection which has to be sorted using
 * the compare() method of this class.
 */
public class LineComparator implements Comparator,Serializable{
    /*
     * fields
     */
    public Rational x;
    public Rational yValue;
    public Line line;
    static Rational bp = RationalFactory.constRational(0);
    static Rational bt = RationalFactory.constRational(0);
    static Rational b1 = RationalFactory.constRational(0);
    static Rational b2 = RationalFactory.constRational(0);


    /*
     * constructors
     */
    /**
     * Cosntructs an 'empty' LineComparator.
     * The x,yValue and line fields are set to NULL.
     */
    public LineComparator() {
	this.x = null;
	this.yValue = null;
	this.line = null;
    }
	

    /**
     * Compares two objects and returns one of {0, 1, -1}.
     * Both objects must be of type Line. Then, the intersection point of both lines with the y axis is computed (if any).<p>
     * Return 0, if both intersection points are equal.<p>
     * Return -1, if the intersection point of ino1 and the y axis is smaller than the intersection point of ino2 and the y axis.<p>
     * Return 1 otherwise
     * Note, that the y axis is not really considered to lie at position x = 0, but depends on the x coordinate in the classes x field.
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WronTypeException if ino1 or ino2 is not of type Line
     */
    public int compare(Object ino1, Object ino2)  throws WrongTypeException {
	if(ino1 instanceof Line &&
	   ino2 instanceof Line) {
	    Line l1 = (Line)ino1;
	    Line l2 = (Line)ino2;
	    if (l1.vert || l2.vert) {
		int res = l1.seg.getStartpoint().compY(l2.seg.getStartpoint());
		if (res != 0) return res;
		else if (l1.vert && !l2.vert) return 1;
		else if (l2.vert && !l1.vert) return -1;
		else return l1.seg.getEndpoint().compY(l2.seg.getEndpoint());
	    }//if
	    this.b1.assign(evaluateLineFunc(l1));
	    this.b2.assign(evaluateLineFunc(l2));
	    if (b1.less(b2)) return -1;
	    if (b1.greater(b2)) return 1;
	    if (l1.m.less(l2.m)) return -1;
	    if (l1.m.greater(l2.m)) return 1;
	    if (l1.seg.getEndpoint().x.less(l2.seg.getEndpoint().x)) return -1;
	    if (l1.seg.getEndpoint().x.greater(l2.seg.getEndpoint().x)) return 1;
	    return 0;
	}//if
	else 
	    throw new WrongTypeException("Exception in LineComparator: Expected type Line - found: "+((Entry)ino1).value.getClass()+"/"+((Entry)ino2).value.getClass());
	
    }//end method compare
    

    /**
     * Sets the x field of the comparator.
     * The x field defines the position of the y axis used in the compare() method.
     *
     * @param the new x value
     */
    public void setX (Rational x) {
	this.x = x;
    }//end method setX


    /**
     * For a given Line object l, evaluates the function f(x) = mx + b and returns the result.
     * The actual x value stored in the x field is used for the computation.
     * @param l the line 
     * @return the reult of the evaluation as Rational
     */
    private Rational evaluateLineFunc(Line l) {
	Rational t1 = l.m.times(this.x,this.bt).plus(l.b,this.bp);
	return t1;
    }//end method evaluateLineFunc
    

}//end class LineComparator
