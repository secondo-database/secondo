import java.util.*;
import java.lang.reflect.*;

class ROSEAlgebra {

  //variables
    static final Class pointClass = (new Point()).getClass();
    static final Class segClass = (new Segment()).getClass();
    static final Class triClass = (new Triangle()).getClass();
    
    static final Class psOpsClass = (new PointSeg_Ops()).getClass();
    static final Class ptOpsClass = (new PointTri_Ops()).getClass();

    static final Class ssOpsClass = (new SegSeg_Ops()).getClass();
    static final Class stOpsClass = (new SegTri_Ops()).getClass();

    static final Class ttOpsClass = (new TriTri_Ops()).getClass();

    int[] te = {1};

    final static Class[] paramListP = { pointClass };
    final static Class[] paramListS = { segClass };
    final static Class[] paramListT = { triClass };

    final static Class[] paramListPS = { pointClass, segClass };
    final static Class[] paramListPT = { pointClass, triClass };
    final static Class[] paramListSS = { segClass, segClass };
    final static Class[] paramListST = { segClass, triClass };
    final static Class[] paramListTT = { triClass, triClass };

    //constructors

    //methods

    public static SegList minus (SegList sl, TriList tl) {
	//comment missing
	
	SegList retList = new SegList();
	Class c = stOpsClass;
	Class c2 = (new SetOps()).getClass();
	Class[] paramList = new Class[4];
	
	try {
	    paramList[0] = Class.forName("ElemList");
	    paramList[1] = Class.forName("ElemList");
	    //paramList[2] = (new Method()).getClass();
	    //paramList[3] = Class.forName("Method");
	    Method m1 = c.getMethod("pintersects",paramListST);
	    //System.out.println("paramList:");
	    //for (int i = 0; i < (m1.getParameterTypes()).length; i++) {
	    //System.out.println("i:"+(m1.getParameterTypes())[i]);
	    //}//for
	    
	    paramList[2] = m1.getClass();
	    paramList[3] = m1.getClass();
	    //System.out.println("c1");
	    Method m2 = c.getMethod("minus",paramListST);
	    //System.out.println("c2");
	    Method m3 = c2.getMethod("subtract",paramList);
	    //Caution: subtract shouldn't be passed that way, because
	    //it is a SetOps method
	    //System.out.println("c3");
	    //LeftJoinPairList ljpl =  new LeftJoinPairList();
	    //ljpl = SetOps.leftOuterJoin(sl,tl,m1);
	    //System.out.println("c3.1");
	    //retList = SegList.convert(SetOps.map(ljpl,m3,m1,m2));
	    //System.out.println("c3.2");
	    retList = SegList.convert(SetOps.map(SetOps.leftOuterJoin(sl,tl,m1),m3,m1,m2));
	    //System.out.println("c4");
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
    
	return retList;
    }//end method minus
    

    public static TriList minus (TriList tl1, TriList tl2) {
	//comment missing
	//same as minus(SegList,TriList)
	
	System.out.println("entering A.minus.. ");

	TriList retList = new TriList();
	Class c = (new Triangle()).getClass();
	Class c2 = (new SetOps()).getClass();
	Class[] paramListT = { c };
	Class[] paramListTT = { c,c };
	Class[] paramList = new Class[4];
	Class[] paramListE = new Class[1];
	
	try {
	    paramListE[0] = Class.forName("Element");
	    paramList[0] = Class.forName("ElemList");
	    paramList[1] = Class.forName("ElemList");
	    
	    Method m1 = c.getMethod("pintersects",paramListT);
	    //System.out.println("found method m1");
	    paramList[2] = m1.getClass();
	    paramList[3] = m1.getClass();
	    
	    Method m2 = c.getMethod("minus",paramListT);
	    //System.out.println("found method m2");
	    Method m3 = c2.getMethod("subtract",paramList);
	    //System.out.println("found method m3");
	    LeftJoinPairList ljpl = SetOps.leftOuterJoin(tl1,tl2,m1);
	    System.out.println("leftOuterJoin passed...");
	    //System.out.println("ljpl.size:"+ljpl.size());
	    //ljpl.print();
	    //System.exit(0);
	    retList = TriList.convert(SetOps.map(SetOps.leftOuterJoin(tl1,tl2,m1),m3,m1,m2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println(0);
	}//catch
	
	return retList;
    }//end method minus


    public static SegList overlap (SegList sl) {
	//comment missing

	SegList retList = new SegList();
	Class c = ssOpsClass;

	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    Method m2 = c.getMethod("theOverlap",paramListSS);
	    retList = (SegList)SetOps.reduce(sl,m1,m2);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retList;
    }//end method overlap

    public static SegList once (SegList sl) {
	//comment missing
	
	SegList retList = new SegList();
	Class c = ssOpsClass;
	
	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    Method m2 = c.getMethod("union",paramListSS);
	    retList = (SegList)SetOps.reduce(sl,m1,m2);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	System.out.println("once.size(): "+retList.size());
	return retList;
    }//end method once


    public static SegList minimal (SegList sl, boolean overlap) {
	//comment missing
	//overlap toggles whether overlapReduce or simple reduce is used
	//true : use overlapReduce
	//false : use reduce

	//System.out.println("entering A.minimal...");
	SegList retList = new SegList();
	Class c = ssOpsClass;

	try {
	    Method m1 = c.getMethod("adjacent",paramListSS);
	    Method m2 = c.getMethod("concat",paramListSS);
	    if (!overlap) { retList = (SegList)SetOps.reduce(sl,m1,m2); }
	    else { retList = SegList.convert(SetOps.overlapReduce(sl,m1,m2,true)); }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//System.out.println("leaving A.minimal.");
	return retList;
    }//end method minimal


    public static SegList unique (SegList sl, boolean overlap) {
	//comment missing
	//overlap toggles whether overlapReduce or simple reduce is used
	//true : use overlapReduce
	//false : use reduce

	SegList retList = new SegList();
	Class c = ssOpsClass;
	//System.out.println("Algebra.unique calling...");

	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    Method m2 = c.getMethod("symDiff",paramListSS);
	    if (!overlap) { retList = (SegList)SetOps.reduce(sl,m1,m2); }
	    else { retList = SegList.convert(SetOps.overlapReduce(sl,m1,m2,true)); }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//System.out.println("A.unique:exit");
	return retList;
    }//end method unique


    public static SegList contour (TriList tl) {
	//comment missing
	
	//System.out.println("entering A.contour...");
	Class c = triClass;
	SegList retList = new SegList();
	try {
	    Method m = c.getMethod("segments",null);
	    //System.out.println("A.c.check01");
	    retList = minimal(unique(SegList.convert(SetOps.map(tl,m)),false),false);
	    //System.out.println("A.c.check02");
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//System.out.println("leaving A.contour.");
	return retList;
    }//end method contour


    public static boolean intersects (ElemList el1, ElemList el2) {
	//comment missing

	Class c;
	PairList retList = new PairList();
	Class [] paramList = new Class [1];

	if (!el1.isEmpty()) { c = el1.getFirst().getClass(); }
	else {
	    if (!el1.isEmpty()) { c = el2.getFirst().getClass(); }
	    else { return false; }
	}//else

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("intersects",paramList);
	    retList = SetOps.join(el1,el2,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method intersects


    /************************************************************
     *the following operations are the original ROSE operations *
     ***********************************************************/
    
    //@ TESTED.
    public static boolean pp_equal (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	boolean retVal = false;

	retVal = SetOps.equal(pl1,pl2);
	return retVal;
    }//end method pp_equal


    //@ TESTED.
    public static boolean ll_equal (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	boolean retVal = false;
	retVal = SetOps.equal(minimal(sl1,false),minimal(sl2,false));
	return retVal;
    }//end method ll_equal


    //@ TESTED.
    public static boolean rr_equal (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	return ll_equal(new Lines(contour(tl1)),new Lines(contour(tl2)));
    }//end method rr_equal


    //@ TESTED.
    public static boolean pp_unequal (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	return !pp_equal(new Points(pl1),new Points(pl2));
    }//end method pp_unequal


    //@ TESTED.
    public static boolean ll_unequal (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	return (!ll_equal(new Lines(sl1),new Lines(sl2)));
    }//end method ll_unequal


    //@ TESTED.
    public static boolean rr_unequal (Regions r1, Regions r2) {
	//comment missing
	//TriList tl1 = r1.trilist;
	//TriList tl2 = r2.trilist;
	return (!rr_equal(r1,r2));
    }//end method rr_unequal


    //@ TESTED.
    public static boolean pp_disjoint (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	return (SetOps.disjoint(pl1,pl2));
    }//end method pp_disjoint
	

    //@ TESTED.
    public static boolean ll_disjoint (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	return (SetOps.disjoint(sl1,sl2));
    }//end method ll_disjoint


    //@ TESTED.
    public static boolean rr_disjoint (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	return (SetOps.disjoint(tl1,tl2));
    }//end method rr_disjoint


    //@ TESTED.
    public static boolean pr_inside (Points p, Regions r) {
	//comment missing
	PointList pl = p.pointlist;
	TriList tl = r.trilist;
	
	Class c = ptOpsClass;
	PointList retList = new PointList();

	try {
	    Method m = c.getMethod("inside",paramListPT);
	    retList = PointList.convert(SetOps.rdup(SetOps.proj1(SetOps.join(pl,tl,m))));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (pl.size() == retList.size()) { return true; }
	else { return false; }
    }//end method pr_inside

    
    //@ TESTED.
    public static boolean lr_inside (Lines l, Regions r) {
	//comment missing
	SegList sl = l.seglist;
	TriList tl = r.trilist;
	SegList retList = minus(sl,tl);
	if (retList.size() == 0) { return true; }
	else { return false; }
    }//end method lr_inside


    //@ TESTED. Why is this function constant false?
    public static boolean rr_inside (Regions r1, Regions r2) {
	return false;
    }

    //@ TESTED. Why is this function constant false?
    public static boolean rr_area_disjoint (Regions r1, Regions r2) {
	return false;
    }

    //@ TESTED. Why is this function constant false?
    public static boolean rr_edge_disjoint (Regions r1, Regions r2) {
	return false;
    }
    
    //@ TESTED. Why is this function constant false?
    public static boolean rr_edge_inside (Regions r1, Regions r2) {
	return false;
    }

    //@ TESTED. Why is this function constant false?
    public static boolean rr_vertex_inside (Regions r1, Regions r2) {
	return false;
    }


    //@ TESTED.
    public static boolean ll_intersects (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	return intersects(sl1,sl2);
    }//end method ll_intersects


    //@ TESTED.
    public static boolean lr_intersects (Lines l, Regions r) {
	//comment missing
	SegList sl = l.seglist;
	TriList tl = r.trilist;
	Class c = stOpsClass;
	PairList retList = new PairList();
	
	try {
	    Method m = c.getMethod("intersects",paramListST);
	    retList = SetOps.join(sl,tl,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method lr_intersects


    //@ TESTED.
    public static boolean rl_intersects (Regions r, Lines l) {
	//comment missing
	return lr_intersects(l,r);
    }//end method rl_intersects


    //@ TESTED.
    public static boolean rr_intersects (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	return intersects(tl1,tl2);
    }//end method rr_intersects


    //@ TESTED. 
    public static boolean ll_meets (Lines l1, Lines l2) {
	return false;
    }
	
    //@ TESTED.
    public static boolean lr_meets (Lines l, Regions r) {
	return false;
    }

    //@ TESTED.
    public static boolean rl_meets (Regions r, Lines l) {
	return false;
    }

    //@ TESTED. Why is this function contant false?
    public static boolean rr_meets (Regions r1, Regions r2) {
	return false;
    }


    //@ TESTED.
    public static boolean ll_border_in_common (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	Class c = ssOpsClass;
	PairList retList = new PairList();

	try {
	    Method m = c.getMethod("overlap",paramListSS);
	    retList = SetOps.join(sl1,sl2,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method ll_border_in_common


    //@ TESTED.
    public static boolean lr_border_in_common (Lines l, Regions r) {
	//comment missing
	SegList sl = l.seglist;
	TriList tl = r.trilist;
	Class c = ssOpsClass;
	PairList retList = new PairList();

	try {
	    Method m = c.getMethod("overlap",paramListSS);
	    retList = SetOps.join(sl,contour(tl),m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method lr_border_in_common

    //@ TESTED.
    public static boolean rl_border_in_common (Regions r, Lines l) {
	//comment missing
	return lr_border_in_common(l,r);
    }//end method rl_border_in_common


    public static boolean rr_border_in_common (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	return ll_border_in_common(new Lines(contour(tl1)),new Lines(contour(tl2)));
    }//end method rr_border_in_common


    //@ TESTED.
    public static boolean rr_adjacent (Regions r1, Regions r2) {
	//comment missing
	return rr_border_in_common(r1,r2);
    }//end method rr_adjacent
      
	
    //@ TESTED. Why is this function contant false?
    public static boolean rr_encloses (Regions r1, Regions r2) {
	return false;
    }


    //@ TESTED(JNI)
    // FEHLER !!!!
    public static boolean pl_on_border_of (Points p, Lines l) {
	return true;
    }//end method pl_on_border_of


    //@ TESTED(JNI)
    // FEHLER !!!!
    public static boolean pr_on_border_of (Points p, Regions r) {
	return true;
    }//end method pr_on_border_of

    //@ TESTED(JNI)
    // FEHLER !!!!
    public static Points pp_intersection (Points p1, Points p2) {
	return p1;
    }//end method pp_intersection


    public static Points ll_intersection (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;

	Class c = segClass;
	ElemList retList = new ElemList();
	Class[] paramList = new Class[1];

	try {
	    paramList[0] = Class.forName("Element");
	    Method m1 = c.getMethod("intersects",paramList);
	    Method m2 = c.getMethod("intersection",paramListS);
	    retList = SetOps.rdup(SetOps.map(SetOps.join(sl1,sl2,m1),m2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return new Points(PointList.convert(retList));
    }//end method ll_intersection


    //@ TESTED(JNI)
    // FEHLER !!!!
    public static Regions rr_intersection (Regions r1, Regions r2) {
	return r1;
    }//end method rr_intersection


    public static Lines rl_intersection (Regions r, Lines l) {
	//comment missing
	TriList tl = r.trilist;
	SegList sl = l.seglist;
	Class c = stOpsClass;
	SegList retList = new SegList();

	try {
	    Method m1 = c.getMethod("intersects",paramListST);
	    Method m2 = c.getMethod("intersection",paramListST);

	    retList = minimal(SegList.convert(SetOps.map(SetOps.join(sl,tl,m1),m2)),false);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return new Lines(retList);
    }//return rl_intersection


    public static Points pp_plus (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	PointList retList = new PointList();
	try {
	    retList = PointList.convert(SetOps.union(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return new Points(retList);
    }//end method pp_plus


    public static Lines ll_plus (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	SegList retList = new SegList();
	
	try {
	    retList = minimal(once((SegList)SetOps.union(sl1,sl2)),false);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return new Lines(retList);
    }//end method ll_plus


    //@ TESTED. FEHLER.
    public static Regions rr_plus (Regions r1, Regions r2) {
	//Regions result = new Regions();
	return r1;
    }
	
    
    public static Points pp_minus (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	PointList retList = new PointList();

	try {
	    retList = PointList.convert(SetOps.difference(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return new Points(retList);
    }//end method pp_minus


    public static Lines ll_minus (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	SegList retList = new SegList();
	
	try {
	    
	    long time1 = System.currentTimeMillis();
	    SegList inters = (SegList)SetOps.intersection(sl1,sl2);
	    SegList first = SegList.convert(SetOps.difference(SetOps.disjointUnion(sl1,sl2),SetOps.intersection(sl1,sl2)));
	    long time2 = System.currentTimeMillis();
	    SegList second = unique(first,true);
	    long time3 = System.currentTimeMillis();
	    SegList third = (SegList)minimal(second,true);
	    long time4 = System.currentTimeMillis();
	    
	    //System.out.println("disjointUnion: "+(time2-time1)+"ms, unique: "+(time3-time2)+"ms, minimal: "+(time4-time3)+"ms, sum: "+(time4-time1)+"ms");
	    //System.exit(0);
	    retList = third;
	    //retList = (SegList)minimal(unique((SegList)SetOps.disjointUnion(sl1,sl2)));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return new Lines(retList);
    }//end method ll_minus


    //@ TESTED(JNI)
    // FEHLER !!!!
    public static Regions rr_minus (Regions r1, Regions r2) {
	return r1;
    }//end method rr_minus


    public static Lines ll_common_border (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	Class c = ssOpsClass;
	PairList joinList = new PairList();
	SegList retList = new SegList();
	
	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    joinList = SetOps.join(sl1,sl2,m1);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (joinList.isEmpty()) { new SegList(); }
	else {
	    try {
		Method m2 = c.getMethod("theOverlap",paramListSS);
		retList = minimal(SegList.convert(SetOps.map(joinList,m2)),false);
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	}//else
	return new Lines(retList);

    }//end method ll_common_border


    public static Lines lr_common_border (Lines l, Regions r) {
	//comment missing
	TriList tl = r.trilist;
	//return minimal(overlap((SegList)SetOps.union(sl,contour(tl))));
	return ll_common_border(l,new Lines(contour(tl)));
    }//end method lr_common_border


    public static Lines rl_common_border (Regions r, Lines l) {
	//comment missing
	return lr_common_border(l,r);
    }//end method rl_common_border


    //@ TESTED.
    public static Lines rr_common_border (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	return ll_common_border(new Lines(contour(tl1)),new Lines(contour(tl2)));
    }//end method rr_common_border


    public static Points l_vertices (Lines l) {
	//comment missing
	SegList sl = l.seglist;
	Class c = segClass;
	PointList retList = new PointList();
	
	try {
	    Method m = c.getMethod("endpoints",null);
	    retList = PointList.convert(SetOps.rdup(SetOps.map(sl,m)));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return new Points(retList);
    }//end method l_vertices


    public static Points r_vertices (Regions r) {
	return l_vertices(r_contour(r));
    }//end method r_vertices


    public static Regions l_interior (Lines l) {
	return new Regions();
    }//end method l_interior


    public static Lines r_contour(Regions r) {
	//comment missing
	TriList tl = r.trilist;
	return new Lines(contour(tl));
    }//end method r_contour


    public static int p_no_of_components (Points p) {
	//comment missing
	PointList pl = p.pointlist;
	return pl.size();
    }//end method p_no_of_components
    

    public static int l_no_of_components (Lines l) {
	//comment missing
	SegList sl = l.seglist;
	int retVal = 0;
	Class c = ssOpsClass;
	
	try {
	    //System.out.println("so.lnocom:c1");
	    Method m = c.getMethod("pointsInCommon",paramListSS);
	    //System.out.println("so.lnocom:c2");
	    retVal = (SetOps.group(sl,m)).size();
	    //System.out.println("so.lnocom:c3");
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method l_no_of_components


    public static int r_no_of_components (Regions r) {
	//comment missing
	TriList tl = r.trilist;
	int retVal = 0;
	Class c = ttOpsClass;

	try {
	    Method m = c.getMethod("adjacent",paramListTT);
	    retVal = (SetOps.group(tl,m)).size();
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method r_no_of_components


    public static Rational pp_dist (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	Rational retVal = new Rational(0);
	Class c = pointClass;
	Class[] paramList = new Class[1];

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.min(pl1,pl2,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method pp_dist


    public static Rational pl_dist (Points p, Lines l) {
	//comment missing
	PointList pl = p.pointlist;
	SegList sl = l.seglist;
	Rational retVal = new Rational(0);
	Class c = psOpsClass;

	try {
	    Method m = c.getMethod("dist",paramListPS);
	    ElemPair retPair = SetOps.min(pl,sl,m);
	    retVal = PointSeg_Ops.dist((Point)retPair.first,(Segment)retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end mehtod pl_dist


    public static Rational pr_dist (Points p, Regions r) {
	//comment missing
	PointList pl = p.pointlist;
	TriList tl = r.trilist;
	Rational retVal = new Rational(0);
	Class c = ptOpsClass;

	try {
	    Method m = c.getMethod("dist",paramListPT);
	    ElemPair retPair = SetOps.min(pl,tl,m);
	    retVal = PointTri_Ops.dist((Point)retPair.first,(Triangle)retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method pr_dist


    public static Rational lp_dist (Lines l, Points p) {
	//comment missing
	return pl_dist(p,l);
    }//end method lp_dist


    public static Rational ll_dist (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	Rational retVal = new Rational(0);
	Class c = segClass;
	Class[] paramList = new Class[1];

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.min(sl1,sl2,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method ll_dist


    public static Rational lr_dist (Lines l, Regions r) {
	//comment missing
	SegList sl = l.seglist;
	TriList tl = r.trilist;
	Rational retVal = new Rational(0);
	Class c = stOpsClass;

	try {
	    Method m = c.getMethod("dist",paramListST);
	    ElemPair retPair = SetOps.min(sl,tl,m);
	    retVal = SegTri_Ops.dist((Segment)retPair.first,(Triangle)retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method lr_dist


    public static Rational rp_dist (Regions r, Points p) {
	//comment missing
	return pr_dist(p,r);
    }//end method rp_dist


    public static Rational rl_dist (Regions r, Lines l) {
	//comment missing
	return lr_dist(l,r);
    }//end method rl_dist


    public static Rational rr_dist (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	Rational retVal = new Rational(0);
	Class c = triClass;
	Class[] paramList = new Class[1];
	
	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.min(tl1,tl2,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method rr_dist


    public static Rational p_diameter (Points p) {
	//comment missing
	PointList pl = p.pointlist;
	Rational retVal = new Rational(0);
	Class c = pointClass;
	Class[] paramList = new Class[1];

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.max(pl,pl,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return retVal;
    }//end method p_diameter


    public static Rational l_diameter (Lines l) {
	//comment missing
	return p_diameter(l_vertices(l));
    }//end method l_diameter


    public static Rational r_diameter (Regions r) {
	//comment missing
	return p_diameter(r_vertices(r));
    }//end method r_diameter


    public static double l_length (Lines l) {
	//sums up the length of every element of sl
	SegList sl = l.seglist;
	double retSum = 0;
	Class c = segClass;
	try {
	    Method m = c.getMethod("length",null);
	    retSum = SetOps.sum(sl,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retSum;
    }//end method length
  

    public static double r_area (Regions r) {
	//comment missing
	TriList tl = r.trilist;
	double retVal = 0;
	Class c = triClass;
	
	try {
	    Method m = c.getMethod("area",null);
	    retVal = SetOps.sum(tl,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method area

    
    public static double r_perimeter (Regions r) {
	//comment missing
	TriList tl = r.trilist;
	return l_length(new Lines(contour(tl)));
    }//end method r_perimeter


}//end class ROSEAlgebra
