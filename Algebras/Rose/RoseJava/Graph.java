import java.util.*;

class Graph {
    //implements an undirected graph
    //the graph's vertices are elements, therefore v is of type ElemList

    //CAUTION: this class doesn't use Iterators and has some O(n2) algorithms
    //v,e eventually should be implemented as arrays!

    //members
    private ElemList v; //vertices
    private PairList e; //edges
    private Vertex[] vArr; //array of vertices
    private LinkedList[] succLists; //lists of successors for every vertex
    

    //constructors
    public Graph() {
	v = new ElemList();
	e = new PairList();
    }
    /*
    public Graph(PairList edges) {
	//v = vertices.copy();
	e = edges.copy();
	e = makeCleanList(e);
	buildSuccLists();
    }
    */
    public Graph(ElemList vertices,PairList edges) {
	v = vertices.copy();
	e = makeCleanList(edges.copy());
	buildSuccListsVE();
    }

    //methods
    private void buildSuccListsVE() {
	//build lists of successing vertices for every vertex
	//both lists, v and e, already exist
	//System.out.println("entering Graph.buildSuccListsVE...");

	succLists = new LinkedList[v.size()];
	for (int i = 0; i < succLists.length; i++) succLists[i] = new LinkedList();
	vArr = new Vertex[v.size()];
	
	//sort vertices and put them in vArr
	SetOps.quicksortX(v);
	ListIterator lit = v.listIterator(0);
	int count = 0;
	while (lit.hasNext()) {
	    vArr[count] = new Vertex((Element)lit.next(),count);
	    count++;
	}//while
	
	//now, go through edge-list and for every vertex in there
	//use binary search on vArr to find itself in vArr
	lit = e.listIterator(0);
	Element acte1;
	Element acte2;
	ElemPair actEP;
	while (lit.hasNext()) {
	    actEP = (ElemPair)lit.next();
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


    public ElemList vertices () {
	//returns the set of vertices
	return v.copy();
    }//end method vertices

    public PairList edges () {
	//returns the set of edges
	return e.copy();
    }//end method edges


    public ConnectedComponentsPair connectedComponents() {
	//returns the connected components of this
	
	//System.out.println();
	//System.out.println("connectedComponents calling...");

	boolean [] marks = new boolean [vArr.length];
	for (int i = 0; i < marks.length; i++) {
	    marks[i] = false;
	}//for i

	int actElem = 0;
	//ElemListList compListV = new ElemListList();
	LinkedList compListV = new LinkedList();
	//PairListList compListE = new PairListList();
	LinkedList compListE = new LinkedList();

	if (!(v.size() == 0)) {
	    //compute depth-first spanning trees
	    while (hasUnvisitedVertices(marks)) {
		//System.out.println("graph still has unvisited vertices...");
		LinkedList compV = new LinkedList();
		LinkedList compE = new LinkedList();
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
	//ccp.compVertices = compListV;
	//ccp.compEdges = compListE;
	
	return ccp;
    }//end method connectedComponentsV
    
      
    public void print () {
	//prints out the graph's data
	System.out.println("vertices: ");
	//v.print();
	for (int i = 0; i < vArr.length; i++) vArr[i].print();
	System.out.println();
	System.out.println("edges: ");
	e.print();
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
    

    private void depthFirst (int actElem, boolean[] marks, LinkedList compV, LinkedList compE) {
	marks[actElem] = true;
	compV.add(vArr[actElem].copy());
	//System.out.println("hasUnvisitedSon?");
	while (hasUnvisitedSon(actElem,marks)) {
	    int next = nextUnvisitedSon(actElem,marks);
	    compE.add(new Edge(vArr[actElem].copy(),vArr[next].copy()));
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


    private static PairList makeCleanList (PairList plIn) throws WrongTypeException {
	//since the PairList that is passed to Graph constructor
	//may have duplicates, which results in double edges
	//these duplicates are removed here

	//first, traverse Pairlist and twist all Pairs that way, that the smaller 
	//Element is at first position
	ListIterator lit = plIn.listIterator(0);
	while (lit.hasNext()) {
	    ElemPair actPair = (ElemPair)lit.next();
	    if (actPair.first.compare(actPair.second) == 1) actPair.twist();
	}//while
	
	//now sort PairList
	SetOps.quicksort(plIn);
	
	//traverse Pairlist and remove all neighbours which are equal
	lit = plIn.listIterator(0);
	ListIterator lit2;
	while (lit.hasNext()) {
	    ElemPair actPair = (ElemPair)lit.next();
	    lit2 = plIn.listIterator(lit.nextIndex());
	    while (lit2.hasNext() && ((ElemPair)lit2.next()).equal(actPair)) {
		lit2.remove();
		lit2 = plIn.listIterator(lit.nextIndex());
		lit = plIn.listIterator(lit.nextIndex()-1);
	    }//while
	}//while
	
	return plIn;
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
	LinkedList actEL;
	Edge actEdge;
	//scan every component
	while (litE.hasNext()) {
	    LinkedList elList = new LinkedList();
	    //set actEL as one component
	    actEL = (LinkedList)litE.next();
	    //traverse component
	    if (!actEL.isEmpty()) {
		ListIterator litE2 = actEL.listIterator(0);
		while (litE2.hasNext()) {
		    //get edge from component
		    actEdge = (Edge)litE2.next();
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
		    
		    /*OLD IMPLEMENTATION
		    boolean found1 = false;
		    boolean found2 = false;
		    ListIterator litV = v.listIterator(0);
		    Vertex actV;
		    boolean wasEmpty1 = false;
		    boolean wasEmpty2 = false;
		    int index1 = -1;
		    int index2 = -1;
		    while (litV.hasNext()) {
			actV = (Element)litV.next();
			if (!found1 && actV.equal(actPair.first)) {
			    found1 = true;
			    if (slCopy[litV.nextIndex()-1].isEmpty()) {
				wasEmpty1 = true; }
			    else {
				index1 = litV.nextIndex()-1; 
				//slCopy[litV.nextIndex()-1].clear();
			    }//else
			}//if
			if (!found2 && actV.equal(actPair.second)) {
			    found2 = true;
			    if (slCopy[litV.nextIndex()-1].isEmpty()) {
				wasEmpty2 = true; }
			    else { 
				index2 = litV.nextIndex()-1;
				//slCopy[litV.nextIndex()-1].clear();
			    }//else
			}//if
			if (found1 && found2) { break; }
		    }//while
		    */
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
	LinkedList actVL;
	litE = retCCP.compEdges.listIterator(0);
	litV = ccp.compVertices.listIterator(0);
	if (!ccp.compEdges.isEmpty()) {
	    while (litE.hasNext()) {
		actVL = (LinkedList)litV.next();
		actEL = (LinkedList)litE.next();
		//to be able to use the SetOps operations, the vertices
		//must be extracted from list of edges (litE) and list of
		//vertices (litV). After the first extraction, the elements must
		//be extracted from the vertices.
		
		//extract elements from list of edges
		ElemList elementsInEdges = new ElemList();
		ListIterator tempLIT = actEL.listIterator(0);
		actEdge = null;
		while (tempLIT.hasNext()) {
		    actEdge = (Edge)tempLIT.next();
		    elementsInEdges.add(actEdge.first.value);
		    elementsInEdges.add(actEdge.second.value);
		}//while

		//extract elements from list of vertices
		ElemList elementsInVertices = new ElemList();
		tempLIT = actVL.listIterator(0);
		Vertex actVertex;
		while (tempLIT.hasNext()) {
		    actVertex = (Vertex)tempLIT.next();
		    elementsInVertices.add(actVertex.value);
		}//while
		
		//ElemList eipl = SetOps.elementsInPairList(actPL);
		ElemList diff = SetOps.difference(elementsInVertices,elementsInEdges);
		//System.out.println("\nelement["+(litE.nextIndex()-1)+"]");
		//System.out.println("eipl:"); eipl.print();
		//System.out.println("diff:"); diff.print();
		
		//now transform the ElemList back to a list of vertices
		tempLIT = diff.listIterator(0);
		LinkedList vertexList = new LinkedList();
		while (tempLIT.hasNext()) {
		    vertexList.add(new Vertex((Element)tempLIT.next(),-1)); }
		retCCP.compVertices.add(vertexList);
	    }//while
	}//if
	else { retCCP.compVertices.addAll(ccp.compVertices); }
	    
	
	//System.out.println("leaving G.cRP");
	return retCCP;
    }//end method computeReducedPair


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


	/* OLD IMPLEMENTATION
	ElemListList retList = new ElemListList();

	ElemList s = new ElemList();
	boolean[] vINs = new boolean[v.size()]; //if vi is in s, then v[i]=true
	int count1 = 1;
	ElemList current = new ElemList();
	Element actV = null;
	int[] dfsNum = new int[this.v.size()];
	int[] lowPT = new int[this.v.size()];
	int[] fatherList = new int[this.v.size()];
	for (int i = 0; i < dfsNum.length; i++) {
	    dfsNum[i] = -1;
	    lowPT[i] = -1;
	    fatherList[i] = -1;
	    vINs[i] = false;
	}//for i
	
	for (int i = 0; i < this.v.size(); i++) {
	    if (!vINs[i]) {
		actV = (Element)this.v.get(i);
		s.add(actV);
		vINs[i] = true;
		fatherList[i] = 0;
		current.add(actV);
		dfsNum[i] = count1;
		bccDFS(i,actV,count1,dfsNum,lowPT,s,vINs,current,retList,fatherList);
	    }//if
	}//for i
	*/
	return retList;
    }//end method computeBCCs


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
		

	/*OLD IMPLEMENTATION
	lowPT[vNum] = dfsNum[vNum];
	Element actSucc = null;
	for (int i = 0; i < this.succLists[vNum].size(); i++) {
	    actSucc = (Element)this.succLists[vNum].get(i);
	    
	    boolean wINs = false;
	    for (int j = 0; j < s.size(); j++) {
		Element actSelem = (Element)s.get(j);
		if (actSelem.equal(actSucc)) {
		    wINs = true;
		    break;
		}//if
	    }//for j

	    int wNum = -1;
	    Element actElem = null;
	    for (int j = 0; j < this.v.size(); j++) {
		actElem = (Element)this.v.get(j);
		if (actElem.equal(actSucc)) {
		    wNum = j;
		    break;
		}//if
	    }//for j
	    if (wNum == -1) {
		System.out.println("Unrecoverable error in Graph.sccDfs() -- 1");
		System.exit(0);
	    }//if

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

	    //if ((dfsNum[wNum] < dfsNum[vNum]) && wInCur) {
	    if (dfsNum[wNum] < dfsNum[vNum]) {
		if (lowPT[vNum] > dfsNum[wNum]) lowPT[vNum] = dfsNum[wNum];
	    }//if
	}//for i

	if ((dfsNum[vNum] >= 2) && (lowPT[vNum] == dfsNum[fatherList[vNum]])) {
	    ElemList newBCC = new ElemList();
	    newBCC.add(v.get(fatherList[vNum]));
	    for (int i = 0; i < current.size(); i++) {
		Element actElem = (Element)current.get(i);
		int wNum = -1;
		Element actW = null;
		for (int j = 0; j < this.v.size(); j++) {
		    actW = (Element)this.v.get(j);
		    if (actW.equal(actElem)) {
			wNum = j;
			break;
		    }//if
		}//for j
		if (wNum == -1) {
		    System.out.println("Unrecoverable error in Graph.bccDfs() -- 3");
		    System.exit(0);
		}//if

		
		if (dfsNum[wNum] >= dfsNum[vNum]) {
		    newBCC.add(actW);
		    current.remove(i);
		    i--;
		}//if
	    }//for i
	    retList.add(newBCC);
	}//if
	*/
    }//end method bccDFS 
		    

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
	if (actV.value.equal(el)) return actPos;
	else {
	    if (actV.value.compare(el) == -1) return binarySearch(el,actPos+1,hi);
	    else return binarySearch(el,lo,actPos-1);
	}//else
    }//end method binarySearch


}//end class Graph
