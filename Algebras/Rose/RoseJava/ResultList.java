class ResultList {
    //supportive class for PolPol_Ops, used as return structure for DAC-algorithm
    
    //members
    PairList pairs;// = new PairList(); // the overlapping pairs already found
    IvlList m;// = new IvlList(); //set of left and right interval borders
    IvlList blue;// = new IvlList(); //blue left and right interval borders
    IvlList green;// = new IvlList(); //green left and right interval borders
    IvlList blueLeft;// = new IvlList(); //blue left interval borders, right partner not in m
    IvlList blueRight;// = new IvlList(); //blue right interval borders, left partner not in m
    IvlList greenLeft;// = new IvlList(); //green left interval borders, right partner not in m
    IvlList greenRight;// = new IvlList(); //green right interval borders, right partner not in m
    
    //constructors
    ResultList() {
	this.pairs = new PairList();
	//this.m = new IvlList();
	this.blue = new IvlList();
	this.green = new IvlList();
	this.blueLeft = new IvlList();
	this.blueRight = new IvlList();
	this.greenLeft = new IvlList();
	this.greenRight = new IvlList();
    }
  
  
  ResultList(PairList pl, IvlList m,
	     IvlList bl, IvlList gr,
	     IvlList bll, IvlList blr,
	     IvlList grl, IvlList grr) {
    this.pairs = pl;
    this.m = m;
    this.blue = bl;
    this.green = gr;
    this.blueLeft = bll;
    this.blueRight = blr;
    this.greenLeft = grl;
    this.greenRight = grr;
  }
  
  //methods
  protected void print() {
    //prints the lengths of the object's elements
    System.out.print("   ResultList:");
    System.out.print(" pairs: "+pairs.size());
    System.out.print(" m: "+m.size());
    System.out.print(" blue: "+blue.size());
    System.out.print(" green: "+green.size());
    System.out.print(" blueLeft: "+blueLeft.size());
    System.out.print(" blueRight: "+blueRight.size());
    System.out.print(" greenLeft: "+greenLeft.size());
    System.out.print(" greenRight: "+greenRight.size());
    System.out.println();
  }//end method print
  
}//end class ResultLists
