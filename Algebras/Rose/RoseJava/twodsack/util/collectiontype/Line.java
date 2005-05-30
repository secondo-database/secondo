/*
 * Line.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;

import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.number.*;


/**
 * Instances of type Line are used for the sweep status structure some sweep line algorithms for segments. It provides fields for storing
 * lines of the form <code>f(x) = mx + b</code>. <tt>m</tt> and <tt>b</tt> are stored directly as fields. Then, <tt>vert</tt> is a flag to store wether
 * the Line is a vertical line or not. <tt>seg</tt> is a reference to the original {@link twodsack.setelement.datatype.basicdatatype.Segment} instance
 * and, furthermore, with <tt>number</tt>, one can assign a number to the line.<p>
 * The line is computed in such a way that the segment from which it was constructed, lies completely on it.
 */
public class Line {
    /*
     * fields
     */
    /**
     * The factor <tt>m</tt> of the line equation.
     * <p>See also method <code>copy</code>.
     */
    public Rational m;

    /**
     * The factor <tt>b</tt> of the line equation.
     */
    public Rational b;

    /**
     * Indicates whether the line is vertical or not.
     */
    public boolean vert;

    /**
     * The referenced segment.
     */
    public Segment seg;

    /**
     * A number for the line.
     */
    public int number;
    
    private Rational mTMP1;
    private Rational mTMP2;
    private Rational bTMP;

    
    /*
     * constructors
     */
    /**
     * Don't use this constructor!
     */
    private Line() {};
    

    /**
     * Constructs a new instance from seg.
     * Depending on seg, <tt>Line.m</tt> and <tt>Line.b</tt> are computed if the segment is not vertical.
     *
     * @param seg the segment
     * @param number an int number
     */
    public Line(Segment seg, int number) {
	this.seg = seg;
	this.number = number;
	if (seg.getStartpoint().x.equal(seg.getEndpoint().x)) {
	    this.m = null;
	    this.b = null;
	    this.vert = true;
	}//if vertical
	else {
	    this.vert = false;
	    this.mTMP1 = RationalFactory.constRational(0);
	    this.mTMP2 = RationalFactory.constRational(0);
	    this.bTMP = RationalFactory.constRational(0);
	    this.m = seg.getStartpoint().y.minus(seg.getEndpoint().y).dividedby(seg.getStartpoint().x.minus(seg.getEndpoint().x));
	    this.b = seg.getStartpoint().y.minus(this.m.times(seg.getStartpoint().x));
	}//else
    }

}//end class Line
