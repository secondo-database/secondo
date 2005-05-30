/*
 * Mathset.java 2005-05-09
 *
 * Dirk Ansorge FernUniversitaet Hagen
 *
 */
package twodsack.util;

import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.number.*;
import java.lang.Math.*;

/**
 * The Mathset class provides some mathematical methods that are need in a lot of methods of the 2D-SACK package. 
 * Most of the methods of this class take an instance of class {@link Point} as a vector in Euclidean space. Examples for
 * this are methods like <code>length: Point -> Rational</code> or <code>mulfac: Point -> Rational</code>.<p>
 * Some of the methods exist twice, like <code>length: Point -> Rational</code> and <code>length Point x Rational ->
 * Rational</code>. In the first case, a new Rational instance is constructed to store the result. In the seconde case,
 * the Rational parameter is used to store the result. By doing this, <code>new</code> calls are avoided. This saves
 * time and memory.
 */
public class Mathset {
    /*
     * constructors
     */
    /**
     * Don't use this constructor.
     */
    private Mathset(){};


    /*
     * fields
     */    
    static final double DERIV_DOUBLE = RationalFactory.readDerivDouble();
    static final double DERIV_DOUBLE_NEG = RationalFactory.readDerivDoubleNeg();
    static boolean PRECISE;
    static boolean preciseDefined;

    //these values are used for linearly_dependent and pointPosition
    static Rational DIFF_RAT1 = RationalFactory.constRational(0);
    static Rational DIFF_RAT2 = RationalFactory.constRational(1);

    /*
     * methods
     */
    /**
     * Returns the length of the passed vector.
     * Note, that during computation the conversion into a double value is necessary. The result may be not precise.
     *
     * @param v the given vector
     * @return the length as Rational
     */
    public static Rational length(Point v) {
	Rational l = RationalFactory.constRational(Math.sqrt((v.x.times(v.x).plus(v.y.times(v.y))).getDouble()));
	return l;
    }//end method length


    /**
     * Returns the length of the passed vector.
     * The result is stored in the parameter <i>in</i>. Note, that during computation the conversion into a double
     * value is necessary. The result may be not precise.
     *
     * @param v the given vector
     * @param in the result is stored in this parameter
     * @return the length as Rational
     */    
    public static Rational length(Point v, Rational in) {
	in.assign(Math.sqrt(v.x.times(v.x,in).plus(v.y.times(v.y,in),in).getDouble()));
	return in;
    }//end method length
    

    /**
     * Returns the length of the passed vector
     *
     * @param v the given vector
     * @return the length as double
     */
    public static double lengthD(Point v) {
	double l = Math.sqrt(v.x.getDouble() * v.x.getDouble() + v.y.getDouble() * v.y.getDouble());
	return l;
    }//end method lengthD


    /**
     * Returns the sum of two vectors
     *
     * @param v1 the first vector
     * @param v2 the second vector
     * @return the sum of <tt>v1</tt>, <tt>v2</tt> as Point
     */
    public static Point sum(Point v1, Point v2) {
	Point v = new Point((v1.x.plus(v2.x)),(v1.y.plus(v2.y)));
	return v;
    }//end method sum
    

    /**
     * Returns the difference of two vectors.
     *
     * @param v1 the first vector
     * @param v2 the second vector
     * @return <tt>v1</tt>-<tt>v2</tt> as Point
     */
    public static Point diff(Point v1, Point v2) {
	Point v = new Point((v1.x.minus(v2.x)),(v1.y.minus(v2.y)));
	return v;
    }//end method diff


    /**
     * Returns the difference of two vectors.
     * The result is stored in <i>in</i>. No new Point instance is constructed.
     *
     * @param v1 the first vector
     * @param v2 the second vector
     * @param in used to store the result
     * @return <tt>v1</tt>-<tt>v2</tt> as Point
     */
    public static Point diff(Point v1, Point v2, Point in) {
	in.x.assign(v1.x.minus(v2.x,DIFF_RAT1));
	in.y.assign(v1.y.minus(v2.y,DIFF_RAT2));
	return in;
    }//end method diff


    /**
     * Returns the product of two vectors.
     * 
     * @param v1 the first vector
     * @param v2 the second vector
     * @return <tt>v1</tt>*<tt>v2</tt> as Rational
     */
    public static Rational prod(Point v1, Point v2) {
	Rational e = RationalFactory.constRational((v1.x.times(v2.x)).plus(v1.y.times(v2.y)));
	return e;
    }//end method prod


