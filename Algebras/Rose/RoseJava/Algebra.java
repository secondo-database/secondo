import java.util.*;
import java.lang.reflect.*;

class Algebra {

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
	Class c3 = ssOpsClass;
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
	    Method m2 = c.getMethod("minus",paramListST);
	    Method m3 = c2.getMethod("subtractRemove",paramList);
	    LeftJoinPairList ljpl =  new LeftJoinPairList();
	    Method m4 = c.getMethod("intersection",paramListST);
	    Method m5 = c3.getMethod("minus",paramListSS);
	    Method m6 = c3.getMethod("overlap",paramListSS);
	    ListIterator lit = sl.listIterator(0);
	    while (lit.hasNext()) {
		((Segment)lit.next()).align(); }
	    ljpl = SetOps.overlapLeftOuterJoin(sl,tl,m1,true);
	    //System.out.println("ljpl:"); ljpl.print();
	    //System.exit(0);
	    ljpl = SetOps.subtractSets(ljpl,m4);
	    //System.out.println("ljpl:"); ljpl.print();
	    //System.exit(0);
	    //System.out.println("leftjoinpairlist:");
	    //ElemList sretlist = SetOps.subtract(ljpl,m5);
	    retList = SegList.convert(SetOps.map(ljpl,m3,m6,m5));
	    //System.out.println("\nsretlist:"); retList.print();
	    //System.exit(0);

	    //retList = SegList.convert(SetOps.map(SetOps.overlapLeftOuterJoin(sl,tl,m1),m3,m1,m2));
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
	
	//System.out.println("entering A.minus.. ");

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
	    paramList[2] = m1.getClass();
	    paramList[3] = m1.getClass();
	    
