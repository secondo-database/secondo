import java.util.*;
import java.io.*;

public class ElemListList extends LinkedList implements Serializable {

    //members
    
    //constructors

    //methods
    public ElemListList copy() {
	//returns a copy of this
	ElemListList copy = new ElemListList();
	for (int i = 0; i < this.size(); i++) {
	    ElemList nl = new ElemList();
	    nl = ((ElemList)this.get(i)).copy();
	    copy.set(i,nl.copy());
	}//for i
	return copy;
    }//end method copy

    public void print() {
	//prints out this
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("Element["+i+"]: ");
	    ((ElemList)this.get(i)).print();
	}//for i
    }//end method print

}//end class ElemListList
