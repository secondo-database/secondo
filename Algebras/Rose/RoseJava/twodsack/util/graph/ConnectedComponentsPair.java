package twodsack.util.graph;

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.util.*;

public class ConnectedComponentsPair {
    //This data structure represents the connected components of a graph.
    //It consists of two lists. The first list is a LinkedList named compVertices, 
    //which has a MultiSet of vertices belonging to one connected component (cc) as elements.
    //The second list is a LinkedList named compEdges, which consists of MultiSets of
    //edges belonging to the cc's.
    //
    //actually, the data structures look as follows:
    //compVertice:
    //SetOfVertices-SetOfVertices-SetOfVertices, so we have three cc's here
    //compEdges:
    //SetOfEdges-SetOfEdges-SetOfEdges, and we have also three cc's here

    //members
    public LinkedList compVertices;
    public LinkedList compEdges;

    //constructors
    public ConnectedComponentsPair() {
	this.compVertices = new LinkedList();
	this.compEdges = new LinkedList();
    }

    public ConnectedComponentsPair(LinkedList v, LinkedList e) {
	this.compVertices = v;
	this.compEdges = e;
    }

    //methods
    public void print() {
	System.out.println("\nconnected components");
	System.out.println("vertices:");
	for (int i = 0; i < compVertices.size(); i++) {
	    System.out.println("component "+i);
	    Iterator it = ((MultiSet)compVertices.get(i)).iterator();
	    while (it.hasNext())
		((Vertex)((MultiSetEntry)it.next()).value).print();
	}//for i
	System.out.println("\nedges:");
	for (int i = 0; i < compEdges.size(); i++) {
	    System.out.println("component "+i);
	    Iterator it = ((MultiSet)compEdges.get(i)).iterator();
	    while (it.hasNext())
		((Edge)((MultiSetEntry)it.next()).value).print();
	}//for i
    }//print

    
    public ElemMultiSetList verticesToEMSList() {
	//extracts the data from LinkedList and returns it as ElemMultiSetList
	ListIterator lit1 = compVertices.listIterator(0);
	Iterator lit2;
	MultiSet actCC;
	//Vertex actV;
	ElemMultiSet actEL;
	ElemMultiSetList retList = new ElemMultiSetList();
	
	while (lit1.hasNext()) {
	    actCC = (MultiSet)lit1.next();
	    lit2 = actCC.iterator();
	    actEL = new ElemMultiSet(new ElemComparator());
	    while (lit2.hasNext()) actEL.add(((Vertex)((MultiSetEntry)lit2.next()).value).value);
	    retList.add(actEL);
	}//while

	return retList;
    }//end method verticesToEMSList


    public PairMultiSetList edgesToPairListList() {
	//extract the data from LinkedList and returns it as PairListList
	ListIterator lit1 = compEdges.listIterator(0);
	Iterator lit2;
	MultiSet actCC;
	PairMultiSet actPL;
	PairMultiSetList retList = new PairMultiSetList();
	Edge actEdge;
	
	while (lit1.hasNext()) {
	    actCC = (MultiSet)lit1.next();
	    lit2 = actCC.iterator();
	    actPL = new PairMultiSet(new ElemPairComparator());
	    while (lit2.hasNext()) {
		actEdge = (Edge)((MultiSetEntry)lit2.next()).value;
		actPL.add(new ElemPair((Element)actEdge.first.value,(Element)actEdge.second.value));
	    }//while
	    retList.add(actPL);
	}//while

	return retList;
    }//end method edgesToPairListList

}//end class ConnectedComponentsPair
