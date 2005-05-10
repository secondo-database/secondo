/*
 * CycleList.java 2004-11-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.collection;

import twodsack.setelement.*;

import java.util.LinkedList;

/**
 * The <code>CycleList</code> class is simply a class with a different name for {@link LinkedList}.
 * It is used as return type or parameter type for several methods. This list should be used to
 * hold lists of segment cycles as shown below.
 * <p> ( ((a,b)(b,c)(c,d)) ((e,f)(f,g)(g,h)) ... )
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

}//end class CycleList