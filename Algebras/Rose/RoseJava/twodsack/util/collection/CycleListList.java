/*
 * CycleListList.java 2004-11-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.collection;

import twodsack.setelement.datatype.basicdatatype.*;
import java.util.*;

/**
 * A <code>CylceListList</code> is a list of cycle lists, where a cycle itself is a list of segments.
 * Therefore, the class is called <code>CycleListList</code>. Since it is nothing more than a
 * simple list, it is directly derived from {@link java.util.LinkedList}. Instances of this class are
 * used as return types and parameter types for various methods.
 */
public class CycleListList extends LinkedList {
    /*
     * constructors
     */
    /**
     * Constructs an 'empty' CycleListList.
     */
    public CycleListList() {}

    /**
     * Prints the elements of <i>this</i> to standard output.
     */
    public void print() {
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("\nCycleList No."+i+":");
	    ((CycleList)(this.get(i))).print();
	}//for i
    }//end method print


    /**
     * Returns true, if both lists are equal.
     * Note: Works only for cycles build of Segment types.
     *
     * @param inlist the list to compare <tt>this</tt> with
     * @return <tt>true</tt> if both lists are equal.
     */
    public boolean equal (CycleListList inlist) {
	Iterator tit1 = this.iterator();
	Iterator lit1 = inlist.iterator();
	Iterator tit2,lit2,tit3,lit3;
	
	//first, compare sizes of the lists
	if (inlist.size() != this.size()) {
	    return false;
	}//if
	
	LinkedList actList1,actList2,actList11,actList22;
	while (tit1.hasNext()) {
	    actList1 = (LinkedList)tit1.next();
	    actList2 = (LinkedList)lit1.next();
	    if (actList1.size() != actList2.size()) {
		return false;
	    } else {
		tit2 = actList1.iterator();
		lit2 = actList2.iterator();
		while (tit2.hasNext()) {
		    actList11 = (LinkedList)tit2.next();
		    actList22 = (LinkedList)lit2.next();
		    if (actList11.size() != actList22.size()) {
			return false;
		    } else {
			tit3 = actList11.iterator();
			lit3 = actList22.iterator();
			if (!((Segment)tit3.next()).equal((Segment)lit3.next())) {
			    return false;
			}//if
		    }//else
		}//while tit2
	    }//else
	}//while tit1
	
	return true;
    }//end method equal
	

}//end class CycleListList
