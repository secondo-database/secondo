class ElemPair {
  //members
  public Element first;
  public Element second;

  //constructors
  ElemPair(){
      first = null;
      second = null;
  };

  ElemPair(Element e1, Element e2) {
    first = (Element)e1.copy();
    second = (Element)e2.copy();
  }
  
  //methods
  public ElemPair copy(){
    //returns a copy of ElemPair
    ElemPair copy = new ElemPair();
    copy.first = (Element)first.copy();
    copy.second = (Element)second.copy();
    return copy;
  }//end method copy

    public boolean equal(ElemPair inEl) {
	//returns true if both elements are equal
	if (this.first.getClass() != inEl.first.getClass() ||
	    this.second.getClass() != inEl.second.getClass()) {
	    return false;
	}//if
	else {
	    if (this.first.equal(inEl.first) &&
		this.second.equal(inEl.second)) {
		return true;
	    }//if
	}//else
	return false;
    }//end method equal

    public boolean equalOrInvertedEqual (ElemPair inEl) {
	//returns true if both elements are equal or if
	//equal, when first/second are inverted
	if (this.equal(inEl)) {
	    return true; }
	else {
	    if (this.first.getClass() != inEl.second.getClass() ||
		this.second.getClass() != inEl.first.getClass()) {
		return false; 
	    }//if
	    else {
		if (this.first.equal(inEl.second) &&
		    this.second.equal(inEl.first)) {
		    return true;
		}//if
	    }//else
	}//else
	return false;
    }//end method equalOrInvertedEqual

    
    public void print () {
	System.out.println("first:");
	first.print();
	System.out.println("second:");
	second.print();
    }//end method print    


    public byte compX (ElemPair inPair) throws WrongTypeException {
	//uses the compX method of Element
	byte res = this.first.compX(inPair.first);
	if (res == 0) {
	    res = this.second.compX(inPair.second); }
	return res;
    }//end method compX

    public byte compY (ElemPair inPair) throws WrongTypeException {
	//uses the compY method of Element
	byte res = this.first.compY(inPair.first);
	if (res == 0) {
	    res = this.second.compY(inPair.second); }
	return res;
    }//end method compY

    public byte compare (ElemPair inPair) throws WrongTypeException {
	//...
	byte res = this.first.compare(inPair.first);
	if (res == 0) res = this.second.compare(inPair.second);
	return res;
    }//end method compare

    public void twist () {
	//twists first and second of this
	Element swap = this.first;
	this.first = this.second;
	this.second = swap;
    }//end method twist

}//end class ElemPair
