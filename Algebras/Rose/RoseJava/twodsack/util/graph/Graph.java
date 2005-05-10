package twodsack.util.graph;

import twodsack.operation.setoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.NoSuchElementException;

public class Graph {
    //implements an undirected graph
    //the graph's vertices are elements, therefore v is of type ElemMultiSet

    //CAUTION: this class doesn't use Iterators and has some O(n2) algorithms
    //v,e eventually should be implemented as arrays!

    //the construction of the succLists can be improved as seen in MeshGenerator
    //where the pointlist and segmentlist are constructed from a SegMultiSet

    //the set of edges is not needed anywhere in this class. Therefore, they 
    //are not computed and stored anymore.

    //members
    static final int NUMBER_OF_BUCKETS = 499;

    private ElemMultiSet v; //vertices
    //private PairMultiSet e; //edges
    private Vertex[] vArr; //array of vertices
    private LinkedList[] succLists; //lists of successors for every vertex
    

    //constructors
    public Graph() {
	v = new ElemMultiSet(new ElemComparator());
	//e = new PairMultiSet(new ElemPairComparator());
	vArr = new Vertex[0];
	succLists = new LinkedList[0];
    }
 

    public Graph(SegMultiSet edges) {
	//constructs a graph from a set of segments
	//use PointLink and hashing to construct succLists
	
	double tt01 = System.currentTimeMillis();
	ElemMultiSet cleanList = makeCleanList(edges);
	double tt02 = System.currentTimeMillis();
	//System.out.println("G.makeCleanList takes "+(tt02-tt01)+"ms for "+edges.size()+" elements.");

	int initialCapacity = NUMBER_OF_BUCKETS;
	Hashtable pointsHT = new Hashtable(initialCapacity);
	
	//fill in the segments points as pointers to original points
	Iterator eit = cleanList.iterator();
	PointLink htEntry;
	Segment actSeg;
	int counter = 0;
	while (eit.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)eit.next()).value;
	    htEntry = new PointLink(actSeg.getStartpoint());
	    //check whether htEntry already exists in hash table
	    //if not, give it a new number and insert it
	    if (!pointsHT.containsKey(htEntry.linkedPoint)) {
		htEntry.number = counter;
		counter++;
		pointsHT.put(htEntry.linkedPoint,htEntry);
	    }//if
	    
