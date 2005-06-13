/*
 * Triangle.java 2005-05-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.basicdatatype;

import twodsack.io.*;
import twodsack.operation.basictypeoperation.*;
import twodsack.operation.setoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.iterator.*;
import twodsack.util.number.*;
import java.lang.Math.*;
import java.util.*;
import java.lang.reflect.*;
import java.io.*;


/**
 * The Triangle class implements one of the three basic datatypes, a triangle.
 * As fields, it has particularly a point array
 * of size 3 to hold the triangle's vertices. Furthermore, it has a <tt>bbox</tt> field for the bounding box and a <tt>segArr</tt> where the
 * segments of the triangle are stored. Note, that both, the <tt>bbox</tt> and <tt>segArr</tt> are not computed when constructing a new
 * triangle but only on demand. Two additional (private) fields, <tt>segArrDefined</tt> and <tt>bboxDefined</tt> are set to <tt>false</tt>, if the
 * appropriate fields were not computed, yet.<p>
 * There is a special configuration field for the use of a <i>garbage test</i>. That garbage test is a method which is invoked,
 * when <tt>USE_GARBAGE_TEST</tt> is set to <tt>true</tt>. Then, when using one of the (overloaded) methods <tt>minus()</tt>, <tt>plus()</tt>,
 * <tt>intersection()</tt>, 
 * the input triangles are tested whether they have a certain minimum angle or side-length. If not, they are dumped, but the
 * execution of the program is not terminated. The critical values can be configured by setting <tt>CRITICAL_SIDE_LENGTH</tt> and 
 * <tt>CRITICAL_ANGLE</tt>. Initially, <tt>CRITICAL_SIDE_LENGTH</tt> is set to 0.0005, and <tt>CRITICAL_ANGLE</tt> is set to 0.25.<p>
 * When calling a triangle's constructor, the triangles vertices can be tested, whether they lie on a line or not. This is done, if the field
 * <tt>TRIANGLE_TEST</tt> is set to <tt>true</tt> (default value is <tt>false</tt>. During this test, the values for <tt>DERIV_DOUBLE</tt> and
 * <tt>DERIV_DOUBLE_NEG</tt> implemented in the {@link Rational} class extension are used.
 */
public class Triangle extends Element implements Serializable {

    /*
     * configuration fields
     */
    /**
     * If <tt>true</tt>, triangles are checked for validity when constructed.<p>
     * Whether a triangle is valid or not depends on the values of {@link #CRITICAL_SIDE_LENGTH} and {@link #CRITICAL_ANGLE}.
     */
    public static boolean USE_GARBAGE_TEST = false;

    /**
     * If <tt>true</tt>, triangles are checked for validity when constructed.<p>
     * If all the triangles vertices lie on one line, the triangle is invalid.
     */
    public static boolean TRIANGLE_TEST = false;

    /**
     * Defines the value for a minimum side length of a triangle.
     */
    public static double CRITICAL_SIDE_LENGTH = 0.005;

    /**
     * Defines the value for a minimum (inner) angle of a triangle.
     */
    public static double CRITICAL_ANGLE = 0.25;

    /*
     * fields
     */
    private static BufferedReader inBR = new BufferedReader(new InputStreamReader(System.in));

    static final TriangleComparator TRIANGLE_COMPARATOR = new TriangleComparator();
    static final PointComparator POINT_COMPARATOR = new PointComparator();
    static final SegmentComparator SEGMENT_COMPARATOR = new SegmentComparator();
    static final PSPointComparator PSPOINT_COMPARATOR = new PSPointComparator();
    static final IvlComparator IVL_COMPARATOR_FALSE = new IvlComparator(false);
    static final IvlComparatorSimple IVL_COMPARATOR_SIMPLE = new IvlComparatorSimple();
    static final IvlComparatorNumber IVL_COMPARATOR_NUMBER = new IvlComparatorNumber();
    static final LineComparator LINE_COMPARATOR = new LineComparator();
    static final Class segClass = (new Segment()).getClass();
    static final Class ssOpsClass = (new SegSeg_Ops()).getClass();
    static Class[] paramListSS = { segClass, segClass };
    static Class[] paramListE = new Class[1];

    static final double DERIV_DOUBLE = RationalFactory.readDerivDouble();
    static final double DERIV_DOUBLE_NEG = RationalFactory.readDerivDoubleNeg();
    static final Rational FAC = RationalFactory.constRational(0.5);
    static boolean PRECISE;
    static boolean preciseDefined;

    static Rational ANGLE_1 = RationalFactory.constRational(0);
    static Rational ANGLE_2 = RationalFactory.constRational(0);
    static Rational ANGLE_3 = RationalFactory.constRational(0);
    static Rational LENGTH = RationalFactory.constRational(0);
    static Point SIT_POINT = new Point(0,0);
    static Rational SITX = RationalFactory.constRational(0);
    static Rational SITY = RationalFactory.constRational(0);
    static Point GB_SIDEA = new Point(0,0);
    static Point GB_SIDEB = new Point(0,0);
    static Point GB_SIDEC = new Point(0,0);
    static Point GB_SIDEC2 = new Point(0,0);
    static Segment PER_SEG = new Segment(0,0,1,1);

    private Point[] vertices = new Point[3];
    private Segment[] segArr;
    private boolean segArrDefined;

    /**
     * The bounding box of the triangle.
     */
    public Rect bbox;
    private boolean bboxDefined;


    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     * Sets the vertices[] entries to NULL.
     */
    public Triangle() {
	this.vertices[0] = null;
	this.vertices[1] = null;
	this.vertices[2] = null;
	this.segArrDefined = false;
	this.bboxDefined = false;
    }


    /**
     * Constructs a new triangle using the given points.<p>
     * While the construction of the triangle, the points are sorted using the {@link #compare(ComparableMSE)} method.
     *
     * @throws NotAValidTriangleException if triangle is not valid
     */
    public Triangle(Point p1, Point p2, Point p3) throws NotAValidTriangleException {
	if (isTriangle(p1,p2,p3)) {
	    this.vertices[0] = p1;
	    this.vertices[1] = p2;
	    this.vertices[2] = p3;
	    this.segArrDefined = false;
	    this.bboxDefined = false;
	    
	    //sort points
	    Point min,temp;
	    int num = 0;
	    boolean found;
	    for (int i = 0; i < 2; i++) {
		min = vertices[i];
		found = false;
		for (int j = i+1; j < 3; j++) {
		    if (vertices[j].compare(min) == -1) {
			min = vertices[j];
			num = j;
			found = true;
		    }//if
		}//for j
		if (found) {
		    temp = (Point)vertices[i].copy();
		    vertices[i] = vertices[num];
		    vertices[num] = temp;
		}//if
	    }//for i
	    
	}//if
	else throw new NotAValidTriangleException("Error in Triangle.constructor: Cannot construct triangle with these points: "+p1+", "+p2+", "+p3);
    }
   
    /*
     * methods
     */
    /**
     * Returns the perimeter of <i>this</i>.
     *
     * @return the perimeter as <tt>double</tt>
     */
    public double perimeter() {
	double sum = 0;
	PER_SEG.set(vertices[0],vertices[1]);
	sum += PER_SEG.length();
	PER_SEG.set(vertices[1],vertices[2]);
	sum += PER_SEG.length();
	PER_SEG.set(vertices[2],vertices[0]);
	sum += PER_SEG.length();
	return sum;
    } //end method perimeter


    /**
     * Returns the area of <i>this</i>.
     *
     * @return the area as <tt>double</tt>
     */
  public double area() {
      Segment[] sArr = this.segmentArray();
      Segment a = sArr[0];
      Segment b = sArr[1];
      Segment c = sArr[2];

      double semiperim = 0.5 * this.perimeter();
      double op1 = semiperim - a.length();
      double op2 = semiperim - b.length();
      double op3 = semiperim - c.length();
      double op4 = semiperim * op1 * op2 * op3;
      double op5 = Math.sqrt(op4);

      return op5;
  } //end method area
  

