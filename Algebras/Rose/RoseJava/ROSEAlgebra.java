import twodsack.operation.basictypeoperation.*;
import twodsack.operation.setoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;

import twodsack.io.*;
import twodsack.util.meshgenerator.*;

import java.util.*;
import java.lang.reflect.*;
import java.io.*;


/**
 * The ROSEAlgebra class implements a lot of static operations for the ROSE's data types Points, Lines and Regions.
 * The algebra itself was invented and described in two papers <p><ul>
 * <li>R.H. Güting and M. Schneider, Realm-Based Spatial Data Types: The ROSE Algebra. VLDB Journal 4 (1995), 100-143.
 * <li>R.H. Güting, Th. de Ridder, and M. Schneider, Implementation of the ROSE Algebra: Efficient Algorithms for Realm-Based Spatial Data Types. Proc. of the 4th Intl. Symposium on Large Spatial Databases (Portland, August 1995), 216-239
 * </ul><p>
 * This implementation of the ROSE algebra and its data types uses the 2D-SACK approach. That appraoch was evented by Dirk Ansorge
 * and R.H. Güting. It shows how simplices and operations on them together with some tricky set operations can be used
 * to re-implement the hole ROSE algebra which usually is implemented using plane-sweep algorithms. Those algorithms generally are
 * very complex and hard to implement. Mostly, extension cannot be made or are very difficult to implement.
 * <p>
 * This work is part of the Ph.D. thesis of Dirk Ansorge.
 * <p>
 */
public class ROSEAlgebra {
    
    /*
     * fields
     */
    //some static class definitions which are used over and over in the code below
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
    

    /*
     * constructors
     */
    /**
     * Don't use this constructor.
     */
    private ROSEAlgebra() {}


