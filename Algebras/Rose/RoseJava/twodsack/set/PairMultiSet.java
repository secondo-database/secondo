/*
 * PairMultiSet.java 2005-05-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.set;

import twodsack.setelement.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.util.*;
import java.io.*;


/**
 * A PairMultiSet is used to store {@link ElemPair}s. An ElemPair is a pair of objects of type {@link Element}. The PairMultiSet
 * class extends {@link MultiSet} and provides only few additional methods.
 */
public class PairMultiSet extends MultiSet implements Serializable {
  
    /*
     * constructors
     */

    /**
     * Constructs a new PairMultiSet using the passed comparator.
     *
     * @param epc the comparator which is responsible for the correct order 
     */
    public PairMultiSet (ElemPairComparator epc) {
	super(epc);
    }

  
    /**
     * Returns a copy of <i>this</i>.
     *
     * @return the copy
     */
    public PairMultiSet copy() {
	PairMultiSet retSet = new PairMultiSet(new ElemPairComparator());
	retSet.addAll(this);
	return (PairMultiSet)retSet;
    }//end method copy


    /**
     * Prints the data stored in <i>this</i> to standard output.
     */
    public void print() {
	if (this.isEmpty()) System.out.println("PairMultiSet is empty.\n");
	else {
	    Iterator it = this.iterator();
	    int count = 0;
	    while (it.hasNext()) {
		MultiSetEntry actEntry = (MultiSetEntry)it.next();
		for (int i = 0; i < actEntry.number; i++) {
		    System.out.println("["+count+"]: ");
		    ((ElemPair)actEntry.value).print();
		    count++;
		}//for i
	    }//while
	}//else
    }//end method print

}//end class PairList
