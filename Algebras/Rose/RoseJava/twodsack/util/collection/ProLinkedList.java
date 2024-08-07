/*
 * ProLinkedList.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.util.collectiontype.*;
import twodsack.util.iterator.*;
import java.util.*;


/**
 * The implementation of a ProLinkedList class was necessary to remove some drawbacks of Sun's {@link java.util.LinkedList}.
 * First, it provides the {@link #addAll(ProLinkedList)} method,
 * that concats two ProLinkedLists without copying any objects. This is possible, because the elements of such lists are connected twice: they
 * have one pointer to their successor and one pointer to their predecessor. Additionally, for a list one pointer points to the head of the list
 * and another pointer points to the last element. Then, a concatenation of two lists is done by simple redirecting some pointers. However,
 * you must be aware that the original lists cannot be used anymore.<p>
 * The second improvement lies in the reusable iterators. Now, if an iterator was constructed, it can be reused over and over using its 
 * <tt>reset()</tt> method. This saves a lot of time and memory.
 */
public class ProLinkedList implements Cloneable {
    /*
     * fields
     */
    /**
     * The size of the list.
     */
    public int size;

    /**
     * This entry points to the first list entry.
     */
    public Entry head; //points to the first Entry

    /**
     * This entry points to the last list entry.
     */
    public Entry last; //points to the last Entry

    /**
     * A counter for being able to check concurrent modifications.
     */
    public  int modCount;

    private Comparator comparator; //is used for method remove(object)

    /**
     * Points to the entry that was added at last.
     */
    public Entry lastAdded; //points to the Entry that was added the last time

    /*
     * constructors
     */
    /**
     * The empty constructor.
     */
    public ProLinkedList() {
	modCount = 0;
	size = 0;
	head = new Entry(null);
	last = new Entry(null);
	comparator = null;
    }


    /**
     * Constructs a new instance using a comparator.
     * The comparator is <u>only</u> used when calling the {@link #remove(Object)} method.
     *
     * @param c the comparator c
     */
    public ProLinkedList(Comparator c) {
	modCount = 0;
	size = 0;
	head = new Entry(null);
	last = new Entry(null);
	comparator = c;
    }


    /**
     * Constructs a new instance from the passed MultiSet.
     * The passed comparator is used for the construction of the new list.
     *
     * @param c the new comparator
     * @param ems the MultiSet
     */
    public ProLinkedList(MultiSet ems, Comparator c) {
	comparator = c;
	modCount = 0;
	size = 0;
	head = new Entry(null);
	last = new Entry(null);
	Iterator it = ems.iterator();
	Object actElem;
	while (it.hasNext()) {
	    actElem = ((MultiSetEntry)it.next()).value;
	    this.add(actElem);
	}//while it
    }
    

    /*
     * methods
     */
    /**
     * Returns the size of <i>this</i>.
     *
     * @return the size as int
     */
    public int size () {
	return this.size;
    }//end method size


    /**
     * Returns <tt>true</tt>, if <i>this</i> is empty.
     *
     * @return <tt>true</tt>, if empty; false otherwise
     */
    public boolean isEmpty() {
	if (head.next == null) return true;
	else return false;
    }//end method isEmpty


    /**
     * Adds an object to <i>this</i>.
     * The new element will be the last element of this.
     *
     * @param o the new element
     */
    public void add (Object o) {
	Entry newEntry = new Entry(o);
	lastAdded = newEntry;
	if (head.next == null) {
	    head.next = newEntry;
	    last.next = newEntry;
	}//if
	else {
	    last.next.next = newEntry;
	    newEntry.prev = last.next;
	    last.next = newEntry;
	}//else
	this.size++;
	this.modCount++;
    }//end method add


