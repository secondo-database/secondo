import java.util.*;

public class PairListList extends LinkedList {
    
    //members

    //constructors

    //methods
    public void print() {
	//prints out this
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("element["+i+"]: ");
	    ((PairList)this.get(i)).print();
	}//for i
    }//end method print

}//end class PairListList
