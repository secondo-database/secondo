/*
 * IvlList.java 2005-05-11
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
 * An IvlList is a list to store intervals. These intervals must be of type {@link twodsack.util.collectiontype.Interval} to assure that the operations of this class work
 * correctly. The most important operations of this class are the
 * {@link #intersect(IvlList,IvlList)}/{@link #merge(IvlList,IvlList,boolean)}/{@link #overlappingIntervals(ProLinkedList[],boolean,int,IvlList,IvlList,PairMultiSet)} methods which are all used by
 * the method <tt>overlappingPairs()</tt> in {@link twodsack.operation.setoperation.SetOps}.<p>
 * This class extends {@link ProLinkedList} and not Sun's LinkedList. A ProLinkedList behaves better than a LinkedList when using iterators.
 */
public class IvlList extends ProLinkedList {
    /*
     * fields
     */
    //all these iterators are declared once and reused many times
    private static ProLinkedList PLL = new ProLinkedList();
    private static ProListIterator PLI_MERGE1 = PLL.listIterator(0);
    private static ProListIterator PLI_MERGE2 = PLL.listIterator(0);
    private static ProListIterator PLI_MINUS1 = PLL.listIterator(0);
    private static ProListIterator PLI_MINUS2 = PLL.listIterator(0);
    private static ProListIterator PLI_INT1 = PLL.listIterator(0);
    private static ProListIterator PLI_INT2 = PLL.listIterator(0);
    private static ProListIterator PLI_3 = PLL.listIterator(0);
    private static ProListIterator PLI_4 = PLL.listIterator(0);
    private static ProListIterator PLI_CL = PLL.listIterator(0);
    private static ProListIterator PLI_CONT1 = PLL.listIterator(0);
    private static ProListIterator PLI_CONT2 = PLL.listIterator(0);
    
    
    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public IvlList(){}

    /*
     * methods
     */
    public void print() {
	//prints out the IvlList's intervals
	if (this.isEmpty()) System.out.println("List is empty.");
	ProListIterator it = this.listIterator(0);
	while (it.hasNext()) {
	    System.out.println("["+it.nextIndex()+"]: ");
	    ((Interval)it.next()).print();
	}//while
    }//end method print
    

