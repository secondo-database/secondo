

package gui;

public  class Pair{
  public Pair(String name, String query){
    this.name = name;
    this.query = query;
  }

  public boolean equals(Object o){
    if(!(o instanceof Pair)){
      return false;
    }
    Pair p = (Pair)o;
    return p.name.equals(this.name);
  }

  String name; 
  String query;
}
