package twodsack.util.collection;

public class ResultList {
    //supportive class for PolPol_Ops, used as return structure for DAC-algorithm
    
    //members
    //PairMultiSet pairs;// = new PairList(); // the overlapping pairs already found
    public IvlList m;// = new IvlList(); //set of left and right interval borders
    public IvlList blue;// = new IvlList(); //blue left and right interval borders
    public IvlList green;// = new IvlList(); //green left and right interval borders
    public IvlList blueLeft;// = new IvlList(); //blue left interval borders, right partner not in m
    public IvlList blueRight;// = new IvlList(); //blue right interval borders, left partner not in m
    public IvlList greenLeft;// = new IvlList(); //green left interval borders, right partner not in m
    public IvlList greenRight;// = new IvlList(); //green right interval borders, right partner not in m
    
    //constructors
    public ResultList() {
	//this.pairs = new PairMultiSet(new ElemPairComparator());
	//this.m = new IvlList();
	this.blue = new IvlList();
	this.green = new IvlList();
	this.blueLeft = new IvlList();
	this.blueRight = new IvlList();
	this.greenLeft = new IvlList();
	this.greenRight = new IvlList();
    }
    
    
    public ResultList(IvlList m,
	       IvlList bl, IvlList gr,
	       IvlList bll, IvlList blr,
	       IvlList grl, IvlList grr) {
	//this.pairs = pl;
	this.m = m;
	this.blue = bl;
	this.green = gr;
	this.blueLeft = bll;
	this.blueRight = blr;
	this.greenLeft = grl;
	this.greenRight = grr;
    }
    
    //methods
    /*
    public void init() {
	this.pairs = new PairMultiSet(new ElemPairComparator());
	//this.m = new IvlList();
	this.blue = new IvlList();
	this.green = new IvlList();
	this.blueLeft = new IvlList();
	this.blueRight = new IvlList();
	this.greenLeft = new IvlList();
	this.greenRight = new IvlList();
    }//end method init
    */

    public void print() {
	//prints the lengths of the object's elements
	System.out.print("   ResultList:");
	//System.out.print(" pairs: "+pairs.size());
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
