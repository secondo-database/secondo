/*
 * SupportOps.java 2004-11-10
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.operation.setoperation;

import twodsack.operation.basictypeoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ListIterator;
import java.io.BufferedReader;
import java.io.InputStreamReader;

/**
 * Placed in this class are some high level algebra operations which are needed to formulate
 * operations of a <i>target algebra</i> with the help of 2D-SACK. Though an algebra implementor
 * is not really forced to use these operations, they make the implementation much easier. If used,
 * a target algebra operation rarely consists of more than a few lines.
 * <p>Operations for the conversion <code>ElemMultiSet <-> LinkedList</code> can be found here, too.
 */
public class SupportOps {
    /*
     * fields
     */
    private static BufferedReader inBR = new BufferedReader(new InputStreamReader(System.in));

    static final Class pointClass = (new Point()).getClass();
    static final Class segClass = (new Segment()).getClass();
    static final Class triClass = (new Triangle()).getClass();

    static final Class ptOpsClass = (new PointTri_Ops()).getClass();
    static final Class ssOpsClass = (new SegSeg_Ops()).getClass();
    static final Class stOpsClass = (new SegTri_Ops()).getClass();
    static final Class emsClass = (new ElemMultiSet(new ElemComparator())).getClass();

    final static Class[] paramListPT = { pointClass, triClass };
    final static Class[] paramListST = { segClass, triClass };
    final static Class[] paramListSS = { segClass, segClass };
    final static Class[] paramListEMS = { emsClass };

    final static SegmentComparator SEGMENT_COMPARATOR = new SegmentComparator();
    final static ElemComparator ELEM_COMPARATOR = new ElemComparator();
    final static TriangleComparator TRIANGLE_COMPARATOR = new TriangleComparator();


    /*
     * constructors
     */
    /**
     * Don't use this constructor.
     */
    private SupportOps(){}


