/*
 * Vertex.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.graph;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.util.*;


/**
 * This class implements the Vertex type for {@link twodsack.util.graph}. It is the counterpart to the {@link twodsack.util.vertex}. A vertex
 * consist of a value field (where the object is stored) and a number. The number is an int value to identify the vertex. Only objects of type
 * Element can be stored inside of a vertex.
 */
public class Vertex implements ComparableMSE {
    /*
     * fields
     */
    public Object value;
    public int number;


    /*
     * constructors
     */
    /**
     * Constructs a new vertex from an Element type and a number.
     */
    public Vertex(Element val, int num) {
	value = val;
	number = num;
    }


    /**
     * methods
     */
    /**
     * Prints the data of <i>this</i> to the standard output.
     */
    public void print() {
	System.out.print(number+": ");
	((Element)value).print();
    }//end method print


    /*
     * Returns a shallow copy of <i>this</i>
     *
     * @return the copy
     */
    public Vertex copy() {
	return new Vertex((Element)this.value,this.number);
    }//end method copy


    /**
     * Returns true, if the objects stored in both Vertex instances are equal.
     * For the equality check the equal() method implemented for all objects of type Element is used.
     *
     * @param inVer the vertex to compare with
     * @return true, if both objects stored in the value fields are equal
     */
    public boolean equal(Vertex inVer) {
	if (((Element)inVer.value).equal((Element)this.value)) return true;
	else return false;
    }//end method equal


    /**
     * Compares both Vertex instances and returns one of {0, 1, -1}.
     * For the comparison the compare method implemented for all objects of type Element is used.<p>
     * Returns 0, if both objects are equal.<p>
     * Returns -1, if <i>this.value</i> is smaller than <i>inVertex.value</i>.<p>
     * Returns 1 otherwise
     *
     * @param inVertex the vertex to compare with
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if inVertex.value is not of type Vertex
     */
    public int compare(ComparableMSE inVertex) throws WrongTypeException {
	Vertex inV;

	if (!(inVertex instanceof Vertex)) throw new WrongTypeException("Expected "+this.getClass()+", found "+inVertex.getClass());
	else inV = (Vertex)inVertex;

	return ((Element)this.value).compare((Element)inV.value);
    }//end method compare

}//end class Vertex
    
