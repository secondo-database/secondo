/*
 * SegMultiSet.java 2005-05-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.set;

import twodsack.operation.setoperation.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*; 
import twodsack.util.graph.*;
import twodsack.util.number.*;
import java.util.*;
import java.lang.reflect.*;


/**
 * Stored in a SegMultiSet are elements of type {@link Segment}. Don't try to store any other objects in an instance of this class.
 * This will result in an exception thrown by the {@link SegmentComparator}. The SegMultiSet class extends {@link ElemMultiSet}.
 * Only few additional methods are implemented.<p>
 * Note, that segments are <i>aligned</i> before they are added to SegMultiSet. This means, that after that the startpoint's
 * coordinates of the segment are smaller than the endpoint's coordinates. This is computed using the align method of the 
 * Segment class.
 */
public class SegMultiSet extends ElemMultiSet {
    
    /**
     * constructors
     */
    /**
     * Constructs a new instance of SegMultiSet using the comparator.
     *
     * @param sc the comparator which is responsible for the correct order
     */
    public SegMultiSet(SegmentComparator sc) {
	super(sc);
    }
    
    /**
     * methods
     */
    /**
     * Adds a segment. Aligns it first.
     *
     * @param inseg the segment that shall be added
     */
    public void add(Segment inseg) {
	inseg.align();
	super.add(inseg);
    }//end method add


    /**
     * Returns <tt>true</tt>, if <i>this</i> already contains the given segment.
     * The segment is align before testing.
     *
     * @param inseg the segment that shall be tested
     * @return <tt>true</tt>, if the segment is already in <i>this</i>
     */
    public boolean contains(Segment inseg) {
	inseg.align();
	return super.contains(inseg);
    }//end method contains


    /**
     * Prints all elements of <i>this</i> to the standard output.
     */
    public void print() {
	if (this.isEmpty()) System.out.println("SegMultiSet is empty.\n");
	else { super.print(); }
    }//end method print
    

    /**
     * Converts an ElemMultiSet to a SegMultiSet.
     * Make sure, that the ElemMultiSet <i>really</i> is of type SegMultiSet.
     *
     * @param ems the 'in' set
     * @return the converted set
     */
    static public SegMultiSet convert(ElemMultiSet ems) {
	SegMultiSet retSet = new SegMultiSet(new SegmentComparator());
	retSet.setTreeSet(ems.treeSet());
	return retSet;
    }//end method convert
    

    /**
     * Changes the coordinates of all segments of <i>this</i> by multiplying them with fact.
     * This is implemented by calling the <tt>zoom()</tt> method of class {@link Segment}.
     *
     * @param fact the number used to multiply with
     * @return the 'zoom'ed set
     */
    public void zoom (Rational fact) {
	Iterator it = this.iterator();
	while (it.hasNext()) {
	    MultiSetEntry actEntry = (MultiSetEntry)it.next();
	    ((Segment)actEntry.value).zoom(fact);
	}//while
    }//end method zoom


    /**
     * Returns a PointMultiSet which includes all endpoints of segments of <i>this</i>.
     * Duplicates are removed.
     *
     * @return the set of endpoints
     */
    public PointMultiSet getAllPoints() {
	PointMultiSet allPoints = null;
	Class c = (new Segment()).getClass();
	try {
	    Method m = c.getMethod("endpoints",null);
	    allPoints = PointMultiSet.convert(SetOps.map(this,m));
	}//try
	catch (Exception e) {
	    System.out.println("Error in SegList.getAllPoints: Can't apply Segment.endpoints()");
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the ROSEAlgebra.");
	}//catch

	//remove duplicates
	allPoints = PointMultiSet.convert(SetOps.rdup(allPoints));

	return allPoints;
    }//end method getAllPoints


    /**    
     * Returns a PointMultiSet which includes all endpoints of segments of <i>this</i>.
     * Duplicates are <i>not</i> removed.
     *
     * @return the set of endpoints
     */
    public PointMultiSet getAllPointsWithDuplicates() {
	PointMultiSet allPoints = null;
	Class c = (new Segment()).getClass();
	try {
	     Method m = c.getMethod("endpoints",null);
	     allPoints = PointMultiSet.convert(SetOps.map(this,m));
	}//try
	catch (Exception e) {
	    System.out.println("Error in SegList.getAllPointsWithDuplicates: Can't apply Segment.endpoints()");
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the ROSEAlgebra.");
	}//catch
	
	return allPoints;
    }//end method getAllPointsWithDuplicates


    /**
     * Returns <tt>true</tt> if the segments stored in the SegMultiSet form a proper cycle.<p>
     * A proper cycle means, that the segments form the border of a polygon. No duplicate or intersecting segments may occur.
     *
     * @return <tt>true</tt> if the segments form such a proper cycle
     */
    public boolean checkForCycle() {
	Graph myGraph = new Graph(this);
	CycleList cl = myGraph.computeFaceCycles();
	return cl.checkCycles();
    }//end method checkForCycle
    
}//end class SegMultiSet
