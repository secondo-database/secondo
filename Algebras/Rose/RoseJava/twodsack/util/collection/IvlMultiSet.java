package twodsack.util.collection;

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.util.*;


public class IvlMultiSet extends MultiSet {
    
    /*
     * constructors
     */

    public IvlMultiSet (Comparator co) {
	super(co);
    }

    /*
     * methods
     */

    public static IvlMultiSet merge (IvlMultiSet set1, IvlMultiSet set2, boolean meet) {
	//returns merged sets
	//removes duplicates, i.e. keeps only one object of one kind

	if (set1.isEmpty()) return set2;
	if (set2.isEmpty()) return set1;

	IvlMultiSet retSet = new IvlMultiSet(new IvlMergeComparator(meet));
	retSet.addAll(set1);
	retSet.addAll(set2);

	//remove duplicates
	Iterator it = retSet.iterator();
	MultiSetEntry mse;
	while (it.hasNext()) {
	    mse = (MultiSetEntry)it.next();
	    mse.number = 1;
	}//while

	retSet.recomputeSize();

	return retSet;
    }//end method merge


    public static IvlMultiSet minus (IvlMultiSet set1, IvlMultiSet set2, boolean meet) {
	//returns set1 minus set2
	//The minus operation is performed with comparing
	//the borders of the intervals and, if equal, the referenced
	//object, too.

	if (set1.isEmpty() || set2.isEmpty()) return set1;
	IvlMultiSet retSet = new IvlMultiSet(new IvlComparator(meet));
	Iterator it1 = set1.iterator();
	Iterator it2 = set2.iterator();
	boolean next1 = true;
	boolean next2 = true;
	Interval ivl1 = null;
	Interval ivl2 = null;

	while ((!next1 || it1.hasNext()) &&
	       (!next2 || it2.hasNext())) {
	    if (next1) ivl1 = (Interval)((MultiSetEntry)it1.next()).value;
	    if (next2) ivl2 = (Interval)((MultiSetEntry)it2.next()).value;
	    next1 = false;
	    next2 = false;
	    
	    if (ivl1.left.less(ivl2.left)) {
		retSet.add(ivl1);
		next1 = true;
	    }//if
	    else if (ivl1.left.greater(ivl2.left))
		next2 = true;
	    else if (ivl1.left.equal(ivl2.left) && ivl1.right.equal(ivl2.right)) {
		if (ivl1.number == ivl2.number) next1 = true;
		else if (ivl1.number < ivl2.number) {
		    retSet.add(ivl1);
		    next1 = true;
		}//if
		else next2 = true;
	    }//if
	    else if (ivl1.left.equal(ivl2.left) && ivl1.right.less(ivl2.right)) {
		retSet.add(ivl1);
		next1 = true;
	    }//if
	    else if (ivl1.left.equal(ivl2.left) && ivl1.right.greater(ivl2.right))
		next2 = true;
	    else {
		System.out.println("IvlMultiSet.minus: uncaught case!");
		ivl1.print();
		ivl2.print();
		System.exit(0);
	    }//else
	}//while

	//save the element that is already in 'next' but not saved yet
	if (!next1) retSet.add(ivl1);

	while (it1.hasNext()) 
	    retSet.add((Interval)it1.next());

	retSet.recomputeSize();

	return retSet;
    }//end method minus


    public static IvlMultiSet intersect (IvlMultiSet set1, IvlMultiSet set2, boolean meet) {
	//returns the intersection of set1 and set2
	//intervals which have equal borders are checked for equality of referenced object (number)

	if (set1.isEmpty()) return set1;
	if (set2.isEmpty()) return set2;

	IvlMultiSet retSet = new IvlMultiSet(new IvlMergeComparator(meet));
	
	retSet.addAll(set1);
	retSet.addAll(set2);
	
	Iterator it = retSet.iterator();
	MultiSetEntry mse;
	while (it.hasNext()) {
	    mse = (MultiSetEntry)it.next();
	    if (mse.number != 2)
		it.remove();
	    else mse.number = 1;
	}//while

	retSet.recomputeSize();

	return retSet;
    }//end method intersect


    public static PairMultiSet overlappingIntervals (LinkedList[] intStore, boolean sameSet, int size, IvlMultiSet set1, IvlMultiSet set2, PairMultiSet retSet) {
	//returns a set containing only the overlapping pairs of intervals of set1/set2
	//information about intersections is stored in intStore, so if an intersection is found,
	//first check if it's already in intStore
	//if sameSet==true, is is additionally checked, whether ivl1.number == ivl2.number-size
	//if true, the pair isn't stored

	if (set1.isEmpty() || set2.isEmpty()) return retSet;
	Iterator it1 = set1.iterator();
	Iterator it2,it3,it4;
	Interval ivl1,ivl2;
	boolean alreadyAdded = false;
	int intVal;
	ElemPair ep;

	while (it1.hasNext()) {
	    ivl1 = (Interval)((MultiSetEntry)it1.next()).value;
	    it2 = set2.iterator();
	    while (it2.hasNext()) {
		ivl2 = (Interval)((MultiSetEntry)it2.next()).value;
		//skip elements with same number
		while ((ivl1.number == ivl2.number) && it2.hasNext())
		    ivl2 = (Interval)((MultiSetEntry)it2.next()).value;

		if ((ivl1.left.lessOrEqual(ivl2.left) && ivl1.right.greater(ivl2.left)) ||
		    (ivl1.left.less(ivl2.right) && ivl1.right.greaterOrEqual(ivl2.right)) ||
		    ivl1.left.equal(ivl2.left) || ivl1.right.equal(ivl2.right) ||
		    (ivl1.left.less(ivl2.left) && ivl1.right.greater(ivl2.right)) ||
		    (ivl1.left.greater(ivl2.left) && ivl1.right.less(ivl2.right)) ||
		    //new cases
		    ivl1.left.equal(ivl2.right) ||
		    ivl1.right.equal(ivl2.left)
		    ) {
		    if (!(sameSet && 
			  (ivl1.number == (ivl2.number-size)) ||
			  (ivl2.number == (ivl1.number-size)))) {
			//check in intStore
			alreadyAdded = false;
			it3 = intStore[ivl1.number].iterator();
			while (it3.hasNext()) {
			    intVal = ((Integer)it3.next()).intValue();
			    if (intVal == ivl2.number) {
				alreadyAdded = true;
				break;
			    }//if
			}//while

			it4 = intStore[ivl2.number].iterator();
			if (!alreadyAdded) {
			    while (it4.hasNext()) {
				intVal = ((Integer)it4.next()).intValue();
				if (intVal == ivl1.number) {
				    alreadyAdded = true;
				    break;
				}//if
			    }//while
			}//if
			if (!alreadyAdded) {
			    //the 'blue' element should be the first in the pair
			    if ((ivl1.mark == "blueleft") || (ivl1.mark == "blueright")) {
				ep = new ElemPair(ivl1.ref,ivl2.ref); }
			    else { ep = new ElemPair(ivl2.ref,ivl1.ref); }
			    retSet.add(ep);
			    intStore[ivl1.number].add(new Integer(ivl2.number));
			    intStore[ivl2.number].add(new Integer(ivl1.number));
			}//if
		    }//if
		}//if
	    }//while
	}//while

	return retSet;
    }//end method overlappingIntervals
	
		     
	
    
}//end class IvlMultiSet