    /**
     * Sets the triangle's vertices to the given points.<p>
     * <tt>segArrDefined</tt> and <tt>bboxDefined</tt> are set to <tt>false</tt>.
     *
     * @param p1 the first point
     * @param p2 the second point
     * @param p3 the third point
     *
     * @return the modified triangle
     */
    public Triangle set (Point p1, Point p2, Point p3){
	this.vertices[0] = p1;
	this.vertices[1] = p2;
	this.vertices[2] = p3;
	this.segArrDefined = false;
	this.bboxDefined = false;

	return this;
    }//end method set


    /**
     * Sets the triangles vertices to the point given by the Point array.<p>
     * <tt>segArrDefined</tt> and <tt>bboxDefined</tt> are set to false.
     *
     * @param p the Point array, must have three Point entries
     * @return the modified triangle
     */
    public Triangle set (Point[] p){
	this.vertices[0] = p[0];
	this.vertices[1] = p[1];
	this.vertices[2] = p[2];
	this.segArrDefined = false;
	this.bboxDefined = false;

	return this;
    }//end method set 
  

    /**
     * Returns the vertices of <i>this</i> as Point array.<p>
     * The returned array is <u>no</u> copy of the triangle's point array. Changes on the vertices affect the triangle.
     *
     * @return the vertices as Point array
     */
    public Point[] vertices(){

	return this.vertices;
    }//end method vertices


    /**
     * Returns the vertices of <i>this</i> as PointMultiSet.<p>
     * The points in the PointMultiSet are <u>not</u> copies of the triangle's points. Changes on the vertices affect the triangle.
     *
     * @return the vertices as PointMultiSet
     */
    public PointMultiSet vertexSet(){
	PointMultiSet retSet = new PointMultiSet(POINT_COMPARATOR);
	retSet.add(this.vertices[0]);
	retSet.add(this.vertices[1]);
	retSet.add(this.vertices[2]);

	return retSet;
    }//end method vertexSet
    

    /**
     * Returns <tt>true</tt>, if <tt>p,q,r</tt> form a triangle.<p>
     * Whether the three points form a triangle or not is checked by finding out, whether all three are lying on one line.
     * If so, <tt>false</tt> is returned.
     *
     * @param p the first point
     * @param q the second point
     * @param r the third point
     * @return <tt>true</tt>, if <tt>p,q,r</tt> form a triangle
     */
    public boolean isTriangle(Point p, Point q, Point r){
	if (!TRIANGLE_TEST)
	    return true;
	else {
	    double res1 = ((r.y.getDouble() + p.y.getDouble()) / 2) * (r.x.getDouble() - p.x.getDouble());
	    double res2 = ((q.y.getDouble() + r.y.getDouble()) / 2) * (q.x.getDouble() - r.x.getDouble());
	    double res3 = ((p.y.getDouble() + q.y.getDouble()) / 2) * (p.x.getDouble() - q.x.getDouble());
	    double result = res1+res2+res3;
	    
	    if (result < DERIV_DOUBLE &&
		result > DERIV_DOUBLE_NEG) {
		result = 0;
	    }//if
	    if (result > DERIV_DOUBLE ||
		result < DERIV_DOUBLE_NEG) 
		return true;
	    else 
		return false;
	}//else
    }//end method isTriangle
    

    /**
     * Returns a 'deep' copy of <i>this</i>.<p>
     * All of the triangle's vertices are copied, so any changes on the copy don't affect the original.
     *
     * @return the triangle's copy
     */
    public Element copy(){
	Triangle copy = new Triangle((Point)this.vertices[0].copy(),
				     (Point)this.vertices[1].copy(),
				     (Point)this.vertices[2].copy());
	
	return copy;
    }//end method copy
    
    
    /**
     * Returns the triangle's border segments as SegMultiSet.<p>
     * The segments are not stored with the triangle's instance. With every call of this method, new instances of {@link Segment}
     * are constructed. Therefore, when calling this method very often, the program may get very slow.
     * 
     * @return the border segments as SegMultiSet
     */
    public SegMultiSet segmentMultiSet(){
	SegMultiSet retSet = new SegMultiSet(SEGMENT_COMPARATOR);
	retSet.add(new Segment(this.vertices[0].x,this.vertices[0].y,
			       this.vertices[1].x,this.vertices[1].y));
	retSet.add(new Segment(this.vertices[1].x,this.vertices[1].y,
			       this.vertices[2].x,this.vertices[2].y));
	retSet.add(new Segment(this.vertices[2].x,this.vertices[2].y,
			       this.vertices[0].x,this.vertices[0].y));
	return retSet;
    }//end method segments
    
    
    /**
     * Returns the triangle's border segments as Segment array.<p>
     * The segments are not stored with the triangle's instance. With every call of this method, new instances of Segment
     * are cosntructed. Therefore, when calling this method very often, the program may get very slow.
     *
     * @return the border segments as Segment array
     */
    public Segment[] segmentArray() {
	if (segArrDefined) return this.segArr;
	else {
	    this.segArr = new Segment[3];
	    int res = this.vertices[0].compare(this.vertices[1]);
	    if (res == -1) this.segArr[0] = new Segment(this.vertices[0].x,this.vertices[0].y,
							this.vertices[1].x,this.vertices[1].y);
	    else this.segArr[0] = new Segment(this.vertices[1].x,this.vertices[1].y,
					      this.vertices[0].x,this.vertices[0].y);
	    res = this.vertices[1].compare(this.vertices[2]);
	    if (res == -1) this.segArr[1] = new Segment(this.vertices[1].x,this.vertices[1].y,
							this.vertices[2].x,this.vertices[2].y);
	    else this.segArr[1] = new Segment(this.vertices[2].x,this.vertices[2].y,
					      this.vertices[1].x,this.vertices[1].y);
	    res = this.vertices[2].compare(this.vertices[0]);
	    if (res == -1) this.segArr[2] = new Segment(this.vertices[2].x,this.vertices[2].y,
							this.vertices[0].x,this.vertices[0].y);
	    else this.segArr[2] = new Segment(this.vertices[0].x,this.vertices[0].y,
					      this.vertices[2].x,this.vertices[2].y);
	    this.segArrDefined = true;
	}//else
	
	return this.segArr;
    }//end method segments
    

