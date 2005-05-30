/*
 * LeftJoinPairMultiSet.java 2004-11-09
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.set;

import twodsack.setelement.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;

import twodsack.util.collectiontype.*;
import java.util.Iterator;

/**
 *
 A <code>LeftJoinPairMultiSet</code> is a {@link MultiSet} that is used to store objects of type {@link twodsack.setelement.LeftJoinPair}.
 * Such a structure is usually a return type of methods from 
 * {@link twodsack.operation.setoperation.SetOps}, e.g. <code>leftOuterJoin()</code> and <code>overlapLeftOuterJoin()</code>.
 */
public class LeftJoinPairMultiSet extends MultiSet {
    /*
     * constructors
     */

    /**
     * The 'empty' constructor.
     * Use this constructor <i>only</i> for initialization.
     */
    public LeftJoinPairMultiSet() {
	super();
    }


    /**
     * Constructs a new instance of LeftJoinPairMultiSet using the appropriate comparator.
     * Use <i>this</i> constructor.
     *
     * @param comp the comparator that is responsible for the correct order
     */    
    public LeftJoinPairMultiSet (LeftJoinPairComparator comp) {
	super(comp);
    }

    /*
     * methods
     */

    /**
     * Prints the elements of <code>this</code> to standard output.
     */
    public void print(){
	if (this.isEmpty()) System.out.println("LeftJoinPairMultiSet is empty.\n");
	else {
	    Iterator it = this.iterator();
	    int count = 0;
	    while (it.hasNext()) {
		MultiSetEntry actEntry = (MultiSetEntry)it.next();
		for (int i = 0; i < actEntry.number; i++) {
		    LeftJoinPair actLJP = (LeftJoinPair)actEntry.value;
		    System.out.println("\nElement "+count+":");
		    actLJP.element.print();
		    if (actLJP.elemSet == null || actLJP.elemSet.isEmpty()) {
			System.out.println("elemSet is empty"); }
		    else {
			System.out.println("elemSet: ");
			Iterator lit = actLJP.elemSet.iterator();
			while (lit.hasNext()) {
			    MultiSetEntry actListEntry = (MultiSetEntry)lit.next();
			    for (int j = 0; j < actListEntry.number; j++) {
				((Element)actListEntry.value).print();
			    }//for j
			}//while lit
		    }//else
		    count++;
		}//for i
	    }//while it
	}//else
    }//end method print
    
}//end class LeftJoinPairList
