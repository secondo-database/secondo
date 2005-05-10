package twodsack.util.collection;

//CAUTION: there is no order used here if left and right 
//borders are equal, but the refs not. It may happen that
//the operations won't work properly in that case.

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.util.collectiontype.*;
import twodsack.util.iterator.*;
import java.util.*;

public class IvlList extends ProLinkedList {
    //supportive class for SetOps
    //implements a double linked list
    
    //members
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
    
    //constructors
    
    //methods
    
    public void print() {
	//prints out the IvlList's intervals
	if (this.isEmpty()) System.out.println("List is empty.");
	ProListIterator it = this.listIterator(0);
	while (it.hasNext()) {
	    System.out.println("["+it.nextIndex()+"]: ");
	    ((Interval)it.next()).print();
	}//while
    }//end method print
    
    /* IS NOT NEEDED CURRENTLY
      public static IvlList copy(IvlList inlist) {
      IvlList copy = new IvlList();
      ListIterator it = inlist.listIterator(0);
      while (it.hasNext()) {
      copy.add(((Interval)it.next()).copy());
      }//while
      return copy;
      }//end method copy
    */

    /*
    public static IvlList insert(IvlList ivll, Interval ivl) throws WrongTypeException {
	//CAUTION:this method is not used
	//if you want to use this, first correct it so that is uses Iterators
	//inserts ivl in this.object, sorted accordant to left borders

	//if list ist empty, insert interval
	if (ivll.size() == 0) {
	    ivll.add(ivl);
	}//if
	//if list isn't empty search for the right position
	else {
	    int pos = -1;
	    for (int i = 0; i < ivll.size(); i++) {
		Interval iel1 = (Interval)ivll.get(i);
		if ((iel1.left.equal(ivl.left) && iel1.right.greater(ivl.right)) ||
		    iel1.left.greater(ivl.left)) {
		    pos = i;
		    break;
		}//if
	    }//for i
	    //if we passed through the whole list simply add interval
	    if (pos == -1) {
		ivll.add(ivl);
	    }//if
	    else {
		ivll.add(pos,ivl);
	    }//else
	}//else
	
	return ivll;
    }//end method insert
    */

