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
 * lines of the form <code>f(x) = mx + b</code>. m and b are stored directly as fields. Then, vert is a flag to store wether the Line is
 * a vertical line or not. seg is a reference to the original Segment instance and, furthermore, with number, one can assign a number to
 * the line.<p>
 * The line is computed in such a way that the segment from which it was constructed, lies completely on it.
 */
public class Line {
    /*
     * fields
     */
    public Rational m;
    public Rational b;
    public boolean vert;
    public Segment seg;
    public int number;
    Rational mTMP1;
    Rational mTMP2;
    Rational bTMP;

    
    /*
     * constructors
     */
    /**
     * Don't use this constructor!
     */
    private Line() {};
    

    /**
     * Constructs a new instance from seg.
     * Depending on seg, Line.m and Line.b are computed, if the segment is not vertical.
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
