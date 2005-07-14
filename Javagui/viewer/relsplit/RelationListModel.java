//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.relsplit;


import javax.swing.*;
import javax.swing.event.*;
import gui.idmanager.*;
import gui.SecondoObject;


public class RelationListModel implements ListModel{

public void addListDataListener(ListDataListener l){
   // there are no changes of this datamodel
}
public void removeListDataListener(ListDataListener l){}


/** create a new RelationListModell for R */
public RelationListModel(Relation R) throws InvalidRelationException {
  if(R==null || !R.isInitialized())
     throw(new InvalidRelationException());
  Rel = R;
  TupleSize = R.getTupleSize();
}


/** returns the number of list entries */
public int getSize(){
  return Rel.getSize()+Rel.getTupleCount()+1; // a seperator between 2 tuples
}


/** returns the element at index */
public Object getElementAt(int index){
   if( index % (TupleSize+1)==0) // if this is a Separator
     return SEPARATORSTRING;
   else{
     int seps = index / (TupleSize+1);
     String S = Rel.get(index-seps-1).getName(); 
     S=S.substring(Rel.toString().length()+2,S.length());
     int lastIndex = S.lastIndexOf("::");
     S = S.substring(0,lastIndex);
     return S;
   }
}



/** returns the index of the line containing S 
  * returns -1 if not found 
  */
public int find(String S,boolean CaseSensitiv,int Offset){

  String US = S.toUpperCase();
  if( Offset % (TupleSize+1)==0)
     Offset++;
  // transform to the Offset in the Relation
  int seps = Offset / (TupleSize+1);
  int indexRel = Rel.find(S,CaseSensitiv,Offset-seps);
  seps = indexRel/TupleSize;
  if(indexRel<0)
    return -1;
  else
    return indexRel+seps+1; 
}



/* returns the secondoobject at index,
 * if not exists the index or at index  is a separator
 * null is returned 
 */
public SecondoObject getSecondoObjectAt(int index){
   if(index <0 || index >Rel.getSize()+Rel.getTupleCount()+1) // out of bounds
      return null;
   if (index % (TupleSize+1)==0)  // a separator
      return null;
   int seps = index /(TupleSize+1);
   return (SecondoObject) Rel.get(index-seps-1);
}


/** returns all SecondoObjects of Attribute at index */
public SecondoObject[] getAttributeAt(int index){

  if(index <0 || index >Rel.getSize()+Rel.getTupleCount()+1) // out of bounds
      return null;
  if (index % (TupleSize+1)==0)  // a separator
      return null;
  
  int start = (index % (TupleSize+1))-1;

  SecondoObject[] Attribut = new SecondoObject[Rel.getTupleCount()];

  int pos = 0;
  for(int i=start;i<Rel.getSize();i=i+Rel.getTupleSize()){
     Attribut[pos]= Rel.get(i);
     pos++;
  }
  return Attribut; 
}



/** returns all SecondoObjects containing in Rel */
public SecondoObject[] getAllObjects(){
  SecondoObject[] SOs = new SecondoObject[Rel.getSize()];
  for (int i=0;i<Rel.getSize();i++)
      SOs[i] = Rel.get(i);
  return SOs;
}


/** returns all Tuples containing in Rel */
public SecondoObject[] getAllTuples(){
  SecondoObject[] TPs = new SecondoObject[Rel.getTupleCount()];
  for (int i=0;i<TPs.length;i++)
      TPs[i] = Rel.getTupleNo(i);
  return TPs;
}

/** returns the relation */
public SecondoObject getRelation(){
  return Rel.getRelation();
}


/** returns all Objects in the tuple*/
public SecondoObject getTupleOnPos(int Pos){
  if(Pos <0 || Pos >Rel.getSize()+Rel.getTupleCount()+1) // out of bounds
      return null;
  if (Pos % (TupleSize+1)==0)  // a separator
      return null;

  int index = (Pos / (Rel.getTupleSize() +1));
  return Rel.getTupleNo(index);
}





/** returns the index of Secondoobject with ID 
  * returns -1 if ID cannot found
  */
public int getIndexOf(ID aID){
  int pos=-1;
  boolean found = false;
  for(int i=0;i<Rel.getSize() && !found;i++){
     if (Rel.get(i).getID().equals(aID)){
        pos = i;
        found = true;
     }
  }

  if(pos>=0){         // add a number of separators 
     int seps = pos / (TupleSize);
     pos = pos+seps+1;
  }
  return pos;
}




public String toString(){ return Rel.toString();}

public ID getID(){ 
   return Rel.getID();
}

private Relation Rel;
private int TupleSize;
private final static String SEPARATORSTRING="****************";

}