    public static IvlList merge(IvlList list2, IvlList list1, boolean keep) {
	//returns merged lists
	//removes duplicates, i.e. keeps one object instead of two
	//if keep = true, both lists are kept unchanged
	//if keep = false, list1 is used to store elements
	


	/* NEWEST IMPLEMENTATION (1)*/
	
	if (!keep) {
	    if (list1.head.next == null) return list2;
	    if (list2.head.next == null) return list1;
	    
	    //System.out.println("\n------------------------------------------");
	    //System.out.println("list1: "); list1.print();
	    //System.out.println("\nlist2: "); list2.print();	
	    
	    /* Can NOT be used here, since only the second list may be changed! 
	     * If both lists could be changed, it would improve the run time.
	     *
	     * //choose the larger of the two lists as base list where to insert the 
	     * //elements of the other list
	     * if (list1.size() < list2.size()) {
	     * IvlList il = list1;
	     * list1 = list2;
	     * list2 = il;
	     * }//if
	     */
	    
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
	    
	    
	    //boolean correct = list1.checkList(false);
	    //System.out.println("\ncheckList: "+list1.checkList(false)+", size: "+list1.size());
	    //System.out.println("merged: "); list1.print();
	    //System.out.println("head.prev: "+list1.head.prev+", last.next: "+list1.last.next+", last.next.next: "+list1.last.next.next);
	    
	    return list1;
	}//if !keep
	
	/* old NEW IMPLEMENTATION  (2)
	 * returns the resulting list in list1
	 */
	/*
	if (list1.isEmpty()) return list2;
	if (list2.isEmpty()) return list1;
	
	//System.out.println("\n------------------------------------------");
	//System.out.println("list1: "); list1.print();
	//System.out.println("\nlist2: "); list2.print();

	ProListIterator it1 = (ProListIterator)list1.listIterator(0);
	ProListIterator it2 = (ProListIterator)list2.listIterator(0);
	boolean next1 = true; //if true, get next element of it1
	boolean next2 = true; //dito for it2
	Interval ivl1 = null;
	Interval ivl2 = null;
	boolean i1LequalI2L;
	
	//Though the condition for the while loop looks strange, it really makes sense:
	//It is not sufficient to let the loop run until one of the iterators has no
	//elements left, because there are always stored intervals in ivl1 and ivl2.
	//Therfore, the loop ends, iff an iterator has no more elements AND the
	//appropriate 'next' is set to true, i.e. the interval stored was used in the 
	//last iteration. In other words, the loop may be entered, when
	// - hasNext is true for an iterator
	// - hasNext is false, but 'next' is also false (i.e. interval still stored)
	while ((!next1 || it1.hasNext()) &&
	       (!next2 || it2.hasNext())) {
	
	    if (next1) ivl1 = (Interval)it1.next();
	    if (next2) ivl2 = (Interval)it2.next();
	    next1 = false;
	    next2 = false;
	    
	    if (ivl1.left.less(ivl2.left))
		next1 = true;

	    else if (ivl1.left.greater(ivl2.left)) {
		it1.addBefore(ivl2);
		next2 = true;
	    }//if
	    else {
		i1LequalI2L = ivl1.left.equal(ivl2.left);
		if (i1LequalI2L &&
		    ivl1.right.equal(ivl2.right)) {
		    if (ivl1.number == ivl2.number) {
			next1 = true;
			next2 = true;
		    }//if
		    
		    else if (ivl1.number < ivl2.number)
			next1 = true;
		    else {
			it1.addBefore(ivl2);
			next2 = true;
		    }//else
		}//if
		else if (i1LequalI2L &&
			 ivl1.right.less(ivl2.right))
		    next1 = true;
		else if (i1LequalI2L &&
			 ivl1.right.greater(ivl2.right)) {
		    it1.addBefore(ivl2);
		    next2 = true;
		}//if
		else {
		    System.out.println("IvlList.merge: uncaught case...");
		    ivl1.print();
		    ivl2.print();
		    System.exit(0);
		}//else
	    }//else
	}//while

	//save the elements that is already in 'next' for it2 but is not saved yet
	if (!next2)
	    if (!it1.hasNext()) list1.add(ivl2);
	    else it1.addBefore(ivl2);

	//store remaining elements of it2
	while (it2.hasNext()) 
	    list1.add((Interval)it2.next());
	
	//System.out.println("merged: "); list1.print();
	
	//boolean correct = list1.checkList(false);
	//System.out.println("list correct: "+correct);

	return list1;
	*/

	else { //if keep = true
	    
	    /* OLD IMPLEMENTATION (3) */
	    
	    if (list1.isEmpty()) return list2;
	    if (list2.isEmpty()) return list1;
	    
	    IvlList retList = new IvlList();
	    int pos1 = 0;
	    int pos2 = 0;
	    int ll1 = list1.size();
	    int ll2 = list2.size();
	    //ProListIterator it1 = list1.listIterator(0);
	    PLI_MERGE1.setList(list1);
	    ProListIterator it1 = PLI_MERGE1;
	    //ProListIterator it2 = list2.listIterator(0);
	    PLI_MERGE2.setList(list2);
	    ProListIterator it2 = PLI_MERGE2;
	    boolean next1 = true;
	    boolean next2 = true;
	    Interval ivl1 = (Interval)list1.getFirst();//just for initialization
	    Interval ivl2 = (Interval)list2.getFirst();//
	    boolean i1LequalI2L;
	    
	    //System.out.println("***************************");
	    //System.out.println("list1:"); list1.print();
	    //System.out.println("\nlist2:"); list2.print();
	    
	    while ((!next1 || it1.hasNext()) && 
		   (!next2 || it2.hasNext())) {
		if (next1) ivl1 = (Interval)it1.next();
		if (next2) ivl2 = (Interval)it2.next();
		//System.out.println("\nivl1:"); ivl1.print();
		//System.out.println("ivl2:"); ivl2.print();
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
	    
	    
	    //System.out.println("\n-------------------------------------\nretList: "+retList+", it1.hasNext: "+it1.hasNext()+", checkList: "+retList.checkList(false));

	    //System.out.println("\n-------------------------------------\nlist1:"); list1.print();
	    //System.out.println("\nretList:"); retList.print();

	    while (it1.hasNext()) {
		retList.add(((Interval)it1.next()));
	    }//while
	    while (it2.hasNext()) {
		retList.add(((Interval)it2.next()));
	    }//while
	    
	    //System.out.println("\nretList:"); retList.print();
	    
	    //boolean contains = retList.checkContains(list1,list2);

	    return retList;
	}//else keep = true
    }//end method merge
    

    public static IvlList minus(IvlList list1, IvlList list2) {
	//returns the list1 minus list2
	//the minus operation is performed with only comparing
	//the borders of the intervals and nothing more
	//caution: now the refs are also checked! this should solve some problems
	if (list1.isEmpty() || list2.isEmpty()) return list1;
	IvlList retList = new IvlList();
	//int pos1 = 0;
	//int pos2 = 0;
	//int ll1 = list1.size();
	//int ll2 = list2.size();
	PLI_MINUS1.setList(list1);
	ProListIterator it1 = PLI_MINUS1;//= list1.listIterator(0);
	PLI_MINUS2.setList(list2);
	ProListIterator it2 = PLI_MINUS2;// list2.listIterator(0);
	boolean next1 = true;
	boolean next2 = true;
	Interval ivl1 = (Interval)list1.getFirst();//just to set a value; is not needed
	Interval ivl2 = (Interval)list2.getFirst();//dito

	//System.out.println("*********************************");
	//System.out.println("list1:"); list1.print();
	//System.out.println("\nlist2:"); list2.print();
	
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
    

  public static IvlList intersect(IvlList list1, IvlList list2) {
    //returns the intersection of list1 and list2
    //caution: an interval which is represented in both is only checked
    //whether its left and right borders are equal
    //the element of list1 is then added to the returnlist
    //caution: now it is additionally checked with refs. this should do it.

    if (list1.isEmpty()) return list1;
    if (list2.isEmpty()) return list2;
    IvlList retList = new IvlList();
    int pos1 = 0;
    int pos2 = 0;
    int ll1 = list1.size();
    int ll2 = list2.size();
    PLI_INT1.setList(list1);
    ProListIterator it1 = PLI_INT1;//list1.listIterator(0);
    PLI_INT2.setList(list2);
    ProListIterator it2 = PLI_INT2;//list2.listIterator(0);
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
    

    public static PairMultiSet overlappingIntervals(ProLinkedList[] intStore, boolean sameSet, int size, IvlList list1, IvlList list2, PairMultiSet retList) {
	//returns a list containing only the overlapping pairs of intervals of l1 and l2
	//all intersections are stored in intStore, so if an intersection
	//is found first check if it's already in intStore
	//if sameSet=true, it is additionally checked whether ivl1.number == ivl2.number-size
	//if true, the pair isn't reported

	//System.out.println("entering overlappingIntervals... , sameSet:"+sameSet+", size:"+size);
	//PairMultiSet retList = new PairMultiSet(new ElemPairComparator());

	/* NEW IMPLEMENTATION */

	
	if (list1.isEmpty() || list2.isEmpty()) return retList;				     


	//System.out.println("\n----------------------------------------");
	//System.out.println("entering IvlL.overlappingIntervals... ("+list1.size()+", "+list2.size()+")");

	//System.out.println("sameSet: "+sameSet+", size: "+size);
	//System.out.println("list1: "); list1.print();
	//System.out.println("\nlist2: "); list2.print();

	if (list1.isEmpty() || list2.isEmpty()) return retList;

	IvlList inList1 = list1;
	IvlList inList2 = list2;
	
	//Iterator it1,it2,it3,it4;
	ProListIterator it3,it4;
	//IvlList actList,otherList;
	Entry actList,othList;
	Interval ivl1,ivl2;
	boolean alreadyAdded = false;
	int intVal;
	ElemPair ep;
	Entry list1Pointer = list1.head.next; //point to the first entry that was not examined yet
	Entry list2Pointer = list2.head.next; //dito
	Entry actPointer,othPointer; //used as iterators
	int id;
	
	//while (!inList1.isEmpty() && !inList2.isEmpty()) {
	while (!(list1Pointer == null || list2Pointer == null)) {
	    //define actList as list with minimal interval
	    if (((Interval)list1Pointer.value).left.less(((Interval)list2Pointer.value).left)) {
	    //if (((Interval)inList1.getFirst()).left.less(((Interval)inList2.getFirst()).left)) {
		//actList = inList1;
		actList = list1Pointer;
		//otherList = inList2;
		othList = list2Pointer;
		id = 1;
		//System.out.println(" [1] chose list1 as actList");
	    } else {
		//actList = inList2;
		actList = list2Pointer;
		//otherList = inList1;
		othList = list1Pointer;
		id = 2;
		//System.out.println(" [2] chose list2 as actList");
	    }//if

	    //get minimal interval
	    //ivl1 = (Interval)actList.getFirst();
	    ivl1 = (Interval)actList.value;

	    //System.out.println(" [2] ivl1: "); ivl1.print();
		
	    //compare right interval of ivl1 with intervals of otherlist and
	    //report a pair if they overlap
	    
	    //it2 = otherList.listIterator(0);
	    othPointer = othList;
	    //while (it2.hasNext()) {
	    while (!(othPointer == null)) {
		//ivl2 = (Interval)it2.next();
		ivl2 = (Interval)othPointer.value;
		othPointer = othPointer.next;
		
		//System.out.println(" [3] ivl2: "); ivl2.print();

		//ivl2.left is greater than ivl1.right
		if (ivl2.left.greater(ivl1.right)) {
		    //there are no overlapping intervals, so break
		    //System.out.println(" ---> no overlapping intervals, too small right interval");
		    break;
		} else {
		    //there must be an overlap, so report it, if not already stored in intStore
		    //If elements are from the same set, i.e. sameSet==true, exclude elements
		    //whith the same number, i.e. don't report an element which is overlapping itself.
		    if (!(sameSet && 
			  ((ivl1.number == (ivl2.number-size)) ||
			   (ivl2.number == (ivl1.number-size))))) {

			//System.out.println("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			//System.out.println(" ---> eventually found pair.");
			
			//check in intStore
			alreadyAdded = false;

			//it3 = intStore[ivl1.number].listIterator(PLI_3,0);
			PLI_3.setList(intStore[ivl1.number]);
			//it3 = ProLinkedList.listIterator(intStore[ivl1.number],PLI_3,0);
			it3 = PLI_3;
			while (it3.hasNext()) {
			    intVal = ((Integer)it3.next()).intValue();
			    if (intVal == ivl2.number) {
				alreadyAdded = true;
				break;
			    }//if
			}//while
			
			//it4 = intStore[ivl2.number].listIterator(PLI_4,0);
			PLI_4.setList(intStore[ivl2.number]);
			//it4 = ProLinkedList.listIterator(intStore[ivl2.number],PLI_4,0);
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
			    /*
			    System.out.println("\n ---> found pair: "); ivl1.print(); ivl2.print();
			    intStore[ivl1.number].add(new Integer(ivl2.number));
			    intStore[ivl2.number].add(new Integer(ivl1.number));
			    System.out.println(" added: intStore["+ivl1.number+"] add "+ivl2.number);
			    System.out.println(" added: intStore["+ivl2.number+"] add "+ivl1.number);
			    */

			}//if
		    }//if !sameset 
		}//must be an overlap
	    }//while it2.hasNext
	    
	    //all overlaps were found, so delete actual element
	    //actList.remove(0);
	    if (id == 1) list1Pointer = list1Pointer.next;
	    else list2Pointer = list2Pointer.next;
	    //System.out.println(" [last] move list pointer to next position");
	}//while no empty list
		
	//System.out.println("list1.size: "+list1.size()+", list2.size: "+list2.size()+", retList.size: "+retList.size());

	return retList;

	
	/* OLD IMPLEMENTATION */
	/*
	if (list1.isEmpty() || list2.isEmpty()) return retList;

	ListIterator it1 = list1.listIterator(0);
	ListIterator it2,it3,it4;
	Interval ivl1;
	Interval ivl2;
	boolean alreadyAdded = false;
	int intVal;
	ElemPair ep;
	
	while (it1.hasNext()) {
	    ivl1 = (Interval)it1.next();
	    it2 = list2.listIterator(0);
	    while (it2.hasNext()) {
		ivl2 = (Interval)it2.next();
		//skip elements with same number
		while ((ivl1.number == ivl2.number) && it2.hasNext()) {
		    ivl2 = (Interval)it2.next(); }
		//System.out.println("actual ivl1:"); ivl1.print();
		//System.out.println("actual ivl2:"); ivl2.print();
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
			it3 = intStore[ivl1.number].listIterator(0);
			while (it3.hasNext()) {
			    intVal = ((Integer)it3.next()).intValue();
			    if (intVal == ivl2.number) {
				alreadyAdded = true;
				break;
			    }//if
			}//while
			
			it4 = intStore[ivl2.number].listIterator(0);
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
			    retList.add(ep);
			    //System.out.println("\n----> added pair\n");
			    intStore[ivl1.number].add(new Integer(ivl2.number));
			    intStore[ivl2.number].add(new Integer(ivl1.number));
			}//if
		    }//if
		}//if
	    }//while
	}//while
	
	//System.out.println("leaving overlappingIntervals");
	return retList;
	*/
    }//end method overlappingIntervals
    
    
    protected void sort() {
	//sorts this using quicksort
	Object[] elArr = new Interval[this.size()];
	elArr = this.toArray();
	quickX(elArr,0,elArr.length-1);
	//copy back
	this.clear();
	for (int i = 0; i < elArr.length; i++) {
	    this.add(elArr[i]);
	}//for i
    }//end method sort
    

    private static void quickX (Object[] elArr, int lo, int hi) {
	//supportive method for sort
	//System.out.println("\nquickX: "+elArr+", lo: "+lo+", hi: "+hi);
	//for (int i = lo; i < hi+1; i++) { System.out.print("["+i+"] "); ((Interval)elArr[i]).print(); }
	int i = lo;
	int j = hi;
	int k = -1;
	Interval kElem;
	Object[] findkRes;

	if (lo < hi) {
	    findkRes = findPivotX(elArr,i,j);
	    k = ((Integer)findkRes[0]).intValue();
	    kElem = (Interval)findkRes[1];
	    //System.out.print("\npos: "+k+", pivot: "); kElem.print();
	    if (k != -1) {
		k = partitionX(elArr,i,j,kElem); //divide
		quickX(elArr,i,k-1); //conquer
		quickX(elArr,k,j); //conquer
	    }//if
	}//if
    }//end method quickX

    private static Object[] findPivotX (Object[] elArr, int i, int j) {
	//find index of not minimal element in array i,j
	//if exists
	//0 otherwise
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
    
    private static int partitionX (Object[] elArr, int i, int j, Interval x) {
	//supportive method for quicksortX
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

    public IvlList subList (int begin, int end) {
	//returns a view on THIS including begin and NOT end
	//a VIEW means, that only both pointers (head, last)
	//are set to the correct list positions on the 
	//original list. Additionally, the list size is set.
	
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

    
    public boolean checkList (boolean meet) {
	//checks whether the list is sorted correctly
	if (this.size() < 2) return true;
	//IvlComparator ic = new IvlComparator(meet);
	//ProListIterator it = this.listIterator(PLI_CL,0);
	//ProListIterator it = ProLinkedList.listIterator(this,PLI_CL,0);
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

	System.out.println("count: "+count);


	if (count != this.size()) {
	    System.out.println("IvlList.checkList: true size is "+count+". Found size "+this.size());
	    System.exit(0);
	}//if

	return true;
    }//end method checkList

    protected boolean checkContains (IvlList list1, IvlList list2) {
	//returns true, if this contains all elements of list1 and list2

	//CAUTION: Not sure, whether this method is still used anywher.
	//Additionally, it seems that the element of list2 are not checked
	//whether they are elements of 'this' or not.


	//ProListIterator it1 = list1.listIterator(PLI_CONT1,0);
	ProListIterator it1;// = ProLinkedList.listIterator(this,PLI_CONT1,0);
	PLI_CONT1.setList(list1);
	it1 = PLI_CONT1;

	Interval actIvl;
	ProListIterator it2;
	Interval thisIvl;
	boolean found;
	while (it1.hasNext()) {
	    actIvl = (Interval)it1.next();
	    //it2 = ProLinkedList.listIterator(this,PLI_CONT2,0);
	    PLI_CONT2.setList(this);
	    it2 = PLI_CONT2;
	    found = false;
	    while (it2.hasNext()) {
		thisIvl = (Interval)it2.next();
		
		if (thisIvl.left.equal(actIvl.left) && thisIvl.right.equal(actIvl.right) &&
		    thisIvl.number == actIvl.number) found = true;
	    }//while
	    if (!found) {
		System.out.println("\n++++++++++++++++++++++++++++++++++\nnot all elements are found in merged list: ");
		System.out.println("list1: "); list1.print();
		System.out.println("\nlist2: "); list2.print();
		System.out.println("\nretList: "); this.print();
		System.exit(0);
	    }//if
	}//while

	System.out.println("checkContains: true");
	return true;
    }//end method checkContains

}//end class IvlList
