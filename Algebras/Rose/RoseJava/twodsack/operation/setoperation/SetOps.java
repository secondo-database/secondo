/*
 * SetOps.java 2005-05-02
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.setoperation;

import twodsack.io.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.graph.*;
import twodsack.util.iterator.*;
import twodsack.util.number.*;
import java.util.*;
import java.lang.reflect.*;
import java.io.*;

/**
 * The SetOps class is one of the central classes of the whole 2D-SACK package.
 * Collected in this class are all
 * of the genereric set operations the package offers. Sets in 2D-SACK are commonly {@link ElemMultiSet}, i.e. they
 * are one of {{@link PointMultiSet}, {@link SegMultiSet}, {@link TriMultiSet}}. Most of the operations provided here are made for this
 * type. Other types are {@link PairMultiSet}, {@link LeftJoinPairMultiSet} and, in one special case a LinkedList.<p>
 * A PairMultiSet is the result type of a join performed on two sets. Each element of such a set is an {@link ElemPair}
 * which holds two elements. A LeftJoinPairMultiSet is the result of an {@link #leftOuterJoin(ElemMultiSet,ElemMultiSet,Method)} and also has pairs
 * as elements. Such a pair looks like this: <code>(Element x ElemMultiSet)</code>. Finally, a LinkedList type
 * only appears in some sorting algorithm in this class.<p>
 * The operations in this class can be roughly divided in four groups:<ol>
 * <li> set operations for ElemMultiSet(s)
 * <li> set operations for PairMultiSet(s)
 * <li> set operations for LeftJoinPairMultiSet(s)
 * <li> supporting operations
 * </ol><p>
 * E.g. the group for set operations for PairMultiSet(s) holds several join operations to construct PairMultiSets
 * from pairs of ElemMultiSet(s). Then, some other operations can be used for further processing, like {@link #filter(PairMultiSet,Method,boolean)},
 * {@link #proj1(PairMultiSet)}, {@link #proj2(PairMultiSet)},  and four different versions of <tt>map(...)</tt>. The operations in the other
 * groups are quite similar.<p>
 * In this class, different implementations can be found for similar operations like <tt>join(...)</tt> and <tt>overlapJoin(...)</tt> or
 * <tt>group(...)</tt>
 * and <tt>overlapGroup</tt>. The <i>overlap</i> prefix indicates a method as using a special mechanism for speeding up
 * the operation. Whereas a simple <tt>join</tt> takes quite a long time, the <tt>overlapJoin</tt> uses a filter&refine technique
 * to reduce the number of possibly interesting pairs of elements before executing (possibly) expensive following
 * operations. The only constraint for these overlap versions of methods is, that the passed predicate and operation
 * for this operation may work only <i>inside of bounding boxes</i>. This means for predicates, that it only holds for 
 * two elements, if their bounding boxes overlap or are adjacent at least. For operations, the result of the operation must
 * lie inside of the bounding box of the two bounding boxes involved.
 */

public class SetOps {
    /*
     * fields
     */
    static final ElemComparator ELEM_COMPARATOR = new ElemComparator();
    static final LeftJoinPairComparator LEFTJOINPAIR_COMPARATOR = new LeftJoinPairComparator();
    static final ElemPairComparator ELEMPAIR_COMPARATOR = new ElemPairComparator();


    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public SetOps(){}


    /*
     * methods
     */
    
    /**
     * Separates the passed PairMultiSet and returns the elements in the two passed ElemMultiSet(s).
     * Both passed sets <tt>ems1,ems2<tt> are cleared before adding new elements.
     *
     * @param pms the set that is separated
     * @param ems1 all first elements are stored in this set
     * @param ems2 all second elements are stored in this set
     */
    static public void separateSets (PairMultiSet pms, ElemMultiSet ems1, ElemMultiSet ems2) {
	if (ems1 == null) ems1 = new ElemMultiSet(ELEM_COMPARATOR);
	if (ems2 == null) ems2 = new ElemMultiSet(ELEM_COMPARATOR);
	ems1.clear();
	ems2.clear();
	System.out.println("constructed.");
	ems1.print(); ems2.print();
	Iterator it = pms.iterator();
	ElemPair actPair;
	while (it.hasNext()) {
	    actPair = (ElemPair)((MultiSetEntry)it.next()).value;
	    ems1.add(actPair.first);
	    ems2.add(actPair.second);
	}//while it
	System.out.println("ems1("+ems1.size()+", ems2("+ems2.size()+")");
    }//end method separateSets



    /**
     * Returns a set of elements which is 'reduced' using the passed <tt>predicate</tt> and <tt>method</tt>.<p>
     * This is another variant of the normal {@link #reduce(ElemMultiSet,Method,Method)} operation. For the functionality of <tt>reduce</tt> itself,
     * have a look at that method. <p>
     * Here, a plane sweep algorithm is used for the computation of the reduced set:<ul>
     * <li> construct the sweep event structure (ses) from the bounding boxes of the elements in ems; store the right and
     * left interval borders 
     * <li> sort the intervals by their x-coordinate, lower y-coordinate, upper y-coordinate, in that order
     * <li> construct a sweep status structure (sss) which is initially empty but will hold intervals later on
     * <li> traverse the ses; for every element do:<ul>
     *  <li> if it is a left interval: check whether it overlaps any of the intervals in sss. If it does:<ul>
     *   <li> evaluate the <tt>predicate</tt> for the two involved objects
     *   <li> evaluate the operation if the <tt>predicate</tt> yields <tt>true</tt>
     *   <li> if the result of the operation is empty, delete the left and right intervals from ses and sss,
     *  otherwise adjust the intervals in ses and sss</ul>
     *  <li> if it is a left interval and the intervals don't overlap, add the interval to sss
     *  <li> if it is a right interval: add the object to the result set and delete the interval from sss</ul>
     * </ul>
     * This method is successfully used in the <tt>SupportOps.minimal()</tt> method. There, it works for sets of segments,
     * the predicate <tt>adjacent()</tt> and the method <tt>concat()</tt>. No other implementations were tested, yet.<p>
     * Note, that the signature of all predicates and methods must be<br>
     * <code>Element x Element -> boolean</code> and <code>Element x Element -> Element</code>, resp.
     *
     * @param ems the input set of elements
     * @param predicate the method that is used to identify candidates for the second method
     * @param method the method that is invoked on pairs identified by the predicate
     * @param meet if <tt>true</tt>, also objects of <tt>ems</tt> which have only adjacent bounding boxes are tested with predicate
     * @return the 'reduced' set of objects
     */
    public static ElemMultiSet overlapReduceSweep (ElemMultiSet ems, Method predicate, Method method, boolean meet) {
	//construct sweep event structure from ems
	//the left and right intervals of the bounding boxes of all elements in ems are stored 	
	if (ems.isEmpty()) return ems;

	//get some information about the passed methods	
	int paramTypeCountP = Array.getLength(predicate.getParameterTypes());
	int paramTypeCountM = Array.getLength(method.getParameterTypes());
	Element[] paramListP = new Element[paramTypeCountP];
	Element[] paramListM = new Element[paramTypeCountM];
	
	//the elements of ems are stored in an array for better accessibility
	Object[] elemStore = ems.toArray();
	
	//declare the sweep event structure (ses) and fill it with the left and right borders of the
	//bounding boxes of all elements in ems.
	MultiSet ses = new MultiSet(new IvlComparator(meet));
	Object[] sesArr;
	Element actEl;
	Rect actRect;
	boolean buddy;
	for (int i = 0; i < elemStore.length; i++) {
	    actEl = (Element)elemStore[i];
	    actRect = actEl.rect();
	    buddy = actRect.ulx.equal(actRect.urx);
	    ses.add(new Interval(actRect.lly,actRect.uly,"blueleft",actRect.ulx,null,i,buddy));
	    ses.add(new Interval(actRect.lry,actRect.ury,"blueright",actRect.urx,null,i,buddy));
	}//for i

	//declare the sweep status structure
	MultiSet sss = new MultiSet(new IvlComparator(meet));
	Interval actIvl,sesIvl;
	Interval sssIvl = null;
	Iterator it,it2,it3;
	boolean predTrue,resultIsEmpty,removedIvl,ivlOverlap = false;
	ElemMultiSet resultSet = new ElemMultiSet(ELEM_COMPARATOR);
	Element resultElement = null;

	sesArr = ses.toArray();
	
	//start the sweep
	for (int sesIdx = 0; sesIdx < sesArr.length; sesIdx++) {
	    //while there are elements left in ses, continue with the sweep
	    while (sesArr[sesIdx] == null) {
		sesIdx++;
	    }//while
	    
	    actIvl = (Interval)sesArr[sesIdx];

	    //the actually visited interval can be marked with blueleft or blueright
	    //the first case handled is blueleft
	    if (actIvl.mark == "blueleft") {
		resultIsEmpty = false;
		it2 = sss.iterator();
		
		//now, traverse the intervals already stored in sss.
		while (!resultIsEmpty && it2.hasNext()) {
		    sssIvl = (Interval)(((MultiSetEntry)it2.next()).value);
		    predTrue = false;
		    
		    if (actIvl.right.less(sssIvl.left)) {
			//break;
		    }
		    else {
			//define ivlOverlap = true, if actIvl and sssIvl overlap
			//in case flag meet is true, ivlOverlap = true if the intervals are adjacent, too
			if (!meet)
			    ivlOverlap =
				(sssIvl.left.less(actIvl.left) && sssIvl.right.greaterOrEqual(actIvl.left)) ||
				(sssIvl.left.equal(actIvl.left) && sssIvl.right.greater(actIvl.left)) ||
				(sssIvl.left.greater(actIvl.left) && sssIvl.left.less(actIvl.right));
			else
			    ivlOverlap = 
				(sssIvl.left.less(actIvl.left) && sssIvl.right.greaterOrEqual(actIvl.left)) ||
				(sssIvl.left.equal(actIvl.left) && sssIvl.right.greater(actIvl.left)) ||
				(sssIvl.left.greater(actIvl.left) && sssIvl.left.less(actIvl.right)) ||
				sssIvl.right.equal(actIvl.left) || sssIvl.left.equal(actIvl.right);;
		    }//else
		    
		    //if both intervals overlap check the predicate
		    if (ivlOverlap) {
			//now check for predicate
			
			//if predicate has one argument
			if (paramTypeCountP == 1) paramListP[0] = (Element)elemStore[actIvl.number];
			//if predicate has two arguments
			else {
			    paramListP[0] = (Element)elemStore[sssIvl.number];
			    paramListP[1] = (Element)elemStore[actIvl.number];
			}//else
			
			//evaluate predicate
			try {
			    predTrue = ((Boolean)predicate.invoke((Element)elemStore[sssIvl.number],paramListP)).booleanValue();
			} catch (Exception e) {
			    System.out.println("Exception in SetOps.overlapReduceSweep when evaluating predicate.");
			    System.out.println("elemStore[sssIvl.number]: "+(Element)elemStore[sssIvl.number]);
			    System.out.println("elemStore[actIvl.number]: "+(Element)elemStore[actIvl.number]);
			    e.printStackTrace();
			    System.exit(0);
			}//catch

			//invoke method on the elements linked to the intervals, if the predicate yields true
			if (predTrue) {
			    //evaluate method, note that result may be empty
			    if (paramTypeCountM == 1) paramListM[0] = (Element)elemStore[actIvl.number];
			    else {
				paramListM[0] = (Element)elemStore[sssIvl.number];
				paramListM[1] = (Element)elemStore[actIvl.number];
			    }//else
			    
			    //evaluate method
			    try {
				resultElement = (Element)method.invoke((Element)elemStore[sssIvl.number],paramListM);
			    } catch (Exception e) {
				System.out.println("Exception in SetOps.overlapReduceSweep when evaluating method.");
				System.out.println("elemStore[sssIvl.number]: "+(Element)elemStore[sssIvl.number]);
				System.out.println("elemStore[actIvl.number]: "+(Element)elemStore[actIvl.number]);
				e.printStackTrace();
				System.exit(0);
			    }//catch
			    
			    if (resultElement == null) resultIsEmpty = true;
			    
			    //update ref in actEl, update interval borders in actEl
			    elemStore[actIvl.number] = resultElement;
			    actIvl.left = resultElement.rect().lly;
			    actIvl.right = resultElement.rect().uly;
			    
			    //remove the sssIvl from sss and remove right interval with number = sssIvl.number from ses,
			    //i.e. remove the interval also from ses, where its "blueright" interval still exists
			    it2.remove();
			    removedIvl = false;
			    for (int i = sesIdx; i < sesArr.length; i++) {
				if ((sesArr[i] != null) &&((Interval)sesArr[i]).number == sssIvl.number) {
				    sesArr[i] = null;
				    removedIvl = true;
				    break;
				}//if
			    }//for i

			}//if predTrue
		    }//if ivlOverlap
		}//while it2

		//if the result of the method invocation is not empty, add the interval of the new/changed
		//object to the sss
		//otherwise delete the right partner of the left interval from sss
		if (!resultIsEmpty)
		    sss.add(actIvl);
		else {
		    //remove the right interval from ses
		    removedIvl = false;
		    for (int i = sesIdx; i < sesArr.length; i++) {
			if ((sesArr[i] != null) && ((Interval)sesArr[i]).number == sssIvl.number) {
			    sesArr[i] = null;
			    removedIvl = true;
			    break;
			}//if
		    }//for i		    
		}//else
	    }//if blueleft


	    //the actually visited interval is marked with blueright
	    else {
		//store the object linked to the interval in the result set
		resultSet.add(elemStore[actIvl.number]);
		
		//remove left interval from sss
		it2 = sss.iterator();
		while (it2.hasNext()) {
		    sesIvl = (Interval)(((MultiSetEntry)it2.next()).value);
		    if (sesIvl.number == actIvl.number) {
			it2.remove();
			break;
		    }//if
		}//while it2
	    }//else if blueright
	}//while it
	
	return resultSet;
    }//end method overlapReduceSweep