	    //do the same with endpoint
	    htEntry = new PointLink(actSeg.getEndpoint());
	    if (!pointsHT.containsKey(htEntry.linkedPoint)) {
		htEntry.number = counter;
		counter++;
		pointsHT.put(htEntry.linkedPoint,htEntry);
	    }//if
	}//while eit

	//construct list of vertices from hashtable
	Enumeration enum = pointsHT.elements();
	vArr = new Vertex[counter];
	Object elem;
	while (enum.hasMoreElements()) {
	    elem = enum.nextElement();
	    vArr[((PointLink)elem).number] = new Vertex(((PointLink)elem).linkedPoint,((PointLink)elem).number);
	}//while enum
	
	//construct succLists
	succLists = new LinkedList[vArr.length];
	for (int i = 0; i < succLists.length; i++) succLists[i] = new LinkedList();
	eit = cleanList.iterator();
	int pointPosS;
	int pointPosE;
	while (eit.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)eit.next()).value;
	    //get the numbers of the segments endpoints
	    pointPosS = ((PointLink)pointsHT.get(actSeg.getStartpoint())).number;
	    pointPosE = ((PointLink)pointsHT.get(actSeg.getEndpoint())).number;
	    //set the proper vertices in succLists
	    succLists[pointPosS].add(vArr[pointPosE]);
	    succLists[pointPosE].add(vArr[pointPosS]);
	}//while eit
    }

    public Graph(ElemMultiSet vertices,PairMultiSet edges) {
	//System.out.println("Graph.constructor("+vertices.size()+","+edges.size()+")");
	//System.out.println("\nvertices: "); vertices.print();
	//System.out.println("\nedges: "); edges.print();
	v = makeCleanList(vertices.copy());
	

	/* NEW implementation uses PointLink and hashing to construct succLists */
	PairMultiSet cleanEdges = makeCleanList(edges.copy());

	
	int initialCapacity = NUMBER_OF_BUCKETS;
	Hashtable vertexHT = new Hashtable(initialCapacity);
	
	//put vertices in hashtable
	Iterator it = v.iterator();
	ObjectLink htEntry;
	Element actElem;
	int counter = 0;
	while (it.hasNext()) {
	    actElem = (Element)((MultiSetEntry)it.next()).value;
	    htEntry = new ObjectLink(actElem);
	    //check whether htEntry already exists in hashtable
	    //if not, give it a new number and insert it
	    if (!vertexHT.containsKey(htEntry.linkedObject)) {
		htEntry.number = counter;
		counter++;
		vertexHT.put(htEntry.linkedObject,htEntry);
	    }//if
	}//while it


	//construct list of vertices from hashtable
	Enumeration enum = vertexHT.elements();
	vArr = new Vertex[counter];
	ObjectLink elem;
	while (enum.hasMoreElements()) {
	    elem = (ObjectLink)enum.nextElement();
	    vArr[((ObjectLink)elem).number] = new Vertex((Element)elem.linkedObject,((ObjectLink)elem).number);
	}//while enum


	//construct succLists
	succLists = new LinkedList[vArr.length];
	for (int i = 0; i < succLists.length; i++) succLists[i] = new LinkedList();
	it = cleanEdges.iterator();
	int pos1 = 0;
	int pos2 = 0;
	ElemPair actPair;
	while (it.hasNext()) {
	    actPair = (ElemPair)((MultiSetEntry)it.next()).value;
	    //get the numbers for both objects in actPair
	    try {
		pos1 = ((ObjectLink)vertexHT.get(actPair.first)).number;
		pos2 = ((ObjectLink)vertexHT.get(actPair.second)).number;
	    } catch (Exception e) {
		System.out.println("Graph.constructor(...): An error occured while constructing the graph. Edges don't match to vertices.");
		System.out.println("\nHashtable: ");
		enum = vertexHT.elements();
		while (enum.hasMoreElements()) {
		    elem = (ObjectLink)enum.nextElement();
		    System.out.println(" "+elem.number+" - "+elem.linkedObject);
		}//while
		System.out.println("\nactual pair: "+actPair.first+"/"+actPair.second);
		System.exit(0);
	    }//catch

	    //set the proper vertices in succLists
	    succLists[pos1].add(vArr[pos2]);
	    succLists[pos2].add(vArr[pos1]);
	}//while it

    }

    protected Graph (MultiSet msV, MultiSet msE) {
	//constructs a subGraph from vertices and edges
	
	//extract elements from msV and msE
	ElemMultiSet extractV = new ElemMultiSet(new ElemComparator());
	Iterator it = msV.iterator();
	while (it.hasNext()) 
	    extractV.add(((Vertex)((MultiSetEntry)it.next()).value).value);
	PairMultiSet extractE = new PairMultiSet(new ElemPairComparator());
	it = msE.iterator();
	while (it.hasNext())
	    extractE.add(new ElemPair((Element)(((Edge)((MultiSetEntry)it.next()).value).first).value,
				      (Element)(((Edge)((MultiSetEntry)it.next()).value).second).value));
	
	ElemMultiSet vertices = extractV;
	PairMultiSet edges = extractE;
	
	/* COPY from constructor above */
	v = makeCleanList(vertices.copy());
	    
	/* NEW implementation uses PointLink and hashing to construct succLists */
	PairMultiSet cleanEdges = makeCleanList(edges.copy());
	
	int initialCapacity = NUMBER_OF_BUCKETS;
	Hashtable vertexHT = new Hashtable(initialCapacity);
	
	//put vertices in hashtable
	it = v.iterator();
	ObjectLink htEntry;
	Element actElem;
	int counter = 0;
	while (it.hasNext()) {
	    actElem = (Element)((MultiSetEntry)it.next()).value;
	    htEntry = new ObjectLink(actElem);
	    //check whether htEntry already exists in hashtable
	    //if not, give it a new number and insert it
	    if (!vertexHT.containsKey(htEntry.linkedObject)) {
		htEntry.number = counter;
		counter++;
		vertexHT.put(htEntry.linkedObject,htEntry);
	    }//if
	}//while it

	//construct list of vertices from hashtable
	Enumeration enum = vertexHT.elements();
	vArr = new Vertex[counter];
	ObjectLink elem;
	while (enum.hasMoreElements()) {
	    elem = (ObjectLink)enum.nextElement();
	    vArr[((ObjectLink)elem).number] = new Vertex((Element)elem.linkedObject,((ObjectLink)elem).number);
	}//while enum

	//construct succLists
	succLists = new LinkedList[vArr.length];
	for (int i = 0; i < succLists.length; i++) succLists[i] = new LinkedList();
	it = cleanEdges.iterator();
	int pos1;
	int pos2;
	ElemPair actPair;
	while (it.hasNext()) {
	    actPair = (ElemPair)((MultiSetEntry)it.next()).value;
	    //get the numbers for both objects in actPair
	    pos1 = ((ObjectLink)vertexHT.get(actPair.first)).number;
	    pos2 = ((ObjectLink)vertexHT.get(actPair.second)).number;
	    //set the proper vertices in succLists
	    succLists[pos1].add(vArr[pos2]);
	    succLists[pos2].add(vArr[pos1]);
	}//while it

	//now replace the ordinary vertices with ElemMultiSets
	ElemComparator ec = new ElemComparator();
	for (int i = 0; i < vArr.length; i++) {
	    Vertex oldV = vArr[i];
	    ElemMultiSet newV = new ElemMultiSet(ec);
	    newV.add(oldV.value);
	    oldV.value = newV;
	}//for i
	    
    }


    //methods
    private void buildSuccListsVE(PairMultiSet e) {
	//build lists of successing vertices for every vertex
	//both lists, v and e, already exist
	//System.out.println("entering Graph.buildSuccListsVE...");
	System.out.println("Graph.buildSuccListsVE is only O(n log n). Improve it with hashing.");

	succLists = new LinkedList[v.size()];
	for (int i = 0; i < succLists.length; i++) succLists[i] = new LinkedList();
	vArr = new Vertex[v.size()];
	
	//sort vertices and put them in vArr
	//SetOps.quicksortX(v);
	Iterator lit = v.iterator();
	int count = 0;
	while (lit.hasNext()) {
	    vArr[count] = new Vertex((Element)((MultiSetEntry)lit.next()).value,count);
	    count++;
	}//while
	
	//now, go through edge-list and for every vertex in there
	//use binary search on vArr to find itself in vArr
	lit = e.iterator();
	Element acte1;
	Element acte2;
	ElemPair actEP;
	while (lit.hasNext()) {
	    actEP = (ElemPair)((MultiSetEntry)lit.next()).value;
	    acte1 = actEP.first;
	    acte2 = actEP.second;
	    
	    //add successors
	    //System.out.println("\nvArr:"); for (int i = 0; i < vArr.length; i++) vArr[i].print();
	    //System.out.println("searching for: "); acte1.print();
	    int pos1 = binarySearch(acte1,0,vArr.length);
	    //System.out.println("result pos: "+pos1);
	    //System.out.println("searching for: "); acte2.print();
	    int pos2 = binarySearch(acte2,0,vArr.length);
	    //System.out.println("result pos: "+pos2);
	    succLists[pos1].add(vArr[pos2]);
	    succLists[pos2].add(vArr[pos1]);
	}//while

	//System.out.println("succLists:"); printSuccLists();
     
	//System.out.println("leaving Graph.buildSuccListsVE");
    }//end buildSuccListsVE

    /*
    public ElemMultiSet vertices () {
	//returns the set of vertices
	return v.copy();
    }//end method vertices
    */

    /*
    public PairMultiSet edges () {
	//returns the set of edges
	return e.copy();
    }//end method edges
    */

    public ConnectedComponentsPair connectedComponents() {
	//returns the connected components of this
	
	//System.out.println();
	//System.out.println("connectedComponents calling...");

	boolean [] marks = new boolean [vArr.length];
	for (int i = 0; i < marks.length; i++) {
	    marks[i] = false;
	}//for i

	int actElem = 0;
	LinkedList compListV = new LinkedList();
	LinkedList compListE = new LinkedList();

	if (!(v.size() == 0)) {
	    //compute depth-first spanning trees
	    while (hasUnvisitedVertices(marks)) {
		//System.out.println("graph still has unvisited vertices...");
		MultiSet compV = new MultiSet();
		MultiSet compE = new MultiSet();
		actElem = getUnvisitedVertex(marks);
		//System.out.println("this vertex is number "+actElem);
		depthFirst(actElem,marks,compV,compE);
		//System.out.println("depthFirst finished");
		compListV.add(compV);
		compListE.add(compE);
	    }//while
	}//if
	else {
	    System.out.println("Graph has no vertices.");
	}//else
	
	ConnectedComponentsPair ccp = new ConnectedComponentsPair(compListV,compListE);
	
	return ccp;
    }//end method connectedComponentsV
    
      
    public MultiSet[] connectedComponentsSubGraphs() {
	//returns the connected components of this as subgraphs
	
	boolean[] marks = new boolean[vArr.length];
	for (int i = 0; i < marks.length; i++)
	    marks[i] = false;
	
	int actElem = 0;
	LinkedList compListV = new LinkedList();
	LinkedList compListE = new LinkedList();
	LinkedList graphList = new LinkedList();
	
	if (!(v.size() == 0)) {
	    //compute depth-first spanning trees
	    while (hasUnvisitedVertices(marks)) {
		MultiSet compV = new MultiSet();
		MultiSet compE = new MultiSet();
		actElem = getUnvisitedVertex(marks);
		depthFirst(actElem,marks,compV,compE);
		
		Graph subgraph = new Graph(compV,compE);
		graphList.add(subgraph);
	    }//while
	}//if
	else 
	    System.out.println("Graph has no vertices.");

	return (MultiSet[])graphList.toArray();
    }//end method connecteComponentsSubGraphs

    public void print () {
	//prints out the graph's data
	System.out.println("vertices: ");
	//v.print();
	for (int i = 0; i < vArr.length; i++) vArr[i].print();
	System.out.println("\nedges: ");
	printSuccLists();
	//System.out.println();
	//System.out.println("edges: ");
	//e.print();
    }//end method print

    
    public void printSuccLists() {
	//prints the list of successors of every vertex
	for (int i = 0; i < vArr.length; i++) {
	    System.out.println("\nvertex: ");
	    vArr[i].print();
	    System.out.println("successors: ");
	    for (int j = 0; j < succLists[i].size(); j++) {
		((Vertex)succLists[i].get(j)).print(); }
	}//for i
    }//end method printSuccLists
    

    private void depthFirst (int actElem, boolean[] marks, MultiSet compV, MultiSet compE) {
	marks[actElem] = true;

	//check, whether a vertex has a self-edge.
	//though this is not relevant for the connected component algo
	//it is needed for some algorithms in SetOps, e.g. overlapReduce.
	ListIterator lit = succLists[actElem].listIterator(0);
	while (lit.hasNext()) {
	    
	    //System.out.println("compE.isEmtpy: "+compE.isEmpty());
	    //if (!compE.isEmpty()) System.out.println("compE.first().class: "+compE.first().getClass());

	    if (((Vertex)lit.next()).equal(vArr[actElem])) {
		compE.add(new Edge(vArr[actElem],vArr[actElem]));
	    }//if
	}//while

	compV.add(vArr[actElem].copy());
	//System.out.println("hasUnvisitedSon?");
	while (hasUnvisitedSon(actElem,marks)) {
	    int next = nextUnvisitedSon(actElem,marks);
	    compE.add(new Edge(vArr[actElem],vArr[next]));
	    depthFirst(next,marks,compV,compE);
	}//while
    }//end method depthFirst


    private boolean hasUnvisitedVertices (boolean [] marks) {
	for (int i = 0; i < marks.length; i++) {
	    if (!marks[i]) { return true; }
	}//for i
	return false;
    }//end method hasUnvisitedVertices
    
    private int getUnvisitedVertex(boolean [] marks) {
	for (int i = 0; i < marks.length; i++) {
	    if (!marks[i]) { return i; }
	}//for i
	System.out.println("Error in Graph.hasUnvisitedVertices --- Has NO unvisited vertices.");
	System.exit(0);
        return -1;
    }//end method getUnvisitedVertex

    private boolean hasUnvisitedSon(int actElem, boolean[] marks) {
	//returns true if one of the sons of vertex actElem wasn't visited yet

	ListIterator lit = succLists[actElem].listIterator(0);
	while (lit.hasNext()) {
	    Vertex actSucc = (Vertex)lit.next();
	    if (!marks[actSucc.number]) return true;
	}//while

	return false;
    }//end method hasUnvisitedSon


    private int nextUnvisitedSon (int actElem, boolean[] marks) {
	//returns the index of the next unvisited son of vertex actElem
	
	ListIterator lit = succLists[actElem].listIterator(0);
	while (lit.hasNext()) {
	    Vertex actSucc = (Vertex)lit.next();
	    if (!marks[actSucc.number]) return actSucc.number;
	}//while
	
	System.out.println("Error in Graph.nextUnvisitedSon --- Has NO unvisited son.");
	System.exit(0);
	return -1;
    }//end method nextUnvisitedSon


    private static PairMultiSet makeCleanList (PairMultiSet plIn) throws WrongTypeException {
	//since the PairList that is passed to Graph constructor
	//may have duplicates, which result in double edges
	//these duplicates are removed here
	//CAUTION: this method may not work properly with all kinds of user-defined objects!

	//first, traverse Pairlist and twist all Pairs that way, that the smaller 
	//Element is at first position
	Iterator lit = plIn.iterator();
	boolean isSegment = false;

	if (!plIn.isEmpty() && 
	    ((((ElemPair)plIn.first()).first) instanceof Segment)) isSegment = true;

	ElemPair actPair;
	while (lit.hasNext()) {
	    actPair = (ElemPair)((MultiSetEntry)lit.next()).value;
	    if (isSegment) {
		((Segment)actPair.first).align();
		((Segment)actPair.second).align();
	    }//if
	    if (actPair.first.compare(actPair.second) == 1) actPair.twist();
	}//while

	/*OLD IMPLEMENTION WITHOUT MULTISETS
	//now sort PairList
	SetOps.quicksort(plIn);

	//traverse Pairlist and remove all neighbours which are equal
	lit = plIn.listIterator(0);
	ListIterator lit2;
	while (lit.hasNext()) {
	    actPair = (ElemPair)lit.next();
	    lit2 = plIn.listIterator(lit.nextIndex());
	    while (lit2.hasNext() && ((ElemPair)lit2.next()).equal(actPair)) {
		lit2.remove();
		lit2 = plIn.listIterator(lit.nextIndex());
		lit = plIn.listIterator(lit.nextIndex()-1);
	    }//while
	}//while
	*/
	//NEW IMPLEMENTATION
	PairMultiSet retSet = new PairMultiSet(new ElemPairComparator());
	retSet.addAll(plIn);
	//remove duplicates manually
	Iterator it = retSet.iterator();
	while (it.hasNext())
	    ((MultiSetEntry)it.next()).number = 1;
      
	return retSet;
    }//end method makeCleanList
		

    private static ElemMultiSet makeCleanList (ElemMultiSet elIn) throws WrongTypeException {
	//sinde the ElemList that is passed to Graph constructor
	//may have duplicates, which result in double vertices
	//these duplicates are removed here
	
	//System.out.println("entering Graph.makeCleanList(ElemList)...");

	//check for segments and align them if necessary (needed for compare)

	if (!elIn.isEmpty() &&
	    (((Element)(elIn.first()) instanceof Segment))) {
	    Iterator lit = elIn.iterator();
	    Element actEl;
	    while (lit.hasNext()) {
		actEl = (Element)((MultiSetEntry)lit.next()).value;
		((Segment)actEl).align();
	    }//while
	}//if

	ElemMultiSet retSet = new ElemMultiSet(new ElemComparator());
	retSet.addAll(elIn);
	retSet = SetOps.rdup(retSet);
	
	//System.out.println("leaving Graph.makeCleanList(ElemList).");

	return retSet;
    }//end method makeCleanList



    /* IT SEEMS THAT THESE ARE NOT USED ANYMORE	
    protected ElemListList computeCycles () {
	//Computes the (minimal) cycles of this and returns them as
	//an ElemListList. Each of the Elemlists is a cycle of this.
	//The whole graph must be connected, otherwise this method doesn't work properly.

	//System.out.println("Graph.computeCycles calling...");

	//ElemListList retList = new ElemListList();
	ElemListList allCycles = new ElemListList();

	//find all cycles
	allCycles = findCycles();
	//retList = allCycles;

	return allCycles;
    }//end method computeCycles
    */
    
    /* IT SEEMS THAT THESE ARE NOT USED ANYMORE
    private ElemListList findCycles() {
	//computes all cycles of this using a modified version of a graph expansion
	
	//System.out.println("Graph.findCycles calling...");

	ElemListList retList = new ElemListList();
	
	//Start at first vertex of v and visit every neighbour vertex that wasn't visited recursively.
	//Keep track of every visited vertex and before adding a new vertex search in the track list if
	//it was already added. If true (and this isn't a sequence A-B-A) then store this track list as a cycle.
	ElemList startList = new ElemList();
	expansion(0,startList,retList);
	return retList;
    }//end method findCycles
    */

    /* IT SEEMS THAT THESE ARE NOT USED ANYMORE
    private void expansion (int actElem, ElemList pred, ElemListList retList) {
	//recursive method for finding cycles
	//pred is the list of predecessors: it contains all already visited vertices on the path from the start vertex to actElem
	//retList contains all already found cycles

	//System.out.println("Graph.expansion calling... actElem:"+actElem);

	pred.add(((Element)v.get(actElem)).copy());
	//System.out.println("actElem added to pred");
	//System.out.println("actual pred:"); pred.print();
	//for (int i = 0; i < ((ElemList)succLists.get(actElem)).size(); i++) {
	for (int i = 0; i < succLists[actElem].size(); i++) {
	    //Element newElem = (Element)((ElemList)succLists.get(actElem)).get(i);
	    Element newElem = (Element)succLists[actElem].get(i);
	    //System.out.println("visiting successors:"+actElem+"-"+actNumber(newElem));
	    //check for sequence A-B-A
	    boolean sequence = false;
	    if ((pred.size() > 1) &&
		(newElem.equal((Element)pred.get(pred.size()-2)))) {
		sequence = true;
		//System.out.println("***sequence:true");
	    }//if
	    if (sequence) {
		//pred.removeLast();
		//System.out.println("removed last element from pred");
	    }//if
	    else {
		//if new neighbour isn't already in pred
		if (!member(newElem,pred)) {
		    //System.out.println("recursive call");
		    expansion(actNumber(newElem),pred.copy(),retList);
		}//if
		//it is in pred, so a cycle was found
		else {
		    //System.out.println("###cycle found:"); //pred.print();
		    //add newElem again
		    pred.add((newElem).copy());
		    ElemList nel = extractCycle(pred);
		    //nel.print();
		    retList.add(nel);
		    //System.out.println("break!");
		    //System.out.println();
		    return;
		}//else
	    }//if
	}//for i
    }//end method expansion
    */
    /* IT SEEMS THAT THESE ARE NOT USED ANYMORE
    private boolean member (Element el, ElemList list) {
	//supportive method for expansion
	//returns true if el is in list
	//false otherwise

	for (int i = 0; i < list.size(); i++) {
	    if (((Element)list.get(i)).equal(el)) {
		return true;
	    }//if
	}//for i
	
	return false;
    }//end method member
    */
    /* IT SEEMS THAT THESE ARE NOT USED ANYMORE
    private int actNumber(Element el) {
	//supportive method for expansion
	//returns the int number of el
	
	for (int i = 0; i < v.size(); i++) {
	    if (el.equal((Element)v.get(i))) {
		return i;
	    }//if
	}//for i
	
	System.out.println("Error in Graph.actNumber:");
	System.exit(0);
	return -1;
    }//end method actNumber
    */
    /* IT SEEMS THAT THESE ARE NOT USED ANYMORE
    private ElemList extractCycle(ElemList el) {
	//extract a the cycle from a ElemList which includes a cylce
	//e.g.: A-B-C-D-E-B -> B-C-D-E
	//we know that the last element is part of the cycle
	ElemList retList = new ElemList();
	int marker = -1;
	for (int i = 0; i < el.size(); i++) {
	    if (((Element)el.get(i)).equal(((Element)el.getLast()))) {
		marker = i;
		//System.out.println("marker:"+marker);
		break;
	    }//if
	}//for i
	if ((marker == -1) || (marker == el.size())) {
	    System.out.println("Error(Graph.extractCycle): List has no cycle.");
	}//if
	else {
	    for (int i = marker; i < el.size()-1; i++) {
		retList.add(((Element)el.get(i)).copy());
	    }//for i
	}//else
	
	return retList;
    }//end method extractCycle
    */
  
    public ConnectedComponentsPair computeReducedPair (ConnectedComponentsPair ccp) {

	//This method takes a connected component and reduces the number
	//of  edges of this component such that every vertex appears only
	//ONCE in a component. So a connected component with four vertices
	//now is represented by maximal two vertices. E.g. the chain
	//A-B-C-D now is represented by A-B and C-D. Another correct
	//representation would be B-C, A, D. The isolated vertices are added 
	//to ccp.compVertices
	//CAUTION the number for every vertex in the resulting ccp is set to -1

	//OLD COMMENT
	//for an existing ccp compute the set of pairs in
	//ccp.compEdges such that no vertex in it appears
	//twice; all vertices, that are now isolated, are
	//added to ccp.compVertices
	//System.out.println("entering G.cRP...");

	//make a copy of succLists
	LinkedList[] slCopy = new LinkedList[succLists.length];
	for (int i = 0; i < succLists.length; i++) {
	    slCopy[i] = (LinkedList)succLists[i].clone(); }
	
	ConnectedComponentsPair retCCP = new ConnectedComponentsPair();
	
	//traverse ccp.compEdges
	ListIterator litE = ccp.compEdges.listIterator(0);
	MultiSet actEL;
	Edge actEdge;
	//scan every component
	while (litE.hasNext()) {
	    MultiSet elList = new MultiSet();
	    //set actEL as one component
	    actEL = (MultiSet)litE.next();
	    //traverse component
	    if (!actEL.isEmpty()) {
		Iterator litE2 = actEL.iterator();
		while (litE2.hasNext()) {
		    //get edge from component
		    actEdge = (Edge)((MultiSetEntry)litE2.next()).value;
		    //scan slCopy for both vertices in actEdge and
		    //remove _all_ successors.
		    //if one of the vertices has no successors,
		    //the edge may not be added

		    //System.out.println("\nscan for:"); actPair.print();
		    //System.out.println("\nactual slCopy:");
		    //for (int i = 0; i < slCopy.length; i++) {
		    //	System.out.println("V:");
		    //	((Element)v.get(i)).print();
		    //	System.out.println("succList:");
		    //	slCopy[i].print();
		    //}//for
		    
		    boolean wasEmpty1 = false;
		    boolean wasEmpty2 = false;
		    Vertex vertex1 = actEdge.first;
		    Vertex vertex2 = actEdge.second;

		    if (slCopy[vertex1.number].isEmpty()) wasEmpty1 = true;
		    if (slCopy[vertex2.number].isEmpty()) wasEmpty2 = true;
		    
		    //If both of the vertices still had successors, 
		    //now remove all successors from slCopy and add
		    //the found edge to the resulting elList.
		    if (!(wasEmpty1 || wasEmpty2)) {
			//delete succLists
			slCopy[vertex1.number].clear();
			slCopy[vertex2.number].clear();
			//add actPair to retList
			//System.out.println("---> added pair");
			elList.add(actEdge); }
		    //else {
			//System.out.println("---> didn't add pair: wasEmpty1:"+wasEmpty1+", wasEmpty2:"+wasEmpty2); }
			
		}//while
	    }//if
	    //if (!plList.isEmpty()) {
	    //add the newly computed list of edges for the actual component to
	    //the resulting list retCCP
	    retCCP.compEdges.add(elList);
	}//while

	//now compute the new compV
	//extract used elements for every compE and compute the difference
	//with old compV
	ListIterator litV;
	MultiSet actVL;
	litE = retCCP.compEdges.listIterator(0);
	litV = ccp.compVertices.listIterator(0);
	if (!ccp.compEdges.isEmpty()) {
	    while (litE.hasNext()) {
		actVL = (MultiSet)litV.next();
		actEL = (MultiSet)litE.next();
		//to be able to use the SetOps operations, the vertices
		//must be extracted from list of edges (litE) and list of
		//vertices (litV). After the first extraction, the elements must
		//be extracted from the vertices.
		
		//extract elements from list of edges
		ElemMultiSet elementsInEdges = new ElemMultiSet(new ElemComparator());
		Iterator tempLIT = actEL.iterator();
		actEdge = null;
		while (tempLIT.hasNext()) {
		    actEdge = (Edge)((MultiSetEntry)tempLIT.next()).value;
		    elementsInEdges.add(actEdge.first.value);
		    elementsInEdges.add(actEdge.second.value);
		}//while

		//extract elements from list of vertices
		ElemMultiSet elementsInVertices = new ElemMultiSet(new ElemComparator());
		tempLIT = actVL.iterator();
		Vertex actVertex;
		while (tempLIT.hasNext()) {
		    actVertex = (Vertex)((MultiSetEntry)tempLIT.next()).value;
		    elementsInVertices.add(actVertex.value);
		}//while
		
		//ElemList eipl = SetOps.elementsInPairList(actPL);
		ElemMultiSet diff = SetOps.difference(elementsInVertices,elementsInEdges);
		//System.out.println("\nelement["+(litE.nextIndex()-1)+"]");
		//System.out.println("eipl:"); eipl.print();
		//System.out.println("diff:"); diff.print();
		
		//now transform the ElemList back to a list of vertices
		tempLIT = diff.iterator();
		MultiSet vertexList = new MultiSet();
		while (tempLIT.hasNext()) {
		    vertexList.add(new Vertex((Element)((MultiSetEntry)tempLIT.next()).value,-1)); }
		retCCP.compVertices.add(vertexList);
	    }//while
	}//if
	else { retCCP.compVertices.addAll(ccp.compVertices); }
	    
	
	//System.out.println("leaving G.cRP");
	return retCCP;
    }//end method computeReducedPair

    /* PROBABLY NOT USED ANYMORE
    public ElemListList computeBCCs() {
	//returns the bi-connected components of THIS as ElemListList
	//this is an implementation of the algorithm described in
	//Kurt Mehlhorn
	//Data Structures and Algorithms2
	//Graph Algorithms and NP-Completeness
	//Page 35-37

	//NOTE: the returned list is an ElemListList, so the values of the
	//graph's vertices are extracted.
	//
	//NOTE: Isolated vertices are not stored as BCC.

	ElemListList retList = new ElemListList();
	
	LinkedList s = new LinkedList();
	boolean[] vINs = new boolean[v.size()]; //if vi is in s, then v[i]=true
	int count1 = 1;
	LinkedList current = new LinkedList();
	Vertex actV = null;
	int[] dfsNum = new int[this.v.size()];
	int[] lowPT = new int[this.v.size()];
	int[] fatherList = new int[this.v.size()];
	for (int i = 0; i < dfsNum.length; i++) {
	    dfsNum[i] = -1;
	    lowPT[i] = -1;
	    fatherList[i] = -1;
	    vINs[i] = false;
	}//for i

	for (int i = 0; i < vArr.length; i++) {
	    if (!vINs[i]) {
		actV = vArr[i];
		s.add(actV);
		vINs[i] = true;
		fatherList[i] = 0;
		current.add(actV);
		dfsNum[i] = count1;
		bccDFS(i,actV,count1,dfsNum,lowPT,s,vINs,current,retList,fatherList);
	    }//if
	}//for i

	return retList;
    }//end method computeBCCs
    */

    /* PROBABLY NOT USED ANYMORE
    private void bccDFS(int vNum, Vertex actV, int count1, int[] dfsNum, int[] lowPT, LinkedList s, boolean[] vINs, LinkedList current, ElemListList retList, int[] fatherList) {
	//supportive method for computeSCCs
	//computes bi-connected components using depth first search
	//this is actually the recursive dfs method
	//
	//NOTE: isolated points are not stored as BCC

	//System.out.println("entering sccDFS... --- vNum:"+vNum);
	//System.out.print("dfsNum:");
	//for (int i = 0; i < dfsNum.length; i++) System.out.print("["+dfsNum[i]+"] ");
	//System.out.println();
	//System.out.print("fatherList:");
	//for (int i = 0; i < fatherList.length; i++) System.out.print("["+fatherList[i]+"] ");
	//System.out.println();
	//System.out.println("s:"); s.print();
	//System.out.println();


	lowPT[vNum] = dfsNum[vNum];
	Vertex actSucc = null;
	ListIterator succLit = this.succLists[vNum].listIterator(0);
	while (succLit.hasNext()) {
	    actSucc = (Vertex)succLit.next();
	    boolean wINs = false;
	    ListIterator sLit = s.listIterator(0);
	    Vertex actSelem;
	    while (sLit.hasNext()) {
		actSelem = (Vertex)sLit.next();
		if (actSelem.value.equal(actSucc.value)) {
		    wINs = true;
		    break;
		}//if
	    }//while

	    int wNum = actSucc.number;

	    if (!wINs) {
		s.add(actSucc);
		vINs[wNum] = true;
		fatherList[wNum] = vNum;
		current.add(actSucc);
		count1++;
		dfsNum[wNum] = count1;
		bccDFS(wNum,actSucc,count1,dfsNum,lowPT,s,vINs,current,retList,fatherList);
		if (lowPT[vNum] > lowPT[wNum]) lowPT[vNum] = lowPT[wNum];
	    }//if
	    
	    if (dfsNum[wNum] < dfsNum[vNum]) {
		if (lowPT[vNum] > dfsNum[wNum]) lowPT[vNum] = dfsNum[wNum];
	    }//if
	}//while

	if ((dfsNum[vNum] >= 2) && (lowPT[vNum] == dfsNum[fatherList[vNum]])) {
	    ElemList newBCC = new ElemList();
	    newBCC.add(vArr[fatherList[vNum]].value);
	    ListIterator currLit = current.listIterator(0);
	    Vertex actVertex;
	    while (currLit.hasNext()) {
		actVertex = (Vertex)currLit.next();
		int wNum = actVertex.number;
		
		if (dfsNum[wNum] >= dfsNum[vNum]) {
		    newBCC.add(actVertex.value);
		    //int index = currLit.nextIndex();
		    currLit.remove();
		    //currLit = current.listIterator(index-2);
		}//if
	    }//while
	    retList.add(newBCC);
	}//if
		
    }//end method bccDFS 
    */    

    private int binarySearch(Element el, int lo, int hi) {
	//searches in vArr for el and returns the index
	//using binary search
	//lo,hi are low/high bound for vArr
	//System.out.println("\nentering Graph.binarySearch..");
	//System.out.println("searching for: "); el.print();
	//System.out.println("vArr:");
	//for (int i = 0; i < vArr.length; i++) { vArr[i].print(); }
	boolean found = false;

	int actPos = (lo+hi)/2;
	Vertex actV = vArr[actPos];
	if (((Element)actV.value).equal(el)) return actPos;
	else {
	    if (((Element)actV.value).compare(el) == -1) return binarySearch(el,actPos+1,hi);
	    else return binarySearch(el,lo,actPos-1);
	}//else
    }//end method binarySearch


    public ElemMultiSetList computeFaces(){
	//returns the faces of this graph as SegLists
	
	//first, sort the vertices in succLists such that
	//the segments (which are formed by the pairs of vertices)
	//are sorted as described in the ROSE implementation paper
	//-> sorting of halfsegments.
	//here, only the vertices are sorted. Not more is needed
	//(especially no construction of halfsegments).
	//
	//CAUTION: Efficiency can be improved by replacing the
	//sorting algorithm (which has time complexity of O(n²).
	//But this is not that important, because n is the degree 
	//of a vertex in this case. Thus, n is rarely greater than 4.
	System.out.println("entering Graph.computeFaces...");

	ElemMultiSetList retList = new ElemMultiSetList();
	boolean isNotEmpty = false;
	for (int i = 0; i < succLists.length; i++) {
	    if (succLists[i].size() > 0) {
		isNotEmpty = true;
		break;
	    }//if
	}//while it
	if (isNotEmpty == false) return retList;

	//make working-copy of succLists
	LinkedList[] succListsCOP = (LinkedList[])this.succLists.clone();

	//sorting
	for (int i = 0; i < succListsCOP.length; i++) {
	    //generate edges from pairs of vertices
	    LinkedList edges = new LinkedList();
	    ListIterator lit = succListsCOP[i].listIterator(0);
	    while (lit.hasNext()) edges.add(new Edge(vArr[i],(Vertex)lit.next()));
	    edges = sortEdgeListWithRespectToHalfSegmentsOrder(edges);
	    //rebuild succListsCOP
	    succListsCOP[i] = extractVerticesOtherThanX(edges,vArr[i]);
	}//for i
	
	//System.out.println("\nGRAPH:"); this.print();
	
	//loop while edges exist
	while (thereAreEdgesLeft(succListsCOP)) {
	    //find starting point by taking the leftmost and downmost point
	    Vertex startVertex = getStartVertex(succListsCOP);
	    
	    //initialize cycle data structure
	    LinkedList cycle = new LinkedList();

	    //begin with following the first edge; choose the one wich is minimal
	    //store in cycle and remove edge from succLists
	    cycle.add(startVertex);
	    Vertex actVertex = (Vertex)succListsCOP[startVertex.number].getFirst();
	    cycle.add(actVertex);
	    succListsCOP[startVertex.number].removeFirst();
	    Vertex prevVertex = startVertex;

	    //follow predessessing edges (according to the sorting) and mark edges
	    //until first vertex is found again
	    int prevVertexPos = -1;
	    int newActVertexPos = -1;
	    do {
		//System.out.println("actVertex.number: "+actVertex.number);
		prevVertexPos = getPosOfVertex(prevVertex,succListsCOP[actVertex.number]);
		//System.out.println("prevVertexPos: "+prevVertexPos);
		newActVertexPos = (prevVertexPos+succListsCOP[actVertex.number].size()-1) % succListsCOP[actVertex.number].size();
		//System.out.println("newActVertexPos: "+newActVertexPos);
		prevVertex = actVertex;
		actVertex = (Vertex)succListsCOP[actVertex.number].get(newActVertexPos);
		//add vertex to actual cycle
		cycle.add(actVertex);
		//remove vertices from succListsCOP
		succListsCOP[prevVertex.number].remove(prevVertexPos);
		//get position of the vertex which shall be deleted
		prevVertexPos = getPosOfVertex(actVertex,succListsCOP[prevVertex.number]);
		succListsCOP[prevVertex.number].remove(prevVertexPos);
	    } while (!actVertex.equal(startVertex));

	    //remove one last vertex from succListsCOP
	    succListsCOP[actVertex.number].remove(getPosOfVertex(prevVertex,succListsCOP[actVertex.number]));
	    //add cycle to retList
	    //System.out.println("# of points in actual (return) cycle: "+cycle.size());
	    retList.add(computeSMSFromCycle(cycle));
	}//for
	
	System.out.println("leaving Graph.computeFaces("+retList.size()+").");
	//retList.print();
	//System.exit(0);
	return retList;
    }//end method computeFaces
    

    public CycleList computeFaceCycles(){
	//returns the faces of this graph as SegLists
	
	//first, sort the vertices in succLists such that
	//the segments (which are formed by the pairs of vertices)
	//are sorted as described in the ROSE implementation paper
	//-> sorting of halfsegments.
	//here, only the vertices are sorted. Not more is needed
	//(especially no construction of halfsegments).
	//
	//CAUTION: Efficiency can be improved by replacing the
	//sorting algorithm (which has time complexity of O(n²).
	//But this is not that important, because n is the degree 
	//of a vertex in this case. Thus, n is rarely greater than 4.
	//NOTE: This is the same implementation as in computeFaces, 
	//but has a different return type

	//System.out.println("entering Graph.computeFaceCycles...");

	CycleList retList = new CycleList();
	boolean isNotEmpty = false;
	for (int i = 0; i < succLists.length; i++) {
	    if (succLists[i].size() > 0) {
		isNotEmpty = true;
		break;
	    }//if
	}//while it
	if (isNotEmpty == false) return retList;

	//make working-copy of succLists
	LinkedList[] succListsCOP = (LinkedList[])this.succLists.clone();

	//sorting
	for (int i = 0; i < succListsCOP.length; i++) {
	    //generate edges from pairs of vertices
	    LinkedList edges = new LinkedList();
	    ListIterator lit = succListsCOP[i].listIterator(0);
	    while (lit.hasNext()) edges.add(new Edge(vArr[i],(Vertex)lit.next()));
	    edges = sortEdgeListWithRespectToHalfSegmentsOrder(edges);
	    //rebuild succListsCOP
	    succListsCOP[i] = extractVerticesOtherThanX(edges,vArr[i]);
	}//for i
	
	//System.out.println("\nGRAPH:"); this.print();
	
	//loop while edges exist
	while (thereAreEdgesLeft(succListsCOP)) {
	    //find starting point by taking the leftmost and downmost point
	    Vertex startVertex = getStartVertex(succListsCOP);
	    
	    //initialize cycle data structure
	    LinkedList cycle = new LinkedList();

	    //begin with following the first edge; choose the one wich is minimal
	    //store in cycle and remove edge from succLists
	    cycle.add(startVertex);
	    Vertex actVertex = (Vertex)succListsCOP[startVertex.number].getFirst();
	    cycle.add(actVertex);
	    succListsCOP[startVertex.number].removeFirst();
	    Vertex prevVertex = startVertex;

	    //follow predessessing edges (according to the sorting) and mark edges
	    //until first vertex is found again
	    int prevVertexPos = -1;
	    int newActVertexPos = -1;
	    do {
		//System.out.println("actVertex.number: "+actVertex.number);
		prevVertexPos = getPosOfVertex(prevVertex,succListsCOP[actVertex.number]);
		//System.out.println("prevVertexPos: "+prevVertexPos);
		newActVertexPos = (prevVertexPos+succListsCOP[actVertex.number].size()-1) % succListsCOP[actVertex.number].size();
		//System.out.println("newActVertexPos: "+newActVertexPos);
		prevVertex = actVertex;
		actVertex = (Vertex)succListsCOP[actVertex.number].get(newActVertexPos);
		//add vertex to actual cycle
		cycle.add(actVertex);
		//remove vertices from succListsCOP
		succListsCOP[prevVertex.number].remove(prevVertexPos);
		//get position of the vertex which shall be deleted
		prevVertexPos = getPosOfVertex(actVertex,succListsCOP[prevVertex.number]);
		succListsCOP[prevVertex.number].remove(prevVertexPos);
	    } while (!actVertex.equal(startVertex));

	    //remove one last vertex from succListsCOP
	    succListsCOP[actVertex.number].remove(getPosOfVertex(prevVertex,succListsCOP[actVertex.number]));
	    //add cycle to retList
	    //System.out.println("# of points in actual (return) cycle: "+cycle.size());
	    retList.add(computeSegListFromCycle(cycle));
	}//for
	
	//System.out.println("leaving Graph.computeFaceCycles("+retList.size()+").");
	//retList.print();
	//System.exit(0);
	return retList;
    }//end method computeFaceCycles


    private static int getPosOfVertex(Vertex vert, LinkedList vertList) {
	//returns the position of vert in vertList
	ListIterator lit = vertList.listIterator(0);
	while (lit.hasNext()) {
	    Vertex actV = (Vertex)lit.next();
	    if (actV.equal(vert)) return lit.nextIndex()-1;
	}//while
	throw new NoSuchElementException();
    }//end getPosOfVertex

    private static SegMultiSet computeSMSFromCycle(LinkedList inCycle) {
	//inCycle is a LinkedList of vertices of the form
	//A-B-C-D-A
	//This method computes a SegList representation of inCycle, i.e.
	//(A-B)(B-C)(C-D)(D-A)
	SegMultiSet retList = new SegMultiSet(new SegmentComparator());

	if (inCycle.size() < 3) {
	    System.out.println("ERROR: found bad cycle in Graph.computeSMSFromCycle.");
	    System.out.println("execution terminated.");
	    System.exit(0);
	}//if

	for (int i = 0; i < inCycle.size()-1; i++) {
	    retList.add(new Segment((Point)((Vertex)inCycle.get(i)).value,
				    (Point)((Vertex)inCycle.get(i+1)).value));
	}//for i

	return retList;
    }//end method computeSMSFromCycle

    private static LinkedList computeSegListFromCycle(LinkedList inCycle) {
	//inCycle is a LinkedList of vertices of the form
	//A-B-C-D-A
	//This method computes a LinkedList representation of inCycle, i.e.
	//(A-B)(B-C)(C-D)(D-A)
	LinkedList retList = new LinkedList();
	//SegMultiSet retList = new SegMultiSet(new SegmentComparator());

	if (inCycle.size() < 3) {
	    System.out.println("ERROR: found bad cycle in Graph.computeSegListFromCycle.");
	    System.out.println("execution terminated.");
	    System.exit(0);
	}//if

	for (int i = 0; i < inCycle.size()-1; i++) {
	    retList.add(new Segment((Point)((Vertex)inCycle.get(i)).value,
				    (Point)((Vertex)inCycle.get(i+1)).value));
	}//for i

	return retList;
    }//end method computeSegListFromCycle

	    
    private Vertex getStartVertex(LinkedList[] sL) {
	//from the vertices in succLists sL find the 
	//leftmost (and then downmost) point (w.r.t. coordinates)
	//since vArr is sorted originally, we can take the first
	//vertex which has successors, i.e. outgoing edges.
	
	for (int i = 0; i < sL.length; i++) {
	    if (!sL[i].isEmpty()) { return vArr[i]; }
	}//for i
	
	System.out.println("ERROR in Graph.getStartVertex: No more vertices with edges found in sL.");
	System.out.println("execution terminated.");
	System.exit(0);
	return null;
    }//end method getStartVertex

    private static boolean thereAreEdgesLeft(LinkedList[] sL) {
	//returns TRUE if there is at least one vertex in 
	//succLists sL which has a successor, i.e. an outgoing edge

	for (int i = 0; i < sL.length; i++) {
	    if (!sL[i].isEmpty()) { return true; }
	}//for i
	
	return false;
    }//end method thereAreEdgesLeft

    private static LinkedList extractVerticesOtherThanX(LinkedList edges, Vertex x) {
	//All edges in the passed list edges have vertex x as one vertex.
	//This method generates a list of vertices wich are all the _other_
	//vertices in the same order.
	LinkedList retList = new LinkedList();
	
	ListIterator lit = edges.listIterator(0);
	while (lit.hasNext()) {
	    retList.add(((Edge)lit.next()).theOtherOne(x));
	}//while

	return retList;
    }//end method extractVerticesOtherThanX
	    
    private static LinkedList sortEdgeListWithRespectToHalfSegmentsOrder(LinkedList edges) {
	//There is an order defined for halfsegments (described in the ROSE
	//implementation paper. This order is used here to sort the edges
	//passed in list edges.
	//The sorted list is returned.
	//CAUTION: A simple sorting algorithm (with O(n²) time complexity) is used.
	//Since the number of edges is rarely greater than 4, this should be no
	//problem.
	//System.out.println("entering sELWRTHSO... size("+edges.size()+")");
	LinkedList retList = edges;
	
	Edge min = null;
	int minPos = 0;
	boolean found = false;
	int edgessize = edges.size();
	//System.out.println("---------------------");
	for (int i = 0; i < edgessize-1; i++) {	    
	    min = (Edge)edges.get(i);
	    found = false;
	    for (int j = i+1; j < edgessize; j++) {
		Edge actJ = (Edge)edges.get(j);
		//System.out.println("comparing: "); min.print(); actJ.print();
		int cmp = min.compare(actJ);
		//System.out.println("compare result: "+cmp);
		if (cmp == 1) {
		    min = (Edge)edges.get(j);
		    minPos = j;
		    found = true;
		}//if
	    }//for j
	    if (found) {
		Edge tmp = (Edge)edges.get(i);
		edges.set(i,min);
		edges.set(minPos,tmp);
	    }//if
	}//for i
		    
	//System.out.println("leaving sELWRTHSO.");
	return retList;
    }//end method sortEdgeListWithRespectToHalfSegmentsOrder

    /*
    public MultiSet reduce (Method predicate, Method method) {
	//comment missing

	//get information about method
	ElemMultiSet retSet;
	int paramTypeCount = Array.getLength(method.getParameterTypes());
	Element[] paramList = new Element[paramTypeCount];
	boolean metTypeElement = false;
	boolean metTypeElemList = false;
	try {
	    if (method.getReturnType().isInstance(Class.forName("Element")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("Element"))) {
		metTypeElement = true; }
	    if (method.getReturnType().isInstance(Class.forName("ElemMultiSet")) ||
		method.getReturnType().getSuperclass().isAssignableFrom(Class.forName("ElemMultiSet"))) {
		metTypeElemList = true; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Error in Graph.reduce: can't examine method.");
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	
	int predParamTypeCount = Array.getLength(predicate.getParameterTypes());
	Element[] predParamList = new Element[predParamTypeCount];

	Vertex actVertex;
	Iterator it1,it2;
	ElemMultiSet ems;
	ElemComparator ec = new ElemComparator();
	ElemMultiSet firstSet,secondSet:
	Element firstEl,secondEl;
	boolean predHolds = false;
	
	for (int vCount = 0; vCount < succLists.length; vCount++) {
	    while (succLists[vCount] != null && !succLists[vCount].isEmpty()) {
		actVertex = (Vertex)succLists[vCount].getFirst();
		
		firstSet = actVertex.value;
		secondSet = actVerex.value;

		while (!secondSet.isEmpty()) {
		    it1 = firstSet.iterator();
		    it2 = secondSet.iterator();
		    
		    while(it1.hasNext()) {
			firstEl = (Element)it1.next();
			while (it2.hasNext()) {
			    secondEl = (Element)it2.next();
			    
			    //check whether method shall be applied
			    if (predParamTypeCount == 1) predParamList[0] = secondEl;
			    else {
				predParamList[0] = firstEl;
				predParamList[1] = secondEl;
			    }//else
			    try {
				predHolds = ((Boolean)predicate.invoke(firstEl,predParamList)).booleanValue();
			    } catch (Exception e) {
				System.out.println("Error in Graph.reduce(): can't invoke predicate.");
				e.printStackTrace();
				System.exit(0);
			    }//catch
			    
			    if (predHolds) {
				
				//compute result of 'edge'
				if (paramTypeCount == 1) paramList[0] = secondEl;
				else {
				    paramList[0] = firstEl;
				    paramList[1] = secondEl;
				}//else
				try {
				    ems = new ElemMultiSet(ec);
				    if (metTypeElement)
					ems.add((Element)(method.invoke(firstEl,paramList)));
				    else {
					if (metTypeelemList) 
					    ems.addAll((ElemMultiSet)(method.invoke(firstEl,paramList)));
					else {
					    System.out.println("Error in Graph.reduce: can't invoke method");
					    System.exit(0);
					}//else
				    }//else
				} catch (Exception e) {
				    System.out.println("Exceptoin: "+e.getClass()+" --- "+e.getMessage());
				    System.out.println("Error in Graph.reduce(). Can't invoke method "+method);
				    e.printStackTrace();
				    System.exit(0);
				}//catch
			    }//predHolds
			    
			    //result is computed for one pair of elements of the two sets (if predHolds = true)
			    //remove both used elements from sets
			    if (predHolds) {
				it1.remove();
				it2.remove();
				break;
			    }//if
			}//while it2
		    }//while it1
		}//while 
		*/

}//end class Graph
