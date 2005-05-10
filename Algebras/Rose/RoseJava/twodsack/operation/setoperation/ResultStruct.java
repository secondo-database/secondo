package twodsack.operation.setoperation;

import twodsack.util.collection.*;
import twodsack.util.comparator.*;

public class ResultStruct {
    //supportive class for PolPol_Ops, used as return structure for DAC-algorithm
    
    //members
    
    //PairMultiSet pairs;// = new PairList(); // the overlapping pairs already found
    IvlMultiSet m;// = new IvlList(); //set of left and right interval borders
    IvlMultiSet blue;// = new IvlList(); //blue left and right interval borders
    IvlMultiSet green;// = new IvlList(); //green left and right interval borders
    IvlMultiSet blueLeft;// = new IvlList(); //blue left interval borders, right partner not in m
    IvlMultiSet blueRight;// = new IvlList(); //blue right interval borders, left partner not in m
    IvlMultiSet greenLeft;// = new IvlList(); //green left interval borders, right partner not in m
    IvlMultiSet greenRight;// = new IvlList(); //green right interval borders, right partner not in m
    
    //constructors
    ResultStruct(boolean meet) {
	IvlComparator ic = new IvlComparator(meet);
	//this.pairs = new PairMultiSet(new ElemPairComparator());
	//this.m = new IvlList();
	this.blue = new IvlMultiSet(ic);
	this.green = new IvlMultiSet(ic);
	this.blueLeft = new IvlMultiSet(ic);
	this.blueRight = new IvlMultiSet(ic);
	this.greenLeft = new IvlMultiSet(ic);
	this.greenRight = new IvlMultiSet(ic);
    }
    
    
    ResultStruct(IvlMultiSet m,
	       IvlMultiSet bl, IvlMultiSet gr,
	       IvlMultiSet bll, IvlMultiSet blr,
	       IvlMultiSet grl, IvlMultiSet grr) {
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

    protected void print() {
	//prints the lengths of the object's elements
	System.out.print("   ResultStruct:");
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
    
}//end class ResultStruct
