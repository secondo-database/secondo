import java.util.*;

class SegList extends ElemList {

  //members

  //constructors

  //methods
  public ElemList copy(){
    SegList copy = new SegList();
    Iterator it = this.listIterator(0);
    while (it.hasNext()) {
	copy.add(((Segment)it.next()).copy());
    }//while
    /*
    for (int i = 0; i < this.size(); i++) {
      copy.add(((Segment)this.get(i)).copy());
    }//for
    */
    return copy;
  }//end method copy

  
  public static int pos (SegList inlist, Segment inseg) {
    //returns the position of inseg in inlist or -1 if not found
    int retpos = -1;
    
    ListIterator it = inlist.listIterator(0);
    while (it.hasNext()) {
	if (((Segment)it.next()).equal(inseg)) {
	    retpos = it.nextIndex()-1;
	    break;
	}//if
    }//while
    /*
    for (int i = 0; i < inlist.size(); i++) {
	if (((Segment)inlist.get(i)).equal(inseg)) {
	    retpos = i;
	    break;
	}//if
    }//for i
    */
    return retpos;
  }//end method pos
    
    
    public void print() {
	//prints out all elements
	for (int i = 0; i < this.size(); i++) {
	    ((Segment)this.get(i)).print();
	}//for i
	if (this.size() == 0) {
	    System.out.println("SegList is empty.");
	System.out.println();
	}//if
    }//end method print
    

    static public SegList convert(ElemList el) {
	//converts an ElemList to a SegList
	SegList retList = new SegList();
	/*
	for (int i = 0; i < el.size(); i ++) {
	    retList.add((Segment)el.get(i));
	}//for i
	*/
	retList.addAll(el);
	return retList;
    }//end method convert

    
    public void zoom (Rational fact) {
	ListIterator lit = this.listIterator(0);
	while (lit.hasNext()) {
	    ((Segment)lit.next()).zoom(fact);
	}//while
    }//end method zoom


    public PointList generatePointList () {
	//generates a PointList from this
	//as follows:
	//(a,b)(c,d)(d,e) -> (a,c,d)
	//this makes sense if this is a cycle:
	//(a,b)(b,c)(c,a) -> (a,b,c)
	PointList retList = new PointList();
	ListIterator lit = this.listIterator(0);
	Segment actSeg;
	while (lit.hasNext()) {
	    actSeg = (Segment)lit.next();
	    retList.add(actSeg.startpoint.copy());
	}//while
	return retList;
    }//end method generatePointList

    /* is not needed but works
    protected void rearrangeCycle () {
	//rearranges a cycle
	//e.g.: (b,a)(b,c)(a,c) -> (a,b)(b,c)(c,a)
	//this doesn't change the segment's position
	System.out.println("\nentering SL.rearrangeCycle...");
	this.print();
	ListIterator lit = this.listIterator(1);
	Point actPoint;
	Segment actSeg = (Segment)this.getFirst();
	if (PointSeg_Ops.isEndpoint(actSeg.startpoint,(Segment)this.get(1))) {
	    actPoint = actSeg.startpoint;
	    actSeg.turn();
	}//if
	else { actPoint = actSeg.endpoint; }
	
	while (lit.hasNext()) {
	    actSeg = (Segment)lit.next();
	    System.out.println("actSeg:"); actSeg.print();
	    System.out.println("actPoint:"); actPoint.print();
	    if (actSeg.startpoint.equal(actPoint)) {
		actPoint = actSeg.endpoint; }
	    else {
		actPoint = actSeg.startpoint;
		actSeg.turn();
		System.out.println("---> turn");
	    }//else
	}//while
    }//end method rearrangeCycle
    */

}//end class SegList
