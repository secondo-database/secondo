import java.util.*;

public class ElemList extends LinkedList{

    //members
    
    //constructors

    //methods
    public ElemList copy() {
	//returns a copy of this
	ElemList copy = new ElemList();
	Iterator it = this.listIterator(0);
	while (it.hasNext()) {
	    copy.add(((Element)it.next()).copy());
	}//while
	/*
	for (int i = 0; i < this.size();i++) {
	    copy.add(((Element)this.get(i)).copy());
	}//for
	*/
	return copy;
    }//end method copy
    

    public void print() {
	//prints out this
	for (int i = 0; i < this.size(); i++) {
	    ((Element)this.get(i)).print();
	}//for i
	if (this.size() == 0) {
	    System.out.println("List is empty.\n");
	}//if
    }//end method print


}//end class ElemList