    /**
     * Returns <tt>true</tt>, if both triangles are equal.
     * 
     * @param trin the passed triangle
     * @return <tt>true</tt>, if both triangles are equal
     * @throws WrongTypeException if <tt>trin</tt> is not of type <tt>Triangle</tt>
     */
    public boolean equal(Element trin) throws WrongTypeException {
	if (trin instanceof Triangle) {
	    Triangle tin = (Triangle)trin;
	    Segment[] sA1 = this.segmentArray();
	    Segment[] sA2 = tin.segmentArray();
	    if ((sA1[0].equal(sA2[0]) ||
		 sA1[0].equal(sA2[1]) ||
		 sA1[0].equal(sA2[2])) &&
		(sA1[1].equal(sA2[0]) ||
		 sA1[1].equal(sA2[1]) ||
		 sA1[1].equal(sA2[2])) &&
		(sA1[2].equal(sA2[0]) ||
		 sA1[2].equal(sA2[1]) ||
		 sA1[2].equal(sA2[2])))
		return true;
	    else return false;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+trin.getClass()); }
    }//end method equal
    

    /**
     * Compares the y-coordinates of the triangle's vertices and returns one of {0, 1, -1}.<p>
     * The vertices are compared in the order they are stored. When constructing a new triangle, the vertices are sorted by their
     * x-coordinate and then by their y-coordinate. Therefore, it may be, that the y-coordiate of the second vertex is smaller
     * than the y-coordinate of the first vertex.<p>
     * Returns 0, if the y-coordinates are equal.<p>
     * Returns 1, if <tt>e</tt> has the smaller y-coordinate.<p>
     * Returns -1 otherwise.
     *
     * @param e the 'in' element
     * @return {0, 1, -1} as <tt>byte</tt>
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>Triangle</tt>
     */
    public byte compY (Element e) throws WrongTypeException {
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;
	    Point[] thisArr = this.vertices();
	    Point[] tinArr = t.vertices();
	    
	    byte cmp = thisArr[0].compY(tinArr[0]);
	    if (cmp != 0) return cmp;
	    else {
		cmp = thisArr[1].compY(tinArr[1]);
		if (cmp != 0) return cmp;
		else {
		    cmp = thisArr[2].compY(tinArr[2]);
		    if (cmp != 0) return cmp;
		    else //both triangles are equal w.r.t. y
			return 0;
		}//else
	    }//else
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
	
    }//end method compY


    /**
     * Compares the x-coordinates of the triangle's vertices and returns one of {0, 1, -1}.<p>
     * The vertices are compared in the order they are stored. When constructing a new triangle, the vertices are sorted by
     * their x-coordinate and then by their y-coordinate as second key.<p>
     * Returns 0, if the x-coordinates are equal.<p>
     * Returns 1, if <tt>e</tt> has the smaller x-coordiante.<p>
     * Returns -1 otherwise
     *
     * @param e the 'in' element
     * @return {0, 1, -1} as <tt>byte</tt>
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>Triangle</tt>
     */
    public byte compX(Element e) throws WrongTypeException {
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;
	    
	    Point[] thisArr = this.vertices();
	    Point[] tinArr = t.vertices();

	    byte cmp = thisArr[0].compX(tinArr[0]);
	    if (cmp != 0) return cmp;
	    else {
		cmp = thisArr[1].compX(tinArr[1]);
		if (cmp != 0) return cmp;
		else {
		    cmp = thisArr[2].compX(tinArr[2]);
		    if (cmp != 0) return cmp;
		    else //both triangles are equal w.r.t. x
			return 0;
		}//else
	    }//else
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
		    
    }//end method compX
    

    /**
     * Compares the coordinates of the triangle's vertices and returns one of {0, 1, -1}.<p>
     * The vertices are compared in the order they are stored. So, first <tt>this.vertices[0]</tt> is compared with <tt>e.vertices[0]</tt>.
     * Then, <tt>this.vertices[1]</tt> is compared with <tt>e.vertices[1]</tt> etc.<p>
     * Returns 0, if all vertices are equal.<p>
     * Returns 1, if <tt>e</tt> has smaller coordinates.<p>
     * Returns -1 otherwise
     *
     * @param e the 'in' element
     * @return {0, 1, -1} as int
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>Triangle</tt>
     */
    public int compare (ComparableMSE e) throws WrongTypeException {
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;

	    int res = this.vertices[0].compare(t.vertices[0]);
	    if (res == 0) {
		res = this.vertices[1].compare(t.vertices[1]);
		if (res == 0) res = this.vertices[2].compare(t.vertices[2]);
	    }//if

	    return res;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
	    
    }//end method compare


    /**
     * Computes the triangle's bounding box and stores it.
     * 
     * @return the bbox
     */
    private Rect computeBbox() {
	if (this.bboxDefined) return this.bbox;
	else {
	    Rational tmp = vertices[0].x;
	    Rational hxv;//highest x value
	    Rational lxv;//lowest x value
	    Rational hyv;//highest y value
	    Rational lyv;//lowest y value
	    
	    //compute hxv
	    for (int i = 1; i < 3; i++) {
		if (tmp.less(vertices[i].x)) { tmp = vertices[i].x; }
	    }//for
	    hxv = tmp;//.copy();
	    
	    //compute lxv
	    tmp = vertices[0].x;    
	    for (int i = 1; i < 3; i++) {
		if (tmp.greater(vertices[i].x)) { tmp = vertices[i].x; }
	    }//for
	    lxv = tmp;//.copy();
	    
	    //compute hyv
	    tmp = vertices[0].y;
	    for (int i = 1; i < 3; i++) {
		if (tmp.less(vertices[i].y)) { tmp = vertices[i].y; }
	    }//for
	    hyv = tmp;//.copy();
	    
	    //compute lyv
	    tmp = vertices[0].y;
	    for (int i = 1; i < 3; i++) {
		if (tmp.greater(vertices[i].y)) { tmp = vertices[i].y; }
	    }//for
	    lyv = tmp;//.copy();
	    
	    //set rectangle
	    this.bbox = new Rect(lxv,hyv,hxv,lyv);
	    this.bboxDefined = true;
	    return this.bbox;
	}//else
    }//end method computeBbox


    /**
     * Prints the triangle's data to standard output.
     */
    public void print() {
	//prints the object's data
	System.out.print("triangle:");
	System.out.print(" ("+vertices[0].x.toString()+", "+vertices[0].y.toString()+")");
	System.out.print(" ("+vertices[1].x.toString()+", "+vertices[1].y.toString()+")");
	System.out.println(" ("+vertices[2].x.toString()+", "+vertices[2].y.toString()+")");
    }//end method print


    /**
     * Returns <tt>true</tt>, if both triangles have common points.
     *
     * @param e the 'in' triangle
     * @return <tt>true</tt>, if both have at least one point in common
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>Triangle</tt>
     */
    public boolean intersects(Element e) throws WrongTypeException {
	if (e instanceof Triangle) {
	    Triangle t;
	    t = (Triangle)e;
	    
	    if (this.equal(t)) return true;

	    boolean intersect = false;
	    
	    if (this.noPointsInside(t) > 0) {
		return true; 
	    }//if
	    
	    Segment[] t1Arr = this.segmentArray();
	    Segment[] t2Arr = t.segmentArray();
	    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		    if (t1Arr[i].intersects(t2Arr[j])) {
			intersect = true;
			return intersect;
		    }//if
		}//for j
	    }//for i
	    
	    return intersect;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method intersects
    
    
    /**
     * Returns <tt>true<7tt>, if both triangles have a common area.
     *
     * @param tin the 'in' triangle
     * @return <tt>true</tt>, if both triangles have a common area
     */
    public boolean pintersects(Triangle tin){
	Segment[] thisSegs = this.segmentArray();
	Segment[] tinSegs = tin.segmentArray();
	for (int i = 0; i < 3; i++) {
	    for (int j = 0; j < 3; j++) {
		if (thisSegs[i].pintersects(tinSegs[j])) { return true; }
	    }//for j
	}//for i

	
	if (this.equal(tin)) { return true; }
	if (TriTri_Ops.inside(this,tin) ||
		TriTri_Ops.inside(tin,this)) { return true; }
	
	return false;
    }//end method pintersects



    /**
     * Return the number of tin's points which lie inside of <i>this</i>.
     *
     * @param tin the 'in' triangle
     * @return the number of points as <tt>int</tt>
     */
    protected int noPointsInside(Triangle tin){
	Point[] tinPl = new Point[3];
	tinPl = tin.vertices();
	int numInside = 0;

	if (PointTri_Ops.inside(tinPl[0],this)) { numInside++; }
	if (PointTri_Ops.inside(tinPl[1],this)) { numInside++; }
	if (PointTri_Ops.inside(tinPl[2],this)) { numInside++; }
	
	return numInside;
    }//end method noPointsInside


    /**
     * Computes, stores and returns the bounding box of <i>this</i>.
     * 
     * @return the bounding box
     */
    public Rect rect() {
	if (this.bboxDefined) return this.bbox;
	else {
	    this.bbox = computeBbox();
	    return this.bbox;
	}//else
    }//end method rect


    /**
     * Returns the Euclidean distance between both triangles.
     *
     * @param e the 'in' element
     * @return the distance as Rational
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>Triangle</tt>
     */
    public Rational dist(Element e) throws WrongTypeException {
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;
	    Segment[] thisArr = this.segmentArray();
	    Segment[] tArr = t.segmentArray();
	    LinkedList distlist = new LinkedList();
	    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		    distlist.add(thisArr[i].dist(tArr[j]));
		}//for j
	    }//for i
	    
	    Rational min = RationalFactory.constRational(0);
	    min = (Rational)distlist.getFirst();
	    for (int i = 0; i < distlist.size(); i++) {
		if (((Rational)distlist.get(i)).less(min)) {
		    min = (Rational)distlist.get(i); }//if
	    }//for i

	    return min.copy();
	    
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method dist


    /**
     * Computes a set of segments which is generated from <i>intersecting</i> both triangles.<p>
     * If two segments of <tt>t1,t2</tt> pintersect or overlap, they are split.
     * Duplicates are removed from the return set.
     * 
     * @param t1 the first triangle
     * @param t2 the second triangle
     * @return the set of 'split' segments
     */
    protected static SegMultiSet computeSegSet (Triangle t1, Triangle t2) {
	//This implementation uses a plane sweep algorithm
	t1 = (Triangle)t1.copy();
	t2 = (Triangle)t2.copy();

	//find pairs of overlapping segments using a plane-sweep algorithm
	//get segments arrays
	Segment[] segArr1 = t1.segmentArray();
	Segment[] segArr2 = t2.segmentArray();
	
	Vector allTSegs = new Vector(6);
	for (int i = 0; i < 3; i++) {
	    allTSegs.add(segArr1[i]);
	    allTSegs.add(segArr2[i]);
	}//for i
	
	MultiSet points = new MultiSet(PSPOINT_COMPARATOR);

	//compute all intersection points
	for (int i = 0; i < 3; i++) {
	    for (int j = 0; j < 3; j++) {
		if (segArr1[i].pintersects(segArr2[j])) {
		    Point intPoint = segArr1[i].intersection(segArr2[j]);
		    points.add(new PSPoint(intPoint,-1,-1,false,true));
		}//if
	    }//for j
	}//for i

	for (int i = 0; i < allTSegs.size(); i++) ((Segment)allTSegs.get(i)).align();

	//add all other segment's points to points
	for (int i = 0; i < 6; i++) {
	    points.add(new PSPoint(((Segment)allTSegs.get(i)).getStartpoint(),i,-1,true,false));
	    points.add(new PSPoint(((Segment)allTSegs.get(i)).getEndpoint(),i,-1,false,false)); 
	}//for i
		
	//construct SweepEventStructure (SES)
	MultiSet ses = points;
		
	//SegmentComparator sc = new SegmentComparator();
	SegMultiSet resSet = new SegMultiSet(SEGMENT_COMPARATOR);
	IvlMultiSet sss = new IvlMultiSet(IVL_COMPARATOR_FALSE);
	
	//start plane-sweep
	Iterator it = ses.iterator();
	Iterator it2;
	PSPoint actPoint;
	Segment actSeg = null;
	final Rational CONST_NULL = RationalFactory.constRational(0); //needed for construction of new intervals
	while (it.hasNext()) {
	    actPoint = (PSPoint)((MultiSetEntry)it.next()).value;
	
	    //check all segments in sss for pmeet
	    it2 = sss.iterator();
	    while (it2.hasNext()) {
		actSeg = (Segment)((Interval)((MultiSetEntry)it2.next()).value).ref;			
		
		//actpoint is a split point, so split appropriate segments
		if (!PointSeg_Ops.isEndpoint(actPoint.point,actSeg) && PointSeg_Ops.liesOn(actPoint.point,actSeg)) {
		    resSet.add(new Segment(actSeg.getStartpoint(),actPoint.point));
		    actSeg.set(actPoint.point,actSeg.getEndpoint());
		}//if		
	    }//while

	    if (actPoint.isStartpoint) {
		//insert new segment in sss
		Segment ref = (Segment)allTSegs.get(actPoint.number);
		if (ref.getStartpoint().y.less(ref.getEndpoint().y))
		    sss.add(new Interval(ref.getStartpoint().y,ref.getEndpoint().y,"",CONST_NULL,ref,actPoint.number,false));
		else 
		    sss.add(new Interval(ref.getEndpoint().y,ref.getStartpoint().y,"",CONST_NULL,ref,actPoint.number,false)); 
	    }//if
	    else if (!actPoint.isIntPoint) {
		//actPoint is endpoint
		//remove segment from sss
		Segment ref = (Segment)allTSegs.get(actPoint.number);
		if (ref.getStartpoint().y.less(ref.getEndpoint().y))
		    sss.removeAllOfThisKind(new Interval(ref.getStartpoint().y,ref.getEndpoint().y,"",CONST_NULL,ref,actPoint.number,false));
		else 
		    sss.removeAllOfThisKind(new Interval(ref.getEndpoint().y,ref.getStartpoint().y,"",CONST_NULL,ref,actPoint.number,false));
		
		//store segment in resSet
		resSet.add((Segment)allTSegs.get(actPoint.number));
	    }//else
	}//while it
	
	return resSet;	
    }//end method computeSegSet

    
    /**
     * Computes a set of segments which is generated from <i>intersecting</i> <att>tIN</tt> with the polygon represented by <tt>tmsIN</tt>.<p>
     * If two segments of <tt>tIN</tt>, <tt>tmsIN</tt> <tt>pintersect</tt> or <tt>overlap</tt>, they are split.<p>
     * The split points are stored in <tt>splitPoints</tt>. Note, that adjacent triangles in <tt>tmsIN</tt> may have two identical segments.
     * These duplicates are not removed. However, if two segments overlap, the overlapping part would occur twice. 
     * Duplicate segments of this kind are removed.
     *
     * @param tIN a single triangle
     * @param tmsIN the set of triangles which has to be intersected with <tt>tIN</tt>
     * @param splitPoints the set of vertices and intersection points
     */
    protected static SegMultiSet computeSegSet (Triangle tIN, TriMultiSet tmsIN, PointMultiSet splitPoints) {
	SegMultiSet resSet = new SegMultiSet(SEGMENT_COMPARATOR);
	ProLinkedList sss = new ProLinkedList(LINE_COMPARATOR);
	LineComparator lComp = LINE_COMPARATOR;
	Iterator it;

	Triangle t = tIN;
	TriMultiSet tms = tmsIN;
	//add all segments of tmsIN to sms
	SegMultiSet sms = new SegMultiSet(SEGMENT_COMPARATOR);
	it = tms.iterator();
	while (it.hasNext())
	    sms.add(((Triangle)((MultiSetEntry)it.next()).value).segmentArray());

	//remove (completely) from sms all duplicate segments
	//'inner' segment duplicates are removed by doing this
	sms = SegMultiSet.convert(SetOps.rdup2(sms));

	//add tIn's segments to sms
	sms.add(t.segmentArray());

	//remove duplicates from sms
	//now, all segments appear only once in the set
	sms = SegMultiSet.convert(SetOps.rdup(sms));

	//store all segments in an array
	Segment[] allTSegs = new Segment[sms.size()];
	it = sms.iterator();
	int idx = 0;
	while (it.hasNext()) {
	    allTSegs[idx] = (Segment)((MultiSetEntry)it.next()).value;
	    idx++;
	}//while it

	
	//make a copy of the array!
	Segment[] arrCOP = new Segment[allTSegs.length];
	for (int i = 0; i < allTSegs.length; i++) 
	    arrCOP[i] = (Segment)allTSegs[i].copy();
	allTSegs = arrCOP;
		 
	MultiSet ses = new MultiSet(PSPOINT_COMPARATOR);
	for (int i = 0; i < allTSegs.length; i++) {
	    ses.add(new PSPoint(allTSegs[i].getStartpoint(),i,-1,true,false));
	    ses.add(new PSPoint(allTSegs[i].getEndpoint(),i,-1,false,false));
	}//for i

	/*
	 * The intersection points between segments are computed and inserted in the SES
	 * during the execution of the sweep. An intersection point can only be found, when
	 * a new segment is inserted in the SSS, i.e. a startpoint is found.
	 * Then, the intervals in SSS are traversed. If an interval exists that overlaps
	 * the interval, check for intersection and insert the new intersection
	 * point in SES and splitPoints if there are any.
	 */ 
	it = ses.iterator();
	ProIterator it2 = sss.listIterator(0);
	ProIterator itVert = sss.listIterator(0);
	PSPoint actPoint;
	
	boolean stillTrue;
	Line actLine;
	Line pred = null;
	Line succ = null;
	Line sssLine = null;
	Entry actEntry = null;
	Entry actEntry2 = null;
	int count = 0;

	while (it.hasNext()) {
	    actPoint = (PSPoint)((MultiSetEntry)it.next()).value;
	    it.remove();
	    it2.reset();

	    if (actPoint.isStartpoint) {
		//actPoint is startpoint
		//construct actLine and insert it at correct position in sss
		actLine = new Line(allTSegs[actPoint.number],actPoint.number);

		stillTrue = true;
		lComp.setX(actPoint.point.x);
		while (it2.hasNext() && stillTrue) {
		    actEntry = (Entry)it2.nextEntry();
		    if (lComp.compare((Line)actEntry.value,actLine) == -1) stillTrue = true;
		    else stillTrue = false;
		}//while it2
		
		if (stillTrue) {
		    it2.add(actLine);
		} else {
		    it2.addBefore(actLine);
		}//else

		//set actEntry to that entry that was just added
		actEntry = sss.lastAdded;

		//if actLine is vertical, test for intersection with all lines in sss		
		if (actLine.vert) {
		    itVert.reset();
		    while (itVert.hasNext()) {
			sssLine = (Line)itVert.next();
			if (sssLine.seg.pintersects(actLine.seg)) {
			    //insert intpoint in ses
			    ses.add(new PSPoint(sssLine.seg.intersection(actLine.seg),
						sssLine.number,actLine.number,false,true));
			}//if
		    }//while itVert
		}//if actLine is vertical
		    
		else {
		    //if actLine is not vertical,
		    //compute intersection points with neighbour lines in sss, if they exist
		    if (!(actEntry.prev == null)) {
			pred = (Line)actEntry.prev.value;
			if (pred.seg.pintersects(actLine.seg)) {
			    //insert intpoint in ses
			    ses.add(new PSPoint(pred.seg.intersection(actLine.seg),
						pred.number,actLine.number,false,true));
			    
			}//if
			if (!PointSeg_Ops.isEndpoint(actLine.seg.getStartpoint(),pred.seg) && 
			    PointSeg_Ops.liesOn(actLine.seg.getStartpoint(),pred.seg)) {
			    //insert meetpoint in ses
			    ses.add(new PSPoint(actLine.seg.getStartpoint(),actLine.number,pred.number,false,true));
			}//if
		    }//if
		    if (!(actEntry.next == null)) {
			succ = (Line)actEntry.next.value;
			if (succ.seg.pintersects(actLine.seg)) {
			    //insert intpoint in ses
			    ses.add(new PSPoint(succ.seg.intersection(actLine.seg),
						succ.number,actLine.number,false,true));
			}//if
			if (!PointSeg_Ops.isEndpoint(actLine.seg.getStartpoint(),succ.seg) &&
			    PointSeg_Ops.liesOn(actLine.seg.getStartpoint(),succ.seg)) {
			    //insert meetpoint in ses
			    ses.add(new PSPoint(actLine.seg.getStartpoint(),actLine.number,succ.number,false,true));
			}//if 
		    }//if		
		}//else
	    }//if is startpoint
	    
	    else if (actPoint.isIntPoint) {
		//actPoint is intpoint or a meetpoint
		//spit both involved segments and store first parts in resSet
		//set involved segments to second part
		boolean meet1 = PointSeg_Ops.isEndpoint(actPoint.point,allTSegs[actPoint.number]);
		boolean meet2 = PointSeg_Ops.isEndpoint(actPoint.point,allTSegs[actPoint.number2]);
		if (!meet1) {
		    resSet.add(new Segment(allTSegs[actPoint.number].getStartpoint(),actPoint.point));
		    
		    allTSegs[actPoint.number].set(actPoint.point,allTSegs[actPoint.number].getEndpoint());
		}//if
		if (!meet2) {
		    resSet.add(new Segment(allTSegs[actPoint.number2].getStartpoint(),actPoint.point));		    
		    allTSegs[actPoint.number2].set(actPoint.point,allTSegs[actPoint.number2].getEndpoint());
		}//if
		//add intpoint to splitPoints
		splitPoints.add(actPoint.point);
		
		if (!meet1 && !meet2) {
		    //interchange positions of both lines in sss and check for new intpoints with neighbour lines
		    //find first of the lines; second line then must be the next entry
		    while (true) {
			actEntry = (Entry)it2.nextEntry();
			if ((((Line)actEntry.value).number == actPoint.number) ||
			    (((Line)actEntry.value).number == actPoint.number2)) break;
		    }//while
		    actEntry2 = actEntry.next;
		    
		    //actEntry is first element of sss
		    if (sss.head.next == actEntry)
			sss.head.next = actEntry2;
		    else actEntry.prev.next = actEntry2;
		    actEntry2.prev = actEntry.prev;
		    actEntry.next = actEntry2.next;
		    actEntry.prev = actEntry2;
		    if (sss.last.next == actEntry2)
			sss.last.next = actEntry;
		    else
			actEntry2.next.prev = actEntry;
		    actEntry2.next = actEntry;
		    
		    if (!(actEntry2.prev == null)) {
			if (((Line)actEntry2.prev.value).seg.pintersects(((Line)actEntry2.value).seg)) {
			    //insert intpoint in ses
			    ses.add(new PSPoint(((Line)actEntry2.prev.value).seg.intersection(((Line)actEntry2.value).seg),
						((Line)actEntry2.prev.value).number,((Line)actEntry2.value).number,false,true));
			}//if
		    }//if
		    
		    if (!(actEntry.next == null)) {
			if (((Line)actEntry.next.value).seg.pintersects(((Line)actEntry.value).seg)) {
			    //insert intpoint in ses
			    ses.add(new PSPoint(((Line)actEntry.next.value).seg.intersection(((Line)actEntry.value).seg),
						((Line)actEntry.next.value).number,((Line)actEntry.value).number,false,true));
			}//if
		    }//if
		}//if
	    }//if is intpoint
	    
	    else {
		//actPoint is endpoint
		//remove appropriate line from sss and store the segment that belongs to it in resSet
		//check for intersection of the new neighbour lines in sss
		while (it2.hasNext()) {
		    actEntry = (Entry)it2.nextEntry();
		    if (((Line)actEntry.value).number == actPoint.number) break;
		}//while it2

		//check for meet with old neighbours
		if (!(actEntry.prev == null) &&
		    !PointSeg_Ops.isEndpoint(actPoint.point,((Line)actEntry.prev.value).seg) &&
		    PointSeg_Ops.liesOn(actPoint.point,((Line)actEntry.prev.value).seg)) {
		    //insert meetpoint in ses
		    ses.add(new PSPoint(actPoint.point,actPoint.number,((Line)actEntry.prev.value).number,false,true));
		}//if
		if (!(actEntry.next == null) &&
		    !PointSeg_Ops.isEndpoint(actPoint.point,((Line)actEntry.next.value).seg) &&
		    PointSeg_Ops.liesOn(actPoint.point,((Line)actEntry.next.value).seg)) {
		    //insert meetpoint in ses
		    ses.add(new PSPoint(actPoint.point,actPoint.number,((Line)actEntry.next.value).number,false,true));
		}//if

		//check for intersection of new neighbours
		if (!(actEntry.prev == null) && !(actEntry.next == null)) {
		    if (((Line)actEntry.prev.value).seg.pintersects(((Line)actEntry.next.value).seg)) 
			ses.add(new PSPoint(((Line)actEntry.prev.value).seg.intersection(((Line)actEntry.next.value).seg),
					    ((Line)actEntry.prev.value).number,((Line)actEntry.next.value).number,false,true));
		}//if
		it2.remove();
		resSet.add(allTSegs[((Line)actEntry.value).number]);
	    }//else
	    it = ses.iterator();
	}//while it
	
	return resSet;	
    }//end method computeSegSet
    
    
    /**
     * Multiplies the coordinates of <i>this</i> with <tt>fact</tt>.
     *
     * @param fact the number which is multiplied with the coordinate values
     */
    protected void zoom (Rational fact) {
	this.vertices[0].zoom(fact);
	this.vertices[1].zoom(fact);
	this.vertices[2].zoom(fact);
    }//end method zoom


    /**
     * Returns <tt>true</tt>, if the triangles borders are shorter than <tt>CRITICAL_SIDE_LENGTH</tt> or if the area is smaller than <tt>CRITICAL_AREA</tt>.
     * Additionally, a message is written to standard output, where the values for the length or area are shown.
     *
     * @return <tt>true</tt>, if the triangle's edges or area are smaller than the defined values
     */
    public boolean garbageTest() {
	Segment[] sArr = this.segmentArray();
	for (int i = 0; i < sArr.length; i++) {
	    if (sArr[i].length() < CRITICAL_SIDE_LENGTH) {
		System.out.println("...trashed triangle (in Triangle.minus()) because of too small border length ("+sArr[i].length()+" < "+CRITICAL_SIDE_LENGTH+")");
		this.print();
		return true;
	    }//if
	}//for i
	
	GB_SIDEA.set(Mathset.diff(this.vertices[2],this.vertices[0],GB_SIDEA));
	GB_SIDEB.set(Mathset.diff(this.vertices[1],this.vertices[0],GB_SIDEB));
	GB_SIDEC.set(Mathset.diff(this.vertices[2],this.vertices[1],GB_SIDEC));
	GB_SIDEC2.set(Mathset.diff(this.vertices[1],this.vertices[2],GB_SIDEC2));

	ANGLE_1.assign(Mathset.angle(GB_SIDEA,GB_SIDEB,ANGLE_1));
	ANGLE_2.assign(Mathset.angle(GB_SIDEA,GB_SIDEC,ANGLE_2));
	ANGLE_3.assign(Mathset.angle(GB_SIDEB,GB_SIDEC2,ANGLE_3));

	if ((ANGLE_1.less(1) || ANGLE_2.less(1) || ANGLE_3.less(1)) &&
	    this.area() < CRITICAL_ANGLE) {
	    System.out.println("...trashed triangle (in Triangle.minus()) because of too small angles ("+ANGLE_1.getDouble()+","+ANGLE_2.getDouble()+","+ANGLE_3.getDouble()+" < 1 degree) and too small area ("+this.area()+" < "+CRITICAL_ANGLE);
	    this.print();
	    return true;
	}//if
	
	return false;
    }//end method garbageTest
    
    
    /**
     * Returns the border segments of the polygon which results from subtracting the polygon in <tt>ems</tt> from <i>this</i>.<p>
     * The passed <tt>ems</tt> may only contain triangles. Those triangles must represent a Polygons value, i.e. the 
     * triangles may not overlap, but they don't need to be adjacent.<p>
     * The result is a <tt>Polygons</tt> value again like described above. In the result set it is represented by its boundary and
     * then can be passed to a <tt>Polygons</tt> constructor. <p>
     * If <tt>ems</tt> is equal to <tt>NULL</tt> of is empty, the segment set of <i>this</i> is returned.
     * 
     * @param ems the set of triangles
     * @return a set of segments
     */
    public SegMultiSet minus (ElemMultiSet ems) {
	if (ems == null || ems.isEmpty()) return this.segmentMultiSet();

	TriMultiSet tms = TriMultiSet.convert(ems);

	//copy tms to array
	Triangle[] tmsArr = new Triangle[tms.size()];
	Iterator tit = tms.iterator();
	int tCount = 0;
	while (tit.hasNext()) {
	    tmsArr[tCount] = (Triangle)((MultiSetEntry)tit.next()).value;
	    tCount++;
	}//while tit


	//garbage collection...
	//removes triangles, if they have a side length shorter than critical value 0.001
	if (USE_GARBAGE_TEST && this.garbageTest() == true) return new SegMultiSet(SEGMENT_COMPARATOR);
		
	PointMultiSet thisPoints = this.vertexSet();
	PointMultiSet tmsPoints = new PointMultiSet(POINT_COMPARATOR);
	
	for (int i = 0; i < tmsArr.length; i++)
	    tmsPoints.addAll(tmsArr[i].vertexSet());
	
	tmsPoints = (PointMultiSet)SetOps.rdup(tmsPoints);
	//intPoints is not only the set of intersection points: It is the union of 
	//intersection points, points of t which lie on the border of tms (and vice versa)
	//and the points which belong to both triangle sets (t and tms).
	PointMultiSet intPoints = new PointMultiSet(POINT_COMPARATOR);

	PointMultiSet splitPoints = new PointMultiSet(POINT_COMPARATOR);

	SegMultiSet segs = computeSegSet(this,tms,splitPoints);

	SegMultiSet chosenSegs = new SegMultiSet(SEGMENT_COMPARATOR);
	SegMultiSet tmsBorder = new SegMultiSet(SEGMENT_COMPARATOR);
	
	for (int i = 0; i < tmsArr.length; i++) {
	    Segment[] tmpSegs = tmsArr[i].segmentArray();
	    
	    tmsBorder.add(tmpSegs[0]);
	    tmsBorder.add(tmpSegs[1]);
	    tmsBorder.add(tmpSegs[2]);
	}//while it
	
	tmsBorder = SegMultiSet.convert(SetOps.rdup2(tmsBorder));
	
	intPoints = PointMultiSet.convert(SetOps.intersection(thisPoints,tmsPoints));
	intPoints.addAll(splitPoints);

	//The set of segments returned by computeSegSets has duplicates. These duplicates show, that
	//these segments are from inside tms. They can be completely ignored, since they are not
	//part of the tms' border.
	SetOps.rdup2(segs);

	//compute the set of resulting segments: chosenSegs
	Point p1,p2;
	boolean p1insideA,p1insideB;
	boolean p2insideA,p2insideB;
	boolean p1elementX, p2elementX;
	boolean p1vertexA,p1vertexB;
	boolean p2vertexA,p2vertexB;
	Iterator it2;
	boolean inside1,inside2,found1,found2;
	Triangle actTri;
	int count = 0;
	
	Object[] segsArr = segs.toArray();
    
	for (int i = 0; i < segsArr.length; i++) {
	    Segment actSeg = (Segment)segsArr[i];
	    
	    p1 = actSeg.getStartpoint();
	    p2 = actSeg.getEndpoint();
	    
	    p1vertexA = thisPoints.contains(p1);
	    p1vertexB = tmsPoints.contains(p1);
	    p2vertexA = thisPoints.contains(p2);
	    p2vertexB = tmsPoints.contains(p2);
	    p1elementX = intPoints.contains(p1);
	    p2elementX = intPoints.contains(p2);
	    
	    //using plumbline algorithm
	    if (!p1vertexB) p1insideB = Polygons.inside(p1,tmsBorder);
	    else p1insideB = false;
	    if (!p2vertexB) p2insideB = Polygons.inside(p2,tmsBorder);
	    else p2insideB = false;

	    if (p1vertexA && p2vertexA && !p1elementX && !p2elementX &&
		((!p1vertexB || !p2vertexB) && (!p1insideB || !p2insideB))) {
		chosenSegs.add(actSeg);
	    }//if
	    
	    else {
		p1insideA = PointTri_Ops.inside(p1,this);
		p2insideA = PointTri_Ops.inside(p2,this);
		
		if ((p1vertexB && p2vertexB) &&
		    ((!p1insideA || !p2insideA) && (p1insideA && p2insideA))) {
		    chosenSegs.add(actSeg);
		}//if
		
		else if (((p1vertexA && p2elementX && !p1insideB) ||
			  (p2vertexA && p1elementX && !p2insideB)) &&
			 !(p1elementX && p1elementX)) {
		    chosenSegs.add(actSeg);
		}//if
		
		else if ((p1vertexB && p2elementX && p1insideA) ||
			 (p2vertexB && p1elementX && p2insideA)) {
		    chosenSegs.add(actSeg);
		}//if
		
		else if (p1vertexB && p2vertexB && p1insideA && p2insideA) {
		    chosenSegs.add(actSeg);
		}//if
		
		else if ((p1vertexA && !p1elementX && !p1insideB) ||
			 (p2vertexA && !p2elementX && !p2insideB)) {
		    chosenSegs.add(actSeg);
		}//if
		
		else {
		    boolean p1p2SegmentInsideB = segmentIsCoveredByTriangle(p1,p2,tms);
		    if (!p1p2SegmentInsideB) {
			chosenSegs.add(actSeg);
		    }//ifp1 = actSeg.getStartpoint();
	  
		
		    if (p1elementX && p2elementX && !SegTri_Ops.overlapsBorder(actSeg,this)) {
			chosenSegs.add(actSeg);
		    }//if
		    
		    else {
			//don't take this segment
		    }//else
		
		}//else
	    }//else
	    
	}//while it
	    
	return chosenSegs;
    }//end method minus


    /**
     * Returns the border segments of the polygon which results from intersecting the polygon in <tt>ems</tt> and <i>this</i>.<p>
     * The passed ems may only contain triangles. Those triangles must represent a <tt>Polygons</tt> value, i.e. the
     * triangles may not overlap, but they don't need to be adjacent.<p>
     * The result is a <tt>Polygons</tt> value again like described above. In the result set it is represented by its boundary and
     * then can be passed to a <tt>Polygons</tt> constructor.<p>
     * If <tt>ems</tt> is equal to <tt>NULL</tt> or is empty, <tt>NULL</tt> is returned.
     *
     * @param ems the set of triangles
     * @return a set of segments
     */
    public SegMultiSet intersection (ElemMultiSet ems) {
	if (ems == null || ems.isEmpty()) return null;

	TriMultiSet tms = TriMultiSet.convert(ems);

	//copy tms to array
	Triangle[] tmsArr = new Triangle[tms.size()];
	Iterator tit = tms.iterator();
	int tCount = 0;
	while (tit.hasNext()) {
	    tmsArr[tCount] = (Triangle)((MultiSetEntry)tit.next()).value;
	    tCount++;
	}//while tit

	//garbage collection...
	//removes triangles, if they have a side length shorter than critical value 0.001
	if (USE_GARBAGE_TEST && this.garbageTest() == true) return new SegMultiSet(SEGMENT_COMPARATOR);

	PointMultiSet thisPoints = this.vertexSet();
	PointMultiSet tmsPoints = new PointMultiSet(POINT_COMPARATOR);

	for (int i = 0; i < tmsArr.length; i++)
	    tmsPoints.addAll(tmsArr[i].vertexSet());
	tmsPoints = (PointMultiSet)SetOps.rdup(tmsPoints);
	//intPOints is not only the set of intersection points: It is the union of
	//intersection points, points of t which lie on the border of tms (and vice versa)
	//and the points which belong to both triangle sets (t and tms).
	PointMultiSet intPoints = new PointMultiSet(POINT_COMPARATOR);
	PointMultiSet splitPoints = new PointMultiSet(POINT_COMPARATOR);
	SegMultiSet segs = computeSegSet(this,tms,splitPoints);
	SegMultiSet chosenSegs = new SegMultiSet(SEGMENT_COMPARATOR);
	SegMultiSet tmsBorder = new SegMultiSet(SEGMENT_COMPARATOR);
	
	for (int i = 0; i < tmsArr.length; i++) {
	    Segment[] tmpSegs = tmsArr[i].segmentArray();
	    tmsBorder.add(tmpSegs[0]);
	    tmsBorder.add(tmpSegs[1]);
	    tmsBorder.add(tmpSegs[2]);
	}//while it
	tmsBorder = SegMultiSet.convert(SetOps.rdup2(tmsBorder));

	intPoints = PointMultiSet.convert(SetOps.intersection(thisPoints,tmsPoints));
	intPoints.addAll(splitPoints);

	//The set of segments returned by computeSegSets has duplicates. These duplicates show, that
	//these segments are from inside tms. They can be completely ignored, since they are not
	//part of the tms' border.
	SetOps.rdup2(segs);

	//compte te set of resulting segments: chosenSegs
	Point p1,p2;
	boolean p1insideA,p1insideB;
	boolean p2insideA,p2insideB;
	boolean p1elementX, p2elementX;
	boolean p1vertexA,p1vertexB;
	boolean p2vertexA,p2vertexB;
	Iterator it2;
	boolean inside1,inside2,found1,found2;
	Triangle actTri;
	int count = 0;

	Object[] segsArr = segs.toArray();

	for (int i = 0; i < segsArr.length; i++) {
	    Segment actSeg = (Segment)segsArr[i];
	    
	    p1 = actSeg.getStartpoint();
	    p2 = actSeg.getEndpoint();
	    
	    p1vertexA = thisPoints.contains(p1);
	    p1vertexB = tmsPoints.contains(p1);
	    p2vertexA = thisPoints.contains(p2);
	    p2vertexB = tmsPoints.contains(p2);
	    p1elementX = intPoints.contains(p1);
	    p2elementX = intPoints.contains(p2);
	    
	    
	    //using plumbline algorithm
	    if (!p1vertexB) p1insideB = Polygons.inside(p1,tmsBorder);
	    else p1insideB = false;
	    if (!p2vertexB) p2insideB = Polygons.inside(p2,tmsBorder);
	    else p2insideB = false;
	    	    

	    if (p1vertexA && p2vertexA && p1insideB && p2insideB) {
		chosenSegs.add(actSeg);
	    }//if

	    else {
		p1insideA = PointTri_Ops.inside(p1,this);
		p2insideA = PointTri_Ops.inside(p2,this);
		
		if (p1vertexB && p2vertexB &&
		    ((p1insideA && p2insideA) && (!p1vertexA || !p2vertexA))) {
		    chosenSegs.add(actSeg);
		}//if
		else if ((p1vertexA && p2elementX && p1insideB) ||
			 (p1elementX && p2vertexA && p2insideB)) {
		    chosenSegs.add(actSeg);
		}//if
		else if ((p1vertexB && p2elementX && p1insideA) ||
			 (p1elementX && p2vertexB && p2insideA)) {
		    chosenSegs.add(actSeg);
		}//if
		else {
		    boolean p1p2SegmentInsideB = segmentIsCoveredByTriangle(p1,p2,tms);
		    if (p1elementX && p2elementX && p1p2SegmentInsideB) {
			chosenSegs.add(actSeg);
		    }//if
		    else {
			//don't take this segment
		    }//else
		}//else
	    }//else
	}//for i

	return chosenSegs;
    }//end method 


    /**
     * Returns the border segments of the polygon which results from adding the polygon in <tt>ems</tt> to <i>this</i>.<p>
     * The passed <tt>ems</tt> may only contain triangles. Those triangles must represent a <tt>Polygons</tt> value, i.e. the
     * triangles may not overlap, but they don't need to be adjacent.<p>
     * The result is a <tt>Polygons</tt> value again like described above. In the result set, it is represented by its boundary.
     * It then can be passed to a <tt>Polygons</tt> constructor.<p>
     * If <tt>ems</tt> is equal to <tt>NULL</tt> or is empty, the set of segments of <i>this</i> is returned
     *
     * @param ems the set of triangles
     * @return a set of segments
     */
    public SegMultiSet plus (ElemMultiSet ems) {
	if (ems == null || ems.isEmpty()) return this.segmentMultiSet();

	TriMultiSet tms = TriMultiSet.convert(ems);

	//copy tms to array
	Triangle[] tmsArr = new Triangle[tms.size()];
	Iterator tit = tms.iterator();
	int tCount = 0;
	while (tit.hasNext()) {
	    tmsArr[tCount] = (Triangle)((MultiSetEntry)tit.next()).value;
	    tCount++;
	}//while tit

	//garbage collection...
	//removes triangles, if they have a side length shorter than critical value 0.001
	if (USE_GARBAGE_TEST && this.garbageTest() == true) return new SegMultiSet(SEGMENT_COMPARATOR);

	PointMultiSet thisPoints = this.vertexSet();
	PointMultiSet tmsPoints = new PointMultiSet(POINT_COMPARATOR);

	for (int i = 0; i < tmsArr.length; i++)
	    tmsPoints.addAll(tmsArr[i].vertexSet());
	tmsPoints = (PointMultiSet)SetOps.rdup(tmsPoints);
	//intPoints is not only the set of intersection points: It is the union of
	//intersection points, points of t which lie on the border of tms (and vice versa)
	//and the points which belong to both triangle sets (t and tms).
	PointMultiSet intPoints = new PointMultiSet(POINT_COMPARATOR);
	PointMultiSet splitPoints = new PointMultiSet(POINT_COMPARATOR);
	SegMultiSet segs = computeSegSet(this,tms,splitPoints);
	SegMultiSet chosenSegs = new SegMultiSet(SEGMENT_COMPARATOR);
	SegMultiSet tmsBorder = new SegMultiSet(SEGMENT_COMPARATOR);
	
	for (int i = 0; i < tmsArr.length; i++) {
	    Segment[] tmpSegs = tmsArr[i].segmentArray();
	    tmsBorder.add(tmpSegs[0]);
	    tmsBorder.add(tmpSegs[1]);
	    tmsBorder.add(tmpSegs[2]);
	}//while it
	tmsBorder = SegMultiSet.convert(SetOps.rdup2(tmsBorder));

	intPoints = PointMultiSet.convert(SetOps.intersection(thisPoints,tmsPoints));
	intPoints.addAll(splitPoints);

	//The set of segments returned by computeSegSets has duplicastes. These duplicates show, that
	//these segments are from inside tms. They can be completely ignored, since they are not
	//part of the tms' border.
	SetOps.rdup2(segs);

	//compte te set of resulting segments: chosenSegs
	Point p1,p2;
	boolean p1insideA,p1insideB;
	boolean p2insideA,p2insideB;
	boolean p1elementX, p2elementX;
	boolean p1vertexA,p1vertexB;
	boolean p2vertexA,p2vertexB;
	Iterator it2;
	boolean inside1,inside2,found1,found2;
	Triangle actTri;
	int count = 0;
	TriMultiSet thisTMS = new TriMultiSet(TRIANGLE_COMPARATOR);
	thisTMS.add(this);

	Object[] segsArr = segs.toArray();

	for (int i = 0; i < segsArr.length; i++) {
	    Segment actSeg = (Segment)segsArr[i];
	    
	    p1 = actSeg.getStartpoint();
	    p2 = actSeg.getEndpoint();
	    
	    p1vertexA = thisPoints.contains(p1);
	    p1vertexB = tmsPoints.contains(p1);
	    p2vertexA = thisPoints.contains(p2);
	    p2vertexB = tmsPoints.contains(p2);
	    p1elementX = intPoints.contains(p1);
	    p2elementX = intPoints.contains(p2);
	    
	    //using plumbline algorithm
	    if (!p1vertexB) p1insideB = Polygons.inside(p1,tmsBorder);
	    else p1insideB = false;
	    if (!p2vertexB) p2insideB = Polygons.inside(p2,tmsBorder);
	    else p2insideB = false;
	    p1insideA = PointTri_Ops.inside(p1,this);
	    p2insideA = PointTri_Ops.inside(p2,this); 

	    
	    if (p1vertexA && p2vertexA && !p1insideB && !p2insideB) {
		chosenSegs.add(actSeg);
	    }//if
	    
	    else if (p1vertexB && p2vertexB && !p1insideA && !p2insideA) {
		chosenSegs.add(actSeg);
	    }//if
	    
	    else if (((p1vertexA && p2elementX) && !p1insideB) ||
		     ((p2vertexA && p1elementX) && !p2insideB)) {
		chosenSegs.add(actSeg);
	    }//if
	    
	    else if (((p1vertexB && p2elementX) && !p1insideA) ||
		     ((p2vertexB && p1elementX) && !p2insideA)){
		chosenSegs.add(actSeg);
	    }//if
	    
	    else if ((p1vertexA && p2vertexB) ||
		     (p1vertexB && p2vertexA)) {
		chosenSegs.add(actSeg);
	    }//if
	    
	    else {
		boolean p1p2SegmentInsideB = segmentInsideTriangle(p1,p2,tms);
		boolean p1p2SegmentInsideA = segmentInsideTriangle(p1,p2,thisTMS);

		if (p1elementX && p2elementX && !p1p2SegmentInsideB && !p1p2SegmentInsideA) {
		System.out.println("case6");
		chosenSegs.add(actSeg);
		}//if
		
		else {
		    //don't take this segment
		}//else
	    }//else

	}//for i

	return chosenSegs;
    }//end method plus


    /**
     * Returns <tt>true</tt>, if both points are covered by the same triangle of <tt>tms</tt>.
     * 
     * @param p1 the first point
     * @param p2 the second point
     * @param tms the set of triangles
     * @return <tt>true</tt>, if <tt>p1,p2</tt> are covered by the same triangle of <tt>tms</tt>
     */
    private boolean coveredBySameTriangle(Point p1, Point p2, TriMultiSet tms) {
	Iterator it = tms.iterator();
	Triangle actTri;
	while (it.hasNext()) {
	    actTri = (Triangle)((MultiSetEntry)it.next()).value;
	    if (PointTri_Ops.isCovered(p1,actTri) && PointTri_Ops.isCovered(p2,actTri))
		return true;
	}//while it
	
	return false;
    }//end method coveredBySameTriangle


    /**
     * Returns <tt>true</tt>, if a point <tt>p</tt> lying on the segment <tt>(p1,p2)</tt> is covered by any triangle of <tt>tms</tt>.
     * <tt>p</tt> may not be <tt>p1</tt> or <tt>p2</tt>.
     *
     * @param p1 the first point
     * @param p2 the second point
     * @param tms the set of triangles
     * @return <tt>true</tt>, if <tt>p</tt> is covered by any triangle of <tt>tms</tt>
     */    
    private static boolean segmentIsCoveredByTriangle(Point p1, Point p2, TriMultiSet tms) {
	//compute a point on segment p1-p2
	SIT_POINT.set(Mathset.diff(p2,p1,SIT_POINT));
	SIT_POINT.set(p1.x.plus(SIT_POINT.x.times(FAC,SITX),SITX),p1.y.plus(SIT_POINT.y.times(FAC,SITY),SITY));
	Iterator it = tms.iterator();
	while (it.hasNext())
	    if (PointTri_Ops.isCovered(SIT_POINT,((Triangle)((MultiSetEntry)it.next()).value))) 
		return true;

	return false;
    }//end method segmentIsCoveredByTriangle


    /**
     * Returns <tt>true</tt>, if a point <tt>p</tt> lying on the segment <tt>(p1,p2)</tt> lies inside of any triangle of <tt>tms</tt>.
     * <tt>p</tt> may not be <tt>p1</tt> or <tt>p2</tt>.
     *
     * @param p1 the first point
     * @param p2 the second point
     * @param tms the set of triangles
     * @return <tt>true</tt>, if <tt>p</tt> lies inside of any triangle of <tt>tms</tt>
     */
    private static boolean segmentInsideTriangle(Point p1, Point p2, TriMultiSet tms) {
	//compute a point on segment p1-p2
	SIT_POINT.set(Mathset.diff(p2,p1,SIT_POINT));
	SIT_POINT.set(p1.x.plus(SIT_POINT.x.times(FAC,SITX),SITX),p1.y.plus(SIT_POINT.y.times(FAC,SITY),SITY));
	Iterator it = tms.iterator();
	while (it.hasNext())
	    if (PointTri_Ops.inside(SIT_POINT,((Triangle)((MultiSetEntry)it.next()).value))) 
		return true;

	return false;
    }//end method segmentInsideTriangle


    /**
     * Returns the hashcode for <i>this</i>.
     * All of the vertices coordinates are added up.
     *
     * @return the hashcode as <tt>int</tt>
     */
    public int hashCode() {
	int sum = 0;
	for (int i = 0; i < 3; i++) 
	    sum += vertices[i].hashCode();

	return sum;
    }//end method hashCode


    /**
     * Returns <tt>true</tt>, if both objects are equal.
     *
     * @param o the object to compare with
     * @return <tt>true</tt>, if <i>this</i> and <tt>o</tt> are equal
     */
    public boolean equals (Object o) {
	return (this.equal((Element)o));
    }//end method equals


    /**
     * Converts the triangle's data to a <tt>String</tt>.
     * Useful for pretty-printing.
     *
     * @return the triangle data as <tt>String</tt>
     */
    public String toString() {
	return "triangle ("+this.vertices[0]+", "+this.vertices[1]+", "+this.vertices[2]+")";
    }//end method toString

} //end class Triangle
  