    /**
     * This method is a variant of overlapReduceSweep for methods of type <tt>Element x Element -> ElemMultiSet</tt>.
     * More information about this method can be found there.
     *
     * @param ems the input set of elements
     * @param predicate the method that is used to identify candidates for the second method
     * @param method the method that is invoked on pairs identified by the predicate
     * @param meet if <tt>true</tt>, also objects of <tt>ems</tt> which have only adjacent bounding boxes are tested with predicate
     * @return the 'reduced' set of objects      
     */
    public static ElemMultiSet overlapReduceSweep2 (ElemMultiSet ems, Method predicate, Method method, boolean meet) {
	//construct sweep event structure (ses) from ems;
	//the left an right intervals of the bounding boxes of all elements in ems are stored in ses
	if (ems.isEmpty()) return ems;
	
	//get some information about the passed methods
	int paramTypeCountP = Array.getLength(predicate.getParameterTypes());
	int paramTypeCountM = Array.getLength(method.getParameterTypes());
	Element[] paramListP = new Element[paramTypeCountP];
	Element[] paramListM = new Element[paramTypeCountM];

	//the elements of ems are stored in a Vector we need a dynamic structure here in contrast the 
	//ordinary overlapReduceSweep, since the SES is changed during the sweep
	Vector elemStore = ems.toVector();
	int number = ems.size();
	
	//declare the sweep event structure (SES) and fill it with the left and right borders of the
	//bounding boxes of all elements of ems
	IvlComparator ivlComp = new IvlComparator(meet);
	MultiSet ses = new MultiSet(ivlComp);
	ProLinkedList sesList;
	Element actEl,actEl2;
	Rect actRect;
	boolean buddy;
	for (int i = 0; i < elemStore.size(); i++) {
	    actEl = (Element)elemStore.get(i);
	    actRect = actEl.rect();
	    buddy = actRect.ulx.equal(actRect.urx);
	    ses.add(new Interval(actRect.lly,actRect.uly,"blueleft",actRect.ulx,null,i,buddy));
	    ses.add(new Interval(actRect.lry,actRect.ury,"blueright",actRect.urx,null,i,buddy));
	}//for i
	
	//declare the sweep status structure SSS
	MultiSet sss = new MultiSet(new IvlComparator(meet));
	Interval actIvl,actIvl2,sesIvl;
	Interval sssIvl = null;
	Iterator it2,it4;
	ProListIterator it,it3;
	boolean predTrue = false;
	boolean resultIsEmpty,removedIvl,ivlOverlap = false;
	ElemMultiSet resultSet = new ElemMultiSet(ELEM_COMPARATOR);
	ElemMultiSet resultEMS = null;
	int sweepPointer;

	sesList = new ProLinkedList(ses,new IvlComparatorSimpleType(meet));
	
	//start the sweep
	it = sesList.listIterator(0);
	sweepPointer = -1;
	//while there are elements left in SES, continue with the sweep
	while (it.hasNext()) {
	    actIvl = (Interval)it.next();
	    sweepPointer++;
	    
	    //the actually visited interval is marked with either "blueleft" or "blueright";
	    //handle "blueleft" first
	    if (actIvl.mark == "blueleft") {

		resultIsEmpty = false;
		predTrue = false;
		it2 = sss.iterator();
		
		//now, traverse the intervals already stored in SSS
		while (it2.hasNext()) {
		    sssIvl = (Interval)(((MultiSetEntry)it2.next()).value);
		    predTrue = false;

		    if (actIvl.right.less(sssIvl.left)) {
			//break;
		    }
		    else {
			//define ivlOverlap = true, if actIvl and sssIvl overlap
			//in case flag meet is true, ivlOverlap = true if the intervals are adjacent, too
			if (!meet)
			    ivlOverlap =
				(sssIvl.left.less(actIvl.left) && sssIvl.right.greaterOrEqual(actIvl.left)) ||
				(sssIvl.left.equal(actIvl.left) && sssIvl.right.greater(actIvl.left)) ||
				(sssIvl.left.greater(actIvl.left) && sssIvl.left.less(actIvl.right));
			else
			    ivlOverlap = 
				(sssIvl.left.less(actIvl.left) && sssIvl.right.greaterOrEqual(actIvl.left)) ||
				(sssIvl.left.equal(actIvl.left) && sssIvl.right.greater(actIvl.left)) ||
				(sssIvl.left.greater(actIvl.left) && sssIvl.left.less(actIvl.right)) ||
				sssIvl.right.equal(actIvl.left) || sssIvl.left.equal(actIvl.right);;
		    }//else
		    
		    //if both intervals overlap check the predicate
		    if (ivlOverlap) {
			//now check for predicate
			
			//if predicate has one argument
			if (paramTypeCountP == 1) paramListP[0] = (Element)elemStore.get(actIvl.number);
			//if predicate has two arguments
			else {
			    paramListP[0] = (Element)elemStore.get(sssIvl.number);
			    paramListP[1] = (Element)elemStore.get(actIvl.number);
			}//else

			//evaluate predicate
			try {
			    predTrue = ((Boolean)predicate.invoke((Element)elemStore.get(sssIvl.number),paramListP)).booleanValue();
			} catch (Exception e) {
			    System.out.println("Exception in SetOps.overlapReduceSweep2 when evaluating predicate.");
			    System.out.println("elemStore.get(sssIvl.number): "+(Element)elemStore.get(sssIvl.number));
			    System.out.println("elemStore.get(actIvl.number): "+(Element)elemStore.get(actIvl.number));
			    e.printStackTrace();
			    System.exit(0);
			}//catch
			
			//invoke method on the elements linked to the intervals, if the predicate yields true
			if (predTrue) {

			    //evaluate method, note that result may be empty
			    if (paramTypeCountM == 1) paramListM[0] = (Element)elemStore.get(actIvl.number);
			    else {
				paramListM[0] = (Element)elemStore.get(sssIvl.number);
				paramListM[1] = (Element)elemStore.get(actIvl.number);
			    }//else

			    //evaluate method
			    try {
				resultEMS = (ElemMultiSet)method.invoke((Element)elemStore.get(sssIvl.number),paramListM);
			    } catch (Exception e) {
				System.out.println("Exception in SetOps.overlapReduceSweep when evaluating method.");
				System.out.println("elemStore.get(sssIvl.number): "+(Element)elemStore.get(sssIvl.number));
				System.out.println("elemStore.get(actIvl.number): "+(Element)elemStore.get(actIvl.number));
				e.printStackTrace();
				System.exit(0);
			    }//catch

			    if ((resultEMS == null) || resultEMS.isEmpty()) resultIsEmpty = true;
			    else resultIsEmpty = false;

			    //remove the sssIvl from sss and remove both intervals marked with sssIvl.number from SES
			    //System.out.println("\nremove sssIvl and its partner from sss and ses. sssIvl.number: "+sssIvl.number);
			    it2.remove();
			    it3 = sesList.listIterator(0);
			    int ct = 0;
			    while (it3.hasNext()) {
				actIvl2 = (Interval)it3.next();
				
				if (actIvl2.number == sssIvl.number) {
				    it3.remove();
				    ct++;
				}//if
				if (ct == 2) break;
			    }//while it3
			    sweepPointer--;
			}//if predTrue			
		    }//if ivlOverlap
		    //if method was invoked, break
		    if (predTrue) break;
		}//while it

		if (predTrue) {		    
		    //delete actIvl and its partner from SES
		    it3 = sesList.listIterator(0);
		    int ct = 0;

		    int ct1 = 0;
		    while (it3.hasNext()) {

			actIvl2 = (Interval)it3.next();
			ct1++;
			if (actIvl2.number == actIvl.number) {
			    it3.remove();
			    ct++;
			}//if
			if (ct == 2) break;
		    }//while it
		    if (ct != 2) {
			System.out.println("\nct = "+ct);
			System.out.println("\ndelete actIvl and its partner from SES. actIvl.nr: "+actIvl.number);
			System.out.println("\n********************************");
			System.out.println("SESLIST:");
			for (int i = 0; i < sesList.size(); i++) {
			    System.out.print("["+i+"] ");((Interval)sesList.get(i)).print();
			}
			System.out.println("********************************");
			System.exit(0);
		    }

		    sweepPointer--;
		    
		    //if the result of the method invocation is not empty, add all new objects to SES and elemStore;
		    int pos;
		    if (!resultIsEmpty) {
			it4 = resultEMS.iterator();
			while (it4.hasNext()) {
			    actEl2 = (Element)((MultiSetEntry)it4.next()).value;

			    elemStore.add(actEl2);
			    actRect = actEl2.rect();
			    buddy = actRect.ulx.equal(actRect.urx);

			    pos = sesList.addSorted(new Interval(actRect.lly,actRect.uly,"blueleft",actRect.ulx,null,number,buddy));
			    //one or none of the new intervals may have been added BEFORE the actual x;
			    //set pointer to pos-1 or pos-2, resp.
			    if (pos <= sweepPointer) {
				sweepPointer++;
			    }//if
			    pos = sesList.addSorted(new Interval(actRect.lry,actRect.ury,"blueright",actRect.urx,null,number,buddy));

			    if (pos <= sweepPointer) {
				sweepPointer++;
			    }//if
			    number++;
			}//while it4
			
			it.reset();
			for (int i = 0; i < sweepPointer+1; i++) it.next();
		    } else {
		      
			//if result is empty, two intervals were deleted, that have positions < X;
			//decrease sweepPointer by 2
			it.reset();
			for (int i = 0; i < sweepPointer+1; i++) it.next();			
		    }//else
		   
		} else {
		    //if predTrue = false;
		    //simply add actIvl to sss
		    sss.add(actIvl);
		}//else
	    }//if blueleft
	    
	    //the actually visited interval is marked "blueright"
	    else {
		//store the object linked to the interval in the result set
		resultSet.add(elemStore.get(actIvl.number));
		
		//remove left interval from sss
		it2 = sss.iterator();
		while (it2.hasNext()) {
		    sesIvl = (Interval)(((MultiSetEntry)it2.next()).value);
		    if (sesIvl.number == actIvl.number) {
			it2.remove();
			break;
		    }//if
		}//while it2

		//remove both intervals from ses
		it.remove(); //removes right interval
		it.reset();
		while (it.hasNext()) {
		    sesIvl = (Interval)(it.next());
		    if (sesIvl.number == actIvl.number) {
			it.remove();
			break;
		    }//if
		}//while it

		sweepPointer = sweepPointer-2;
		it.reset();
		for (int i = 0; i < sweepPointer+1; i++)
		    if (it.hasNext()) it.next();
	    }//if blueright
	}//while it

	return resultSet;
    }//end method overlapReduceSweep2

    /**
     * Collects elements from the parameter set and stores them in the resulting set.
     * Every element of <tt>ljpMS</tt> consists of a pair <tt>(Element x ElemMultiSet)</tt>. This method traverses the set and
     * stores the <tt>Element</tt> in the result set if <tt>Element != NULL</tt>. If <tt>Element == NULL</tt>, the <tt>ElemMultiSet</tt> is stored
     * in the result set instead.
     *
     * @param ljpMS the 'in' set
     * @return the collected elements of <tt>ljpMS</tt> stored in an <tt>ElemMultiSet</tt>
     */
    public static ElemMultiSet collect (LeftJoinPairMultiSet ljpMS) {
	ElemMultiSet result = new ElemMultiSet(ELEM_COMPARATOR);
	
	if (ljpMS.isEmpty()) return result;
	
	Iterator it = ljpMS.iterator();
	LeftJoinPair actPair;
	
	while (it.hasNext()) {
	    actPair = (LeftJoinPair)((MultiSetEntry)it.next()).value;
	    if (actPair.elemSet == null) {
		result.add(actPair.element); }
	    else result.addAll(actPair.elemSet);
	}//while it.hasNext

	return result;
    }//end method collect


    /**
     * Collects elements from the parameter set and stores them in the resulting set.
     * This method collects <i>only</i> the elements of the <tt>ElemMultiSet</tt>s of every entry of <tt>ljpMS</tt>.
     * The first entries, i.e. the <tt>Element</tt> part of every pair, is ignored.
     *
     * @param ljpMS the 'in' set
     * @return the collected elements stored in a ElemMultiSet
     */
    public static ElemMultiSet collect2nd (LeftJoinPairMultiSet ljpMS) {
	ElemMultiSet result = new ElemMultiSet(ELEM_COMPARATOR);
	
	if (ljpMS.isEmpty()) return result;
	
	Iterator it = ljpMS.iterator();
	LeftJoinPair actPair;

	while (it.hasNext()) {
	    actPair = (LeftJoinPair)((MultiSetEntry)it.next()).value;
	    if (actPair.elemSet != null) {
		result.addAll(actPair.elemSet);
	    }//if
	}//while

	return result;
    }//end method collect2nd
	

