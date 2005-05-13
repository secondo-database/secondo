/*
 * ConnectedComponentsPair.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.graph;

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.util.*;


/**
 * An instance of the ConnectedComponentsPair class represents the connected components of a graph. The graph itself is an instance of the
 * {@link twodsack.util.graph} class. A ConnectedComponentsPair instance consists of two lists. The first list is a LinkedList named
 * compVertices, which has as elements MultiSet(s) of vertices belonging to one connected component. The second list is a LinkedList with
 * the name compEdges, which consists of MultiSets of edges belonging to the connected components. Both lists have always the same length
 * and they are somehow connected: If there is only one entry in compVertices' element0, compEdges element0 cannot have an entry. But, for
 * two entries in comVertices' element1, compEdges' element1 must have exactly one entry.
 */
public class ConnectedComponentsPair {
    /*
     * fields
     */
    public LinkedList compVertices;
    public LinkedList compEdges;

    /*
     * constructors
     */
    /**
     * Constructs an 'empty' instance.
     * All fields are initialized to empty LinkedList(s).
     */
    public ConnectedComponentsPair() {
	this.compVertices = new LinkedList();
	this.compEdges = new LinkedList();
    }


    /**
     * Constructs a new instance using the passed LinkedList(s).
     *
     * @param v the list of MultiSets for connected components for vertices
     * @param e the list of MultiSets for connected components for edges 
     */
    public ConnectedComponentsPair(LinkedList v, LinkedList e) {
	this.compVertices = v;
	this.compEdges = e;
    }

    
    /*
     * methods
     */
    /**
     * Prints the data of <i>this</i> to standard output.
     */
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

    
    /**
     * Converts the list of MultiSet(s) for vertices to an ElemMultiSetList.
     * 
     * @return the converted list 
     */
    public ElemMultiSetList verticesToEMSList() {
	ListIterator lit1 = compVertices.listIterator(0);
	Iterator lit2;
	MultiSet actCC;
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


    /**
     * Converts the list of MultiSet(s) for edges to a PairMultiSetList.
     *
     * @return the converted list
     */
    public PairMultiSetList edgesToPairListList() {
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
