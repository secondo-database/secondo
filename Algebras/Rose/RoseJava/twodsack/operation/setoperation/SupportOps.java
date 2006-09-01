/*
 * SupportOps.java 2004-11-10
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.operation.setoperation;

import twodsack.io.*;
import twodsack.operation.basictypeoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.graph.*;
import twodsack.util.number.*;

import java.lang.reflect.Method;
import java.util.Arrays;
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
    final private static int INITIAL_CAPACITY = 499;

    static final Class pointClass = (new Point()).getClass();
    static final Class segClass = (new Segment()).getClass();
    static final Class triClass = (new Triangle()).getClass();

    static final Class ptOpsClass = (new PointTri_Ops()).getClass();
    static final Class ssOpsClass = (new SegSeg_Ops()).getClass();
    static final Class stOpsClass = (new SegTri_Ops()).getClass();
    static final Class ttOpsClass = (new TriTri_Ops()).getClass();
    static final Class emsClass = (new ElemMultiSet(new ElemComparator())).getClass();

    final static Class[] paramListT = { triClass };
    final static Class[] paramListPT = { pointClass, triClass };
    final static Class[] paramListTT = { triClass, triClass };
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
     * @param handleCycles if <tt>true</tt>, the set of segments is divided into cycles it may build. Then,
     *                     minimal is invoked on every cycle.
     * @return the new, reduced set of segments as <code>SegMultiSet</code>
     */
    public static SegMultiSet minimal (SegMultiSet sl, boolean overlap, boolean bboxFilter, boolean handleCycles) {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	Class c = ssOpsClass;

	try {
	    Method m1 = c.getMethod("adjacent",paramListSS);
	    Method m2 = c.getMethod("concat",paramListSS);

	    if (handleCycles) {
		//compute the cycles of sl
		Graph myGraph = new Graph(sl);
		ElemMultiSetList emsl = myGraph.computeFaces();
		//compute minimal for every single cycle of emsl
		for (int i = 0; i < emsl.size(); i++)
		    if (!overlap)
			retList.addAll(SegMultiSet.convert(SetOps.reduce((ElemMultiSet)emsl.get(i),m1,m2)));
		    else {
			try {
			    boolean meet = true;
			    boolean earlyExit = false;
			    int setNumber = 0;
			    retList.addAll(SegMultiSet.convert(SetOps.overlapReduceSweep((ElemMultiSet)emsl.get(i),m1,m2,meet)));
			} catch (Exception e) {
			    System.out.println("SupportOps.minimal: Caucht unexpected exception: "+e);
			    e.printStackTrace();
			    throw new RuntimeException("An error occurred in the 2DSACK package.");
			}//catch
		    }//else
	    }//if handleCycles
	    else {	
		if (!overlap)
		    retList = SegMultiSet.convert(SetOps.reduce(sl,m1,m2));
		else {
		    try {
			boolean meet = true;
			boolean earlyExit = false;
			int setNumber = 0;
			//this is the 'old' version of reduce that uses overlapReduce instead of overlapReduceSweep
			//retList = SegMultiSet.convert(SetOps.overlapReduce(sl,m1,m2,meet,bboxFilter,earlyExit,setNumber)); 
			retList = SegMultiSet.convert(SetOps.overlapReduceSweep(sl,m1,m2,meet));
		    } catch (Exception e) {
			System.out.println("SupportOps.minimal: Caught unexpected exception: "+e);
			e.printStackTrace();
			throw new RuntimeException("An error occurred in the 2DSACK package.");
		    }//catch 
		}//else
	    }//else
	    }//try
	    catch (Exception e) {
		System.out.println("Exception was thrown in SupportOps.minimal(SegMultiSet,boolean):");
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		e.printStackTrace();
		throw new RuntimeException("An error occurred in the 2DSACK package.");
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
     * @param sweep If <tt>true</code>, the fastest algorithm is used: <tt>SetOps.overlapReduceSweep2</tt>.
     * @param bboxFilter if <tt>true</tt>, a bounding box filter is used (may help to reduce time spent)
     * @return the new, reduced set of segments as <code>SegMultiSet</code>
     */
    public static SegMultiSet unique (SegMultiSet sl, boolean overlap, boolean sweep, boolean bboxFilter) {
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	Class c = ssOpsClass;
	
	//remove duplicates with SetOps.rdup2
	SetOps.rdup2(sl);
	
	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    Method m2 = c.getMethod("symDiff",paramListSS);
	    if (!overlap && !sweep)
		retList = SegMultiSet.convert(SetOps.reduce(sl,m1,m2));
	    else { 
		try {
		    boolean meet = true;
		    boolean earlyExit = false;
		    int setNumber = 0;

		    if (!sweep)
			retList = SegMultiSet.convert(SetOps.overlapReduce(sl,m1,m2,meet,bboxFilter,earlyExit,setNumber));
		    else
			retList = SegMultiSet.convert(SetOps.overlapReduceSweep2(sl,m1,m2,meet));
		} catch (Exception e) {
		    System.out.println("SupportOps.unique: Caught unexpected exception: "+e);
		    e.printStackTrace();
		    throw new RuntimeException("An error occurred in the 2DSACK package.");
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
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
	}//catch
	return SegMultiSet.convert(SetOps.rdup2(retList));
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
     * @param uniSweep If<tt>true</tt>, the fastest algorithm is used for <tt>unique</tt>.
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce time spent for this operation)
     * @param computeMinimalSet if <code>true</code>, <code>minimal</code> is used to reduce the set of segments in the
     *                          resulting set
     * @param handleCycles this parameter is passed to {@link #minimal(SegMultiSet,boolean,boolean,boolean)}
     * @return the contour as <code>SegMultiSet</code>
     * @see #minimal(SegMultiSet,boolean,boolean,boolean)
     * @see #unique(SegMultiSet,boolean,boolean,boolean)
     */
    public static SegMultiSet contourGeneral (TriMultiSet tl,boolean minOverlap,boolean uniOverlap,boolean uniSweep,boolean bboxFilter, boolean computeMinimalSet, boolean handleCycles) {
	Class c = triClass;
	SegMultiSet retList = new SegMultiSet(SEGMENT_COMPARATOR);
	try {
	    Method m = c.getMethod("segmentArray",null);
	    if (computeMinimalSet) 
		retList = minimal(unique(SegMultiSet.convert(SetOps.map(tl,m)),uniOverlap,uniSweep,bboxFilter),minOverlap,bboxFilter,handleCycles);
	    else
		retList = unique(SegMultiSet.convert(SetOps.map(tl,m)),uniOverlap,uniSweep,bboxFilter);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in SupportOps.contourGeneral(TriMultiSet,boolean,boolean):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Exception cause: "+e.getCause());
	    System.out.println("Exception string: "+e.toString());
	    System.out.println("stack trace:"); e.printStackTrace();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
	}//catch
	return retList;
    }//end method contourGeneral


    /**
     * The contour of a triangle set (representing a polygon) is computed with this method.<p>
     * When using this method instead of <code>contourGeneral</code> it is assumed, that the triangle set
     * was computed using the methods of class {@link twodsack.setelement.datatype.compositetype.Polygons}. In that case, bounding segments of the 
     * triangle set don't overlap. This method doesn't work for triangles with overlapping (instead of identical) segments.
     * {@link #minimal(SegMultiSet,boolean,boolean,boolean)} is executed on the resulting set to combine collinear and adjacent segments,
     * if <tt>minimal</tt> parameter is set.
     *
     * @param ts the set of triangles
     * @param minimal if <tt>true</tt>, <tt>minimal</tt> is invoked on the resulting segment set.
     * @param handleCycles this parameter is passed to {@link #minimal(SegMultiSet,boolean,boolean,boolean)}
     * @return the contour as <code>SegMultiSet</code>
     * @see #contourGeneral(TriMultiSet,boolean,boolean,boolean,boolean,boolean,boolean)
     */
    public static SegMultiSet contour (TriMultiSet ts, boolean minimal, boolean handleCycles) {
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

	if (minimal)
	    retSet = minimal(SegMultiSet.convert(SetOps.rdup2(retSet)),true,false,handleCycles);
	else
	    retSet = SegMultiSet.convert(SetOps.rdup2(retSet));
	
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
	Method m4 = null;
	Method m7 = null;
	LeftJoinPairMultiSet ljpl;

	try {
	    paramList[0] = Class.forName("twodsack.set.ElemMultiSet");
	    paramList[1] = Class.forName("twodsack.set.ElemMultiSet");
	    m1 = c.getMethod("isCovered",paramListST);
	    paramList[2] = m1.getClass();
	    paramList[3] = m1.getClass();

	    ljpl =  new LeftJoinPairMultiSet();
	    m4 = c.getMethod("intersection",paramListST);
	    m7 = segClass.getMethod("minus",paramListEMS);
	    lit = sl.iterator();
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in SupportOps.minus(SegMultiSet,TriMultiSet):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
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
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
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

	    double mill1 = System.currentTimeMillis();

	    LeftJoinPairMultiSet ljpMS = SetOps.overlapLeftOuterJoin(tl1,tl2,m1,meet,bboxFilter,earlyExit,setNumber);

	    double mill2 = System.currentTimeMillis();

	    ljpMS = SetOps.map(ljpMS,null,m4);

	    double mill3 = System.currentTimeMillis();

	    SegMultiSet retSetS = SegMultiSet.convert(SetOps.rdup2(SetOps.collect(ljpMS)));

	    double mill4 = System.currentTimeMillis();

	    retSetS = minimal(retSetS,true,false,true);

	    double mill5 = System.currentTimeMillis();

	    retSet = Polygons.computeMesh(retSetS,true);

	    double mill6 = System.currentTimeMillis();
	    System.out.println("SupportOps.minus: ");
	    System.out.println("time for overlapLeftOuterJoin: "+((mill2-mill1)/1000)+" s");
	    System.out.println("time for map: "+((mill3-mill2)/1000)+" s");
	    System.out.println("time for convert/rdup2/collect: "+((mill4-mill3)/1000)+" s");
	    System.out.println("time for minimal: "+((mill5-mill3)/1000)+" s");
	    System.out.println("time for computMesh: "+((mill6-mill5)/1000)+" s");
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.minus(TriMultiSet,TriMultiSet):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
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
	    retSetS = minimal(retSetS,true,false,true);
	    retSet = Polygons.computeMesh(retSetS,true);
	} catch (Exception e) {
	    System.out.println("SupportOps.intersection: Caught an unexpected exception: "+e);
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
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
     * Using the <tt>recomputeTriangleSet</tt> switch, the user may decide, whether for two polygons, which don't
     * overlap, but meet, the result is re-triangulated or not. The number um triangles is reduced in most cases,
     * when the result is re-triangulated.<p>
     * Note: When setting <tt>recomputeTriangleSet = false</tt>, the resulting triangle set cannot be used with
     * {@link #contour(TriMultiSet,boolean,boolean)} no more. The more expensive {@link #contourGeneral(TriMultiSet,boolean,boolean,boolean,boolean,boolean,boolean)} has to be used to compute the contour
     * of such a triangle set.
     *
     * @param ts1 the first set of triangles
     * @param ts2 the second set of triangles
     * @param bboxFilter if <code>true</code>, a bounding box filter is used (may reduce the time spent for this operation)
     * @param recomputeTriangleSet if <tt>true</tt> the result of the <tt>plus</tt> operation is triangulated for two
     *        polygons which don't overlap but meet only
     * @return the union of <tt>ts1, ts2</tt>
     */
    public static TriMultiSet plus (TriMultiSet ts1, TriMultiSet ts2, boolean bboxFilter, boolean recomputeTriangleSet) {


	/**
	 * The following implementation can be used to apply some kind of filtering mechanism on the triangle sets.
	 * If used, both sets are traversed first to find the faces of the regions. After having done that, these faces are
	 * checked for overlapping pairs (with one face element of first set and the other set element of the other set).
	 * Then, the PLUS operation is computed only for those pairs, while the triangulation of the other faces remains
	 * unchanged.
	 * This means a dramatic reduction of triangulation costs, when working properly. Unfortunately, the costs for
	 * computing the faces are very high. In fact, not the faces (i.e. their borders) have to be computed, but the
	 * triangle sets representing the faces. This is done via the GROUP operation using the ADJACENT predicate. For
	 * a huge triangle set this operation is extremely expensive due to the construction of the graph. The graph construction
	 * is very expensive, since for every triangle, 2 new() operations are called (once for an insertion in a hash table
	 * and another time for the construction of an adjacency list).
	 * These are the reasons, why this mechanism is NOT used. Instead, we use only a small part of this algorithm, namely
	 * the first filtering step:
	 * Instead of building triangle groups, we compute the faces (i.e. segment groups) of the regions. Then, we check 
	 * for overlapping pairs of faces (just like explained above). If no faces (i.e. their bounding boxes) overlap, the triangle
	 * sets are simply unified. Otherwise, minus is computed for the whole triangle set.
	 */
	/* OLD CODE as described above
	   System.out.println("Entering SO.plus.");
	   
	   Runtime rt = Runtime.getRuntime();
	   
	   System.out.println("JAVA memory total: "+Double.toString(((int)(rt.totalMemory()/1048.567))/1000.0)+" mb");
	   System.out.println("JAVA memory free: "+Double.toString(((int)(rt.freeMemory()/1048.567))/1000.0)+" mb\n");
	   
	   double x0 = System.currentTimeMillis();
	   PairSet ps = overlappingPairs(ts1,ts2,recomputeTriangleSet);
	   double x1 = System.currentTimeMillis();
	   System.out.println("time for ovPairs: "+(x1-x0)+" ms");
	   
	   System.out.println("overlappingPairs found: "+ps.pairSet.size());
	   TriMultiSet retSet = new TriMultiSet(TRIANGLE_COMPARATOR);
	   
	   //if number of pairs found == 0, then just sum up triangle sets and return
	   if (ps.pairSet.size() == 0) {
	   retSet.addAll(ts1);
	   retSet.addAll(ts2);
	   return retSet;
	   }//if
	   
	   //...otherwise, compute plus for the pairs and add all regions which are not involved in pairs directly
	   //put in the retSet all unused triangles from tms1 (which are now in ps.firstSet and
	   //put in the set the results of the minus operation invoked on the pairs of ps.pairSet
	   
	   retSet.addAll(ps.firstSet);
	   retSet.addAll(ps.secondSet);
	   
	   //compute minus for all pairs in ps.pairSet
	   Iterator it = ps.pairSet.iterator();
	   Class c2 = (new ElemMultiSet(ELEM_COMPARATOR)).getClass();
	   Class[] paramListT = { triClass };
	   Class[] paramListTMS = { c2 };
	   SegMultiSet retSetS = new SegMultiSet(SEGMENT_COMPARATOR);
	   Method pintersectsM, minusM;
	   
	   try {
	   pintersectsM = triClass.getMethod("pintersects",paramListT);
	   minusM = triClass.getMethod("minus",paramListTMS);
	   } catch (Exception e) {
	   System.out.println("Caught an exception in SupportOps.plus.");
	   e.printStackTrace();
	   System.out.println("Returning empty value.");
	   return new TriMultiSet(TRIANGLE_COMPARATOR);
	   }//catch
	   
	   boolean meet = false;
	   boolean earlyExit = false;
	   int setNumber = 0;
	   LeftJoinPairMultiSet ljpMS;
	   ElemPair actPair;
	   int itno = 0;
	   
	   while (it.hasNext()) {
	   System.out.println("\nexamine pair "+itno+" of "+(ps.pairSet.size()-1)); itno++;
	   actPair = (ElemPair)((MultiSetEntry)it.next()).value;
	   
	   try {
	   //ljpMS = SetOps.overlapLeftOuterJoin(((EMSPointer)actPair.second).set,((EMSPointer)actPair.first).set,pintersectsM,meet,bboxFilter,earlyExit,setNumber);
	   //recompute sizes for sets
	   ((EMSPointer)actPair.first).set.recomputeSize();
	   ((EMSPointer)actPair.second).set.recomputeSize();
	   ljpMS = SetOps.overlapLeftOuterJoin(((EMSPointer)actPair.first).set,((EMSPointer)actPair.second).set,pintersectsM,meet,bboxFilter,earlyExit,setNumber);
	   
	   } catch (Exception e) {
	   System.out.println("Caught unexpected exception. Returning empty value.");
	   e.printStackTrace();
	   return new TriMultiSet(TRIANGLE_COMPARATOR);
	   }//catch
	   
	    //if the number of overlapping triangles is 0, re-retriangulate, if switch RECOMPUTETRIANGLESET=true
	    //if RECOMPUTETRIANGLESET=false, simply add triangle set to result set
	    System.out.println("\nnumber of overlapping pairs: "+ljpMS.size());
	    Iterator sit = ljpMS.iterator();
	    int cc = 0;
	    while (sit.hasNext()) {
	    LeftJoinPair pp = (LeftJoinPair)((MultiSetEntry)sit.next()).value;
	    if (pp.elemSet != null) cc++;
	    }
	    
	    System.out.println("non-empty pairs: "+cc);
	    
	    if (recomputeTriangleSet) {
	    ljpMS = SetOps.map(ljpMS,null,minusM);
	    retSetS.addAll(SegMultiSet.convert(SetOps.collect(ljpMS)));
	    //System.out.println("retSetS after adding ljpMS: "); retSetS.print();
	    } else {
	    System.out.println("simply adding sets");
	    retSet.addAll(((EMSPointer)actPair.second).set);
	    retSet.addAll(((EMSPointer)actPair.first).set);
	    }//else
	    }//while it
       
	    if (recomputeTriangleSet) {
	    //now, add to retSetS all second sets which are involved (add them one time only)
 	    retSetS.addAll(contour(TriMultiSet.convert(ps.fourthSet),true,false));
	    retSetS = unique(retSetS,true,false);
	    retSetS = minimal(retSetS,true,false,true);
	    retSet.addAll(Polygons.computeMesh(retSetS,false));
	    }//if
	OLD CODE as decribed above */
	
	/* NEW CODE */
	//test triangle sets for faces with overlapping bounding boxes
	//System.out.println("SO.contour");
	SegMultiSet contour1, contour2;
	
	if (recomputeTriangleSet) {
	    contour1 = contour(ts1,true,true);
	    contour2 = contour(ts2,true,true);
	} else {
	    contour1 = contourGeneral(ts1,true,true,true,false,true,true);
	    contour2 = contourGeneral(ts2,true,true,true,false,true,true);
	}//else
	
	//construct graphs from contours
	//System.out.println("SO.graphs");
	Graph graph1 = new Graph(contour1);
	Graph graph2 = new Graph(contour2);
	
	//compute cycles from graphs
	//System.out.println("SO.computeFaces");
	ElemMultiSetList groups1 = graph1.computeFaces();
	ElemMultiSetList groups2 = graph2.computeFaces();

	//wrap sets in EMSPointers and store them in ElemMultiSets
	Iterator git = groups1.iterator();
	int number = 0;
	ElemMultiSet actSet;
	ElemMultiSet groups1EMS = new ElemMultiSet(ELEM_COMPARATOR);
	while (git.hasNext()) {
	    actSet = (ElemMultiSet)git.next();
	    groups1EMS.add(new EMSPointer(actSet,actSet.rect(),number));
	    number++;
	}//while
	
	git = groups2.iterator();
	number = 0;
	ElemMultiSet groups2EMS = new ElemMultiSet(ELEM_COMPARATOR);
	while (git.hasNext()) {
	    actSet = (ElemMultiSet)git.next();
	    groups2EMS.add(new EMSPointer(actSet,actSet.rect(),number));
	    number++;
	}//while git
	
	//compute pairs of overlapping EMSPointers
	PairMultiSet pairs = null;
	try {
	    pairs = SetOps.overlappingPairs(groups1EMS,groups2EMS,false,true,false,false,-1);
	} catch (Exception e) {
	    System.out.println("Caught an unexpected exception in SupportOps.overlappingPairs.");
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
	}//catch
	

	TriMultiSet retSet = new TriMultiSet(TRIANGLE_COMPARATOR);
	
	//if number of pairs found == 0, then just sum up triangle sets and return
	if (pairs.size() == 0) {
	    retSet.addAll(ts1);
	    retSet.addAll(ts2);
	    return retSet;
	}//if

	//...otherwise, compute plus for complete triangle sets. This is done by computing
	//retSet = (tms2 - tms1) + tms1
	//compute (tms2 - tms1)
	Method methodPINT = null;
	Method methodMINUS = null;
	LeftJoinPairMultiSet ljpMS = null;
	try {
	    methodPINT = triClass.getMethod("pintersects",paramListT);
	    methodMINUS = triClass.getMethod("minus",paramListEMS);
	    ljpMS = SetOps.overlapLeftOuterJoin(ts2,ts1,methodPINT,false,bboxFilter,false,-1);
	} catch (Exception e) {
	    System.out.println("Caught exception in SupportOps.plus.");
	    e.printStackTrace();
	    System.out.println("Returning empty value.");
	    return new TriMultiSet(TRIANGLE_COMPARATOR);
	}//catch

	ljpMS = SetOps.map(ljpMS,null,methodMINUS);
	SegMultiSet retSetS = SegMultiSet.convert(SetOps.collect(ljpMS));
	retSetS.addAll(contour(ts1,false,false));
	retSetS = unique(retSetS,true,true,false);
	retSetS = minimal(retSetS,true,false,true);
	retSet = Polygons.computeMesh(retSetS,false);	
	/* END of NEW CODE */

	return retSet;
    }//end method plus


    /**
     * Computes pairs of polygons which have overlapping bounding boxes.
     * This method first groups the triangles of every triangle in such a way, that every group builds a polygon which is not 
     * adjacent to any other group. After that, bounding boxes are computed for every polygon. Then, pairs of polygons are computed
     * where the bounding boxes of the polygons overlap. These pairs are returned in the <tt>PairSets</tt> return type together
     * with two sets of polygons (e.g. triangle sets) of both initial sets which are not involved in those pairs.
     *
     * @param tms1 the first set of triangles
     * @param tms2 the second set of triangles
     * @param simpleContour if <tt>true</tt>, the simpler (and cheaper) <tt>contour</tt> method is used instead of the more expensive
     *                      <tt>contourGeneral</tt>
     * @return the resulting sets stored in a <tt>PairSet</tt>
     */
    public static PairSet overlappingPairs (TriMultiSet tms1, TriMultiSet tms2, boolean simpleContour) {
	//compute groups for both triangle sets
	double tt01 = System.currentTimeMillis();
	ElemMultiSetList groups1, groups2;
	PairMultiSet pairs = null;

	/**
	 * This piece of code must be used instead of the code below marked with NEW CODE, if the special filtering mechanism
	 * described in PLUS method shall be used.
	 * OLD CODE using SetOps.group to find groups; this is very slow.
	 
	 Method padjacent = null;
	 
	 try {
	 padjacent = ttOpsClass.getMethod("adjacent",paramListTT);
	 } catch (Exception e) {
	 System.out.println("Error in SupportOps.overlappingPairs.");
	 e.printStackTrace();
	 System.exit(0);
	 }//catch
	 
	 double tt02 = System.currentTimeMillis();
	 
	 groups1 = SetOps.overlapGroup(tms1,padjacent,true);
	 groups2 = SetOps.overlapGroup(tms2,padjacent,true);
	 
	 double tt03 = System.currentTimeMillis();
	 
	 END of OLD CODE */
	
	/* NEW CODE using Graph.computeCycles to find groups; this is faster (hopefully) */
	//first, compute contours of both sets
	SegMultiSet contour1,contour2;
	
	if (simpleContour) {
	    contour1 = contour(tms1,true,true);
	    contour2 = contour(tms2,true,true);
	} else {
	    contour1 = contourGeneral(tms1,true,true,true,false,true,true);
	    contour2 = contourGeneral(tms2,true,true,true,false,true,true);
	}//else
	
	double tt02 = System.currentTimeMillis();

	//construct graphs from contours
	Graph graph1 = new Graph(contour1);
	Graph graph2 = new Graph(contour2);
	
	double tt021 = System.currentTimeMillis();
	
	//compute cycles from graphs
	groups1 = graph1.computeFaces();
	groups2 = graph2.computeFaces();
	
	double tt03 = System.currentTimeMillis();
	/* END of NEW CODE */
	
	//wrap sets in EMSPointers and store them in ElemMultiSets
	Iterator git = groups1.iterator();
	int number = 0;
	ElemMultiSet actSet;
	ElemMultiSet groups1EMS = new ElemMultiSet(ELEM_COMPARATOR);
	while (git.hasNext()) {
	    actSet = (ElemMultiSet)git.next();
	    groups1EMS.add(new EMSPointer(actSet,actSet.rect(),number));
	    number++;
	}//while
	
	double tt04 = System.currentTimeMillis();

	git = groups2.iterator();
	number = 0;
	ElemMultiSet groups2EMS = new ElemMultiSet(ELEM_COMPARATOR);
	while (git.hasNext()) {
	    actSet = (ElemMultiSet)git.next();
	    groups2EMS.add(new EMSPointer(actSet,actSet.rect(),number));
	    number++;
	}//while git
	
	double tt05 = System.currentTimeMillis();

	//compute pairs of overlapping EMSPointers
	try {
	    pairs = SetOps.overlappingPairs(groups1EMS,groups2EMS,false,true,false,false,-1);
	} catch (Exception e) {
	    System.out.println("Caught an unexpected exception in SupportOps.overlappingPairs.");
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
	}//catch
	
	double tt06 = System.currentTimeMillis();

	//build return set
	//construct boolean arrays for groups1EMS and groups1EMS, where TRUE stands for "is used in pairs"
	boolean[] groups1Used = new boolean[groups1EMS.size()];
	boolean[] groups2Used = new boolean[groups2EMS.size()];

	//System.out.println("constructed arrays of size "+groups1Used.length+" and "+groups2Used.length);

	double tt07 = System.currentTimeMillis();

	//initialize boolean arrays
	for (int i = 0; i < groups1Used.length; i++) groups1Used[i] = false;
	for (int i = 0; i < groups2Used.length; i++) groups2Used[i] = false;
	
	//traverse pairs and build new PairMultiSet, where the first element is a EMS of groups1EMS and
	//the second set is the union of all EMS of groups2EMS which occur as pairs of the first element
	//in PAIRS. e.g.:
	//pairs: <1,12> <1,13> <2,14> <3,20> <3,21>
	//pairSet: <1, (union of 12,13)> <2,14> <3, (union of 20,21)>

	double tt08 = System.currentTimeMillis();

	PairSet retSet = new PairSet();
	
	Iterator it = pairs.iterator();
	EMSPointer actGroup1,actGroup2;
	ElemPair actPair = null;
	int lastKey = -1;
	MultiSetEntry mse;
	while (it.hasNext()) {
	    mse = (MultiSetEntry)it.next();
	    actGroup1 = (EMSPointer)(((ElemPair)mse.value).first);
	    actGroup2 = (EMSPointer)(((ElemPair)mse.value).second);
	    //mark groups in boolean arrays
	    groups1Used[actGroup1.key] = true;
	    groups2Used[actGroup2.key] = true;
	    
	    //a pair with a new group was found
	    if (lastKey != actGroup1.key) {
		//construct new entry in retPair
		ElemPair newPair = new ElemPair(actGroup1,actGroup2);
		actPair = newPair;
		retSet.pairSet.add(newPair);
		lastKey = actGroup1.key;
	    } else {
		//a pair with the actual key already exists
		//add triangles of actGroup2 to actual pair
		((EMSPointer)(actPair.second)).set.addAll(actGroup2.set);
	    }//else
	}//while

	double tt09 = System.currentTimeMillis();

	//now traverse group sets and store all elements from unused sets in retSet.firstSet
	//and retSet.secondSet; store used elements of retSet.firstSet in thirdSet and used
	//elements of retSet.secondet in fourthSet
	it = groups1EMS.iterator();
	while (it.hasNext()) {
	    actGroup1 = (EMSPointer)((MultiSetEntry)it.next()).value;
	    if (!groups1Used[actGroup1.key])
		retSet.firstSet.addAll(actGroup1.set);
	    else
		retSet.thirdSet.addAll(actGroup1.set);
	}//while

	it = groups2EMS.iterator();
	while (it.hasNext()) {
	    actGroup2 = (EMSPointer)((MultiSetEntry)it.next()).value;
	    if (!groups2Used[actGroup2.key])
		retSet.secondSet.addAll(actGroup2.set);
	    else
		retSet.fourthSet.addAll(actGroup2.set);
	}//while
	
	double tt10 = System.currentTimeMillis();
	/*
	  System.out.println("retSet: "+retSet.firstSet.size()+" element(s) in firstSet; "+retSet.secondSet.size+" element(s) in secondSet; "+retSet.pairSet.size+" pairs in pairSet");
	  
	  System.out.println("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	  System.out.println("time spent for: ");
	  System.out.println("contour: "+(tt021-tt01)+" ms");
	  System.out.println("construct graphs: "+(tt02-tt021)+" ms");
	  System.out.println("computeFaces: "+(tt03-tt02)+" ms");
	  System.out.println("wrap sets: "+(tt04-tt03)+" ms");
	  System.out.println("wrap sets2: "+(tt05-tt04)+" ms");
	  System.out.println("pairs of overlapping pointers: "+(tt06-tt05)+" ms");
	  System.out.println("construct arrays: "+(tt07-tt06)+" ms");
	  System.out.println("initialize: "+(tt08-tt07)+" ms");
	  System.out.println("sort pointers: "+(tt09-tt08)+" ms");
	  System.out.println("traverse group sets: "+(tt10-tt09)+" ms");
	*/
	  
	return retSet;
    }//end method overlappingPairs
	   
    /**
     * This method evaluates the intersects and meets predicate for special cases.
     * A special case occurs, if an endpoint of a segment S of SET1 lies on an endpoint of T of SET2. Another special
     * case is an endpoint of S which lies on T, but not on its endpoint. Both of these cases are treated here.
     *
     * @param pms the set of pairs of segments that have common points
     * @return a boolean array with two elements; the first value stands for the result of the intersects predicate and
     *         the second value represents the meets predicate
     */
    static public Boolean[] evaluateIntersectionAndMeets (PairMultiSet pms) {
	Method methodFormALine,methodSecondHasPointOnFirst;
	try {
	    methodSecondHasPointOnFirst = ssOpsClass.getMethod("secondHasPointOnFirst",paramListSS);
	    	    
	    ElemMultiSet set1 = new ElemMultiSet(ELEM_COMPARATOR);
	    ElemMultiSet set2 = new ElemMultiSet(ELEM_COMPARATOR);
	    SetOps.separateSets(pms,set1,set2);

	    Boolean[] retArr = SupportOps.checkForIntersectionAndMeetsSameEndpoints(SegMultiSet.convert(set1),
										    SegMultiSet.convert(set2));

	    if (retArr[0].booleanValue() == true) return retArr;
	    
	    //Now we can still have a special case, where one (or more) segment(s) of S1 has(have) an endpoint somewhere on a
	    //segment of S2, but _not_ on its endpoint. To extract those cases, collect all elements from the pair set and
	    //construct two LeftJoinPairSets from them using an "endpoint_on_segment" predicate; one for each 'direction'. Then,
	    //if a segment has only one element in ist elemSet, meets=true and intersects=false. If there are more than one
	    //elements in elemSet, test for the sides on which their endpoints lie. If they are on different sides of the
	    //key element in a LeftJoinPair, then intersects=true and meets=false. If they are on the same side,
	    //intersects=false and meets=true.	
	    LeftJoinPairMultiSet ljp1,ljp2;
	    ljp1 = SetOps.overlapLeftOuterJoin(set1,set2,methodSecondHasPointOnFirst,true,false,false,-1);
	    ljp2 = SetOps.overlapLeftOuterJoin(set2,set1,methodSecondHasPointOnFirst,true,false,false,-1);
	    
	    Boolean[] retArr2 = SupportOps.checkForIntersectionAndMeets(ljp1,ljp2);

	    if (retArr[0].booleanValue() || retArr2[0].booleanValue()) retArr[0] = Boolean.TRUE;
	    if (retArr[1].booleanValue() || retArr2[1].booleanValue()) retArr[1] = Boolean.TRUE;
	    
	    return retArr;
	} catch (Exception e) {
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in SupportOps.evaluateIntersectionAndMeets.");
	}//catch
    }//end method evaluateIntersectionAndMeets
    

    /**
     * This methods checks two sets of segments for the intersects and meets predicate.
     * It treats a special case, where pairs of segments have the same endpoints. This method works as follows:
     * First, it constructs a graph for every segment set and gets a sorted version of the graphs' vertices.
     * Then, these vertex arrays are traversed in parallel. For every pair of equal vertices, we have to 
     * examine, whether any of the both predicates holds. Consider the vertex we are currently examining as the
     * middle point M. Then, all outgoing edges end up in other points V1 and V2, depending on the graph they
     * belong to. We start now at an arbitrary point and define the degree D at this point as 0°. All other points Vx
     * are sorted. Then, these vertices are traversed in ascending D-order. We count the number of changes
     * from vertices of V1 to V2 and vice versa. If the number gets higher than 2, the intersects predicate holds.
     * in all other cases, the meets predicate holds.<p>
     * If there are no equal vertices in both of the vertex arrays, none of the predicates holds.
     */
    private static Boolean[] checkForIntersectionAndMeetsSameEndpoints(SegMultiSet sms1, SegMultiSet sms2) {
	Boolean[] retArr = { Boolean.FALSE, Boolean.FALSE };

	//construct graphs from segsets
	Graph graph1 = new Graph(sms1);
	Graph graph2 = new Graph(sms2);
	
	Vertex[] g1vArr = graph1.getSortedVertexArray();
	Vertex[] g2vArr = graph2.getSortedVertexArray();
	
	//traverse both arrays in parallel
	int arrIdx1 = 0;
	int arrIdx2 = 0;
	boolean getNext1 = true;
	boolean getNext2 = true;
	Vertex actV1 = null;
	Vertex actV2 = null;
	Vertex[] neighbours1 = null;
	Vertex[] neighbours2 = null;

	while ((arrIdx1 < g1vArr.length || !getNext1) && 
	       (arrIdx2 < g2vArr.length || !getNext2)) {

	    if (getNext1) {
		//arrIdx1++;
		actV1 = g1vArr[arrIdx1];
		getNext1 = false;
	    }//if
	    if (getNext2) {
		//arrIdx2++;
		actV2 = g2vArr[arrIdx2];
		getNext2 = false;
	    }//if
	    
	    //if vertices are equal, get neighbours
	    if (actV1.equal(actV2)) {
		neighbours1 = graph1.getNeighbours(actV1);
		neighbours2 = graph2.getNeighbours(actV2);
		
		//test lengthes of neighbour arrays
		if ((neighbours1.length == 1 && neighbours2.length == 1) ||
		    (neighbours1.length > 1 && neighbours2.length == 1) ||
		    (neighbours1.length == 1 && neighbours2.length > 1)) {
		    retArr[1] = Boolean.TRUE;
		} else {
		    //traverse both arrays in parallel using the angularly sorting
		    double[] angle1 = new double[neighbours1.length];
		    double[] angle2 = new double[neighbours2.length];
		    //compute the angles of the other vertices compared to the 0° which is a 
		    //horizontally drawn segment from the center point to the right.
		    Point midP = (Point)actV1.value;
		    Point startP = new Point(midP.x.plus(RationalFactory.constRational(10)),midP.y);

		    for (int i = 0; i < angle1.length; i++)
			if (Mathset.pointPosition(midP,startP,(Point)neighbours1[i].value) <= 0)
			    angle1[i] = Mathset.angleD(Mathset.diff(midP,startP),
						       Mathset.diff(midP,(Point)neighbours1[i].value));
			else 
			    angle1[i] = 360 - Mathset.angleD(Mathset.diff(midP,startP),
							     Mathset.diff(midP,(Point)neighbours1[i].value));
		    for (int i = 0; i < angle2.length; i++)
			if (Mathset.pointPosition(midP,startP,(Point)neighbours2[i].value) <= 0)
			    angle2[i] = Mathset.angleD(Mathset.diff(midP,startP),
						       Mathset.diff(midP,(Point)neighbours2[i].value));
			else
			    angle2[i] = 360 - Mathset.angleD(Mathset.diff(midP,startP),
							     Mathset.diff(midP,(Point)neighbours2[i].value));

		    //sort arrays
		    Arrays.sort(angle1);
		    Arrays.sort(angle2);
		    
		    //traverse arrays in parallel and count the number of 'array changes'
		    int changes = 0;
		    boolean next1 = true;
		    boolean next2 = true;
		    int aIdx1 = 0;
		    int aIdx2 = 0;
		    //define start value
		    double actD = 0.0;
		    boolean lastTakenFrom = false; //false = angle1, true = angle2,
		    if (angle1[0] < angle2[0]) {
			actD = angle1[0];
			aIdx1++;
			next1 = false;
			lastTakenFrom = false;
		    } else {
			actD = angle2[0];
			aIdx2++;
			next2 = false;
			lastTakenFrom = true;
		    }//else
		    

		    //start traversal
		    while (((aIdx1 < angle1.length || !next1)) && ((aIdx2 < angle2.length) || !next2)) {
			if (next1) {
			    //aIdx1++;
			    next1 = false;
			}//if
			if (next2) {
			    //aIdx2++;
			    next2 = false;
			}//if
			
			//compare values and set variable changes
			if (angle1[aIdx1] < angle2[aIdx2]) {
			    next1 = true;
			    aIdx1++;
			    if (lastTakenFrom) changes++;
			    lastTakenFrom = false;
			} else {
			    next2 = true;
			    aIdx2++;
			    if (!lastTakenFrom) changes++;
			    lastTakenFrom = true;
			}//else
		    }//while
		    
		    //check, whether there are still unvisited fields and modify the changes variable
		    if ((aIdx1 < angle1.length+1) || (aIdx2 < angle2.length+1)) changes++;

		    //check variable changes and set boolean array
		    if (changes > 2) {
			retArr[0] = Boolean.TRUE;
			retArr[1] = Boolean.FALSE;
			return retArr;
		    } else {
			retArr[0] = Boolean.FALSE;
			retArr[1] = Boolean.TRUE;
		    }//if
		}//else lengthes were > 1
	    		    
		//now go one step forward in both arrays
		getNext1 = true;
		arrIdx1++;
		getNext2 = true;
		arrIdx2++;
		
	    } else {
		//neighbours are not equal, proceed in one of the arrays
		int res = actV1.compare(actV2);
		
		if (res == -1) {
		    arrIdx1++;
		    getNext1 = true;
		} else {
		    arrIdx2++;
		    getNext2 = true;
		}//else
	    }//else
	}//while
      
	return retArr;
    }//end method checkForIntersectionAndMeetSameEndpoints


    /**
     * This method computes the intersects and meets predicate for a special case.
     * That special case is, when a segment S1 (of SET1) has and endpoint on S2 (of SET2), but not on an endpoint of S2. Then,
     * we have meets==true, if there are no other segments of SET2 which also have endpoints on S1. In the latter case
     * it must be examined, whether those other segments end up in the same point or whether they lie on different sides
     * of S1. All this is done here.<p>
     * As a result, an array is returned which holds two boolean values. The first value represents the intersects predicate,
     * whereas the second one stands for the meets predicate. Obviously, not both can be true at the same time.
     * Valid configurations are [false,false], [true,false] and [false,true].
     *
     * @param ljp1 a set with LeftJoinPairs, where for every element of such a pair the elemSet holds only segments, which
     *             have an endpoint on that segment
     * @param ljp2 same as ljp1, but now the elements of each pair are of SET2 (in ljp1 of SET1) and the segments in elemSet
     *             are of SET1 (in ljp1 of SET2);
     * @return the array that holds the result
     */
    private static Boolean[] checkForIntersectionAndMeets(LeftJoinPairMultiSet ljp1, LeftJoinPairMultiSet ljp2) {
	//for both sets (ljp1,ljp2) do the following:
	//traverse set and look at elemSet of every setelement (S,ES):
	//if ES is empty: do nothing
	//if ES has 1 element: set meets = true
	//if ES has more elements:
	//   - for every element E do:
	//        * find the point P that lies on the S
	//        * store P in a hashtable (use P as key for E), if key doesn't exist already in the hashtable
	//        * if segments with P are already there, check all other linked segments (Es), on which side of the S they lie,
	//          if any segment lies on the other side, set intersects=true, set meets=false and BREAK;
	//          if all other segments lie on the same side, set intersects=false, set meets= true and CONTINUE; store P in the hashtable
	//
	//          NOTE: We check only the first found element and not ALL elements, because, every time a segment is added to the 
	//          hashtable, it is checked for the correct side.
	//
	//after both sets were traversed:
	//   - intersects must be false, otherwise this point would never have been reached;
	//     if meets=true, return meets=true,intersects=false;
	//     if meets=false, return meets=false,intersects=false
	Boolean[] retArr = { Boolean.FALSE, Boolean.FALSE };
	Iterator it = ljp1.iterator();
	LeftJoinPair actLjp;
	ProHashtable ht = new ProHashtable(INITIAL_CAPACITY);
	Iterator sit;
	Segment actSeg,htSeg,actLjpSeg;

	while (it.hasNext()) {
	    actLjp = (LeftJoinPair)((MultiSetEntry)it.next()).value;
	    
	    actLjpSeg = (Segment)actLjp.element;

	    if (actLjp.elemSet == null || actLjp.elemSet.isEmpty()) {
		//do nothing
	    }//if
	    else {
		if (actLjp.elemSet.size() == 1) {
		    retArr[1] = Boolean.TRUE;
		}//if
		else {
		    //ES has more elements
		    ht.clear();
		    sit = actLjp.elemSet.iterator();
		    while (sit.hasNext()) {
			actSeg = (Segment)((MultiSetEntry)sit.next()).value;
			Point commPoint = actLjpSeg.intersection(actSeg);
			//store segment in hashtable
			if (ht.containsKey(commPoint)) {
			    //get first segment from hashtable with the same key
			    htSeg = (Segment)ht.getFirst(commPoint);
			    //check, whether the other segment lies 'on the other side'
			    //NOTE: it's not necessary to check _all_ elements, since every time, a new element is
			    //added to the hashtable, it's checked for the correct side. Therefore, we check only
			    //the first found segment.
			    if (Mathset.pointPosition(actLjpSeg.getStartpoint(),actLjpSeg.getEndpoint(),actSeg.theOtherOne(commPoint)) !=
				Mathset.pointPosition(actLjpSeg.getStartpoint(),actLjpSeg.getEndpoint(),htSeg.theOtherOne(commPoint))) {
				retArr[0] = Boolean.TRUE;
				retArr[1] = Boolean.FALSE;
				return retArr;
			    }//if
			    
			    //store segment in the hashtable, otherwise
			    ht.put(commPoint,actSeg);
			} else {		    
			    //simply store segment
			    ht.put(commPoint,actSeg);
			    retArr[1] = Boolean.TRUE;
			}//else
		    }//while sit
		}//else
	    }//else
	}//while it
	
	//now do the same for the other LeftJoinPairMultiSet
	it = ljp2.iterator();

	while (it.hasNext()) {
	    actLjp = (LeftJoinPair)((MultiSetEntry)it.next()).value;
	    
	    actLjpSeg = (Segment)actLjp.element;

	    if (actLjp.elemSet == null || actLjp.elemSet.isEmpty()) {
		//do nothing
	    }//if
	    else {
		if (actLjp.elemSet.size() == 1) {
		    retArr[1] = Boolean.TRUE;
		}//if
		else {
		    //ES has more elements
		    ht.clear();
		    sit = actLjp.elemSet.iterator();
		    while (sit.hasNext()) {
			actSeg = (Segment)((MultiSetEntry)sit.next()).value;
			Point commPoint = actLjpSeg.intersection(actSeg);
			//store segment in hashtable
			if (ht.containsKey(commPoint)) {
			    //get first segment from hashtable with the same key
			    htSeg = (Segment)ht.getFirst(commPoint);
			    //check, whether the other segment lies 'on the other side'
			    //NOTE: it's not necessary to check _all_ elements, since every time, a new element is
			    //added to the hashtable, it's checked for the correct side. Therefore, we check only
			    //the first found segment.
			    if (Mathset.pointPosition(actLjpSeg.getStartpoint(),actLjpSeg.getEndpoint(),actSeg.theOtherOne(commPoint)) !=
				Mathset.pointPosition(actLjpSeg.getStartpoint(),actLjpSeg.getEndpoint(),htSeg.theOtherOne(commPoint))) {
				retArr[0] = Boolean.TRUE;
				retArr[1] = Boolean.FALSE;
				return retArr;
			    }//if
			    
			    //store segment in the hashtable, otherwise
			    ht.put(commPoint,actSeg);
			} else {		    
			    //simply store segment
			    ht.put(commPoint,actSeg);
			    retArr[1] = Boolean.TRUE;
			}//else
		    }//while sit
		}//else
	    }//else
	}//while it

	return retArr;
    }//end method checkForIntersectionAndMeets
    
}//end class SupportOps
