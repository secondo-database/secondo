/*
 * TriTri_Ops.java 2005-05-02
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.basictypeoperation;

import twodsack.setelement.datatype.basicdatatype.*;
import java.util.*;
import java.lang.reflect.*;

/**
 * In this class, methods for <code>Triangle x Triangle</code> can be found. If you are searching for specific
 * operations on triangles which are not in this class, have a look at class {@link Triangle} itself.
 */
public class TriTri_Ops {
    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public TriTri_Ops(){}


    /*
     * methods
     */

    /**
     * Returns <tt>true</tt>, if the first triangle lies fully inside of the second triangle.<p>
     * The inside criterium is checked using the inside method for points and triangles. If all of the vertices of
     * the first triangle are considered to be inside of the second triangle, this method returns <tt>true</tt>.
     *
     * @param t1 the first triangle
     * @param t2 the second triangle
     * @return <tt>true</tt>, if the first triangle lies inside of the second triangle
     */
    public static boolean inside(Triangle t1, Triangle t2) {
	Point[] t1verts = t1.vertices();
	for (int i = 0; i < t1verts.length; i++) {
	    if (!(PointTri_Ops.inside(t1verts[i],t2) ||
		  PointTri_Ops.liesOnBorder(t1verts[i],t2))) {
		return false;
	    }//if
	}//for i
	return true;
    }//end method inside


    /**
     * Returns <tt>true</tt>, if both triangles are area disjoint but have a common boundary.<p>
     * A common boundary means, that both triangles have a pair of overlapping (boundary) segments.
     * If both triangles only meet in one point, this method returns <tt>false</tt>.
     *
     * @param t1 the first triangle
     * @param t2 the second triangle
     * @return <tt>true</tt>, if <tt>t1,t2</tt> are adjacent
     */
    public static boolean adjacent(Triangle t1, Triangle t2) {
	boolean t1IntersectsT2 = false;
	
	t1IntersectsT2 = t1.pintersects(t2);
	if (t1IntersectsT2) { return false; }
	else {
	    Segment[]t1Arr = t1.segmentArray();
	    Segment[]t2Arr = t2.segmentArray();
	    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		    if (t1Arr[i].equal(t2Arr[j]) || SegSeg_Ops.overlap(t1Arr[i],t2Arr[j])) {
			return true;
		    }//if
		}//for j
	    }//for i
	}//else
	return false;
    }//end method adjacent


    /**
     * Returns <tt>true</tt>, if the triangles meet in one point.<p>
     * To return <tt>true</tt>, it is neccessary that both triangles only have <i>one</i> common point.
     * If they have a common boundary, this method returns <tt>false</tt>.
     *
     * @param t1 the first triangle
     * @param t2 the second triangle
     * @return <tt>true</tt>, if <tt>t1,t2</tt> meet
     */
    public static boolean meets(Triangle t1, Triangle t2) {
	boolean t1IntersectsT2 = false;
	
	t1IntersectsT2 = t1.pintersects(t2);
	if (t1IntersectsT2) { return false; }
	else {
	    Segment[]t1Arr = t1.segmentArray();
	    Segment[]t2Arr = t2.segmentArray();
	    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		    if (t1Arr[i].equal(t2Arr[j]) || SegSeg_Ops.overlap(t1Arr[i],t2Arr[j])) {
			return false;
		    }//if
		}//for j
	    }//for i
	    
	    for (int i = 0; i < 3;i++) {
		for (int j = 0; j < 3; j++) {
		    if (PointSeg_Ops.liesOn(t1.vertices()[i],t2Arr[j])) {
			return true;
		    }//if

		    if (PointSeg_Ops.liesOn(t2.vertices()[i],t1Arr[j])) {
			return true;
 		    }//if
 		}//for j
 	    }//for i
	    
	}//else
	return false;
    }//end method meet

}//end class TriTri_Ops
