/*
 * CycleList.java 2004-11-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.collection;

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;

import java.util.*;

/**
 * The <code>CycleList</code> class is simply a class with a different name for {@link java.util.LinkedList}.
 * It is used as return type or parameter type for several methods. This list should be used to
 * hold lists of segment cycles as shown below.
 * <p> <tt>( ((a,b)(b,c)(c,a)), ((e,f)(f,g)(g,e)), ... )</tt>
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


    /**
     * Returns <tt>true</tt>, if <i>this</i> is a list of proper cycles.<p>
     * Checks the connectivity of the cycles of <i>this</i> by traversing the segment lists and checking for equality of vertices.<p>
     * The cycles must have the following structure:<p>
     * <tt>( (a,b)(b,c)(c,d)(d,a) )</tt><p>
     * If they do, <tt>true</tt> is returned. <tt>false</tt> otherwise. If a cycle's size is smaller than 3, <tt>false</tt> is returned.
     * Note, that is is not checked, whether segments intersect, which is not allowed for proper cycles.
     * @return <tt>true</tt> if <i>this</i> is a proper cycle.
     */
    public boolean checkCycles() {
	System.out.println("\nCL.checkCycles...("+this.size()+")");
	ListIterator lit = this.listIterator(0);
	
	/*
	  while (lit.hasNext()) {
	  System.out.println("\nLIST "+lit.nextIndex());
	  LinkedList actList = (LinkedList)lit.next();
	  ListIterator lit2 = actList.listIterator();
	  while (lit2.hasNext())
	  System.out.println("["+lit2.nextIndex()+"]: "+(Segment)lit2.next());
	  }//while
	  System.out.println();
	*/
	
	
	Iterator outerIT = this.listIterator(0);
	LinkedList actCycle;
	int num = 0;
	System.out.print("-clLENGTH:"+this.size());
	while (outerIT.hasNext()) {
	    actCycle = (LinkedList)outerIT.next();
	    
	    if (actCycle.size() < 3) {
		System.out.print("-TOOLESSSEGMENTS");
		return false;
	    }//if
	    
	    Iterator it = actCycle.listIterator(0);
	    Segment firstSeg;
	    Segment nextSeg;
	    Segment initialSeg;
	    firstSeg = (Segment)it.next();
	    nextSeg = (Segment)it.next();
	    initialSeg = firstSeg;
	    
	    if (!firstSeg.getEndpoint().equal(nextSeg.getStartpoint())) {
		System.out.print("-FIRSTSEGDOESNTFIT");
		return false;
	    }//if	    

	    while (it.hasNext()) {
		num++;
		firstSeg = nextSeg;
		nextSeg = (Segment)it.next();
		if (!firstSeg.getEndpoint().equal(nextSeg.getStartpoint())) {
		    System.out.println("-SOMESEGDONTFIT(NUM:"+num+"/"+actCycle.size()+")");
		    int lowBound;
		    int highBound;
		    if (num > 2) lowBound = num-2; else lowBound = 0;
		    if (num+2 < actCycle.size()) highBound = num+2; else highBound = actCycle.size();
		    for (int n = lowBound; n < highBound; n++) {
			System.out.println("["+n+"]: "+(Segment)actCycle.get(n)); }
		    
		    return false;
		}//if
	    }//while it
	    
	    if (!nextSeg.getEndpoint().equal(initialSeg.getStartpoint())) {
		System.out.print("-CIRCLESEGDOESNTFIT");
		return false;
	    }//if
	}//while outerIT    
	
	return true;
    }//end method checkCycle
    
}//end class CycleList
