package viewer.relsplit;

import java.util.Vector;
import gui.SecondoObject;
import sj.lang.ListExpr;
import gui.idmanager.*;

public class Relation{


public Relation(){
    SecondoObjects = new Vector();
    TupleIDs = new Vector();
    head = new Head(); 
    TupleType = ListExpr.theEmptyList();
    WholeRelation = null;
    initialized = false;
}

public boolean readFromSecondoObject(SecondoObject SO){
   myID = SO.getID();
   Name = SO.getName();
   initialized = false;
   ListExpr LE = SO.toListExpr();
   if(LE.listLength()!=2){
      System.out.println("relplit.Relation.readFromSecondoObject : wrong list length");
      return false;
   }
   if(!head.readFromRelTypeLE(LE.first())){
      System.out.println("relplit.Relation.readFromSecondoObject : wrong type list");
      return false;
   }
   if(!readValue(head,LE.second())){
      System.out.println("relplit.Relation.readFromSecondoObject : wrong value list");
      return false;
   }

   WholeRelation = SO;
   TupleType = SO.toListExpr().first().second();
   initialized = true;
   return true;
}


public String toString(){
    return Name;
}


public int find(String S,boolean CaseSensitiv,int OffSet){
  boolean found =false;
  int pos = -1;
  String US = S.toUpperCase();
  for(int i=OffSet;i<SecondoObjects.size()&&!found;i++){
     String Name = get(i).getName();
     if(CaseSensitiv){
          if (Name.indexOf(S)>=0){
               found=true;
               pos=i;
           }
     } else{
          if (Name.toUpperCase().indexOf(US)>=0){
               found=true;
               pos=i;
           }

     }
  }
  return pos;
}


/** read the Value of this Relation */
private boolean readValue(Head H,ListExpr ValueList){
  ListExpr NextTuple;
  ListExpr Rest = ValueList;
  SecondoObjects.clear();
  TupleIDs.clear();
  WholeRelation = null;
  TupleType = ListExpr.theEmptyList();
  boolean ok = true;
  int T_no = 0;
  while(Rest.listLength()>0 && ok){
    NextTuple = Rest.first();
    T_no++;
    Rest = Rest.rest();
    if(NextTuple.listLength()!=H.getSize())  // wrong tuplelength
       ok = false;
    else{
       SecondoObject SO;
       int No = 0;
       TupleIDs.add(IDManager.getNextID());
       while(NextTuple.listLength()>0){
          SO = new SecondoObject(IDManager.getNextID());
          ListExpr Type = ListExpr.symbolAtom( H.get(No).Type);
          SO.fromList(ListExpr.twoElemList(Type,NextTuple.first()));
          String aName = computeObjectName(H.get(No).Name,H.get(No).Type,NextTuple.first());
          SO.setName(Name+"::"+aName+"::"+T_no);
          NextTuple = NextTuple.rest();
          SecondoObjects.add(SO);
          No++;
       }
    }
  }
  if(!ok){
    SecondoObjects.clear();
    TupleIDs.clear();
    TupleType = ListExpr.theEmptyList();
  }
  return ok;
}



/** computes a short Name for a object */
private String computeObjectName(String name,String type,ListExpr value){
  int len = head.getMaxNameLength();
  String Name="";
  for(int i=0;i<len+1-name.length();i++)
     Name = Name+" ";
  Name += name+" ";

  String ValueString;
  if(!value.isAtom() && value.listLength()==1 && value.first().atomType()==ListExpr.TEXT_ATOM)
     value = value.first();

  if (!value.isAtom()){
       ValueString = type;
  }
  else{
      int atomType = value.atomType();
      switch (atomType){
        case ListExpr.REAL_ATOM : ValueString=""+value.realValue(); break;
        case ListExpr.STRING_ATOM : ValueString= value.stringValue(); break;
        case ListExpr.INT_ATOM : ValueString = ""+value.intValue(); break;
        case ListExpr.SYMBOL_ATOM : ValueString = value.symbolValue(); break;
        case ListExpr.BOOL_ATOM : ValueString = ""+value.boolValue(); break;
        case ListExpr.TEXT_ATOM :  if(value.textLength()>48)
	                               ValueString = "TEXT "+value.textLength()+" chars";
				    else
				       ValueString = value.textValue();
	                            break;
        case ListExpr.NO_ATOM : ValueString= type; break;
        default : ValueString = "unknow type";
      }
  }
  return Name+": "+ValueString;
}



/** check if SO contains a relation */
public static boolean isRelation(SecondoObject SO){
  return Head.isRelation(SO.toListExpr());
}


/** return the SecondoObject on given position
  * both numbers are started with 0
  */
public SecondoObject getSecondoObject(int TupleNumber,int ObjectNumber){
  if (!initialized)
    return null;
  int index = head.getSize()*TupleNumber+ObjectNumber;
  if(index<0 |  index>SecondoObjects.size())
     return null;
  else
     return (SecondoObject) SecondoObjects.get(index);
}


/** return the Tuple on give Position */
private SecondoObject[] getTupleAt(int index){
   if(!initialized)
      return null;
   int startTuple = index*head.getSize();
   if(startTuple<0 || startTuple+head.getSize()>SecondoObjects.size())
      return null;

   SecondoObject[] Tuple = new SecondoObject[head.getSize()];
   for(int i=0;i<head.getSize();i++)
      Tuple[i]  = (SecondoObject) SecondoObjects.get(i+startTuple);
   return Tuple;
}



/** computes a Tuple and returns it */
public SecondoObject getTupleNo(int index){
  SecondoObject[] Content = getTupleAt(index);
  if (Content==null)
    return null;
  // compute the value_list
  ListExpr Value,Last=null;
  if(Content.length==0)
      Value = ListExpr.theEmptyList();
  else{
    Value = ListExpr.oneElemList(Content[0].toListExpr().second());
    Last = Value;
  }
  ListExpr Next;
  for(int i=1;i<Content.length;i++){
      Next = (Content[i].toListExpr().second());
      Last = ListExpr.append(Last,Next);
  }

  SecondoObject Tuple = new SecondoObject((ID)TupleIDs.get(index));
  Tuple.fromList(ListExpr.twoElemList(TupleType,Value));
  Tuple.setName(Name+" ["+index+"]");
  return Tuple;
}


/** returns the Relation */
public SecondoObject getRelation(){
  return WholeRelation;
}


/** returns the object on index */
public SecondoObject get(int index){
  if(index<0 || index>SecondoObjects.size())
     return null;
  else
     return (SecondoObject) SecondoObjects.get(index);
}


/** returns the number of containing tuples ,
  * if this Relation is not initialized -1 is returned
  */
public int getTupleCount(){
  if(!initialized)
     return -1;
  else
     return SecondoObjects.size()/head.getSize();
}

/* returns the number of objects in a tuple
 * if this relation not initialized -1 is returned
 */
public int getTupleSize(){
   if (!initialized)
     return -1;
   else
     return head.getSize();
}

/* returns the number of all containing objects
 * if this relation not initialized -1 is returned
 */
public int getSize(){
  if(!initialized)
    return -1;
  else
    return SecondoObjects.size();
}


public boolean isInitialized(){ return initialized;}


public ID getID(){
   if(!initialized)
      return null;
   else
      return myID;
}


private Head   head;
private Vector SecondoObjects;
private boolean initialized;
private ID myID;
private String Name;
private Vector TupleIDs;
private ListExpr TupleType;
private SecondoObject WholeRelation;
}