    /**
     * Adds an object to <i>this</i>.
     * The new element will be sorted correctly in the list using the comparator of <i>this</i>.
     * If no comparator is specified, an exception is thrown.
     *
     * @param o the new element
     * @return the position of the list where the new element was added; first position is 0
     * @throws ComparatorNotDefinedException if no comparator is defined
     */
    public int addSorted (Object o) {
	int pos = -1;
	if (this.comparator == null)
	    throw new ComparatorNotDefinedException("Comparator is needed for this operation. Use constructor ProLinkedList(Comparator).");
	Entry newEntry = new Entry(o);
	lastAdded = newEntry;
	if (head.next == null) {
	    head.next = newEntry;
	    last.next = newEntry;
	    pos = 0;
	}//if
	else if (comparator.compare(o,head.next.value) == -1) {
	    //if new entry is smaller than first entry
	    newEntry.next = head.next;
	    newEntry.prev = null;
	    head.next.prev = newEntry;
	    head.next = newEntry;
	    pos = 0;
	}//if
	//if new entry is greater than last entry
	else if (comparator.compare(o,last.next.value) == 1) {
	    newEntry.prev = last.next;
	    newEntry.next = null;
	    last.next.next = newEntry;
	    last.next = newEntry;
	    pos = this.size;
	}//if
	else {
	    //traverse list using comparator
	    Entry nextEntry = head.next;
	    pos = 0;
	    while (comparator.compare(o,nextEntry.value) >= 0) {
		nextEntry = nextEntry.next;
		pos++;
	    }//while
	    newEntry.next = nextEntry;
	    newEntry.prev = nextEntry.prev;
	    nextEntry.prev.next = newEntry;
	    nextEntry.prev = newEntry;
	}//else
	this.size++;
	this.modCount++;
	return pos;
    }//end method addSorted

 
    /**
     * Returns the first element of <i>this</i>.
     *
     * @return the first object
     */
    public Object getFirst() {
	return head.next.value;
    }//end method getFirst


    /**
     * Returns the last element of <i>this</i>.
     *
     * @return the last object
     */
    public Object getLast() {
	return last.next.value;
    }//end method getLast


    /**
     * Adds to <i>this</i> all elements of <tt>pll</tt>.
     * The resulting list is a concatenation of both lists.
     *
     * @param pll the second list
     */
    public void addAll (ProLinkedList pll) {
	if (this.size == 0) {
	    this.head.next = pll.head.next;
	    this.head.prev = pll.head.prev;
	    this.last.next = pll.last.next;
	    this.last.prev = pll.last.prev;
	    this.modCount = pll.modCount;
	    this.size = pll.size;
	}//if
	else {
	    if (pll.size > 0) {
		this.last.next.next = pll.head.next;
		pll.head.next.prev = this.last.next;
		this.last.next = pll.last.next;
		this.size = this.size+pll.size;
		this.modCount = this.modCount+pll.size;
	    }//if
	}//else
    }//end method addAll


    /**
     * Returns a new iterator for <i>this</i>.
     * The iterator starts at <tt>index</tt>.
     *  
     * @param index the start position for the iterator
     * @return the iterator for <i>this</i>
     */
    public ProListIterator listIterator (int index) {
	return new ProListIterator(this,index);
    }//end method listIterator


    /**
     * Adds a new entry directly before <tt>e</tt>.
     * 
     * @param o the object that shall be added
     * @param e the entry that shall have the position directly after o in the result list
     */
    public void addBeforeAct(Object o, Entry e) {
	Entry newEntry = new Entry(o);
	lastAdded = newEntry;
	modCount++;
	if (!(e == null)) {
	    newEntry.next = e;
	    newEntry.prev = e.prev;
	    e.prev = newEntry;
	    if (newEntry.prev != null) 
		newEntry.prev.next = newEntry;
	    else head.next = newEntry;
	} else {
	    if (size > 0) {
		newEntry.next = null;
		newEntry.prev = last.next;
		last.next.next = newEntry;
		last.next = newEntry;
	    } else {
		newEntry.next = null;
		newEntry.prev = null;
		head.next = newEntry;
		last.next = newEntry;
	    }//else
	}//else
	size++;
    }//end method addBefore
    

    /**
     * Removes an object from <i>this</i>.
     * Note: This method doesn't work, if the list was constructed without a comparator.
     *
     * @param o the object that shall be removed
     * @throws ComparatorNotDefinedException if the list was not constructed using a comparator
     */
    public void remove (Object o) throws ComparatorNotDefinedException {
	if (this.comparator == null)
	    throw new ComparatorNotDefinedException("Comparator is needed for this operation. Use constructor ProLinkedList(Comparator).");
	
	if (!(o == null)) {
	    //if (o.equals(head.next.value)) {
	    if (this.comparator.compare(o,head.next.value) == 0) {
		if (size == 1) {
		    head.next = null;
		    last.next = null;
		}//if
		else {
		    head.next.next.prev = null;
		    head.next = head.next.next;
		}//else
	    }//if
	    else if (this.comparator.compare(o,last.next.value) == 0) {
		last.next = last.next.prev;
		last.next.next = null;
	    }//else if
	    else {
		for (Entry e = head.next; e != null; e = e.next) {
		    //if (o.equals(e.value)) {
		    if (this.comparator.compare(o,e.value) == 0) {
			if (e.next != null) {
			    e.prev.next = e.next;
			    e.next.prev = e.prev;
			}//if
			else {
			    e.prev.next = null;
			    e.prev = null;
			}//else
		    }//if
		}//for
	    }//else
	    size--;
	    modCount++;
	}//if
    }//end method remove


