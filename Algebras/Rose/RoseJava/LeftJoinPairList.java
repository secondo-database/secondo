import java.util.*;

class LeftJoinPairList extends ElemList {

    //members

    //constructors

    //methods
    public ElemList copy(){
	LeftJoinPairList copy = new LeftJoinPairList();
	for (int i = 0; i < this.size(); i++) {
	    copy.add(((LeftJoinPair)this.get(i)).copy());
	}//for
	return copy;
    }//end method copy

    public void print(){
	//prints out the list's elements
	for (int i = 0; i < this.size(); i++) {
	    System.out.println("\nElement "+i+":");
	    ((LeftJoinPair)this.get(i)).element.print();
	    if (((LeftJoinPair)this.get(i)).elemList.isEmpty()) {
		System.out.println("list is empty"); }
	    else {
		for (int j = 0; j < ((LeftJoinPair)this.get(i)).elemList.size(); j++) {
		    System.out.println("Element "+j+" of list, size:"+((LeftJoinPair)this.get(i)).elemList.size());
		    ((Element)((LeftJoinPair)this.get(i)).elemList.get(j)).print();
		}//for j
	    }//else
	}//for i
    }//end method print

}//end class LeftJoinPairList
