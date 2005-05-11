package twodsack.util.collection;

import java.util.*;

public class CycleListListPoints extends LinkedList {
    //this is a Linked List of CycleLists
    //Cycles are represented as points here

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
