/*
 * PairMultiSetList.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;

import twodsack.set.*;

import java.util.*;
import java.io.*;

/**
 * The PairMultiSetList class is an extension of a LinkedList that is used to store PairMultiSets. Instances of this class are needed
 * for storing connected components which are computed in the {@link twodsack.util.graph} class. 
 */
public class PairMultiSetList extends LinkedList implements Serializable {
    /**
     * Print the data of <i>this</i> to the standard output.
     */
    public void print() {
	if (this.isEmpty()) System.out.println("PairMultiSetList is empty.\n");
	else {
	    for (int i = 0; i < this.size(); i++) {
		System.out.println("element["+i+"]: ");
		((PairMultiSet)this.get(i)).print();
	    }//for i
	}//else
    }//end method print

}//end class PairListList
