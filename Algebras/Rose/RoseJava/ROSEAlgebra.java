//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import java.util.*;
import java.lang.reflect.*;

public class ROSEAlgebra {

  //variables
    //static final Rational deriv = new Rational(0.00000000001); //allowed derivation for comparisons to be equal
    //this is used in: Rational,...
    //static final Rational deriv2 = new Rational(0.000000000005); //used in Mathset...

    //static final int NUM_DIGITS = 7; //number of digits used right of the decimal point
    //used in Rational

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
	
	//System.out.println("\nentering RA.minus...");

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

	    //align all segments
	    while (lit.hasNext()) {
		((Segment)lit.next()).align(); }

	    //compute pairlists: 1 segment n intersecting triangles
	    ljpl = SetOps.overlapLeftOuterJoin(sl,tl,m1,true);

	    //System.out.println("List of triangles intersection segments (ljpl):"); ljpl.print();
	    //System.exit(0);

	    //compute from leftjoinlist a new leftjoinlist with
	    //Elem=actual segment and
	    //ElemList=segment parts which have to be subtracted from actual segment
	    ljpl = SetOps.subtractSets(ljpl,m4);

	    //System.out.println("\nList of segments to subtract from actual segment (subtractSets): ");
	    //ljpl.print();
	    //System.exit(0);
	    //System.out.println("leftjoinpairlist:");
	    //ElemList sretlist = SetOps.subtract(ljpl,m5);

	    //sutract all segment parts from actual segments in leftjoinpairlist
	    retList = SegList.convert(SetOps.map(ljpl,m3,m6,m5));
	    //System.out.println("\nRemaining parts of segment (retlist):"); retList.print();
	    //System.exit(0);

	    //retList = SegList.convert(SetOps.map(SetOps.overlapLeftOuterJoin(sl,tl,m1),m3,m1,m2));
	    //System.out.println("c4");
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.minus(SegList,TriList):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
    
