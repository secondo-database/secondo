import java.util.*;

class Graph {
    //implements an undirected graph
    //the graph's vertices are elements, therefore V is of type ElemList

    //CAUTION: this class doesn't use Iterators and has some O(n2) algorithms
    //v,e eventually should be implemented as arrays!

    //members
    private ElemList v; //vertices
    private PairList e; //edges
    //private ElemListList succLists;
    private ElemList[] succLists;
    

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
    /*
    private void buildSuccLists() {
	//builds lists of successing vertices for every vertex
	//build v by the way
	//CAUTION: has O(n2)?

	succLists = new ElemList[v.size()];
	ElemList verts = new ElemList();
	PairList ecop = (PairList)this.e.clone();
	//add successing vertices for every vertex in succLists
	while (!ecop.isEmpty()) {
	    //System.out.println("while...");
	    Element acte1 = ((ElemPair)ecop.getFirst()).first;
	    Element acte2 = ((ElemPair)ecop.getFirst()).second;
	    boolean added1 = false;
	    boolean added2 = false;
	    //search in verts whether the vertices are already added
	    //if yes, add the partner to succLists
	    for (int i = 0; i < verts.size();i++) {
		if ((!added1) && ((Element)verts.get(i)).equal(acte1)) {
		    ((ElemList)succLists.get(i)).add(acte2.copy());
		    added1 = true;
		}//if
		if ((!added2) && ((Element)verts.get(i)).equal(acte2)) {
		    ((ElemList)succLists.get(i)).add(acte1.copy());
		    added2 = true;
		}//if
		if (added1 && added2) { break; }
	    }//for
	    //if the vertices weren't found, add them
	    if (!added1) {
		verts.add(acte1.copy());
		ElemList nl = new ElemList();
		nl.add(acte2.copy());
		succLists.add(nl);
		//System.out.println("add1");
	    }//if
	    if (!added2) {
		verts.add(acte2.copy());
		ElemList nl = new ElemList();
		nl.add(acte1.copy());
		succLists.add(nl);
		//System.out.println("add2");
	    }//if

	    ecop.remove(0);
	}//while

	//build vertices v
	v = verts; 
    }//end method buildSuccLists
    */

