class Interval {
  //supportive class for class SetOps
  //'right' has to be greater than or equal to 'left'
  
  //members
    Rational left;// = new Rational(0);
    Rational right;// = new Rational(0);
    //boolean leftopen;
    //boolean rightopen;
    String mark;
    Rational x;// = new Rational(0);
    Element ref;
    int number;
  //Triangle reftri;
  
  //constructors
  Interval() {
    this.left = new Rational(0);
    this.right = new Rational(0);
    this.mark = "";
    this.x = new Rational(0);
    this.number = -1;
    //this.reftri = new Triangle();
  }
  
  Interval(Rational l, Rational r, String m, Rational x, Element e, int n) {
    if (l.lessOrEqual(r)) {
      this.left = l.copy();
      this.right = r.copy();
      this.mark = m;
      this.x = x.copy();
      this.ref = e;//.copy()
      this.number = n;
      //this.reftri = t;
    }//if
    else { System.out.println("Error: Couldn't create interval."); }
  }//end constructor
  
  //methods
  Interval copy () {
    //returns a copy of this.object
    return (new Interval(this.left.copy(),this.right.copy(),this.mark,this.x.copy(),(Element)this.ref,this.number));
  }//end constructor
  
  public boolean equal(Interval in) {
    //returns true if this.object and 'in' are equal
    //false else
    //attention: checks only for the interval an NOT for other data
    
    if ((this.left.equal(in.left)) &&
	(this.right.equal(in.right))) {
      return true;
    }//if
    else { return false; }
  }//end method equal

    protected void print() {
	//prints the object's data
	System.out.print("Interval:");
	System.out.print(" down: "+this.left.toString());
	System.out.print(" top: "+this.right.toString());
	System.out.print(" mark: "+this.mark);
	System.out.print(" x: "+this.x.toString());
	System.out.print(" number: "+this.number);
	System.out.println(" ref: ");
	this.ref.print();
    }//end method print
  

    protected byte comp(Interval in) {
	//returns 0 if this and in are equal
	//returns -1 if x of this is smaller
	//returns +1 if x of this is greater
	//if x is equal, left, right are compared in this order
	if (this.x.less(in.x)) { return -1; }
	if (this.x.greater(in.x)) { return 1; }
	//x must be equal
	if (this.left.less(in.left)) { return -1; }
	if (this.left.greater(in.left)) { return 1; }
	//left must be equal
	if (this.right.less(in.right)) { return -1; }
	if (this.right.greater(in.right)) { return 1; }
	//right must be equal
	//byte erg = this.ref.compX(in.ref);
	//if (erg == 0) { erg = this.ref.compY(in.ref); }
	return 0;
    }//end method comp


}//end class Interval
