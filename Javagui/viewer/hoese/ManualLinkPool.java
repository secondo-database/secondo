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

package viewer.hoese;

import java.util.*;
import sj.lang.*;


public class ManualLinkPool{

/** create a new ManualLinkPool */
private ManualLinkPool(){
    ManualLinks = new Vector();
}

/** adds a new  ManualLink
  * @return true if name of this Link not exists, is not null and is not a empty String
  */
public static boolean add(AttrCatList NewLink){
  String N = NewLink.getName();
  if(N==null) return false;
  if(N.equals("")) return false;
  if(exists(N))
     return false;
  MLPool.ManualLinks.add(NewLink);
  return true;
}


public static boolean update(String OldName,AttrCatList UD){
   if(OldName==null) return false;
   String N = UD.getName();
   if(!OldName.equals(N)){ // name can't exists
      if(N==null) return false;
      if(N.equals("")) return false;
      if(exists(N)) return false;
   }
   // search the reference to be updatet
   boolean found = false;
   int Pos=-1;
   for(int i=0;i<MLPool.ManualLinks.size()&!found;i++)
      if(OldName.equals( ((AttrCatList)MLPool.ManualLinks.get(i)).getName())){
         Pos = i;
	 found = true;
       }
   if(!found) return false;
   MLPool.ManualLinks.set(Pos,UD);
   return true;
}

/** returns true if a AttrCatList with given name exists */
public static boolean exists(String Name){
  if(Name==null) return false;
  if(Name.equals("")) return false;
  for(int i=0;i<MLPool.ManualLinks.size();i++){
     if(Name.equals( ((AttrCatList)MLPool.ManualLinks.get(i)).getName()))
        return true;
  }
  return false;
}


/** returns the number of containing Links */
public static int numberOfLinks(){
    return MLPool.ManualLinks.size();
}


/** removes the Link with given name
  * @return true if name found else false is returned
  */
public static boolean removeLinkWithName(String Name){
  if(Name==null) return false;
  if(Name.equals("")) return false;
  for(int i=0;i<MLPool.ManualLinks.size();i++){
     if(Name.equals( ((AttrCatList)MLPool.ManualLinks.get(i)).getName())){
        MLPool.ManualLinks.remove(i);
        return true;
     }
  }
  return false;
}


/** returns the Link with given Name;
  * if the name can't found null is returned
  */
public static AttrCatList getLinkWithName(String Name){
  if(Name==null) return null;
  if(Name.equals("")) return null;
  for(int i=0;i<MLPool.ManualLinks.size();i++){
     if(Name.equals( ((AttrCatList)MLPool.ManualLinks.get(i)).getName())){
          return (AttrCatList) MLPool.ManualLinks.get(i);
     }
  }
  return null;
}



/** get the Link at given Position
  * if index don't exists null is returned
  */
public static AttrCatList get(int index){
   if(index<0 | index>MLPool.ManualLinks.size())
     return null;
   return (AttrCatList)  MLPool.ManualLinks.get(index);
}




/** returns a list representation of this pool */
public static ListExpr toListExpr(){
   ListExpr AllLinks;
   ListExpr Last = null;
   if(MLPool.ManualLinks.size()==0)
      AllLinks = ListExpr.theEmptyList();
   else{
      AllLinks = ListExpr.oneElemList( ((AttrCatList)MLPool.ManualLinks.get(0)).toListExpr());
      Last = AllLinks;
   }
   for(int i=1;i<MLPool.ManualLinks.size();i++)
      Last = Last.append(Last,((AttrCatList)MLPool.ManualLinks.get(i)).toListExpr());
   return ListExpr.twoElemList(ListExpr.symbolAtom("AttrCatPool"),AllLinks);
}

/** read all valid ManualLinks in this pool
  * All old values are removed
  * @return false if this file not contains a valid pool
  */
public static boolean readFromListExpr(ListExpr LE){
   if(LE.listLength()!=2)
      return false;
   ListExpr TypeList = LE.first();
   if(TypeList.atomType()!=ListExpr.SYMBOL_ATOM || !TypeList.symbolValue().equals("AttrCatPool"))
      return false;
   ListExpr Values = LE.second();
   MLPool.ManualLinks.clear();
   AttrCatList ACL;
   while(!Values.isEmpty()){
      ACL = new AttrCatList();
      if (ACL.readFromListExpr(Values.first()))
         MLPool.ManualLinks.add(ACL);
      Values = Values.rest();
   }
   return true;
}


/** returns the position of AttrCatList with given name
  * if the name not exists -1 is returned
  */
private int getPos(String Name){
  for(int i=0;i<ManualLinks.size();i++)
     if( ((AttrCatList)ManualLinks.get(i)).getName().equals(Name))
        return i;
  return -1;
}

/** read the ManualLinks from this listexpr
  * if a name in this list conflicts with old
  * names the the old ManualLink is updatet
  * @return true if LE is a valid AttrCatPool list
  */
public static boolean addFromListExpr(ListExpr LE){
   if(LE.listLength()!=2)
      return false;
   ListExpr TypeList = LE.first();
   if(TypeList.atomType()!=ListExpr.SYMBOL_ATOM || !TypeList.symbolValue().equals("AttrCatPool"))
      return false;
   ListExpr Values = LE.second();
   AttrCatList ACL;
   while(!Values.isEmpty()){
      ACL = new AttrCatList();
      if (ACL.readFromListExpr(Values.first())){
        int pos = MLPool.getPos(ACL.getName());
	if(pos<0)
           MLPool.ManualLinks.add(ACL);
	else
	   MLPool.ManualLinks.set(pos,ACL);
      }
      Values = Values.rest();
   }
   return true;
}

private Vector ManualLinks;
public static ManualLinkPool MLPool = new ManualLinkPool();

}
