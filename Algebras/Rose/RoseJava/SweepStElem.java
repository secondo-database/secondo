class SweepStElem {
    //supportive class for the sweep line status structure of the triangulation method in Polygons
    //this is also used for the sweep line status structure in Triangle.java

  //members
  boolean is_top;
  boolean is_bottom;
  PointList pointChain;
  boolean in; //true if attribute "in polygons" holds
    String col; //may be "red","blue", "both", ""
  Point rimoEl; //rightmost element

  //constructors
  protected SweepStElem() {
    boolean is_top = false;
    boolean is_bottom = false;
    pointChain = new PointList();
    in = false;
    col = "";
    rimoEl = new Point();
  }

    //methods
    protected void print() {
	//prints the elements attributes
	System.out.println("SweepStElem:");
	System.out.println("is_top:"+is_top+", is_bottom:"+is_bottom+", in:"+in+", col:"+col);
	System.out.println("pointChain:"+pointChain.size());
	pointChain.print();
	System.out.println("rimoEl:");
	rimoEl.print();
    }//end method print

}//end class SweepStElem