    /**
     * Returns the product of two vectors.
     * The result is stored in <i>in</i>. No new Rational instance is constructed.
     *
     * @param v1 the first vector
     * @param v2 the second vector
     * @param in used to store the result
     * @return <tt>v1</tt>*<tt>v2</tt> as Rational
     */
    public static Rational prod(Point v1, Point v2, Rational in) {
	in.assign((v1.x.times(v2.x,in)).plus(v1.y.times(v2.y,in),in));
	return in;
    }//end method prod


    /**
     * Returns a new vector wich is the result of <code>fac * p</code>.
     *
     * @param p the vector
     * @param fac the number
     * @return a new Point instance which is equal to <code>fac * p</code>
     */
    public static Point mulfac(Point p, Rational fac) {
	return new Point(p.x.times(fac),p.y.times(fac));
    }//end method mulfac


    /**
     * Returns a new vector which is the result of <code>(p.x + fac, p.y + fac)</code>.
     *
     * @param p the vector
     * @param fac the number
     * @return a new Point instance which is equal to <code>(p.x + fac, p.y + fac)</code>
     */
    public static Point addfac(Point p, double fac) {
	Rational facR = RationalFactory.constRational(fac);
	return new Point(p.x.plus(facR),p.y.plus(facR));
    }//end method addfac


    /**
     * Returns a new vector which is the result of <code>(p.x - fac, p.y - fac)</code>.
     *
     * @param p the vector
     * @param fac the number
     * @return a new Point instance which is equal to <code>(p.x + fac, p.y + fac)</code>
     */
    public static Point subfac(Point p, double fac) {
	Rational facR = RationalFactory.constRational(fac);
	return new Point(p.x.minus(facR),p.y.minus(facR));
    }//end method subfac


    /**
     * Returns <tt>true</tt>, if both segments are collinear.
     *
     * @param s1 the first segment
     * @param s2 the second segment
     * @return <tt>true</tt>, if <tt>s1</tt>, <tt>s2</tt> are collinear
     */
    public static boolean linearly_dependent(Segment s1, Segment s2) {
	if (!preciseDefined) {
	    PRECISE = RationalFactory.readPrecise();
	    preciseDefined = true;
	}//if

	if (PRECISE) {
	    //PRECISE == true, use Deriv
	    
	    Point sv1 = new Point(s1.getEndpoint().x.minus(s1.getStartpoint().x),
				  s1.getEndpoint().y.minus(s1.getStartpoint().y));
	    Point sv2 = new Point(s2.getEndpoint().x.minus(s2.getStartpoint().x),
				  s2.getEndpoint().y.minus(s2.getStartpoint().y));
	    boolean sv2X0 = sv2.x.equal(0);
	    boolean sv2Y0 = sv2.y.equal(0);

	    if (sv2X0 && sv2Y0) return true;

	    Rational t1 = RationalFactory.constRational(0);
	    Rational t2 = RationalFactory.constRational(0);

	    if (!sv2X0) t1 = sv1.x.dividedby(sv2.x);
	    if (!sv2Y0) t2 = sv1.y.dividedby(sv2.y);

	    boolean t1t2equal = false;
	    if (t1.minus(t2).equal(0)) t1t2equal = true;
	    else {
		Rational zwires = (t1.minus(t2)).abs();
		if (zwires.less(RationalFactory.readDeriv())) t1t2equal = true;
		else t1t2equal = false;
	    }//else

	    if (t1t2equal && !(t1.equal(0) && t2.equal(0))) return true;

	    boolean compsv1x = (sv2.x.times(t1).minus(sv1.x)).abs().lessOrEqual(RationalFactory.readDeriv());
	    boolean compsv1y = (sv2.y.times(t2).minus(sv1.y)).abs().lessOrEqual(RationalFactory.readDeriv());

	    if (compsv1x && compsv1y) return true;

	    return false;
	}//if

	else {
	    
	    //PRECISE == false, use double constants as derivation value
	    //DERIV_DOUBLE and DERIV_DOUBLE_NEG
	    
	    double px = 0;
	    double py = 0;
	    double qx = s1.getEndpoint().x.getDouble() - s1.getStartpoint().x.getDouble();
	    double qy = s1.getEndpoint().y.getDouble() - s1.getStartpoint().y.getDouble();
	    double rx = 0;
	    double ry = 0;
	    double sx = s2.getEndpoint().x.getDouble() - s2.getStartpoint().x.getDouble();
	    double sy = s2.getEndpoint().y.getDouble() - s2.getStartpoint().y.getDouble();
	    

	    double res1 = ((ry + py) / 2) * (rx - px);
	    double res2 = ((qy + ry) / 2) * (qx - rx);
	    double res3 = ((py + qy) / 2) * (px - qx);
	    double result1 = res1+res2+res3;
	    if (result1 > DERIV_DOUBLE ||
		result1 < DERIV_DOUBLE_NEG) return false;
	    
	    double res4 = ((sy + py) / 2) * (sx - px);
	    double res5 = ((qy + sy) / 2) * (qx - sx);
	    double res6 = ((py + qy) / 2) * (px - qx);
	    double result2 = res4+res5+res6;

	    if (result2 > DERIV_DOUBLE ||
		result2 < DERIV_DOUBLE_NEG) return false;

	    return true;
	}//else
    }//end method linear_dependent
    
    
    /**
     * Returns the angle between two vectors.
     * The scalar product is used to compute the angle.
     *
     * @param p1 the first vector
     * @param p2 the second vector
     * @return the angle as Rational
     */
    public static Rational angle(Point p1, Point p2) {
	Rational e = RationalFactory.constRational(prod(p1,p2).dividedby(length(p1).times(length(p2))));
	e = RationalFactory.constRational(Math.acos(e.getDouble()));
	e = RationalFactory.constRational(Math.toDegrees(e.getDouble()));
	return e;
    }//end method angle
    

