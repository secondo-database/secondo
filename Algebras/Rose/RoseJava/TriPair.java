class TriPair {
  
  //members
  Triangle first;
  Triangle second;

  //constructors
  TriPair() {
    first = new Triangle();
    second = new Triangle();
  }
  TriPair(Triangle t1, Triangle t2) {
    first = (Triangle)t1.copy();
    second = (Triangle)t2.copy();
  }
  
  //methods
  public TriPair copy(){
    //returns a copy of TriPair
    TriPair copy = new TriPair();
    copy.first = (Triangle)first.copy();
    copy.second = (Triangle)second.copy();
    return copy;
  }//end method copy

}//end class TriPair
