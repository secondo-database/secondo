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

import java.util.LinkedList;
import java.util.ListIterator;

class ConnectedComponentsPair {
    //This data structure represents the connected components of a graph.
    //It consists of two lists. The first list is an LinkedList named compVertices, 
    //which has a lists of vertices belonging to one connected component (cc) as elements.
    //The second list is a LinkedList named compEdges, which consists of lists of
    //edges belonging to the cc's.
    //
    //actually, the data structures look as follows:
    //compVertice:
    //ListOfVertices-ListOfVertices-ListOfVertices, so we have three cc's here
    //compEdges:
    //ListOfEdges-ListOfEdges-ListOfEdges, and we have also three cc's here

    //members
    LinkedList compVertices;
    LinkedList compEdges;

    //constructors
    ConnectedComponentsPair() {
	this.compVertices = new LinkedList();
	this.compEdges = new LinkedList();
    }

    ConnectedComponentsPair(LinkedList v, LinkedList e) {
	this.compVertices = v;
	this.compEdges = e;
    }

    //methods
    public void print() {
	System.out.println("\nconnected components");
	System.out.println("vertices:");
	for (int i = 0; i < compVertices.size(); i++) {
	    System.out.println("component "+i);
	    LinkedList actComp = (LinkedList)compVertices.get(i);
	    for (int j = 0; j < actComp.size(); j++) {
		((Vertex)actComp.get(j)).print();
	    }
	}//for i
	System.out.println("\nedges:");
	for (int i = 0; i < compEdges.size(); i++) {
	    System.out.println("component "+i);
	    LinkedList actComp = (LinkedList)compEdges.get(i);
	    for (int j = 0; j < actComp.size(); j++)
		((Edge)actComp.get(j)).print();
	}//for i
    }//print

    
    public ElemListList verticesToElemListList() {
	//extracts the data from LinkedList and returns it as ElemListList
	ListIterator lit1 = compVertices.listIterator(0);
	ListIterator lit2;
	LinkedList actCC;
	//Vertex actV;
	ElemList actEL;
	ElemListList retList = new ElemListList();
	
	while (lit1.hasNext()) {
	    actCC = (LinkedList)lit1.next();
	    lit2 = actCC.listIterator(0);
	    actEL = new ElemList();
	    while (lit2.hasNext()) actEL.add(((Vertex)lit2.next()).value);
	    retList.add(actEL);
	}//while

	return retList;
    }//end method verticesToElemListList


    public PairListList edgesToPairListList() {
	//extract the data from LinkedList and returns it as PairListList
	ListIterator lit1 = compEdges.listIterator(0);
	ListIterator lit2;
	LinkedList actCC;
	PairList actPL;
	PairListList retList = new PairListList();
	Edge actEdge;
	
	while (lit1.hasNext()) {
	    actCC = (LinkedList)lit1.next();
	    lit2 = actCC.listIterator(0);
	    actPL = new PairList();
	    while (lit2.hasNext()) {
		actEdge = (Edge)lit2.next();
		actPL.add(new ElemPair(actEdge.first.value,actEdge.second.value));
	    }//while
	    retList.add(actPL);
	}//while

	return retList;
    }//end method edgesToPairListList

}//end class ConnectedComponentsPair
