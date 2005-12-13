/*
 * Graph.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.graph;

import java.io.*;
import twodsack.io.*;
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
import java.util.Stack;


/**
 * This class implements an undirected graph. The graph's vertices are of type {@link twodsack.setelement.Element}. The set of vertices <i>V</i>
 * is of type {@link twodsack.set.ElemMultiSet}. Additionally, the set of vertices is stored as Vertex array. The successors for vertices
 * are stored in adjacency lists, which are implemented as {@link java.util.LinkedList}s, here.<p>
 * The most interesting methods of this class are {@link #computeFaces()}, which computes a set of faces for a graph, and {@link #connectedComponents()}.
 * The latter computes the connected components of a graph and stores them in a {@link ConnectedComponentsPair}.<p>
 * Note, that the vertices for this graph can hold any type of objects that extends the Element class.
 */
public class Graph {
    /*
     * fields
     */
    static final int NUMBER_OF_BUCKETS = 499;

    private ElemMultiSet v; //vertices
    private Vertex[] vArr; //array of vertices
    private LinkedList[] succLists; //lists of successors for every vertex

    /**
     * Stored in this array is the same set of vertices as in vArr. Unlike <tt>vArr</tt>, the vertices are sorted in ascending order.
     * The array has size n. This array is used to support the <tt>getVertex</tt> method, particularly it allows binary search.
     */
    private Vertex[] sortedVertices;
    /**
     * This array stores the indices which 'translate' from <tt>sortedVertices</tt> to <tt>vArr</tt>, i.e. the vertex V with with
     * sortedVertices[i] = V can be found in vArr[indexArray[i]].
     */
    private int[] indexArray;
    /**
     * This flag is true, if <tt>sortedVertices</tt> is defined.
     */
    private boolean sortedVerticesValid = false;

    /*
     * constructors
     */
    /**
     * Constructs an 'empty' graph.
     * All fields are initialized.
     */
    public Graph() {
	v = new ElemMultiSet(new ElemComparator());
	vArr = new Vertex[0];
	succLists = new LinkedList[0];
    }
 

    /**
     * Constructs a graph from a set of edges.
     * When using this constructor, the internal fields are set including the <tt>succLists</tt> field. The <tt>succLists</tt> are the list of
     * successors for each vertex. They are computed using a hash table.
     *
     * @param edges the set of edges
     */
    public Graph(SegMultiSet edges) {	
	ElemMultiSet cleanList = makeCleanList(edges);

	int initialCapacity = NUMBER_OF_BUCKETS;
	Hashtable pointsHT = new Hashtable(initialCapacity);
	
	//fill in the segments points as pointers to original points
	Iterator eit = cleanList.iterator();
	PointLink htEntry;
	Segment actSeg;
	int counter = 0;
	int count2 = 0;
	while (eit.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)eit.next()).value;
	    htEntry = new PointLink(actSeg.getStartpoint());
	    //check whether htEntry already exists in hash table
	    //if not, give it a new number and insert it
	    if (!pointsHT.containsKey(htEntry.linkedPoint)) {
	    htEntry.number = counter;
		counter++;
		pointsHT.put(htEntry.linkedPoint,htEntry);

		PointLink pl = (PointLink)pointsHT.get(actSeg.getStartpoint());
	    }//if
	    
	    //do the same with endpoint
	    htEntry = new PointLink(actSeg.getEndpoint());
	
