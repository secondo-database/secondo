class Rect{

  //members
  protected SPoint ul; //upper left point
  protected SPoint ur; //upper right point
  protected SPoint ll; //lower left point
  protected SPoint lr; //lower right point


  //constructors
  public Rect() {
    //fill in dummies
    this.ul = new SPoint();
    this.ur = new SPoint();
    this.ll = new SPoint();
    this.lr = new SPoint();
  }
  
    public Rect(Rational x1, Rational y1, Rational x2, Rational y2) {
	//coordinates are for upper left and lower right point
	ul = new SPoint(x1,y1);
	ur = new SPoint(x2,y1);
	ll = new SPoint(x1,y2);
	lr = new SPoint(x2,y2);

	/*
	ul = new SPoint(x1.copy(),y1.copy());
	ur = new SPoint(x2.copy(),y1.copy());
	ll = new SPoint(x1.copy(),y2.copy());
	lr = new SPoint(x2.copy(),y2.copy());
	*/
    }
	

	/*
  public Rect(Rational x1, Rational y1,
	      Rational x2, Rational y2,
	      Rational x3, Rational y3,
	      Rational x4, Rational y4) {
    this.ul = new SPoint(x1.copy(),y1.copy());
    this.ur = new SPoint(x2.copy(),y2.copy());
    this.ll = new SPoint(x3.copy(),y3.copy());
    this.lr = new SPoint(x4.copy(),y4.copy());
  }
	*/

  //methods
  private boolean p_inside(SPoint inpoint){
    //suppportive method for overlap
    //returns true if inpoint is inside of this.object
    if ((inpoint.x.greater(ul.x)) && (inpoint.x.less(ur.x)) &&
	(inpoint.y.greater(ll.y)) && (inpoint.y.less(ul.y)))
      return true;
    else { return false; }
  }//end method p_inside
  

  public boolean overlap(Rect inrect) {
    //returns true if this.object and inrect overlap

    //one of inrects edges lies in this.object
    if (p_inside(inrect.ul) || p_inside(inrect.ur) ||
	p_inside(inrect.ll) || p_inside(inrect.lr))
      { return true; }
    
    //this.object and inrect are congruent
    if (ul.equal(inrect.ul) && ur.equal(inrect.ur) &&
	ll.equal(inrect.ll) && lr.equal(inrect.lr))
      { return true; }
    return false;
  }//end method overlap

    protected void print() {
	//prints the object's values
	System.out.print("Rect: ");
	System.out.print(" ul: ("+ul.x.toString()+", "+ul.y.toString()+")");
	System.out.print(" ur: ("+ur.x.toString()+", "+ur.y.toString()+")");
	System.out.print(" ll: ("+ll.x.toString()+", "+ll.y.toString()+")");
	System.out.println(" lr: ("+lr.x.toString()+", "+lr.y.toString()+")");
    }//end method print

    public Rect copy(){
	//returns a copy of this.object
	return new Rect(this.ul.x,this.ul.y,this.lr.x,this.lr.y);
    }//end method copy

}//end class Rect
