class ColPoint {
    //ColPoint is an SPoint with an attribute color
    //it is used to provide a Point data type with an extra
    //attribute for Triangle.computeIntersectionTriangles
    //additionally it has an integer variable which can be
    //used as a counter {necessary for the Intersection method}
    //it also has a second integer variable to store
    //a further attribute of the point
    

  //members
    public Point point = new Point();
    public String color = "";
    public int counter = 0;
    public int attrib = -1;
  
  //constructors
  public ColPoint() {
    this.point = new Point();
    this.color = "";
    this.attrib = -1;
  }
  
  public ColPoint(Point p, String s, int a) {
    this.point = (Point)p.copy();
    this.color = s;
    this.attrib = a;
  }

  //methods
  public void print() {
    //prints the object's data
    System.out.println("ColPoint ("+point.x.toString()+","+point.y.toString()+","+color+")");
  }//end method print

  public ColPoint copy(){
    //returns a copy of this.object
    ColPoint cp = new ColPoint((Point)this.point.copy(),color,attrib);
    return cp;
  }//end method copy

}//end class ColPoint
