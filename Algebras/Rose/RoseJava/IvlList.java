//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//CAUTION: there is no order used here if left and right 
//borders are equal, but the refs not. It may happen that
//the operations won't work properly in that case.


import java.util.*;

class IvlList extends LinkedList{
  //supportive class for SegOps
  
  //members
  
  //constructors
  
  //methods
    public void print() {
	//prints out the IvlList's intervals
	Iterator it = this.listIterator(0);
	while (it.hasNext()) {
	    ((Interval)it.next()).print();
	}//while
    }//end method print

  public static IvlList copy(IvlList inlist) {
    IvlList copy = new IvlList();
    Iterator it = inlist.listIterator(0);
    while (it.hasNext()) {
	copy.add(((Interval)it.next()).copy());
    }//while
    return copy;
  }//end method copy

  
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
  

    public static IvlList merge(IvlList list1, IvlList list2) {
	//returns merged lists
	//removes duplicates

	if (list1.isEmpty()) return list2;
	if (list2.isEmpty()) return list1;

	IvlList retList = new IvlList();
	int pos1 = 0;
	int pos2 = 0;
	int ll1 = list1.size();
	int ll2 = list2.size();
	Iterator it1 = list1.listIterator(0);
	Iterator it2 = list2.listIterator(0);
	boolean next1 = true;
	boolean next2 = true;
	Interval ivl1 = (Interval)list1.getFirst();//just for initialization
	Interval ivl2 = (Interval)list2.getFirst();//
	
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
	    else if (ivl1.left.equal(ivl2.left) &&
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
	    else if (ivl1.left.equal(ivl2.left) &&
		     ivl1.right.less(ivl2.right)) {
		retList.add(ivl1);
		next1 = true;
	    }//if
	    else if (ivl1.left.equal(ivl2.left) &&
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

	//System.out.println("\nretList:"); retList.print();

	return retList;
    }//end method merge
    

    public static IvlList minus(IvlList list1, IvlList list2) {
	//returns the list1 minus list2
	//the minus operation is performed with only comparing
	//the borders of the intervals and nothing more
	//caution: now the refs are also checked! this should solve some problems
	if (list1.isEmpty() || list2.isEmpty()) return list1;
	IvlList retList = new IvlList();
	int pos1 = 0;
	int pos2 = 0;
	int ll1 = list1.size();
	int ll2 = list2.size();
	Iterator it1 = list1.listIterator(0);
	Iterator it2 = list2.listIterator(0);
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
    Iterator it1 = list1.listIterator(0);
    Iterator it2 = list2.listIterator(0);
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
    

    public static PairList overlappingIntervals(LinkedList[] intStore, boolean sameSet, int size, IvlList list1, IvlList list2) {
	//returns a list containing only the overlapping pairs of intervals of l1 and l2
	//all intersections are stored in intStore, so if an intersection
	//is found first check if it's already in intStore
	//if sameSet=true, it is additionally checked whether ivl1.number == ivl2.number-size
	//if true, the pair isn't reported

	//System.out.println("entering overlappingIntervals... , sameSet:"+sameSet+", size:"+size);
	
	PairList retList = new PairList();
	
	if (list1.isEmpty() || list2.isEmpty()) return retList;
	
	int pos1 = 0;
	int pos2 = 0;
	int ll1 = list1.size();
	int ll2 = list2.size();
	Iterator it1 = list1.listIterator(0);
	Iterator it2,it3,it4;
	//boolean next1 = true;
	//boolean next2 = true;
	Interval ivl1;
	Interval ivl2;
	boolean alreadyAdded = false;
	int intVal;
	ElemPair ep;
	
	if ((ll1 == 0) || (ll2 == 0)) { return retList; } 
	
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
			/*
			if (sameSet && !alreadyAdded && ((ivl2.number-2) >= 0)) {
			    //the pair could be already added vice versa
			    //if sameSet=true; check that
			    it3 = intStore[ivl2.number-size].listIterator(0);
			    while (it3.hasNext()) {
				intVal = ((Integer)it3.next()).intValue();
				if (intVal == ivl1.number+size) {
				    alreadyAdded = true;
				    break;
				}//if
			    }//while
			}//if
			*/
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
    
    
}//end class IvlList
