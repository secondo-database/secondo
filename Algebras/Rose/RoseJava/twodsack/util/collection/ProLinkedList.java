package twodsack.util.collection;

import twodsack.util.collectiontype.*;
import twodsack.util.iterator.*;
import java.util.*;

public class ProLinkedList implements Cloneable {
    
    //members
    public static int countAccess = 0;
    public int size;
    public Entry head; //points to the first Entry
    public Entry last; //points to the last Entry
    public  int modCount;
    private Comparator comparator; //is used for method remove(object)
    public Entry lastAdded; //points to the Entry that was added the last time

    //constructors
    public ProLinkedList() {
	this.countAccess++;
	modCount = 0;
	size = 0;
	head = new Entry(null);
	last = new Entry(null);
	comparator = null;
    }

    public ProLinkedList(Comparator c) {
	this.countAccess++;
	modCount = 0;
	size = 0;
	head = new Entry(null);
	last = new Entry(null);
	comparator = c;
    }
    

    //methods
    public int size () {
	return this.size;
    }//end method size

    public boolean isEmpty() {
	if (head.next == null) return true;
	else return false;
    }//end method isEmpty

    public void add (Object o) {
	//adds a new entry to this list at last position
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


    public Object getFirst() {
	return head.next.value;
    }//end method getFirst

    public Object getLast() {
	return last.next.value;
    }//end method getLast

    public void addAll (ProLinkedList pll) {
	//System.out.println("\nentering PLL.addAll... (this.size: "+this.size()+", pll.size: "+pll.size()+")");
	//re-implementation: OLD CODE:
	/*
	  Iterator pit = pll.listIterator(0);
	  while (pit.hasNext()) 
	  this.add(pit.next());
	*/
	//NEW CODE: this method doesn't any longer
	//add every single entry but redirects two
	//pointers instead
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
		//System.out.println("this.last.next: "+this.last.next);
		//System.out.println("this.last.next.next: "+this.last.next.next);
		//System.out.println("pll.head.next: "+pll.head.next);
		this.last.next.next = pll.head.next;
		pll.head.next.prev = this.last.next;
		this.last.next = pll.last.next;
		this.size = this.size+pll.size;
		this.modCount = this.modCount+pll.size;
	    }//if
	}//else
	//System.out.println("leaving PLL.addAll");
    }//end method addAll


    public ProListIterator listIterator (int index) {
	return new ProListIterator(this,index);
    }//end method listIterator

    /*
    public static ProListIterator listIterator (ProLinkedList pll, ProListIterator pli, int index) {
	//reuses pli as iterator for THIS
	pli.lastReturned = pll.head;
	pli.expectedModCount = pll.modCount;
	
	if (index < 0 || index > pll.size)
	    throw new IndexOutOfBoundsException("Index: "+index+", Size: "+pll.size);
	
	if ((index == 0) || index < (pll.size >> 1)) {
	    pli.next = pll.head.next;
	    for (pli.nextIndex=0; pli.nextIndex < index; pli.nextIndex++)
		pli.next = pli.next.next;
	}
	else {
	    pli.next = pll.last.next;
	    for (pli.nextIndex = pll.size; pli.nextIndex > index; pli.nextIndex--)
		pli.next = pli.next.prev;
	}//else

	System.out.println("\n++++++++++");
	System.out.println("pli.next: "+pli.next);
	System.out.println("pli.prev: "+pli.prev);
	

	return pli;
    }//end method listIterator
    */

       
    public  void addBeforeAct(Object o, Entry e) {
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
    
    public void remove (Object o) {
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

    public Object get (int idx) {
	Entry actEntry = head.next;
	if (idx > size || idx < 0) 
	    throw new IndexOutOfBoundsException("Index: "+idx+", Size: "+size);
	else {
	    for (int count = 0; count < idx; count++)
		actEntry = actEntry.next;
	}//else
	return actEntry.value;
    }//end method get

    public void add (int idx, Object o) {
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

    public void clear() {
	head.next = null;
	last.next = null;
	modCount++;
	size = 0;
    }//end method clear
	
    public Object[] toArray() {
	//System.out.println("ProLinkedList.toArray() called.");
	Object[] result = new Object[size];
	Entry actEntry = head.next;
	ProListIterator it = this.listIterator(0);
	while (it.hasNext())
	    result[it.nextIndex()] = it.next();
	return result;
    }//end method toArray
	
    
    public void print() {
	if (this.isEmpty()) System.out.println("List is empty.");
	ProListIterator it = this.listIterator(0);
	int count = 0;
	while (it.hasNext()) {
	    System.out.println("["+count+"]: "+it.next());
	    count++;
	    //it.next().print();
	}//while
    }//end method print
    
    
    public Object clone () {
	Object clone;
	try { clone = super.clone(); }
	catch(Exception e) {
	    throw new InternalError("ProLinkedList: cloning not supported.");
	}//catch
	return clone;
    }//end method clone


    /*
    protected void checkSize() {
	//System.out.println("entering checkSize...");
	int realSize = 0;
	for (Entry actEntry = head.next; actEntry != null; actEntry = actEntry.next)
	    realSize++;
	int itSize = 0;
	ListIterator lit = this.listIterator(0);
	while (lit.hasNext()) {
	    itSize++;
	    lit.next();
	}//while
    }//checkSize
    */
    /*  
    public ProLinkedList subList (int begin, int end) {
	//returns a view on THIS including begin and NOT end
	ProLinkedList retList = new ProLinkedList();
	if ((begin == 0) || begin < (size >> 1)) {
	    retList.head = this.head.next;
	    for (int ind = 0; ind < begin; ind++)
		retList.head = retList.head.next.next;
	}//if
	else {
	    retList.head = this.last.next;
	    for (int ind = this.size; ind > begin; ind--)
		retList.head = retList.head.next.prev;
	}//else
	
	if ((end == 0) || end < (size >> 1)) {
	    retList.last = this.head.next;
	    for (int ind = 0; ind < begin; ind++)
		retList.last = retList.last.next.next;
	}//if
	else {
	    if (end == size)
		retList.last.next = this.last.next;
	    else {
		retList.last = this.last.next;
		for (int ind = this.size; ind > end; ind--)
		    retList.last = retList.head.next.prev;
	    }//else
	}//else

	retList.size = end-begin;
	
	return retList;
    }//end method subList
    */

}//end method ProLinkedList