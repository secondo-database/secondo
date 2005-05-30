/*
 * PointMultiSet.java 2005-05-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.set;

import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;
import java.util.*;

/**
 * Stored in a PointMultiSet are elements of type {@link Point}. Don't try to store any other objects in an instance of this
 * class. This will result in an Exception thrown by the {@link PointComparator}. The PointMultiSet class extends {@link ElemMultiSet}.
 * Only few additional methods are implemented.<p>
 * Two of the more interesting methods are the {@link #convert(ElemMultiSet)}, which does the conversion <tt>ElemMultiSet -> PointMultiSet</tt> and the
 * {@link #zoom(Rational)} method, which allows to zoom all of the {@link Point} objects stored inside of a PointMultiSet instance.
 */
public class PointMultiSet extends ElemMultiSet {
    /**
     * constructors
     */    
    /**
     * Constructs a new PointMultiSet instance using the given comparator.
     *
     * @param pc the comparator which is responsible for the correct order
     */
    public PointMultiSet(PointComparator pc) {
	super(pc);
    }
    

    /**
     * Prints all of the objects elements to the standard output.
     */
    public void print () {
	if (this.isEmpty()) System.out.println("PointMultiSet is empty.\n");
	else { super.print(); }
    }//end method print
    

    /**
     * Converts an ElemMultiSet to a PointMultiSet.
     * Make sure, that the ElemMultiSet <i>really</i> is of type PointMultiSet.
     *
     * @param ems the 'in' set
     * @return the converted set
     */
    static public PointMultiSet convert(ElemMultiSet ems) {
	PointMultiSet retSet = new PointMultiSet(new PointComparator());
	retSet.setTreeSet(ems.treeSet());
	return retSet;
    }//end method convert


    /**
     * Changes the coordinates of all points of <i>this</i> by multiplying them with fact.
     * This is implemented by calling the <tt>Point.zoom()</tt> method.
     *
     * @param fact the number used to multiply with
     * @return the 'zoom'ed set
     */
    public void zoom (Rational fact) {
	Iterator it = this.iterator();
	while (it.hasNext()) {
	    MultiSetEntry actEntry = (MultiSetEntry)it.next();
	    ((Point)actEntry.value).zoom(fact);
	}//while
    }//end method zoom
    
}//end class PointMultiSet
