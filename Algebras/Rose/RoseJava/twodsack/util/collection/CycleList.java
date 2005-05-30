/*
 * CycleList.java 2004-11-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.collection;

import twodsack.setelement.*;

import java.util.Iterator;
import java.util.LinkedList;

/**
 * The <code>CycleList</code> class is simply a class with a different name for {@link java.util.LinkedList}.
 * It is used as return type or parameter type for several methods. This list should be used to
 * hold lists of segment cycles as shown below.
 * <p> <tt>( ((a,b)(b,c)(c,d)) ((e,f)(f,g)(g,h)) ... )</tt>
 */
public class CycleList extends LinkedList {
    /**
     * Prints the elements of <code>this</code> to standard output.
     */
    public void print() {
	LinkedList actList;
	for (int i = 0; i < this.size(); i++) {
	    actList = (LinkedList)this.get(i);
	    System.out.println("--- Cycle No."+i+" ---");
	    for (int j = 0; j < actList.size(); j++) {
		((Element)(actList.get(j))).print();
	    }//for j
	}//for i
    }//end method print


    /**
     * Returns a 'deep' copy of <i>this</i>.
     * This means, that changes on the copy don't affect <i>this</i>.
     *
     * @return the copy
     */
    public CycleList copy() {
	CycleList copy = new CycleList();
	Iterator it = this.listIterator(0);
	Iterator it2;
	while (it.hasNext()) {
	    LinkedList cycleCopy = new LinkedList();
	    it2 = ((LinkedList)it.next()).listIterator(0);
	    while (it2.hasNext()) 
		cycleCopy.add(((Element)it2.next()).copy());
	    copy.add(cycleCopy);
	}//while it

	return copy;
    }//end method copy

}//end class CycleList
