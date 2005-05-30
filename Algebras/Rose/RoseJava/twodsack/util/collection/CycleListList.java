/*
 * CycleListList.java 2004-11-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.collection;

import java.util.LinkedList;

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

}//end class CycleListList