    /**
     * Merges two instances of IvlList.
     * Duplicates are removed, i.e. for two lists <tt>(a,b,c) x (b,c,d)</tt> the result is <tt>(a,b,c,d)</tt>.
     * The result list is constructed as a new instance
     * of IvlList, if <i>keep</i> == <tt>true</tt>. Otherwise, the result list is stored in <tt>list1</tt>.
     *
     * @param list2 the first list
     * @param list1 the second list
     * @param keep if <tt>false</tt>, result is stored in <tt>list1</tt>; otherwise, a new list is constructed for the result
     * @return the merged lists
     */
    public static IvlList merge(IvlList list2, IvlList list1, boolean keep) {
	if (!keep) {
	    if (list1.head.next == null) return list2;
	    if (list2.head.next == null) return list1;
	    
	    Entry pointerL1 = list1.head.next;
	    Entry pointerL2 = list2.head.next;
	    Interval ivl1,ivl2;
	    Entry help;
	    while (pointerL1 != null && pointerL2 != null) {
		ivl1 = (Interval)pointerL1.value;
		ivl2 = (Interval)pointerL2.value;
		if (ivl1.left.less(ivl2.left)) 
		    pointerL1 = pointerL1.next;
		
		else if (ivl1.left.greater(ivl2.left)) {
		    //insert ivl2 in list1
		    list1.size++;
		    help = pointerL2.copy();
		    pointerL2 = pointerL2.next;
		    help.next = pointerL1;
		    if (pointerL1.prev == null) {
			//is first element
			list1.head.next = help;
			help.prev = null;
		    }//if
		    else {
			pointerL1.prev.next = help;
			help.prev = pointerL1.prev;
		    }//else
		    pointerL1.prev = help;
		}//else if
		
		else {
		    boolean i1LequalI2L = ivl1.left.equal(ivl2.left);
		    if (i1LequalI2L && ivl1.right.equal(ivl2.right)) {
			if (ivl1.number == ivl2.number) {
			    pointerL1 = pointerL1.next;
			    pointerL2 = pointerL2.next;
			}//if
			
			else if (ivl1.number < ivl2.number)
			    pointerL1 = pointerL1.next;
			
			else {
			    //insert ivl2 in list1
			    list1.size++;
			    help = pointerL2.copy();
			    pointerL2 = pointerL2.next;
			    help.next = pointerL1;
			    if (pointerL1.prev == null) {
				//is first element
				list1.head.next = help;
				help.prev = null;
			    }//if
			    else {
				pointerL1.prev.next = help;
				help.prev = pointerL1.prev;
			    }//else
			    pointerL1.prev = help;
			}//else
		    }//if
		    
		    else if (i1LequalI2L && ivl1.right.less(ivl2.right))
			pointerL1 = pointerL1.next;
		    
		    else if (i1LequalI2L && ivl1.right.greater(ivl2.right)) {
			//insert ivl2 in list1
			list1.size++;
			help = pointerL2.copy();
			pointerL2 = pointerL2.next;
			help.next = pointerL1;
			if (pointerL1.prev == null) {
			    //is first element
			    list1.head.next = help;
			    help.prev = null;
			}//if
			else {
			    pointerL1.prev.next = help;
			    help.prev = pointerL1.prev;
			}//else
			pointerL1.prev = help;
		    }//else if
		    else {
			System.out.println("IvlList.merge: uncaught case...");
			ivl1.print();
			ivl2.print();
			System.exit(0);
		    }//else
		}//if
	    }//while
	    
	    //Now, there may be intervals left in list2. Store them in list1.
	    if (pointerL2 != null) {
		//count remaining elements in list2
		int count = 0;
		help = pointerL2;
		while (help != null) {
		    count++;
		    help = help.next;
		}//while
		list1.size = list1.size + count;
		list1.last.next.next = pointerL2;
		pointerL2.prev = list1.last.next;
		list1.last.next = list2.last.next;
	    }//if
	    
	    return list1;
	}//if !keep

	else { //if keep = true
	    
	    if (list1.isEmpty()) return list2;
	    if (list2.isEmpty()) return list1;
	    
	    IvlList retList = new IvlList();
	    int pos1 = 0;
	    int pos2 = 0;
	    int ll1 = list1.size();
	    int ll2 = list2.size();
	    PLI_MERGE1.setList(list1);
	    ProListIterator it1 = PLI_MERGE1;
	    PLI_MERGE2.setList(list2);
	    ProListIterator it2 = PLI_MERGE2;
	    boolean next1 = true;
	    boolean next2 = true;
	    Interval ivl1 = (Interval)list1.getFirst();//just for initialization
	    Interval ivl2 = (Interval)list2.getFirst();//
	    boolean i1LequalI2L;
	    	    
	    while ((!next1 || it1.hasNext()) && 
		   (!next2 || it2.hasNext())) {
		if (next1) ivl1 = (Interval)it1.next();
		if (next2) ivl2 = (Interval)it2.next();
		
		next1 = false;
		next2 = false;
		if (ivl1.left.less(ivl2.left)) {
		    retList.add(ivl1);
		    next1 = true;
		}//if
		else if (ivl1.left.greater(ivl2.left)) {
		    retList.add(ivl2);
		    next2 = true;
		}//if
		else {
		    i1LequalI2L = ivl1.left.equal(ivl2.left);
		    if (i1LequalI2L &&
			ivl1.right.equal(ivl2.right)) {
			if (ivl1.number == ivl2.number) {
			    retList.add(ivl1);
			    next1 = true;
			    next2 = true;
			}//if
			else if (ivl1.number < ivl2.number) {
			    retList.add(ivl1);
			    next1 = true;
			}//if
			else {
			    retList.add(ivl2);
			    next2 = true;
			}//if
		    }//if
		    else if (i1LequalI2L &&
			     ivl1.right.less(ivl2.right)) {
			retList.add(ivl1);
			next1 = true;
		    }//if
		    else if (i1LequalI2L &&
			     ivl1.right.greater(ivl2.right)) {
			retList.add(ivl2);
			next2 = true;
		    }//if
		    else {
			System.out.println("IvlList.merge: uncaught case...");
			ivl1.print();
			ivl2.print();
			System.exit(0);
		    }//else
		}//if
	    }//while
	    
	    //save the element that is already in 'next' but
	    //not saved yet:
	    if (!next1) { retList.add(ivl1); }
	    if (!next2) { retList.add(ivl2); }
	    
	    while (it1.hasNext()) {
		retList.add(((Interval)it1.next()));
	    }//while
	    while (it2.hasNext()) {
		retList.add(((Interval)it2.next()));
	    }//while
	    
	    return retList;
	}//else keep = true
    }//end method merge
    

