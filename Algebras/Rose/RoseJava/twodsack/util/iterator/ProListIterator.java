package twodsack.util.iterator;

import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import java.util.*;

public class ProListIterator implements ProIterator {
    //members
    protected Entry lastReturned;
    protected Entry next;
    protected int nextIndex;
    protected int expectedModCount;
    protected ProLinkedList pll;
    
    //constructors
    public ProListIterator (ProLinkedList inPll, int index) {
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
    
    //methods
    public boolean hasNext() {
	return nextIndex != pll.size;
    }//end method hasNext
    
    
    public Object next() {
	checkForComodification();
	if (nextIndex == pll.size)
	    throw new NoSuchElementException();
	lastReturned = next;
	next = next.next;
	nextIndex++;
	return lastReturned.value;
    }//end method next
    

    public Entry nextEntry() {
	checkForComodification();
	if (nextIndex == pll.size)
	    throw new NoSuchElementException();
	lastReturned = next;
	next = next.next;
	nextIndex++;
	return lastReturned;
    }//end method nextEntry
    
    
    public boolean hasPrevious() {
	return nextIndex != 0;
    }//end method hasPrevious

    
    public Object previous() {
	if (nextIndex == 0)
	    throw new NoSuchElementException();
	lastReturned = next = next.prev;
	nextIndex--;
	checkForComodification();
	return lastReturned.value;
    }//end method previous

    
    public int nextIndex() {
	return nextIndex;
    }//end method nextIndex
    

    public int previousIndex() {
	return nextIndex-1;
    }//end method previousIndex
    
    
    public void remove() {
	/* OLD IMPLEMENTATION traverses list up to element that shall be deleted!
	   checkForComodification();
	   try {
	   ProLinkedList.this.remove(lastReturned);
	   }//try
	   catch (NoSuchElementException e) {
	   throw new IllegalStateException();
	   }//catch
	   if (next == lastReturned)
	   next = lastReturned.next;
	   else nextIndex--;
	   lastReturned = head;
	   expectedModCount++;
	*/
	/* NEW IMPLEMENTATION */
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
    

    public void set (Object o) {
	if (lastReturned == pll.head)
	    throw new IllegalStateException();
	checkForComodification();
	lastReturned.value = o;
    }//end method set
    
    public void addBefore (Object o) {
	checkForComodification();
	pll.addBeforeAct(o,lastReturned);
	expectedModCount++;
	nextIndex++;
    }//end method addBefore
    
    
    public void add (Object o) {
	checkForComodification();
	lastReturned = pll.head;
	pll.addBeforeAct(o, next);
	nextIndex++;
	expectedModCount++;
    }//end method add
    

    public final void checkForComodification() {
	if (pll.modCount != expectedModCount)
	    throw new ConcurrentModificationException();
    }//end method checkForComodification
    

    public void reset() {
	//sets iterator to the first entry of the list
	lastReturned = pll.head;
	expectedModCount = pll.modCount;
	next = pll.head.next;
	nextIndex = 0;
    }//end method reset

    
    public void setList(ProLinkedList inPll) {
	//sets the underlying ProLinkedList to a new list
	//by doing this, the iterator can be reused for an other list
	//After having set the list the iterator's next method returnes the first elements of inPll.
	this.pll = inPll;
	reset();
    }//end method setList

    
}//end class ProListIterator