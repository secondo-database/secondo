import java.util.*;

class PointList extends ElemList {

  //members

  //constructors

  //methods
  public ElemList copy(){
    PointList copy = new PointList();
    Iterator it = this.listIterator(0);
    while (it.hasNext()) {
	copy.add(((Point)it.next()).copy());
    }//while
    /*
    for (int i = 0; i < this.size(); i++) {
      copy.add(((Point)this.get(i)).copy());
    }//for
    */
    return copy;
  }//end method copy
    
    
    public void print () {
	//prints out all elements
	
	for (int i = 0; i < this.size(); i++) {
	    ((Point)this.get(i)).print();
	}//for i
	if (this.size() == 0) {
	    System.out.println("PointList is empty.");
	}//if
	System.out.println();
    }//end method print


    static public PointList convert(ElemList el) {
	//converts an ElemList to a PointList
	PointList retList = new PointList();
	/*
	for (int i = 0; i < el.size(); i++) {
	    retList.add((Point)el.get(i));
	}//for i
	*/
	retList.addAll(el);
	return retList;
    }//end method convert

    
    public int contains(Point p) {
	//returns the position of p if p element this
	//-1 otherwise
	for (int i = 0; i < this.size(); i ++) {
	    if (p.equal((Point)this.get(i))) {
		return i;
	    }//if
	}//for i
	return -1;
    }//end method contains


}//end class PointList
