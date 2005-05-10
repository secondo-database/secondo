import twodsack.operation.basictypeoperation.*;
import twodsack.operation.setoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.collection.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;

import java.util.*;
import java.lang.reflect.*;
import java.io.*;

public class ROSEAlgebra {
    /* When using the methods of this algebra, make sure, that you don't use 
     * triangle sets in the REGIONS constructor that are made 'by hand'.
     * Problems arise, if the triangle sets are no proper meshes as generated
     * by the mesh generator. Particularly, the contour operation doesn't
     * work for triangles which have overlapping edges. All triangle edges
     * which are not part of the polygons border must exist exactly in 
     * to triangles. Hence, every inner triangle of a (meshed) polygon
     * has exactly three partners.
     * If you want to use such a 'dirty' triangle set for any reason, 
     * the implementation of some of the below operation has to be changed.
     * SupportOps.contourGeneral has to be used instead of SupportOps.contour, then.
     * It is much slower, but should work in that case.
     */

    //fields
    static final Class POINT_CLASS = (new Point()).getClass();
    static final Class SEG_CLASS = (new Segment()).getClass();
    static final Class TRI_CLASS = (new Triangle()).getClass();
    
    static final Class PS_OPS_CLASS = (new PointSeg_Ops()).getClass();
    static final Class PT_OPS_CLASS = (new PointTri_Ops()).getClass();
    static final Class SS_OPS_CLASS = (new SegSeg_Ops()).getClass();
    static final Class ST_OPS_CLASS = (new SegTri_Ops()).getClass();

    static final Class EMS_CLASS = (new ElemMultiSet(new ElemComparator())).getClass();

    final static Class[] PARAMLIST_P = { POINT_CLASS };
    final static Class[] PARAMLIST_S = { SEG_CLASS };
    final static Class[] PARAMLIST_T = { TRI_CLASS };

    final static Class[] PARAMLIST_PS = { POINT_CLASS, SEG_CLASS };
    final static Class[] PARAMLIST_PT = { POINT_CLASS, TRI_CLASS };
    final static Class[] PARAMLIST_SS = { SEG_CLASS, SEG_CLASS };
    final static Class[] PARAMLIST_ST = { SEG_CLASS, TRI_CLASS };

    final static Class[] PARAMLIST_EMS = { EMS_CLASS };
    
    static Class ELEM_CLASS;

