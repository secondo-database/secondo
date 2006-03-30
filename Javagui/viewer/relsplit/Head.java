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

import sj.lang.ListExpr;
import java.util.Vector;
import tools.Reporter;

public class Head{

private Vector V = new Vector();
private int MaxTypeLength=0;
private int MaxNameLength=0;


/** returns the maximal length of a type name */
public int getMaxTypeLength(){
  return MaxTypeLength;
}

/** returns the maximal length of a attribute name */
public int getMaxNameLength(){
    return MaxNameLength;
}


/** check if LE = (<type> <value>) is a relation */
public static boolean isRelation(ListExpr LE){
  if(LE.listLength()!=2)
     return false;
  ListExpr Type = LE.first();
  if(Type.listLength()!=2)
    return false;
  ListExpr R = Type.first();
  return (R.isAtom() && (R.atomType()==ListExpr.SYMBOL_ATOM) &&
           (R.symbolValue().equals("rel") | R.symbolValue().equals("mrel")));
}



public boolean readFromRelTypeLE(ListExpr LE){
  V.clear();
  MaxTypeLength = 0;
  MaxNameLength = 0;
  if(LE.listLength()!=2) // (rel (tuple ... ))
    return false;
  ListExpr TName = LE.first();
  if(TName.atomType()!=ListExpr.SYMBOL_ATOM)
     return false;
  String Name = TName.symbolValue();
  if( ! (Name.equals("rel") | Name.equals("mrel")))
     return false;

  ListExpr Tuple = LE.second();
  if(Tuple.listLength()!=2){
     Reporter.writeError("wrong tuple listlength");
     return false;
  }


  ListExpr TupleType = Tuple.first();
  if ( !TupleType.isAtom() || !(TupleType.atomType()==ListExpr.SYMBOL_ATOM) ||
       !(TupleType.symbolValue().equals("tuple") | TupleType.symbolValue().equals("mtuple")) )
     return false;


  ListExpr TupleValue = Tuple.second(); // should be ( <name type) (name type) ... )
  boolean ok = true;


  while (TupleValue.listLength()>0 & ok) {
     ListExpr EntryList = TupleValue.first();
     if(EntryList.listLength()!=2){  // (name type)
       ok = false;
     }
     else{
       ListExpr NameList = EntryList.first();
       ListExpr TypeList = EntryList.second();
       if(! (NameList.isAtom() && NameList.atomType()==ListExpr.SYMBOL_ATOM)){
          ok =false;
       }
       if(!( TypeList.isAtom() && TypeList.atomType()==ListExpr.SYMBOL_ATOM)){
          ok = false;
       }
       if(ok){
          V.add(new HeadEntry(NameList.symbolValue(),TypeList.symbolValue() ) );
          MaxTypeLength = Math.max(MaxTypeLength,TypeList.symbolValue().length());
	  MaxNameLength = Math.max(MaxNameLength,NameList.symbolValue().length());
       }
     }
     TupleValue = TupleValue.rest();
  }
  if(!ok){
     V.clear();
     MaxNameLength = 0;
     MaxTypeLength = 0;
  }
  return ok;
}

public int getSize(){
   return V.size();
}

public HeadEntry get(int index){
  if(index<0 | index>V.size())
     return null;
  else
     return (HeadEntry) V.get(index); 
}


}

