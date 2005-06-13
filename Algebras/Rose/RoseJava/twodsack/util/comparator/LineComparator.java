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
 * A LineComparator is used to sort ojects of type {@link twodsack.util.collectiontype.Line}.
 * It is usually passed to a constructor of a collection which has to be sorted using
 * the {@link #compare(Object,Object)} method of this class.
 */
public class LineComparator implements Comparator,Serializable{
    /*
     * fields
     */
    /**
     * The actual <tt>x</tt> coordinate. This is needed to compute the correct intersection point of a Line object and a vertical Line
     * e.g. the y axis.
     */
    public Rational x;

    /**
     * The result of an evaluated Line equation.
     */
    public Rational yValue;

    /**
     * The actual Line object.
     */
    public Line line;

    private static Rational bp = RationalFactory.constRational(0);
    private static Rational bt = RationalFactory.constRational(0);
    private static Rational b1 = RationalFactory.constRational(0);
    private static Rational b2 = RationalFactory.constRational(0);


    /*
     * constructors
     */
    /**
     * Cosntructs an 'empty' LineComparator.
     * The <tt>x</tt>, <tt>y</tt> values and <tt>line</tt> fields are set to NULL.
     */
    public LineComparator() {
	this.x = null;
	this.yValue = null;
	this.line = null;
    }
	

    /**
     * Compares two objects and returns one of {0, 1, -1}.
     * Both objects must be of type {@link twodsack.util.collectiontype.Line}. Then, the intersection point of both lines with the
     * y axis is computed (if any).<p>
     * Return 0, if both intersection points are equal AND the underlying segments are equal.<p>
     * Return -1, if the intersection point of <tt>ino1</tt> and the y axis is smaller than the intersection point of <tt>ino2</tt> and the y axis OR if the intersection points are equal and <tt>ino1.seg < ino2.seg</tt>.<p>
     * Return 1 otherwise<p>
     * Note, that the y axis is not really considered to lie at position <tt>x = 0</tt>, but depends on the <tt>x</tt> coordinate in the classes <tt>x</tt> field.
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WronTypeException if <tt>ino1</tt> or <tt>ino2</tt> is not of type Line
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
	    return l1.seg.compare(l2.seg);
	}//if
	else 
	    throw new WrongTypeException("Exception in LineComparator: Expected type Line - found: "+((Entry)ino1).value.getClass()+"/"+((Entry)ino2).value.getClass());
	
    }//end method compare
    

    /**
     * Sets the <tt>x</tt> field of the comparator.
     * The <tt>x</tt> field defines the position of the y axis used in the <tt>compare()</tt> method.
     *
     * @param x the new <tt>x</tt> value
     */
    public void setX (Rational x) {
	this.x = x;
    }//end method setX


    /**
     * For a given Line object <tt>l</tt>, evaluates the function <tt>f(x) = mx + b</tt> and returns the result.
     * The actual <tt>x</tt> value stored in the <tt>x</tt> field is used for the computation.
     * @param l the line 
     * @return the result of the evaluation as Rational
     */
    private Rational evaluateLineFunc(Line l) {
	Rational t1 = l.m.times(this.x,this.bt).plus(l.b,this.bp);
	return t1;
    }//end method evaluateLineFunc
    

}//end class LineComparator
