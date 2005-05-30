/*
 * IvlMultiSet.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.util.*;

/**
 * In an instance of IvlMultiSet, objects of type {@link twodsack.util.collectiontype} are stored. As in every {@link MultiSet},
 * this class allows to store more than one of the same instances of {@link twodsack.util.collectiontype.Interval}. This is an
 * alternative implementation to {@link IvlList} and it also implements
 * the methods needed by the method <tt>overlappingPairs()</tt> in {@link twodsack.operation.setoperation.SetOps}.
 */
public class IvlMultiSet extends MultiSet {
    
    /*
     * constructors
     */
    /**
     * Constructs a new IvlMultiSet using the passed comparator.
     *
     * @param co the comparator
     */
    public IvlMultiSet (Comparator co) {
	super(co);
    }

    /*
     * methods
     */
    /**
     * Returns the merged sets.<p>
     * For two sets <tt>(a,a,b,c) x (b,c,d)</tt> the result of <i>merge</i> is <tt>(a,b,c,d)</tt>. Duplicates are removed.
     * The <i>meet</i> parameter is passed to the {@link twodsack.util.comparator.IvlMergeComparator} which is used for the
     * constructor for the resulting IvlMultiSet.
     *
     * @param set1 the first set
     * @param set2 the second set
     * @param meet is passed to the comparator that is used for the constructor of the result set
     */
    public static IvlMultiSet merge (IvlMultiSet set1, IvlMultiSet set2, boolean meet) {
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


    /**
     * Returns set1 minus set2.<p>
     * For two sets <tt>(a,a,b,c) x (b,c,d)</tt> the result of <i>minus</i> is <tt>(a,a)</tt>.
     * The <i>meet</i> parameter is passed to the constructor of the IvlMultiSet which needs to construct a
     * {@link twodsack.util.comparator.IvlComparator} with it.
     *
     * @param set1 the first set
     * @param set2 the second set
     * @param meet is used for the IvlComparator that is passed to the constructor of the resulting IvlMultiSet
     */
    public static IvlMultiSet minus (IvlMultiSet set1, IvlMultiSet set2, boolean meet) {
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


    /**
     * Returns the intersection of two sets of Intervals.<p>
     * For two sets <tt>(a,a,b,c) x (b,c,d)</tt> the result of <i>intersect</i> is <tt>(b,c)</tt>.
     *
     * @param set1 the first set
     * @param set2 the second set
     * @param meet is passed to the {@link twodsack.util.comparator.IvlMergeComparator} constructor which is needed for the constructor 
     *             of the resulting IvlMultiSet
     */
    public static IvlMultiSet intersect (IvlMultiSet set1, IvlMultiSet set2, boolean meet) {
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


    /**
     * Returns a set of pairs containing the pairs of overlapping intervals of both sets.
     * In <i>intStore</i> lists of intervals are stored which were already found as overlapping intervals. In there, the number of the
     * interval is stored as Integer object. If two intervals are found, it is looked up, whether this pair was found before. If so, it
     * is not stored in the result set.<p>
     * If <i>sameSet</i> = <tt>true</tt>, <tt>set1</tt> and <tt>set2</tt> have the same elements. So, intervals with the same number
     * or the same referenced object are not reported.
     *
     * @param intStore an array of {@link twodsack.util.collection.ProLinkedList}s; every list contains Integer values which are numbers of intervals
     * @param sameSet <tt>true</tt>, if <tt>set1</tt> and <tt>set2</tt> are referencing the same set
     * @param size the size of <tt>set1</tt>
     * @param set1 the first set
     * @param set2 the second set
     * @param retSet new pairs are stored in this set; it is idenctical to the returned set
     */
    public static PairMultiSet overlappingIntervals (LinkedList[] intStore, boolean sameSet, int size, IvlMultiSet set1, IvlMultiSet set2, PairMultiSet retSet) {
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