    /**
     * For the two passed sets, a LeftJoinpPairMultiSet is computed where every element of the set has the form <tt>(Element x ElemMultiSet)</tt>.
     * For every pair <tt>Element x ElementOfEMS</tt> of this set element, the <tt>predicate</tt> holds. This set is computed as follows:<ul>
     * <li> call <tt>overlappingPairs()</tt> to find all the pairs <tt>e1 x e2</tt>, <tt>e1</tt> element of <tt>el1</tt> and
     * <tt>e2</tt> element of <tt>el2</tt>, which have
     * overlapping bounding boxes; an {@link EarlyExit} exception is thrown when the <tt>earlyExit</tt> flag is set
     * <li> in a filter step, the number of pairs is reduced using the passed <tt>predicate</tt>
     * <li> while the set of pairs is traversed, all pairs which have the same first element, are collected and all of them are
     * stored in one single entry of the resulting LeftJoinPairMultiSet
     * </ul>
     * The new LeftJoinPairMultiSet is returned, then.<p>
     * This is the 'overlap' version of the normal {@link #leftOuterJoin(ElemMultiSet,ElemMultiSet,Method)}. It only works for predicates which solely hold
     * for elements with overlapping bounding boxes.<p>
     * If, in a special case, for one element <tt>ex</tt> of <tt>el1</tt> no element <tt>ey</tt> of <tt>el2</tt> is found, the entry constructed in the
     * resulting set is <tt>(ex x NULL)</tt>, i.e. the <tt>ElemMultiSet</tt> is <u>not</u> <tt>initialized</tt>.<p>
     * The <tt>predicate</tt> must have the signature <code>Element x Element -> boolean</code>.
     *
     * @param el1 the first set of elements. These elements can be found again in the resulting LeftJoinPairMultiSet
     * as the first entry of every <tt>(Element x ElemMultiSet)</tt>. In particular, this means that if <tt>el1</tt> is <tt>(e1, e2, e3...)</tt>,
     * the resulting set is <tt>( (e1 x ems1), (e2 x ems2), (e3 x ems3) ...)</tt>.
     * @param el2 the second set of elements. The elements of this set are found in the second entries (i.e. in the sets)
     * of the resulting LeftJoinPairMultiSet
     * @param predicate according to this predicate, the ElemMultiSet(s) in the LeftJoinPairMultiSet's entries are built
     * @param useOvLapPairsMeet this flag is passed to <tt>overlappingPairs()</tt>; if <tt>true</tt> objects whith adjacent bounding boxes are
     * reported, too
     * @param bboxFilter this flag is passed to <tt>overlappingPairs()</tt>; if <tt>true</tt>, in a pre-processing step the number of 
     * candidates is reduced by removing objects of <tt>el1</tt>,</tt>el2</tt> which don't overlap the unified bounding box of <tt>el1</tt>,
     * <tt>el2</tt> resp.
     * @param earlyExit if <tt>true</tt>, an {@link EarlyExit} exception is thrown immediately when the first object of <tt>elN</tt> is found which
     * doesn't have a partner in <tt>elM</tt>, i.e. at least one element of <tt>elM</tt> which has a bounding box that overlaps the 
     * bounding box of the element of <tt>elN</tt>. <tt>N</tt> is the passed <tt>setNumber</tt>. earlyExit is evaluated in <tt>bboxFilter</tt>, so if shall
     * be used, be sure that <tt>bboxFilter = true</tt>
     * @param setNumber must be 1 or 2; specifies the set for earlyExit
     * @return the resulting LeftJoinPairMultiSet
     * @throws EarlyExit
     * @see #leftOuterJoin(ElemMultiSet,ElemMultiSet,Method)
     */    
    public static LeftJoinPairMultiSet overlapLeftOuterJoin (ElemMultiSet el1, ElemMultiSet el2, Method predicate, boolean useOvLapPairsMeet, boolean bboxFilter, boolean earlyExit, int setNumber)
    throws EarlyExit {
	//compute overlapping pairs
	PairMultiSet pl;
	try {
	    pl = overlappingPairs(el1,el2,false,useOvLapPairsMeet,bboxFilter,earlyExit,setNumber);
	} catch (NoOverlappingBoxFoundException nop) {
	    throw new EarlyExit();
	}//catch

	//filter set
	pl = filter(pl,predicate,true);

	//make final set; note, that the elements in pl are already sorted
	LeftJoinPairMultiSet retList = new LeftJoinPairMultiSet(LEFTJOINPAIR_COMPARATOR);
	Iterator lit1 = el1.iterator();
	Iterator lit2;
	Element act1;
	ElemPair act2;
	LeftJoinPair ljp;
	boolean stillTrue;
	boolean constructedEMS;
	int num;
	while (lit1.hasNext()) {
	    act1 = (Element)((MultiSetEntry)lit1.next()).value;
	    ljp = new LeftJoinPair();
	    ljp.element = act1;
	    constructedEMS = false;
	    
	    while (pl.size() > 0) {
		act2 = (ElemPair)pl.first();
		num = pl.firstMSE().number;
		if (act2.first.equal(act1)) {
		    if (!constructedEMS) {
			ljp.elemSet = new ElemMultiSet(ELEM_COMPARATOR);
			constructedEMS = true;
		    }//if
		    ljp.elemSet.add(act2.second);
		    pl.removeAllOfThisKind(act2,num);
		}//if
		else break;
	    }//while pl.size > 0

	    retList.add(ljp);
	}//while

	return retList;
    }//end method overlapLeftOuterJoin
    
    
    /**
     * Invokes the passed method on every pairs of elements in every entry of <tt>ljpl</tt>.
     * An entry the passed <tt>ljpl</tt> has the form <tt>(Element x ElemMultiSet)</tt>. Now, let the ElemMultiSet be <tt>(l1, l2 ... ln)</tt>.
     * Then, using the passed method <tt>subtractSets</tt> computes from such a pair <tt>(Element x (l1, l2 ... ln))</tt> a new pair
     * <tt>(method(Element,l1), method(Element,l2) ... method(Element,ln))</tt>.<p>
     * The method must have the signature <code>Element x Element -> Element</code> or
     * <code>Element x Element -> ElemMultiSet</code>.
     *
     * @param ljpl the passed LeftJoinPairMultiSet
     * @param method the method that is invoked on the elements
     * @return the result set with the changed ElemMultiSet(s)
     */
    public static LeftJoinPairMultiSet subtractSets (LeftJoinPairMultiSet ljpl, Method method) {
	Iterator lit1 = ljpl.iterator();
	Iterator lit2;
	LeftJoinPair actLjp;
	ElemMultiSet actList;
	Element actElem1;
	Element actElem2;
	ElemMultiSet subList;
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	boolean metTypeElement = false;
	boolean metTypeElemMultiSet = false;

	//examine passed method
	try {
	    if (method.getReturnType().isInstance(Class.forName("twodsack.setelement.Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.setelement.Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("twodsack.set.ElemMultiSet")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.set.ElemMultiSet"))) {
		metTypeElemMultiSet = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.subtractSets: can't examine method.");
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	//traverse element sets
	while (lit1.hasNext()) {
	    actLjp = (LeftJoinPair)((MultiSetEntry)lit1.next()).value;
	    actElem1 = actLjp.element;
	    actList = actLjp.elemSet;
	    if (!(actList == null)) {
		lit2 = actList.iterator();
		subList = new ElemMultiSet(ELEM_COMPARATOR);
		while (lit2.hasNext()) {
		    actElem2 = (Element)((MultiSetEntry)lit2.next()).value;
		    //if method has one argument
		    if (paramTypeCount == 1) {
			paramList[0] = actElem2; }
		    //if method has two arguments
		    else {
			paramList[0] = actElem1;
			paramList[1] = actElem2; }
		    try {
			if (metTypeElement) {
			    subList.add((Element)(method.invoke(actElem1,paramList))); }
			else if (metTypeElemMultiSet) {
			    subList.addAll((ElemMultiSet)(method.invoke(actElem1,paramList))); }
			else {
			    System.out.println("Error in SetOps.subtractSets: can't invoke method");
			    System.exit(0); }
		    }//try
		    catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			System.out.println("Error in SetOps.subtractSets: Problem with using method.");
			e.printStackTrace();
			System.exit(0);
		    }//catch
		}//while
		//substitute original elemSet with subList
		actLjp.elemSet = subList;
	    }//if != null
	}//while

	return ljpl;
    }//end method subtractSets
   

    /**
     * Returns a set of elements which is 'reduced' using the passed predicate and method.
     * This is a variant of the normal {@link #reduce(ElemMultiSet,Method,Method)} operation. For the fuctionality of <tt>reduce</tt> itself,
     * have a look at that method.<p>
     * This is a recursive algorithm which uses graph algorithms to find groups of candidates for the invocation
     * of the passed method. First note, that the predicate must be an 'overlap' predicate, i.e. is may only yield
     * <tt>true</tt>, if the bounding boxes of both elements overlap or are adjacent at least. Additionally, the method
     * may only return an object which bounding box lies inside of the bounding box of the bounding box of the 
     * objects it was constructed from (for clarification: the result may not be <i>bigger</i> than the original two
     * objects.)<p>
     * <tt>overlapReduce(el,PRED,METH,...)</tt> works as follows:<ol>
     * <li> perform <tt>overlappingPairs</tt> on <tt>el</tt> to find all pairs of candidates, result is called <tt>PL</tt>; if <tt>earlyExit</tt>
     * flag is set,
     * throw an <tt>EarlyExit</tt> exception if neccessary
     * <li> filter <tt>PL</tt> using <tt>PRED</tt>
     * <li> if then, no pairs still exist, return <tt>el</tt>
     * <li> build a graph with vertices (all elements of <tt>el</tt>) and edges; an edge exists between <tt>x,y</tt> (of <tt>el</tt>) if a pair
     * <tt>(x,y)</tt> exists in <tt>PL</tt>
     * <li> compute the connected components of that graph
     * <li> for every component <tt>COMP</tt> of the connected components, do the following <ul>
     *  <li> compute a set of (independent) pairs of that component, such that no element is part of two pairs
     *  <li> invoke the method <tt>METH</tt> on all those pairs
     *  <li> replace every pair <tt>(x,y)</tt> with <tt>METH(x,y)</tt>
     *  <li> call <tt>overlapReduce(COMP,...)</tt>
     * </ul></ol><p>
     * The predicate must have the signature <code>Element x Element -> boolean</code> and method must have the signature
     * <code>Element x Element -> ElemMultiSet</code>
     *
     * @param el the set that shall be reduced
     * @param predicate according to this predicate, candidates are filtered
     * @param method this method is invoked on the pairs of elements that were found
     * @param ovLapPairsMeet this flag is passed to <tt>overlappingPairs()</tt>; if <tt>true</tt>, objects with adjacent bounding boxes are
     * reported, too
     * @param bboxFilter this flag is passed to <tt>overlappingPairs()</tt>; if <tt>true</tt>, in a pre-processing step the number of 
     * candidates is reduced by removing objects of <tt>el1,el2</tt> which don't overlap the unified bounding box of <tt>el1,el2</tt> resp.
     * @param earlyExit if <tt>true</tt>, an <tt>EarlyExit</tt> exception is thrown immediately when the first object of <tt>elN</tt> is found which
     * doesn't have a partner in <tt>elM</tt>, i.e. at least one element of <tt>elM</tt> which has a bounding box that overlaps the 
     * bounding box of the element of <tt>elN</tt>. <tt>N</tt> is the passed setNumber. <tt>earlyExit</tt> is evaluated in <tt>bboxFilter</tt>, so if shall
     * be used, be sure that <tt>bboxFilter = true</tt>
     * @param setNumber must be 1 or 2; specifies the set for <tt>earlyExit</tt>
     * @return the 'reduced' set of elements
     * @throws EarlyExit
     */ 
    public static ElemMultiSet overlapReduce (ElemMultiSet el, Method predicate, Method method, boolean ovLapPairsMeet, boolean bboxFilter, boolean earlyExit, int setNumber)
	throws EarlyExit {
	ElemMultiSet retList = new ElemMultiSet(ELEM_COMPARATOR);
	PairMultiSet pl;
	try {
	    pl = overlappingPairs(el,el,true,ovLapPairsMeet,bboxFilter,earlyExit,setNumber);
	} catch (NoOverlappingBoxFoundException nop) {
	    throw new EarlyExit();
	}//catch
	
	pl = filter(pl,predicate,true);
	if (pl.isEmpty()) {
	    return el; }
	Graph g = new Graph(el,pl);
	ConnectedComponentsPair ccp = g.connectedComponents();

	//the pairs from ccE must not be computed all at once
	//so compute the set of pairs that may be computed
	ccp = g.computeReducedPair(ccp);
	
	ElemMultiSetList vertices = ccp.verticesToEMSList();
	PairMultiSetList edges = ccp.edgesToPairListList();

	ListIterator litE = edges.listIterator(0);
	ListIterator litV = vertices.listIterator(0);
	PairMultiSet actPL;
	ElemMultiSet actEL;
	ElemMultiSet ml = new ElemMultiSet(ELEM_COMPARATOR);
	while (litE.hasNext()) {
	    actPL = (PairMultiSet)litE.next();
	    actEL = (ElemMultiSet)litV.next();

	    //if there are no pairs, get the isolated elements from actEL
	    if (!actPL.isEmpty()) {
		ml = map(actPL,method); }
	    else { ml = new ElemMultiSet(ELEM_COMPARATOR); }
	    retList.addAll(overlapReduce(disjointUnion(ml,actEL),predicate,method,ovLapPairsMeet,bboxFilter,earlyExit,setNumber));
		
	}//for i
	rdup(retList);
	return retList;
	 
    }//end method overlapReduce
    

    /**
     * Lets through pairs of the set depending on the predicate and the 'keep' flag.
     * If <tt>keep = true</tt>, a pair <tt>(x,y)</tt> is kept if <tt>predicate(x,y) = true</tt>. All other pairs are deleted. If <tt>keep = false</tt>,
     * the inverse set is returned.<p>
     * The predicate must have the signature <code>Element x Element -> boolean</code>.
     * 
     * @param plIn the set of pairs
     * @param predicate the predicate that is used to filter the pairs
     * @param keep if <tt>true</tt>, pairs are kept for which the predicate holds
     * @return the filtered set
     */
    public static PairMultiSet filter (PairMultiSet plIn, Method predicate, boolean keep) {
	int paramTypeCount = Array.getLength(predicate.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	boolean predHolds = false;
	Iterator it = plIn.iterator();
	ElemPair actElem;
	MultiSetEntry mse;
	int number;
	while (it.hasNext()) {
	    actElem = (ElemPair)((MultiSetEntry)it.next()).value;

	    //if predicate is non-static
	    if (paramTypeCount == 1) {
		paramList[0] = actElem.second;
	    }//if
	    //if predicate is static
	    else {
		paramList[0] = actElem.first;
		paramList[1] = actElem.second;
	    }//else

	    try {
		predHolds = (((Boolean)predicate.invoke(actElem.first,paramList)).booleanValue());
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" in SetOps.filter(PairList,Method,boolean). Can't invoke method '"+predicate+"' on");
		actElem.first.print();
		actElem.second.print();
		e.printStackTrace();
		System.exit(0);
	    }//catch
	    if ((predHolds && !keep) ||
		(!predHolds && keep)) { it.remove(); }	    
	}//while
	plIn.recomputeSize();
	return plIn;
    }//end method filter
    

    /**
     * Divides the elements of the passed set in groups according to the parameter predicate.<p>
     * This is the 'overlap' variant of the ordinary {@link #group(ElemMultiSet,Method)}. The result of this method is a list of <tt>ElemMultiSet</tt>(s).
     * For every pair of elements <tt>(x,y)</tt> of such a group (<tt>ElemMultiSet</tt>), <tt>predicate(x,y)</tt> holds.<p>
     * First note, that the passed predicate must be an 'overlap' predicate. Such a predicate may only hold, if 
     * the bounding box of its parameter objects overlap or are adjacent at least.<p>
     * This method works as follows:<ul>
     * <li> call <tt>overlappingPairs</tt> to compute the candidate pairs; store the result in <tt>PL</tt>
     * <li> filter <tt>PL</tt> using <tt>predicate</tt>
     * <li> construct a graph from <tt>PL</tt> using <tt>ems</tt> as vertices and <tt>PL</tt> as edges
     * <li> compute the connected components for that graph
     * <li> store all elements of such a component in a single <tt>ElemMultiSet</tt>; store all <tt>ElemMultiSet</tt>(s) in 
     * a <tt>ElemMultiSetList</tt>
     * <li> return that <tt>ElemMultiSetList</tt><p>
     * The signature of the predicate must be <code>Element x Element -> boolean</code>.
     *
     * @param ems the set of elements that shall be divided in groups
     * @param predicate the predicate that is used to filter the candidate pairs
     * @param ovLapPairsMeet this flag is passed to <tt>overlappingPairs()</tt>; if <tt>true</tt>, objects with adjacent bounding boxes are
     * reported, too
     * @return the groups of elements
     */
    public static ElemMultiSetList overlapGroup (ElemMultiSet ems, Method predicate, boolean ovLapPairsMeet) {
	//find the overlapping pairs of elements
	boolean sameSet = true;
	boolean bboxFilter = false;
	boolean earlyExit = false;
	PairMultiSet pl = null;

	double tt01 = System.currentTimeMillis();

	try {
	    pl = overlappingPairs(ems,ems,sameSet,ovLapPairsMeet,bboxFilter,earlyExit,0);
	} catch (Exception e) {
	    System.out.println("Unexpected error in SetOps.overlapGroup during execution of overlappingPairs.");
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	double tt02 = System.currentTimeMillis();

	//filter method: remove pairs for which predicate doesn't hold
	pl = filter(pl,predicate,true);

	double tt03 = System.currentTimeMillis();

	//construct a graph with vertices: elements and edges exist for pairs of elements
	Graph g = new Graph(ems,pl);

	double tt04 = System.currentTimeMillis();

	//compute the connected components
	ConnectedComponentsPair ccp = g.connectedComponents();
	
	double tt05 = System.currentTimeMillis();

	ElemMultiSetList retList = ccp.verticesToEMSList();
	
	double tt06 = System.currentTimeMillis();
	/*
	  System.out.println("\n+++++++++++++++++++++++++++++++++++++");
	  System.out.println("costs for overlapGroup (in detail):");
	  System.out.println("overlappingPairs: "+(tt02-tt01)+" ms");
	  System.out.println("filter: "+(tt03-tt02)+" ms");
	  System.out.println("construct graph: "+(tt04-tt03)+" ms");
	  System.out.println("connectedComponents: "+(tt05-tt04)+" ms");
	  System.out.println("construct result list: "+(tt06-tt05)+" ms");
	  System.out.println("+++++++++++++++++++++++++++++++++++++");
	*/
	return retList;
    }//end method overlapGroup

    
    /**
     * Computes a join on two element sets.<p>
     * This is a variant of the normal <tt>join</tt> operation which uses 'overlap' predicates. 
     * First note, that the passed predicate must be an 'overlap' predicate. Such a predicate may only hold, if 
     * the bounding box of its parameter objects overlap or are adjacent at least.<p>
     * That 'overlap' predicate is also the join predicate. For all pairs <tt>(x,y)</tt> in the resulting set, that predicate holds.<p>
     * The join is computed by first calling <tt>overlappingPairs(ems1,ems2,...)</tt>. After that, the resulting set is filtered
     * using predicate.<p>
     * The predicate must have the signature <code>Element x Element -> boolean</code>.
     *
     * @param ems1 the first set
     * @param ems2 the second set
     * @param predicate the join predicate
     * @param ovLapPairsMeet this flag is passed to <tt>overlappingPairs()</tt>; if <tt>true</tt>, objects with adjacent bounding boxes are
     * reported, too
     * @param bboxFilter this flag is passed to <tt>overlappingPairs()</tt>; if <tt>true</tt>, in a pre-processing step the number of 
     * candidates is reduced by removing objects of <tt>el1,el2</tt> which don't overlap the unified bounding box of <tt>el1,el2</tt> resp.
     * @param earlyExit if <tt>true</tt>, an <tt>EarlyExit</tt> exception is thrown immediately when the first object of <tt>elN</tt> is found which
     * doesn't have a partner in <tt>elM</tt>, i.e. at least one element of <tt>elM</tt> which has a bounding box that overlaps the 
     * bounding box of the element of <tt>elN</tt>. <tt>N</tt> is the passed setNumber. <tt>earlyExit</tt> is evaluated in <tt>bboxFilter</tt>, so if shall
     * be used, be sure that <tt>bboxFilter = true</tt>
     * @param setNumber must be 1 or 2; specifies the set for <tt>earlyExit</tt>
     * @return the PairMultiSet as join of <tt>ems1</tt> and <tt>ems2</tt>
     * @throws EarlyExit
     */
    public static PairMultiSet overlapJoin (ElemMultiSet ems1, ElemMultiSet ems2, Method predicate, boolean ovLapPairsMeet, boolean bboxFilter, boolean earlyExit, int setNumber) 
	throws EarlyExit {
	PairMultiSet retSet;
	try {
	    retSet = overlappingPairs(ems1,ems2,false,ovLapPairsMeet,bboxFilter,earlyExit,setNumber);
	} catch (NoOverlappingBoxFoundException nop) {
	    throw new EarlyExit();
	}//catch

	retSet = filter(retSet,predicate,true);

	return retSet;
    }//end method overlapJoin
   

    /**
     * This method constructs groups from the passed set such that for every element <tt>e</tt> in a group another element <tt>f</tt> can be found with <tt>predicate(e,f) = true</tt>.<p>
     * Example: Let the original set be <tt>{a,b,c,d}</tt>. Assume, that the predicate <tt>p</tt> holds for the pairs <tt>(a,b),(b,c)</tt>.
     * Then, the resulting groups are <tt>{a,b,c}, {d}</tt>.<p>
     * The predicate's signature must be: <tt>Element x Element -> boolean</tt>
     *
     * @param ems the set that shall be divided in groups
     * @param predicate the predicate that is responsible for the division into groups
     * @return the list of groups
     * @see #overlapGroup(ElemMultiSet,Method,boolean)
     */
    public static ElemMultiSetList group (ElemMultiSet ems, Method predicate) {
	/*
	 * This method works as follows: It traverses ems and for each element E it checks all already existing groups.
	 * It traverses each group. For each element F of such group it invokes predicate(E,F) until the predicate holds.
	 * Then, it stores the group in a special list. This is done for each group. The reason why this complicated 
	 * computation is needed is that a new element E can _combine_ two or more groups. Therefore, all groups must be visited.
	 * After all groups were checked, all groups in the special list are joined. If there is no entry in the special list
	 * a new group is build for E.
	 */	

	ElemMultiSetList retList = new ElemMultiSetList();
	
	if (ems == null || ems.isEmpty()) return retList;
	int paramTypeCount = Array.getLength(predicate.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];

	Iterator it = ems.iterator();
	Iterator itRL, it2;
	Element actElem1, actElem2;
	ElemMultiSet actGroup;
	boolean belongsToGroup = false;
	LinkedList idxList = new LinkedList();
 
	//traverse elements of ems
	while (it.hasNext()) {
	    actElem1 = (Element)((MultiSetEntry)it.next()).value;
	    belongsToGroup = false;
	    idxList.clear();
	    
	    if (retList.isEmpty()) {
		//build new group and add it to retList
		ElemMultiSet group = new ElemMultiSet(ELEM_COMPARATOR);
		group.add(actElem1);
		retList.add(group);
	    } else {
		//other groups already exist
		itRL = retList.listIterator(0);
		while (itRL.hasNext()) {
		    actGroup = (ElemMultiSet)itRL.next();

		    it2 = actGroup.iterator();
		    while (it2.hasNext()) {
			actElem2 = (Element)((MultiSetEntry)it2.next()).value;
			if (paramTypeCount == 1)
			    //predicate is non-static
			    paramList[0] = actElem2;
			else {
			    //predicate is static
			    paramList[0] = actElem1;
			    paramList[1] = actElem2;
			}//else
			
			//evaluate predicate
			try {
			    belongsToGroup = ((Boolean)predicate.invoke(actElem1,paramList)).booleanValue();
			} catch (Exception e) {
			    System.out.println("Error in SetOps.group: Wasn't able to invoke predicate.");
			    e.printStackTrace();
			}//catch
			
			System.out.println(belongsToGroup);

			if (belongsToGroup) {
			    //actGroup.add(actElem2);
			    idxList.add(actGroup);
			    break;
			}//if
		    }//while it2
		}//while itRL
		
		if (belongsToGroup || !idxList.isEmpty()) {
		    //merge all groups for which belongsToGroup was true

		    while (idxList.size() > 1) {
			((ElemMultiSet)idxList.get(idxList.size()-2)).addAll((ElemMultiSet)idxList.get(idxList.size()-1));
			//clear and remove second set
			((ElemMultiSet)idxList.get(idxList.size()-1)).clear();
			idxList.remove(idxList.size()-1);
		    }//while
		    
		    //now, add the new element
		    ((ElemMultiSet)idxList.get(0)).add(actElem1);

		    //remove all empty groups from retList
		    itRL = retList.listIterator(0);
		    while (itRL.hasNext()) {
			actGroup = (ElemMultiSet)itRL.next();
			if (actGroup.isEmpty()) itRL.remove();
		    }//while		    

		} else {
		    //actElem doesn't belong to any group
		    //build new group and add it to retList

		    ElemMultiSet group = new ElemMultiSet(ELEM_COMPARATOR);
		    group.add(actElem1);
		    retList.add(group);
		}//else
	    }//else other groups exist
	}//while it hasNext

	return retList;
    }//end method group

    
    /**
     * This map method invokes the <tt>secMethod</tt> on pairs of elements of <tt>ljpl</tt> for which the <tt>predicate</tt> holds.<p>
     * The <tt>mainMethod</tt> itself must be a set operation with the signature <code>Element x ElemMultiSet x predicate
     * x secMethod -> ElemMultiSet</code>. Such a method can be found in this class.
     * Then, this map operation only splits every entry of <tt>ljpl</tt> and calls
     * <tt>mainMethod</tt> with the two objects of the entry and the parameters <tt>predicate</tt> and <tt>secMethod</tt>.<p>
     * The signatures for predicate and secMethod have to be <code>Element x Element -> boolean</code> and
     * <code>Element x Element -> Element</code>, resp.<p>
     * The resulting sets of mainMethods are collected and stored in the result set.
     * @param ljpl the 'in' set
     * @param mainMethod this method is called for every entry of <tt>ljpl</tt>
     * @param predicate the predicate is passed to <tt>mainMethod</tt>
     * @param secMethod this method is passed to <tt>mainMethod</tt>
     * @return the union of the results of <tt>mainMethod</tt>
     */
    public static ElemMultiSet map (LeftJoinPairMultiSet ljpl, Method mainMethod, Method predicate, Method secMethod) {
	ElemMultiSet retList = new ElemMultiSet(ELEM_COMPARATOR);
	Object[] paramListMM = new Object[4];

	ElemMultiSet paramList1 = new ElemMultiSet(ELEM_COMPARATOR);

	int count = 0;

	Iterator lit = ljpl.iterator();
	LeftJoinPair actLjp;
	while (lit.hasNext()) {
	    actLjp = (LeftJoinPair)((MultiSetEntry)lit.next()).value;
	    //if second list is not empty do the computation
	    if (!(actLjp.elemSet == null) && !actLjp.elemSet.isEmpty()) {
		try {
		    paramList1.clear();
		    paramList1.add(actLjp.element);
		    paramListMM[0] = paramList1;
		    paramListMM[1] = actLjp.elemSet;
		    paramListMM[2] = predicate;
		    paramListMM[3] = secMethod;
		    
		    retList.addAll((ElemMultiSet)mainMethod.invoke(null,paramListMM));
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.map(ljpl,m,m,m). Can't invoke method "+mainMethod);
		    e.printStackTrace();
		    System.exit(0);
		}//catch
	    }//if
	    else { retList.add(actLjp.element); }
	}//for i
	
	return retList;
    }//end method map
    

    /**
     * Perfoms a projection on the first element.
     * The first partner of every pair is stored in the result set.
     *
     * @param pl the 'in' set
     * @return the projection on the first element  
     */
    public static ElemMultiSet proj1 (PairMultiSet pl) {
	ElemMultiSet retSet = new ElemMultiSet(ELEM_COMPARATOR);

	Iterator it = pl.iterator();
	while (it.hasNext()) retSet.add(((ElemPair)((MultiSetEntry)it.next()).value).first);

	return retSet;
    }//end method proj1
    

    /**
     * Performs a projection on the second element.
     * The second partner of every pair is stored in the result set.
     *
     * @param pl the 'in' set
     * @return the projection on the second element
     */
    public static ElemMultiSet proj2 (PairMultiSet pl) { 
	ElemMultiSet retSet = new ElemMultiSet(ELEM_COMPARATOR);

	Iterator it = pl.iterator();
	while (it.hasNext()) retSet.add(((ElemPair)((MultiSetEntry)it.next()).value).second);

	return retSet;
    }//end method proj2
   

    /**
     * Returns <tt>true</tt>, if both sets are disjoint.
     * Takes use of the <tt>intersects()</tt> method which must be implemented for each type which implements the {@link Element} interface.
     * An <tt>overlapJoin()</tt> is computed using that <tt>intersects()</tt> method. If the result is empty, <tt>true</tt> is returned.
     * <tt>false</tt> otherwise.
     *
     * @param ems1 the first set
     * @param ems2 the second set
     * @return <tt>true</tt>, if <tt>ems1,ems2</tt> are disjoint
     */
    public static boolean disjoint (ElemMultiSet ems1, ElemMultiSet ems2) {
	if (ems1 == null || ems1.isEmpty() ||
	    ems2 == null || ems2.isEmpty()) return true;
	 
	Class [] paramList = new Class [1];
	PairMultiSet rms = new PairMultiSet(ELEMPAIR_COMPARATOR);
	Class c = ems1.first().getClass();
	
	try {
	    paramList[0] = Class.forName("twodsack.setelement.Element");
	    Method intersectsM = c.getMethod("intersects",paramList);
	    rms = overlapJoin(ems1,ems2,intersectsM,true,true,false,0);
	} catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.disjoint. Can't get Method intersects.");
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	if (rms.isEmpty()) { return true; }
	else { return false; }
    }//end method disjoint


    /**
     * Returns true, if both sets are equal.<p>
     * Uses the <tt>compare()</tt> method which must be implemented for every type which implements the {@link Element} interface.<p>
     * Throws a <tt>WrongTypeException</tt>, if the sets are not of the same type.<p>
     * Note, that <tt>equal((a,a,b), (a,b))</tt> returns <tt>false</tt>.
     *
     * @param el1 the first set
     * @param el2 the second set
     * @return <tt>true</tt>, if both sets are equal
     */
    public static boolean equal (ElemMultiSet el1, ElemMultiSet el2) throws WrongTypeException {
	if (el1.size() != el2.size()) return false;

	Iterator it1 = el1.iterator();
	Iterator it2 = el2.iterator();
	ElemComparator ec = ELEM_COMPARATOR;

	while (it1.hasNext())
	    if (ec.compare(it1.next(),it2.next()) != 0)
		return false;

	return true;
	
    }//end method equal
    
    
    /**
     * For two sets <tt>E,F</tt> and a predicate <tt>p</tt> this method returns a set of {@link LeftJoinPair}s where the first elements of the pairs are elements of <tt>E</tt> and their partners are elements of <tt>F</tt>.
     * Each element of <tt>E</tt> appears exactly once in the resulting {@link LeftJoinPairMultiSet}. For all of the partners of such an
     * element <tt>e</tt>, <tt>p(e,g)</tt>, <tt>g</tt> element of the partner set of <tt>e</tt>, holds.<p>
     * Given an example, if <tt>E,F</tt> are sets of triangles and <tt>p</tt> is a predicate <tt>overlap: Triangle x Triangle -> boolean</tt>.
     * <tt>E = {a,b,c,d}, F = {e,f,g}</tt>. Now, <tt>e</tt> overlaps <tt>a,b</tt> and <tt>g</tt> overlaps <tt>a,b,c</tt>. Then the result would be:<p>
     * <tt>{<br>
     *  (a x {e,g}),<br>
     *  (b x {e,g}),<br>
     *  (c x {g}),<br>
     *  (d x {})<br>
     * }</tt><p>
     * The predicate must have the signature: <tt>Element x Element -> boolean</tt>.
     *
     * @param ems1 the first set
     * @param ems2 the second set
     * @param predicate the join predicate used to construct the <tt>LeftJoinPair</tt>s
     * @return the resulting <tt>LeftJoinPairMultiSet</tt>
     * @see #overlapLeftOuterJoin(ElemMultiSet,ElemMultiSet,Method,boolean,boolean,boolean,int)
     */
    public static LeftJoinPairMultiSet leftOuterJoin (ElemMultiSet ems1, ElemMultiSet ems2, Method predicate) {
	LeftJoinPairMultiSet retSet = new LeftJoinPairMultiSet(LEFTJOINPAIR_COMPARATOR);
	int paramTypeCount = Array.getLength(predicate.getParameterTypes());
	boolean predHolds = false;
	Element[] paramList = new Element[paramTypeCount];
	
	Iterator it1 = ems1.iterator();
	Iterator it2;
	Element actElem1,actElem2;

	//traverse the first set
	while (it1.hasNext()) {
	    actElem1 = (Element)((MultiSetEntry)it1.next()).value;
	    
	    //construct new LeftJoinPair and initialize values
	    LeftJoinPair newLjp = new LeftJoinPair();
	    newLjp.element = actElem1;
	    newLjp.elemSet = new ElemMultiSet(ELEM_COMPARATOR);
	    
	    //traverse the second set
	    it2 = ems2.iterator();
	    while (it2.hasNext()) {
		actElem2 = (Element)((MultiSetEntry)it2.next()).value;
		
		//set paramList
		if (paramTypeCount == 1)
		    //predicate is non-static
		    paramList[0] = actElem2;
		else {
		    //predicate is static
		    paramList[0] = actElem1;
		    paramList[1] = actElem2;
		}//else
		
		//invoke predicate
		try {
		    predHolds = ((Boolean)predicate.invoke(actElem1,paramList)).booleanValue();
		} catch (Exception e) {
		    System.out.println("Error in SetOps.leftOuterJoin: Can't invoke predicate.");
		    e.printStackTrace();
		}//catch
		
		//add element to elemSet if predHolds == true
		if (predHolds)
		    newLjp.elemSet.add(actElem2);
	    }//while it2

	    //add new LeftJoinPair to retSet
	    retSet.add(newLjp);
	}//while it1

	return retSet;
    }//end method leftOuterJoin



    /**
     * For every pair <tt>(Element x ElemMultiSet)</tt> of <tt>ljpMS</tt> the method is invoked on that pair if the predicate holds.
     * The predicate may be <tt>NULL</tt>. If so, it is assumed, that the predicate holds for all pairs of <tt>ljpMS</tt>. Then, the
     * method is invoked on all entries with no further checks.<p>
     * For every {@link LeftJoinPair}, the <tt>methody</tt> is invoked on that pair, if the <tt>predicate</tt> holds (if not <tt>NULL</tt>). Then, the
     * result of the method invocation is stored in <tt>LeftJoinPair.elemSet</tt>, i.e. it replaces the original
     * <tt>ElemMultiSet</tt>. The first argument of the method (the <tt>Element</tt>) remains unchanged.<p>
     * The allowed signature for predicate is <code>Element x ElemMultiSet -> boolean</code>. The method must have
     * the signature <code>Element x ElemMultiSet -> ElemMultiSet</code>.
     *
     * @param ljpMS the 'in' set
     * @param predicate it checks whether the <tt>method</tt> may be invoked on a pair; may be <tt>NULL</tt>
     * @param method is invoked on each pair for which the predicate holds
     * @return the changed <tt>leftJoinPairMultiSet</tt>
     */
    public static LeftJoinPairMultiSet map (LeftJoinPairMultiSet ljpMS, Method predicate, Method method) {
	boolean predOkay = (predicate != null);
	int paramTypeCountP = 0;
	Object[] paramListP = null;
	if (predOkay) {
	    paramTypeCountP = Array.getLength(predicate.getParameterTypes());
	    paramListP = new Object[paramTypeCountP];
	}//if
	int paramTypeCountM = Array.getLength(method.getParameterTypes());
	Object[] paramListM = new Object[paramTypeCountM];
	ElemMultiSet returnSet = new ElemMultiSet(ELEM_COMPARATOR);
	
	Iterator it = ljpMS.iterator();
	LeftJoinPair actLJP;
	Element actElem;
	ElemMultiSet actSet;
	boolean isTrue = false;
	ElemMultiSet mResult = null;
	while (it.hasNext()) {
	    actLJP = (LeftJoinPair)((MultiSetEntry)it.next()).value;
	    actElem = actLJP.element;
	    actSet = actLJP.elemSet;
	    
	    //if predicate is okay and non-static
	    if (predOkay) {
		if (paramTypeCountP == 1) {
		    paramListP[0] = actSet; }
		else {
		    paramListP[0] = actElem;
		    paramListP[1] = actSet;
		}//else
		
		//compute boolean value for predicate
		try {
		    isTrue = ((Boolean)predicate.invoke(actElem,paramListP)).booleanValue();
		} catch (Exception e) {
		    System.out.println("\nException in SetOps.map(ljpMS,pred,met) when computing boolean value.");
		    e.printStackTrace();
		    System.exit(0);
		}//catch
		
	    }//if
	    else isTrue = true;
	    
	    if (isTrue) {
		//if method has one argument
		if (paramTypeCountM == 1) 
		    paramListM[0] = actSet;
		else {
		    paramListM[0] = actElem;
		    paramListM[1] = actSet;
		}//else
		
		//invoke method
		try {
		    mResult = (ElemMultiSet)method.invoke(actElem,paramListM);
		} catch (Exception e) {
		    System.out.println("Exception in SetOps.map(ljpMS,pred,met) when invoking method.");
		    System.out.println("\nmethod: "+method);
		    System.out.println("\nactElem: "+actElem);
		    System.out.println("\nparamListM[0]: "+paramListM[0]);
		    e.printStackTrace();
		    System.exit(0);
		}//catch
		
		//set result
		actLJP.elemSet = mResult;
	    }//if isTrue
	}//while it.hasNext
	       
	return ljpMS;
    }//end method map
    

    /**
     * As long as a pair <tt>e1</tt> of <tt>el1 x e2</tt> of <tt>el2</tt> exists which <tt>predicate(e1,e2) == true</tt>, replace <tt>e1</tt> in <tt>el</tt> with <tt>method(e1,e2)</tt>.
     * The <tt>predicate</tt> and <tt>method</tt> must have the signature <code>Element x Element -> boolean</code> and
     * <code>Element x Element -> Element</code> (of the same type as <tt>el1</tt>), resp.<p>
     * This is a very expensive operation w.r.t. time consumption!
     *
     * @param el1 the first set
     * @param el2 the second set
     * @param predicate chooses the candidates for <tt>method</tt>
     * @param method is invoked on candidates selected by <tt>predicate</tt>
     * @return the 'subtracted' set
     */
    public static ElemMultiSet subtract (ElemMultiSet el1, ElemMultiSet el2, Method predicate, Method  method) {
	int paramTypeCountPred = Array.getLength(predicate.getParameterTypes());
	int paramTypeCountMet = Array.getLength(method.getParameterTypes());
	ElemMultiSet retSet = (ElemMultiSet)el1.clone();
	ElemMultiSet paramList = (ElemMultiSet)el2.clone();
	boolean stillExist = false;
	boolean isTrue = false;
	Element[] paramListP = new Element[paramTypeCountPred];
	Element[] paramListM = new Element[paramTypeCountMet];

	ElemMultiSet mreturn = new ElemMultiSet(ELEM_COMPARATOR);

	Iterator lit1;
	Iterator lit2;
	Element actEl1;
	Element actEl2;
	ElemMultiSet finalSet = new ElemMultiSet(ELEM_COMPARATOR);
	MultiSetEntry mse1;

	do {
	    stillExist = false;
	    lit1 = retSet.iterator(); 
	    while (lit1.hasNext()) {
		mse1 = (MultiSetEntry)lit1.next();
		actEl1 = (Element)mse1.value;

		isTrue = false;
		lit2 = paramList.iterator();
		while (lit2.hasNext()) {
		    actEl2 = (Element)((MultiSetEntry)lit2.next()).value;

		    //if predicate has one argument
		    if (paramTypeCountPred == 1) {
			paramListP[0] = actEl2;
		    }//if

		    //if predicate has two arguments
		    else {
			paramListP[0] = actEl1;
			paramListP[1] = actEl2;
		    }//else
		    try {
			isTrue = ((Boolean)predicate.invoke(actEl1,paramListP)).booleanValue();
		    }//try
		    catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			System.out.println("Error in SetOps.subtract(el,el,pr,m).");
			System.out.println("Cause for Exception: "+e.getCause());
			System.out.println("Exception String: "+e.toString());
			e.printStackTrace();
			System.exit(0);
		    }//catch
		    if (isTrue) {
			//if method has one argument
			if (paramTypeCountMet == 1) {
			    paramListM[0] = actEl2;
			}//if
			//if method has two arguments
			else {
			    paramListM[0] = actEl1;
			    paramListM[1] = actEl2;
			}//else
			try {
			    mreturn = (ElemMultiSet)(method.invoke(actEl1,paramListM));
			}//try
			catch (Exception e) {
			    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			    System.out.println("Error in SetOps.subtract(el,el,pr,m). Can't invoke method "+method);
			    System.out.println("Cause for Exception: "+e.getCause());
			    System.out.println("Exception String: "+e.toString());
			    e.printStackTrace();
			    System.exit(0);
			}//catch

			if (isTrue) {
			    //copy first elements of retSet to finalSet
			    //these elements are not needed anymore
			    SortedSet ss = retSet.treeSet().headSet(mse1);
			    Iterator tempIt = ss.iterator();
			    while (tempIt.hasNext()) {
				finalSet.add((Element)((MultiSetEntry)tempIt.next()).value); }

			    retSet.removeAllOfThisKind(actEl1); 
			    retSet.addAll(mreturn);
			    mreturn = null;
			    stillExist = true;
			    lit1 = retSet.iterator();
			    break;
			}//if
	
			if (retSet.isEmpty()) { break; }
		    }//if
		}//for j
		if (retSet.isEmpty()) {
		    break; }
	    }//for i
	}//do
	while (stillExist);

	//copy all elements of retSet to finalSet
	Iterator tmpIt = retSet.iterator();
	while (tmpIt.hasNext()) {
	    finalSet.add((Element)((MultiSetEntry)tmpIt.next()).value); }

	return finalSet;
    }//end method subtract


    /**
     * Returns a pair of elements which returns the maximum value of all possible pairs of <tt>ems1,ems2</tt> using method.<p>
     * All pairs of <tt>e1</tt> of <tt>ems1</tt> and <tt>e2</tt> of <tt>ems2</tt> are checked for their return value of
     * <tt>method(e1,e2)</tt>. That pair, which returns
     * the maximum value of all pairs, is returned.<p>
     * The passed method must have the signature <code>Element x Element -> Rational</code>.
     *
     * @param ems1 the first set
     * @param ems2 the second set
     * @param method the method that is used to compute the values for <tt>ElemPair</tt>(s)
     * @return the pair with the maximum value
     */
    public static ElemPair max (ElemMultiSet ems1, ElemMultiSet ems2, Method method) {
	Element elem1 = (Element)ems1.first();
	Element elem2 = (Element)ems2.first();
	Rational maximum = RationalFactory.constRational(0); // just for initialization
	Rational value = RationalFactory.constRational(0); //dto.
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];

	//initialization
	try {
	    //if method has one argument
	    if (paramTypeCount ==1) paramList[0] = (Element)ems2.first();
	    //if method has two arguments (is static)
	    else { 
		paramList[0] = (Element)ems1.first();
		paramList[1] = (Element)ems2.first();
	    }//else
	    maximum = ((Rational)method.invoke(elem1,paramList));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.max. Can't invoke method "+method);
	    e.printStackTrace();
	    System.exit(0);
	}//catch

	Iterator it1 = ems1.iterator();
	Iterator it2;
	Element actElem1;
	Element actElem2;

	while (it1.hasNext()) {
	    actElem1 = (Element)((MultiSetEntry)it1.next()).value;
	    it2 = ems2.iterator();
	    while (it2.hasNext()) {
		actElem2 = (Element)((MultiSetEntry)it2.next()).value;
		try {
		    //if method has one argument
		    if (paramTypeCount == 1) {
			paramList[0] = actElem2;
		    }//if
		    //if method has two arguments (is static)
		    else {
			paramList[0] = actElem1;
			paramList[1] = actElem2;
		    }//else
		    value = ((Rational)method.invoke(actElem1,paramList));
		} catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.max. Can't invoke method "+method);
		    e.printStackTrace();
		    System.exit(0);
		}//catch

		if (value.greater(maximum)) {
		    maximum = value.copy();
		    elem1 = actElem1;
		    elem2 = actElem2;
		}//if
	    }//while it2
	}//while it1