    /**
     * Returns <tt>list1</tt> minus <tt>list2</tt>.
     * For two lists <tt>(a,b,c) x (b,c,d)</tt> the result of minus is <tt>(a)</tt>
     *
     * @param list1 the first list
     * @param list2 the second list
     * @return <tt>list1</tt> minus <tt>list2</tt>
     */
    public static IvlList minus(IvlList list1, IvlList list2) {
	if (list1.isEmpty() || list2.isEmpty()) return list1;
	IvlList retList = new IvlList();

	PLI_MINUS1.setList(list1);
	ProListIterator it1 = PLI_MINUS1;
	PLI_MINUS2.setList(list2);
	ProListIterator it2 = PLI_MINUS2;
	boolean next1 = true;
	boolean next2 = true;
	Interval ivl1 = (Interval)list1.getFirst();//just to set a value; is not needed
	Interval ivl2 = (Interval)list2.getFirst();//dito

	while ((!next1 || it1.hasNext()) &&
	       (!next2 || it2.hasNext())) {
	    if (next1) ivl1 = (Interval)it1.next();
	    if (next2) ivl2 = (Interval)it2.next();
	    next1 = false;
	    next2 = false;
	    
	    if (ivl1.left.less(ivl2.left)) {
		retList.add(ivl1);
		next1 = true;
	    }//if
	    else if (ivl1.left.greater(ivl2.left)) {
		next2 = true;
	    }//if
	    else if (ivl1.left.equal(ivl2.left) &&
		     ivl1.right.equal(ivl2.right)) {
		if (ivl1.number == ivl2.number) { next1 = true; }
		else if (ivl1.number < ivl2.number) {
		    retList.add(ivl1);
		    next1 = true;
		}//if
		else { next2 = true; }
	    }//if
	    else if (ivl1.left.equal(ivl2.left) &&
		     ivl1.right.less(ivl2.right)) {
		retList.add(ivl1);
		next1 = true;
	    }//if
	    else if (ivl1.left.equal(ivl2.left) &&
		     ivl1.right.greater(ivl2.right)) {
		next2 = true;
	    }//if
	    else {
		System.out.println("IvlList.minus: uncaught case!");
		ivl1.print();
		ivl2.print();
		System.exit(0);
	    }//else
	    
	}//while

	//save the element that is already in 'next' but
	//not saved yet:
	if (!next1) { retList.add(ivl1); }

	while (it1.hasNext()) {
	    retList.add(((Interval)it1.next()));
	}//if
	
	//System.out.println("\nretList:"); retList.print();

	return retList;
    }//end method minus


