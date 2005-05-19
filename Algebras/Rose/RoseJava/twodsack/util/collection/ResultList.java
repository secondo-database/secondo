/*
 * ResultList.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;


/**
 * A ResultList is used as data structure for storing data between the different iterations of the DAC algorithm implemented in 
 * SetOps.overlappingPairs. Particularly, it stores several lists of intervals which have different marks.
 */
public class ResultList {
    /*
     * fields
     */
    public IvlList m;          //set of left and right interval borders
    public IvlList blue;       //blue left and right interval borders
    public IvlList green;      //green left and right interval borders
    public IvlList blueLeft;   //blue left interval borders, right partner not in m
    public IvlList blueRight;  //blue right interval borders, left partner not in m
    public IvlList greenLeft;  //green left interval borders, right partner not in m
    public IvlList greenRight; //green right interval borders, right partner not in m
    

    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     * Initializes the lists.
     */
    public ResultList() {
	this.blue = new IvlList();
	this.green = new IvlList();
	this.blueLeft = new IvlList();
	this.blueRight = new IvlList();
	this.greenLeft = new IvlList();
	this.greenRight = new IvlList();
    }
    
    
    /**
     * Constructs a new instance with the passed IvlList(s).
     *
     * @param m a list of left and right interval borders
     * @param bl a list of left and right interval borders marked with blue
     * @param gr a list of left and right interval borders marked with green
     * @param bll a list of left interval borders marked with blue
     * @param blr a list of right interval borders marked with blue
     * @param grl a list of green interval borders marked with green
     * @param grr a list of right interval borders marked with green
     */
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
    

    /*
     * methods
     */
    /**
     * Prints the interval lists to standard output.
     */
    public void print() {
	//prints the lengths of the object's elements
	System.out.print("   ResultList:");
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
