import java.util.*;
import java.io.*;

class PairList extends LinkedList implements Serializable {
  
  //members

  //constructors

  //methods
    public PairList copy() {
	PairList copy = new PairList();
	for (int i = 0; i < this.size(); i++) {
	    copy.add(((ElemPair)this.get(i)).copy());
	}//for
	return copy;
    }//end method copy

    public void print() {
	if (this.isEmpty()) { System.out.println("list is empty"); }
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("Element "+i);
	    if (((ElemPair)this.get(i)).first != null) ((ElemPair)this.get(i)).first.print();
	    else System.out.println("element is NULL");
	    if (((ElemPair)this.get(i)).second != null)((ElemPair)this.get(i)).second.print();
	    else System.out.println("element is NULL");
	}//for i
    }//end method print


    protected void twistElements() throws WrongTypeException {
	//checks every ElemPair whether the first or
	//second Element is smaller regarding compX
	//CAUTION: works only if both Elements are of the same type
	byte result;
	for (int i = 0; i < this.size(); i++) {
	    Element el1 = ((ElemPair)this.get(i)).first;
	    Element el2 = ((ElemPair)this.get(i)).second;
	    result = el1.compX(el2);
	    if (result == 0) {
		result = el1.compY(el2);
	    }//if
	    switch (result) {
	    case -1 : break;
	    case 0 : break;
	    case 1 : this.set(i,new ElemPair(el2,el1));
	    }//switch
	}//for i
    }//end method twistElements

}//end class PairList
