/*
 * TriMultiSet.java 2005-05-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.set;

import twodsack.util.comparator.*;
import java.util.*;

/**
 * Stored in a TriMultiSet are elements of type {@link twodsack.setelement.datatype.basicdatatype.Triangle}.
 * Don't try to store any other objects in an instance of this class.
 * This will result in an exception thrown by the {@link twodsack.util.comparator.TriangleComparator}.
 * The TriMultiSet class extends {@link ElemMultiSet}.
 * Only few additional methods are implemented.
 */

public class TriMultiSet extends ElemMultiSet {
    
    /*
     * constructors
     */
    /**
     * Constructs a new instance of TriMultiSet with a comparator.
     *
     * @param tc the comparator which is responsible for the correct order
     */
    public TriMultiSet(TriangleComparator tc) {
	super(tc);
	//this.bboxDefined = false;	
    }

    
    /**
     * methods
     */
    /**
     * Prints the elements data to the standard output.
     */
    public void print() {
	if (this.isEmpty()) System.out.println("TriMultiSet is empty.\n");
	else super.print();
    }//end method print


    /**
     * Converts the passed {@link ElemMultiSet} to a <tt>TriMultiSet</tt>.<p>
     * Make sure, that the elements stored in <tt>ems</tt> are really of type {@link twodsack.setelement.datatype.basicdatatype.Triangle}.
     * Otherwise an exception will be thrown by the {@link TriangleComparator}.
     *
     * @param ems the set that shall be converted
     * @return the converted set
     */
    static public TriMultiSet convert(ElemMultiSet ems) {
	TriMultiSet retSet = new TriMultiSet(new TriangleComparator());
	retSet.setTreeSet(ems.treeSet());

	return retSet;
    }//end method convert
    
}//end class TriMultiSet
