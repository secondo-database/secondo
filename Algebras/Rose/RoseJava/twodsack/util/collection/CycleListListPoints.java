/*
 * CycleListListPoints.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.collection;

import java.util.*;


/**
 * An instance of the CycleListListPoints class is a representation of a Regions value. A Regions type is usually not part of the
 * 2DSACK package. However, to support the conversion from nested list <-> Java object, the implementation of this class was 
 * necessary.<p>
 * A CycleListListPoints itself is a LinkedList which has {@link CycleList}(s) as elements. Each of those CycleList(s) represents one face
 * of the Regions value. Stored inside of such a list are cycles. The first cycle is always the outer cycle of such a face. All other
 * cycles (if any) are hole cycles. A CycleList has LinkedList(s) as elements that have 
 * {@link twodsack.setelement.datatype.basicdatatype.Point} objects as elements, here.
 */
public class CycleListListPoints extends LinkedList {
    /*
     * constructors
     */
    /**
     * Constructs an 'empty' CycleListListPoints.
     */
    public CycleListListPoints(){}

    /*
     * methods
     */
    /**
     * This method is needed to support the Clone() function in RoseAlgebra.cpp.
     */
    public CycleListListPoints copy() {
	//returns a deep copy of <i>this</i>
	CycleListListPoints copy = new CycleListListPoints();
	Iterator it = this.listIterator(0);
	while (it.hasNext()) {
	    copy.add(((CycleList)it.next()).copy());
	}//while it

	return copy;
    }//end copy

}//end class CycleListListPoints