    /**
     * Returns the intersection of two IntervalList(s).
     * For two lists <tt>(a,b,c) x (b,c,d)</tt> the result of intersect is <tt>(b,c)</tt>.
     *
     * @param list1 the first list
     * @param list2 the second list
     * @return the intersection of <tt>list1</tt> and <tt>list2</tt>   
     */
    public static IvlList intersect(IvlList list1, IvlList list2) {
	if (list1.isEmpty()) return list1;
	if (list2.isEmpty()) return list2;
	IvlList retList = new IvlList();
	int pos1 = 0;
	int pos2 = 0;
	int ll1 = list1.size();
	int ll2 = list2.size();
	PLI_INT1.setList(list1);
	ProListIterator it1 = PLI_INT1;
	PLI_INT2.setList(list2);
	ProListIterator it2 = PLI_INT2;
	boolean next1 = true;
	boolean next2 = true;
	Interval ivl1 = (Interval)list1.getFirst();//just for initialization
	Interval ivl2 = (Interval)list2.getFirst();//dito
	
	while ((!next1 || it1.hasNext()) &&
	       (!next2 || it2.hasNext())) {
	    if (next1) ivl1 = (Interval)it1.next();
	    if (next2) ivl2 = (Interval)it2.next();
	    next1 = false;
	    next2 = false;
	    
	    if (ivl1.left.less(ivl2.left)) {
		next1 = true;
	    }//if
	    else if (ivl1.left.greater(ivl2.left)) {
		next2 = true;
	    }//if
	    else if (ivl1.left.equal(ivl2.left) &&
		     ivl1.right.equal(ivl2.right)) {
		if (ivl1.number == ivl2.number) {
		    retList.add(ivl1);
		    next1 = true;
		    next2 = true;
		}//if
		else if (ivl1.number < ivl2.number) { next1 = true; }
		else { next2 = true; }
	    }//if
	    else if (ivl1.left.equal(ivl2.left) &&
		     ivl1.right.less(ivl2.right)) {
		next1 = true;
	    }//if
	    else if (ivl1.left.equal(ivl2.left) &&
		     ivl1.right.greater(ivl2.right)) {
		next2 = true;
	    }//if
	    else {
		System.out.println("IvlList.intersect: uncaught case!");
		ivl1.print();
		ivl2.print();
		System.exit(0);
	    }//else
	}//while
	
	return retList;
    }//end method intersect
    

    /**
     * Returns a set of pairs containing the pairs of overlapping intervals of both lists.
     * In <i>intStore</i> lists of intervals are stored which were already found as overlapping intervals. In there, the number of the 
     * interval is stored. If two intervals are found, it is looked up, whether this pair was found before. If so, it is not stored in 
     * the result set.<p>
     * If <i>sameSet</i> == <tt>true</tt>, <tt>list1</tt> and <tt>list2</tt> have the same elements. So intervals with the same number
     * or the same referenced object are not reported.
     *
     * @param intStore an array of ProLinkedList(s); every list contains Integer values which are numbers of intervals
     * @param sameSet true, if <tt>list1</tt> and <tt>list2</tt> are referencing the same set
     * @param size the size of <tt>list1</tt>
     * @param list1 the first list of intervals
     * @param list2 the second list of intervals
     * @param retList new pairs are stored in this set; it is identical to the returned set
     */
    public static PairMultiSet overlappingIntervals(ProLinkedList[] intStore, boolean sameSet, int size, IvlList list1, IvlList list2, PairMultiSet retList) {
	if (list1.isEmpty() || list2.isEmpty()) return retList;

	IvlList inList1 = list1;
	IvlList inList2 = list2;
	
	ProListIterator it3,it4;
	Entry actList,othList;
	Interval ivl1,ivl2;
	boolean alreadyAdded = false;
	int intVal;
	ElemPair ep;
	Entry list1Pointer = list1.head.next; //point to the first entry that was not examined yet
	Entry list2Pointer = list2.head.next; //dito
	Entry actPointer,othPointer; //used as iterators
	int id;
	
	while (!(list1Pointer == null || list2Pointer == null)) {
	    //define actList as list with minimal interval
	    if (((Interval)list1Pointer.value).left.less(((Interval)list2Pointer.value).left)) {
		actList = list1Pointer;
		othList = list2Pointer;
		id = 1;
	    } else {
		actList = list2Pointer;
		othList = list1Pointer;
		id = 2;
	    }//if

	    //get minimal interval
	    ivl1 = (Interval)actList.value;

	    //compare right interval of ivl1 with intervals of otherlist and
	    //report a pair if they overlap
	    
	    othPointer = othList;
	    while (!(othPointer == null)) {
		ivl2 = (Interval)othPointer.value;
		othPointer = othPointer.next;
		
		//ivl2.left is greater than ivl1.right
		if (ivl2.left.greater(ivl1.right)) {
		    //there are no overlapping intervals, so break
		    break;
		} else {
		    //there must be an overlap, so report it, if not already stored in intStore
		    //If elements are from the same set, i.e. sameSet==true, exclude elements
		    //whith the same number, i.e. don't report an element which is overlapping itself.
		    if (!(sameSet && 
			  ((ivl1.number == (ivl2.number-size)) ||
			   (ivl2.number == (ivl1.number-size))))) {
			//check in intStore
			alreadyAdded = false;
			
			PLI_3.setList(intStore[ivl1.number]);
			it3 = PLI_3;
			while (it3.hasNext()) {
			    intVal = ((Integer)it3.next()).intValue();
			    if (intVal == ivl2.number) {
				alreadyAdded = true;
				break;
			    }//if
			}//while
			
			PLI_4.setList(intStore[ivl2.number]);
			it4 = PLI_4;
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
			    /* using the code below, intStore can be printed out
			       System.out.println("intStore: ");
			       for (int idd = 0; idd < intStore.length; idd++) {
			       System.out.print("["+idd+"]");
			       ProListIterator plit = intStore[idd].listIterator(0);
			       while (plit.hasNext()) System.out.print(" - "+plit.next());
			       System.out.println();
			       }
			       System.out.println();
			    */

			    //the 'blue' element should be the first in the pair
			    if ((ivl1.mark == "blueleft") || (ivl1.mark == "blueright")) {
				ep = new ElemPair(ivl1.ref,ivl2.ref);
				intStore[ivl1.number].add(new Integer(ivl2.number));
				intStore[ivl2.number].add(new Integer(ivl1.number));
				//additional entries for sameSet=true
				if (sameSet) {
				    intStore[ivl1.number+size].add(new Integer(ivl2.number-size));
				    intStore[ivl2.number-size].add(new Integer(ivl1.number+size));
				}//if sameSet
			    }//if
			    else {
				ep = new ElemPair(ivl2.ref,ivl1.ref);
				intStore[ivl2.number].add(new Integer(ivl1.number));
				intStore[ivl1.number].add(new Integer(ivl2.number));
				//additional entries for sameSet=true
				if (sameSet) {
				    intStore[ivl2.number+size].add(new Integer(ivl1.number-size));
				    intStore[ivl1.number-size].add(new Integer(ivl2.number+size));
				}//if sameSet
			    }//else

			    retList.add(ep);
			}//if
		    }//if !sameset 
		}//must be an overlap
	    }//while it2.hasNext
	    
	    //all overlaps were found, so delete actual element
	    //actList.remove(0);
	    if (id == 1) list1Pointer = list1Pointer.next;
	    else list2Pointer = list2Pointer.next;
	}//while no empty list
		
