import java.util.*;

class PolList extends ElemList{

  //members

  //constructors

  //methods
  public ElemList copy(){
    PolList copy = new PolList();
    for (int i = 0; i < this.size(); i++) {
      copy.add(((Polygons)this.get(i)).copy());
    }//for
    return copy;
  }//end method copy
  

}//end class PolList
