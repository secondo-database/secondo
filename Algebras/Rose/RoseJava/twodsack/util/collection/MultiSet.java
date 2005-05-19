/*
 * MultiSet.java 2004-11-05
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;

import twodsack.util.collectiontype.*;
import java.util.Comparator;
import java.util.TreeSet;
import java.util.SortedSet;
import java.util.Iterator;
import java.io.Serializable;

/**
 * A <code>MultiSet</code> provides a tree structure to store elements. Moreover, in a
 * <code>MultiSet</code> duplicate elements can be stored (which cannot be stored in structures
 * like {@link java.util.TreeSet} directly. Nevertheless, <code>MultiSet</code> extends 
 * <code>TreeSet</code>. In a <code>MultiSet</code> only types {@link MultiSetEntry} can be 
 * stored. Such types implement the interface {@link ComparableMSE}, so a total order
 * is defined.<br>
 * Usually, a <code>MultiSet</code> is not used directly, but is extendes. An example for this is
 * {@link ElemMultiSet}.
 */
public class MultiSet implements Serializable {
    /*
     * fields
     */
    private TreeSet ts;
    private Comparator comparatorTS;
    public int size;
    

    /*
     * constructors
     */
    /**
     * Default constructor.
     * Constructs new instance and sets default values.
     */
    public MultiSet () {
	ts = new TreeSet();
	size = 0;
	comparatorTS = null;
    }

    /**
     * Standard constructor.
     * A {@link Comparator} must be specified for the construction of a <code>MultiSet</code>, since
     * the elements cannot be stored in order otherwise.
     *
     * @param comp the <code>Comparator</code>
     */
    public MultiSet (Comparator comp) {
	ts = new TreeSet(comp);
	this.comparatorTS = comp;
	size = 0;
    }
    
    /*
     * methods
     */

    /**
     * Adds an Object <code>o</code>.
     * First, the object <code>o</code> is wrapped in a {@link MultiSetEntry}.
     * Then, if the object is already present in the structure, the counter for
     * this object is increased. Otherwise, the wrapped object is inserted in the structure.
     *
     * @param o the object to be added
     */
    public void add(Object o) {
	MultiSetEntry compO = new MultiSetEntry(o,1);

	this.size++;
	
	if (!ts.add(compO)) {
	    SortedSet ss = ts.tailSet(compO);
	    ((MultiSetEntry)ss.first()).number++;
	}//else
	
    }//end method add
    
    /**
     * Adds an Object <code>o</code> <i>n</i> times.
     * First, the object <code>o</code> is wrapped in a {@link MultiSetEntry}.
     * Then, if the object is already present in the structure, the counter for 
     * this object is increased by <i>n</i>. Otherwise, the wrapped object is inserted in the structer with 
     * counter = <i>n</i>.
     *
     * @param o the object to be added
     * @number n the number of instances to be added
     */
    public void add(Object o, int n) {
	MultiSetEntry compO = new MultiSetEntry(o,n);
	this.size += n;
	if (!ts.add(compO)) {
	    SortedSet ss = ts.tailSet(compO);
	    ((MultiSetEntry)ss.first()).number += n;
	}//if
    }//end method add

    /**
     * Adds an object which is already wrapped in a {@link MultiSetEntry}.
     * This method is used with {@link addAll}.
     * 
     * @param mse the MultiSetEntry
     */
    private void add(MultiSetEntry mse) {
	this.size += mse.number;
	if (!ts.add(mse)) {
	    SortedSet ss = ts.tailSet(mse);
	    ((MultiSetEntry)ss.first()).number += mse.number;
	}//if
    }//end method add

    /**
     * Adds all objects stored in <code>m</code>.
     * 
     * @param m the <code>MultiSet</code> that holds the elements which have to be stored
     */
    public void addAll(MultiSet m) {
	Iterator it = m.iterator();
	MultiSetEntry actEntry;
	while (it.hasNext()) {
	    actEntry = (MultiSetEntry)it.next();
	    add(actEntry);
	}//while
    }//end method addAll
    

    /**
     * Adds all objects stored in <code>arr</code>.
     *
     * @param arr the array that holds the elements which have to be stored.
     */
    public void add(Object[] arr) {
	for (int i = 0; i < arr.length; i++) 
	    add(arr[i]);
    }//end method add


    /**
     * Deletes all elements in <code>this</code>
     * Sets the size of <code>this</code> to 0.
     */
    public void clear() {
	this.size = 0;
	ts.clear();
    }//end method clear


    /**
     * Produces a <i>clone</i> of <code>this</code>.
     * Note, that a clone is not a <i>copy</i>, i.e. the <code>TreeSet</code>, which stores
     * the elements is not copied, but is the same as in the original.
     *
     * @return the resulting <code>TreeSet</code> as <code>Object</code>
     */
    public Object clone() {
	System.out.println("MS.clone() called!");
	MultiSet retSet;
	if (!(comparatorTS == null)) {
	    retSet = new MultiSet(comparatorTS);
	    retSet.comparatorTS = this.comparatorTS;
	}//if
	else {
	    retSet = new MultiSet();
	    retSet.comparatorTS = null;
	}//else
	
	retSet.ts = (TreeSet)this.ts.clone();
	return retSet;
    }//end method clone


    /**
     * Returns the <code>Comparator</code> which is used to keep the structure in order.
     *
     * @return the comparator as <code>Comparator</code>.
     */
    public Comparator comparator() {
	return this.comparatorTS;
    }//end method comparator


