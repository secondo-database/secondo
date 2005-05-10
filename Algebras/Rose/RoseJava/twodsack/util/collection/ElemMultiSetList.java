/*
 * ElemMultiSetList.java 2004-11-05
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;


import twodsack.set.*;
import twodsack.util.comparator.*;
import java.util.LinkedList;
import java.io.Serializable;


/**
 * A <code>ElemMultiSetList</code> is a list of <code>ElemMultiSets</code>. In short, this is
 * a list of sets, where every set is a {@link ElemMultiSet}.
 */

public class ElemMultiSetList extends LinkedList implements Serializable {
    /*
     * methods
     */ 

    /**
     * Makes a <i>real</i> copy of the structure.
     * This means, that every single element is copied and stored in the new copy.
     *
     * @return the copy
     */
    public ElemMultiSetList copy() {
	ElemMultiSetList copy = new ElemMultiSetList();
	for (int i = 0; i < this.size(); i++) {
	    ElemMultiSet ms = new ElemMultiSet(new ElemComparator());
	    ms = ((ElemMultiSet)this.get(i)).copy();
	    copy.set(i,ms.copy());
	}//for i
	return copy;
    }//end method copy


    /**
     * Prints all elements of <code>this</code> to standard output.
     */
    public void print() {
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("Element["+i+"]: ");
	    ((ElemMultiSet)this.get(i)).print();
	}//for i
    }//end method print

}//end class ElemMultiSetList