    /**
     * For a set of segments, <code>minimal</code> tests this set for adjacent segments and concats them.
     * This method is ment to be used for the border of a <code>Polygons</code> instance, but can be
     * used for any <code>SegMultiSet</code>. This methods finds pairs of segments which are adjacent,
     * i.e. they are linearly dependant and <i>one</i> of their endpoints is the same (in short: they
     * are 'neighbours' without overlap). If such a pair is found, both segments are concatenated. As
     * result, the number of segments is reduced by one for every such pair found.
     *
     * @param sl the set of segments
     * @param overlap If <code>false</code>, the more robust but slow <code>SetOps.reduce()</code> is
     * used to find pairs of adjacent segments. Otherwise, <code>SetOps.overlapReduceSweep</code> is used.
     * @param bboxFilter if <tt>true</tt>, a bounding box filter is applied (may reduce the time spent for this operation)
     * @return the new, reduced set of segments as <code>SegMultiSet</code>
     */
    public static SegMultiSet minimal (SegMultiSet sl, boolean overlap, boolean bboxFilter) {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	Class c = ssOpsClass;

	try {
	    Method m1 = c.getMethod("adjacent",paramListSS);
	    Method m2 = c.getMethod("concat",paramListSS);
	    if (!overlap) { retList = SegMultiSet.convert(SetOps.reduce(sl,m1,m2)); }
	    else {
		try {
		    boolean meet = true;
		    boolean earlyExit = false;
		    int setNumber = 0;
		    //retList = SegMultiSet.convert(SetOps.overlapReduce(sl,m1,m2,meet,bboxFilter,earlyExit,setNumber)); 
		    retList = SegMultiSet.convert(SetOps.overlapReduceSweep(sl,m1,m2,meet));
		} catch (Exception e) {
		    System.out.println("SupportOps.minimal: Caught unexpected exception: "+e);
		    e.printStackTrace();
		    System.exit(0);
		}//catch 
	    }//else
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in SupportOps.minimal(SegMultiSet,boolean):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	return retList;
    }//end method minimal


    /**
     * For a set of segments, <code>unique</code> finds pairs of overlapping pairs and removes the overlapping part.<p>
     * Generally, this method is used to compute the contour of a <code>Polygons</code> instance.
     * In that case, the passed set of segments consists of all border segments of the triangles
     * representing the <code>Polygons</code> instance. Nevertheless, this method can be applied on
     * an arbitrary set of segments.
     * <p><code>unique</code> finds pairs of segments which overlap, computes that overlapping part
     * and <i>removes</i> it from both segments. For two identical segments the result set would be
     * empty. 
     *
     * @param sl the set of segments
     * @param overlap If <code>false</code>, the more robust but slow <code>SetOps.reduce()</code> is
     * used to find pairs of overlapping segments. Otherwise, <code>SetOps.overlapReduce</code> is used.
     * @param bboxFilter if <tt>true</tt>, a bounding box filter is used (may help to reduce time spent)
     * @return the new, reduced set of segments as <code>SegMultiSet</code>
     */
    public static SegMultiSet unique (SegMultiSet sl, boolean overlap, boolean bboxFilter) {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	Class c = ssOpsClass;

	//remove duplicates with SetOps.rdup2
	SetOps.rdup2(sl);

	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    Method m2 = c.getMethod("symDiff",paramListSS);
	    if (!overlap) { retList = (SegMultiSet)SetOps.reduce(sl,m1,m2); }
	    else { 
		try {
		    boolean meet = true;
		    boolean earlyExit = false;
		    int setNumber = 0;
		    retList = SegMultiSet.convert(SetOps.overlapReduce(sl,m1,m2,meet,bboxFilter,earlyExit,setNumber));
		    //*doesn't work *** retList = SegMultiSet.convert(SetOps.overlapReduceSweep(sl,m1,m2,meet));
		} catch (Exception e) {
		    System.out.println("SupportOps.unique: Caught unexpected exception: "+e);
		    e.printStackTrace();
		    System.exit(0);
		}//catch
	    }//else
	    
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in SupportOps.unique(SegMultiSet,boolean):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Exception cause: "+e.getCause());
	    System.out.println("Exception string: "+e.toString());
	    System.out.println("stack trace: ");
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	return retList;
    }//end method unique


    /**
     * The contour of a triangle set (representing a polygon) is computed with this method.<p>
     * This is done in three steps:
     * <p><ol>
     * <li>decomposition of all triangles into their segments</li>
     * <li>usage of <code>unique</code> to remove all overlapping parts of segments</li>
     * <li>usage of <code>minimal</code> to concat adjacent segments</li>
     * </ol><p>
     * The usage of <code>SetOps.reduce</code> or <code>SetOps.overlapReduce</code> is
     * toggled via <code>minOverlap</code> and <code>uniOverlap</code>.
     *
     * @param tl the set of triangles
     * @param minOverlap If <code>false</code>, the more robust but slow <code>SetOps.reduce()</code> is
     * used in <code>minimal</code>. Otherwise, <code>SetOps.overlapReduce</code> is used.
     * @param uniOverlap If <code>false</code>, the more robust but slow <code>SetOps.reduce()</code> is
     * used in <code>unique</code>. Otherwise, <code>SetOps.overlapReduce</code> is used.
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce time spent for this operation)
     * @param computeMinimalSet if <code>true</code>, <code>minimal</code> is used to reduce the set of segments in the
     *                          resulting set
     * @return the contour as <code>SegMultiSet</code>
     * @see #minimal(SegMultiSet,boolean,boolean)
     * @see #unique(SegMultiSet,boolean,boolean)
     */
    public static SegMultiSet contourGeneral (TriMultiSet tl,boolean minOverlap,boolean uniOverlap,boolean bboxFilter, boolean computeMinimalSet) {
	Class c = triClass;
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	try {
	    Method m = c.getMethod("segmentArray",null);
	    if (computeMinimalSet) 
		retList = minimal(unique(SegMultiSet.convert(SetOps.map(tl,m)),uniOverlap,bboxFilter),minOverlap,bboxFilter);
	    else
		retList = unique(SegMultiSet.convert(SetOps.map(tl,m)),uniOverlap,bboxFilter);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in SupportOps.contourGeneral(TriMultiSet,boolean,boolean):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Exception cause: "+e.getCause());
	    System.out.println("Exception string: "+e.toString());
	    System.out.println("stack trace:"); e.printStackTrace();
	    System.exit(0);
	}//catch
	return retList;
    }//end method contourGeneral


    /**
     * The contour of a triangle set (representing a polygon) is computed with this method.<p>
     * When using this method instead of <code>contourGeneral</code> it is assumed, that the triangle set
     * was computed using the methods of class {@link twodsack.setelement.datatype.compositetype.Polygons}. In that case, bounding segments of the 
     * triangle set don't overlap. This method doesn't work for triangles with overlapping (instead of identical) segments.
     * {@link #minimal(SegMultiSet,boolean,boolean)} is executed on the resulting set to combine collinear and adjacent segments.
     *
     * @param ts the set of triangles
     * @return the contour as <code>SegMultiSet</code>
     * @see #contourGeneral(TriMultiSet,boolean,boolean,boolean,boolean)
     */
    public static SegMultiSet contour (TriMultiSet ts) {
	SegMultiSet retSet = new SegMultiSet(SEGMENT_COMPARATOR);
	Triangle actTri;
	Segment[] actSegs;
	Iterator it = ts.iterator();
	while (it.hasNext()) {
	    actTri = (Triangle)((MultiSetEntry)it.next()).value;
	    actSegs = actTri.segmentArray();
	    retSet.add(actSegs[0]);
	    retSet.add(actSegs[1]);
	    retSet.add(actSegs[2]);
	}//while

	retSet = minimal(SegMultiSet.convert(SetOps.rdup2(retSet)),true,false);
	
	return retSet;
    }//end method contour
    

    /**
     * Removes from the set of segments all parts that are covered by triangles of the triangle set.<p>
     * For a set of segments that is completely covered by triangles, the returned set is empty.
     * This method works a follows:
     * <p><ol>
     * <li>Alignment of all segments. This is necessary to have a correct order on the segments.</li>
     * <li>Computation of all intersecting triangles for each segment. These pairs are stored in a 
     * {@link twodsack.set.LeftJoinPairMultiSet}.</li>
     * <li>For every pair: Computation of the part(s) of the segment, that is (are) covered by triangles</li>
     * <li>For every pair: Removal of the covered parts from the covered segments.</li>
     * </ol><p>
     * Finally, the remainig parts of the segments are returned.
     *
     * @param sl the set of segments
     * @param tl the set of triangles
     * @param bboxFilter if <tt>true</tt>, a bounding box filter is applied (may help to reduce time spent)
     * @param earlyExit if <tt>true</tt>, execution is stopped immediately, when no overlapping pair is found 
     *                  for an element of the set specified by <code>setNumber</code>
     * @param setNumber specifies the <tt>setNumber</tt>
     * @return the set of remaining segments
     * @throws EarlyExit
     */
    public static SegMultiSet minus (SegMultiSet sl, TriMultiSet tl, boolean bboxFilter,boolean earlyExit, int setNumber) 
	throws EarlyExit {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	Class c = stOpsClass;
	Class c2 = (new SetOps()).getClass();
	Class c3 = ssOpsClass;
	Class[] paramList = new Class[4];
	Iterator lit = null;
	Method m1 = null;
	Method m2 = null;
	Method m3 = null;
	Method m4 = null;
	Method m5 = null;
	Method m6 = null;
	Method m7 = null;
	LeftJoinPairMultiSet ljpl;

	try {
	    paramList[0] = Class.forName("twodsack.set.ElemMultiSet");
	    paramList[1] = Class.forName("twodsack.set.ElemMultiSet");
	    m1 = c.getMethod("isCovered",paramListST);
	    paramList[2] = m1.getClass();
	    paramList[3] = m1.getClass();
	    //m2 = c.getMethod("minus",paramListST);
	    ljpl =  new LeftJoinPairMultiSet();
	    m4 = c.getMethod("intersection",paramListST);
	    //m5 = c3.getMethod("minus",paramListSS);
	    //m6 = c3.getMethod("overlap",paramListSS);
	    m7 = segClass.getMethod("minus",paramListEMS);
	    lit = sl.iterator();
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in SupportOps.minus(SegMultiSet,TriMultiSet):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	//align all segments
	while (lit.hasNext()) {
	    ((Segment)((MultiSetEntry)lit.next()).value).align(); }
	
	//compute pairlists: 1 segment n intersecting triangles
	try {
	    boolean meet = true;
	    ljpl = SetOps.overlapLeftOuterJoin(sl,tl,m1,meet,bboxFilter,earlyExit,setNumber);
	} catch (EarlyExit exit) {
	    throw exit;
	}//catch
	
	//compute from leftjoinlist a new leftjoinlist with
	//Elem=actual segment and
	//ElemList=segment parts which have to be subtracted from actual segment
	ljpl = SetOps.subtractSets(ljpl,m4);
	
	//sutract all segment parts from actual segments in leftjoinpairlist
	ljpl = SetOps.map(ljpl,null,m7);
	retList = SegMultiSet.convert(SetOps.collect(ljpl));
	
	return retList;
    }//end method minus
    
   
    /**
     * Returns <code>true</code> if the set of segments completely lies inside of the triangle set.<p>
     * Actually, this method implements a <i>high level</i> operation which would be a part of a possible
     * <i>target algebra</i>. But since it is needed at some places inside of the 2D-SACK framework,
     * an implementation is provided here.
     * 
     * @param sl the set of segments
     * @param tl the set of triangles
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce the time spent)
     * @param earlyExit if <tt>true</tt>, execution is stopped immediately, when no overlapping pair is found for 
     *                  an element of the set specified by <code>setNumber</code>
     * @param setNumber specifies the setNumber
     * @return {<code>true</code>, <code>false</code>} depending on the mutual position of the objects
     * @throws EarlyExit
     */
    public static boolean lr_inside (SegMultiSet sl, TriMultiSet tl, boolean bboxFilter,boolean earlyExit, int setNumber) 
	throws EarlyExit {
	SegMultiSet retList = minus(sl,tl,bboxFilter,earlyExit,setNumber);
	if (retList.size() == 0) { return true; }
	else { return false; }
    }//end method lr_inside    
    

    /**
     * Returns <code>true</code> if the set of points completely lies inside of the triangle set.<p>
     * Actually, this method implements a <i>high level</i> operation which would be a part of a possible
     * <i>target algebra</i>. But since it is needed at some places inside of the 2D-SACK framework,
     * an implementation is provided here.
     *
     * @param ps the set of points
     * @param ts the set of triangles
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce the time spent for this operation
     * @return {<code>true</code>, <code>false</code>} depending on the mutual position of the objects
     */
    public static boolean pr_inside (PointMultiSet ps, TriMultiSet ts, boolean bboxFilter) {
	PointMultiSet retList = null;
	try {
	    Method m = ptOpsClass.getMethod("isCovered",paramListPT);
	    try {
		boolean meet = true;
		boolean earlyExit = true;
		int setNumber = 1;
		retList = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(ps,ts,m,meet,bboxFilter,earlyExit,setNumber))));
	    } catch (EarlyExit exit) {
		return false;
	    }//catch
	} catch (Exception e) {
	    System.out.println("Exception was thrown in SupportOps.pr_inside(PointMultiSet,TriMultiSet):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	if (ps.size() == retList.size()) return true;
	else return false;
    }//end method pr_inside


    /**
     * Converts a set from <code>ElemMultiSet</code> to <code>LinkedList</code> representation.
     * The resulting list is sorted by the comparator of the <code>ElemMultiSet</code>.
     *
     * @param inMS the passed <code>ElemMultiSet</code>
     * @return the same set in <code>LinkedList</code> format
     * @see #convert(LinkedList)
     */
    public static LinkedList convert(ElemMultiSet inMS) {
	LinkedList retList = new LinkedList();
	Iterator it = inMS.iterator();
	MultiSetEntry mse;
	int number;
	while (it.hasNext()) {
	    mse = (MultiSetEntry)it.next();
	    number = mse.number;
	    for (int i = 0; i < number; i++) {
		retList.add((Element)mse.value);
	    }//for i
	}//while it
	return retList;
    }//end method convert


    /**
     * Converts a list from <code>LinkedList</code> to <code>ElemMultiSet</code> representation.
     * 
     * @param inLL the passed <code>LinkedList</code>
     * @return the same list in <code>ElemMultiSet</code> format
     * @see #convert(ElemMultiSet)
     */
    public static ElemMultiSet convert(LinkedList inLL) {
	ElemMultiSet retSet = new ElemMultiSet(ELEM_COMPARATOR);
	ListIterator lit = inLL.listIterator(0);
	while (lit.hasNext())
	    retSet.add(lit.next());
	return retSet;
    }//end method convert
    
    
     /**
     * Returns that part of the first set of triangles, which is not covered by the second set of triangles.<p>
     * This method works as follows:
     * <p><ol>
     * <li>For each triangle of <code>tl1</code> find all overlapping triangles of <code>tl2</code>.
     * This set of pairs (<code>triangle x set of triangles</code>) is stored in a
     * <code>LeftJoinPairMultiSet</code>.</li>
     * <li>For each element (pair) of the <code>LeftJoinPairMultiSet</code> subtract the covered area
     * from the triangle.</li>
     * </ol><p>
     * Afterwards, the uncovered parts of triangles are returned.
     *
     * @param tl1 the first (covered) set of triangles
     * @param tl2 the second (covering) set of triangles
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce time spent for this operation)
     * @return the uncovered set of (new) triangles
     */
    public static TriMultiSet minus (TriMultiSet tl1, TriMultiSet tl2, boolean bboxFilter) {
	TriMultiSet retSet = null;
	
	Class c = (new Triangle()).getClass();
	Class c3 = (new ElemMultiSet(ELEM_COMPARATOR)).getClass();
	Class[] paramListT = { c };
	Class[] paramListTMS = { c3 };
	
	try {
	    Method m1 = c.getMethod("pintersects",paramListT);
	    Method m4 = c.getMethod("minus",paramListTMS);
	    
	    boolean meet = false;
	    boolean earlyExit = false;
	    int setNumber = 0;
	    LeftJoinPairMultiSet ljpMS = SetOps.overlapLeftOuterJoin(tl1,tl2,m1,meet,bboxFilter,earlyExit,setNumber);
	    ljpMS = SetOps.map(ljpMS,null,m4);
	    SegMultiSet retSetS = SegMultiSet.convert(SetOps.rdup2(SetOps.collect(ljpMS)));
	    retSetS = minimal(retSetS,true,false);
	    retSet = Polygons.computeMesh(retSetS,true);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.minus(TriMultiSet,TriMultiSet):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return retSet;
    }//end method minus


    /**
     * Returns the set of (not overlapping) triangles that covers the intersection of the passed two triangle sets.<p>
     * This method works as follows:
     * <p><ol>
     * <li>For each triangle of <code>ts1</code> find all overlapping triangles of <code>ts2</code>.
     * This set of pairs (<code>triangle x set of triagles</code>) is stored in a <code>LeftJoinPairMultiSet</code>.</li>
     * <li>For each element (pair) of the <code>LeftJoinPairMultiSet</code> compute the intersection.</li>
     * </ol><p>
     * Afterwards, the set of triangles representing the intersection is returned.
     *
     * @param ts1 the first set of triangles
     * @param ts2 the second set of triangles
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce the time spent for this operation)
     * @return the intersection of <tt>ts1, ts2</tt>
     */
    public static TriMultiSet intersection (TriMultiSet ts1, TriMultiSet ts2, boolean bboxFilter) {
	TriMultiSet retSet = null;

	Class c = (new Triangle()).getClass();
	Class c2 = (new ElemMultiSet(ELEM_COMPARATOR)).getClass();
	Class[] paramListT = { c };
	Class[] paramListTMS = { c2 };

	try {
	    Method pintersectsM = c.getMethod("pintersects",paramListT);
	    Method intersectionM = c.getMethod("intersection",paramListTMS);
	    
	    boolean meet = false;
	    boolean earlyExit = false;
	    int setNumber = 0;
	    LeftJoinPairMultiSet ljpMS = SetOps.overlapLeftOuterJoin(ts1,ts2,pintersectsM,meet,bboxFilter,earlyExit,setNumber);
	    ljpMS = SetOps.map(ljpMS,null,intersectionM);
	    SegMultiSet retSetS = SegMultiSet.convert(SetOps.rdup2(SetOps.collect2nd(ljpMS)));
	    retSetS = minimal(retSetS,true,false);
	    retSet = Polygons.computeMesh(retSetS,true);
	} catch (Exception e) {
	    System.out.println("SupportOps.intersection: Caught an unexpected exception: "+e);
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return retSet;
    }//end method intersection
    

    /**
     * Returns the set of (not overlapping) triangles building the union of the passed two triangle sets.
     * This method works as follows:
     * <p><ol>
     * <li>For the two triangle sets <tt>R</tt> and <tt>S</tt>,  <tt>Q = minus(S,R)</tt> is computed</li>
     * <li>compute <tt>disjointUnion(R,Q)</tt></li>
     * </ol><p>
     * At the end, the set of triangles representing the union is returned.
     *
     * @param ts1 the first set of triangles
     * @param ts2 the second set of triangles
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce the time spent for this operation)
     * @return the union of <tt>ts1, ts2</tt>
     */
    public static TriMultiSet plus (TriMultiSet ts1, TriMultiSet ts2, boolean bboxFilter) {
	TriMultiSet retSet = null;
	
	Class c = (new Triangle()).getClass();
	Class c2 = (new ElemMultiSet(ELEM_COMPARATOR)).getClass();
	Class[] paramListT = { c };
	Class[] paramListTMS = { c2 };
	SegMultiSet retSetS = null;

	try {
	    Method pintersectsM = c.getMethod("pintersects",paramListT);
	    Method minusM = c.getMethod("minus",paramListTMS);

	    boolean meet = false;
	    boolean earlyExit = false;
	    int setNumber = 0;
	    LeftJoinPairMultiSet ljpMS = SetOps.overlapLeftOuterJoin(ts2,ts1,pintersectsM,meet,bboxFilter,earlyExit,setNumber);
	    ljpMS = SetOps.map(ljpMS,null,minusM);
	    retSetS = SegMultiSet.convert(SetOps.rdup2(SetOps.collect(ljpMS)));
	    retSetS = minimal(retSetS,true,false);
	    retSet = Polygons.computeMesh(retSetS,true);
	} catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.plus(TriMultiSet,TriMultiSet,boolean):");
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	//compute union
	retSet.addAll(ts1);
		
	retSetS = contourGeneral(retSet,true,true,true,true);
	retSet = Polygons.computeMesh(retSetS,true);

	return retSet;

    }//end method plus
	   
}//end class SupportOps