    static {
	try {
	    ELEM_CLASS = Class.forName("twodsack.setelement.Element");
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
    }//static

    final static Class[] PARAMLIST_E = { ELEM_CLASS };
	
    final static PointComparator POINT_COMPARATOR = new PointComparator();
    final static SegmentComparator SEGMENT_COMPARATOR = new SegmentComparator();
    final static TriangleComparator TRIANGLE_COMPARATOR = new TriangleComparator();
    final static ElemComparator ELEMENT_COMPARATOR = new ElemComparator();
    final static ElemPairComparator ELEMPAIR_COMPARATOR = new ElemPairComparator();
    
    //constructors

    //methods
    public static boolean intersects (ElemMultiSet ems1, ElemMultiSet ems2) {
	//This method is used in ll_intersects and rr_intersects

	if (ems1 == null || ems1.isEmpty() || ems2 == null || ems2.isEmpty()) return false;
	
	PairMultiSet pms = new PairMultiSet(ELEMPAIR_COMPARATOR);
	Class [] paramList = new Class [1];
	Class c = ems1.first().getClass();
	
	try {
	    paramList[0] = Class.forName("twodsack.setelement.Element");
	    Method methodINTERSECTS = c.getMethod("intersects",paramList);
	    pms = SetOps.overlapJoin(ems1,ems2,methodINTERSECTS,true,true,false,0);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	if (pms.isEmpty())
	    return false;
	else
	    return true;
    }//end method intersects


    /*************************************************************
     * the following operations are the original ROSE operations *
     ************************************************************/

    
    public static boolean pp_equal (Points p1, Points p2) {
	return SetOps.equal(p1.pointset,p2.pointset);
    }//end method pp_equal
    
    
    public static boolean ll_equal (Lines l1, Lines l2) {
	return SetOps.equal(SupportOps.minimal(l1.segset,true,false),SupportOps.minimal(l2.segset,true,false));
    }//end method ll_equal
    

    public static boolean rr_equal (Regions r1, Regions r2) {	
	return ll_equal(new Lines(SupportOps.contour(r1.triset)),new Lines(SupportOps.contour(r2.triset)));
    }//end method rr_equal

   
    public static boolean pp_unequal (Points p1, Points p2) {
	return !pp_equal(p1,p2); 
    }//end method pp_unequal

    
    public static boolean ll_unequal (Lines l1, Lines l2) {
	return !ll_equal(new Lines(l1.segset),new Lines(l2.segset));
    }//end method ll_unequal
    

    public static boolean rr_unequal (Regions r1, Regions r2) {
	return !rr_equal(r1,r2); 
    }//end method rr_unequal
    
    
    public static boolean pp_disjoint (Points p1, Points p2) {
	return SetOps.disjoint(p1.pointset,p2.pointset);
    }//end method pp_disjoint 
    

    public static boolean ll_disjoint (Lines l1, Lines l2) {
	return SetOps.disjoint(l1.segset,l2.segset);
    }//end method ll_disjoint


    public static boolean rr_disjoint (Regions r1, Regions r2) {
	return SetOps.disjoint(r1.triset,r2.triset);
    }//end method rr_disjoint

    
    public static boolean pr_inside (Points p, Regions r) {
	PointMultiSet retSet = null;
	PairMultiSet pms = null;
	try {
	    Method methodINSIDE = PT_OPS_CLASS.getMethod("inside",PARAMLIST_PT);
	    try {
		pms = SetOps.overlapJoin(p.pointset,r.triset,methodINSIDE,true,true,true,1);
	    } catch (EarlyExit exit) {
		return false;
	    }//catch
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(pms)));
	if (p.pointset.size() == retSet.size())
	    return true;
	else
	    return false;
    }//end method pr_inside
   
    
    public static boolean lr_inside (Lines l, Regions r) {
	SegMultiSet retSet = null;
	try {
	    retSet = SupportOps.minus(l.segset,r.triset,true,false,1);
	} catch (EarlyExit exit) {
	    return false;
	}//catch
	
	if (retSet.size() == 0)
	    return true;
	else
	    return false;
    }//end method lr_inside
    

    public static boolean rr_inside (Regions r1, Regions r2) {
	return rr_minus(r2,r1).triset.isEmpty();
    }//end method rr_inside 
    
    
    public static boolean rr_area_disjoint (Regions r1, Regions r2) {
	PairMultiSet pms = null;
	
	try {
	    Method methodPINTERSECTS = TRI_CLASS.getMethod("pintersects",PARAMLIST_T);
	    pms = SetOps.overlapJoin(r1.triset,r2.triset,methodPINTERSECTS,false,true,false,0);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	if (pms == null || pms.size() == 0)
	    return true;
	else
	    return false;
    }//end method rr_area_disjoint
    
    
    public static boolean rr_edge_disjoint (Regions r1, Regions r2) {
	return (rr_area_disjoint(r1,r2) && !rr_border_in_common(r1,r2));
    }//end method rr_edge_disjoint
   
    
    public static boolean rr_edge_inside (Regions r1, Regions r2) {
	//returns true if r1 is edge_inside of r2
	SegMultiSet r1contour = SupportOps.contour(r1.triset);
	SegMultiSet r2contour = SupportOps.contour(r2.triset);
	return lr_inside(new Lines(r1contour),r2) &&
	    !ll_border_in_common(new Lines(r1contour), new Lines(r2contour));
    }//end method rr_edge_inside
    
    
    public static boolean rr_vertex_inside (Regions r1, Regions r2) {
	//returns true, if r2 is vertex_inside of r1
	PointMultiSet pmsR2 = r_vertices(r2).pointset;
	PairMultiSet pairs = null;
	PointMultiSet insidePoints = null;
	Method methodLIESON = null;

	try {
	    Method methodISCOVERED = PT_OPS_CLASS.getMethod("isCovered",PARAMLIST_PT);
	    methodLIESON = PS_OPS_CLASS.getMethod("liesOn",PARAMLIST_PS);
	    try {
		pairs = SetOps.overlapJoin(pmsR2,r1.triset,methodISCOVERED,true,true,true,1);
	    } catch (EarlyExit exit) {
		return false;
	    }//catch
	    insidePoints = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(pairs))); 
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	if (pmsR2.size != insidePoints.size()) return false;
	    
	//now check, whether any points of insidePoints lie on the border of r1
	PointMultiSet retSet = null;
	
	try {
	    retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(insidePoints,SupportOps.contour(r1.triset),methodLIESON,true,true,true,1))));
	} catch
	    (Exception EarlyExit) {
	    return false;
	}//catch
	if (retSet.size != 0) 
	    return false;
	else
	    return true;
    }//end method rr_vertex_inside
    
    
    public static boolean ll_intersects (Lines l1, Lines l2) {
	//returns true, if l1,l2 have common points
	return intersects(l1.segset,l2.segset);
    }//end method ll_intersects
  

    public static boolean lr_intersects (Lines l, Regions r) {
	PairMultiSet retSet = null;
	
	try {
	    Method mPINTERSECTS = ST_OPS_CLASS.getMethod("pintersects",PARAMLIST_ST);
	    retSet = SetOps.overlapJoin(l.segset,r.triset,mPINTERSECTS,false,true,false,0);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	if (retSet.isEmpty())
	    return false; 
	else
	    return true;
    }//end method lr_intersects
    

    public static boolean rl_intersects (Regions r, Lines l) {
	return lr_intersects(l,r); 
    }//end method rl_intersects 

    
    public static boolean rr_intersects (Regions r1, Regions r2) {
	//returns true if r1,r2 have common points
	return intersects(r1.triset,r2.triset); 
    }//end method rr_intersects 


    public static boolean ll_meets (Lines l1, Lines l2) {
	//Compute a set with all intersecting lines. Then, check whether there are pairs of lines
	//that overlap or pintersect. If any, return false. If there are pairs left, they must be
	//lines that meet. Return true in that case.
	PairMultiSet retSet = null;
	try {
	    Method methodINTERSECTS = SEG_CLASS.getMethod("intersects",PARAMLIST_E);
	    retSet = SetOps.overlapJoin(l1.segset,l2.segset,methodINTERSECTS,true,true,false,0);
	    int rsSize = retSet.size();
	    if (rsSize == 0) return false;
	    Method methodPINTERSECTS = SEG_CLASS.getMethod("pintersects",PARAMLIST_E);
	    retSet = SetOps.filter(retSet,methodPINTERSECTS,false);
	    if (retSet.size() < rsSize) return false;
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    retSet = SetOps.filter(retSet,methodOVERLAP,false);
	    if (retSet.size() < rsSize) return false;					
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return true;
    }//end method ll_meets
  

    public static boolean lr_meets (Lines l, Regions r) {
	return !lr_intersects(l,r) && ll_meets(l,r_contour(r));
    }//end method lr_meets
   

    public static boolean rl_meets (Regions r, Lines l) {
	return lr_meets(l,r);
    }//end method rl_meets

 
    public static boolean rr_meets (Regions r1, Regions r2) {
	PairMultiSet retSet = null;
	try {
	    Method methodPINTERSECTS = TRI_CLASS.getMethod("pintersects",PARAMLIST_T);
	    retSet = SetOps.overlapJoin(r1.triset,r2.triset,methodPINTERSECTS,false,true,false,0);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	return (retSet.size()) == 0 && ll_meets(r_contour(r1),r_contour(r2));
    }//end method rr_meets


    public static boolean ll_border_in_common (Lines l1, Lines l2) {
 	PairMultiSet retSet = null;

	try {
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    retSet = SetOps.overlapJoin(l1.segset,l2.segset,methodOVERLAP,true,true,false,0);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0); 
	}//catch
	if (retSet.isEmpty())
	    return false;
	else
	    return true;
    }//end method ll_border_in_common
    

    public static boolean lr_border_in_common (Lines l, Regions r) {
	PairMultiSet retSet = null;

 	try {
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    retSet = SetOps.overlapJoin(l.segset,SupportOps.contour(r.triset),methodOVERLAP,true,true,false,0);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	if (retSet.isEmpty())
	    return false;
	else
	    return true;
    }//end method lr_border_in_common


    public static boolean rl_border_in_common (Regions r, Lines l) {
	return lr_border_in_common(l,r);
    }//end method rl_border_in_common

    
    public static boolean rr_border_in_common (Regions r1, Regions r2) {
	return ll_border_in_common(new Lines(SupportOps.contour(r1.triset)),new Lines(SupportOps.contour(r2.triset)));
    }//end method rr_border_in_common
    

    public static boolean rr_adjacent (Regions r1, Regions r2) {
	return rr_border_in_common(r1,r2);
    }//end method rr_adjacent
    
	
    public static boolean rr_encloses (Regions r1, Regions r2) {
	//The implementation of this operation works as follows:
	// - compute the cycles of r1 using Polygons.cyclesSegments
	//As result, we get a CycleListList which has for every face
	//of the region a list of cycles. These cycles are the holes
	//of that regions. (Note, that islands in holes are not supported
	//by Polygons.cyclesSegments.)
	// - store all hole cycles in HC
	// - compute a mesh for HC
	// - compute RES = minus(r2,HC)
	// - if RES is empty, return true, otherwise return false
	
	Polygons r1POL = new Polygons(r1.triset);
	CycleListList r1cycles = r1POL.cyclesSegments();

	//store hole cycles in holePolygon
	SegMultiSet collectedHoleSegments = new SegMultiSet(SEGMENT_COMPARATOR);
	Iterator it = r1cycles.iterator();
	Iterator it2 = null;
	Iterator it3 = null;
	CycleList actCycleList;
	LinkedList actCycle;
	while (it.hasNext()) {
	    actCycleList = (CycleList)it.next();
	    //start with the second cycle, if there is one, because the first cycle
	    //is the outer cycle for the actual face
	    if (actCycleList.size() > 1) {
		it2 = actCycleList.listIterator(1);
		while (it2.hasNext()) {
		    actCycle = (LinkedList)it2.next();
		    //store the segments of actCycle in collectedHoleSegments
		    it3 = actCycle.iterator();
		    while (it3.hasNext()) {
			collectedHoleSegments.add((Segment)it3.next());
		    }//while it3
		}//while it2
	    }//if size > 1
	}//while it

	Regions resRegion = rr_minus(r2,new Regions(collectedHoleSegments));

	if (resRegion.triset.size() == 0)
	    return true;
	else
	    return false;
    }//end method rr_encloses 
    
    
    public static boolean pl_on_border_of (Points p, Lines l) {
	PointMultiSet retSet = null;
 	try {
	    Method methodLIESON = PS_OPS_CLASS.getMethod("liesOn",PARAMLIST_PS);
	    try {
		retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(p.pointset,l.segset,methodLIESON,true,false,true,1))));
	    } catch (EarlyExit exit) {
		return false;
	    }//catch
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0); 
	}//catch
	
	return pp_equal(new Points(retSet),p);
    }//end method pl_on_border_of

   
    public static boolean pr_on_border_of (Points p, Regions r) {
	PointMultiSet retSet = null;
	try {
	    Method methodLIESON = PS_OPS_CLASS.getMethod("liesOn",PARAMLIST_PS);
	    try {
		retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(p.pointset,SupportOps.contour(r.triset),methodLIESON,true,true,true,1))));
	    } catch (Exception EarlyExit) {
		return false; 
	    }//catch
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0); 
	}//catch
	
	return pp_equal(new Points(retSet),p);
    }//end method pr_on_border_of
    

    public static Points pp_intersection (Points p1, Points p2) {
	PointMultiSet retSet = null;
	
	try { 
	    retSet = PointMultiSet.convert(SetOps.intersection(p1.pointset,p2.pointset));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return new Points(retSet);
    }//end method pp_intersection

    
    public static Points ll_intersection (Lines l1, Lines l2) {
	ElemMultiSet retSet = null;

	try {
	    Method m1 = SEG_CLASS.getMethod("pintersects",PARAMLIST_E);
	    Method m2 = SEG_CLASS.getMethod("intersection",PARAMLIST_S);
	    retSet = SetOps.rdup(SetOps.map(SetOps.overlapJoin(l1.segset,l2.segset,m1,true,true,false,0),m2));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	return new Points(PointMultiSet.convert(retSet));
    }//end method ll_intersection

    
    public static Regions rr_intersection (Regions r1, Regions r2) {
	return new Regions(SupportOps.intersection(r1.triset,r2.triset,true));
    }//end method rr_intersection
    
    
    public static Lines rl_intersection (Regions r, Lines l) {
	SegMultiSet retSet = null;

	try {
	    Method methodPINTERSECTS = ST_OPS_CLASS.getMethod("pintersects",PARAMLIST_ST);
	    Method methodINTERSECTION = ST_OPS_CLASS.getMethod("intersection",PARAMLIST_ST);
	    
	    retSet = SupportOps.minimal(SegMultiSet.convert(SetOps.map(SetOps.overlapJoin(l.segset,r.triset,methodPINTERSECTS,true,true,false,0),methodINTERSECTION)),true,true);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return new Lines(retSet);
    }//return rl_intersection
    
    
    public static Points pp_plus (Points p1, Points p2) {
	PointMultiSet retSet = null;
	
	try {
	    retSet = PointMultiSet.convert(SetOps.union(p1.pointset,p2.pointset));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	return new Points(retSet);
    }//end method pp_plus
    
    
    public static Lines ll_plus (Lines l1, Lines l2) {
	//Problem with ll_plus: This operation cannot be implemented similar to ll_minus.
	//The reason is that a segment l1 of L can overlap two segments m1,m2 of M. When
	//using the same mechanism, we would get two overlapping segments in the returned set
	//from Segment.plus. Therefore, we choose another algorithm:
	// - compute the union of l1,l2
	// - compute overlapGroup with predicate SSO.overlap
	// - for every such group, call Segment.plus using the group's first element as 
	//   base element and the rest as parameter
	// - return the union of the results of Segment.plus

	SegMultiSet retSet = new SegMultiSet(SEGMENT_COMPARATOR);
	
	//compute union of l1,l2
	SegMultiSet workSet = new SegMultiSet (SEGMENT_COMPARATOR);
	workSet.addAll(l1.segset);
	workSet.addAll(l2.segset);

	ElemMultiSetList emsl = null;

	try {
	    Method methodPLUS = SEG_CLASS.getMethod("plus",PARAMLIST_EMS);
	    
	    //compute overlapGroup
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    emsl = SetOps.overlapGroup(workSet,methodOVERLAP,true);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
       
	//for every group, compute Segment.plus
	Iterator it = emsl.iterator();
	ElemMultiSet actGroup;
	while (it.hasNext()) {
	    actGroup = (ElemMultiSet)it.next();
	    retSet.addAll(((Segment)actGroup.first()).plus(actGroup));
	}//while
	
	return new Lines(retSet);
    }//end method ll_plus
    

    public static Regions rr_plus (Regions r1, Regions r2) {
	return new Regions(SupportOps.plus(r1.triset,r2.triset,false));
    }//end method rr_plus
    
 
    public static Points pp_minus (Points p1, Points p2) {
	PointMultiSet retSet = null;

	try {
	    retSet = PointMultiSet.convert(SetOps.difference(p1.pointset,p2.pointset));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch 

 	return new Points(retSet);
    }//end method pp_minus

    
    public static Lines ll_minus (Lines l1, Lines l2) {
	SegMultiSet retSet = null;
	
	try {
	    Method methodMINUS = SEG_CLASS.getMethod("minus",PARAMLIST_EMS);
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    LeftJoinPairMultiSet ljpms = SetOps.overlapLeftOuterJoin(l1.segset,l2.segset,methodOVERLAP,true,true,false,0);
	    ljpms = SetOps.map(ljpms,null,methodMINUS);
	    retSet = SegMultiSet.convert(SetOps.collect(ljpms));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return new Lines(retSet);
    }//end method ll_minus


    public static Regions rr_minus (Regions r1, Regions r2) {
	return new Regions(SupportOps.minus(r1.triset,r2.triset,true));
    }//end method rr_minus
    
    
    public static Lines ll_common_border (Lines l1, Lines l2) {
	PairMultiSet joinSet = null;
	SegMultiSet retSet = null;
	
	try {
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    joinSet = SetOps.overlapJoin(l1.segset,l2.segset,methodOVERLAP,true,true,false,0);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	if (joinSet.isEmpty()) return new Lines(retSet);
	try {
	    Method methodTHEOVERLAP = SS_OPS_CLASS.getMethod("theOverlap",PARAMLIST_SS);
	    retSet = SupportOps.minimal(SegMultiSet.convert(SetOps.map(joinSet,methodTHEOVERLAP)),true,true);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return new Lines(retSet); 
    }//end method ll_common_border
    

    public static Lines lr_common_border (Lines l, Regions r) {
	return ll_common_border(l,new Lines(SupportOps.contour(r.triset)));
    }//end method lr_common_border


    public static Lines rl_common_border (Regions r, Lines l) {
	return lr_common_border(l,r);
    }//end method rl_common_border
    

    public static Lines rr_common_border (Regions r1, Regions r2) {
	return ll_common_border(new Lines(SupportOps.contour(r1.triset)),new Lines(SupportOps.contour(r2.triset)));
    }//end method rr_common_border
    

    public static Points l_vertices (Lines l) {
 	PointMultiSet retSet = null;
	
	try {
	    Method methodENDPOINTS = SEG_CLASS.getMethod("endpoints",null);
	    retSet = PointMultiSet.convert(SetOps.rdup(SetOps.map(l.segset,methodENDPOINTS)));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	return new Points(retSet);
    }//end method l_vertices

   
    public static Points r_vertices (Regions r) {
	return l_vertices(new Lines(SupportOps.contour(r.triset)));
    }//end method r_vertices
    

    public static Regions l_interior (Lines l) {
	return new Regions(Polygons.computeMesh(l.segset,true));
    }//end method l_interior


    public static Lines r_contour(Regions r) {
	return new Lines(SupportOps.contour(r.triset));
    }//end method r_contour

    
    public static int p_no_of_components (Points p) {
	return SetOps.rdup(p.pointset).size();
    }//end method p_no_of_components
   

    public static int l_no_of_components (Lines l) {
	int retVal = 0;
	
	try {
	    Method methodPOINTSINCOMMON = SS_OPS_CLASS.getMethod("pointsInCommon",PARAMLIST_SS);
	    retVal = (SetOps.overlapGroup(l.segset,methodPOINTSINCOMMON,true)).size();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
 	}//catch
	
	return retVal;
    }//end method l_no_of_components

    
    public static int r_no_of_components (Regions r) {
	int retVal = 0;
	
	try {
	    Method methodINTERSECTS = TRI_CLASS.getMethod("intersects",PARAMLIST_E);
	    retVal = (SetOps.overlapGroup(r.triset,methodINTERSECTS,true)).size();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
 	}//catch
	
	return retVal;
    }//end method r_no_of_components


    public static Rational pp_dist (Points p1, Points p2) {
	//returns -1, if one of p1,p2 is empty
	Rational retVal = RationalFactory.constRational(0);

	try {
	    Method methodDIST = POINT_CLASS.getMethod("dist",PARAMLIST_E);
	    ElemPair retPair = SetOps.min(p1.pointset,p2.pointset,methodDIST);
	    if (retVal != null)
		retVal = retPair.first.dist(retPair.second);
	    else 
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
 	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method pp_dist
    
    
    public static Rational pl_dist (Points p, Lines l) {
	//returns -1, if one of p,l is empty
	Rational retVal = RationalFactory.constRational(0);

	try {
	    Method methodDIST = PS_OPS_CLASS.getMethod("dist",PARAMLIST_PS);
	    ElemPair retPair = SetOps.min(p.pointset,l.segset,methodDIST);
	    if (retVal != null)
		retVal = PointSeg_Ops.dist((Point)retPair.first,(Segment)retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return retVal; 
    }//end mehtod pl_dist

    
    public static Rational pr_dist (Points p, Regions r) {
	//returns -1 if one of p,r is empty
	Rational retVal = RationalFactory.constRational(0);

	try {
	    Method methodDIST = PT_OPS_CLASS.getMethod("dist",PARAMLIST_PT);
	    ElemPair retPair = SetOps.min(p.pointset,r.triset,methodDIST);
	    if (retVal != null) 
		retVal = PointTri_Ops.dist((Point)retPair.first,(Triangle)retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0); 
	}//catch 
	
	return retVal; 
    }//end method pr_dist 

   
    public static Rational lp_dist (Lines l, Points p) {
	//returns -1, if one of l,p is empty
	return pl_dist(p,l);
    }//end method lp_dist
   
    
    public static Rational ll_dist (Lines l1, Lines l2) {
	//returns -1, if l1 or l2 is empty
	Rational retVal = RationalFactory.constRational(0);

	try {
	    Method methodDIST = SEG_CLASS.getMethod("dist",PARAMLIST_E);
	    ElemPair retPair = SetOps.min(l1.segset,l2.segset,methodDIST);
	    if (retVal != null)
		retVal = retPair.first.dist(retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
 	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method ll_dist
   

    public static Rational lr_dist (Lines l, Regions r) {
	//returns -1, if one of l,r is empty
	Rational retVal = RationalFactory.constRational(0);

	try {
	    Method methodDIST = ST_OPS_CLASS.getMethod("dist",PARAMLIST_ST);
	    ElemPair retPair = SetOps.min(l.segset,r.triset,methodDIST);
	    if (retVal != null)
		retVal = SegTri_Ops.dist((Segment)retPair.first,(Triangle)retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
 	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0); 
	}//catch 

	return retVal; 
    }//end method lr_dist 

    
    public static Rational rp_dist (Regions r, Points p) {
	//returns -1, if one of r,p is empty
	return pr_dist(p,r);
    }//end method rp_dist
   

    public static Rational rl_dist (Regions r, Lines l) {
	//returns -1, if oneof r,p is empty
	return lr_dist(l,r);
    }//end method rl_dist

    
    public static Rational rr_dist (Regions r1, Regions r2) {
	//returns -1, if one of r1,r2 is empty
	Rational retVal = RationalFactory.constRational(0);
	
	try {
	    Method methodDIST = TRI_CLASS.getMethod("dist",PARAMLIST_E);
	    ElemPair retPair = SetOps.min(r1.triset,r2.triset,methodDIST);
	    if (retVal != null)
		retVal = retPair.first.dist(retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	return retVal;
    }//end method rr_dist

    
     public static Rational p_diameter (Points p) {
	Rational retVal = RationalFactory.constRational(0);

	try {
	    Method m = POINT_CLASS.getMethod("dist",PARAMLIST_E);
	    ElemPair retPair = SetOps.max(p.pointset,p.pointset,m);
	    retVal = retPair.first.dist(retPair.second);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	return retVal;
     }//end method p_diameter

    
     public static Rational l_diameter (Lines l) {
	return p_diameter(l_vertices(l));
     }//end method l_diameter

   
    public static Rational r_diameter (Regions r) {
	return p_diameter(r_vertices(r));
    }//end method r_diameter
   

    public static double l_length (Lines l) {
	double retSum = 0;
	
	try {
	    Method methodLENGTH = SEG_CLASS.getMethod("length",null);
	    retSum = SetOps.sum(l.segset,methodLENGTH);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	return retSum;
    }//end method l_length

   
    public static double r_area (Regions r) {
	double retVal = 0;
	
	try {
	    Method methodAREA = TRI_CLASS.getMethod("area",null);
	    retVal = SetOps.sum(r.triset,methodAREA);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	return retVal;
    }//end method r_area

   
    public static double r_perimeter (Regions r) {
	return l_length(new Lines(SupportOps.contour(r.triset)));
    }//end method r_perimeter

}//end class ROSEAlgebras
