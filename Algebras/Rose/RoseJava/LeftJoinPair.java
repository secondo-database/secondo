class LeftJoinPair {

    //This is a class providing the result type
    //of the leftouterjoin operation.
    //It has two elements: a single object of type Element
    //and an ElemList

    //members
    public Element element;
    public ElemList elemList;

    //constructors
    LeftJoinPair() {};

    LeftJoinPair(Element el, ElemList ell) {
	element = el.copy();
	elemList = ell.copy();
    }

    //methods
    public LeftJoinPair copy() {
	//returns a copy of LeftJoinPair
	LeftJoinPair copy = new LeftJoinPair();
	copy.element = this.element.copy();
	copy.elemList = this.elemList.copy();
	return copy;
    }//end method copy

}//end class LeftJoinPair