    /**
     * Returns <code>true</code> if <code>o</code> is element of this structure.
     * <code>false</code> otherwise.
     * 
     * @param  o the object that has to be found in the structure
     * @return {<code>true</code>, <code>false</code>} depending on the existance of <code>o</code> in
     *         <code>this</code>
     */
    public boolean contains (Object o) {
	return ts.contains(new MultiSetEntry(o,1));
    }//end method contains


    /**
     * Returns the <i>first</i> object of the structure w.r.t. the comparator.
     * The returned object is <i>not</i> a <code>MultiSetEntry</code> but the value stored in it.
     *
     * @return the first object
     */
    public Object first () {
	return ((MultiSetEntry)ts.first()).value;
    }//end method first


    /**
     * Returns the <i>first</i> MultiSetEntry of the structure w.r.t. the comparator.
     * 
     * @return the first MultiSetEntry
     */
    public MultiSetEntry firstMSE() {
	return (MultiSetEntry)ts.first();
    }//end method firstMSE		

    /**
     * Returns <code>true</code> if the underlying {@link TreeSet} is empty.
     * <code>false</code> else.
     *
     * @return {<code>true</code>, <code>false</code>} depending on the size of <code>this</code>
     */
    public boolean isEmpty() {
	return ts.isEmpty();
    }//end method isEmpty


    /**
     * Returns an iterator for the elements in the structure.
     * Note: If the <code>remove</code> method of class <code>Iterator</code> is used,
     * the field variable <code>size</code> is not updated properly. Use {@link recomputeSize}
     * in that case.
     *
     * @return the iterator as {@link java.util.Iterator}
     */
    public Iterator iterator() {
	return ts.iterator();
    }//end method iterator


    /**
     * Returns the <i>last</i> object of the structure w.r.t. the comparator.
     *
     * @return the last object
     */
    public Object last() {
	return ((MultiSetEntry)ts.last()).value;
    }//end method last


    /**
     * For a multiple entry, remove <i>one</i> of them.
     * If the entry exists only once, remove it completely.
     * If the entry doesn't exist, do nothing.
     *
     * @param o the entry that shall be deleted
     */
    public void removeOneEntry(Object o) {MultiSetEntry compO = new MultiSetEntry(o,1);
	
	if (ts.contains(compO)) {
	    SortedSet ss = ts.tailSet(compO);
	    if (((MultiSetEntry)ss.first()).number == 1) ts.remove(compO);
	    else {
		((MultiSetEntry)ss.first()).number--;
	    }//else
	    this.size--;
	}//if
    }//end method remove
    

    /**
     * For a multiple entry, remove it <i>completely</i>.
     * If the entry doesn't exist, do nothing.
     *
     * @param o the entry that shall be deleted
     */
    public void removeAllOfThisKind(Object o) {
	
	MultiSetEntry compO = new MultiSetEntry(o,1);
	
	SortedSet ss = ts.tailSet(compO);
	int num = ((MultiSetEntry)ss.first()).number;
	this.size = this.size - num;
	ts.remove(compO);
    }//end method removeAllOfThisKind


    /**
     * For a multiple entry, remove it <i>completely</i>.
     * If the entry doesn't exist, do nothing. This version is much faster than the <code>removeAllOfThisKind(Object)</code>
     * version.
     *
     * @param o the entry that shall be deleted
     * @param num the number of entries that the object has
     */
    public void removeAllOfThisKind(Object o, int num) {
	MultiSetEntry compO = new MultiSetEntry(o,1);
	ts.remove(compO);
	this.size = this.size - num;
    }//end method removeAllOfThisKind

    /**
     * Returns the number of elements stored in the structure.
     * For a structure which holds the elements (1,1,1,2,2,3) the size would be 6.
     * Caution: This operator may return the wrong size if <code>remove</code> was applied on an Iterator of 
     * <code>this</code>. To be sure to get the right result, call {@link recomputeSize} before.
     *
     * @return the size as <code>int</code>
     */
    public int size() {
	return this.size;
    }//end method size


    /**
     * Stores all elements of the structure in an array.
     * This is an expensive method for a big tree structure. Don't use this intensively.
     *
     * @return an array of (sorted) objects
     */
    public Object[] toArray() {
	Object[] retArr = new Object[this.size];
	Iterator it = ts.iterator();
	int index = 0;
	while (it.hasNext()) {
	    MultiSetEntry actEntry = (MultiSetEntry)it.next();
	    int num = actEntry.number;
	    for (int i = 0; i < num; i++) {
		retArr[index] = actEntry.value;
		index++;
	    }//for
	}//while
	return retArr;
    }//end method toArray


    /**
     * Compute the size of the structure and store it.
     * This is just to be safe. Use this method if <code>Iterator.remove()</code> is used.
     *
     */
    public void recomputeSize() {
	//computes and sets the size of this
	Iterator it = ts.iterator();
	int newSize = 0;
	while (it.hasNext()) {
	    newSize = newSize + ((MultiSetEntry)it.next()).number; }
	this.size = newSize;
    }//end method recomputeSize


    /**
     * Returns the underlying {@link java.util.TreeSet}
     *
     * @return the <code>TreeSet</code>
     */
    public TreeSet treeSet() {
	return this.ts;
    }//end method treeSet


    /**
     * Sets the <code>TreeSet</code> of <code>this</code> to <code>ts</code>.
     * Afterwards, the size is re-computed.
     *
     * @param ts the new <code>TreeSet</code>
     */
    public void setTreeSet(TreeSet ts) {
	this.ts = ts;
	recomputeSize();
    }//end method setTreeSet

}//end class MultiSet