    private void buildSuccListsVE() {
	//build lists of successing vertices for every vertex
	//in contrast to the buildSuccList we already have V
	//CAUTION: has O(n2)?
	//System.out.println("entering Graph.buildSuccListsVE...");

	succLists = new ElemList[v.size()];
	for (int i = 0; i < v.size(); i++) {
	    succLists[i] = new ElemList(); }
	PairList ecop = (PairList)this.e.clone();
	Element acte1;
	Element acte2;
	boolean added1 = false;
	boolean added2 = false;
	Element actV;
	ListIterator lit;
	int index = 0;
	//add successing vertices for every vertex in succLists
	while (!ecop.isEmpty()) {
	    acte1 = ((ElemPair)ecop.getFirst()).first;
	    acte2 = ((ElemPair)ecop.getFirst()).second;
	    added1 = false;
	    added2 = false;
	    lit = v.listIterator(0);
	    index = 0;
	    //search for the right vertex in V and add partner to succLists
	    while (lit.hasNext()) {
		actV = (Element)lit.next();
		if ((!added1) && actV.equal(acte1)) {
		    index = lit.nextIndex()-1;
		    succLists[index].add(acte2);
		    added1 = true;
		}//if
		if ((!added2) && actV.equal(acte2)) {
		    index = lit.nextIndex()-1;
		    succLists[index].add(acte1);
		    added2 = true;
		}//if
		if (added1 && added2) { break; }
	    }//while
	    if (!(added1 && added2)) {
		System.out.println("Error in Graph.buildSuccListsVE: didn't find vertex.");
		System.exit(0);
	    }//if
	    ecop.remove(0);
	}//while
	/*
	System.out.println("\nsuccLists:");
	for (int i = 0; i < succLists.length; i++) {
	    System.out.print("\n["+i+"]:"); ((Element)v.get(i)).print();
	    System.out.println("list:"); succLists[i].print();
	}//for
	*/

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

	boolean [] marks = new boolean [v.size()];
	for (int i = 0; i < marks.length; i++) {
	    marks[i] = false;
	}//for i

	int actElem = 0;
	ElemListList compListV = new ElemListList();
	PairListList compListE = new PairListList();

	if (!(v.size() == 0)) {
	    //compute depth-first spanning trees
	    while (hasUnvisitedVertices(marks)) {
		//System.out.println("graph still has unvisited vertices...");
		ElemList compV = new ElemList();
		PairList compE = new PairList();
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
    
    /*
    public PairListList connectedComponentsE() {
	//returns the connected components of this
	//as PairListList; the edges are returned

	boolean [] marks = new boolean [v.size()];
	for (int i = 0; i < marks.length; i++) {
	    marks[i] = false; }

	int actElem = 0;
	PairListList compListE = new PairListList();
	
	if (!(v.size() == 0)) {
	    //compute depth-first spanning trees
	    while (hasUnvisitedVertices(marks)) {
		PairList compE = new PairList();
		actElem = getUnvisitedVertex(marks);
		depthFirstE(actElem,marks,compE);
		compListE.add(compE);
	    }//while
	}//if
	else {
	    System.out.println("Graph has no vertices.");
	}//else
	
	return compListE;
    }//end method connectedComponentsE
    */
      
    public void print () {
	//prints out the graph's data
	System.out.println("vertices: ");
	v.print();
	System.out.println();
	System.out.println("edges: ");
	e.print();
    }//end method print

    private void depthFirst (int actElem, boolean[] marks, ElemList compV, PairList compE) {
	marks[actElem] = true;
	compV.add(((Element)v.get(actElem)).copy());
	//System.out.println("hasUnvisitedSon?");
	while (hasUnvisitedSon(actElem,marks)) {
	    int next = nextUnvisitedSon(actElem,marks);
	    compE.add(new ElemPair((Element)v.get(actElem),(Element)v.get(next)));
	    depthFirst(next,marks,compV,compE);
	}//while
    }//end method depthFirst
	
    /*
    private void depthFirstE (int actElem, boolean[] marks, PairList compE) {
	marks[actElem] = true;
	while (hasUnvisitedSon(actElem,marks)) {
	    int next = nextUnvisitedSon(actElem,marks);
	    compE.add(new ElemPair((Element)v.get(actElem),(Element)v.get(next)));
	    depthFirstE(next,marks,compE);
	}//while
    }//end method depthFirstE
    */

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
	//System.out.println("G.hUS actElem: "+actElem);
	//for (int i = 0; i < ((ElemList)succLists.get(actElem)).size(); i++) {
	for (int i = 0; i < succLists[actElem].size(); i++) {
	    for (int j = 0; j < v.size(); j++) {
		//System.out.println("i:"+i+" j:"+j);
		//if (((Element)((ElemList)succLists.get(actElem)).get(i)).equal(((Element)v.get(j)))) {
		if (((Element)succLists[actElem].get(i)).equal(((Element)v.get(j)))) {
		    if (!marks[j]) {
			//System.out.println("hasUnvisitedSon:true");
			return true;
		    }//if
		    //System.out.println("mark already set");
		}//if
	    }//for j
	}//for i
	//System.out.println("hasUnvisitedSon:false");
	return false;
    }//end method hasUnvisitedSon

    private int nextUnvisitedSon (int actElem, boolean[] marks) {
	//returns the index of the next unvisited son of vertex actElem
	//for (int i = 0; i < ((ElemList)succLists.get(actElem)).size(); i++) {
	for (int i = 0; i < succLists[actElem].size(); i++) {
	    for (int j = 0; j < v.size(); j++) {
		//if (((Element)((ElemList)succLists.get(actElem)).get(i)).equal(((Element)v.get(j)))) {
		if (((Element)succLists[actElem].get(i)).equal(((Element)v.get(j)))) {
		    if (!marks[j]) return j;
		}//if
	    }//for j
	}//for i
	System.out.println("Error in Graph.nextUnvisitedSon --- Has NO unvisited son.");
	System.exit(0);
	return -1;
    }//end method nextUnvisitedSon


    private static PairList makeCleanList (PairList plIn) throws WrongTypeException {
	//since the PairList that is passed to Graph constructor
	//may have duplicates, which results in double edges
	//these duplicates are removed here
	//BAD IMPLEMENTATION
	PairList pl = (PairList)plIn.clone();

	for (int i = 0; i < pl.size()-1; i++) {
	    for (int j = i+1; j < pl.size(); j++) {
		if (((ElemPair)pl.get(i)).equalOrInvertedEqual((ElemPair)(pl.get(j)))) {
		    pl.remove(j);
		    j--;
		}//if
	    }//for j
	}//for i
	return pl;
    }//end method makeCleanList
		
			
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

  
    public ConnectedComponentsPair computeReducedPair (ConnectedComponentsPair ccp) {
	//for an existing ccp compute the set of pairs in
	//ccp.compEdges such that no vertex in it appears
	//twice; all vertices, that are now isolated, are
	//added to ccp.compVertices
	//System.out.println("entering G.cRP...");

	//make a copy of succLists
	ElemList[] slCopy = new ElemList[succLists.length];
	for (int i = 0; i < succLists.length; i++) {
	    slCopy[i] = (ElemList)succLists[i].clone(); }
	
	ConnectedComponentsPair retCCP = new ConnectedComponentsPair();
	
	//traverse ccp.compEdges
	ListIterator litE = ccp.compEdges.listIterator(0);
	PairList actPL;
	ElemPair actPair;
	while (litE.hasNext()) {
	    PairList plList = new PairList();
	    actPL = (PairList)litE.next();
	    if (!actPL.isEmpty()) {
		//scan component
		ListIterator litE2 = actPL.listIterator(0);
		while (litE2.hasNext()) {
		    actPair = (ElemPair)litE2.next();
		    //scan slCopy for both elements in actPair and
		    //remove all successors.
		    //if one of the vertices has no successors,
		    //the elempair may not be added

		    //System.out.println("\nscan for:"); actPair.print();
		    //System.out.println("\nactual slCopy:");
		    //for (int i = 0; i < slCopy.length; i++) {
		    //	System.out.println("V:");
		    //	((Element)v.get(i)).print();
		    //	System.out.println("succList:");
		    //	slCopy[i].print();
		    //}//for
			
		    boolean found1 = false;
		    boolean found2 = false;
		    ListIterator litV = v.listIterator(0);
		    Element actV;
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
		    if (!(wasEmpty1 || wasEmpty2)) {
			//delete succLists
			slCopy[index1].clear();
			slCopy[index2].clear();
			//add actPair to retList
			//System.out.println("---> added pair");
			plList.add(actPair); }
		    //else {
			//System.out.println("---> didn't add pair: wasEmpty1:"+wasEmpty1+", wasEmpty2:"+wasEmpty2); }
			
		}//while
	    }//if
	    //if (!plList.isEmpty()) {
	    retCCP.compEdges.add(plList);
	}//while

	//now compute the new compV
	//extract used elements every compE and compute the difference
	//with old compV
	ListIterator litV;
	ElemList actEL;
	litE = retCCP.compEdges.listIterator(0);
	litV = ccp.compVertices.listIterator(0);
	if (!ccp.compEdges.isEmpty()) {
	    while (litE.hasNext()) {
		actEL = (ElemList)litV.next();
		actPL = (PairList)litE.next();
		ElemList eipl = SetOps.elementsInPairList(actPL);
		ElemList diff = SetOps.difference(actEL,eipl);
		//System.out.println("\nelement["+(litE.nextIndex()-1)+"]");
		//System.out.println("eipl:"); eipl.print();
		//System.out.println("diff:"); diff.print();
		retCCP.compVertices.add(SetOps.difference(actEL,eipl));
	    }//while
	}//if
	else { retCCP.compVertices.addAll(ccp.compVertices); }
	    
	
	//System.out.println("leaving G.cRP");
	return retCCP;
    }//end method computeReducedPair



}//end class Graph