	    Method m2 = c.getMethod("minus",paramListT);
	    Method m3 = c2.getMethod("subtract",paramList);
	    //LeftJoinPairList ljpl = SetOps.overlapLeftOuterJoin(tl1,tl2,m1);
	    //System.out.println("ljpl.size:"+ljpl.size());
	    //ljpl.print();
	    //System.exit(0);
	    retList = TriList.convert(SetOps.map(SetOps.overlapLeftOuterJoin(tl1,tl2,m1,false),m3,m1,m2));
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
	//SegList retList = sl;
	
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
	//System.out.println("entering A.unique...");

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
	//System.out.println("leaving A.unique.");
	return retList;
    }//end method unique


    public static SegList contour (TriList tl,boolean minOverlap,boolean uniOverlap) {
	//comment missing
	//minOverlap/uniOverlap toggle whether overlapReduce(true) of Reduce(false)
	//is used
	
	//System.out.println("entering A.contour...");
	Class c = triClass;
	SegList retList = new SegList();
	try {
	    Method m = c.getMethod("segments",null);
	    //ElemList ll = SetOps.map(tl,m);
	    //SetOps.quicksortX(ll);ll.print();
	    //System.exit(0);
	    //ElemList el = unique(SegList.convert(SetOps.map(tl,m)),true);
	    //System.out.println("unique:");
	    //el.print();
	    //System.exit(0);
	    retList = minimal(unique(SegList.convert(SetOps.map(tl,m)),uniOverlap),minOverlap);
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


    public static boolean pp_equal (PointList pl1, PointList pl2) {
	//comment missing
	boolean retVal = false;

	retVal = SetOps.equal(pl1,pl2);
	return retVal;
    }//end method pp_equal


    public static boolean ll_equal (SegList sl1, SegList sl2) {
	//comment missing
	boolean retVal = false;
	retVal = SetOps.equal(minimal(sl1,false),minimal(sl2,false));
	return retVal;
    }//end method ll_equal

    public static boolean rr_equal (TriList tl1, TriList tl2) {
	//comment missing

	return ll_equal(contour(tl1,true,true),contour(tl2,true,true));
    }//end method rr_equal


    public static boolean pp_unequal (PointList pl1, PointList pl2) {
	//comment missing

	return !pp_equal(pl1,pl2);
    }//end method pp_unequal


    public static boolean ll_unequal (SegList sl1, SegList sl2) {
	//comment missing

	return (!ll_equal(sl1,sl2));
    }//end method ll_unequal


    public static boolean rr_unequal (TriList tl1, TriList tl2) {
	//comment missing

	return (!rr_equal(tl1,tl2));
    }//end method rr_unequal


    public static boolean pp_disjoint (PointList pl1, PointList pl2) {
	//comment missing

	return (SetOps.disjoint(pl1,pl2));
    }//end method pp_disjoint
	

    public static boolean ll_disjoint (SegList sl1, SegList sl2) {
	//comment missing

	return (SetOps.disjoint(sl1,sl2));
    }//end method ll_disjoint


    public static boolean rr_disjoint (TriList tl1, TriList tl2) {
	//comment missing

	return (SetOps.disjoint(tl1,tl2));
    }//end method rr_disjoint


    public static boolean pr_inside (PointList pl, TriList tl) {
	//comment missing

	Class c = ptOpsClass;
	PointList retList = new PointList();

	try {
	    Method m = c.getMethod("inside",paramListPT);
	    /* the following implementation uses the primitive join
	       retList = PointList.convert(SetOps.rdup(SetOps.proj1(SetOps.join(pl,tl,m))));
	    */
	    //new implementation
	    //long time1 = System.currentTimeMillis();
	    PairList el1 = SetOps.overlapJoin(pl,tl,m,true);
	    //long time2 = System.currentTimeMillis();
	    ElemList el2 = SetOps.proj1(el1);
	    //long time3 = System.currentTimeMillis();
	    ElemList el3 = SetOps.rdup(el2);
	    //long time4 = System.currentTimeMillis();
	    retList = PointList.convert(el3);
	    //long time5 = System.currentTimeMillis();
	    //retList = PointList.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(pl,tl,m))));
	    //System.out.println("overlapJoin: "+(time2-time1)+"ms, proj1: "+(time3-time2)+"ms, rdup: "+(time4-time3)+"ms, convert: "+(time5-time4)+"ms");
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (pl.size() == retList.size()) { return true; }
	else { return false; }
    }//end method pr_inside

    
    public static boolean lr_inside (SegList sl, TriList tl) {
	//comment missing
	SegList retList = minus(sl,tl);
	if (retList.size() == 0) { return true; }
	else { return false; }
    }//end method lr_inside


    public static boolean rr_inside (TriList tl1, TriList tl2) {
	return false;
    }


    public static boolean rr_area_disjoint (TriList tl1, TriList tl2) {
	return false;
    }


    public static boolean rr_edge_disjoint (TriList tl1, TriList tl2) {
	return false;
    }
    

    public static boolean rr_edge_inside (TriList tl1, TriList tl2) {
	return false;
    }


    public static boolean rr_vertex_inside (TriList tl1, TriList tl2) {
	return false;
    }


    public static boolean ll_intersects (SegList sl1, SegList sl2) {
	//comment missing

	return intersects(sl1,sl2);
    }//end method ll_intersects


    public static boolean lr_intersects (SegList sl, TriList tl) {
	//comment missing

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


    public static boolean rl_intersects (TriList tl, SegList sl) {
	//comment missing

	return lr_intersects(sl,tl);
    }//end method rl_intersects


    public static boolean rr_intersects (TriList tl1, TriList tl2) {
	//comment missing

	return intersects(tl1,tl2);
    }//end method rr_intersects


    public static boolean ll_meets (SegList sl1, SegList sl2) {
	return false;
    }
	

    public static boolean lr_meets (SegList sl, TriList tl) {
	return false;
    }


    public static boolean rl_meets (TriList tl, SegList sl) {
	return false;
    }


    public static boolean rr_meets (TriList tl1, TriList tl2) {
	return false;
    }


    public static boolean ll_border_in_common (SegList sl1, SegList sl2) {
	//comment missing

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


    public static boolean lr_border_in_common (SegList sl, TriList tl) {
	//comment missing

	Class c = ssOpsClass;
	PairList retList = new PairList();

	try {
	    Method m = c.getMethod("overlap",paramListSS);
	    retList = SetOps.join(sl,contour(tl,true,true),m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method lr_border_in_common


    public static boolean rl_border_in_common (TriList tl, SegList sl) {
	//comment missing

	return lr_border_in_common(sl,tl);
    }//end method rl_border_in_common


    public static boolean rr_border_in_common (TriList tl1, TriList tl2) {
	//comment missing

	return ll_border_in_common(contour(tl1,true,true),contour(tl2,true,true));
    }//end method rr_border_in_common


    public static boolean rr_adjacent (TriList tl1, TriList tl2) {
	//comment missing

	return rr_border_in_common(tl1,tl2);
    }//end method rr_adjacent
      
	
    public static boolean rr_encloses (TriList tl1, TriList tl2) {
	return false;
    }


    public static boolean pl_on_border_of (PointList pl, SegList sl) {
	//comment missing

	Class c = psOpsClass;
	PointList retList = new PointList();
	
	try {
	    Method m = c.getMethod("lies_on",paramListPS);
	    retList = PointList.convert(SetOps.rdup(SetOps.proj1(SetOps.join(pl,sl,m))));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return pp_equal(retList,pl);
    }//end method pl_on_border_of


    public static boolean pr_on_border_of (PointList pl, TriList tl) {
	//comment missing

	Class c = psOpsClass;
	PointList retList = new PointList();

	try{
	    Method m = c.getMethod("lies_on",paramListPS);
	    retList = PointList.convert(SetOps.rdup(SetOps.proj1(SetOps.join(pl,contour(tl,true,true),m))));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return pp_equal(retList,pl);
    }//end method pr_on_border_of


    public static PointList pp_intersection (PointList pl1, PointList pl2) {
	//comment missing
	PointList retList = new PointList();
	
	try {
	    retList = PointList.convert(SetOps.intersection(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return retList;
    }//end method pp_intersection


    public static PointList ll_intersection (SegList sl1, SegList sl2) {
	//comment missing
	
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
	return PointList.convert(retList);
    }//end method ll_intersection


    public static TriList rr_intersection (TriList tl1, TriList tl2) {
	//comment missing
	TriList retList = new TriList();
	Class c = triClass;
	Class[] paramListT = { c };
	SegList contourP = new SegList();
	
	try {
	    Method m1 = c.getMethod("pintersects",paramListT);
	    Method m2 = c.getMethod("intersection",paramListT);
	    //contourP = contour(TriList.convert(SetOps.map(SetOps.join(tl1,tl2,m1),m2)));
	    //System.out.println("computed contourP");
	    //PairList pl = SetOps.overlapJoin(tl1,tl2,m1);
	    //System.out.println("pairlist:"); pl.print();
	    //ElemList el = SetOps.map(pl,m2);
	    //contourP = contour(TriList.convert(el));
	    //System.out.println("contourP("+contourP.size()+"):"); contourP.print();
	    //System.exit(0);
	    contourP = contour(TriList.convert(SetOps.map(SetOps.overlapJoin(tl1,tl2,m1,false),m2)),false,true);
	    
	    GFXout gg = new GFXout();
	    gg.initWindow();
	    //gg.addList(el);
	    gg.addList(contourP);
	    gg.showIt();
	    
	    
	    //long time3 = System.currentTimeMillis();
	    //PairList tl01 = SetOps.join(tl1,tl2,m1);
	    //long time4 = System.currentTimeMillis();
	    //System.out.println("elapsed time (join):"+(time4-time3)+"ms");
	    //System.out.println("join:"); //tl01.print();
	    //long time1 = System.currentTimeMillis();
	    //PairList tl02 = SetOps.overlapJoin(tl1,tl2,m1);
	    //long time2 = System.currentTimeMillis();
	    //System.out.println("elapsed time (overlapJoin): "+(time2-time1)+"ms");
	    //System.exit(0);
	    //System.out.println("ovjoin:"); tl02.print();
	    //PairList tl03 = SetOps.difference(tl01,tl02);
	    //System.out.println("diffList:");
	    //tl03.print();
	    //System.exit(0);
	    //System.out.println();
	    
	    return new TriList();
	    //retList = Polygons.computeTriangles(contourP);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retList;
    }//end method rr_intersection


    public static SegList rl_intersection (TriList tl, SegList sl) {
	//comment missing

	Class c = stOpsClass;
	SegList retList = new SegList();

	try {
	    Method m1 = c.getMethod("pintersects",paramListST);
	    Method m2 = c.getMethod("intersection",paramListST);

	    //retList = minimal(SegList.convert(SetOps.map(SetOps.join(sl,tl,m1),m2)),false);
	    //retList = minimal(SegList.convert(SetOps.map(SetOps.overlapJoin(sl,tl,m1),m2)),false);
	    //this is without minimal!!!
	    retList = SegList.convert(SetOps.map(SetOps.overlapJoin(sl,tl,m1,true),m2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retList;
    }//return rl_intersection


    public static PointList pp_plus (PointList pl1, PointList pl2) {
	//comment missing
	PointList retList = new PointList();
	try {
	    retList = PointList.convert(SetOps.union(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retList;
    }//end method pp_plus


    public static SegList ll_plus (SegList sl1, SegList sl2) {
	//comment missing
	SegList retList = new SegList();
	
	try {
	    retList = minimal(once((SegList)SetOps.union(sl1,sl2)),false);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return retList;
    }//end method ll_plus


    public static TriList rr_plus (TriList tl1, TriList tl2) {
	return new TriList();
    }
	
    
    public static PointList pp_minus (PointList pl1, PointList pl2) {
	//comment missing
	PointList retList = new PointList();

	try {
	    retList = PointList.convert(SetOps.difference(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return retList;
    }//end method pp_minus


    public static SegList ll_minus (SegList sl1, SegList sl2) {
	//comment missing
	SegList retList = new SegList();
	
	try {
	    
	    long time1 = System.currentTimeMillis();
	    //SegList inters = (SegList)SetOps.intersection(sl1,sl2);
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
	
	return retList;
    }//end method ll_minus


    public static TriList rr_minus (TriList tl1, TriList tl2) {
	//comment missing
	//System.out.println("\n\nentering rr_minus...");
	return minus(tl1,tl2);
    }//end method rr_minus


    public static SegList ll_common_border (SegList sl1, SegList sl2) {
	//comment missing

	Class c = ssOpsClass;
	PairList joinList = new PairList();
	SegList retList = new SegList();
	
	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    joinList = SetOps.overlapJoin(sl1,sl2,m1,false);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (joinList.isEmpty()) { return retList; }
	else {
	    try {
		Method m2 = c.getMethod("theOverlap",paramListSS);
		retList = minimal(SegList.convert(SetOps.map(joinList,m2)),true);
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	}//else
	return retList;

    }//end method ll_common_border


    public static SegList lr_common_border (SegList sl, TriList tl) {
	//comment missing

	//return minimal(overlap((SegList)SetOps.union(sl,contour(tl))));
	return ll_common_border(sl,contour(tl,true,true));
    }//end method lr_common_border


    public static SegList rl_common_border (TriList tl, SegList sl) {
	//comment missing

	return lr_common_border(sl,tl);
    }//end method rl_common_border


    public static SegList rr_common_border (TriList tl1, TriList tl2) {
	//comment missing

	return ll_common_border(contour(tl1,true,true),contour(tl2,true,true));
    }//end method rr_common_border


    public static PointList l_vertices (SegList sl) {
	//comment missing
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

	return retList;
    }//end method l_vertices

    public static PointList r_vertices (TriList tl) {
	return l_vertices(r_contour(tl));
    }//end method r_vertices


    public static TriList l_interior (SegList sl) {
	//comment missing
	Polygons pol = new Polygons(sl);
	return pol.triangles();
    }//end method l_interior


    public static SegList r_contour(TriList tl) {
	//comment missing
	return contour(tl,true,true);
    }//end method r_contour


    public static int p_no_of_components (PointList pl) {
	//comment missing
	return pl.size();
    }//end method p_no_of_components
    

    public static int l_no_of_components (SegList sl) {
	//comment missing
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


    public static int r_no_of_components (TriList tl) {
	//comment missing
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


    public static Rational pp_dist (PointList pl1, PointList pl2) {
	//comment missing

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


    public static Rational pl_dist (PointList pl, SegList sl) {
	//comment missing

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


    public static Rational pr_dist (PointList pl, TriList tl) {
	//comment missing

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


    public static Rational lp_dist (SegList sl, PointList pl) {
	//comment missing

	return pl_dist(pl,sl);
    }//end method lp_dist


    public static Rational ll_dist (SegList sl1, SegList sl2) {
	//comment missing

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


    public static Rational lr_dist (SegList sl, TriList tl) {
	//comment missing

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


    public static Rational rp_dist (TriList tl, PointList pl) {
	//comment missing

	return pr_dist(pl,tl);
    }//end method rp_dist


    public static Rational rl_dist (TriList tl, SegList sl) {
	//comment missing

	return lr_dist(sl,tl);
    }//end method rl_dist


    public static Rational rr_dist (TriList tl1, TriList tl2) {
	//comment missing

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


    public static Rational p_diameter (PointList pl) {
	//comment missing

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


    public static Rational l_diameter (SegList sl) {
	//comment missing

	return p_diameter(l_vertices(sl));
    }//end method l_diameter


    public static Rational r_diameter (TriList tl) {
	//comment missing
	
	return p_diameter(r_vertices(tl));
    }//end method r_diameter


    public static double l_length (SegList sl) {
	//sums up the length of every element of sl
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
  

    public static double r_area (TriList tl) {
	//comment missing

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

    
    public static double r_perimeter (TriList tl) {
	//comment missing

	return l_length(contour(tl,true,true));
    }//end method r_perimeter


}//end class Algebra
