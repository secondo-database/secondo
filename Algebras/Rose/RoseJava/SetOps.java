import java.util.*;
import java.lang.reflect.*;

class SetOps {
    //in this class we only find set operations working with 'elements'
    //and not with specific primitive types
    
    
    //members

    //constructors
    
    //methods
    public static ElemList subtractRemove (ElemList el1, ElemList el2, Method predicate, Method  method) {
	//...

	//System.out.println("\nentering SO.subtractRemove...");

	int paramTypeCountPred = Array.getLength(predicate.getParameterTypes());
	int paramTypeCountMet = Array.getLength(method.getParameterTypes());
	ElemList retList = (ElemList)el1.clone();
	boolean stillExist = false;
	boolean isTrue = false;
	Element[] paramListP = new Element[paramTypeCountPred];
	Element[] paramListM = new Element[paramTypeCountMet];
	ElemList mreturn = new ElemList();
	ListIterator lit1;
	ListIterator lit2;
	Element actEl1;
	Element actEl2;

	do {
	    stillExist = false;
	    //System.out.println("\nsize retList:"+retList.size()+", size el2:"+el2.size());
	    lit1 = retList.listIterator(0); 
	    //for (int i = 0; i < retList.size(); i++) {
	    while (lit1.hasNext()) {
		actEl1 = (Element)lit1.next();
		isTrue = false;
		lit2 = el2.listIterator(0);
		while (lit2.hasNext()) {
		    //for (int j = 0; j < el2.size(); j++) {
		    actEl2 = (Element)lit2.next();
		    //System.out.println("\ni:"+(lit1.nextIndex()-1)+", j:"+(lit2.nextIndex()-1));
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
			//System.out.println("checked for predicate..."+isTrue);
			//actEl1.print();
			//actEl2.print();
		    }//try
		    catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" ---"+e.getMessage());
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
			    mreturn = (ElemList)(method.invoke(actEl1,paramListM));
			}//try
			catch (Exception e) {
			    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			    System.exit(0);
			}//catch
			if (isTrue) {
			    //System.out.println("*****mreturn:");
			    //mreturn.print();
			    //System.out.println("*****mreturn-end");
			    int index = lit1.nextIndex()-1;
			    lit1.remove();
			    lit2.remove();
			    retList.addAll(mreturn);
			    stillExist = true;
			    lit1 = retList.listIterator(index);
			    lit2 = el2.listIterator(0);
			    break;
			}//if
			if (retList.isEmpty()) { break; }
		    }//if
		}//for j
		if (retList.isEmpty()) {
		    //System.out.println("retList is empty ----> break;");
		    break; }
	    }//for i
	}//do
	while (stillExist);
	/*
	if (!retList.isEmpty()) {
	    System.out.println("retList is not empty:");
	    retList.print();
	    System.exit(0); }
	*/
	//System.out.println("leaving SO.subtract.");
	return retList;
    }//end method subtract


    public static LeftJoinPairList overlapLeftOuterJoin (ElemList el1, ElemList el2, Method predicate, boolean useOvLapPairsMeet) {
	//overlap version of leftOuterJoin
	//
	//useOvLapPairsMeet is passed to overlappingPairs
	
	//compute overlapping pairs
	PairList pl = overlappingPairs(el1,el2,false,useOvLapPairsMeet);
	//filter list
	pl = filter(pl,predicate,true);
	
	//sort pl and el1
	quicksortX(el1);
	quicksort(pl);

	//System.out.println("sorted pl:"); pl.print();
	//System.exit(0);

	//make final list
	LeftJoinPairList retList = new LeftJoinPairList();
	ListIterator lit1 = el1.listIterator(0);
	ListIterator lit2 = pl.listIterator(0);
	Element act1;
	ElemPair act2;
	while (lit1.hasNext()) {
	    act1 = (Element)lit1.next();
	    LeftJoinPair ljp = new LeftJoinPair();
	    ljp.element = act1;
	    ljp.elemList = new ElemList();
	    while (lit2.hasNext()) {
		act2 = (ElemPair)lit2.next();
		if (act2.first.equal(act1)) {
		    ljp.elemList.add(act2.second); }
		else {
		    if (lit2.hasPrevious()) {
			act2 = (ElemPair)lit2.previous(); }
		    break;
		}//else
	    }//while
	    retList.add(ljp);
	}//while

	//System.out.println("retList:"); retList.print();
	//System.exit(0);

	return retList;
    }//end method overlapLeftOuterJoin


    public static ElemList overlapSubtract (ElemList el1, ElemList el2, Method predicate, Method method, boolean ovLapPairsMeet) {
	//this is a variant of the operation subtract
	//ovLapPairsMeet is passed to overlappingPairs	

	PairList pl = overlappingPairs(el1,el2,false,ovLapPairsMeet);
	pl = filter(pl,predicate,true);
	if (pl.isEmpty()) {
	    return el1; }
	//...;


	return el1;
    }//end method overlapSubtract

    
    public static LeftJoinPairList subtractSets (LeftJoinPairList ljpl, Method method) {
	//computes subtractSets

	ListIterator lit1 = ljpl.listIterator(0);
	ListIterator lit2;
	LeftJoinPair actLjp;
	ElemList actList;
	Element actElem1;
	Element actElem2;
	ElemList subList;
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	boolean metTypeElement = false;
	boolean metTypeElemList = false;
	//System.out.println("check1");
	try {
	    if (method.getReturnType().isInstance(Class.forName("Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("ElemList")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("ElemList"))) {
		metTypeElemList = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.subtractSets: can't examine method.");
	    System.exit(0);
	}//catch

	//System.out.println("check2");
	
	while (lit1.hasNext()) {
	    actLjp = (LeftJoinPair)lit1.next();
	    actElem1 = actLjp.element;
	    actList = actLjp.elemList;
	    lit2 = actList.listIterator(0);
	    subList = new ElemList();
	    while (lit2.hasNext()) {
		actElem2 = (Element)lit2.next();
		//System.out.println("lit1:"+(lit1.nextIndex()-1)+", lit2:"+(lit2.nextIndex()-1));
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
		    else if (metTypeElemList) {
			subList.addAll((ElemList)(method.invoke(actElem1,paramList))); }
		    else {
			System.out.println("Error in SetOps.subtractSets: can't invoke method");
			System.exit(0); }
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.subtractSets: Problem with using method.");
		    System.exit(0);
		}//catch
	    }//while
	    //substitute original elemList with subList
	    actLjp.elemList = subList;
	}//while

	return ljpl;
    }//end method subtractSets

    
    public static ElemList subtract (LeftJoinPairList ljpl, Method method) {
	//subtracts the elements in elemList of every ljpl from the elem in ljpl
	ElemList retList = new ElemList();
	ListIterator lit1 = ljpl.listIterator(0);
	ListIterator lit2;
	Element actElem1;
	Element actElem2;
	ElemList actList;
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	boolean metTypeElement = false;
	boolean metTypeElemList = false;
	try {
	    if (method.getReturnType().isInstance(Class.forName("Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("ElemList")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("ElemList"))) {
		metTypeElemList = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.subtractSets: can't examine method.");
	    System.exit(0);
	}//catch
	
	while (lit1.hasNext()) {
	    actElem1 = ((LeftJoinPair)lit1.next()).element;
	    actList = ((LeftJoinPair)lit1.next()).elemList;
	    lit2 = actList.listIterator(0);
	    while (lit2.hasNext()) {
		actElem2 = (Element)lit2.next();
		//System.out.println("subtract: lit1:"+(lit1.nextIndex()-1)+", lit2:"+(lit2.nextIndex()-1));
		//if method has one argument
		if (paramTypeCount == 1) {
		    paramList[0] = actElem2; }
		//if method has two arguments
		else {
		    paramList[0] = actElem1;
		    paramList[1] = actElem2; }
		try {
		    if (metTypeElement) {
			retList.add((Element)(method.invoke(actElem1,paramList))); }
		    else if (metTypeElemList) {
			//System.out.println("Elemlist:"+method);
			retList.addAll((ElemList)(method.invoke(actElem1,paramList))); }
		    else {
			System.out.println("Error in SetOps.subtract: can't invoke method");
			System.exit(0);
		    }//else
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.out.println("Error in SetOps.subtract: Problem with using method.");
		    System.exit(0);
		}//catch
	    }//while
	}//while
	
	return retList;
    }//end method subtract



    public static ElemList overlapReduce (ElemList el, Method predicate, Method method, boolean ovLapPairsMeet) {
	//this is a variant of the operation reduce
	//it only works for predicates which hold if the bounding boxes of the elements overlap
	//additionally the result of the method must also lie inside of the former bb of the
	//elements to grant tc

	//ovLapPairsMeet is passed to overlappingPairs
	
	//it works as followes:
	//1. perform overlappingPairs
	//2. build a graph with vertices: elements and an edge exist between x,y if a pair
	//(x,y) exists in the result of 1.
	//3. compute the connected components of the graph
	//4. for each component do the following (recursively):
	//  - perform method on a pair (x,y)
	//  - replace pair by method(x,y)
	//  - call ovRed for this component
	
	//System.out.println("\nentering SO.overlapReduce(el.size():"+el.size()+")");
	//System.out.println("predicate:"+predicate);
	//System.out.println("method:"+method);
	//System.out.println("set of elements:"); el.print();
	ElemList retList = new ElemList();
	PairList pl = overlappingPairs(el,el,true,ovLapPairsMeet);

	//remove pairs(x,x) from pl
	/*
	Class c = (el.getFirst()).getClass();
	Class[] paramList = new Class[1];
	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("equal",paramList);
	    pl = filter(pl,m,false);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	*/
	pl = filter(pl,predicate,true);
	if (pl.isEmpty()) {
	    //System.out.println("\npl.isEmpty.");
	    return el; }
	Graph g = new Graph(el,pl);
	//System.out.println("graph:");
	//g.print();
	ConnectedComponentsPair ccp = g.connectedComponents();
	//System.out.println("\nccp.vertices:");
	//ccp.compVertices.print();
	//the pairs from ccE mustn't be computed all at once
	//so compute the set of pairs that may be computed
	ccp = g.computeReducedPair(ccp);
	//System.out.println("\nreducedPair:");
	//ccp.compEdges.print();
	//System.out.println("\nreducedPair(vertices):");
	//ccp.compVertices.print();

	ListIterator litE = ccp.compEdges.listIterator(0);
	ListIterator litV = ccp.compVertices.listIterator(0);
	PairList actPL;
	ElemList actEL;
	ElemList ml = new ElemList();
	while (litE.hasNext()) {
	    actPL = (PairList)litE.next();
	    actEL = (ElemList)litV.next();
	    //if there are no pairs, get the isolated elements from actEL
	    if (!actPL.isEmpty()) {
		ml = map(actPL,method); }
	    else { ml = new ElemList(); }
	    //System.out.println("\nml:"); ml.print();
	    //System.out.println("actEL:"); actEL.print();
	    //System.out.println("\nintersection:"); intersection(ml,actEL);
	    //ml = disjointUnion(ml,actEL);
	    retList.addAll(overlapReduce(disjointUnion(ml,actEL),predicate,method,ovLapPairsMeet));
	}//for i
	    
	//System.out.println("leaving SO.overlapReduce.");
	return rdup(retList);
    }//end method overlapReduce
	

    public static PairList filter (PairList plIn, Method predicate, boolean keep) {
	//comment missing
	//if keep is true, pairs (x,y) with predicate(x,y)=true are kept
	//else they are deleted
	
	/* use this for a clean version 
	   PairList pl = plIn.copy();
	*/
	//dirty version: destroys plIn
	PairList pl = plIn;
	//System.out.println("filter: plIn.size(): "+plIn.size());

	int paramTypeCount = Array.getLength(predicate.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	boolean predHolds = false;
	Iterator it = pl.listIterator(0);
	ElemPair actElem;
	while (it.hasNext()) {
	    actElem = (ElemPair)it.next();
	    //if predicate is non-static
	    if (paramTypeCount == 1) {
		paramList[0] = actElem.second;
	    }//if
	    //if predicate is static
	    else {
		paramList[0] = actElem.first;
		paramList[1] = actElem.second;
	    }//else
	    //boolean predHolds = false;
	    try {
		predHolds = (((Boolean)predicate.invoke(actElem.first,paramList)).booleanValue());
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	    if ((predHolds && !keep) ||
		(!predHolds && keep)) {	it.remove(); }
	    
	}//while
	return pl;
    }//end method filter


    public static ElemListList overlapGroup (ElemList el, Method predicate, boolean ovLapPairsMeet) {
	//this is a variant of the operation group
	//it only works for predicates which hold if the bounding boxes of
	//the elements overlap
	//this method has a much better tc than the ordinary group
	//UNTESTED
	//ovLapPairsMeet is passed to overlappingPairs

	//find the overlapping pairs of elements
	PairList pl = overlappingPairs(el,el,true,ovLapPairsMeet);
	//filter method: remove pairs for which predicate doesn't hold
	pl = filter(pl,predicate,true);
	//build a graph with vertices: elements and edges exist for pairs of elements
	Graph g = new Graph(el,pl);
	//compute the connected components
	ConnectedComponentsPair ccp = g.connectedComponents();
	ElemListList retList = ccp.compVertices;
	
	return retList;
    }//end method overlapGroup

    public static PairList overlapJoin (ElemList el1, ElemList el2, Method predicate, boolean ovLapPairsMeet) {
	//computes the JOIN on el1,el2 using predicate
	//predicate must be a method Element x Element -> boolean (static or dynamic)
	//In contrast to the ordinary JOIN-operation this one first computes
	//the pairs of overlapping elements of el1,el2 and then uses predicate
	//on the pairs.
	//Therefore it has a time complexity of O(nlogn) instead of O(n^2).
	//CAUTION: use this operation only if the predicate holds only for overlapping elements
	//if predicate holds also for non-overlapping elements, the result won't be computed correctly
	//ovLapPairsMeet is passed to overlappingPairs

	System.out.println("entering SO.overlapJoin...");
	long time1 = System.currentTimeMillis();
	PairList retList = overlappingPairs(el1,el2,false,ovLapPairsMeet);
	long time2 = System.currentTimeMillis();
	System.out.println("elapsed time (overlappingPairs"+el1.size()+","+el2.size()+"): "+(time2-time1)+"ms");
	System.out.println("retList has "+retList.size()+" elements");
	long time3 = System.currentTimeMillis();
	retList = filter(retList,predicate,true);
	long time4 = System.currentTimeMillis();
	System.out.println("elapsed time (filter): "+(time4-time3)+"ms");
	System.out.println("after filter retList has "+retList.size()+" elements");
	//System.out.println("retlist:");
	//quicksort(retList); //this may be removed
	//retList.print();
	//System.exit(0);
	System.out.println("leaving SO.overlapJoin...");
	return retList;
    }//end method overlapJoin


    public static ElemListList group (ElemList el, Method predicate) {
	//comment missing
	//predicate(static): Element x Element -> boolean
	//this method currently only supports static methods
	ElemListList retList = new ElemListList();
	Element[] paramList = new Element[2];
	boolean belongsToGroup = false;
	LinkedList indexList = new LinkedList();
	for (int i = 0; i < el.size(); i++) {
	    belongsToGroup = false;
	    indexList.clear();
	    //no list exists, so build a new one and add element
	    if (retList.isEmpty()) {
		//System.out.println("group:c1 -> build first group with element:");
		//((Element)el.get(i)).print();
		ElemList nl = new ElemList();
		nl.add(el.get(i));
		retList.add(nl.copy());
	    }//if
	    //there are already existing groups
	    else {
		for (int j = 0; j < retList.size(); j++) {
		    //current list is empty
		    //System.out.println("group:c2");
		    if (((ElemList)retList.get(j)).isEmpty()) {
			//System.out.println("group:c3 -> build new group with element");
			//((Element)el.get(i)).print();
			ElemList nl2 = new ElemList();
			nl2.add(el.get(i));
			retList.add(nl2.copy());
		    }//if
		    //current list is not empty
		    //check wether predicate holds for any element of the list
		    else {
			//System.out.println("group:c3");
			paramList[0] = (Element)el.get(i);
			for (int k = 0; k < ((ElemList)retList.get(j)).size(); k++) {
			    //System.out.println("group:c4 -> compare elements");
			    paramList[1] = (Element)((ElemList)retList.get(j)).get(k);
			    //System.out.println("Elements:");
			    //paramList[0].print();
			    //paramList[1].print();
			    try {
				belongsToGroup = ((Boolean)predicate.invoke(null,paramList)).booleanValue();
			    }//try
			    catch (Exception e) {
				System.out.println("Exception: "+e.getClass()+" ---"+e.getMessage());
				System.exit(0);
			    }//catch
			    if (belongsToGroup) {
				//the current element could belong to several groups
				//therefore the index j is stored
				//System.out.println("group:c5 -> belongs to group");
				//((ElemList)retList.get(j)).add(el.get(i));
				indexList.add(new Integer(j));
				belongsToGroup = false;
				break;
			    }//if
			}//for k
		    }//else
		    if (belongsToGroup || indexList.size() > 0) {
			//now merge all lists for which belongsToGroup is true
			//if this is true for only one list then add current element
			while (indexList.size() > 1)
			    {
				//merge last two lists
				ElemList tmpList1 = (ElemList)retList.get(((Integer)indexList.get(indexList.size()-2)).intValue());
				ElemList tmpList2 = (ElemList)retList.get(((Integer)indexList.get(indexList.size()-1)).intValue());
				tmpList1.addAll(tmpList2);
				retList.remove(((Integer)indexList.get(indexList.size()-1)).intValue());
				indexList.remove(indexList.size()-1);
			    }//do
			//now only one list is left, so add current element
			ElemList tmpList3 = (ElemList)retList.get(((Integer)indexList.getFirst()).intValue());
			tmpList3.add(el.get(i));
			belongsToGroup = true;
			break;
		    }//if
		}//for j
		if (!belongsToGroup) {
		    //System.out.println("group:c6 -> build new group");
		    //((Element)el.get(i)).print();
		    ElemList nl3 = new ElemList();
		    nl3.add(el.get(i));
		    retList.add(nl3.copy());
		    belongsToGroup = true;
		}//if
	    }//else
	}//for i
	//retList.print();

	return retList;
    }//end mehtod group

    public static ElemList map (LeftJoinPairList ljpl, Method mainMethod, Method predicate, Method secMethod) {
	//comment missing
	//CAUTION: this currently works only for one configuration:
	//mainMethod: Element x ElemList x predicate x secMethod -> ElemList
	//predicate(static): Element x Element -> Element
	//secMethod(static): Element x Element -> Element
	ElemList retList = new ElemList();
	Object[] paramListMM = new Object[4];
	//Element[] paramListPR = new Element[2];
	//Element[] paramListSM = new Element[2];
	ElemList paramList1 = new ElemList();

	//System.out.println("entering SO.map...");

	//System.out.println("LeftJoinPairList:");
	//ljpl.print();
	//System.exit(0);

	//paramListMM[2] = predicate;
	//paramListMM[3] = secMethod;
	
	ListIterator lit = ljpl.listIterator(0);
	LeftJoinPair actLjp;

	//for (int i = 0; i < ljpl.size(); i++) {
	while (lit.hasNext()) {
	    actLjp = (LeftJoinPair)lit.next();
	    //if second list is not empty do the computation
	    if (!actLjp.elemList.isEmpty()) {
		//System.out.println("\n++++++++++++++++++++++++++++++++++");
		//System.out.println("processing "+(lit.nextIndex()-1)+" of "+ljpl.size());
		try {
		    paramList1.clear();
		    paramList1.add(actLjp.element);
		    paramListMM[0] = paramList1;
		    paramListMM[1] = actLjp.elemList;
		    paramListMM[2] = predicate;
		    paramListMM[3] = secMethod;
		    
		    //System.out.println("invoke method on:");
		    //System.out.println("Element:");
		    //actLjp.element.print();
		    //System.out.println("ElemList:");
		    //actLjp.elemList.print();
		    retList.addAll((ElemList)mainMethod.invoke(null,paramListMM));
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.exit(0);
		}//catch
	    }//if
	    else { retList.add(actLjp.element); }
	}//for i
	
	//System.out.println("leaving map.");
	return retList;
    }//end method map


    public static ElemList proj1 (PairList pl) {
	//returns an ElemList of the first elements
	//of pl's tuples
	//CAUTION: We have a problem here. The proper type of
	//the first tuple-element must be retrieved and a new
	//list must be instantiated, but the classname is not
	//known at this point. Therefore the instantiation can
	//not be done.
	//SOLUTION???: Use a LinkedList and cast to ElemList!
	//...seems to be solved. Better check again!

	ElemList retList = new ElemList();

	for (int i = 0; i < pl.size(); i++) {
	    retList.add(((ElemPair)pl.get(i)).first);
	}//for i

	return retList;
    }//end method proj1
       

    public static boolean disjoint (ElemList el1, ElemList el2) {
	//returns TRUE if join(el1,el2,intersects()) is empty
	//returns FALSE otherwise
	
	Class c;
	Class [] paramList = new Class [1];
	PairList retList = new PairList();
	if (!el1.isEmpty()) {
	    c = el1.getFirst().getClass();
	}//if
	else {
	    if (!el2.isEmpty()) {
		c = el2.getFirst().getClass();
	    }//if
	    else { return true; }
	}//else

	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("intersects",paramList);
	    retList = join(el1,el2,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	if (retList.isEmpty()) { return true; }
	else { return false; }
    }//end method disjoint


    public static boolean equal (ElemList el1, ElemList el2) throws WrongTypeException {
	//returns TRUE if el1,el2 are equal
	//returns FALSE otherwise
	//note: equal((a,a,b), (a,b)) -> false

	//sort lists
	
	if (el1.size() != el2.size()) { return false; }
	//long time1 = System.currentTimeMillis();
	quicksortX(el1);
	quicksortX(el2);
	//long time2 = System.currentTimeMillis();
	//System.out.println("elapsed time for 2*quicksort:"+(time2-time1)+"ms");

	Iterator it1 = el1.listIterator(0);
	Iterator it2 = el2.listIterator(0);
	
	while(it1.hasNext()) {
	    if (!((Element)it1.next()).equal((Element)it2.next())) {
		return false;
	    }//if
	}//while
	
	/*
	for (int i = 0; i < el1.size(); i++) {
	    //System.out.println("scanning..."+i);
	    //((Element)el1.get(i)).print();
	    //((Element)el2.get(i)).print();
	    if (!(((Element)el1.get(i)).equal((Element)el2.get(i)))) {
		return false; }
	}//for i
	*/
	//System.out.println("leaving SO.equal");
	return true;
    }//end method equal

    
    public static LeftJoinPairList leftOuterJoin (ElemList el1, ElemList el2, Method predicate) {
	//returns a list of LeftJoinPairs which is the result of 
	//an operation that first performs a left outer join and
	//then groups by left elements
	//predicate must be a method Element x Element --> boolean
	//predicate may be a static or non-static method

	//CAUTION: WE NEED AN OVERLAP VERSION HERE, TOO!!!

	System.out.println("entering SO.leftOuterJoin...");

	int paramTypeCountPred = Array.getLength(predicate.getParameterTypes());
	LeftJoinPairList retList = new LeftJoinPairList();
	boolean isTrue = false;
	Element[] paramList = new Element[paramTypeCountPred];
	ListIterator lit1 = el1.listIterator(0);
	ListIterator lit2;
	Element actEl1;
	Element actEl2;

	while (lit1.hasNext()) {
	    actEl1 = (Element)lit1.next();
	    System.out.println("SO.loj: processing "+(lit1.nextIndex()-1));
	    LeftJoinPair actLjp = new LeftJoinPair();
	    actLjp.element = actEl1;
	    //actEl.elemList = ((ElemList)el2.copy());
	    actLjp.elemList = new ElemList();
	    //actEl.elemList.clear();//get an empty list
	    lit2 = el2.listIterator(0);
	    while (lit2.hasNext()) {
		//for (int j = 0; j < el2.size(); j++) {
		actEl2 = (Element)lit2.next();
		//isTrue = false;
		//if predicate has one argument
		if (paramTypeCountPred == 1) {
		    paramList[0] = actEl2;
		}//if
		//if predicate has two arguments (is static)
		else {
		    paramList[0] = actEl1;
		    paramList[1] = actEl2;
		}//else
		try {
		    isTrue = ((Boolean)predicate.invoke(actEl1,paramList)).booleanValue();
		    //System.out.println("SO.leftOuterJoin:invoked predicate on: "+isTrue);
		    //((Element)el1.get(i)).print();
		    //((Element)el2.get(i)).print();
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" ---"+e.getMessage());
		    System.exit(0);
		}//catch
		if (isTrue) {
		    actLjp.elemList.add(actEl2);
		}//if
	    }//for j
	    retList.add(actLjp);
	}//for i
	
	System.out.println("leaving leftOuterJoin.");
	return retList;
    }//end method leftOuterJoin


    public static ElemList subtract (ElemList el1, ElemList el2, Method predicate, Method  method) {
	//while two elements r,s (r e el1, s e el2) exist with predicate(r,s)==true
	//remove r from el1 and add method(r,s) to el1
	//return el1
	//predicate must be a method Element x Element --> boolean
	//predicate may be static or non-static
	//method must be a method Element x Element --> Element (of same type)

	//System.out.println("entering SO.subtract...");

	int paramTypeCountPred = Array.getLength(predicate.getParameterTypes());
	int paramTypeCountMet = Array.getLength(method.getParameterTypes());
	ElemList retList = (ElemList)el1.clone();
	boolean stillExist = false;
	boolean isTrue = false;
	Element[] paramListP = new Element[paramTypeCountPred];
	Element[] paramListM = new Element[paramTypeCountMet];
	//Element[] paramList = new Element[2];
	//SegList helpList = new SegList();
	ElemList mreturn = new ElemList();
	int count = 0;//DELETE THIS
	ListIterator lit1;
	ListIterator lit2;
	Element actEl1;
	Element actEl2;

	do {
	    stillExist = false;
	    //System.out.println("\nsize retList:"+retList.size()+", size el2:"+el2.size());
	    lit1 = retList.listIterator(0); 
	    //for (int i = 0; i < retList.size(); i++) {
	    while (lit1.hasNext()) {
		actEl1 = (Element)lit1.next();
		isTrue = false;
		lit2 = el2.listIterator(0);
		while (lit2.hasNext()) {
		    //for (int j = 0; j < el2.size(); j++) {
		    actEl2 = (Element)lit2.next();
		    System.out.println("i:"+(lit1.nextIndex()-1)+", j:"+(lit2.nextIndex()-1));
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
			System.out.println("checked for predicate..."+isTrue);
			actEl1.print();
			actEl2.print();
		    }//try
		    catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" ---"+e.getMessage());
			System.exit(0);
		    }//catch
		    if (isTrue) {
			//System.out.println("try to invoke method...");
			//System.out.println("...on following elements:");
			//((Element)retList.get(i)).print();
			//((Element)el2.get(j)).print();
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
			    mreturn = (ElemList)(method.invoke(actEl1,paramListM));
			    //retList.addAll((ElemList)(method.invoke((Element)retList.get(i),paramListM)));
			    //System.out.println("invoking successful.");
			}//try
			catch (Exception e) {
			    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			    System.exit(0);
			}//catch
			//System.out.println("method was invoked");
			if (isTrue) {
			    System.out.println("*****mreturn:");
			    mreturn.print();
			    System.out.println("*****mreturn-end");
			    int index = lit1.nextIndex()-1;
			    lit1.remove();
			    retList.addAll(mreturn);
			    //retList.remove(i);
			    stillExist = true;
			    //i = -1;
			    lit1 = retList.listIterator(index);
			    //j = el2.size();
			    break;
			}//if
			/*
			if (retList.isEmpty() || (i == retList.size())) {
			    stillExist = false;
			    break;
			}//if
			*/
			if (retList.isEmpty()) { break; }
			//stillExist = true;
			//isTrue = false;
		    }//if
		}//for j
		if (retList.isEmpty()) {
		    //System.out.println("retList is empty ----> break;");
		    break; }
	    }//for i
	    /*
	    System.exit(0);
	    count++;
	    if (count == 4) {
		System.out.println("\n***emergency exit***");
		System.exit(0);
	    }//if
	    */
	}//do
	while (stillExist);
	//while (stillExist && (retList.size() > 0));

	//System.out.println("leaving SO.subtract.");
	return retList;
    }//end method subtract


    public static ElemPair max (ElemList el1, ElemList el2, Method method) {
	//returns the pair r,s of elements from el1,el2 with method(r,s)
	//the maximum of all pairs
	//method must be a method Element x Element --> Rational

	Element elem1 = ((Element)el1.getFirst()).copy();
	Element elem2 = ((Element)el2.getFirst()).copy();
	Rational maximum = new Rational(0); // just for initialization
	Rational value = new Rational(0); //dto.
	//Element[] paramList = new Element[1];
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];

	//initialization
	//paramList[0] = elem2;
	try {
	    //if method has one argument
	    if (paramTypeCount ==1) {
		paramList[0] = (Element)el1.getFirst();
	    }//if
	    //if method has two arguments (is static)
	    else {
		paramList[0] = (Element)el1.getFirst();
		paramList[1] = (Element)el2.getFirst();
	    }//else
	    maximum = ((Rational)method.invoke(elem1,paramList)).copy();
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	for (int i = 0; i < el1.size(); i++) {
	    for (int j = 0; j < el2.size(); j++) {
		try {
		    //if method has one argument
		    if (paramTypeCount == 1) {
			paramList[0] = (Element)el2.get(j);
		    }//if
		    //if method has two arguments (is static)
		    else {
			paramList[0] = (Element)el1.get(i);
			paramList[1] = (Element)el2.get(j);
		    }//else
		    value = ((Rational)method.invoke(el1.get(i),paramList)).copy();
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.exit(0);
		}//catch

		if (value.greater(maximum)) {
		    maximum = value.copy();
		    elem1 = ((Element)el1.get(i)).copy();
		    elem2 = ((Element)el2.get(j)).copy();
		}//if
	    }//for j
	}//for i

	return new ElemPair(elem1,elem2);
    }//end method max



    public static ElemPair min (ElemList el1, ElemList el2, Method method) {
	//returns the pair r,s of elements from el1,el2 with method(r,s)
	//the minimum of all pairs
	//method must be a method Element x Element --> Rational
	
	Element elem1 = ((Element)el1.getFirst()).copy();
	Element elem2 = ((Element)el2.getFirst()).copy();
	Rational minimum = new Rational(100000); //just for initialization
	Rational value = new Rational(100000); //dto.
	//Element[] paramList = new Element[1];
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	
	//initialization
	try {
	    //if method has one argument
	    if (paramTypeCount == 1) {
		paramList[0] = (Element)el2.getFirst();
	    }//if
	    //if method has two arguments (is static)
	    else {
		paramList[0] = (Element)el1.getFirst();
		paramList[1] = (Element)el2.getFirst();
	    }//else
	    minimum = ((Rational)method.invoke(el1.getFirst(),paramList)).copy();
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	for (int i = 0; i < el1.size(); i++) {
	    for (int j = 0; j < el2.size(); j++) {
		//if method has one argument
		if (paramTypeCount == 1) {
		    paramList[0] = (Element)el2.get(j);
		}//if
		//if method has two arguments (is static)
		else {
		    paramList[0] = (Element)el1.get(i);
		    paramList[1] = (Element)el2.get(j);
		}//else
		try {
		    value = ((Rational)method.invoke(el1.get(i),paramList)).copy();
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.exit(0);
		}//catch
		
		if (value.less(minimum)) {
		    minimum = value.copy();
		    elem1 = ((Element)el1.get(i)).copy();
		    elem2 = ((Element)el2.get(j)).copy();
		}//if
	    }//for j
	}//for i
	
	return new ElemPair(elem1,elem2);
    }//end method min
	

    public static ElemList reduce (ElemList el, Method predicate, Method method) {
	//while two elements r,s exist with predicate(r,s)==true
	//remove both elements from el and add method(r,s)
	//return el
	//predicate must be a method Element x Element --> boolean
	//predicate may be static or dynamic
	//method must be a method Element x Element --> Element (of same type)
	//method can also be a method Element x Element --> ElemList
	//method is still static

	ElemList retList = el.copy();
	boolean stillExist;
	boolean isTrue;
	int paramTypeCountPred = Array.getLength(predicate.getParameterTypes());
	Element[] paramList = new Element[paramTypeCountPred];
	Element[] paramListMet = new Element[2];

	//System.out.println("entering SetOps.reduce");
	//System.out.println("el:"); el.print();
	//System.out.println("predicate:"+predicate);
	//System.out.println("method:"+method);
	
	//ListIterator it1 = retList.listIterator(0);
	//ListIterator it2;
	Element actElem1;
	Element actElem2;
	//these variables are used to determine wether the method
	//has return type Element or ElemList
	boolean metTypeElement = false;
	boolean metTypeElemList = false;
	//int numElems = 0; //number of added elements
	try {
	    if (method.getReturnType().isInstance(Class.forName("Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("ElemList")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("ElemList"))) {
		metTypeElemList = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.reduce: can't examine method.");
	    System.exit(0);
	}//catch
	
	/* new code, but still throws exception
	do {
	    stillExist = false;
	    while (it1.hasNext()) {
		isTrue = false;
		actElem1 = (Element)it1.next();
		it2 = retList.listIterator(1);
		while (it2.hasNext()) {
		    actElem2 = (Element)it2.next();
		    //if predicate has one argument
		    if (paramTypeCountPred == 1) {
			paramList[0] = actElem2;
		    }//if
		    //if predicate has two arguments (is static)
		    else {
			paramList[0] = actElem1;
			paramList[1] = actElem2;
		    }//else
		    try {
			isTrue = ((Boolean)predicate.invoke(actElem1,paramList)).booleanValue();
		    }//try
		    catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			System.out.println("Error in SetOps.reduce: Problem with using predicate.");
			System.exit(0);
		    }//catch
		    //now invoke method if isTrue
		    if (isTrue) {
			paramListMet[0] = actElem1;
			paramListMet[1] = actElem2;
			try {
			    //if method has returnType Element
			    if (metTypeElement) {
				//retList.add((Element)(method.invoke(null,paramListMet)));
				it1.add(method.invoke(null,paramListMet));
				numElems = 1;
			    }//if
			    else {
				if (metTypeElemList) {
				    //retList.addAll((ElemList)(method.invoke(null,paramListMet)));
				    ElemList mlist = (ElemList)method.invoke(null,paramListMet);
				    Iterator mit = mlist.listIterator();
				    while (mit.hasNext()) {
					it1.add(mit.next()); }
				    numElems = mlist.size();
				}//if
				else {
				    System.out.println("ERROR (SO.reduce): can't invoke method");
				    System.exit(0);
				}//else
			    }//else
			}//try
			catch (Exception e) {
			    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			    System.out.println("Error in SetOps.reduce: Problem with using method.");
			    System.exit(0);
			}//catch
			for (int i = 0; i < numElems+1; i++) {
			    it1.previous(); }
			it1.remove();
			it2.remove();
			stillExist = true;
			isTrue = false;
			if (it1.hasNext()) { actElem1 = (Element)it1.next(); }
			else {
			    stillExist = false;
			    break;
			}//else
		    }//if
		}//while it2
	    }//while it1
	}//do
	while (stillExist);
	
	return retList;
*/

	//old code, works but doesn't use iterators
	do {
	    stillExist = false;
	    isTrue = false;
	    for (int i = 0; i < retList.size()-1; i++) {
		actElem1 = (Element)retList.get(i);
		isTrue = false;
		for (int j = i+1; j < retList.size(); j++) {
		    actElem2 = (Element)retList.get(j);
		    //if predicate has one argument
		    if (paramTypeCountPred == 1) {
			paramList[0] = actElem2;
		    }//if
		    //if predicate has two arguments (is static)
		    else {
			paramList[0] = actElem1;
			paramList[1] = actElem2;
		    }//else
		    isTrue = false;
		    try {
			//System.out.println("\ntest elements("+i+","+j+"):");
			//actElem1.print();
			//actElem2.print();
			isTrue = ((Boolean)predicate.invoke(actElem1,paramList)).booleanValue();
			//System.out.println("isTrue:"+isTrue);
			
		    }//try
		    catch (Exception e) {
			System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			System.out.println("Error in SetOps.reduce: Problem with using predicate.");
			System.exit(0);
		    }//catch
		    
		    if (isTrue) {
			paramListMet[0] = actElem1;
			paramListMet[1] = actElem2;
			try {
			    //if method has returnType Element
			    if (metTypeElement) {
				retList.add((Element)(method.invoke(null,paramListMet)));
			    }//if
			    else {
				if (metTypeElemList) {
				    retList.addAll((ElemList)(method.invoke(null,paramListMet)));
				}//if
				else {
				    System.out.println("ERROR (SetOps.reduce): can't invoke method");
				    System.exit(0);
				}//else
			    }//else
			}//try
			catch (Exception e) {
			    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
			    System.out.println("Error in SetOps.reduce: Problem with using method.");
			    System.exit(0);
			}//catch
			retList.remove(j);
			retList.remove(i);
			//System.out.println("retList:"); retList.print();
			stillExist = true;
			isTrue = false;
			if (retList.size() > 0) {
			    i = 0;
			    actElem1 = (Element)retList.get(i);
			    if ((j-1) > 0) { j--; }
			    //if (j == 0) { j = 1; }
			}//if
			else { stillExist = false; }
		    }//if
		}//for j
	    }//for i
	}//do
	while (stillExist);

	//System.out.println("leaving SO.reduce.");
	return retList;
    }//end method reduce
	

    public static PairList join (ElemList el1, ElemList el2, Method predicate) {
	//computes the JOIN on el1,el2 using predicate
	//predicate must be a method Element,Element -> boolean (static or dynamic)
	//complexity n^2
	//System.out.println("entering SO.join...");
	PairList retList = new PairList();
	int paramTypeCount = Array.getLength(predicate.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	
	Iterator it1 = el1.listIterator(0);
	Iterator it2;
	Element actElem1;
	Element actElem2;
	
	while (it1.hasNext()) {
	    actElem1 = (Element)it1.next();
	    it2 = el2.listIterator(0);
	    while (it2.hasNext()) {
		actElem2 = (Element)it2.next();
		try {
		    //if predicate has one argument
		    if (paramTypeCount == 1) {
			paramList[0] = actElem2;
		    }//if
		    //if predicate has two arguments (is static)
		    else {
			paramList[0] = actElem1;
			paramList[1] = actElem2;
		    }//else
		    
		    if (((Boolean)predicate.invoke(actElem1,paramList)).booleanValue()) {
			retList.add(new ElemPair(actElem1,actElem2));
		    }//if
		}//try
		catch (Exception e) {
		    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		    System.exit(0);
		}//catch
	    }//while
	}//while
	//System.out.println("SO.join.retList("+retList.size()+"):"); //retList.print();
	//System.exit(0);
	//System.out.println("leaving SO.join.");
	
	return retList;
    }//end method join


    public static ElemList map (PairList pl, Method method) {
	//invokes m on every single tuple of pl and puts the
	//results in an ElemList
	//duplicates are removed afterwards
	//m must be a method: Element x Element -> Element

	//System.out.println("\nentering SO.map(pl,m)...");
	//System.out.println("method:"+method);
	//System.out.println("pl.size: "+pl.size());

	ElemList retList = new ElemList();
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	ListIterator lit = pl.listIterator(0);
	ElemPair actPair;
	boolean metTypeElement = false;
	boolean metTypeElemList = false;
	int count = 0; //DELETE THIS
	try {
	    if (method.getReturnType().isInstance(Class.forName("Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("ElemList")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("ElemList"))) {
		metTypeElemList = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.reduce: can't examine method.");
	    System.exit(0);
	}//catch
	
	while (lit.hasNext()) {
	    actPair = (ElemPair)lit.next();
	    //System.out.println("***** index: "+(lit.nextIndex()-1)+" *****");
	    count++;
	    try {
		//if method has one argument
		if (paramTypeCount == 1) {
		    //paramList[0] = ((ElemPair)pl.get(i)).second;
		    paramList[0] = actPair.second;
		}//if
		//if method has two arguments (is static)
		else {
		    //paramList[0] = ((ElemPair)pl.get(i)).first;
		    //paramList[1] = ((ElemPair)pl.get(i)).second;
		    paramList[0] = actPair.first;
		    paramList[1] = actPair.second;
		}//else
		//System.out.println("SO.map: trying to invoke method... on:");
		//actPair.first.print();
		//actPair.second.print();
		//if returntype of m is Element
		if (metTypeElement) {
		    retList.add((Element)(method.invoke(actPair.first,paramList)));
		    //retList.add((Element)(m.invoke(((ElemPair)pl.get(i)).first,paramList)));
		}//if
		//if returntype of m is ElemList
		else {
		    if (metTypeElemList) {
			retList.addAll((ElemList)(method.invoke(actPair.first,paramList)));
			//retList.addAll((ElemList)(m.invoke(((ElemPair)pl.get(i)).first,paramList)));
		    }//if
		    else {
			System.out.println("Error in SetOps.map): can't invoke method");
			System.exit(0);
		    }//else
		}//else
		//System.out.println("retList:");
		//retList.print();
		//System.exit(0);
		//System.out.println("SO.map: succesfully invoked method.");
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	    
	    //if (count == 4) {
	    //System.out.println("emergency exit!");
	    //System.exit(0); }
	   
	}//while 
	//System.out.println("leaving SO.map(pl,m).");
	//retList = rdup((ElemList)retList);
	return retList;
    }//end method map


    public static ElemList map (ElemList el, Method m) {
	//invokes m on every single element of el and removes
	//all doubles afterwards
	//m must be a method: Element -> Element
	//m can also be a method: Element -> ElemList
	//m can also be a method: Element -> Segment/Triangle/Point/etc
	//m can also be a method: Element -> SegList/TriList/PointList/etc
	//System.out.println("entering SO.map(el,m)...");
	//System.out.println("method: "+m);
	ElemList retList = new ElemList();
	boolean mRetTypeElem = false;
	boolean mRetTypeElemList = false;
	try {
	    mRetTypeElem = (m.getReturnType().isInstance(Class.forName("Element")) ||
			    m.getReturnType().getSuperclass().isAssignableFrom(Class.forName("Element")));
	    mRetTypeElemList = (m.getReturnType().isInstance(Class.forName("ElemList")) ||
				m.getReturnType().getSuperclass().isAssignableFrom(Class.forName("ElemList"))); 
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in SetOps.subtractSets: can't examine method.");
	    System.exit(0);
	}//catch

	ListIterator lit = el.listIterator(0);
	Element actEl;

	//for (int i = 0; i < el.size(); i++) {
	while (lit.hasNext()) {
	    actEl = (Element)lit.next();
	    try {
		//if returntype of m is Element
		if (mRetTypeElem) {
		    retList.add(m.invoke(actEl,null));
		}//if
		//if returntype of m is ElemList
		else {
		    if (mRetTypeElemList) {
			retList.addAll((ElemList)m.invoke(actEl,null));
		    }//if
		    else {
			System.out.println("ERROR (SetOps.map): can't invoke method");
			System.exit(0);
		    }//else
		}//else
		
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	}//while
	//System.out.println("leaving SO.map(el,m).");
	//retList = rdup(retList);
	return retList;
    }//end method map


    public static ElemList disjointUnion (ElemList el1, ElemList el2) {
	//computes the disjoint union of el1,el2
	//e.g. (a,b) (b,c) -> (a,b,b,c)
	ElemList eUnion = el1.copy();

	eUnion.addAll(el2);
	return eUnion;

    }//end method disjointUnion
	

    public static ElemList intersection (ElemList el1In, ElemList el2In) throws WrongTypeException {
	//computes the intersection of el1, el2
	//e.g. (a,b) (b,c) -> (b)
	ElemList eRet = el1In.copy();
	eRet.clear();
	ElemList el1 = el1In.copy();
	ElemList el2 = el2In.copy();
	int i = 0;
	int j = 0;
	int el1size = el1.size();
	int el2size = el2.size();

	if (el1.isEmpty() || el2.isEmpty()) { return eRet; }
	
	//sort and remove duplicates
	quicksortX(el1);
	el1 = rdup(el1);
	quicksortX(el1);
	el2 = rdup(el2);

	while ((i < el1size) && (j < el2.size())) {
	    Element act1 = (Element)el1.get(i);
	    Element act2 = (Element)el2.get(j);
	    if (act1.equal(act2)) {
		eRet.add(act1.copy());
		i++;
		j++;
	    }//if
	    else {
		byte comp = act1.compare(act2);
		//byte comp = act1.compX(act2);
		if (comp == -1) { i++; }
		else if (comp == 1) { j++; }
		else if (comp == 0) {
		    /*
		    byte compY = act1.compY(act2);
		    if (compY == -1) { i++; }
		    else if (compY == 1) { j++; }
		    else if (compY == 0) {
			System.out.println("really strange...");
			System.exit(0);
		    }//if
		    */
		}//if
		else {
		    System.out.print("Strange...");
		    System.exit(0);
		}//else 
	    }//else
	}//while
	
	return eRet;
    }//end method intersection


    public static ElemList difference (ElemList el1, ElemList el2) throws WrongTypeException {
	//computes the difference of el1, el2
	//e.g. (a,b,c) (a,b) -> (c)

	//System.out.println("\nentering SO.difference(el,el)...");
	//ElemList eDiff = el1.copy();
	//eDiff.clear();
	ElemList eDiff = new ElemList();
	
	//ElemList el1 = el1In.copy();
	//ElemList el2 = el2In.copy();
	//int i = 0;
	//int j = 0;
	int el1size = el1.size();
	int el2size= el2.size();

	quicksortX(el1);
	el1 = rdup(el1);
	quicksortX(el2);
	el2 = rdup(el2);
	
	if (el2.isEmpty()) { return el1; }

	Iterator it1 = el1.listIterator(0);
	Iterator it2 = el2.listIterator(0);
	boolean next1 = true;
	boolean next2 = true;
	Element elem1 = new Point();//just for initialization
	Element elem2 = new Point();//dito
	byte comp;
	
	while ((!next1 || it1.hasNext()) &&
	       (!next2 || it2.hasNext())) {
	    if (next1) elem1 = (Element)it1.next();
	    if (next2) elem2 = (Element)it2.next();
	    next1 = false;
	    next2 = false;
	    
	    if (elem1.equal(elem2)) {
		next1 = true;
		next2 = true;
	    }//if
	    else {
		//comp = elem1.compX(elem2);
		//if (comp == 0) { comp = elem1.compY(elem2); }
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
	    eDiff.add(((Element)it1.next()).copy());
	}//while
	//System.out.println("leaving SO.difference(el,el).");
	return eDiff;

	/*
	  //old code
	while ((i < el1size) && (j < el2.size())) {
	    Element act1 = (Element)el1.get(i);
	    Element act2 = (Element)el2.get(j);
	    if (act1.equal(act2)) {
		i++;
		j++;
	    }//if
	    else {
		byte comp = act1.compX(act2);
		if (comp == -1) {
		    eDiff.add(act1.copy());
		    i++;
		}//if
		else if (comp == 1) { j++; }//if
		else if (comp == 0) {
		    byte compY = act1.compY(act2);
		    if (compY == -1) {
			eDiff.add(act1.copy());
			i++;
		    }//if
		    else if (compY == 1) { j++; }
		    else if (compY == 0) {
			System.out.println("really strange...");
			System.exit(0);
		    }//if
		}//if
	    }//else
	}//while

	//System.out.println("i:"+i+", j:"+j+", el1size:"+el1size+", el1In.size:"+el1In.size());

	if (i < el1size) {
	    for (int k = i; k < el1size; k++) {
		//System.out.println("k:"+k);
		eDiff.add(((Element)el1.get(k)).copy());
	    }//for k
	}//if

	//System.out.println("diff.size()"+eDiff.size());

	return eDiff;
	*/
    }//end method difference


    public static ElemList rdup (ElemList elIn) throws WrongTypeException {
	//removes duplicates from elIn
	//e.g. (a,b,b,c) -> (a,b,c)
	//as a side-effect elIn is sorted afterwards
	
	//dirty version: destroys elIn
	if (elIn.isEmpty()) { return elIn; }
	quicksortX(elIn);
	ListIterator it = elIn.listIterator(0);
	Element act = (Element)it.next();
	Element next;
	while (it.hasNext()) {
	    next = (Element)it.next();
	    if(act.equal(next)) {
		it.remove(); }
	    else { act = next; }
	}//while
	return elIn;
    
		

	/* clean version: doesn't destroy elIn
	ElemList el = (ElemList)elIn.clone();
	ElemList elRet = new ElemList();
	
	if (el.isEmpty()) { return elRet; }

	quicksortX(el);

	Element act = (Element)el.getFirst();
	Element next;
	elRet.add(act.copy());
	
	Iterator it = el.listIterator(0);
	while (it.hasNext()) {
	    next = (Element)it.next();
	    if (!act.equal(next)) {
		elRet.add(next.copy());
		act = next;
	    }//if
	}//while

	//System.out.println("leaving SetOps.rdup.");
	return elRet;
	*/
    }//end method rdup


    public static PairList rdup (PairList plIn) throws WrongTypeException {
	//removes duplicates from plIn
	//e.g. ((a,b)(b,c)(a,b)(c,b)) -> ((a,b)(b,c))
	//System.out.println("entering SO.rdup...");
	PairList pl = plIn.copy();
	PairList plRet = new PairList();
	
	if (plIn.isEmpty()) { return pl; }
	
	quicksort(pl);
	ElemPair act = (ElemPair)pl.getFirst();
	plRet.add(act.copy());

	for (int i = 0; i < pl.size(); i++) {
	    if (!act.equalOrInvertedEqual((ElemPair)pl.get(i))) {
		act = ((ElemPair)pl.get(i)).copy();
		plRet.add(act.copy());
	    }//if
	}//for i
	//System.out.println("leaving SO.rdup");

	return plRet;
    }//end method rdup
		  

    public static ElemList union (ElemList el1, ElemList el2) throws WrongTypeException {
	//computes the union of el1,el2
	//e.g. (a,b) x (b,c) -> (a,b,c)
	ElemList eRet = new ElemList();

	eRet = rdup(disjointUnion(el1,el2));
	//eRet = disjointUnion(el1,el2);

	return eRet;
    }//end method union


    public static double sum (ElemList el, Method m) {
	//sums up the results of m used on every element of el
	//m must be a method: Element -> double
	double retSum = 0;
	try {
	    for (int i = 0; i < el.size(); i++) {
		retSum = retSum + ((Double)m.invoke(el.get(i),null)).doubleValue();
		//System.out.println("sum: "+retSum);
	    }//for i
	}// try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retSum;
    }//end method sum


  public static PairList overlappingPairs(ElemList el1, ElemList el2, boolean sameSet, boolean meet) {
      //generates a PairList of overlapping pairs from elements of el1 and el2
      //using a DAC-algorithm
      //
      //if sameSet=true, el1,el2 are the same set and the overlapping pairs
      //for only one set, i.e. the pairs with equal elements from el1,el2 are removed
      //
      //if meet = true, also those pairs are generated, which don't have overlapping, but
      //meeting bounding boxes
 
      //System.out.println("entering overlappingPairs...");

    PairList pairs = new PairList();

    if (el1.isEmpty() || el2.isEmpty()) return pairs;

    //generate vertical edge list from the elements' bounding boxes
    IvlList elvert = new IvlList();
    int counter = 0;
    Iterator it1 = el1.listIterator(0);
    Iterator it2 = el2.listIterator(0);
    Element actEl;
    while (it1.hasNext()) {
	actEl = (Element)it1.next();
	elvert.add(new Interval(
				actEl.rect().ll.y,
				actEl.rect().ul.y,
				"blueleft",
				actEl.rect().ul.x,
				actEl,
				counter));
	elvert.add(new Interval(
				actEl.rect().lr.y,
				actEl.rect().ur.y,
				"blueright",
				actEl.rect().ur.x,
				actEl,
				counter));
	counter++;
    }//while

    while (it2.hasNext()) {
	actEl = (Element)it2.next();
	elvert.add(new Interval(
				actEl.rect().ll.y,
				actEl.rect().ul.y,
				"greenleft",
				actEl.rect().ul.x,
				actEl,
				counter));
	elvert.add(new Interval(
				actEl.rect().lr.y,
				actEl.rect().ur.y,
				"greenright",
				actEl.rect().ur.x,
				actEl,
				counter));
	counter++;
    }//while

    //sort elvert regarding x
    //System.out.println("generated intervalLists...");

    //long time1 = System.currentTimeMillis();
    elvert.sort();
    //long time2 = System.currentTimeMillis();
    //System.out.println("sort1: "+(time2-time1)+"ms");
    //elvert.print();

    //System.out.println("sorted intervalLists...first");
    IvlList finalSort = new IvlList();
    if (meet) {
	//if meet = true
	//for every x-coordinate sort the existing intervals as follows:
	//first, take all left borders of bboxes which right partner also has
	//this (actual) x-coordinate
	//second, take all left borders of bboxes which have right borders that
	//have a x-coordinate greater than actual x
	//third, take all right borders of bboxes which have left borders that have
	//a x-coordinate smaller than actual x
	//fourth, take all right borders of bboxes which left partner also has
	//this (actual) x-coordinate
	
	//long time3 = System.currentTimeMillis();
	//IvlList finalSort = new IvlList();
	Interval actInt1;
	Interval actInt2;
	while(!elvert.isEmpty()) {
	    IvlList sortVert = new IvlList();
	    IvlList sameX = new IvlList();
	    Rational actX = ((Interval)elvert.getFirst()).x;
	    IvlList leftPartners = new IvlList();
	    IvlList rightPartners = new IvlList();
	    IvlList lonelyLeft = new IvlList();
	    //IvlList lonelyRight = new IvlList();
	    //collect all intervals with the same x value in sameX
	    while (!elvert.isEmpty() &&
		   ((Interval)elvert.getFirst()).x.equal(actX)) {
		sameX.add(((Interval)elvert.getFirst()));
		elvert.remove(0);
	    }//while
	    //find left ends in sameX
	    for (int i = 0; ((i < sameX.size()) && i > -1); i++) {
		actInt1 = (Interval)sameX.get(i);
		if (actInt1.mark == "blueleft" ||
		    actInt1.mark == "greenleft") {
		    boolean partnerExists = false;
		    //look for the partner of the actual interval
		    int saveIndJ = -1;
		    for (int j = 0; j < sameX.size(); j++) {
			actInt2 = (Interval)sameX.get(j);
			if ((actInt2.number == actInt1.number) &&
			    !(actInt2.mark == actInt1.mark)) {
			    partnerExists = true;
			    saveIndJ = j;
			    break;
			}//if
		    }//for
		    //if no partner exists, move interval to lonelyLeft
		    if (!partnerExists) {
			lonelyLeft.add(actInt1);
			sameX.remove(i);
			i--;
		    }//if
		    //if partner exists, move intervals to leftPartners, rightPartners resp.
		    else {
			leftPartners.add(actInt1);
			rightPartners.add(((Interval)sameX.get(saveIndJ)));
			if (saveIndJ > i) {
			    sameX.remove(saveIndJ);
			    sameX.remove(i);
			    i = i-2;
			}//if
			else {
			    sameX.remove(i);
			    sameX.remove(saveIndJ);
			    i = i-2;
			}//else
		    }//else
		}//if
	    }//for i

	    //now the remaining intervals in sameX are all right intervals
	    //without partner; simply add them
	    //construct list
	    sortVert.addAll(leftPartners);
	    sortVert.addAll(lonelyLeft);
	    sortVert.addAll(sameX); //these are lonelyRight
	    sortVert.addAll(rightPartners);
	    //now the intervals with actual x are sorted
	    //add this sequence to finalSort
	    finalSort.addAll(sortVert);
	}//while
    }//if meet
    else {
	//if meet = false
	//for every x-coordinate sort the existing intervals as follows:
	//first, take all right borders of bboxes which left partner has
	//a x-coordinate less than actual x
	//second, take all pairs of left and right borders which have the actual x
	//third, take all left borders of bboxes which right partner has
	//a x-coordinate greater than actual x
	//IvlList finalSort = new IvlList();
	Interval actInt1;
	Interval actInt2;
	while(!elvert.isEmpty()) {
	    IvlList sortVert = new IvlList();
	    IvlList sameX = new IvlList();
	    Rational actX = ((Interval)elvert.getFirst()).x;
	    IvlList lonelyRight = new IvlList();
	    IvlList leftPartners = new IvlList();
	    IvlList rightPartners = new IvlList();
	    
	    //collect all intervals with the same x value in sameX
	    while (!elvert.isEmpty() &&
		   ((Interval)elvert.getFirst()).x.equal(actX)) {
		sameX.add(((Interval)elvert.getFirst()));
		elvert.remove(0);
	    }//while
	    
	    //find right borders in sameX
	    for (int i = 0; ((i < sameX.size()) && i > -1); i++) {
		actInt1 = (Interval)sameX.get(i);
		if (actInt1.mark == "blueright" ||
		    actInt1.mark == "greenright") {
		    boolean partnerExists = false;
		    //look for the partner of the actual interval
		    int saveIndJ = -1;
		    for (int j = 0; j < sameX.size(); j++) {
			actInt2 = (Interval)sameX.get(j);
			if ((actInt2.number == actInt1.number) &&
			    !(actInt2.mark == actInt1.mark)) {
			    partnerExists = true;
			    saveIndJ = j;
			    break;
			}//if
		    }//for j
		    
		    //if no partner exists, move interval to lonelyRight
		    if (!partnerExists) {
			lonelyRight.add(actInt1);
			sameX.remove(i);
			i--;
		    }//if

		    //if partner exists, move intervals to leftPartners, rightPartners resp.
		    else {
			leftPartners.add(((Interval)sameX.get(saveIndJ)));
			rightPartners.add(actInt1);
			if (saveIndJ > i) {
			    sameX.remove(saveIndJ);
			    sameX.remove(i);
			    i = i-2;
			}//if
			else {
			    sameX.remove(i);
			    sameX.remove(saveIndJ);
			    i = i-2;
			}//else
		    }//else
		}//if
	    }//for i
	    
	    //now the remaining intervals in sameX are all left intervals
	    //without partners; simply add them
	    //construct list
	    sortVert.addAll(lonelyRight);
	    for (int k = 0; k < leftPartners.size(); k++) {
		sortVert.add(leftPartners.get(k));
		sortVert.add(rightPartners.get(k));
	    }//for k
	    sortVert.addAll(sameX); //these are lonelyLeft
	    //now the intevals with actual x are sorted
	    //add this sequence to finalSort
	    finalSort.addAll(sortVert);
	}//while
    }//else



    //write list back
    //elvert = IvlList.copy(finalSort);
    elvert = finalSort;
    //long time4 = System.currentTimeMillis();
    //System.out.println("sort2: "+(time4-time3)+"ms");
    
    //System.out.println("sorted intervalLists...second");

    //everything is done so start the DAC
    ResultList inlist = new ResultList();
    inlist.m = elvert;
    
    //initialize an structure for storing the already found intersections
    LinkedList[] intStore = new LinkedList[counter];
    //System.out.println("counter:"+counter+", intStore.length:"+intStore.length);
    for (int i = 0; i < intStore.length; i++) {
	intStore[i] = new LinkedList(); }

    //System.out.println("compute overlaps...");
    //System.out.println("el1.size:"+el1.size()+", el2.size:"+el2.size()+", elvert.size:"+elvert.size());
    //long time10 = System.currentTimeMillis();
    ResultList rl = computeOverlaps(intStore,inlist,sameSet,el1.size());
    //long time11 = System.currentTimeMillis();
    //System.out.println("elapsed time (computeOverlaps): "+(time11-time10)+"ms");
    
    //System.out.println("leaving SO.overlappingPairs.");
  return rl.pairs;
}//end method overlappingPairs


    private static ResultList computeOverlaps(LinkedList[] intStore, ResultList il, boolean sameSet, int size) {
	//this is the DAC main function
	//initially il.m is the full intervall list
	ResultList rl = new ResultList();
	
	//is m small enough?
	if (il.m.size() == 1) {
	    rl.pairs = il.pairs;
	    rl.m = il.m;
	    //dependent on the only element in m compute the sets
	    //blue, green, blueLeft, blueRight, greenLeft, greenRight
	    if (((Interval)il.m.getFirst()).mark == "blueleft") {
		//ResultList rl = new ResultList();
		rl.blue.add((Interval)il.m.getFirst());
		rl.green = il.green;
		rl.blueLeft.add((Interval)il.m.getFirst());
		rl.blueRight = il.blueRight;
		rl.greenLeft = il.greenLeft;
		rl.greenRight = il.greenRight;
	    }//if
	    if (((Interval)il.m.getFirst()).mark == "blueright") {
		//ResultList rl = new ResultList();
		rl.blue.add((Interval)il.m.getFirst());
		rl.green = il.green;
		rl.blueLeft = il.blueLeft;
		rl.blueRight.add((Interval)il.m.getFirst());
		rl.greenLeft = il.greenLeft;
		rl.greenRight = il.greenRight;
	    }//if
	    if (((Interval)il.m.getFirst()).mark == "greenleft") {
		//ResultList rl = new ResultList();
		rl.blue = il.blue;
		rl.green.add((Interval)il.m.getFirst());;
		rl.blueLeft = il.blueLeft;
		rl.blueRight = il.blueRight;
		rl.greenLeft.add((Interval)il.m.getFirst());
		rl.greenRight = il.greenRight;
	    }//if
	    if (((Interval)il.m.getFirst()).mark == "greenright") {
		//ResultList rl = new ResultList();
		rl.blue = il.blue;
		rl.green.add((Interval)il.m.getFirst());
		rl.blueLeft = il.blueLeft;
		rl.blueRight = il.blueRight;
		rl.greenLeft = il.greenLeft;
		rl.greenRight.add((Interval)il.m.getFirst());
	    }//if
	}//if size() == 1
	
	if (il.m.size() > 1) {
	    //DIVIDE
	    IvlList m1 = new IvlList();
	    ListIterator lit = il.m.listIterator(0);
	    Interval actEl;
	    int mhalf = il.m.size()/2;
	    //for (int i = 0; i < ((int)il.m.size() / 2); i++){
	    while ((lit.nextIndex() < mhalf) && (lit.hasNext())) {
		actEl = (Interval)lit.next();
		//m1.add(il.m.get(i));
		m1.add(actEl);
	    }//for i
	    IvlList m2 = new IvlList();
	    //for (int i = ((int)il.m.size() /2); i < il.m.size(); i++) {
	    while (lit.hasNext()) {
		actEl = (Interval)lit.next();
		//m2.add(il.m.get(i));
		m2.add(actEl);
	    }//for i
	   
 
	    //CONQUER
	    ResultList rl1 =
		computeOverlaps(intStore,
				new ResultList(il.pairs, m1,
					       il.blue, il.green,
					       il.blueLeft, il.blueRight,
					       il.greenLeft, il.greenRight), sameSet, size);
	    ResultList rl2 =
		computeOverlaps(intStore,
				new ResultList(il.pairs, m2,
					       il.blue, il.green,
					       il.blueLeft, il.blueRight,
					       il.greenLeft, il.greenRight), sameSet, size);
	    
	    //MERGE
	    IvlList fullBlue;// = new IvlList();
	    IvlList fullGreen;// = new IvlList();
	    
	    fullBlue = IvlList.intersect(rl1.blueLeft,rl2.blueRight);
	    fullGreen = IvlList.intersect(rl1.greenLeft,rl2.greenRight);
	    
	    rl.pairs.addAll(rl1.pairs);
	    rl.pairs.addAll(rl2.pairs);
	    rl.blue = IvlList.merge(rl1.blue,rl2.blue);
	    rl.green = IvlList.merge(rl1.green,rl2.green);
	    
	    rl.blueLeft = IvlList.merge(IvlList.minus(rl1.blueLeft,fullBlue),rl2.blueLeft);
	    rl.blueRight = IvlList.merge(IvlList.minus(rl2.blueRight,fullBlue),rl1.blueRight);
	    rl.greenLeft = IvlList.merge(IvlList.minus(rl1.greenLeft,fullGreen),rl2.greenLeft);
	    rl.greenRight = IvlList.merge(IvlList.minus(rl2.greenRight,fullGreen),rl1.greenRight);
      
	    //compute pairs
	    rl.pairs.addAll(IvlList.overlappingIntervals(intStore,sameSet,size,(IvlList.minus(rl1.blueLeft,fullBlue)), rl2.green));
	    //rl.pairs.addAll(IvlList.overlappingIntervals(intStore,sameSet,size,(IvlList.minus(rl1.greenLeft,fullGreen)), rl2.blue));
	    rl.pairs.addAll(IvlList.overlappingIntervals(intStore,sameSet,size,rl2.blue,(IvlList.minus(rl1.greenLeft,fullGreen))));
	    rl.pairs.addAll(IvlList.overlappingIntervals(intStore,sameSet,size,(IvlList.minus(rl2.blueRight,fullBlue)), rl1.green));
	    //rl.pairs.addAll(IvlList.overlappingIntervals(intStore,sameSet,size,(IvlList.minus(rl2.greenRight,fullGreen)), rl1.blue));
	    rl.pairs.addAll(IvlList.overlappingIntervals(intStore,sameSet,size,rl1.blue,(IvlList.minus(rl2.greenRight,fullGreen))));
    }//if size() > 1
    
    return rl;
  }//end method compute overlaps
  

    public static void mergesortX (ElemList el) {
	//array version
	
	//copy list to an array
	Object[] elArr = new Element[el.size()];
	elArr = el.toArray();
	mergesX(elArr,0,elArr.length);
	//copy back
	el.clear();
	for (int i = 0; i < elArr.length; i++) {
	    el.add(elArr[i]);
	}//for i
    }//end method mergesortX

    private static void mergesX (Object[] elArr, int lo, int hi) {
	int m;
	if (lo < hi) {
	    m = (lo+hi)/2;
	    mergesX(elArr,lo,m);
	    mergesX(elArr,m+1,hi);
	    mergeX(elArr,lo,hi);
	}//if
    }//end method mergesX

    private static void mergeX (Object[] elArr,int lo, int hi) { 
	byte erg;
	int m = (lo+hi)/2;
	int pos1 = lo;
	int pos2 = m;

	while ((pos1 < m) && (pos2 < hi)) {
	    erg = (((Element)elArr[pos1]).compare((Element)elArr[pos2]));
	    //erg = (((Element)elArr[pos1]).compX((Element)elArr[pos2]));
	    //if (erg == 0) { erg = (((Element)elArr[pos1]).compY((Element)elArr[2])); }
	    switch (erg) {
	    case -1 : {
		pos1++;
		break; }
	    case 1 : {
		Element help = ((Element)elArr[pos1]).copy();
		elArr[pos1] = elArr[pos2];
		elArr[pos2] = help;
		pos1++;
		break; }
	    case 0 : pos1++;
	    }//switch
	}//while
    }//end method mergeX

    /*
    public static void mergesortX (ElemList el) {
	//simply calls mergesX
	//ISMAIL, but this was corrected by me and works properly!
	//first sorts by x-coordinates and for equal x-coordinates
	//sorting is _ascending_ for y-coordinates
	mergesX(el,0,el.size()-1);
    }//end method mergesortX


    private static void mergesX(ElemList list, int lo, int hi) {
      //performs a mergesort on list
      //lo,hi are the indices to determine which part of the list should be sorted
	//ISMAIL
      int m;
      
      if (lo < hi) {
	  m = (lo + hi) / 2;
	  mergesX (list, lo, m);
	  mergesX (list, m+1, hi);
	  mergeX (list, lo, hi);
      }//if
  
  }//end method mergesX

    
  private static void mergeX(ElemList list, int lo, int hi) {
      //supportive method for mergesortX
      //ISMAIL
      
      //System.out.println("SO.mergeX calling...");
      int i, j, k, m, n = hi-lo+1;
      Element [] tmp = new Element[n];// temporary array
      byte erg;
      
      //System.out.println("check1.0");
      k = 0;
      m = (lo+hi)/2;
      //copy lower half to array b
      for (i = lo; i <= m; i++)
	  tmp[k++] = (Element)list.get(i);
      //copy upper half in reverse order to array b
      for (j = hi; j >= m+1; j--)
	  tmp[k++] = (Element)list.get(j);
      
      //System.out.println("check2.0");
      i = 0;
      j = n-1;
      k = lo;
      //int tmpX = 0; //delete this!!!

      //copy the next greatest element back until i,j meet
      while (i <= j) {
	  //System.out.print("i["+i+"]: "+tmp[i]+", j["+j+"]: "+tmp[j]+", k:"+k);
	  //tmpX++; //delete this!!! 
	  //if (tmpX==20) { System.exit(0); } //delete this!!!
	  erg = tmp[i].compX(tmp[j]); //System.out.println(" erg: "+erg);
	  if (erg == 0) {
	      erg = tmp[i].compY(tmp[j]);
	  }//if
	  
	  switch (erg) {
	      case -1 : list.set(k++,(Element)tmp[i++]); break;
	      case 0 : list.set(k++,(Element)tmp[i++]); break;
	      case 1 : list.set(k++,(Element)tmp[j--]);
	  }//switch
	 
      }//while
      //System.out.println("SO.mergeX exit.");
  }//end method mergeX
    */

    public static void mergesortXY (ElemList el) {
	//simply calls mergesX
	//first sorts by x-coordinates and for equal x-coordinate
	//sorting is _descending_ for y-coordinates
	mergesXY(el,0,el.size()-1);
    }//end method mergesortXY

    private static void mergesXY (ElemList list, int lo, int hi) {
	//performs a mergesort on list
	//lo,hi are indices to determine which part of the list shoult be sorted
	int m;
	if (lo < hi) {
	    m = (lo + hi) / 2;
	    mergesXY (list,lo,m);
	    mergesXY (list,m+1,hi);
	    mergeXY (list,lo,hi);
	}//if
    }//end method mergesXY

    
    private static void mergeXY(ElemList list, int lo, int hi) {
	//supportive method for mergesortXY
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


    public static void mergesort (PairList pl) {
	//simply calls mergesX
	//first sorts by the first element and then by the second
	merges(pl,0,pl.size()-1);
    }//end method mergesort

    private static void merges (PairList list, int lo, int hi) {
	//performs a mergesort on list
	//lo,hi are the indices to determine which part of the list should be sorted
	int m;
	if (lo < hi) {
	    m = (lo + hi) / 2;
	    merges(list,lo,m);
	    merges(list,m+1,hi);
	    merge(list,lo,hi);
	}//if
    }//end method merges

    
    private static void merge(PairList list, int lo, int hi) {
	//supportive method for mergesort
	//System.out.println("entering SO.merge...");
	int i, j, k, m, n = hi-lo+1;
	ElemPair[] tmp = new ElemPair[n];//temporary array
	byte erg;
	
	k = 0;
	m = (lo+hi)/2;
	//copy lower half to array b
	for (i = lo; i <= m; i++) {
	    tmp[k++] = (ElemPair)list.get(i); }
	for (j = hi; j >= m+1; j--) {
	    tmp[k++] = (ElemPair)list.get(j); }
	i = 0;
	j = n-1;
	k = lo;
	
	//copy the next greatest ElemPair back until i,j meet
	while (i <= j) {
	    Element el1 = tmp[i].first;
	    Element el2 = tmp[j].first;
	    erg = ((Element)tmp[i].first).compX((Element)tmp[j].first);
	    if (erg == 0) {
		erg = tmp[i].first.compY(tmp[j].first);
		if (erg == 0) {
		    erg = tmp[i].second.compX(tmp[j].second);
		    if (erg == 0) {
			erg = tmp[i].second.compY(tmp[j].second);
		    }//if
		}//if
	    }//if
	    
	    switch (erg) {
	    case -1 : list.set(k++,(ElemPair)tmp[i++]); break;
	    case 0 : list.set(k++,(ElemPair)tmp[i++]); break;
	    case 1 : list.set(k++,(ElemPair)tmp[j--]);
	    }//switch
	}//while
	//System.out.println("leaving SO.merge.");
    }//end method merge


    public static void quicksortX (ElemList el) {
	//there seems to be an error in findX - it's patched by changing
	//!(k == 0) to !(k == -1) in quickX...
	//array version
	Object[] elArr = new Element[el.size()];
	elArr = el.toArray();
	quickX(elArr,0,elArr.length-1);
	//copy back
	el.clear();
	for (int i = 0; i < elArr.length; i++) {
	    el.add(elArr[i]);
	}//for i
	//System.out.println("leaving SO.quicksortX...");
    }//end method quicksortX

    private static void quickX (Object[] elArr, int lo, int hi) {
	//supportive method for quicksortX
	int i = lo;
	int j = hi;
	int k = -1;
	Element kElem;
	Object[] findkRes;// = new Object[2];
	
	if (lo < hi) {
	    findkRes = findPivotX(elArr,i,j);
	    k = ((Integer)findkRes[0]).intValue();
	    kElem = (Element)findkRes[1];
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
	while ((k <= j) && (((Element)elArr[k]).equal((Element)elArr[k-1]))) {
	    k++; } //while
	if (k > j) {
	    retArr[0] = new Integer(-1);
	    retArr[1] = (Element)elArr[0];
	    return retArr;
	}//if
	else {
	    res = ((Element)elArr[k]).compare((Element)elArr[k-1]);
	    //res = ((Element)elArr[k]).compX((Element)elArr[k-1]);
	    //if (res == 0) { res = ((Element)elArr[k]).compY((Element)elArr[k-1]); }
	    if (res == 1) {
		retArr[0] = new Integer(k);
		retArr[1] = (Element)elArr[k];
		return retArr;
	    }//if
	    else {
		retArr[0] = new Integer(k-1);
		retArr[1] = (Element)elArr[k-1];
		return retArr;
	    }//else
	}//else
    }//end method findPivotX
	
    private static int partitionX (Object[] elArr, int i, int j, Element x) {
	//supportive method for quicksortX
	int l = i;
	int r = j;
	Object tmp;
	byte res;
	boolean found = false;
	while (true) {
	    do {
		found = false;
		res = ((Element)elArr[l]).compare(x);
		//res = ((Element)elArr[l]).compX(x);
		//if (res == 0) { res = ((Element)elArr[l]).compY(x); }
		if (res == -1) { l++; }
		else { found = true; }
	    } while (!found);
	    //while ((Element)elArr[l].compX(x) == -1) { l++; }
	    do {
		found = false;
		res = ((Element)elArr[r]).compare(x);
		//res = ((Element)elArr[r]).compX(x);
		//if (res == 0) { res = ((Element)elArr[r]).compY(x); }
		if (res != -1) { r--; }
		else { found = true; }
	    } while (!found);
	    //while ((Element)elArr[r].compX(x) != -1) { r--; }
	    if (l > r) { break; }
	    //tmp = ((Element)elArr[l]).copy();
	    tmp = elArr[l];
	    elArr[l] = elArr[r];
	    elArr[r] = tmp;
	}//while
	return l;
    }//end method partitionX
	    
    
    public static void quicksort (PairList pl) {
	//array version
	//quicksort for Pairlist
	Object[] plArr = new Element[pl.size()];
	plArr = pl.toArray();
	quick(plArr,0,plArr.length-1);
	//copy back
	pl.clear();
	for (int i = 0; i < plArr.length; i++) {
	    pl.add(plArr[i]);
	}//for i
    }//end method quicksort

    private static void quick (Object[] plArr, int lo, int hi) {
	//supportive method for quicksort
	int i = lo;
	int j = hi;
	int k = -1;
	ElemPair kElem;
	Object[] findkRes = new Object[2];
	
	if (lo < hi) {
	    findkRes = findPivot(plArr,i,j);
	    k = ((Integer)findkRes[0]).intValue();
	    kElem = (ElemPair)findkRes[1];
	    if (k != -1) {
		k = partition(plArr,i,j,kElem); //divide
		quick(plArr,i,k-1); //conquer
		quick(plArr,k,j); //conquer
	    }//if
	}//if
    }//end method quick

    private static Object[] findPivot (Object[] plArr, int i, int j) {
	//this is for the Pairlist-version
	//find index of not minimal element in array i,j
	//if exists
	//0 otherwise
	Object[] retArr = new Object[2];
	byte res;
	int k = 0;
	k = i+1;
	while ((k <= j) && (((ElemPair)plArr[k]).equal((ElemPair)plArr[k-1]))) {
	    k++; }//while
	if (k > j) {
	    retArr[0] = new Integer(-1);
	    retArr[1] = (ElemPair)plArr[0];
	    return retArr;
	}//if
	else {
	    res = ((ElemPair)plArr[k]).compare((ElemPair)plArr[k-1]);
	    //res = ((ElemPair)plArr[k]).compX((ElemPair)plArr[k-1]);
	    //if (res == 0) { res = ((ElemPair)plArr[k]).compY((ElemPair)plArr[k-1]); }
	    if (res == 1) {
		retArr[0] = new Integer(k);
		retArr[1] = (ElemPair)plArr[k];
		return retArr;
	    }//if
	    else {
		retArr[0] = new Integer(k-1);
		retArr[1] = (ElemPair)plArr[k-1];
		return retArr;
	    }//else
	}//else
    }//end method findPivot

    private static int partition (Object[] plArr, int i, int j, ElemPair x) {
	//supportive method for quicksort
	//Pairlist version
	int l = i;
	int r = j;
	ElemPair tmp;
	byte res;
	boolean found = false;
	while (true) {
	    do {
		found = false;
		res = ((ElemPair)plArr[l]).compare(x);
		//res = ((ElemPair)plArr[l]).compX(x);
		//if (res == 0) { res = ((ElemPair)plArr[l]).compY(x); }
		if (res == -1) { l++; }
		else { found = true; }
	    } while (!found);
	    do {
		found = false;
		res = ((ElemPair)plArr[r]).compare(x);
		//res = ((ElemPair)plArr[r]).compX(x);
		//if (res == 0) { res = ((ElemPair)plArr[r]).compY(x); }
		if (res != -1) { r--; }
		else { found = true; }
	    } while (!found);
	    if (l > r) { break; }
	    tmp = ((ElemPair)plArr[l]).copy();
	    plArr[l] = plArr[r];
	    plArr[r] = tmp;
	}//while
	return l;
    }//end method partition


    protected static PairList difference (PairList pl1In, PairList pl2In) {
	//returns the difference of two PairLists
	//CAUTION: the elements of the pairs are swapped
	
	System.out.println("entering SO.difference(pl,pl)...");

	PairList retList = new PairList();
	PairList pl1 = pl1In.copy();
	PairList pl2 = pl2In.copy();
	int i = 0;
	int j = 0;
	int pl1size = pl1.size();
	int pl2size = pl2.size();

	pl1.twistElements();
	pl2.twistElements();
	pl1 = rdup(pl1);
	pl2 = rdup(pl2);
	System.out.println("pl1.size:"+pl1.size());
	System.out.println("pl2.size:"+pl2.size());
	
	if (pl2.isEmpty()) { return pl1; }

	while ((i < pl1size) && (j < pl2.size())) {
	    ElemPair act1 = (ElemPair)pl1.get(i);
	    ElemPair act2 = (ElemPair)pl2.get(j);
	    if (act1.equal(act2)) {
		i++;
		j++;
	    }//if
	    else {
		byte comp = act1.first.compare(act2.first);
		//byte comp = act1.first.compX(act2.first);
		if (comp == 0) comp = act1.second.compare(act2.second);
		/*
		if (comp == 0) {
		    comp = act1.first.compY(act2.first);
		    if (comp == 0) {
			comp = act1.second.compX(act2.second);
			if (comp == 0) {
			    comp = act1.second.compY(act2.second);
			}//if
		    }//if
		}//if
		*/
		switch (comp) {
		case -1 : { retList.add(act1.copy()); i++; break; }
		case 1 : { j++; }
		}//switch
	    }//else
	}//while


	if (i < pl1size) {
	    for (int k = i; k < pl1size; k++) {
		retList.add(((ElemPair)pl1.get(k)).copy());
	    }//for k
	}//if
	System.out.println("leaving SO.difference(pl,pl.)");
	return retList;
    }//end method difference


    public static ElemList elementsInPairList (PairList pl) {
	//returns a list of all Elements that are in pl
	ElemList retList = new ElemList();
	Iterator it = pl.listIterator();
	ElemPair actElem;
	//System.out.println("entering SO.eIP...");
	while (it.hasNext()) {
	    actElem = (ElemPair)it.next();
	    retList.add(actElem.first);
	    retList.add(actElem.second);
	}//while
	//retList = rdup(retList);
	//System.out.println("leaving SO.eIP.");
	return retList;
    }//end method elementsInPairList

}//class SetOps
  