    /*
     * methods
     */
    /**
     * This is a supportive method for ll_intersects and rr_intersects.
     * It returns <tt>true</tt> if elements of both sets intersect. The <tt>intersects</tt> method of the elements is used for this
     * predicate.
     *
     * @param ems1 the first set of elements
     * @param ems2 the second set of elements
     * @return <tt>true</tt> if there is at least one pair of intersecting elments
     */
    private static boolean intersects (ElemMultiSet ems1, ElemMultiSet ems2) {
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

    /**
     * Returns <tt>true</tt> if both Points values are equal.
     *
     * @param p1 the first point set
     * @param p2 the second point set
     * @return <tt>true</tt> if both values are equal
     */    
    public static boolean pp_equal (Points p1, Points p2) {
	try {
	    return SetOps.equal(p1.pointset,p2.pointset);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pp_equal. Return false by default.");
	}//catch
	return false;
    }//end method pp_equal
    
    
    /**
     * Returns <tt>true</tt> if both Lines values are equal.
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return <tt>true</tt> if both values are equal
     */
    public static boolean ll_equal (Lines l1, Lines l2) {
	try {
	    return SetOps.equal(SupportOps.minimal(l1.segset,true,false,false),SupportOps.minimal(l2.segset,true,false,false));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_equal. Returning false by default.");
	}//catch
	return false;
    }//end method ll_equal
    

    /**
     * Returns <tt>true</tt> if both Regions values are equal.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt> if both values are equal
     */
    public static boolean rr_equal (Regions r1, Regions r2) {	
	try {
	    return ll_equal(new Lines(SupportOps.contour(r1.triset,true,false)),new Lines(SupportOps.contour(r2.triset,true,false)));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_equal. Returning false by default.");
	}//catch
	return false;
    }//end method rr_equal

   
    /**
     * Returns <tt>true</tt> if the Points values are not equal.
     *
     * @param p1 the first Points value
     * @param p2 the second Points value
     * @return <tt>true</tt> if the values are not equal
     */
    public static boolean pp_unequal (Points p1, Points p2) {
	try {
	    return !pp_equal(p1,p2); 
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pp_unequal. Returning false by default.");
	}//catch
	return false;
    }//end method pp_unequal

    
    /**
     * Returns <tt>true</tt> if the Lines values are not equal.
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return <tt>true</tt> if the values are not equal
     */
    public static boolean ll_unequal (Lines l1, Lines l2) {
	try {
	    return !ll_equal(new Lines(l1.segset),new Lines(l2.segset));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_unequal. Returning false by default.");
	}//catch
	return false;
    }//end method ll_unequal
    

    /**
     * Returns <tt>true</tt> if the Regions values are not equal.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt> if the Regions are not equal
     */
    public static boolean rr_unequal (Regions r1, Regions r2) {
	try {
	    return !rr_equal(r1,r2); 
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_unequal. Returning false by default.");
	}//catch
	return false;
    }//end method rr_unequal
    
    
    /**
     * Returns <tt>true</tt> if there is no single point which is element of both Points values.
     *
     * @param p1 the first Points value
     * @param p2 the second Points value
     * @return <tt>true</tt> if the intersection of both Points values is empty
     */
    public static boolean pp_disjoint (Points p1, Points p2) {
	try {
	    return SetOps.disjoint(p1.pointset,p2.pointset);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pp_disjoint. Returning false by default.");
	}//catch
	return false;
    }//end method pp_disjoint 
    

    /**
     * Returns <tt>true</tt> if no two elements of both Lines values intersect.
     * Common points of two segments of the Lines values suffice to make this predicate true.
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return <tt>true</tt>, if two intersecting segments exist
     */
    public static boolean ll_disjoint (Lines l1, Lines l2) {
	try {
	    return SetOps.disjoint(l1.segset,l2.segset);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_disjoint. Returning false by default.");
	}//catch
	return false;
    }//end method ll_disjoint


    /**
     * Returns <tt>true</tt> if no two elements of both Regions values intersect.
     * Common points on the border of the Regions values suffice to make this predicate true.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt>, if the Regions values have at least one common point
     */
    public static boolean rr_disjoint (Regions r1, Regions r2) {
	try {
	    return SetOps.disjoint(r1.triset,r2.triset);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_disjoint. Returning false by default.");
	}//catch
	return false;
    }//end method rr_disjoint

    
    /**
     * Returns <tt>true</tt>, if <tt>p</tt> lies inside of <tt>r</tt>.
     * If <tt>p</tt> lies on the border of <tt>r</tt> <tt>false</tt> is returned.
     *
     * @param p the Points values
     * @param r the Regions value
     * @return <tt>true</tt>, if <tt>p</tt> properly lies inside of <tt>r</tt>
     */
    public static boolean pr_inside (Points p, Regions r) {
	try {
	    PointMultiSet retSet = null;
	    PairMultiSet pms = null;
	    Method methodINSIDE = PT_OPS_CLASS.getMethod("inside",PARAMLIST_PT);
	    try {
		pms = SetOps.overlapJoin(p.pointset,r.triset,methodINSIDE,true,true,true,1);
	    } catch (EarlyExit exit) {
		return false;
	    }//catch
	    
	    retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(pms)));
	    if (p.pointset.size() == retSet.size())
		return true;
	    else
		return false;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pr_inside. Returning false by default.");
	}//catch
	return false;
    }//end method pr_inside
   
    
    /**
     * Returns <tt>true</tt>, if <tt>l</tt> lies inside of <tt>r</tt>.
     *
     * @param l the Lines value
     * @param r the Regions value
     * @return <tt>true</tt>, if <tt>l</tt> lies inside of <tt>r</tt>
     */
    public static boolean lr_inside (Lines l, Regions r) {
	try {
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
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.lr_inside. Returning false by default.");
	}//catch
	return false;
    }//end method lr_inside
    

    /**
     * Returns <tt>true</tt>, if one Regions value lies fully inside of the other one.
     * I.e. one region must be completely covered by the other region to make this predicate true.
     * 
     * @param r1 the Regions value that covers the other one
     * @param r2 the Regions value that must be covered
     * @param <tt>true</tt>, if <tt>r2</tt> is covered by <tt>r1</tt>
     */
    public static boolean rr_inside (Regions r1, Regions r2) {
	try {
	    return rr_minus(r2,r1).triset.isEmpty();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_inside. Returning false by default.");
	}//catch
	return false;
    }//end method rr_inside 
    
    
    /**
     * Returns <tt>true</tt> if both Regions values have no common area.
     *
     * @param r1 the first region
     * @param r2 the second region
     * @return <tt>true</tt>, if no common area exists
     */
    public static boolean rr_area_disjoint (Regions r1, Regions r2) {
	try {
	    PairMultiSet pms = null;
	    
	    Method methodPINTERSECTS = TRI_CLASS.getMethod("pintersects",PARAMLIST_T);
	    pms = SetOps.overlapJoin(r1.triset,r2.triset,methodPINTERSECTS,false,true,false,0);
	    
	    if (pms == null || pms.size() == 0)
		return true;
	    else
		return false;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_area_disjoint. Returning false by default.");
	}//catch
	return false;
    }//end method rr_area_disjoint
    
    
    /**
     * Returns <tt>true</tt>, if both Regions have no common border.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt>, if no common border exists
     */
    public static boolean rr_edge_disjoint (Regions r1, Regions r2) {
	try {
	    return (rr_area_disjoint(r1,r2) && !rr_border_in_common(r1,r2));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_edge_disjoint. Returning false by default.");
	}//catch
	return false;
    }//end method rr_edge_disjoint
   
    
    /**
     * Returns <tt>true</tt> if <tt>r1</tt> is edge_inside of <tt>r2</tt>.
     * For <tt>r1</tt> being edge_inside of <tt>r2</tt>, it must have its complete border inside of <tt>r2</tt> and 
     * both regions must not have some overlapping border segments.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt>, if <rr>r1</tt> is edge_inside of <tt>r2</tt>
     */
    public static boolean rr_edge_inside (Regions r1, Regions r2) {
	try {
	    SegMultiSet r1contour = SupportOps.contour(r1.triset,true,false);
	    SegMultiSet r2contour = SupportOps.contour(r2.triset,true,false);
	    return lr_inside(new Lines(r1contour),r2) &&
		!ll_border_in_common(new Lines(r1contour), new Lines(r2contour));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_edge_inside. Returning false by default.");
	}//catch
	return false;
    }//end method rr_edge_inside
    
    
    /**
     * Returns <tt>true</tt>, if <tt>r2</tt> is vertex_inside of <tt>r1</tt>.
     * To be vertex_inside of <tt>r1</tt>, <tt>r2</tt> must lie completely inside of <tt>r1</tt> and must not 
     * have equal vertices.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt>, if <tt>r2</tt> is vertex_inside of <tt>r1</tt>
     */
    public static boolean rr_vertex_inside (Regions r1, Regions r2) {
	try {
	    PointMultiSet pmsR2 = r_vertices(r2).pointset;
	    PairMultiSet pairs = null;
	    PointMultiSet insidePoints = null;
	    Method methodLIESON = null;
	    
	    Method methodISCOVERED = PT_OPS_CLASS.getMethod("isCovered",PARAMLIST_PT);
	    methodLIESON = PS_OPS_CLASS.getMethod("liesOn",PARAMLIST_PS);
	    try {
		pairs = SetOps.overlapJoin(pmsR2,r1.triset,methodISCOVERED,true,true,true,1);
	    } catch (EarlyExit exit) {
		return false;
	    }//catch
	    insidePoints = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(pairs))); 
	    
	    if (pmsR2.size != insidePoints.size()) return false;
	    
	    //now check, whether any points of insidePoints lie on the border of r1
	    PointMultiSet retSet = null;
	    
	    try {
		retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(insidePoints,SupportOps.contour(r1.triset,true,false),methodLIESON,true,true,true,1))));
	    } catch
		(Exception EarlyExit) {
		return false;
	    }//catch
	    if (retSet.size != 0) 
		return false;
	    else
		return true;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_vertex_inside. Returning false by default.");
	}//catch
	return false;
    }//end method rr_vertex_inside
    
    
    /**
     * Returns <tt>true</tt>, if at least one pair of segments of <tt>l1,l2</tt> has a common point.
     * A proper intersection point is not needed for this predicate to hold.
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return <tt>true</tt> if <tt>l1,l2</tt> intersect
     */
    public static boolean ll_intersects (Lines l1, Lines l2) {
	try {
	    return intersects(l1.segset,l2.segset);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_intersects. Returning false by default.");
	}//catch
	return false;
    }//end method ll_intersects
  

    /**
     * Returns <tt>true</tt>, if <tt>l,r</tt> properly intersect.
     * This predicate doesn't hold, if <tt>l</tt> lies on the border of <tt>r</tt>.
     *
     * @param l the Lines value
     * @param r the Regions value
     * @return <tt>true</tt>, if the intersection of <tt>l,r</tt> is a Lines value
     */
    public static boolean lr_intersects (Lines l, Regions r) {
	try {
	    PairMultiSet retSet = null;
	    
	    Method mPINTERSECTS = ST_OPS_CLASS.getMethod("pintersects",PARAMLIST_ST);
	    retSet = SetOps.overlapJoin(l.segset,r.triset,mPINTERSECTS,false,true,false,0);
	
	    if (retSet.isEmpty())
		return false; 
	    else
		return true;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.lr_intersects. Returning false by default.");
	}//catch
	return false;
    }//end method lr_intersects
    

    /**
     * Returns <tt>true</tt>, if <tt>l,r</tt> properly intersect.
     * This predicate doesn't hold, if <tt>l</tt> lies on the border of <tt>r</tt>.
     *
     * @param r the Regions value
     * @param l the Lines value
     * @return <tt>true</tt>, if the intersection of <tt>l,r</tt> is a Lines value
     */
    public static boolean rl_intersects (Regions r, Lines l) {
	try {
	    return lr_intersects(l,r); 
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rl_intersects. Returning false by default.");
	}//catch
	return false;
    }//end method rl_intersects 

    
    /**
     * Returns <tt>true</tt>, if <tt>r1,r2</tt> have common points.
     * A common area is not needed for this predicate to hold.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt> if <tt>r1,r2</tt> intersect
     */
    public static boolean rr_intersects (Regions r1, Regions r2) {
	try {
	    return intersects(r1.triset,r2.triset); 
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_intersects. Returning false by default.");
	}//catch
	return false;
    }//end method rr_intersects 


    /**
     * Returns <tt>true</tt>, if <tt>l1,l2</tt> meet.
     * There must be at least one pair of line segments that meets, i.e. they have one common point. But there must be
     * no pair of line segments that properly intersects.
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return <tt>true</tt>, if <tt>l1,l2</tt> meet
     */
    public static boolean ll_meets (Lines l1, Lines l2) {
	//Compute a set with all intersecting lines. Then, check whether there are pairs of lines
	//that overlap or pintersect. If any, return false. If there are pairs left, they must be
	//lines that meet. Return true in that case.
	try {
	    PairMultiSet retSet = null;
	    
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
	    
	    return true;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_meets. Returning false by default.");
	}//catch
	return false;
    }//end method ll_meets
  

    /**
     * Returns <tt>true</tt>, if <tt>l</tt> and <tt>r</tt> meet.
     * This predicate holds, if <tt>l</tt> doesn't intersect <tt>r</tt>, but meets its border.
     *
     * @param l the Lines value
     * @param r the Regions value
     * @return <tt>true</tt>, if <tt>l,r</tt> meet
     */
    public static boolean lr_meets (Lines l, Regions r) {
	try {
	    return !lr_intersects(l,r) && ll_meets(l,r_contour(r));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.lr_meets. Returning false by default.");
	}//catch
	return false;
    }//end method lr_meets
   

    /**
     * Returns <tt>true</tt>, if <tt>l</tt> and <tt>r</tt> meet.
     * This predicate holds, if <tt>l</tt> doesn't intersect <tt>r</tt>, but meets its border.
     *
     * @param r the Regions value
     * @param l the Lines value
     * @return <tt>true</tt>, if <tt>l,r</tt> meet
     */
    public static boolean rl_meets (Regions r, Lines l) {
	try {
	    return lr_meets(l,r);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rl_meets. Returning false by default.");
	}//catch
	return false;
    }//end method rl_meets

 
    /**
     * Returns <tt>true</tt>, if <tt>r1,r2</tt> meet.
     * Two Regions values meet, if they don't have a common area, but the borders meet (in one point).
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt>, if <tt>r1,r2</tt> meet
     */
    public static boolean rr_meets (Regions r1, Regions r2) {
	try {
	    PairMultiSet retSet = null;
	
	    Method methodPINTERSECTS = TRI_CLASS.getMethod("pintersects",PARAMLIST_T);
	    retSet = SetOps.overlapJoin(r1.triset,r2.triset,methodPINTERSECTS,false,true,false,0);
	    
	    return (retSet.size()) == 0 && ll_meets(r_contour(r1),r_contour(r2));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_meets. Returning false by default.");
	}//catch
	return false;
    }//end method rr_meets


    /**
     * Returns <tt>true</tt>, if <tt>l1,l2</tt> have some border segments in common.
     * These common border doesn't have to consist of completely equal line segments. Overlapping line segments suffice
     * to make this predicate true.
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return <tt>true</tt>, if <tt>l1,l2</tt> have a common border
     */
    public static boolean ll_border_in_common (Lines l1, Lines l2) {
	try {
	    PairMultiSet retSet = null;
	    
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    retSet = SetOps.overlapJoin(l1.segset,l2.segset,methodOVERLAP,true,true,false,0);
	
	    if (retSet.isEmpty())
		return false;
	    else
		return true;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_border_in_common. Returning false by default.");
	}//catch
	return false;
    }//end method ll_border_in_common
    

    /**
     * Returns <tt>true</tt>, if <tt>l,r</tt> have a common border.
     * <tt>l</tt> has to overlap the border of <tt>r</tt> in order to make this predicate true.
     *
     * @param l the Lines value
     * @param r the Regions value
     * @return <tt>true</tt>, if <tt>l,r</tt> have a common border
     */
    public static boolean lr_border_in_common (Lines l, Regions r) {
	try {
	    PairMultiSet retSet = null;
	    
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    retSet = SetOps.overlapJoin(l.segset,SupportOps.contour(r.triset,true,false),methodOVERLAP,true,true,false,0);
	
	    if (retSet.isEmpty())
		return false;
	    else
		return true;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.lr_border_in_common. Returning false by default.");
	}//catch
	return false;
    }//end method lr_border_in_common


    /**
     * Returns <tt>true</tt>, if <tt>l,r</tt> have a common border.
     * <tt>l</tt> has to overlap the border of <tt>r</tt> in order to make this predicate true.
     *
     * @param r the Regions value
     * @param l the Lines value
     * @return <tt>true</tt>, if <tt>l,r</tt> have a common border
     */
    public static boolean rl_border_in_common (Regions r, Lines l) {
	try {
	    return lr_border_in_common(l,r);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rl_border_in_common. Returning false by default.");
	}//catch
	return false;
    }//end method rl_border_in_common

    
    /**
     * Returns <tt>true</tt>, if <tt>r1,r2</tt> have a common border.
     * Some part of the Regions value's border must overlap. Then, this predicate holds.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt>, if <tt>r1,r2</tt> have a common border
     */
    public static boolean rr_border_in_common (Regions r1, Regions r2) {
	try {
	    return ll_border_in_common(new Lines(SupportOps.contour(r1.triset,true,false)),new Lines(SupportOps.contour(r2.triset,true,false)));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_border_in_common. Returning false by default.");
	}//catch
	return false;
    }//end method rr_border_in_common
    

    /**
     * Returns <tt>true</tt>, if <tt>r1,r2</tt> don't have a common area but have a comon boundary.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regiond value
     * @return <tt>true</tt>, if <tt>r1,r2</tt> are adjacent
     */
    public static boolean rr_adjacent (Regions r1, Regions r2) {
	try {
	    PairMultiSet pms = null;
	    
	    Method methodPINTERSECTS = TRI_CLASS.getMethod("pintersects",PARAMLIST_T);
	    pms = SetOps.overlapJoin(r1.triset,r2.triset,methodPINTERSECTS,false,true,false,0);
	
	    boolean intersection = !(pms == null || pms.size() == 0);
	    
	    return !intersection && rr_border_in_common(r1,r2);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_adjacent. Returning false by default.");
	}//catch
	return false;
    }//end method rr_adjacent
    
	
    /**
     * Returns <tt>true</tt>, if <rr>r2</tt> lies completely inside of holes of <tt>r1</tt>.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>true</tt>, if <rr>r1</tt> encloses <tt>r2</tt>
     */
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
	try {
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
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_encloses. Returning false by default.");
	}//catch
	return false;
    }//end method rr_encloses 
    
    
    /**
     * Returns <tt>true</tt>, if <tt>p</tt> lies on the border of <tt>l</tt>.
     * No single point of <tt>p</tt> may lie <u>not</u> on <tt>l</tt>.
     *
     * @param p the Points value
     * @param l the Lines value
     * @return <tt>true</tt>, if <tt>p</tt> lies on <tt>l</tt>
     */
    public static boolean pl_on_border_of (Points p, Lines l) {
	try {
	    PointMultiSet retSet = null;
	    
	    Method methodLIESON = PS_OPS_CLASS.getMethod("liesOn",PARAMLIST_PS);
	    try {
		retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(p.pointset,l.segset,methodLIESON,true,false,true,1))));
	    } catch (EarlyExit exit) {
		return false;
	    }//catch
		
	    return pp_equal(new Points(retSet),p);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pl_on_border_of. Returning false by default.");
	}//catch
	return false;
    }//end method pl_on_border_of

   
    /**
     * Returns <tt>true</tt>, if <tt>p</tt> lies on the border of <tt>r</tt>.
     *
     * @param p the Points value
     * @param r the Regions value
     * @return <tt>true</tt>, if <tt>p</tt> lies on the border of <tt>r</tt>
     */
    public static boolean pr_on_border_of (Points p, Regions r) {
	try {
	    PointMultiSet retSet = null;
	    
	    Method methodLIESON = PS_OPS_CLASS.getMethod("liesOn",PARAMLIST_PS);
	    try {
		retSet = PointMultiSet.convert(SetOps.rdup(SetOps.proj1(SetOps.overlapJoin(p.pointset,SupportOps.contour(r.triset,true,false),methodLIESON,true,true,true,1))));
	    } catch (Exception EarlyExit) {
		return false; 
	    }//catch

	    return pp_equal(new Points(retSet),p);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pr_on_border_of. Returning false by default.");
	}//catch
	return false;
    }//end method pr_on_border_of
    

    /**
     * Returns the intersection of two Points values.
     * Since a Points value represents a set of points, the result simply is the intersection of both sets.
     *
     * @param p1 the first Points value
     * @param p2 the second Points value
     * @return the intersection of <tt>p1,p2</tt>
     */
    public static Points pp_intersection (Points p1, Points p2) {
	try {
	    PointMultiSet retSet = null;
	    
	    retSet = PointMultiSet.convert(SetOps.intersection(p1.pointset,p2.pointset));
	    return new Points(retSet);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pp_intersection. Returning empty Points value.");
	}//catch
	return new Points();
    }//end method pp_intersection

    
    /**
     * Returns a new Points value which consists of the intersection points of <tt>l1,l2</tt>.
     * If there are no intersection points, an empty set is returned.
     * 
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return the set of intersection points
     */
    public static Points ll_intersection (Lines l1, Lines l2) {
	try {
	    ElemMultiSet retSet = null;
	    
	    Method m1 = SEG_CLASS.getMethod("pintersects",PARAMLIST_E);
	    Method m2 = SEG_CLASS.getMethod("intersection",PARAMLIST_S);
	    retSet = SetOps.rdup(SetOps.map(SetOps.overlapJoin(l1.segset,l2.segset,m1,true,true,false,0),m2));
	
	    return new Points(PointMultiSet.convert(retSet));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_intersection. Returning empty Points value.");
	}//catch
	return new Points();
    }//end method ll_intersection

    
    /**
     * For two intersection Regions values, this method returns a new Regions value that represents the common area of both.
     * If the Regions don't intersect, an empty Regions value is returned.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return the area representing the intersection of both regions
     */
    public static Regions rr_intersection (Regions r1, Regions r2) {
	try {
	    Regions res = new Regions(SupportOps.intersection(r1.triset,r2.triset,true));
	    return res;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_intersection. Returning empty Regions value.");
	}//catch
	return new Regions();
    }//end method rr_intersection
    
    
    /**
     * Returns that part of <tt>l</tt> that is covered by <tt>r</tt>.
     * 
     * @param r the Regions value
     * @param l the Lines value
     * @return the covered part of <tt>l</tt>
     */
    public static Lines rl_intersection (Regions r, Lines l) {
	try {
	    SegMultiSet retSet = null;
	    
	    Method methodPINTERSECTS = ST_OPS_CLASS.getMethod("pintersects",PARAMLIST_ST);
	    Method methodINTERSECTION = ST_OPS_CLASS.getMethod("intersection",PARAMLIST_ST);
	    
	    retSet = SupportOps.minimal(SegMultiSet.convert(SetOps.map(SetOps.overlapJoin(l.segset,r.triset,methodPINTERSECTS,true,true,false,0),methodINTERSECTION)),true,true,false);
	    
	    return new Lines(retSet);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rl_intersection. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//return rl_intersection
    
    
    /**
     * Returns the union of <tt>p1,p2</tt>.
     *
     * @param p1 the first Points value
     * @param p2 the second Points value
     * @return the union of <tt>p1,p2</tt>
     */
    public static Points pp_plus (Points p1, Points p2) {
	try {
	    PointMultiSet retSet = null;
	    
	    retSet = PointMultiSet.convert(SetOps.union(p1.pointset,p2.pointset));
	    return new Points(retSet);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pp_plusturning empty Points value.");
	}//catch
	return new Points();
    }//end method pp_plus
    
    
    /**
     * Returns the union of <tt>l1,l2</tt>.
     * Overlapping line segments are merged to new line segments.
     * 
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return the union of <tt>l1,l2</tt>
     */
    public static Lines ll_plus (Lines l1, Lines l2) {
	//Explanations for ll_plus: This operation cannot be implemented similar to ll_minus.
	//The reason is that a segment l1 of L can overlap two segments m1,m2 of M. When
	//using the same mechanism, we would get two overlapping segments in the returned set
	//from Segment.plus. Therefore, we choose another algorithm:
	// - compute the union of l1,l2
	// - compute overlapGroup with predicate SSO.overlap
	// - for every such group, call Segment.plus using the group's first element as 
	//   base element and the rest as parameter
	// - return the union of the results of Segment.plus
	try {
	    SegMultiSet retSet = new SegMultiSet(SEGMENT_COMPARATOR);
	    
	    //compute union of l1,l2
	    SegMultiSet workSet = new SegMultiSet (SEGMENT_COMPARATOR);
	    workSet.addAll(l1.segset);
	    workSet.addAll(l2.segset);
	    
	    ElemMultiSetList emsl = null;
	    
	    Method methodPLUS = SEG_CLASS.getMethod("plus",PARAMLIST_EMS);
	    
	    //compute overlapGroup
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    emsl = SetOps.overlapGroup(workSet,methodOVERLAP,true);
	    
	    //for every group, compute Segment.plus
	    Iterator it = emsl.iterator();
	    ElemMultiSet actGroup;
	    while (it.hasNext()) {
		actGroup = (ElemMultiSet)it.next();
		retSet.addAll(((Segment)actGroup.first()).plus(actGroup));
	    }//while
	    
	    return new Lines(retSet);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_plus. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//end method ll_plus
    

    /**
     * Returns the union of <tt>r1,r2</tt>.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return the union of <tt>r1,r2</tt>
     */
    public static Regions rr_plus (Regions r1, Regions r2) {
	try {
	    Regions res;
	    
	    res = new Regions(SupportOps.plus(r1.triset,r2.triset,false,true));
	    return res;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_plus. Returning empty Regions value.");
	}//catch
	return new Regions();
    }//end method rr_plus
    
 
    /**
     * Returns <tt>p1</tt> minus <tt>p2</tt>.
     * Since Points values simply store sets of points, equal points are removed from <tt>p1</tt> here.
     *
     * @param p1 the first Points value
     * @param p2 the second Points value
     * @return <tt>p1 - p2</tt>
     */
    public static Points pp_minus (Points p1, Points p2) {
	try {
	    PointMultiSet retSet = null;
	    
	    retSet = PointMultiSet.convert(SetOps.difference(p1.pointset,p2.pointset));
	    return new Points(retSet);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pp_minus. Returning empty Points value.");
	}//catch
	return new Points();
    }//end method pp_minus

    
    /**
     * Subtracts <tt>l2</tt> from <tt>l1</tt>.
     * If <tt>l2</tt> has line segments that overlap line segments from <tt>l1</tt>, these parts are removed from <tt>l1</tt>.
     *
     * @param l1 the first set
     * @param l2 the second set
     * @return <tt>l1 - l2</tt>
     */
    public static Lines ll_minus (Lines l1, Lines l2) {
	try {
	    SegMultiSet retSet = null;
	    
	    Method methodMINUS = SEG_CLASS.getMethod("minus",PARAMLIST_EMS);
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    LeftJoinPairMultiSet ljpms = SetOps.overlapLeftOuterJoin(l1.segset,l2.segset,methodOVERLAP,true,true,false,0);
	    ljpms = SetOps.map(ljpms,null,methodMINUS);
	    retSet = SegMultiSet.convert(SetOps.collect(ljpms));
	    
	    return new Lines(retSet);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_minus. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//end method ll_minus


    /**
     * Subtracts <tt>r2</tt> from <tt>r1</tt>.
     * The common part of <tt>r1,r2</tt> is removed from <tt>r1</tt>.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return <tt>r1 - r2</tt>
     */
    public static Regions rr_minus (Regions r1, Regions r2) {
	try {
	    Regions result = new Regions(SupportOps.minus(r1.triset,r2.triset,true));
	    return result;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_minus. Returning empty Regions value.");
	}//catch
	return new Regions();
    }//end method rr_minus
    
    
    /**
     * Returns a new Lines value that stores the intersection of <tt>l1,l2</tt>
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return the intersection of <tt>l1,l2</tt>
     */
    public static Lines ll_common_border (Lines l1, Lines l2) {
	try {
	    PairMultiSet joinSet = null;
	    SegMultiSet retSet = null;
	    
	    Method methodOVERLAP = SS_OPS_CLASS.getMethod("overlap",PARAMLIST_SS);
	    joinSet = SetOps.overlapJoin(l1.segset,l2.segset,methodOVERLAP,true,true,false,0);
	    
	    if (joinSet.isEmpty()) return new Lines(retSet);
	    
	    Method methodTHEOVERLAP = SS_OPS_CLASS.getMethod("theOverlap",PARAMLIST_SS);
	    retSet = SupportOps.minimal(SegMultiSet.convert(SetOps.map(joinSet,methodTHEOVERLAP)),true,true,false);
	    
	    return new Lines(retSet); 
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_common_border. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//end method ll_common_border
    

    /**
     * Computes the intersection of <tt>l</tt> and the border of <tt>r</tt>.
     *
     * @param l the Lines value
     * @param r the Regions value
     * @return that part of <tt>l</tt> that overlaps the border of <tt>r</tt>
     */
    public static Lines lr_common_border (Lines l, Regions r) {
	try {
	    return ll_common_border(l,new Lines(SupportOps.contour(r.triset,true,false)));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.lr_common_border. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//end method lr_common_border


    /**
     * Computes the intersection of <tt>l</tt> and the border of <tt>r</tt>.
     *
     * @param r the Regions value
     * @param l the Lines value
     * @return that part of <tt>l</tt> that overlaps the border of <tt>r</tt>
     */
    public static Lines rl_common_border (Regions r, Lines l) {
	try {
	    return lr_common_border(l,r);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rl_common_border. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//end method rl_common_border
    

    /**
     * Returns a new Lines value that stores the line segments that form the common border of <tt>r1,r2</tt>.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return the common border of <tt>r1,r2</tt>
     */
    public static Lines rr_common_border (Regions r1, Regions r2) {
	try {
	    return ll_common_border(new Lines(SupportOps.contour(r1.triset,true,false)),new Lines(SupportOps.contour(r2.triset,true,false)));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_common_border. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//end method rr_common_border
    

    /**
     * Returns all vertices of <tt>l</tt>.
     * The vertex set of <tt>l</tt> consists of all startpoints and endpoints of the line segments. Duplicates are removed.
     * 
     * @param l the Lines value
     * @return the vertices of <tt>l</tt>
     */
    public static Points l_vertices (Lines l) {
	try {
	    PointMultiSet retSet = null;
	    
	    Method methodENDPOINTS = SEG_CLASS.getMethod("endpoints",null);
	    retSet = PointMultiSet.convert(SetOps.rdup(SetOps.map(l.segset,methodENDPOINTS)));
	    return new Points(retSet);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.l_vertices. Returning empty Points value.");
	}//catch
	return new Points();
    }//end method l_vertices

   
    /**
     * Returns all vertices of <tt>r</tt>.
     * The vertex set of <tt>r</tt> consists of all startpoints and enspoints of the border of <tt>r</tt>. Duplicates are removed.
     *
     * @param r the Regions value
     * @return the set of vertices
     */
    public static Points r_vertices (Regions r) {
	try {
	    return l_vertices(new Lines(SupportOps.contour(r.triset,true,true)));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.r_vertices. Returning empty Points value.");
	}//catch
	return new Points();
    }//end method r_vertices
    

    /**
     * From a given Lines value, this method computes its interior, i.e. a new Regions value.
     *
     * @param l the Lines value
     * @return the interior of <tt>l</tt>
     */
    public static Regions l_interior (Lines l) {
	try {
	    return new Regions(Polygons.computeMesh(l.segset,true));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.l_interior. Returning empty Regions value.");
	}//catch
	return new Regions();
    }//end method l_interior


    /**
     * Returns the contour of <tt>r</tt>.
     * The contour is not the same as the border. Instead, it consists of all outer cycles without holes.
     *
     * @param r the Regions value
     * @return the contour of <tt>r</tt>
     */
    public static Lines r_contour(Regions r) {
	//Holes are omitted.
	try {
	    if (r.triset.isEmpty()) return new Lines();
	    Polygons pol = new Polygons(r.triset);
	    CycleListList polCLL = null;
	    polCLL = pol.cyclesSegments2();
	
	    SegMultiSet sms = new SegMultiSet(SEGMENT_COMPARATOR);
	    Iterator it = polCLL.iterator();
	    while (it.hasNext()) {
		//get the first cycle of every list
		sms.addAll(SegMultiSet.convert(SupportOps.convert( (LinkedList)((LinkedList)it.next()).getFirst())));
	    }//while
	    
	    return new Lines(sms);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.r_contour. Returning empty Lines value.");
	}//catch
	return new Lines();
    }//end method r_contour

    
    /**
     * Returns the number of components of <tt>p</tt>.
     * The number of components is the number of (different) points.
     *
     * @param p the Points value
     * @return the number of components as int
     */
    public static int p_no_of_components (Points p) {
	try {
	    return SetOps.rdup(p.pointset).size();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.p_no_of_components. Returning 0.");
	}//catch
	return 0;
    }//end method p_no_of_components
   

    /**
     * Returns the number of components of <tt>l</tt>.
     * The number of components is the number of merged line segments.
     *
     * @param l the Lines value
     * @return the number of components as int
     */
    public static int l_no_of_components (Lines l) {
	try {
	    int retVal = 0;
	    
	    Method methodPOINTSINCOMMON = SS_OPS_CLASS.getMethod("pointsInCommon",PARAMLIST_SS);
	    retVal = (SetOps.overlapGroup(l.segset,methodPOINTSINCOMMON,true)).size();
	    
	    return retVal;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.l_no_of_components. Returning 0.");
	}//catch
	return 0;
    }//end method l_no_of_components

    
    /**
     * Returns the number of components of <tt>r</tt>.
     * The number of components of a Regions value is the number of polygons it stores.
     *
     * @param r the Regions value
     * @return the number of compononts as int
     */
    public static int r_no_of_components (Regions r) {
	try {
	    int retVal = 0;
	    
	    Method methodINTERSECTS = TRI_CLASS.getMethod("intersects",PARAMLIST_E);
	    retVal = (SetOps.overlapGroup(r.triset,methodINTERSECTS,true)).size();
	    
	    return retVal;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.r_no_of_components. Returning 0.");
	}//catch
	return 0;
    }//end method r_no_of_components


    /**
     * Returns the Eucledian distance between <tt>p1,p2</tt>.
     * This method returns the smallest distance between any two points of <tt>p1,p2</tt>. Returns
     * -1, if one of both Points values is empty.
     *
     * @param p1 the first Points values
     * @param p2 the second Ponits value
     * @return the distance as Rationsl
     */
    public static Rational pp_dist (Points p1, Points p2) {
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
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pp_dist. Returning 0.");
	}//catch
		
	return retVal;
    }//end method pp_dist
    

    /**
     * Returns the Eucledian distance between <tt>p,l</tt>.
     * This method returns the smallest distance between any pair point/line segment of <tt>p,l</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param p the Points value
     * @param l the Lines value
     * @return the distance as Rationsl
     */
    public static Rational pl_dist (Points p, Lines l) {
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
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pl_dist. Returning 0.");
	}//catch
		
	return retVal; 
    }//end mehtod pl_dist


    /**
     * Returns the Eucledian distance between <tt>p,r</tt>.
     * This method returns the smallest distance between any pair point/polygon of <tt>p,r</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param p the Points value
     * @param r the Regions value
     * @return the distance as Rational
     */    
    public static Rational pr_dist (Points p, Regions r) {
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
	    System.out.println("There was an error when trying to execute ROSEAlgebra.pr_dist. Returning 0.");
	}//catch
		
	return retVal; 
    }//end method pr_dist 

   
    /**
     * Returns the Eucledian distance between <tt>p,l</tt>.
     * This method returns the smallest distance between any pair point/line segment of <tt>p,l</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param l the Lines value
     * @param p the Points value
     * @return the distance as Rationsl
     */
    public static Rational lp_dist (Lines l, Points p) {
	return pl_dist(p,l);
    }//end method lp_dist
   
    
    /**
     * Returns the Eucledian distance between <tt>l1,l2</tt>.
     * This method returns the smallest distance between any two line segments of <tt>l1,l2</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param l1 the first Lines value
     * @param l2 the second Lines value
     * @return the distance as Rational
     */
    public static Rational ll_dist (Lines l1, Lines l2) {
	Rational retVal = RationalFactory.constRational(0);
	try {
	    if (ll_disjoint(l1,l2)) return retVal;
	    
	    Method methodDIST = SEG_CLASS.getMethod("dist",PARAMLIST_E);
	    ElemPair retPair = SetOps.min(l1.segset,l2.segset,methodDIST);
	    if (retVal != null)
		retVal = retPair.first.dist(retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.ll_dist. Returning 0.");
	}//catch
		
	return retVal;
    }//end method ll_dist
   

    /**
     * Returns the Eucledian distance between <tt>l,r</tt>.
     * This method returns the smallest distance between any pair line segment/polygon of <tt>l,r</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param l the Lines value
     * @param r the Regions value
     * @return the distance as Rational
     */
    public static Rational lr_dist (Lines l, Regions r) {
	Rational retVal = RationalFactory.constRational(0);
	try {
	    if (lr_intersects(l,r)) return retVal;
	    
	    Method methodDIST = ST_OPS_CLASS.getMethod("dist",PARAMLIST_ST);
	    ElemPair retPair = SetOps.min(l.segset,r.triset,methodDIST);
	    if (retVal != null)
		retVal = SegTri_Ops.dist((Segment)retPair.first,(Triangle)retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.lr_dist. Returning 0.");
	}//catch
		
	return retVal; 
    }//end method lr_dist 

    
    /**
     * Returns the Eucledian distance between <tt>p,r</tt>.
     * This method returns the smallest distance between any pair point/polygon of <tt>p,r</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param r the Regions value
     * @param p the Points value
     * @return the distance as Rational
     */    
    public static Rational rp_dist (Regions r, Points p) {
	return pr_dist(p,r);
    }//end method rp_dist
   

    /**
     * Returns the Eucledian distance between <tt>l,r</tt>.
     * This method returns the smallest distance between any pair line segment/polygon of <tt>l,r</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param r the Regions value
     * @param l the Lines value
     * @return the distance as Rational
     */
    public static Rational rl_dist (Regions r, Lines l) {
	return lr_dist(l,r);
    }//end method rl_dist

    
    /**
     * Returns the Eucledian distance between <tt>r1,r2</tt>.
     * This method returns the smallest distance between any two polygons of <tt>r1,r2</tt>.
     * Returns -1, if one of both values is empty.
     *
     * @param r1 the first Regions value
     * @param r2 the second Regions value
     * @return the distance as Rational
     */
    public static Rational rr_dist (Regions r1, Regions r2) {
	Rational retVal = RationalFactory.constRational(0);
	try {
	    if (rr_disjoint(r1,r2)) return retVal;
	    
	    Method methodDIST = TRI_CLASS.getMethod("dist",PARAMLIST_E);
	    ElemPair retPair = SetOps.min(r1.triset,r2.triset,methodDIST);
	    if (retVal != null)
		retVal = retPair.first.dist(retPair.second);
	    else
		retVal = RationalFactory.constRational(-1);
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.rr_dist. Returning 0.");
	}//catch
		
	return retVal;
    }//end method rr_dist


    /**
     * Returns the diameter of <tt>p</tt>.
     *
     * @param p the Points value
     * @return the diameter as Rational
     */    
    public static Rational p_diameter (Points p) {
	Rational retVal = RationalFactory.constRational(0);
	try {
	     Method m = POINT_CLASS.getMethod("dist",PARAMLIST_E);
	     ElemPair retPair = SetOps.max(p.pointset,p.pointset,m);
	     retVal = retPair.first.dist(retPair.second);
	 } catch (Exception e) {
	     e.printStackTrace();
	     System.out.println("There was an error when trying to execute ROSEAlgebra.p_diameter. Returning 0.");
	 }//catch
	 
	 return retVal;
     }//end method p_diameter
    
    
    /**
     * Returns the diameter of <tt>l</tt>.
     *
     * @param l the Lines value
     * @return the diameter as Rational
     */
     public static Rational l_diameter (Lines l) {
	return p_diameter(l_vertices(l));
     }//end method l_diameter

   
    /**
     * Returns the diameter of <tt>r</tt>.
     *
     * @param r the Regions value
     * @return the diameter as Rational
     */
    public static Rational r_diameter (Regions r) {
	return p_diameter(r_vertices(r));
    }//end method r_diameter
   

    /**
     * Returns the length of <tt>l</tt>.
     *
     * @param l the Lines value
     * @return the length as double
     */
    public static double l_length (Lines l) {
	try {
	    double retSum = 0;
	    
	    Method methodLENGTH = SEG_CLASS.getMethod("length",null);
	    retSum = SetOps.sum(l.segset,methodLENGTH);
	    return retSum;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.l_length. Returning 0.");
	}//catch
	
	return 0;
    }//end method l_length

   
    /**
     * Returns the area of <rr>r</tt>.
     * 
     * @param r the Regions value
     * @return the area as double
     */
    public static double r_area (Regions r) {
	try {
	    double retVal = 0;
	    
	    Method methodAREA = TRI_CLASS.getMethod("area",null);
	    retVal = SetOps.sum(r.triset,methodAREA);
	    return retVal;
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.r_area. Returning 0.");
	}//catch
	
	return 0;
    }//end method r_area


    /**
     * Returns the perimeter of <tt>r</tt>.
     * The perimeter of <tt>r</tt> is the length of its border.
     *
     * @param r the Regions value
     * @return the perimeter as double
     */   
    public static double r_perimeter (Regions r) {
	try {
	    return l_length(new Lines(SupportOps.contour(r.triset,true,false)));
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("There was an error when trying to execute ROSEAlgebra.r_perimeter. Returning 0.");
	}//catch
		
	return 0;
    }//end method r_perimeter

}//end class ROSEAlgebras
