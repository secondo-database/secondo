/*
 * PointSeg_Ops.java 2005-04-28
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.basictypeoperation;

import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;
import twodsack.util.number.*;

import java.util.*;

/**
 * The PointSeg_Ops class holds methods which all have one {@link Point} object and one {@link Segment} object as parameters.
 * All of these methods also could have been implemented in class Point or class Segment
 * or both. The decision to put them in this class is debatable to some point, but it's better to
 * have them collected in one class instead of implementing them more than once.<p>
 * However, when searching for a specific operation with Point/Segment as parameters in some cases
 * you'll have to search in PointSeg_Ops, Point and Segment.
 */
public class PointSeg_Ops {
    
    /*
     * fields
     */
    static final double DERIV_DOUBLE = RationalFactory.readDerivDouble();
    static final double DERIV_DOUBLE_NEG = RationalFactory.readDerivDoubleNeg();
    static final double DERIV_DOUBLE_PLUS1 = DERIV_DOUBLE+1;
    private static boolean PRECISE;
    private static boolean preciseDefined;
    
    
    /*
     *constructors
     */
    /**
     * The standard constructor.
     */
    public PointSeg_Ops(){}


    /*
     * methods
     */
    /**
     * Returns <tt>true</tt>, if the point <tt>p</tt> lies on the segment <tt>s</tt>.
     * A point is considered to lie on a segment if the segment <u>covers</u> the point, i.e.
     * in the special case that a point is an endpoint of the segment, this method returns <tt>true</tt>.
     *
     * @param p the point
     * @param s the segment
     * @return <tt>true</tt>, if the point is covered by the segment
     */
    public static boolean liesOn(Point p, Segment s) {
	if (!preciseDefined) {
	    PRECISE = RationalFactory.readPrecise();
	    preciseDefined = true;
	}//if
	
	if (PRECISE) {
	    //PRECISE == true, use Deriv

	    //
	    //CHANGE THIS AS SEEN BELOW!!!
	    //

	    Rational DERIV = RationalFactory.readDeriv();
	    if (isEndpoint(p,s)) { return true; }
	    Point s1p = Mathset.diff(p,s.getStartpoint());
	    Point s1s2 = Mathset.diff(s.getEndpoint(),s.getStartpoint());
	    int count = 0;
	    boolean s1pX0 = s1p.x.equal(0);
	    boolean s1pY0 = s1p.y.equal(0);
	    if (s1pX0 && s1pY0) return true;
	    
	    Rational t1 = RationalFactory.constRational(0);//must be set to 0
	    Rational t2 = RationalFactory.constRational(0);//dito
	    if (!s1pX0) t1 = s1s2.x.dividedby(s1p.x);
	    if (!s1pY0) t2 = s1s2.y.dividedby(s1p.y);

	    Rational t1MINt2 = t1.minus(t2);
	    boolean t1t2equal = t1MINt2.less(DERIV) && t1MINt2.greater(DERIV.times(-1));
	    
	    if (!(s1pX0 || s1pY0) && !t1t2equal) {
		return false; }

	    Rational s1s2compXVal = (s1p.x.times(t1)).minus(s1s2.x);
	    boolean s1s2compX = (DERIV.equal(s1s2compXVal)) || 
		(s1s2compXVal.less(DERIV) && s1s2compXVal.greater(DERIV.times(-1)));
	    Rational s1s2compYVal = (s1p.y.times(t2)).minus(s1s2.y);
	    boolean s1s2compY = (DERIV.equal(s1s2compYVal)) ||
		(s1s2compYVal.less(DERIV) && s1s2compYVal.greater(DERIV.times(-1)));

	    boolean t1valid = t1.equal(0) || !(t1.less(DERIV.plus(1)));
	    boolean t2valid = t2.equal(0) || !(t2.less(DERIV.plus(1)));

	    if (s1s2compX && s1s2compY &&
		t1valid && t2valid) return true;
	    return false;
	}//if
	else {
	    //PRECISE == false
	    Point q = s.getStartpoint();
	    Point r = s.getEndpoint();

	    //test whether all points are on one line
	    double res1 = ((r.y.getDouble() + p.y.getDouble()) / 2) * (r.x.getDouble() - p.x.getDouble());
	    double res2 = ((q.y.getDouble() + r.y.getDouble()) / 2) * (q.x.getDouble() - r.x.getDouble());
	    double res3 = ((p.y.getDouble() + q.y.getDouble()) / 2) * (p.x.getDouble() - q.x.getDouble());
	    double result = res1+res2+res3;
	    
	    if (result > DERIV_DOUBLE ||
		result < DERIV_DOUBLE_NEG) return false;
	    
	    //does p lie between q and r?
	    Point min = Point.min(q,r);
	    Point max = Point.max(q,r);

	    if (p.compare(min) >= 0 &&
		p.compare(max) <= 0) return true;

	    return false;
	}//else
    } //end method liesOn
    

    /**
     * Returns true, if <tt>p</tt> is an endpoint of <tt>s</tt>.
     *
     * @param p the point
     * @param s the segment
     * @return returns <tt>true</tt>, if one of the segment's endpoints equals <tt>p</tt>
     */
    public static boolean isEndpoint(Point p, Segment s) {
	return (p.equal(s.getEndpoint()) || p.equal(s.getStartpoint()));
    } //end method isEndpoint


    /**
     * Returns the Euclidian distance of a point and a segment as Rational.
     * 
     * @param p the point
     * @param s the segment
     * @return the Euclidian distance as Rational
     */
    public static Rational dist(Point p, Segment s){
	if (liesOn(p,s)) { return (RationalFactory.constRational(0)); }
	LinkedList distlist = new LinkedList();
	
	distlist.add(s.getStartpoint().dist(p));
	distlist.add(s.getEndpoint().dist(p));
	Point proj = Mathset.projectionPointLine(p,s.getStartpoint(),s.getEndpoint());
	if (PointSeg_Ops.liesOn(proj,s)) { distlist.add(proj.dist(p)); }
	
	Rational min = (Rational)distlist.getFirst();
	for (int i = 1; i < distlist.size(); i++) {
	   if (((Rational)distlist.get(i)).less(min)) {
		min = (Rational)distlist.get(i); }//if
	}//for i

	return min;
    }//end method dist

} //end class PointSeg_Ops