	return retList;
    }//end method overlappingIntervals
    

    /**
     * Sorts the intervals in <i>this</i> using quicksort.
     */
    protected void sort() {
	Object[] elArr = new Interval[this.size()];
	elArr = this.toArray();
	quickX(elArr,0,elArr.length-1);
	//copy back
	this.clear();
	for (int i = 0; i < elArr.length; i++) {
	    this.add(elArr[i]);
	}//for i
    }//end method sort
    

    /**
     * Supportive method for sort(). Is the recursive part of quicksort.
     *
     * @param elArr the array of elements
     * @param lo the low index of elArr
     * @param hi the high index of elArr
     */
    private static void quickX (Object[] elArr, int lo, int hi) {
	int i = lo;
	int j = hi;
	int k = -1;
	Interval kElem;
	Object[] findkRes;

	if (lo < hi) {
	    findkRes = findPivotX(elArr,i,j);
	    k = ((Integer)findkRes[0]).intValue();
	    kElem = (Interval)findkRes[1];
	    if (k != -1) {
		k = partitionX(elArr,i,j,kElem); //divide
		quickX(elArr,i,k-1); //conquer
		quickX(elArr,k,j); //conquer
	    }//if
	}//if
    }//end method quickX


    /**
     * Supportive method for quickX. Finds and returns the pivot element.
     * 
     * @param elArr the interval array
     * @param i low index of elArr
     * @param j hi index of elArr
     * @return an array with two elements: first element is the index of the pivot element, the second element is the Interval object<p>
     * if no such pivot element exists, 0 is returned as index
     */
    private static Object[] findPivotX (Object[] elArr, int i, int j) {
	Object[] retArr = new Object[2];
	byte res;
	int k = 0;
	k = i+1;
	while ((k <= j) && (((Interval)elArr[k]).equalX((Interval)elArr[k-1]))) {
	    k++; }//while
	if (k > j) {
	    retArr[0] = new Integer(-1);
	    retArr[1] = (Interval)elArr[0];
	    return retArr;
	}//if
	else {
	    res = ((Interval)elArr[k]).comp((Interval)elArr[k-1]);
	    if (res == 1) {
		retArr[0] = new Integer(k);
		retArr[1] = (Interval)elArr[k];
		return retArr;
	    }//if
	    else {
		retArr[0] = new Integer(k-1);
		retArr[1] = (Interval)elArr[k-1];
		return retArr;
	    }//else
	}//else
    }//end method findPivotX


    /**
     * Suppotive method for quickX. Restructures an array using a pivot element x.
     *
     * @param elArr the array of Intervals
     * @param i the low index
     * @param j the high index
     * @param x the pivot element
     */
    private static int partitionX (Object[] elArr, int i, int j, Interval x) {
	int l = i;
	int r = j;
	Object tmp;
	byte res;
	boolean found = false;
	while (true) {
	    do {
		found = false;
		res = ((Interval)elArr[l]).comp(x);
		if (res == -1) { l++; }
		else { found = true; }
	    } while (!found);
	    do {
		found = false;
		res = ((Interval)elArr[r]).comp(x);
		if (res != -1) { r--; }
		else { found = true; }
	    } while (!found);
	    if (l > r) { break; }
	    tmp = elArr[l];
	    elArr[l] = elArr[r];
	    elArr[r] = tmp;
	}//while
	return l;
    }//end method partitionX


    /**
     * Returns a <i>view</i> on <i>this</i>.
     * This means, that the head and last pointers of a new list are set to the <i>begin</i> and <i>end</i> positions including 
     * begin, but <u>not</u> end. These pointers point to the elements of the original IvlList, so changes on the elements
     * affect the original. However, the {@link #size} method works correctly for the new list.
     *
     * @param begin the index of the first element that shall be in the view
     * @param end the index of the first element that shall not be in the view; <tt>end >= begin</tt>
     */
    public IvlList subList (int begin, int end) {
	IvlList retList = new IvlList();

	//set head pointer
	if ((begin == 0) || begin < (size >> 1)) {
	    retList.head.next = this.head.next;
	    for (int ind = 0; ind < begin; ind++) {
		retList.head.next = retList.head.next.next;
	    }//for 
	}//if
	else {
	    retList.head.next = this.last.next;
	    for (int ind = this.size; ind > begin; ind--)
		retList.head.next = retList.head.next.prev;
	}//else
	
	//set last pointer
	if ((end == 0) || end < (size >> 1)) {
	    retList.last.next = this.head.next;
	    for (int ind = 0; ind < begin; ind++)
		retList.last.next = retList.last.next.next;
	}//if
	else {
	    if (end == size) {
		retList.last.next = this.last.next;
	    }//if
	    else {
		retList.last.next = this.last.next;
		for (int ind = this.size; ind > end; ind--)
		    retList.last.next = retList.last.next.prev;
	    }//else
	}//else

	retList.size = end-begin;
	return retList;
    }//end method subList

    
    /**
     * Returns true, if <i>this</i> is sorted correctly.
     *
     * @return true, if sorted correctly
     */
    public boolean checkList () {
	if (this.size() < 2) return true;
	PLI_CL.setList(this);
	ProListIterator it = PLI_CL;
	Interval actIvl = (Interval)it.next();
	it.next();
	Interval nextIvl;
	while (it.hasNext()) {
	    nextIvl = (Interval)it.next();
	    if (actIvl.compY(nextIvl) >= 0) {
		System.out.println("\n\nIvlList.checkList: list not sorted correctly.");
		this.print();
		System.out.println("\nactIvl: ");actIvl.print();
		System.out.println("nextIvl: ");nextIvl.print();
		System.exit(0);
	    }//if
	    actIvl = nextIvl;
	}//while it

	//check for size
	Entry pointer = this.head.next;
	int count = 0;
	while (pointer != null) {
	    count++;
	    pointer = pointer.next;
	}//while

	return true;
    }//end method checkList

}//end class IvlList
