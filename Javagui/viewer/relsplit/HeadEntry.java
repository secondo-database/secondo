package viewer.relsplit;


class HeadEntry{
  
 public HeadEntry(String Name, String Type){
    this.Name = Name;
    this.Type = Type;
  } 

  public String Name;
  public String Type;

  
  public String toString(){
    return Name +"  :  "+Type;
  } 

}

