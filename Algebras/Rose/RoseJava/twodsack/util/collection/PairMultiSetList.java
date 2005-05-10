package twodsack.util.collection;

import twodsack.set.*;

import java.util.*;
import java.io.*;

public class PairMultiSetList extends LinkedList implements Serializable {
    
    //members

    //constructors

    //methods
    public void print() {
	//prints out this
	if (this.isEmpty()) System.out.println("PairMultiSetList is empty.\n");
	else {
	    for (int i = 0; i < this.size(); i++) {
		System.out.println("element["+i+"]: ");
		((PairMultiSet)this.get(i)).print();
	    }//for i
	}//else
    }//end method print

}//end class PairListList