	    if (!pointsHT.containsKey(htEntry.linkedPoint)) {
	    htEntry.number = counter;
		counter++;
		pointsHT.put(htEntry.linkedPoint,htEntry);

		PointLink pl2 = (PointLink)pointsHT.get(actSeg.getEndpoint());
	    }//if
	    count2++;
	}//while eit
	
	//construct list of vertices from hashtable
	Enumeration enumr = pointsHT.elements();
	vArr = new Vertex[counter];
	Object elem;
	while (enumr.hasMoreElements()) {
	    elem = enumr.nextElement();
	    vArr[((PointLink)elem).number] = new Vertex(((PointLink)elem).linkedPoint,((PointLink)elem).number);
	}//while enumr
	
	//construct succLists
	count2 = 0;
	succLists = new LinkedList[vArr.length];
	for (int i = 0; i < succLists.length; i++) succLists[i] = new LinkedList();
	eit = cleanList.iterator();
	int pointPosS;
	int pointPosE;
	while (eit.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)eit.next()).value;
	    //get the numbers of the segments' endpoints
	    pointPosS = ((PointLink)pointsHT.get(actSeg.getStartpoint())).number;
	    pointPosE = ((PointLink)pointsHT.get(actSeg.getEndpoint())).number;
	    //set the proper vertices in succLists
	    succLists[pointPosS].add(vArr[pointPosE]);
	    succLists[pointPosE].add(vArr[pointPosS]);
	    count2++;
	}//while eit
    }//end constructor


    /**
     * Constructs a graph from sets of vertices and edges.
     * When using this constructor, the internal fields are set including the <tt>succLists</tt> field. The <tt>succLists</tt> are the
     * lists of successors for each vertex. They are computed using a hash table.<p>
     * Make sure, that the types of the objects stored in the vertices match the object types in the edges. Otherwise a 
     * {@link twodsack.setelement.datatype.WrongTypeException} will be thrown.
     *
     * @param vertices the set of vertices
     * @param edges the set of edges
     * @throws WrongTypeException if the object types of the vertices/edges doesn't match
     */
    public Graph(ElemMultiSet vertices,PairMultiSet edges) throws WrongTypeException {
	v = makeCleanList(vertices.copy());
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
	Enumeration enumr = vertexHT.elements();
	vArr = new Vertex[counter];
	ObjectLink elem;
	while (enumr.hasMoreElements()) {
	    elem = (ObjectLink)enumr.nextElement();
	    vArr[((ObjectLink)elem).number] = new Vertex((Element)elem.linkedObject,((ObjectLink)elem).number);
	}//while enumr

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
		throw new WrongTypeException("Graph.constructor(...): An error occured while constructing the graph. Edges don't match to vertices.");
	    }//catch

	    //set the proper vertices in succLists
	    succLists[pos1].add(vArr[pos2]);
	    succLists[pos2].add(vArr[pos1]);
	}//while it
    }


    /**
     * Constructs a subgraph from vertices and edges.
     *
     * @param msV the set of vertices
     * @param msE the set of edges
     */
    protected Graph (MultiSet msV, MultiSet msE) {
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
	Enumeration enumr = vertexHT.elements();
	vArr = new Vertex[counter];
	ObjectLink elem;
	while (enumr.hasMoreElements()) {
	    elem = (ObjectLink)enumr.nextElement();
	    vArr[((ObjectLink)elem).number] = new Vertex((Element)elem.linkedObject,((ObjectLink)elem).number);
	}//while enumr

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


    /*
     * methods
     */
    /**
     * Returns the connected components of <i>this</i>.
     * The result of this method is stored in a {@link ConnectedComponentsPair}.
     *
     * @return the connected components as a ConnectedComponentsPair
     */
    public ConnectedComponentsPair connectedComponents() {
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
		MultiSet compV = new MultiSet();
		MultiSet compE = new MultiSet();
		actElem = getUnvisitedVertex(marks);
		depthFirst(actElem,marks,compV,compE);
		compListV.add(compV);
		compListE.add(compE);
	    }//while
	}//if
	else {
	    //System.out.println("Graph has no vertices.");
	}//else
	
	ConnectedComponentsPair ccp = new ConnectedComponentsPair(compListV,compListE);
	
	return ccp;
    }//end method connectedComponentsV
    
      
    /**
     * Prints the data of <i>this</i> to standard output.
     */
    public void print () {
	System.out.println("vertices: ");
	for (int i = 0; i < vArr.length; i++) vArr[i].print();
	System.out.println("\nedges: ");
	printSuccLists();
    }//end method print


    /**
     * Prints the adjacency lists to standard output.
     */
    public void printSuccLists() {
	for (int i = 0; i < vArr.length; i++) {
	    System.out.println("\nvertex: ");
	    vArr[i].print();
	    System.out.println("successors: ");
	    for (int j = 0; j < succLists[i].size(); j++) {
		((Vertex)succLists[i].get(j)).print(); }
	}//for i
    }//end method printSuccLists
    

    /**
     * Computes a depth first search on the graph and stores the result in the parameters.
     *
     * @param actElem the index of the actually visited vertex
     * @param marks an array which indicates which vertices were already visited
     * @param compV represents the actual connected component; stores the vertices of it
     * @param compE represents the actual connected component; stores the edges of it
     */
    private void depthFirst (int actElem, boolean[] marks, MultiSet compV, MultiSet compE) {
	marks[actElem] = true;

	//check, whether a vertex has a self-edge.
	//though this is not relevant for the connected component algo
	//it is needed for some algorithms in SetOps, e.g. overlapReduce.
	ListIterator lit = succLists[actElem].listIterator(0);
	while (lit.hasNext()) {

	    if (((Vertex)lit.next()).equal(vArr[actElem])) {
		compE.add(new Edge(vArr[actElem],vArr[actElem]));
	    }//if
	}//while

	compV.add(vArr[actElem].copy());
	while (hasUnvisitedSon(actElem,marks)) {
	    int next = nextUnvisitedSon(actElem,marks);
	    compE.add(new Edge(vArr[actElem],vArr[next]));
	    depthFirst(next,marks,compV,compE);
	}//while
    }//end method depthFirst


    /**
     * Returns true, if the given array has still FALSE buckets.
     *
     * @param marks an array which indicates which vertices were already visited
     */
    private boolean hasUnvisitedVertices (boolean [] marks) {
	for (int i = 0; i < marks.length; i++) {
	    if (!marks[i]) { return true; }
	}//for i
	return false;
    }//end method hasUnvisitedVertices
    

    /**
     * Returns the array index of the first FALSE bucket.
     * @param marks an array which indicates which vertices were already visited
     */
    private int getUnvisitedVertex(boolean [] marks) {
	for (int i = 0; i < marks.length; i++) {
	    if (!marks[i]) { return i; }
	}//for i
	return -1;
    }//end method getUnvisitedVertex


    /**
     * Returns true, if a vertex has a successor which wasn't visited yet.
     *
     * @param actElem the index of the vertex
     * @param marks an array which indicates which vertices were already visited
     */
    private boolean hasUnvisitedSon(int actElem, boolean[] marks) {
	ListIterator lit = succLists[actElem].listIterator(0);
	while (lit.hasNext()) {
	    Vertex actSucc = (Vertex)lit.next();
	    if (!marks[actSucc.number]) return true;
	}//while

	return false;
    }//end method hasUnvisitedSon


    /**
     * Returns the next index of a vertex which wasn't visited yet.
     *
     * @param actElem the index of the vertex
     * @param marks an array which indicates which vertices were already visited
     */
    private int nextUnvisitedSon (int actElem, boolean[] marks) {
	ListIterator lit = succLists[actElem].listIterator(0);
	while (lit.hasNext()) {
	    Vertex actSucc = (Vertex)lit.next();
	    if (!marks[actSucc.number]) return actSucc.number;
	}//while
	
	return -1;
    }//end method nextUnvisitedSon


    /**
     * Removes all duplicates from the given set.
     * The PairMultiSet that is passed to the Graph constructor may have duplicates. That would result in duplicate edges. 
     * Since no duplicate edges are allowed in the graph representation, they are removed here.
     *
     * @param plIn the set of edges as PairMultiSet
     * @return the 'clean' set
     * @throws WrongTypeException is thrown by the compare() method for Element
     */
    private static PairMultiSet makeCleanList (PairMultiSet plIn) throws WrongTypeException {
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

	PairMultiSet retSet = new PairMultiSet(new ElemPairComparator());
	retSet.addAll(plIn);
	//remove duplicates manually
	Iterator it = retSet.iterator();
	while (it.hasNext())
	    ((MultiSetEntry)it.next()).number = 1;
      
	return retSet;
    }//end method makeCleanList
		

    /**
     * Removes all duplicates from the given set.
     * The ElemMultiSet that is passed to the Graph constructor may have duplicates. That would result in duplicate vertices. Since no duplicates
     * are allowed in the graph representation, they are removed here.
     *
     * @param elIn the set of vertices
     * @return the 'clean' set
     * @throws WrongTypeException is thrown by the compare() method of Element
     */
    private static ElemMultiSet makeCleanList (ElemMultiSet elIn) throws WrongTypeException {
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
	
	return retSet;
    }//end method makeCleanList


    /**
     * Computes a reduced pair for a connected components.
     * For every component, the vertices of <tt>compVertices</tt> may occur in more than one edge in the appropriate <tt>compEdges</tt>.
     * In the result of this method
     * this is no longer true. Example: A ConnectedComponentsPair could be:<p>
     * <tt>compVertices: (A,B,C,D)<br>
     * compEdges: (A-B, B-C, C-D)</tt><p>
     * As you can see, some of the vertices of <tt>compVertices</tt> occur more than once in <tt>compEdges</tt>. Now, a reduced pair is
     * computed. One possible result could be:<p>
     * <tt>compVertices: ()<br>
     * compEdges: (A-B, C-D)</tt><p>
     * Another result is:<p>
     * <tt>compVertices: (A,D)<br>
     * compEdges: (B-C)</tt><p>
     * All of the vertices that are not used in the reduced set of edges are stored in <tt>compVertices</tt>.
     *
     * @param ccp the 'in' pair
     * @return the reduced pair
     */
    public ConnectedComponentsPair computeReducedPair (ConnectedComponentsPair ccp) {
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
			elList.add(actEdge); }			
		}//while
	    }//if
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
		
		ElemMultiSet diff = SetOps.difference(elementsInVertices,elementsInEdges);
		
		//now transform the ElemList back to a list of vertices
		tempLIT = diff.iterator();
		MultiSet vertexList = new MultiSet();
		while (tempLIT.hasNext()) {
		    vertexList.add(new Vertex((Element)((MultiSetEntry)tempLIT.next()).value,-1)); }
		retCCP.compVertices.add(vertexList);
	    }//while
	}//if
	else { retCCP.compVertices.addAll(ccp.compVertices); }
	    
	return retCCP;
    }//end method computeReducedPair


    /**
     * Returns the faces of this graph.
     * Here, the edges of the graph are interpreted as segments which bound a certain area. The graph's vertices are assumed to be points.
     * Then, the graph is a representation of a polygon which has faces. The face cycles of this polygon are stored as sets of segments
     * in {@link twodsack.set.ElemMultiSet}s.
     * For each face, such a ElemMultiSet exist. Since the faces are computed beginning from the outside, the outmost face cycle is the
     * first cycle in the result list.
     *
     * @return the set of cycles representing the faces
     */
    public ElemMultiSetList computeFaces(){
	ElemMultiSetList retList = new ElemMultiSetList();
	CycleList cl = computeFaceCycles();
	
	//store all cycles of cl in retList
	for (int i = 0; i < cl.size(); i++) {
	    SegMultiSet sms = SegMultiSet.convert(SupportOps.convert(((LinkedList)cl.get(i))));
	    //retList.add(SegMultiSet.convert((LinkedList)cl.get(i)));
	    retList.add(sms);
	}//for i
	return retList;
    }//end method computeFaces
    

    /**
     * Returns the faces of this graph.
     * Here, the edges of the graph are interpreted as segments which bound a certain area. The graph's vertices are assumed to be points.
     * Then, the graph is a representation of a polygon which has faces. The face cycles of this polygon are stored as sets of segments
     * in {@link twodsack.util.collection.CycleList}s.<p>
     * Since the faces are computed beginning from the outside, the outmost face cycle is the
     * first cycle in the result list.
     *
     * @return the set of cycles representing the faces
     */
    public CycleList computeFaceCycles(){
	//first, sort the vertices in succLists such that
	//the segments (which are formed by the pairs of vertices)
	//are sorted as described in the ROSE implementation paper
	//-> sorting of halfsegments.
	//here, only the vertices are sorted. More is not needed
	//(especially no construction of halfsegments).
	
	//System.out.println("\nEntering Graph.computeFaceCycles.");

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

	/* find cycles */
	//loop while edges exist
	while (thereAreEdgesLeft(succListsCOP)) {
	    //find starting point: Select leftmost and downmost point
	    Vertex startVertex = getStartVertex(succListsCOP);
	    
	    //System.out.println("startVertex: "+startVertex);

	    //call sub-routine findCycles
	    findCycles(succListsCOP,startVertex,retList);
	}//while
	
	return retList;
    }//end method computeFaceCycles


    /**
     * Returns the postion of vert in vertList.
     *
     * @param vert a vertex
     * @param vertList a list of vertices
     * @return the positon as int
     * @throws NoSuchElementException if the vertex cannot be found
     */
    private static int getPosOfVertex(Vertex vert, LinkedList vertList) throws NoSuchElementException {
	ListIterator lit = vertList.listIterator(0);
	while (lit.hasNext()) {
	    Vertex actV = (Vertex)lit.next();
	    if (actV.equal(vert)) return lit.nextIndex()-1;
	}//while
	//an error ocurred, print error messages
	System.out.println("\nVertex list: ");
	lit = vertList.listIterator(0);
	if (vertList.isEmpty())
	    System.out.println("Graph.getPosOfVertex: vertList is empty!");
	else {
	    while (lit.hasNext())
		((Element)((Vertex)lit.next()).value).print();
	}//else
	throw new NoSuchElementException(vert+" cannot be found in vertex list. Typically, this error is a precision error. Change the value for DerivDouble in Algebra Initialization.");
    }//end getPosOfVertex


    /**
     * Computes a set of Segments from an object cycle.
     * Works only for vertices of type Point.
     *
     * @param inCycle a cycle with vertices of type Point
     * @return the cycle converted to a SegMultiSet
     */
    private static SegMultiSet computeSMSFromCycle(LinkedList inCycle) {
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


    /**
     * Computes a set of segments from a cycle of Point(s).
     * If inCycle is a list of the form (A-B-C-D-A), new segments are constructed such, that the result is
     * (A-B)(B-C)(C-D)(D-A).
     * 
     * @param inCycle the cycle of Point(s)
     * @return the cycle of segments
     */
    private static LinkedList computeSegListFromCycle(LinkedList inCycle) {
	LinkedList retList = new LinkedList();

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

	    
    /**
     * Returns the leftmost and downmost vertex (w.r.t. the coordinates).
     *
     * @param sL the adjacencyc list of <i>this</i>
     */
    private Vertex getStartVertex(LinkedList[] sL) {
	//since vArr is sorted originally, we can take the first
	//vertex which has successors, i.e. outgoing edges.
	
	for (int i = 0; i < sL.length; i++) {
	    if (!sL[i].isEmpty()) { return vArr[i]; }
	}//for i
	return null;
    }//end method getStartVertex


    /**    
     * Returns true, if there is at least one vertex in sL which has a successor, i.e. an outgoing edge.
     *
     * @param sL the adjacency lists
     * @return true, if there is such a vertex
     */
    private static boolean thereAreEdgesLeft(LinkedList[] sL) {
	for (int i = 0; i < sL.length; i++) {
	    if (!sL[i].isEmpty()) { return true; }
	}//for i
	
	return false;
    }//end method thereAreEdgesLeft


    /**
     * Returns a list with all (other) vertices of elements of <i>edges</i>.
     * Example: edges ((A-B)(B-C)(B-D)) x = B. Then, the result is (A,C,D)
     * @param edges the list of edges
     * @param x the vertex x
     * @return the list of 'other' vertices
     */
    private static LinkedList extractVerticesOtherThanX(LinkedList edges, Vertex x) {
	LinkedList retList = new LinkedList();
	
	ListIterator lit = edges.listIterator(0);
	while (lit.hasNext()) {
	    retList.add(((Edge)lit.next()).theOtherOne(x));
	}//while

	return retList;
    }//end method extractVerticesOtherThanX


    /**
     * Returns the sorted paramter list.
     * The elements are sorted using the order for halfsegments (described in the ROSE implementation paper).
     *
     * @param edges the list of edges
     * @return the sorted list
     */    
    private static LinkedList sortEdgeListWithRespectToHalfSegmentsOrder(LinkedList edges) {
	LinkedList retList = edges;
	
	Edge min = null;
	int minPos = 0;
	boolean found = false;
	int edgessize = edges.size();

	for (int i = 0; i < edgessize-1; i++) {	    
	    min = (Edge)edges.get(i);
	    found = false;
	    for (int j = i+1; j < edgessize; j++) {
		Edge actJ = (Edge)edges.get(j);
		int cmp = min.compare(actJ);
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
		    
	return retList;
    }//end method sortEdgeListWithRespectToHalfSegmentsOrder
 

    /**
     * Supportive method for computeFaceCycles. Computes all cycles connected to the starting point.
     * At least one new cycle is added to <tt>retlist</tt> for every call of <tt>findCycles</tt>.
     * 
     * @param succLists is a copy of the adjacency lists for the graph; may be modified
     * @param startVertex the point from where the search for cycles begins
     * @param retList all found cycles are added to this list
     */
    private void findCycles (LinkedList[] succLists, Vertex startVertex, CycleList retList) {
	//The algorithm for finding cycles works as follows: First, find a starting vertex. This is the
	//bottom left vertex. Then, follow the edges from there. The next edge is given by chosing the
	//_predecessor_ in the succLists (or the next vertex resp.). Remove the edge from succLists and
	//mark the vertex as visited (in the visited array). Push all visited vertices on the stack.
	//When meeting a vertex again (entry in visited array is TRUE), remove all entries from the stack
	//until the actual vertex is found. Store all these vertices as new cycle. Then continue with
	//following edges.
	//Note: All edges must be removed twice, since every edge has two entries in succLists.

	Stack stack = new Stack();
	//initialize visited
	boolean[] visited = new boolean[succLists.length];
	for (int i = 0; i < visited.length; i++)
	    visited[i] = false;

	//push startVertex on stack, set visited
	
	stack.push(startVertex);
	visited[startVertex.number] = true;

	Vertex stackVertex = null;

	//follow edges until no more edges exist
	Vertex actVertex = startVertex;
	Vertex nextVertex = null;
	Vertex prevVertex = null;
	boolean alreadySet = false;
	do {
	    //get next Vertex
	    if (!alreadySet)
		nextVertex = getPredecessor(succLists,actVertex,prevVertex);
	    else
		alreadySet = false;
	
	    //remove edge by removing both involved vertices from succLists    
	    //...
	    if (visited[nextVertex.number]) {
		//finished another cycle, remove it from stack and store it
		LinkedList newCycle = new LinkedList();
		newCycle.add(nextVertex);
		
		//find next vertex before the cycle vertices are deleted
		Vertex nextVertexS = getPredecessor(succLists,nextVertex,actVertex);
		
		alreadySet = true;

		//pop elements form stack until element is equal to nextVertex
		do {
		    stackVertex = (Vertex)stack.pop();
		    newCycle.add(stackVertex);
		} while (!stackVertex.equal(nextVertex));
		
		//push nextVertex on stack again
		stack.push(nextVertex);

		//add new cycle to retList
		retList.add(computeSegListFromCycle(newCycle));

		//remove edges from succLists
		for (int i = 0; i < newCycle.size()-1; i++)
		    removeVertices(succLists,(Vertex)newCycle.get(i),(Vertex)newCycle.get(i+1));
		
		actVertex = nextVertex;
		nextVertex = nextVertexS;
	       
	    } else {
		//set visited for nextVertex
		visited[nextVertex.number] = true;
		//push nextVertex on stack
		stack.push(nextVertex);
		
		prevVertex = actVertex;
		actVertex = nextVertex;
	    }//else    	    
	} while (succLists[actVertex.number].size() > 0);
    }//end method findCycles


    /**
     * Supportive method for findCycles. Removes vertices from succList.
     * Given, the succLists and two vertices, this method removes the first vertex from the successor list of the other
     * one and vice versa.
     *
     * @param succLists the successor lists
     * @param firstVertex one of the vertices
     * @param secondVertex the other vertex
     */
    private void removeVertices (LinkedList[] succLists, Vertex firstVertex, Vertex secondVertex) {
	Iterator it;
	Vertex actVertex;
	//remove firstVertex from list of secondVertex
	it = succLists[secondVertex.number].iterator();
	while (it.hasNext()) {
	    actVertex = (Vertex)it.next();
	    if (actVertex.equal(firstVertex)) it.remove();
	}//while
		
	//remove secondVertex from list of firstVertex
	it = succLists[firstVertex.number].iterator();
	while (it.hasNext()) {
	    actVertex = (Vertex)it.next();
	    if (actVertex.equal(secondVertex)) it.remove();
	}//while
    }//end method removeVertices


    /**
     * Supportive method for findCycles. Returns the vertex which is the predecessor of a certain vertex from <tt>succLists</tt>.
     * Note: A special case occurs, if <tt>prevVertex</tt> = null. Then, the first vertex of the appropriate <tt>succLists</tt> is returned.
     * @param succLists the successor lists
     * @param actVertex the actual vertex
     * @param prevVertex the previous vertex
     * @return the vertex which is the predecessor of <tt>actVertex</tt>
     */
    private Vertex getPredecessor(LinkedList[] succLists, Vertex actVertex, Vertex prevVertex) {
	int sLaV = succLists[actVertex.number].size();
	//special case:
	if ((prevVertex == null) || (sLaV == 1))
	    return (Vertex)succLists[actVertex.number].getFirst();
	
	//find actVertex in list
	int pos = -1;
	Vertex sVertex;
	//get position of prevVertex
	for (int i = 0; i < sLaV; i++) {
	    sVertex = (Vertex)succLists[actVertex.number].get(i);
	    if (sVertex.equal(prevVertex)) pos = i;
	}//for i
	
	//return predecessor
	return (Vertex)succLists[actVertex.number].get((sLaV+pos-1)%sLaV);
    }//end method getPredecessor


    /**
     * For a given Point instance, this method returns the Vertex of the graph.
     * Note: Works only, if the vertices of <tt>this</tt> store Element types.
     *
     * @param the element
     * @return the vertex for the element
     * @throws WrongTypeException if vertices don't store Element types
     */
    public Vertex getVertex(Element queryElement) throws WrongTypeException {
	//check for correct type
	if (!(vArr[0].value instanceof Element))
	    throw new WrongTypeException("Graph.getVertex: Graph has vertex types "+vArr[0].value.getClass()+". Class Element is needed instead.");

	//look whether sortedVertices already exists
	if (!sortedVerticesValid) {
	    //sort!
	    sortVertices();
	    this.sortedVerticesValid = true;
	}//if
	//binary search
	int pos = binarySearch(sortedVertices,0,vArr.length,queryElement);
	if (pos > -1) return vArr[indexArray[pos]];
	else {
	    System.out.println("Vertex wasn't found in vertex array.");
	    for (int i = 0; i < vArr.length; i++) 
		System.out.println("["+i+"] "+((Element)vArr[i].value));
	    System.exit(0);
	    return null;
	}//else
    }//end  method getVertex


    private int binarySearch(Vertex[] arr, int low, int high, Element el) {
	if (low > high)
	    return -1;
	else {
	    int m = (int)((low+high) / 2);
	    int comp = ((Element)arr[m].value).compare(el);
	    if (comp == 0) {
		return m;
	    }//if
	    else {
		if (comp == -1)
		    return binarySearch(arr,m+1,high,el);
		else
		    return binarySearch(arr,low,m-1,el);
	    }//else
	}//else
    }//end method binarySearch



    /**
     * Returns a sorted version of vArr.
     * Uses quicksort to sort the vertices. Stored in the second fields are the indexes for the vertices in vArr.
     */
    private void sortVertices() {
	//construct new sortedVertices array
	this.sortedVertices = new Vertex[vArr.length];
	this.indexArray = new int[vArr.length];
	for (int i = 0; i < vArr.length; i++) {
	    sortedVertices[i] = (Vertex)(vArr[i].copy());
	    indexArray[i] = i;
	}//for i
	
	//sort array
	quicksort(sortedVertices,0,sortedVertices.length-1,this.indexArray);
    }//end method sortVertices

    /**
     * This is the standard quicksort algorithm
     */
    private static void quicksort(Vertex[] arr, int i, int j,int[] idxArr) {
	int k, xIndex;
	if (i < j) {
	    xIndex = findX(arr,i,j);
	    if (xIndex != -1) {
		//DIVIDE
		k = partition(arr,i,j,(Element)arr[xIndex].value,idxArr);
		//CONQUER
		quicksort(arr,i,k-1,idxArr);
		quicksort(arr,k,j,idxArr);
		//MERGE - nothing
	    }//if
	}//if
    }//end method quicksort

    /**
     * This method is part of the quicksort algorithm.
     */
    private static int partition (Vertex[] arr, int i, int j, Element x, int[] idxArr) {
	int l = i;
	int r = j;
	while (l < r) {
	    while (((Element)arr[l].value).compare(x) == -1) l++;
	    while (x.compare((Element)arr[r].value) == -1) r--;
	    if (l < r) swap(arr,l,r,idxArr);
	}//while
	return l;
    }//end method partition


    /**
     * This method is part of the quicksort algorithm.
     */
    private static int findX(Vertex[] arr, int i, int j) {
	int k = i+1;
	while (k <= j && ((Element)arr[k].value).equal((Element)arr[k-1].value)) k++;
	if (k > j) return -1;
	else if (((Element)arr[k-1].value).compare((Element)arr[k].value) == -1)
	    return k;
	else return k-1;
    }//end method findX

    /**
     * This method is part of the quicksort algorithm.
     */
    private static void swap(Vertex[] arr, int l, int r, int[] idxArr) {
	Vertex tmpV = (Vertex)arr[l].copy();
	int tmpI = idxArr[l];
	arr[l] = arr[r];
	idxArr[l] = idxArr[r];
	arr[r] = tmpV;
	idxArr[r] = tmpI;
    }//end method swap
    

    /**
     * For a pair of vertices, this method returns another vertex, which is the 'successor' of these vertices.
     * For three vertices <tt>a,b,c</tt> with <tt>b,c</tt> neighbours of <tt>a</tt>, this method returns <tt>c</tt>
     * when called with <tt>b</tt> as queryVertex and <tt>a</tt> as prevVertex.<p>
     * Note: It is assumed, that a vertex has the degree 2.
     *
     * @param queryVertex the returned vertex is a neighbour of this vertex
     * @param prevVertex this vertex is another neighbour of queryVertex
     * @return the vertex which is a neighbour of queryVertex but which is not equal to prevVertex
     */
    public Vertex getNextVertex(Vertex queryVertex, Vertex prevVertex) {
	if (succLists[queryVertex.number].size() != 2) {
	    System.out.println("Graph.getNextVertex: Too many neighbour vertices. May have only two neighbours.");
	    System.exit(0);
	}//if
	//get both neighbour vertices from succLists
	Vertex v0 = (Vertex)succLists[queryVertex.number].get(0);
	Vertex v1 = (Vertex)succLists[queryVertex.number].get(1);
	
	//return correct vertex
	if (((Element)v0.value).equal((Element)prevVertex.value))
	    return v1;
	else
	    return v0;							
    }//end method getNextVertex


    /**
     * For a queryVertex this method returns an array of all neighbours of this vertex.
     *
     * @param queryVertex the vertex for which the neighbours are demanded
     * @return an array with the neighbour vertices
     */    
    public Vertex[] getNeighbours(Vertex queryVertex) {
	//construct new array
	Vertex[] resArr = new Vertex[succLists[queryVertex.number].size()];
       
	//store vertices in resArr
	for (int i = 0; i < resArr.length; i++)
	    resArr[i] = (Vertex)succLists[queryVertex.number].get(i);

	return resArr;
    }//end method getNeighbours

}//end class Graph
