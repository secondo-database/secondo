/*
 * PointTri_Ops.java 2005-04-29
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.basictypeoperation;

import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.number.*;
import java.util.*;

/**
 * The PointTri_Ops class holds methods which all have one point value and one triangle value as 
 * parameters. Some of the methods that are implemented here, could also be implemented in class 
 * Triangle or in class Point. To some point it is debatable, where these methods should be implemented.
 * When searching for a specific operation with the pair Point/Triangle as parameters you should also
 * look in the class Point.
 */
public class PointTri_Ops {
    
    /*
     * fields
     */
    static final double DERIV_DOUBLE = RationalFactory.readDerivDouble();
    static final double DERIV_DOUBLE_NEG = RationalFactory.readDerivDoubleNeg();
    
    /*
     * methods
     */

    /**
     * Returns true, if the point lies inside of the triangle.
     * A point is not considered to ly <u>inside</u> of the triangle, if it's lying on its border.
     * 
     * @param p the point
     * @param t the triangle
     * @return true, if the p lies inside of t
     */
    public static boolean inside (Point p, Triangle t) {
	Point[] vertices = t.vertices();

	double det1 = det(vertices[0],vertices[1],p);
	double det2 = det(vertices[1],vertices[2],p);
	double det3 = det(vertices[2],vertices[0],p);

	if ((det1 > 0 && det2 > 0 && det3 > 0) ||
	    (det1 < 0 && det2 < 0 && det3 < 0)) return true;
	else return false;
    }//end method inside
    
    
    /**
     * Returns true, if the point lies on the border of the triangle.
     * If the point is equal to one of the vertices of the triangle, true is returned.
     *
     * @param p the point
     * @param t the triangle
     * @return true, if p lies on the border of t
     */
    public static boolean liesOnBorder (Point p, Triangle t) {
	if (t.vertices()[0].equal(p) ||
	    t.vertices()[1].equal(p) ||
	    t.vertices()[2].equal(p)) {
	    return true;
	}//if
	
	Segment[] sArr = t.segmentArray();
	for (int i = 0; i < sArr.length; i++) {
	    if (PointSeg_Ops.liesOn(p,sArr[i]))
		return true;
	}//for i
	
	return false;
    }//end method liesOnBorder

    /**
     * Returns true, if the point equals a vertex of the triangle.
     *
     * @param p the point
     * @param t the triangle
     * @return true, if p equals a vertex of t
     */
    public static boolean isVertex (Point p, Triangle t) {
	if (t.vertices()[0].equal(p) ||
	    t.vertices()[1].equal(p) ||
	    t.vertices()[2].equal(p)) {
	    return true; }
	else return false;
    }//end method is_vertice


    /**
     * Returns the Euclidean distance between the point and the triangle.
     *
     * @param p the point
     * @param t the triangle
     * @return the Euclidean distance as Rational
     */
    public static Rational dist(Point p, Triangle t) {
	LinkedList distList = new LinkedList();
	
	if (inside(p,t)) return (RationalFactory.constRational(0));
	Segment[] tArr = t.segmentArray();
	for (int i = 0; i < 3; i++) { distList.add(PointSeg_Ops.dist(p,tArr[i])); }
	Rational min = (Rational)distList.getFirst();
	for (int i = 1; i < 3; i++) {
	    if (((Rational)distList.get(i)).less(min)) {
		min = (Rational)distList.get(i); }//if
	}//for i
	return min.copy();
    }//end method dist


    /**
     * Returns true, if the point is covered by the triangle.
     * In particular, this method returns true, if the point lies on the triangle's border or equals
     * one of its vertices.
     *
     * @param p the point
     * @param t the triangle
     * @return true, if p is covered by t
     */
    public static boolean isCovered (Point p, Triangle t) {
	Point[] vertices = t.vertices();

	double det1 = det(vertices[0],vertices[1],p);
	double det2 = det(vertices[1],vertices[2],p);
	double det3 = det(vertices[2],vertices[0],p);
	
	if ((det1 >= 0 && det2 >= 0 && det3 >= 0) ||
	    (det1 <= 0 && det2 <= 0 && det3 <= 0)) return true;
	else return false;
    }//end method isCovered


    /**
     * This is a supportive method for this class that computes the determinant for a 3*2 matrix.
     * The matrix is determined by the given three points (of a triangle mostly). The result is
     * 0 if p,q,r is not a triangle, or a positive or negative number, if it is a triangle.
     *
     * @param p the first point
     * @param q the second point
     * @param r the third point
     * @return 0, if p,q,r is not a triangle
     */
    protected static double det (Point p, Point q, Point r) {

	double res1 = ((r.y.getDouble() + p.y.getDouble()) / 2) * (r.x.getDouble() - p.x.getDouble());
	double res2 = ((q.y.getDouble() + r.y.getDouble()) / 2) * (q.x.getDouble() - r.x.getDouble());
	double res3 = ((p.y.getDouble() + q.y.getDouble()) / 2) * (p.x.getDouble() - q.x.getDouble());
	
	double result = res1+res2+res3;

	if (result < DERIV_DOUBLE &&
	    result > DERIV_DOUBLE_NEG) {
	    result = 0;
	}//if

	return result;
    }//end method det

}//end class PointTri_Ops