    /**
     * Returns the angle between two vectors.
     * The scalar product is used to compute the angle. The result is stored in <i>in</i>.
     *
     * @param p1 the first point
     * @param p2 the second point
     * @param in the result is stored in this parameter
     * @return the angle as Rational
     */
    public static Rational angle(Point p1, Point p2, Rational in) {
	in.assign(prod(p1,p2).dividedby(length(p1).times(length(p2))));
	double e = Math.acos(in.getDouble());
	e = Math.toDegrees(e);
	in.assign(e);
	return in;
    }//end method angle


    /**
     * Returns the angle between two vectors.
     * The scalar product is used to compute the angle.
     * 
     * @param p1 the first vector
     * @param p2 the second vector
     * @return the result as double
     */
    public static double angleD(Point p1, Point p2) {
	double e = (prod(p1,p2)).getDouble() / (lengthD(p1) * lengthD(p2));
	e = Math.acos(e);
	e = Math.toDegrees(e);
	return e;
    }//end method angleD
    

    /**
     * Normalizes the vector.
     *
     * @param p the 'in' vector
     * @return the normalized <tt>p</tt>
     */
    public static Point normalize(Point p){
	p.x = p.x.dividedby(length(p));
	p.y = p.y.dividedby(length(p));
	return p;
    }//end method normalize
    

    /**
     * Returns one of {0, -1, 1} depending on the position of <tt>p</tt> and the line through <tt>g1</tt>, <tt>g2</tt>.
     * Returns 0, if <tt>p</tt> lies on the line through <tt>g1</tt>, <tt>g2</tt>.<p>
     * Returns 1, if <tt>p</tt> lies on the right side of the line through <tt>g1</tt>, <tt>g2</tt>.<p>
     * Returns -1 otherwise.
     *
     * @param g1 the first point of the line
     * @param g2 the second point of the line
     * @param p the point that is checked
     * @return one of {0, -1, 1} as byte
     */
    public static byte pointPosition(Point g1, Point g2, Point p) {
	double s1x = g2.x.getDouble() - g1.x.getDouble();
	double s1y = g2.y.getDouble() - g1.y.getDouble();
	double s2x = p.x.getDouble() - g1.x.getDouble();
	double s2y = p.y.getDouble() - g1.y.getDouble();

	double t0 = s1x * s2y - s1y * s2x;

	if (t0 < DERIV_DOUBLE_NEG) return 1;
	else if (t0 > DERIV_DOUBLE) return -1;
	else return 0;
    }//end method pointPosition
    

    /**
     * Returns the projection of point <tt>p</tt> on the line through <tt>a</tt>, <tt>b</tt>.
     *
     * @param p the point that is projected
     * @param a the first point of the line
     * @param b the second point of the line
     * @return the projected point
     */
    public static Point projectionPointLine(Point p, Point a, Point b) {
	Point AProj;
	Point AB;
	Point AP;
	AB = diff(b,a);
	AP = diff(p,a);
	AProj = mulfac(AB,prod(AB,AP).dividedby(prod(AB,AB)));
	Point retPoint = sum(AProj,a);
	return retPoint;
    }//end method projectionPointLine

    
    /**
     * Computes the center of three points representing a triangle.
     *
     * @param p1 first point
     * @param p2 second point
     * @param p3 third point
     * @return center
     */
    public static Point center (Point p1, Point p2, Point p3) {
	return new Point (((p1.x.plus(p2.x.plus(p3.x))).dividedby(3)),
			  ((p1.y.plus(p2.y.plus(p3.y))).dividedby(3)));
    }//end method center

}//end class Mathset
