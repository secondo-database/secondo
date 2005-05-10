package twodsack.util.graph;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.util.*;

public class Vertex implements ComparableMSE {
    //provides a vertex for class Graph

    //members
    //public Element value;
    public Object value;
    public int number;

    //constructors
    public Vertex(Element val, int num) {
	value = val;
	number = num;
    }

    //methods
    public void print() {
	System.out.print(number+": ");
	((Element)value).print();
    }//end method print

    public Vertex copy() {
	return new Vertex((Element)this.value,this.number);
    }//end method copy

    public boolean equal(Vertex inVer) {
	if (((Element)inVer.value).equal((Element)this.value)) return true;
	else return false;
    }//end method equal

    public int compare(ComparableMSE inVertex) {

	Vertex inV;

	if (!(inVertex instanceof Vertex)) throw new WrongTypeException("Expected "+this.getClass()+", found "+inVertex.getClass());
	else inV = (Vertex)inVertex;

	return ((Element)this.value).compare((Element)inV.value);
    }//end method compare


    public Object getValue() {
	return this.value;
    }//end method getValue

}//end class Vertex
    
