import java.util.*;

class TriList extends ElemList {
  
  //members

  //constructors

  //methods
  public ElemList copy(){
    TriList copy = new TriList();
    for (int i = 0; i < this.size(); i++) {
      copy.add(((Triangle)this.get(i)).copy());
    }//for
    return copy;
  }//end method copy

    public void print() {
	//prints out all elements
	for (int i = 0; i < this.size(); i++) {
	    ((Triangle)this.get(i)).print();
	}//for i
	if (this.size() == 0) {
	    System.out.println("TriList is empty.");
	}//if
	System.out.println();
    }//end method print


    public static TriList convert(ElemList el) {
	//converts an ElemList to a TriList
	TriList retList = new TriList();
	/*
	for (int i = 0; i < el.size(); i++) {
	    retList.add((Triangle)el.get(i));
	}//for i
	*/
	retList.addAll(el);
	return retList;
    }//end method convert

}//end class TriList