    /**
     * Removes the object at position <tt>idx</tt> from the list.
     *
     * @param idx the object at that position shall be removed
     * @throws IndexOutOfBoundsException if <tt>idx</tt> is greater than <tt>this.size</tt> or less than 0
     */
    public void remove (int idx) {
	if (idx >= size || idx < 0)
	    throw new IndexOutOfBoundsException("Index: "+idx+", Size: "+size);
	else {
	    if (idx == 0) {
		if (head.next.next != null) {
		    head.next.next.prev = null;
		    head.next = head.next.next;
		}//if
		else {
		    head.next = null;
		    last.next = null;
		}//else
	    }//if
	    else {
		Entry actEntry = head.next;
		for (int count = 0; count < idx; count++)
		    actEntry = actEntry.next;
		if (actEntry.next != null) {
		    actEntry.prev.next = actEntry.next;
		    actEntry.next.prev = actEntry.prev;
		}//if
		else {
		    last.next = actEntry.prev;
		    actEntry.prev.next = null;
		    actEntry.prev = null;
		}//else
		
	    }//else
	    modCount++;
	    size--;
	}//else
    }//end method remove


    /**
     * Returns the objects which at position <tt>idx</tt>.
     * 
     * @param idx the index of the object
     * @throws IndexOutOfBoundsException if <tt>idx</tt> is greater than <tt>this.size</tt> or less than 0
     */
    public Object get (int idx) throws IndexOutOfBoundsException {
	Entry actEntry = head.next;
	if (idx > size || idx < 0) 
	    throw new IndexOutOfBoundsException("Index: "+idx+", Size: "+size);
	else {
	    for (int count = 0; count < idx; count++)
		actEntry = actEntry.next;
	}//else
	return actEntry.value;
    }//end method get


    /**
     * Adds an object at position <tt>idx</tt>.
     * 
     * @param idx the index where o shall be added
     * @param o the new object
     * @throws IndexOutOfBoundsException if <tt>idx</tt> is greater than <tt>this.size</tt> or less than 0
     */
    public void add (int idx, Object o) throws IndexOutOfBoundsException {
	if (idx > size || idx < 0) 
	    throw new IndexOutOfBoundsException("Index: "+idx+", Size: "+size);
	Entry actEntry = head.next;
	for (int count = 0; count < idx; count++)
	    actEntry = actEntry.next;
	Entry newEntry = new Entry(o);
	if (actEntry.prev == null) {
	    newEntry.next = head.next;
	    head.next.prev = newEntry;
	    head.next = newEntry;
	}//if
	else {
	    newEntry.next = actEntry;
	    newEntry.prev = actEntry.prev;
	    actEntry.prev.next = newEntry;
	    actEntry.prev = newEntry;
	}//else
	modCount++;
	size++;
	lastAdded = newEntry;
    }//end method add


    /**
     * Removes all objects from the list.
     */
    public void clear() {
	head.next = null;
	last.next = null;
	modCount++;
	size = 0;
    }//end method clear
	

    /**
     * Writes all object of this to an array.
     *
     * @return the objects of <i>this</i> stored in an array
     */
    public Object[] toArray() {
	Object[] result = new Object[size];
	Entry actEntry = head.next;
	ProListIterator it = this.listIterator(0);
	while (it.hasNext())
	    result[it.nextIndex()] = it.next();
	return result;
    }//end method toArray
	

    /**
     * Prints the objects of the list to standard output.
     */
    public void print() {
	if (this.isEmpty()) System.out.println("List is empty.");
	ProListIterator it = this.listIterator(0);
	int count = 0;
	while (it.hasNext()) {
	    System.out.println("["+count+"]: "+it.next());
	    count++;
	}//while
    }//end method print
    

    /**
     * Returns a clone of <i>this</i>.
     *
     * @return the clone as Object
     */
    public Object clone () {
	Object clone;
	try { clone = super.clone(); }
	catch(Exception e) {
	    throw new InternalError("ProLinkedList: cloning not supported.");
	}//catch
	return clone;
    }//end method clone

}//end method ProLinkedList
