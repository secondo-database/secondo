import java.util.*;
import java.io.*;

public class ElemListListList extends LinkedList implements Serializable {
    
    //members
    
    //constructors

    //methods
    public void print() {
	//prints out this
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("\nElemList["+i+"]: ");
	    ((ElemListList)this.get(i)).print();
	}//for i
    }//end method print

}//end class ElemListListList