	return new ElemPair(elem1,elem2);
    }//end method max


    /**
     * Returns a pair of elements which returns the minimum value of all possible pairs of <tt>ems1,ems2</tt> using method.<p>
     * All pairs of <tt>e1</tt> of <tt>ems1</tt> and <tt>e2</tt> of <tt>ems2</tt> are checked for their return value of
     * <tt>method(e1,e2)</tt>. That pair, which returns
     * the minimum value of all pairs, is returned.<p>
     * The passed method must have the signature <code>Element x Element -> Rational</code>.
     *
     * @param el1 the first set
     * @param el2 the second set
     * @param method the method that is used to compute the values for <tt>ElemPair</tt>(s)
     * @return the pair with the maximum value
     */
    public static ElemPair min (ElemMultiSet el1, ElemMultiSet el2, Method method) {
	if (el1 == null || el1.isEmpty() || el2 == null || el2.isEmpty()) return null;

	Element elem1 = (Element)el1.first();
	Element elem2 = (Element)el2.first();;
	Rational value = null;
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	Rational minimum = null;
	
	//initialization
	try {
	    //if method has one argument
	    if (paramTypeCount == 1) {
		paramList[0] = (Element)el2.first();
	    }//if
	    //if method has two arguments (is static)
	    else {
		paramList[0] = (Element)el1.first();
		paramList[1] = (Element)el2.first();
	    }//else
	    minimum = ((Rational)method.invoke(el1.first(),paramList));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.min. Can't invoke method "+method);
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	Iterator it1 = el1.iterator();
	Iterator it2;
	Element actElem1;
	Element actElem2;
	
	while (it1.hasNext()) {
	    actElem1 = (Element)((MultiSetEntry)it1.next()).value;
	    it2 = el2.iterator();
	    while (it2.hasNext()) {
		actElem2 = (Element)((MultiSetEntry)it2.next()).value;
		//if method has one argument
		if (paramTypeCount == 1) {
		    paramList[0] = actElem2;
		}//if
		//if method has two arguments (is static)
		else {
		    paramList[0] = actElem1;
		    paramList[1] = actElem2;
		}//else
		try {
		    value = ((Rational)method.invoke(actElem1,paramList));
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.min. Can't invoke method "+method);
		    e.printStackTrace();
		    System.exit(0);
		}//catch
		
		if (value.equal(0)) return new ElemPair(actElem1,actElem2);

		if (value.less(minimum)) {
		    minimum = value.copy();
		    elem1 = actElem1;
		    elem2 = actElem2;
		}//if
	    }//while j
	}//while i
	
	return new ElemPair(elem1,elem2);
    }//end method min
    

    /**
     * As long as a pair <tt>e1,e2</tt> of <tt>el</tt> exists with <tt>predicate(e1,e2) == true</tt>, replace both elements by <tt>method(e1,e2)</tt>.<p>
     * The reduce operation traverses the set <tt>el</tt> and searches for pairs of elements for which the <tt>predicate</tt> holds.
     * If such a pair is found, both elements are removed from the set and the result of the <tt>method</tt> invoked on that
     * pair is added to the set. If, at some point, no such pair can be found, reduce exits and returns the 'reduced' set.<p>
     * The <tt>predicate</tt> must have the signature <code>Element x Element -> boolean</code>. The <tt>method</tt> must have the signature
     * <code>Element x Element -> Element</code> (of the same type).<p>
     * This method is very time consuming. Some other implementations of <tt>reduce</tt> exist and can be found in this class.
     *
     * @param el the 'in' set that shall be reduced
     * @param predicate checks for candidates for the method
     * @param method is invoked on the candidates found by the predicate
     * @return the 'reduced' set
     * @see #overlapReduce(ElemMultiSet,Method,Method,boolean,boolean,boolean,int)
     * @see #overlapReduceSweep(ElemMultiSet,Method,Method,boolean)
     */
    public static ElemMultiSet reduce (ElemMultiSet el, Method predicate, Method method) {
	ElemMultiSet retSet = el.copy();

	boolean stillExist;
	boolean isTrue;
	int paramTypeCountPred = Array.getLength(predicate.getParameterTypes());
	Element[] paramList = new Element[paramTypeCountPred];
	Element[] paramListMet = new Element[2];

	Element actElem1;
	Element actElem2;
	//these variables are used to determine wether the method
	//has return type Element or ElemList
	boolean metTypeElement = false;
	boolean metTypeElemMultiSet = false;

	try {
	    if (method.getReturnType().isInstance(Class.forName("twodsack.setelement.Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.setelement.Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("twodsack.set.ElemMultiSet")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.set.ElemMultiSet"))) {
		metTypeElemMultiSet = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.reduce: can't examine method.");
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	//NEW CODE WITH ITERATORS AND MULTISETS
	Iterator it1;
	Iterator it2;
	MultiSetEntry mse1;
	int number1;
	MultiSetEntry mse2;
	int number2;

	//first, handle multiple elements:
	//the set is traversed and for all duplicates, the entries are replaced by the method result,
	//if the predicate holds
	it1 = retSet.iterator();
	while(it1.hasNext()) {
	    mse1 = (MultiSetEntry)it1.next();
	    if (mse1.number > 1) {
		//check for predicate
		
		//if predicate has one argument
		if (paramTypeCountPred == 1)
		    paramList[0] = (Element)mse1.value;
		//if predicate has two arguments (is static)
		else {
		    paramList[0] = (Element)mse1.value;
		    paramList[1] = (Element)mse1.value;
		}//else
		isTrue = false;
		try {
		    isTrue = ((Boolean)predicate.invoke(mse1.value,paramList)).booleanValue();
		} catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.reduce: Problem with using predicate.");
		    e.printStackTrace();
		    System.exit(0);
		}//catch
		
		if (isTrue) {
		    paramListMet[0] = (Element)mse1.value;
		    paramListMet[1] = (Element)mse1.value;
		    try {
			//if method has returnType Element
			if (metTypeElement)
			    retSet.add((Element)(method.invoke(null,paramListMet)));
			else {
			    if (metTypeElemMultiSet)
				retSet.addAll((ElemMultiSet)(method.invoke(null,paramListMet)));
			    else {
				System.out.println("ERROR in SO.reduce(): can't invoke method");
				System.exit(0);
			    }//else
			}//else
		    } catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			System.out.println("Error in SetOps.reduce: Problem with using method.");
			e.printStackTrace();
			System.exit(0);
		    }//catch
		    
		    //remove element from set
		    it1.remove();
		}//if isTrue
	    }//if mse1.number > 1
	}//while it.hasNext

	//now, all duplicates should be removed
	
	//start processing the remaining elements
       
	do {
	    //reset loop variables
	    stillExist = false;
	    isTrue = false;
	    
	    it1 = retSet.iterator();
	    while (it1.hasNext()) {
		//get element from first set
		mse1 = (MultiSetEntry)it1.next();
		actElem1 = (Element)mse1.value;
		isTrue = false;
		it2 = retSet.iterator();
		while (it2.hasNext()) {
		    //get element from second set
		    mse2 = (MultiSetEntry)it2.next();
		    if (((Object)mse2).equals((Object)mse1))
			if (it2.hasNext()) mse2 = (MultiSetEntry)it2.next();
			else break;
			
		    number2 = mse2.number;
		    actElem2 = (Element)mse2.value;
		    
		    //if predicate has one argument
		    if (paramTypeCountPred == 1)
			paramList[0] = actElem2;
		    //if predicate has two arguments (is static)
		    else {
			paramList[0] = actElem1;
			paramList[1] = actElem2;
		    }//else
		    isTrue = false;
		    try {
			isTrue = ((Boolean)predicate.invoke(actElem1,paramList)).booleanValue();
		    }//try
		    catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			System.out.println("Error in SetOps.reduce: Problem with using predicate.");
			e.printStackTrace();
			System.exit(0);
		    }//catch
		    
		    if (isTrue) {
			paramListMet[0] = actElem1;
			paramListMet[1] = actElem2;
			try {
			    //if method has returnType Element
			    if (metTypeElement)
				retSet.add((Element)(method.invoke(null,paramListMet)));
			    else {
				if (metTypeElemMultiSet)
				    retSet.addAll((ElemMultiSet)(method.invoke(null,paramListMet)));
				else {
				    System.out.println("ERROR in SO.reduce(): can't invoke method");
				    System.exit(0);
				}//else
			    }//else
			}//try
			catch (Exception e) {
			    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			    System.out.println("Error in SetOps.reduce: Problem with using method.");
			    e.printStackTrace();
			    System.exit(0);
			}//catch
			
			//now remove elements from retSet
			retSet.removeOneEntry(actElem1);
			retSet.removeOneEntry(actElem2);
			
			stillExist = true;
			isTrue = false;
			if (retSet.isEmpty())
			    stillExist = false;
		    }//if isTrue
		    if (stillExist) break;
		}//while it2
		if (stillExist) break;
	    }//while it1
	} while (stillExist);
				    	   
	return retSet;
    }//end method reduce
    

    /**
     * From two sets <tt>E,F</tt> and a predicate <tt>p</tt> this method computes a set of pairs <tt>(e,f)</tt> with <tt>e</tt> element of
     * <tt>E</tt>, <tt>f</tt> element of <tt>F</tt> and <tt>p(e,f) == true</tt>.<p>
     * The predicate's signature must be <tt>Element x Element -> boolean</tt>. This method uses a O(n²) algorithm.
     *
     * @param ems1 the first set
     * @param ems2 the second set
     * @param predicate the join predicate
     * @return the set of pairs
     * @see #overlapJoin(ElemMultiSet,ElemMultiSet,Method,boolean,boolean,boolean,int)
     */
    public static PairMultiSet join (ElemMultiSet ems1, ElemMultiSet ems2, Method predicate) {
	PairMultiSet retSet = new PairMultiSet(new ElemPairComparator());
	if (ems1 == null || ems2 == null || ems1.isEmpty() || ems2.isEmpty()) return retSet;
	
	int paramTypeCount = Array.getLength(predicate.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];

	Iterator it1 = ems1.iterator();
	Iterator it2;
	Element actElem1, actElem2;
	
	while (it1.hasNext()) {
	    actElem1 = (Element)((MultiSetEntry)it1.next()).value;
	    it2 = ems2.iterator();
	    while (it2.hasNext()) {
		actElem2 = (Element)((MultiSetEntry)it2.next()).value;
		
		if (paramTypeCount == 1)
		    //predicate is non-static
		    paramList[0] = actElem2;
		else {
		    //predicate is static
		    paramList[0] = actElem1;
		    paramList[1] = actElem2;
		}//else
		
		//invoke predicate
		try {
		    if (((Boolean)predicate.invoke(actElem1,paramList)).booleanValue())
			retSet.add(new ElemPair(actElem1,actElem2));
		} catch (Exception e) {
		    System.out.println("Error in SetOps.join: Wasn't able to invoke predicate.");
		    e.printStackTrace();
		}//catch
	    }//while it2
	}//while it1

	return retSet;
    }//end method join


    
    /**
     * Invokes the passed method on each pair of the passed PairMultiSet. The result is collected and returned.<p>
     * The method must have the signature <code>Element x Element -> Element</code> or 
     * <tt>Element x Element -> ElemMultiSet</tt>.<p>
     * At the end, duplicates are removed.
     *
     * @param pl the set of ElemPair(s)
     * @param method the method that is invoked on the ElemPair(s)
     * @return the set of results collected in an ElemMultiSet
     */
    public static ElemMultiSet map (PairMultiSet pl, Method method) {
	ElemMultiSet retSet = new ElemMultiSet(ELEM_COMPARATOR);
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	Iterator lit = pl.iterator();
	ElemPair actPair;
	boolean metTypeElement = false;
	boolean metTypeElemList = false;
	Element result;
	try {
	    if (method.getReturnType().isInstance(Class.forName("twodsack.setelement.Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.setelement.Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("twodsack.set.ElemMultiSet")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.set.ElemMultiSet"))) {
		metTypeElemList = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.reduce: can't examine method.");
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	MultiSetEntry mse;
	int number;
	while (lit.hasNext()) {
	    mse = (MultiSetEntry)lit.next();
	    number = mse.number;
	    actPair = (ElemPair)mse.value;
	    //muliset handling
	    for (int msenum = 0; msenum < number; msenum++) {
		try {

		    //if method has one argument
		    if (paramTypeCount == 1) {
			paramList[0] = actPair.second;
		    }//if
		    //if method has two arguments (is static)
		    else {
			paramList[0] = actPair.first;
			paramList[1] = actPair.second;
		    }//else

		    //if returntype of m is Element
		    if (metTypeElement) {
			result = (Element)method.invoke(actPair.first,paramList);
			if (result != null)
			    retSet.add(result);
			//retSet.add((Element)(method.invoke(actPair.first,paramList)));

		    }//if
		    //if returntype of m is ElemList
		    else {
			if (metTypeElemList) {
			    retSet.addAll((ElemMultiSet)(method.invoke(actPair.first,paramList)));

			}//if
			else {
			    System.out.println("Error in SetOps.map: can't invoke method");
			    System.exit(0);
			}//else
		    }//else

		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.map(pl,m). Can't invoke method "+method);
		    e.printStackTrace();
		    System.exit(0);
		}//catch
	    }//for
	}//while 
	return retSet;
    }//end method map
    

    /**
     * Invokes the method on every element of the set and returns the resulting set.<p>
     * Duplicates are removed afterwards.<p>
     * The signature of the method <tt>m</tt> must be one of
     * <code>Element -> Element</code>,<p>
     * <code>Element -> ElemMultiSet</code> or<p>
     * <code>Element -> Element[]</code>.
     *
     * @param el the 'in' set
     * @param m the method that is invoked on the elements of <tt>el</tt>
     * @return the changed <tt>el</tt>
     */
    public static ElemMultiSet map (ElemMultiSet el, Method m) {
	ElemMultiSet retList = new ElemMultiSet(ELEM_COMPARATOR);
	boolean mRetTypeElem = false;
	boolean mRetTypeElemMultiSet = false;
	boolean mRetTypeElemArray = false; 

	try {
	    mRetTypeElemArray = m.getReturnType().isArray();
	    if (!mRetTypeElemArray) {
		mRetTypeElem = (m.getReturnType().isInstance(Class.forName("twodsack.setelement.Element")) ||
				m.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.setelement.Element")));
		if (!mRetTypeElem)
		    mRetTypeElemMultiSet = (m.getReturnType().isInstance(Class.forName("twodsack.set.ElemMultiSet")) ||
					    m.getReturnType().getSuperclass().isAssignableFrom(Class.forName("twodsack.set.ElemMultiSet"))); 
	    }//if
	} catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.subtractSets: can't examine method.");
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	Iterator lit = el.iterator();
	Element actEl;
	MultiSetEntry mse;
	int numOfmse;

	while (lit.hasNext()) {
	    mse = (MultiSetEntry)lit.next();
	    actEl = (Element)mse.value;
	    numOfmse = mse.number;
	    //handling multiple entries
	    for (int nom = 0; nom < numOfmse; nom++) {
		try {
		    //if returntype of m is Element
		    if (mRetTypeElem) {
			retList.add(m.invoke(actEl,null));
		    }//if
		    //if returntype of m is ElemList
		    else {
			if (mRetTypeElemMultiSet) {
			    retList.addAll((ElemMultiSet)m.invoke(actEl,null));
			}//if
			else {
			    if (mRetTypeElemArray) {
				retList.add((Element[])m.invoke(actEl,null));
			    }//if
			    else {
				System.out.println("ERROR (SetOps.map): can't invoke method");
				System.exit(0);
			    }//else
			}//else
		    }//else
		    
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.map. Can't invoke method "+m);
		    e.printStackTrace();
		    System.exit(0);
		}//catch
	    }//for nom
	}//while

	return retList;
    }//end method map
 

    /**
     * Returns the disjoint union of both sets.
     * This means that <tt>disjointUnion((a,b),(b,c)) -> (a,b,b,c)</tt>.
     *
     * @param el1 the first set
     * @param el2 the second set
     * @return the disjoint union of <tt>el1,el2</tt>
     */
    public static ElemMultiSet disjointUnion (ElemMultiSet el1, ElemMultiSet el2) {
	ElemMultiSet eUnion = el1.copy();

	eUnion.addAll(el2);
	return eUnion;
    }//end method disjointUnion
    

    /**
     * Returns the intersecion of both sets.<p>
     * This means that <tt>intersection((a,b),(b,c)) -> (b)</tt>.<p>
     * This method uses the <tt>compare()</tt> method which must be implemented for any type that implements the Element interface and
     * throws a WrongTypeException if the sets are not of the same type.
     *
     * @param el1In the first set
     * @param el2In the second set
     * @return the intersection of <tt>el1In,el2In</tt>
     * @throws WrongTypeException
     */
    public static ElemMultiSet intersection (ElemMultiSet el1In, ElemMultiSet el2In) throws WrongTypeException {
	ElemMultiSet eRet = new ElemMultiSet(ELEM_COMPARATOR);

	if ((el1In.size() == 0) ||
	    (el2In.size() == 0)) 
	    return eRet;

	ElemMultiSet ems1 = el1In;
	ElemMultiSet ems2 = el2In;

	Iterator it1 = ems1.iterator();
	Iterator it2 = ems2.iterator();
	boolean getNextE1 = true;
	boolean getNextE2 = true;
	MultiSetEntry mse1 = null;
	MultiSetEntry mse2 = null;
	Element e1,e2;

	while (true) {
	    if (getNextE1) { mse1 = (MultiSetEntry)it1.next(); }
	    if (getNextE2) { mse2 = (MultiSetEntry)it2.next(); }
	    e1 = (Element)mse1.value;
	    e2 = (Element)mse2.value;
	    
	    int cmp = e1.compare(e2);
	    if (cmp == 0) {
		eRet.add(e1);
		getNextE1 = true;
		getNextE2 = true;
	    }//if
	    else if (cmp == -1) {
		getNextE1 = true;
		getNextE2 = false;
	    }//if
	    else {
		getNextE1 = false;
		getNextE2 = true;
	    }//else
	    if ((getNextE1 && !it1.hasNext()) ||
		(getNextE2 && !it2.hasNext()))
		break;
	}//while

	return eRet;
    }//end method intersection
    

    /**    
     * Returns the difference of the two sets.<p>
     * This means that <tt>difference((a,b,c),(a,b)) -> (c)</tt>.<p>
     * This method uses the <tt>compare()</tt> method which must be implemented for any type that implements the <tt>Element</tt> interface and
     * throws a WrongTypeException if the sets are not of the same type.
     *
     * @param el1 the first set
     * @param el2 the second set
     * @return the difference <tt>el - el2</tt>
     * @throws WrongTypeException
     */
    public static ElemMultiSet difference (ElemMultiSet el1, ElemMultiSet el2) throws WrongTypeException {
	ElemMultiSet eDiff = new ElemMultiSet(ELEM_COMPARATOR);
	
	int el1size = el1.size();
	int el2size= el2.size();
	
	el1 = rdup(el1);
	el2 = rdup(el2);
	
	if (el2.isEmpty()) { return el1; }

	Iterator it1 = el1.iterator();
	Iterator it2 = el2.iterator();
	boolean next1 = true;
	boolean next2 = true;
	Element elem1 = null;//just for initialization
	Element elem2 = null;//dito
	int comp;
	
	while ((!next1 || it1.hasNext()) &&
	       (!next2 || it2.hasNext())) {
	    if (next1) elem1 = (Element)((MultiSetEntry)it1.next()).value;
	    if (next2) elem2 = (Element)((MultiSetEntry)it2.next()).value;
            next1 = false;
            next2 = false;
	    
	    if (elem1.equal(elem2)) {
		next1 = true;
		next2 = true;
	    }//if
	    else {
		comp = elem1.compare(elem2);
		switch (comp) {
		case -1 : {
		    eDiff.add(elem1.copy());
		    next1 = true;
		    break; }
		case 1 : { next2 = true; }
		}//switch
	    }//else
	}//while

	if (!next1) { eDiff.add(elem1.copy()); }

	while (it1.hasNext()) {
	    eDiff.add(((Element)((MultiSetEntry)it1.next()).value).copy());
	}//while

	return eDiff;
    }//end method difference
    

    /**
     * Removes the duplicates from the (multi-)set.
     * As an ElemMultiSet is a set type that allows duplicates, this method removes all duplicates, i.e.
     * <tt>rdup((a,b,b,c)) -> (a,b,c)</tt>.
     *
     * @param elIn the 'in' set
     * @return <tt>elIn</tt> without duplicates
     * @throws WrongTypeException
     */
    public static ElemMultiSet rdup (ElemMultiSet elIn) throws WrongTypeException {
	Iterator it = elIn.iterator();
	MultiSetEntry mse;
	int num;
	while (it.hasNext()) {
	    mse = (MultiSetEntry)it.next();
	    num = mse.number;
	    elIn.size = elIn.size - num + 1;
	    mse.number = 1;
	}//while

	return elIn;    
    }//end method rdup


    /**
     * Removes from the set all elements which are found more than once.
     * This means that <tt>rdup2((a,b,b,c)) -> (a,c)</tt>.
     *
     * @param elIn the 'in' set
     * @return <tt>elIn</tt> without any duplicates
     */     
    public static ElemMultiSet rdup2 (ElemMultiSet elIn) {
	Iterator it = elIn.iterator();
	while (it.hasNext()) {
	    MultiSetEntry mse = (MultiSetEntry)it.next();
	    if (mse.number > 1)
		it.remove();
	}//while
	elIn.recomputeSize();

	return elIn;
    }//end method rdup2


    /**
     * Computes the union of both sets.<p>
     * This means that <tt>union((a,b),(b,c)) -> (a,b,c)</tt>. The duplicates are removed.
     * A <tt>WrongTypeException</tt> is thrown, if the sets are not of the same type.
     *
     * @param el1 the first set
     * @param el2 the second set
     * @return the union of <tt>el1,el2</tt>
     * @throws WrongTypeException
     */
    public static ElemMultiSet union (ElemMultiSet el1, ElemMultiSet el2) throws WrongTypeException {
	ElemMultiSet eRet = new ElemMultiSet(ELEM_COMPARATOR);
	eRet = rdup(disjointUnion(el1,el2));
	return eRet;
    }//end method union
    

    /**
     * Sums up the results of method <tt>m</tt> invoked on every element of the set.
     * Method <tt>m</tt> must have the signature <code>Element -> double</code>.
     *
     * @param ems the 'in' set
     * @param m the method that is invoked on the elements of ems
     * @return the sum of the results of <tt>m</tt>'s invocation
     */
    public static double sum (ElemMultiSet ems, Method m) {
	double retSum = 0;
	Iterator it = ems.iterator();
	MultiSetEntry mse;

	try {
	    while (it.hasNext()) {
		mse = (MultiSetEntry)it.next();
		for (int count = 0; count < mse.number; count++) {
		    retSum = retSum + ((Double)m.invoke((Element)mse.value,null)).doubleValue();
		}//for count
	    }//while
	} catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.sum. Can't invoke method "+m);
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	return retSum;
    }//end method sum
    

    /**
     * Computes for two sets of geometrical objects a set of object pairs for which holds, that their bounding boxes overlap.<p>
     * This method implements a pretty complex DAC algorithm which is not described here.<p>
     * In general, pairs are computed for objects of two sets (which don't have to be of the same type). But, when using the 
     * <tt>sameSet</tt> flag, one may indicate that the sets passed are actually the same object. In that case, all pairs of identical
     * objects are removed. To clarify this: If <tt>overlappingPairs</tt> is invoked on <tt>(e1,e2,e3...) x (f1,f2,f3)</tt> one would get (at least)
     * the pairs <tt<(e1,f1),(e2,f2),(e3,f3)</tt< because <tt>(e1,f2)</tt> etc. are identical and therefore have overlapping bounding boxes.
     * If the <tt>sameSet</tt> flag is set, these pairs are not found in the resulting set.<p>
     * Using the <tt>meet</tt> flag, an overlap of the object's bounding boxes is not required. Now, adjacency of bounding boxes is
     * sufficient for an object pair to be stored in the result set.<p>
     * If the <tt>bboxFilter</tt> flag is set, first the bounding box of the complete sets <tt>el1,el2</tt> is computed, i.e. that complete bounding box
     * includes <u>all</u> of the set's objects. Then, all object's bounding boxes of <tt>el1</tt> are checked against the <tt>el2</tt> complete box.*
     * Objects with boxes that don't intersect the set's box are removed. The same is done for the elements of <tt>el2</tt> and the box of <tt>el1</tt>.
     * <p>
     * When using the <tt>earlyExit</tt> flag, the <tt>setNumber</tt> is evaluated. This number must be 1 or 2 and specifies one of the two sets. When,
     * during the execution of <tt>bboxFilter</tt> one object of that set is removed for the reason that it doesn't overlap the big box
     * of the other set, a <tt>NoOverlappingBoxFoundException</tt> is thrown.<p>
     * Note, that the <tt>earlyExit</tt> flag can <u>only</u> be used together with the <tt>bboxFilter</tt> flag and the <tt>setNumber</tt>.
     *
     * @param el1 the first set
     * @param el2 the second set
     * @param sameSet choose <tt>true</tt>, if identical pairs should not be reported in the case that <tt>el1 == el2</tt>
     * @param meet set to <tt>true</tt>, if adjancency for bounding boxed should suffice instead of overlap
     * @param bboxFilter use <tt>true</tt>, if a filter step should be used to reduce the number of objects
     * @param earlyExit if set to <tt>true</tt>, an <tt>NoOverlappingboxException</tt> will be thrown, if one element of the set specified by
     * setNumber finds no partner
     * @param setNumber specifies the set for <tt>earlyExit</tt>
     * @return the set of object pairs which all have overlapping (or adjacent) bounding boxes
     * @throws NoOverlappingBoxFoundException
     */
    public static PairMultiSet overlappingPairs(ElemMultiSet el1, ElemMultiSet el2, boolean sameSet, boolean meet, boolean bboxFilter, boolean earlyExit, int setNumber) throws NoOverlappingBoxFoundException {
	PairMultiSet pairs = new PairMultiSet(new ElemPairComparator());
		
	if (el1.isEmpty() || el2.isEmpty()) return pairs;
	
	Object[] ivlArr;
	
	if (bboxFilter) {
	    //Use bboxFilter to reduce number of elements.
	    ElemMultiSet[] newSets = bboxFilter(el1,el2,earlyExit,setNumber);
	    if (newSets[0].isEmpty() || newSets[1].isEmpty()) return pairs;

	    //generate interval list which stores left,right vertical
	    //intervals of the elements bboxes of el1,el2      
	    MultiSet ems = generateIntervalList(newSets[0],newSets[1],meet);
	    ivlArr = ems.toArray();
	}//if bboxFilter
	else {
	    //generate interval list which stores left,right vertical
	    //intervals of the elements bboxes of el1,el2      
	    //Store elements in an array.
	    MultiSet ems = generateIntervalList(el1,el2,meet);
	    ivlArr = ems.toArray();
	}//else     
	
	//everything is done so start the DAC
	IvlList mtList = new IvlList();
	ResultList inlist = new ResultList(mtList,mtList,mtList,mtList,mtList,mtList,mtList);
	
	//initialize a structure for storing the already found intersections
	ProLinkedList[] intStore = new ProLinkedList[el1.size()+el2.size()];
	for (int i = 0; i < intStore.length; i++) {
	    intStore[i] = new ProLinkedList(); }
	
	PairMultiSet pairList = new PairMultiSet(new ElemPairComparator());
	int idx1 = 0;
	int idx2 = ivlArr.length-1;
	ResultList rl = computeOverlaps(intStore,inlist,sameSet,el1.size(),pairList,ivlArr,idx1,idx2);
	
	return pairList;
    }//end method overlappingPairs
    
    
    /**
     * This method does the <i>real</i> work for the DAC.
     * Comments are found in the code.
     *
     * @param intStore this array stores for every object of the first set a list with objects of the second set which have overlapping
     * or adjacent bounding boxes; in fact, not the objects themselves are stored in that array, but the object IDs
     * @param il in this structure, all left,right,blue,green etc. intervals are stored; additionally, all information which is passed
     * through the several recursive steps of this algorithm are stored in here
     * @param sameSet the flag which indicates whether both (initial) sets are equal or not
     * @param size the size of the first (initial)set
     * @param resultingPairs the set of pairs which has to be computed
     * @param ivlArr stores the left and right border intervals of the objects bounding boxes
     * @param idx1 defines the actual left index on <tt>ivlArr</tt>
     * @param idx2 defines the acutal right index on <tt>ivlArr</tt>
     * @return the new ResulList structure
     */
    private static ResultList computeOverlaps(ProLinkedList[] intStore, ResultList il, boolean sameSet, int size, PairMultiSet resultingPairs,Object[] ivlArr, int idx1, int idx2) {
	//initially il.m is the full intervall list
	ResultList rl;
	
	//is m small enough?
	//if (il.m.size() == 1) {
	if (idx2 == idx1) {

	    rl = new ResultList(il.m,null,null,null,null,null,null);
		
	    //dependent on the only element in m compute the sets
	    //blue, green, blueLeft, blueRight, greenLeft, greenRight
	    //Interval actInt = (Interval)il.m.getFirst();
	    Interval actInt = (Interval)ivlArr[idx1];
	    if (actInt.mark == "blueleft") {
		if (rl.blue == null) rl.blue = new IvlList();
		rl.blue.add(actInt);
		rl.green = il.green;
		if (rl.blueLeft == null) rl.blueLeft = new IvlList();
		rl.blueLeft.add(actInt);
		rl.blueRight = il.blueRight;
		rl.greenLeft = il.greenLeft;
		rl.greenRight = il.greenRight;
	    }//if
	    else if (actInt.mark == "blueright") {
		if (rl.blue == null) rl.blue = new IvlList();
		rl.blue.add(actInt);
		rl.green = il.green;
		rl.blueLeft = il.blueLeft;
		if (rl.blueRight == null) rl.blueRight = new IvlList();
		rl.blueRight.add(actInt);
		rl.greenLeft = il.greenLeft;
		rl.greenRight = il.greenRight;
	    }//if
	    else if (actInt.mark == "greenleft") {
		rl.blue = il.blue;
		if (rl.green == null) rl.green = new IvlList();
		rl.green.add(actInt);;
		rl.blueLeft = il.blueLeft;
		rl.blueRight = il.blueRight;
		if (rl.greenLeft == null) rl.greenLeft = new IvlList();
		rl.greenLeft.add(actInt);
		rl.greenRight = il.greenRight;
	    }//if
	    else if (actInt.mark == "greenright") {
		rl.blue = il.blue;
		if (rl.green == null) rl.green = new IvlList();
		rl.green.add(actInt);
		rl.blueLeft = il.blueLeft;
		rl.blueRight = il.blueRight;
		rl.greenLeft = il.greenLeft;
		if (rl.greenRight == null) rl.greenRight = new IvlList();
		rl.greenRight.add(actInt);
	    }//if
	}//if size() == 1
	
	else {
	    //DIVIDE
	    int half = (idx2-idx1)/2;
	    
	    int m1idx1 = idx1;
	    int m1idx2 = idx1+half;

	    int m2idx1 = m1idx2+1;
	    int m2idx2 = idx2;

 
	    //CONQUER
	    ResultList rl1 =
		computeOverlaps(intStore,
				new ResultList(null,il.blue,il.green,il.blueLeft,il.blueRight,il.greenLeft,il.greenRight),
				sameSet,size,resultingPairs,
				ivlArr,m1idx1,m1idx2);
	    ResultList rl2 = 
		computeOverlaps(intStore,
				new ResultList(null,il.blue,il.green,il.blueLeft,il.blueRight,il.greenLeft,il.greenRight),
				sameSet,size,resultingPairs,
				ivlArr,m2idx1,m2idx2);

	    //MERGE
	    IvlList fullBlue;
	    IvlList fullGreen;
	   
	    fullBlue = IvlList.intersect(rl1.blueLeft,rl2.blueRight);
	    fullGreen = IvlList.intersect(rl1.greenLeft,rl2.greenRight);
	   
	    rl = new ResultList(null,null,null,null,null,null,null);
	    
	    rl.blue = IvlList.merge(rl1.blue,rl2.blue,true);
	    rl.green = IvlList.merge(rl1.green,rl2.green,true);

	    IvlList rl1BlueLeftMINUSfullBlue = IvlList.minus(rl1.blueLeft,fullBlue);
	    IvlList rl1GreenLeftMINUSfullGreen = IvlList.minus(rl1.greenLeft,fullGreen);
	    IvlList rl2BlueRightMINUSfullBlue = IvlList.minus(rl2.blueRight,fullBlue);
	    IvlList rl2GreenRightMINUSfullGreen = IvlList.minus(rl2.greenRight,fullGreen);
	    
	    rl.blueLeft = IvlList.merge(rl1BlueLeftMINUSfullBlue,rl2.blueLeft,false);
	    rl.blueRight = IvlList.merge(rl2BlueRightMINUSfullBlue,rl1.blueRight,false);
	    rl.greenLeft = IvlList.merge(rl1GreenLeftMINUSfullGreen,rl2.greenLeft,false);
	    rl.greenRight = IvlList.merge(rl2GreenRightMINUSfullGreen,rl1.greenRight,false);
      
	    //compute pairs
	    resultingPairs = IvlList.overlappingIntervals(intStore,sameSet,size,
							  rl1BlueLeftMINUSfullBlue,
							  rl2.green, resultingPairs);
	    resultingPairs = IvlList.overlappingIntervals(intStore,sameSet,size,rl2.blue,
							  rl1GreenLeftMINUSfullGreen,
							  resultingPairs);
	    resultingPairs = IvlList.overlappingIntervals(intStore,sameSet,size,
							  rl2BlueRightMINUSfullBlue,
							  rl1.green,resultingPairs);
	    resultingPairs = IvlList.overlappingIntervals(intStore,sameSet,size,rl1.blue,
							  rl2GreenRightMINUSfullGreen,
							  resultingPairs);
	}//else

	return rl;
    }//end method compute overlaps
    

    /**
     * Constructs a set of intervals which are the border intervals form the bounding boxes of <tt>el1</tt>'s and <tt>el2</tt>'s objects.
     * This is a supportive method for the <tt>overlappingPairs</tt> method. From the elements of <tt>el1,el2</tt>, the vertical intervals
     * of the bounding boxes are taken and stored as intervals in the resulting MultiSet. All elements of el1 are marked
     * with "blueleft" or "blueright" and elements of <tt>el2</tt> are marked with "greenleft" and "greenright". Additionally,
     * all bounding boxes get a number which is assigned to the intervals. A flag is set for every interval, whether
     * the partner of an inerval is located at the same x-coordinate or not.
     *
     * @param el1 the first set
     * @param el2 the second set
     * @param meet <tt>true</tt>, if adjacency of bounding boxes suffices for reporting it in the result set
     * @return the MultiSet with the intervals
     */
    static private MultiSet generateIntervalList (ElemMultiSet el1, ElemMultiSet el2, boolean meet) {
	MultiSet retSet = new MultiSet(new IvlComparator(meet));
	int counter = 0;
	Iterator it1 = el1.iterator();
	Iterator it2 = el2.iterator();
	Element actEl;
	Rect actRect;
	MultiSetEntry mse;
	boolean buddy;

	while (it1.hasNext()) {
	    mse = (MultiSetEntry)it1.next();

	    actEl = (Element)mse.value;
	    actRect = actEl.rect();
	    buddy = actRect.ulx.equal(actRect.urx);
	    //multiset handling
	    for (int msenum = 0; msenum < mse.number; msenum++) {
		retSet.add(new Interval(
					actRect.lly,
					actRect.uly,
					"blueleft",
					actRect.ulx,
					actEl,
					counter,
					buddy));	

		retSet.add(new Interval(
					actRect.lry,
					actRect.ury,
					"blueright",
					actRect.urx,
					actEl,
					counter,
					buddy));
		
		counter++;
	    }//for
	}//while
	
	while (it2.hasNext()) {
	    mse = (MultiSetEntry)it2.next();

	    actEl = (Element)mse.value;
	    actRect = actEl.rect();
	    buddy = actRect.ulx.equal(actRect.urx);
	    for (int msenum = 0; msenum < mse.number; msenum++) {
		retSet.add(new Interval(
					actRect.lly,
					actRect.uly,
					"greenleft",
					actRect.ulx,
					actEl,
					counter,
					buddy));

		retSet.add(new Interval(
					actRect.lry,
					actRect.ury,
					"greenright",
					actRect.urx,
					actEl,
					counter,
					buddy));

		counter++;
	    }//for
	}//while	
       
	return retSet;
    }//end method generateIntervalList


    /**
     * Sorts the input list using mergesort.
     * First, elements are sorted using their <tt>compareX()</tt> method, then using their <tt>compareY()</tt> method.<p>
     * This method can <i>only</i> be used for <tt>Element</tt> lists.
     * 
     * @param el the unsorted list
     * @return the sorted list
     */
    public static void mergesortXY (LinkedList el) {
	mergesXY(el,0,el.size()-1);
    }//end method mergesortXY


    /**
     * This is a supportive method for {@link #mergesortXY(LinkedList)}.
     *
     * @param list the list to be sorted
     * @param lo the bottom index
     * @param hi the top index
     * @return a list that is sorted form lo to hi
     */
    private static void mergesXY (LinkedList list, int lo, int hi) {
	//lo,hi are indices to determine which part of the list shoult be sorted
	int m;
	if (lo < hi) {
	    m = (lo + hi) / 2;
	    mergesXY (list,lo,m);
	    mergesXY (list,m+1,hi);
	    mergeXY (list,lo,hi); 
	}//if
    }//end method mergesXY


    /**
     * Supportive method for {@link #mergesXY(LinkedList,int,int)}.
     *
     * @param list the list to be sorted
     * @param lo the bottom index
     * @param hi the top index
     * @return the list that is sorted from <tt<lo</tt> to <tt>hi</tt>
     */
    private static void mergeXY(LinkedList list, int lo, int hi) {
	int i,j,k,m,n = hi-lo+1;
	Element[] tmp = new Element[n];//temporary array
	byte erg;
	
	k = 0;
	m = (lo+hi)/2;
	//copy lower half to array b
	for (i = lo; i <= m; i++)
	    tmp[k++] = (Element)list.get(i);
	//copy upper half in reverse order to array b
	for (j = hi; j >= m+1; j--)
	    tmp[k++] = (Element)list.get(j);

	i  = 0;
	j = n-1;
	k = lo;

	//copy the next greatest element back until i,j meet
	while (i <= j) {
	    erg = tmp[i].compX(tmp[j]);
	    if (erg == 0) {
		erg = tmp[i].compY(tmp[j]);
		switch (erg) {
		case -1 : { erg = 1; break; }
		case 1 :  { erg = -1; break; }
		case 0 : erg = 0;
		}//switch
	    }//if

	    switch (erg) {
	    case -1 : { list.set(k++,(Element)tmp[i++]); break; }
	    case 0 : { list.set(k++,(Element)tmp[i++]); break; }
	    case 1 : list.set(k++,(Element)tmp[j--]);
	    }//switch
	}//while
    }//end method mergeXY
 
    
    /**
     * Returns the subsets of elements which bounding boxes overlap the bounding box of the element set of the other set.<p>
     * The bounding boxes for every passed set is computed first. Then, the intersection of these bounding boxes,
     * which is again a rectangle and can be described as the "convex hull rectangle", is computed.
     * Afterwards, the elements of both sets are compared to this convex hull rectangle. If the bounding box of such
     * an element has at least one common point with the hull, it belongs to the subset for a set. Subsets are computed
     * for both sets and are returned in an array.
     * The parameter <code>earlyExit</code> can be used to stop the execution of the calling algorithm. Therefore, 
     * a <code>setNumber</code> is passed together with <code>earlyExit</code>. If <code>true</code>,
     * <code>bboxFilter</code> throws a {@link NoOverlappingBoxFoundException} if the bounding box of an element of the
     * set specified by <code>setNumber</code> doesn't overlap the hull of the other set.
     *
     * @param ems1 the first set of elements
     * @param ems2 the second set of elements
     * @param earlyExit must be <tt>true</tt>, if execution shall be stopped
     * @param setNumber the number of the set which is examined if <tt>earlyExit == true</tt>
     * @return the subsets
     */
    public static ElemMultiSet[] bboxFilter (ElemMultiSet ems1, ElemMultiSet ems2, boolean earlyExit, int setNumber)
	throws NoOverlappingBoxFoundException {
	
	if (earlyExit && (setNumber < 1 || setNumber > 2)) throw new InternalError("SetOps.bboxFilter: setNumber must be 1 or 2.");

	ElemMultiSet[] retArr = new ElemMultiSet[2];
	if (ems1.size() == 0 || ems2.size() == 0) return retArr;

	ElemMultiSet ems1Clone = (ElemMultiSet)ems1.clone();
	ElemMultiSet ems2Clone = (ElemMultiSet)ems2.clone();

	Rect ems1Rect = ems1Clone.rect();
	Rect ems2Rect = ems2Clone.rect();

	Rect combinedRect = ems1Rect.intersection(ems2Rect);

	Element actElem;
	Iterator it;

	it = ems1Clone.iterator();
	while (it.hasNext()) {
	    actElem = (Element)((MultiSetEntry)it.next()).value;
	    if (!actElem.rect().hasCommonPoints(combinedRect)) {
		if (earlyExit && setNumber == 1) throw new NoOverlappingBoxFoundException();
		it.remove();
	    }//if
	}//while
	ems1Clone.recomputeSize();
	retArr[0] = ems1Clone; 
	
	it = ems2Clone.iterator();
	while (it.hasNext()) {
	    actElem = (Element)((MultiSetEntry)it.next()).value;
	    if (!actElem.rect().hasCommonPoints(combinedRect)) {
		if (earlyExit && setNumber == 2) throw new NoOverlappingBoxFoundException();
		it.remove();
	    }//if
	}//while
	ems2Clone.recomputeSize();
	retArr[1] = ems2Clone;
	
	return retArr;
    }//end method bboxFilter

}//class SetOps  






