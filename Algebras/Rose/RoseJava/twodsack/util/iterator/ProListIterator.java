/*
 * ProListIterator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.iterator;

import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import java.util.*;


/**
 * The ProListIterator is a list iterator that implements the ProIterator interface. It is used for the ProLinkedList type as has the main advantage
 * over standard iterators that it can be reset. By calling the method {@link #reset} the iterator is set to the beginning of the list.
 * Another improvement is the {@link setList} method which can be used to reuse an iterator for another list.
 * 
 */
public class ProListIterator implements ProIterator {
    /*
     * fields
     */
    protected Entry lastReturned;
    protected Entry next;
    protected int nextIndex;
    protected int expectedModCount;
    protected ProLinkedList pll;


    /*
     * constructors
     */
    /**
     * Constructs a new iterator for inPll starting at index <i>index</i>.
     *
     * @param inPll the list for which the iterator is constructed
     * @param index the start index for the iterator
     * @throws IndexOutOfBoundsException if the index is too small or high
     */
    public ProListIterator (ProLinkedList inPll, int index) throws IndexOutOfBoundsException {
	this.pll = inPll;
	this.lastReturned = pll.head;
	this.expectedModCount = pll.modCount;
	
	if (index < 0 || index > pll.size)
	    throw new IndexOutOfBoundsException("Index: "+index+", Size: "+pll.size);
	
	if ((index == 0) || index < (pll.size >> 1)) {
	    next = pll.head.next;
	    for (nextIndex=0; nextIndex < index; nextIndex++)
		next = next.next;
	}
	else {
	    next = pll.last.next;
	    for (nextIndex = pll.size; nextIndex > index; nextIndex--)
		next = next.prev;
	}//else
    }    
    

    /*
     * methods
     */
    /**
     * Returns true, if the set has at least one more element.
     */
    public boolean hasNext() {
	return nextIndex != pll.size;
    }//end method hasNext
    

    /**
     * Returns the next object of the set.
     *
     * @return the object stored in the next entry
     * @throws NoSuchElementException if there is no next object
     */
    public Object next() throws NoSuchElementException{
	checkForComodification();
	if (nextIndex == pll.size)
	    throw new NoSuchElementException();
	lastReturned = next;
	next = next.next;
	nextIndex++;
	return lastReturned.value;
    }//end method next
    

    /**
     * Returns the next entry of the set.
     *
     * @return the next entry
     * @throws NoSuchElementException if there is no such entry
     */
    public Entry nextEntry() throws NoSuchElementException {
	checkForComodification();
	if (nextIndex == pll.size)
	    throw new NoSuchElementException();
	lastReturned = next;
	next = next.next;
	nextIndex++;
	return lastReturned;
    }//end method nextEntry
    
    
    /**
     * Returns true, if the set has at least one more element at the positin before the actual element.
     *
     * @return true, if such an element exists
     */
    public boolean hasPrevious() {
	return nextIndex != 0;
    }//end method hasPrevious

    
    /**
     * Returns the object on the position before the actual element.
     *
     * @return the previous object
     * @throws NoSuchElementException if there is no such element
     */
    public Object previous() throws NoSuchElementException {
	if (nextIndex == 0)
	    throw new NoSuchElementException();
	lastReturned = next = next.prev;
	nextIndex--;
	checkForComodification();
	return lastReturned.value;
    }//end method previous

    
    /**
     * Returns the next index.
     *
     * @return the next index as int
     */
    public int nextIndex() {
	return nextIndex;
    }//end method nextIndex
    

    /**
     * Returns the previous index.
     *
     * @return the previous index as int
     */
    public int previousIndex() {
	return nextIndex-1;
    }//end method previousIndex
    

    /**
     * Removes the last element returned from the set.
     */
    public void remove() {
	checkForComodification();
	if (pll.size == 1) {
	    pll.head.next = null;
	    pll.last.next = null;
	    pll.size = 0;
	    lastReturned = pll.head;
	    expectedModCount++;
	} else {
	    if (pll.head.next == lastReturned) {
		pll.head.next = lastReturned.next;
		pll.head.next.prev = null;
	    } else if (pll.last.next == lastReturned) {
		pll.last.next = lastReturned.prev;
		pll.last.next.next = null;
	    } else {
		lastReturned.prev.next = lastReturned.next;
		lastReturned.next.prev = lastReturned.prev;
	    }//else
	    pll.size--;
	}//if
    }//end method remove
    

    /**
     * Sets the value of the last reeturned object to o.
     *
     * @param o the object that shall be added
     * @throws IllegalStateException if there was no object returned yet
     */
    public void set (Object o) throws IllegalStateException {
	if (lastReturned == pll.head)
	    throw new IllegalStateException();
	checkForComodification();
	lastReturned.value = o;
    }//end method set
    

    /**
     * Adds the object o before the object that was returned before.
     *
     * @param o the object that shall be added
     */
    public void addBefore (Object o) {
	checkForComodification();
	pll.addBeforeAct(o,lastReturned);
	expectedModCount++;
	nextIndex++;
    }//end method addBefore
    

    /**
     * Adds the object o after the object that was returned before.
     *
     * @param o the object that shall be added
     */
    public void add (Object o) {
	checkForComodification();
	lastReturned = pll.head;
	pll.addBeforeAct(o, next);
	nextIndex++;
	expectedModCount++;
    }//end method add
    

    /**
     * Checks whether the iterator was used properly.
     * The proper use is calling hasNext() first and then one of the other operations. E.g. it is
     * not allowed to call next() two times without any other operation between.
     *
     * @throws ConcurrentModificationException if the iterator was not used properly
     */
    final void checkForComodification() {
	if (pll.modCount != expectedModCount)
	    throw new ConcurrentModificationException();
    }//end method checkForComodification
    

    /**
     * Resets the iterator to the beginning of the list.
     */
    public void reset() {
	lastReturned = pll.head;
	expectedModCount = pll.modCount;
	next = pll.head.next;
	nextIndex = 0;
    }//end method reset


    /**
     * Sets the iterator to another list.
     * By doing this, the iterator can be used for another list without constructing a new iterator
     * instance. After having set the list of the iterator's next() method returnes the first
     * element of inPll.
     *
     * @param inPll the new list for the iterator
     */
    public void setList(ProLinkedList inPll) {
	//sets the underlying ProLinkedList to a new list
	//by doing this, the iterator can be reused for an other list
	//After having set the list the iterator's next method returnes the first elements of inPll.
	this.pll = inPll;
	reset();
    }//end method setList

    
}//end class ProListIterator