	return retList;
    }//end method minus
    

    public static TriList minus (TriList tl1, TriList tl2) {
	//comment missing
	//same as minus(SegList,TriList)
	
	System.out.println("entering ROSEA.minus(tl,tl).. ");
	System.out.println("tl1.size: "+tl1.size()+", tl2.size: "+tl2.size());

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
	    //System.out.println("ROSEA.minus: got all methods");
	    //LeftJoinPairList ljpl = SetOps.overlapLeftOuterJoin(tl1,tl2,m1,false);
	    //System.out.println("ROSEA.minus: finished overlapLeftOuterJoin -- ljpl.size:"+ljpl.size());
	    //ljpl.print();
	    //System.exit(0);
	    retList = TriList.convert(SetOps.map(SetOps.overlapLeftOuterJoin(tl1,tl2,m1,false),m3,m1,m2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.minus(TriList,TriList):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println(0);
	}//catch
	
	System.out.println("leaving ROSEA.minus.");

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
	    System.out.println("Exception was thrown in ROSEAlgebra.overlap(SegList):");
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
	    System.out.println("Exception was thrown in ROSEAlgebra.once(SegList):");
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

	//System.out.println("\nentering ROSEA.minimal...");
	//SegList retList = sl;
	
	//long time1 = System.currentTimeMillis();

	SegList retList = new SegList();
	Class c = ssOpsClass;

	try {
	    Method m1 = c.getMethod("adjacent",paramListSS);
	    Method m2 = c.getMethod("concat",paramListSS);
	    if (!overlap) { retList = (SegList)SetOps.reduce(sl,m1,m2); }
	    else { retList = SegList.convert(SetOps.overlapReduce(sl,m1,m2,true)); }
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.minimal(SegList,boolean):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for minimal: "+(time2-time1)+"ms");
	//System.out.println("leaving ROSEA.minimal.");
	return retList;
    }//end method minimal


    public static SegList unique (SegList sl, boolean overlap) {
	//comment missing
	//overlap toggles whether overlapReduce or simple reduce is used
	//true : use overlapReduce
	//false : use reduce

	//System.out.println("\nentering ROSEA.unique...");

	//long time1 = System.currentTimeMillis();

	SegList retList = new SegList();
	Class c = ssOpsClass;

	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    Method m2 = c.getMethod("symDiff",paramListSS);
	    if (!overlap) { retList = (SegList)SetOps.reduce(sl,m1,m2); }
	    else { retList = SegList.convert(SetOps.overlapReduce(sl,m1,m2,true)); }
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.unique(SegList,boolean):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Exception cause: "+e.getCause());
	    System.out.println("Exception string: "+e.toString());
	    System.out.println("stack trace: ");
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for unique: "+(time2-time1)+"ms");
	//System.out.println("leaving ROSEA.unique.");
	return retList;
    }//end method unique


    public static SegList contour (TriList tl,boolean minOverlap,boolean uniOverlap) {
	//For a set of triangles this method computes the contour and returns it as
	//a set of segments.
	//This is done in three steps:
	// 1) decompose all triangles into their segments
	// 2) use method unique to remove all overlapping parts of segments
	// 3) use method minimal to join connected segments
	//
	//parameters:
	//minOverlap = true:  use overlapReduce for unique
	//minOverlap = false: use reduce for unique
	//uniOverlap = true:  use overlapReduce for minimal
	//minOverlap = false: use reduce for minimal
	
	//System.out.println("\nentering ROSEA.contour...");
	//long time1 = System.currentTimeMillis();
	Class c = triClass;
	SegList retList = new SegList();
	try {
	    Method m = c.getMethod("segments",null);
	    
	    retList = minimal(unique(SegList.convert(SetOps.map(tl,m)),uniOverlap),minOverlap);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.contour(TriList,boolean,boolean):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Exception cause: "+e.getCause());
	    System.out.println("Exception string: "+e.toString());
	    System.out.println("stack trace:"); e.printStackTrace();
	    System.exit(0);
	}//catch
	//System.out.println("contour: "); retList.print();
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for contour("+minOverlap+","+uniOverlap+"): "+(time2-time1)+"ms");
	//System.out.println("leaving ROSEA.contour.");
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
	    System.out.println("Exception was thrown in ROSEAlgebra.intersects(ElemList,ElemList):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method intersects


    /************************************************************
     *the following operations are the original ROSE operations *
     ***********************************************************/


    public static boolean pp_equal (Points p1, Points p2) {
	//comment missing
	//long time1 = System.currentTimeMillis();

	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	boolean retVal = false;

	retVal = SetOps.equal(pl1,pl2);
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for pp_equal: "+(time2-time1)+"ms");
	return retVal;
    }//end method pp_equal


    public static boolean ll_equal (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;
	boolean retVal = false;
	retVal = SetOps.equal(minimal(sl1,false),minimal(sl2,false));
	return retVal;
    }//end method ll_equal

    public static boolean rr_equal (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;

	return ll_equal(new Lines(contour(tl1,true,true)),new Lines(contour(tl2,true,true)));
    }//end method rr_equal


    public static boolean pp_unequal (Points p1, Points p2) {
	//comment missing
	return !pp_equal(p1,p2);
    }//end method pp_unequal


    public static boolean ll_unequal (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;

	return (!ll_equal(new Lines(sl1),new Lines(sl2)));
    }//end method ll_unequal


    public static boolean rr_unequal (Regions r1, Regions r2) {
	//comment missing

	return (!rr_equal(r1,r2));
    }//end method rr_unequal


    public static boolean pp_disjoint (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;

	return (SetOps.disjoint(pl1,pl2));
    }//end method pp_disjoint
	

    public static boolean ll_disjoint (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;

	return (SetOps.disjoint(sl1,sl2));
    }//end method ll_disjoint


    public static boolean rr_disjoint (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;

	return (SetOps.disjoint(tl1,tl2));
    }//end method rr_disjoint


    public static boolean pr_inside (Points p, Regions r) {
	//comment missing
	//long time1 = System.currentTimeMillis();
	PointList pl = p.pointlist;
	TriList tl = r.trilist;

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
	    System.out.println("Exception was thrown in ROSEAlgebra.pr_inside(Points,Regions):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for pr_inside: "+(time2-time1)+"ms");
	if (pl.size() == retList.size()) { return true; }
	else { return false; }
    }//end method pr_inside

    
    public static boolean lr_inside (Lines l, Regions r) {
	//comment missing
	SegList sl = l.seglist;
	TriList tl = r.trilist;

	SegList retList = minus(sl,tl);
	//System.out.println("RA.lr_inside: result of minus.size(): "+retList.size());
	/*
	GFXout g = new GFXout();
	g.initWindow();
	g.addList(retList.copy());
	g.showIt();
	try { int data = System.in.read(); }
	catch (Exception e) { System.exit(0); }
	g.kill();
	*/
	if (retList.size() == 0) { return true; }
	else { return false; }
    }//end method lr_inside


    public static boolean rr_inside (Regions r1, Regions r2) {
	System.out.println("rr_inside is currently not implemented.");
	return false;
    }


    public static boolean rr_area_disjoint (Regions r1, Regions r2) {
	System.out.println("rr_area_disjoint is currently not implemented.");
	return false;
    }


    public static boolean rr_edge_disjoint (Regions r1, Regions r2) {
	System.out.println("rr_edge_disjoint is currently not implemented.");
	return false;
    }
    

    public static boolean rr_edge_inside (Regions r1, Regions r2) {
	System.out.println("rr_edge_inside is currently not implemented.");
	return false;
    }


    public static boolean rr_vertex_inside (Regions r1, Regions r2) {
	System.out.println("rr_vertex_inside is currently not implemented.");
	return false;
    }


    public static boolean ll_intersects (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;

	return intersects(sl1,sl2);
    }//end method ll_intersects


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
	    System.out.println("Exception was thrown in lr_intersects(Lines,Regions):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method lr_intersects


    public static boolean rl_intersects (Regions r, Lines l) {
	//comment missing

	return lr_intersects(l,r);
    }//end method rl_intersects


    public static boolean rr_intersects (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;

	return intersects(tl1,tl2);
    }//end method rr_intersects


    public static boolean ll_meets (Lines l1, Lines l2) {
	System.out.println("ll_meets is currently not implemented.");
	return false;
    }
	

    public static boolean lr_meets (Lines l, Regions r) {
	System.out.println("lr_meets is currently not implemented.");
	return false;
    }


    public static boolean rl_meets (Regions r, Lines l) {
	System.out.println("rl_meets is currently not implemented.");
	return false;
    }


    public static boolean rr_meets (Regions r1, Regions r2) {
	System.out.println("rr_meets is currently not implemented.");
	return false;
    }


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
	    System.out.println("Exception was thrown in ROSEAlgebra.ll_border_in_common(Lines,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method ll_border_in_common


    public static boolean lr_border_in_common (Lines l, Regions r) {
	//comment missing
	SegList sl = l.seglist;
	TriList tl = r.trilist;

	Class c = ssOpsClass;
	PairList retList = new PairList();

	try {
	    Method m = c.getMethod("overlap",paramListSS);
	    retList = SetOps.join(sl,contour(tl,true,true),m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.lr_border_in_common(Lines,Regions)");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return false; }
	else { return true; }
    }//end method lr_border_in_common


    public static boolean rl_border_in_common (Regions r, Lines l) {
	//comment missing

	return lr_border_in_common(l,r);
    }//end method rl_border_in_common


    public static boolean rr_border_in_common (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;

	return ll_border_in_common(new Lines(contour(tl1,true,true)),new Lines(contour(tl2,true,true)));
    }//end method rr_border_in_common


    public static boolean rr_adjacent (Regions r1, Regions r2) {
	//comment missing

	return rr_border_in_common(r1,r2);
    }//end method rr_adjacent
      
	
    public static boolean rr_encloses (Regions r1, Regions r2) {
	System.out.println("rr_encloses is currently not implemented");
	return false;
    }


    public static boolean pl_on_border_of (Points p, Lines l) {
	//comment missing
	PointList pl = p.pointlist;
	SegList sl = l.seglist;

	Class c = psOpsClass;
	PointList retList = new PointList();
	
	try {
	    Method m = c.getMethod("lies_on",paramListPS);
	    retList = PointList.convert(SetOps.rdup(SetOps.proj1(SetOps.join(pl,sl,m))));
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pl_on_border_of(Points,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return pp_equal(new Points(retList),p);
    }//end method pl_on_border_of


    public static boolean pr_on_border_of (Points p, Regions r) {
	//comment missing
	PointList pl = p.pointlist;
	TriList tl = r.trilist;

	Class c = psOpsClass;
	PointList retList = new PointList();

	try{
	    Method m = c.getMethod("lies_on",paramListPS);
	    retList = PointList.convert(SetOps.rdup(SetOps.proj1(SetOps.join(pl,contour(tl,true,true),m))));
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pr_on_border_of(Points,Regions):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return pp_equal(new Points(retList),p);
    }//end method pr_on_border_of


    public static Points pp_intersection (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;

	PointList retList = new PointList();
	
	try {
	    retList = PointList.convert(SetOps.intersection(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pp_intersection(Points,Points):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return new Points(retList);
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
	    System.out.println("Exception was thrown in ROSEAlgebra.ll_intersection(Lines,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return new Points(PointList.convert(retList));
    }//end method ll_intersection


    public static Regions rr_intersection (Regions r1, Regions r2) {
	//comment missing
	
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	
	TriList retList = new TriList();
	Class c = triClass;
	Class[] paramListT = { c };
	SegList contourP = new SegList();
	
	try {
	    Method m1 = c.getMethod("pintersects",paramListT);
	    Method m2 = c.getMethod("intersection",paramListT);
	    System.out.println("RA.rr_intersection: Got methods");
	    PairList pl = SetOps.overlapJoin(tl1,tl2,m1,false);
	    System.out.println("RA.rr_intersection: overlapJoin finished pl.size:"+pl.size());
	    
	    ElemList el = SetOps.map(pl,m2);
	    System.out.println("RA.rr_intersection: map finished");
	    contourP = contour(TriList.convert(el),false,true);
	    System.out.println("RA.rr_intersection: contourP("+contourP.size()+") computation finished"); //contourP.print();
	 
	    //++++++++
	    //this is the right line:
	    //+++++++ 
	    //contourP = contour(TriList.convert(SetOps.map(SetOps.overlapJoin(tl1,tl2,m1,false),m2)),false,true);
	    
	    retList = Polygons.computeTriangles(contourP);
	    //System.out.println("RA.rr_intersection: computeTriangles finished");
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.rr_intersection(Regions,Regions):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    e.printStackTrace();
	    System.exit(0);
	}//catch 
	Regions regret = new Regions(retList);
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for rr_intersection: "+(time2-time1)+"ms");
	return regret;
	 
    }//end method rr_intersection


    public static Lines rl_intersection (Regions r, Lines l) {
	//comment missing
	TriList tl = r.trilist;
	SegList sl = l.seglist;

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
	    System.out.println("Exception was thrown in ROSEAlgebra.rl_intersection(Regions,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return new Lines(retList);
    }//return rl_intersection


    public static Points pp_plus (Points p1, Points p2) {
	//comment missing
	//long time1 = System.currentTimeMillis();

	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;

	PointList retList = new PointList();
	try {
	    retList = PointList.convert(SetOps.union(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pp_plus(Points,Points):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for pp_plus: "+(time2-time1)+"ms");
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
	    System.out.println("Exception was thrown in ROSEAlgebra.ll_plus(Lines,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return new Lines(retList);
    }//end method ll_plus


    public static Regions rr_plus (Regions r1, Regions r2) {
	//comment missing
	
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;
	
	TriList min = minus(tl2,tl1);
	System.out.println("RA.rr_plus: #triangles after minus: "+min.size());

	TriList sum = TriList.convert(SetOps.disjointUnion(tl1,min));
	System.out.println("RA.rr_plus: #triangles after disjointUnion: "+sum.size());
	
	SegList contour = new SegList();
	if (sum.size() > 0) {
	    contour = contour(min,true,true);
	}//if
	System.out.println("leaving RA.rr_plus...");
	return new Regions(contour);
    }//end method rr_plus
	
    
    public static Points pp_minus (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;
	
	PointList retList = new PointList();

	try {
	    retList = PointList.convert(SetOps.difference(pl1,pl2));
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pp_minus(Points,Points):");
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
	    System.out.println("Exception was thrown in ROSEAlgebra.ll_minus(Lines,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return new Lines(retList);
    }//end method ll_minus


    public static Regions rr_minus (Regions r1, Regions r2) {
	//comment missing
	System.out.println("entering ROSEA.rr_minus...");
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;

	/*
	GFXout gt = new GFXout();
	Rational fact = RationalFactory.constRational(40);
	TriList tlcop = (TriList)tl1.copy();
	tlcop.addAll(tl2.copy());
	for (int i = 0; i < tlcop.size(); i++) {
	    ((Triangle)tlcop.get(i)).zoom(fact); }
	gt.initWindow();
	gt.addList(tlcop);
	gt.showIt();
	try { int data = System.in.read(); }
	catch (Exception e) { System.exit(0); }
	gt.kill();
	*/
	TriList min = minus(tl1,tl2);
	System.out.println("ROSEA.rr_minus: elements after minus: "+min.size());
	/*
	GFXout gt = new GFXout();
	gt.initWindow();
	gt.addList(min.copy());
	gt.showIt();
	try { int data = System.in.read(); }
	catch (Exception e) { System.exit(0); }
	gt.kill();
	*/
	SegList cont = new SegList();
	if (min.size() > 0) {
	    System.out.println("RA.rr_minus reducing number of triangles...");
	    
	    //reduce the numbers of triangles by computing contour
	    cont = contour(min,true,true);
	}//if
	
	System.out.println("leaving ROSEA.rr_minus.");
	return new Regions(cont);
	//return new Regions(minus(tl1,tl2));
    }//end method rr_minus


    public static Lines ll_common_border (Lines l1, Lines l2) {
	//comment missing
	SegList sl1 = l1.seglist;
	SegList sl2 = l2.seglist;

	Class c = ssOpsClass;
	PairList joinList = new PairList();
	SegList retList = new SegList();
	/*
	System.out.println("reduce contour... size:"+sl1.size());
	for (int i = 0; i < 455; i++) sl1.removeFirst();
	sl1.print();
	GFXout zui = new GFXout();
	zui.initWindow();
	zui.addList(sl1);
	zui.showIt();
	try { int data1 = System.in.read(); }
	catch (Exception e) { System.exit(0); }
	System.exit(0);
	*/
	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    joinList = SetOps.overlapJoin(sl1,sl2,m1,false);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.ll_common_border(Lines,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (joinList.isEmpty()) { return new Lines(retList); }
	else {
	    /*
	    GFXout oiu = new GFXout();
	    oiu.initWindow();
	    oiu.addList(SetOps.proj1(joinList));
	    oiu.showIt();
	    try { int data = System.in.read(); }
	    catch (Exception e) { System.exit(0); }
	    System.exit(0);
	    */
	    try {
		Method m2 = c.getMethod("theOverlap",paramListSS);
		retList = minimal(SegList.convert(SetOps.map(joinList,m2)),true);
	    }//try
	    catch (Exception e) {
		System.out.println("Exception was thrown in ROSEAlgebra.ll_common_border(Lines,Lines):");
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
	return ll_common_border(l,new Lines(contour(tl,true,true)));
    }//end method lr_common_border


    public static Lines rl_common_border (Regions r, Lines l) {
	//comment missing

	return lr_common_border(l,r);
    }//end method rl_common_border


    public static Lines rr_common_border (Regions r1, Regions r2) {
	//comment missing
	TriList tl1 = r1.trilist;
	TriList tl2 = r2.trilist;

	return ll_common_border(new Lines(contour(tl1,true,true)),new Lines(contour(tl2,true,true)));
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
	    System.out.println("Exception was thrown in ROSEAlgebra.l_vertices(Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	return new Points(retList);
    }//end method l_vertices

    public static Points r_vertices (Regions r) {
	return l_vertices(r_contour(r));
    }//end method r_vertices


    public static Regions l_interior (Lines l) {
	//comment missing
	SegList sl = l.seglist;
	Polygons pol = new Polygons(sl);
	return new Regions(pol.triangles());
    }//end method l_interior


    public static Lines r_contour(Regions r) {
	//comment missing
	TriList tl = r.trilist;
	return new Lines(contour(tl,true,true));
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
	    System.out.println("Exception was thrown in ROSEAlgebra.l_no_of_components(Lines):");
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
	    System.out.println("Exception was thrown in ROSEAlgebra.r_no_of_components(Regions):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method r_no_of_components


    public static Rational pp_dist (Points p1, Points p2) {
	//comment missing
	PointList pl1 = p1.pointlist;
	PointList pl2 = p2.pointlist;

	Rational retVal = RationalFactory.constRational(0);
	Class c = pointClass;
	Class[] paramList = new Class[1];

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.min(pl1,pl2,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pp_dist(Points,Points):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method pp_dist


    public static Rational pl_dist (Points p, Lines l) {
	//comment missing
	PointList pl = p.pointlist;
	SegList sl = l.seglist;

	Rational retVal = RationalFactory.constRational(0);
	Class c = psOpsClass;

	try {
	    Method m = c.getMethod("dist",paramListPS);
	    ElemPair retPair = SetOps.min(pl,sl,m);
	    retVal = PointSeg_Ops.dist((Point)retPair.first,(Segment)retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pl_dist(Points,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end mehtod pl_dist


    public static Rational pr_dist (Points p, Regions r) {
	//comment missing
	PointList pl = p.pointlist;
	TriList tl = r.trilist;

	Rational retVal = RationalFactory.constRational(0);
	Class c = ptOpsClass;

	try {
	    Method m = c.getMethod("dist",paramListPT);
	    ElemPair retPair = SetOps.min(pl,tl,m);
	    retVal = PointTri_Ops.dist((Point)retPair.first,(Triangle)retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.pr_dist(Points,Regions):");
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

	Rational retVal = RationalFactory.constRational(0);
	Class c = segClass;
	Class[] paramList = new Class[1];

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.min(sl1,sl2,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.ll_dist(Lines,Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method ll_dist


    public static Rational lr_dist (Lines l, Regions r) {
	//comment missing
	SegList sl = l.seglist;
	TriList tl = r.trilist;

	Rational retVal = RationalFactory.constRational(0);
	Class c = stOpsClass;

	try {
	    Method m = c.getMethod("dist",paramListST);
	    ElemPair retPair = SetOps.min(sl,tl,m);
	    retVal = SegTri_Ops.dist((Segment)retPair.first,(Triangle)retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.lr_dist(Lines,Regions):");
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

	Rational retVal = RationalFactory.constRational(0);
	Class c = triClass;
	Class[] paramList = new Class[1];
	
	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.min(tl1,tl2,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.rr_dist(Regions,Regions):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retVal;
    }//end method rr_dist


    public static Rational p_diameter (Points p) {
	//comment missing
	PointList pl = p.pointlist;

	Rational retVal = RationalFactory.constRational(0);
	Class c = pointClass;
	Class[] paramList = new Class[1];

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("dist",paramList);
	    ElemPair retPair = SetOps.max(pl,pl,m);
	    retVal = retPair.first.dist(retPair.second);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.p_diameter(Points):");
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

	//long time1 = System.currentTimeMillis();
	
	SegList sl = l.seglist;
	double retSum = 0;
	Class c = segClass;
	try {
	    Method m = c.getMethod("length",null);
	    retSum = SetOps.sum(sl,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in ROSEAlgebra.l_length(Lines):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for l_length: "+(time2-time1)+"ms");
	return retSum;
    }//end method l_length
  

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
	    System.out.println("Exception was thrown in ROSEAlgebra.r_area(Regions):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method r_area

    
    public static double r_perimeter (Regions r) {
	//comment missing
	//long time1 = System.currentTimeMillis();
	
	TriList tl = r.trilist;
	double length = l_length(new Lines(contour(tl,true,true)));

	//long time2 = System.currentTimeMillis();
	//System.out.println("-->elapsed time for r_perimeter: "+(time2-time1)+"ms");
	return length;
    }//end method r_perimeter


}//end class Algebra
