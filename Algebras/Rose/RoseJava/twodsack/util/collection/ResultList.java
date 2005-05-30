/*
 * ResultList.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;


/**
 * A ResultList is used as data structure for storing data between the different iterations of the DAC algorithm implemented in <tt>SetOps.overlappingPairs</tt>.
 * Particularly, it stores several lists of intervals which have different marks.
 */
public class ResultList {
    /*
     * fields
     */
    /**
     * Set of left and right interval borders.
     */
    public IvlList m;

    /**
     * Set of left and right interval borders marked with "blue".
     */
    public IvlList blue;

    /**
     * Set of left and right interval borders markes with "green".
     */
    public IvlList green;

    /**
     * Set of left interval borders marked with "blue". Right partners are not in <tt>m</tt>.
     */
    public IvlList blueLeft;

    /**
     * Set of right interval borders marked with "blue". Left partners are not in <tt>m</tt>.
     */
    public IvlList blueRight;

    /**
     * Set of left interval borders marked with "green". Right partners are not in <tt>m</tt>.
     */
    public IvlList greenLeft;

    /**
     * Set of right interval borders marked with "green". Left partners are not in <tt>m</tt>.
     */
    public IvlList greenRight;
    

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
