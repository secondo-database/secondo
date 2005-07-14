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
import sj.lang.ListExpr;

/** this class provides a link from attribute values to
    category names */


public class AttrCatList{

 private String Name;
 private TreeSet Links;
 private static int no = 0;  // automatic names

 /** create a new AttrCatList with given name */
 public AttrCatList(String Name){
     Links = new TreeSet(new LinkComparator());
     if (Name==null)
        this.Name ="Attribute_Category_Link_"+no++;
     else
        this.Name=Name;
 }

 /** create a new AttrCatList with automatic name */
 public AttrCatList(){
     this("Attribute_Category_Link_"+no++);
 }

 /** returns the name of this AttrCatList */
 public String getName(){
   return Name;
 }

 /** set the Name */
 public void setName(String N){
   if(N!=null);
      Name=N;
 }

 /** add a new Link
   * @return true, if the pair (Value,CatName) is not contained in
   * the set of links
   */
 public boolean addLink(String Value,String CatName){
    return Links.add(new Link(Value,CatName));
 }


  public Iterator getCatNames(){
     TreeSet  CN = new TreeSet(new StringComparator());
     Iterator it = Links.iterator();
     while (it.hasNext()){
        CN.add(((Link)it.next()).getCatName());
     }
     return CN.iterator();
  }

/** add a new Link
   * @return true, if the pair (Value,CatName) is not contained in
   * the set of links
   */
 public boolean addLink(double Value,String CatName){
    return Links.add(new Link(Value,CatName));
 }


 /** add a new Link
   * @return true, if the pair (Value,CatName) is not contained in
   * the set of links
   */
 public boolean addLink(boolean Value,String CatName){
    return Links.add(new Link(Value,CatName));
 }

 /** add a new Link
   * @return true, if the pair (Value,CatName) is not contained in
   * the set of links
   */
 public boolean addLink(int Value,String CatName){
    return Links.add(new Link(Value,CatName));
 }

 /** set a new category name for a given value
   * @return false if the link set not contains a link with given value
   */
 public boolean updateLink(int Value,String CatName){
    Link SearchLink = new Link(Value,"");
    if(!Links.contains(SearchLink))
       return false;
    Link L = (Link) Links.tailSet(SearchLink).first();
    L.setCatName(CatName);
    return true;
 }

 /** set a new category name for a given value
   * @return false if the link set not contains a link with given value
   */
 public boolean updateLink(String Value,String CatName){
    Link SearchLink = new Link(Value,"");
    if(!Links.contains(SearchLink))
       return false;
    Link L = (Link) Links.tailSet(SearchLink).first();
    L.setCatName(CatName);
    return true;
 }

  /** set a new category name for a given value
   * @return false if the link set not contains a link with given value
   */
 public boolean updateLink(boolean Value,String CatName){
    Link SearchLink = new Link(Value,"");
    if(!Links.contains(SearchLink))
       return false;
    Link L = (Link) Links.tailSet(SearchLink).first();
    L.setCatName(CatName);
    return true;
 }

  /** set a new category name for a given value
   * @return false if the link set not contains a link with given value
   */
 public boolean updateLink(double Value,String CatName){
    Link SearchLink = new Link(Value,"");
    if(!Links.contains(SearchLink))
       return false;
    Link L = (Link) Links.tailSet(SearchLink).first();
    L.setCatName(CatName);
    return true;
 }


 /** exists a link to the given value ? */
 public boolean exists(int Value){
    Link SearchLink = new Link(Value,"");
    return Links.contains(SearchLink);
 }

 /** exists a link to the given value ? */
 public boolean exists(double Value){
    Link SearchLink = new Link(Value,"");
    return Links.contains(SearchLink);
 }

 /** exists a link to the given value ? */
 public boolean exists(String Value){
    Link SearchLink = new Link(Value,"");
    return Links.contains(SearchLink);
 }

 /** exists a link to the given value ? */
 public boolean exists(boolean Value){
    Link SearchLink = new Link(Value,"");
    return Links.contains(SearchLink);
 }


 /** returns the name of the Category for the Attribut in LE
   * if LE is a komplex List (i.e. not an Integer,Symbol,String Int or Real
   * or not a Castegory name exists for this value then null is returned */
 public String getCatName(ListExpr LE){
   if(LE.atomType()==ListExpr.INT_ATOM)
      return getCatName(LE.intValue());
   else if(LE.atomType()==ListExpr.REAL_ATOM)
      return getCatName(LE.realValue());
   else if(LE.atomType()==ListExpr.BOOL_ATOM)
      return getCatName(LE.boolValue());
   else if (LE.atomType()==ListExpr.STRING_ATOM)
      return getCatName(LE.stringValue());
   else if (LE.atomType()==ListExpr.SYMBOL_ATOM)
      return getCatName(LE.symbolValue());
   else return null;
 }


 /** returns the number of attribute in List value with existing
   * link to a Category */
 public int  numberOfLinksFor(ListExpr List){
   ListExpr Values  = List;
   int tno =0;
   while (!Values.isEmpty()){
     ListExpr LE=Values.first();
     Values = Values.rest();
    if(LE.atomType()==ListExpr.INT_ATOM){
         if(getCatName(LE.intValue())!=null)
            tno++;
         }
      else if(LE.atomType()==ListExpr.REAL_ATOM){
           if(getCatName(LE.realValue())!=null)
             tno++;
         }
      else if(LE.atomType()==ListExpr.BOOL_ATOM){
           if (getCatName(LE.boolValue())!=null)
             tno++;
         }
      else if (LE.atomType()==ListExpr.STRING_ATOM){
           if(getCatName(LE.stringValue())!=null)
              tno++;
         }
      else if (LE.atomType()==ListExpr.SYMBOL_ATOM){
           if (getCatName(LE.symbolValue())!=null)
	      tno++;
         }
   }
   return tno;
 }



 /** search the category name for given value
   * if this value don't exists in the set of
   * links  the null is returned */
 public String getCatName(String Value){
     Link SearchLink = new Link(Value,"");
     SortedSet SS = Links.tailSet(SearchLink);
     if(SS.isEmpty())
        return null;
     return ((Link)SS.first()).getCatName();
 }

 /** search the category name for given value
   * if this value don't exists in the set of
   * links  the null is returned */
 public String getCatName(boolean Value){
     Link SearchLink = new Link(Value,"");
     SortedSet SS = Links.tailSet(SearchLink);
     if(SS.isEmpty())
        return null;
     return ((Link)SS.first()).getCatName();
 }

 /** search the category name for given value
   * if this value don't exists in the set of
   * links  the null is returned */
 public String getCatName(int Value){
     Link SearchLink = new Link(Value,"");
     SortedSet SS = Links.tailSet(SearchLink);
     if(SS.isEmpty())
        return null;
     return ((Link)SS.first()).getCatName();
 }

 /** search the category name for given value
   * if this value don't exists in the set of
   * links  the null is returned */
 public String getCatName(double Value){
     Link SearchLink = new Link(Value,"");
     SortedSet SS = Links.tailSet(SearchLink);
     if(SS.isEmpty())
        return null;
     return ((Link)SS.first()).getCatName();
 }


/** removes all links from this */
public void clear(){
  Links.clear();
}


/** returns the number of links */
public int getSize(){
   return Links.size();
}


/** returns the String represetation of the value at position index,
  * if this index don't exists, null is returned */
public String getValueStringAt(int index){
  if(index <0 || index>=Links.size())
     return null;
  Iterator it = Links.iterator();
  Object o=null;
  for(int i=0;i<index+1;i++)
      o = it.next();
  return ((Link) o).getValueString();
}

/** returns the name of category on position index,
  * if this index don't exists, null is returned */
public String getCatNameAt(int index){
  if(index <0 || index>=Links.size())
     return null;
  Iterator it = Links.iterator();
  Object o=null;
  for(int i=0;i<index+1;i++)
      o = it.next();
  return ((Link) o).getCatName();
}

/** set the Name of category at position index*/
public void setCatNameAt(int index,String Name){
  if(index <0 || index>=Links.size()||Name==null)
     return;
  Iterator it = Links.iterator();
  Object o=null;
  for(int i=0;i<index+1;i++)
      o = it.next();
  ((Link) o).setCatName(Name);
}

/** set the given CatName for all existing values */
public void setDefaultCatName(String Name){
   Iterator it = Links.iterator();
   while (it.hasNext()){
     ((Link)it.next()).setCatName(Name);
   }
}

/** returns a real clone of this object, this means all
  * containing objects are cloned */
public Object clone(){
   AttrCatList Clone = new AttrCatList(new String(Name));
   // clone the whole treeset
   Iterator it = Links.iterator();
   while(it.hasNext()){
      Clone.Links.add(((Link)it.next()).clone());
   }
   return Clone;
}

/** returns a nestedList representation of this links */
public ListExpr toListExpr(){
  ListExpr LinkList;
  Iterator it = Links.iterator();
  ListExpr Last=null;
  if(Links.size()==0)
     LinkList = ListExpr.theEmptyList();
   else{
     LinkList = ListExpr.oneElemList( ((Link) it.next() ).toListExpr());
     Last = LinkList;
   }
   while(it.hasNext()){
      Last = ListExpr.append(Last,((Link) it.next()).toListExpr());
   }
   return ListExpr.twoElemList( ListExpr.symbolAtom("AttrCatLink"),ListExpr.twoElemList(
                           ListExpr.stringAtom(Name),LinkList));
}


/** read all Values from  a given nested List
  * all valid entries are readed
  */
public boolean readFromListExpr(ListExpr LE){
  if(LE.listLength()!=2)
     return false;
  ListExpr Type = LE.first();
  if(Type.atomType()!=ListExpr.SYMBOL_ATOM)
     return false;
  if(!Type.symbolValue().equals("AttrCatLink"))
     return false;
  ListExpr Value = LE.second();
  if(Value.listLength()!=2)
     return false;
  if(Value.first().atomType()!=ListExpr.STRING_ATOM)
     return false;
  String N = Value.first().stringValue();
  if(N.equals(""))
     return false;
  Name = N;
  ListExpr Ls = Value.second();
  // read all containing links
  Links.clear();
  while(!Ls.isEmpty()){
     Link L = new Link(0,"");
     if (L.readFromListExpr(Ls.first()))
        Links.add(L);
     Ls = Ls.rest();
  }
    return true;
}


private class Link{

  private int type;
  private String  StringValue = null;
  private double  DoubleValue = 0.0;
  private boolean BoolValue  = false;
  private int IntValue = 0;
  private String CatName;


  public Object clone(){
    Link cp = new Link(0,"");
    cp.type = type;
    cp.StringValue = new String(StringValue);
    cp.DoubleValue = DoubleValue;
    cp.BoolValue = BoolValue;
    cp.IntValue = IntValue;
    cp.CatName = new String(CatName);
    return cp;
  }

  /* creates a Link from given String and CategoryName */
  Link(String S,String CN){
    StringValue = S;
    if (S==null)
       StringValue = "";
    type = STRINGTYPE;
    CatName = CN;
  }

  /* creates a Link from given double and CategoryName */
  Link(double d,String CN){
    DoubleValue = d;
    type = REALTYPE;
    CatName = CN;
  }

  /* creates a Link from given boolean and CategoryName */
  Link(boolean b,String CN){
     BoolValue = b;
     type = BOOLTYPE;
     CatName = CN;
  }

  /* creates a Link from given int and CategoryName */
  Link(int i,String CN){
    IntValue = i;
    type = INTTYPE;
    CatName = CN;
  }

  /** returns the CatName of this Link */
  String getCatName(){
    return  CatName;
  }

  /** set the category name */
  void setCatName(String N){
    if(N!=null)
       CatName = N;
  }

  /** returns the string representation of the value */
  String getValueString(){
    if(type==INTTYPE)
       return ""+IntValue;
    if(type==BOOLTYPE)
       return ""+BoolValue;
    if(type==REALTYPE)
       return ""+DoubleValue;
    if(type==STRINGTYPE)
       return StringValue;
    return "error: unknow type";
  }

  /** returns the list representation of this link */
  ListExpr toListExpr(){
    if(type==INTTYPE)
       return ListExpr.threeElemList(ListExpr.symbolAtom("int"),ListExpr.intAtom(IntValue),ListExpr.stringAtom(CatName));
    else if(type==BOOLTYPE)
       return ListExpr.threeElemList(ListExpr.symbolAtom("bool"),ListExpr.boolAtom(BoolValue),ListExpr.stringAtom(CatName));
    else if(type==REALTYPE)
       return ListExpr.threeElemList(ListExpr.symbolAtom("real"),ListExpr.realAtom(DoubleValue),ListExpr.stringAtom(CatName));
    else if(type==STRINGTYPE)
       return ListExpr.threeElemList(ListExpr.symbolAtom("string"),ListExpr.stringAtom(StringValue),ListExpr.stringAtom(CatName));
    else
       return ListExpr.theEmptyList();
    }

   /** read this link from a given KListExpr */
   boolean readFromListExpr(ListExpr LE){
      if(LE.listLength()!=3)
         return false;
      ListExpr typeList=LE.first();
      ListExpr valueList = LE.second();
      ListExpr nameList = LE.third();

      if(typeList.atomType()!=ListExpr.SYMBOL_ATOM)
         return false;
      if(nameList.atomType()!=ListExpr.STRING_ATOM)
         return false;

       String typeString = typeList.symbolValue();
      if(typeString.equals("int")){
         if(valueList.atomType()!=ListExpr.INT_ATOM)
	    return false;
	 type = INTTYPE;
	 StringValue = null;
         IntValue = valueList.intValue();
      } else if(typeString.equals("bool")){
         if(valueList.atomType()!=ListExpr.BOOL_ATOM)
	     return false;
	 type = BOOLTYPE;
	 StringValue = null;
	 BoolValue = valueList.boolValue();
      } else if(typeString.equals("real")){
          if(valueList.atomType()!=ListExpr.REAL_ATOM)
	     return false;
	  type = REALTYPE;
	  StringValue = null;
          DoubleValue = valueList.realValue();
      } else if(typeString.equals("string")){
          if(valueList.atomType()!=ListExpr.STRING_ATOM)
	     return false;
	  type = STRINGTYPE;
	  StringValue = valueList.stringValue();
      } else
          return false;
      CatName = nameList.stringValue();
      return true;
   }

   /** compares 2 Links excluding CatNames */
   int compareTo(Link L){
      if(type<L.type)
          return -1;
      if(type>L.type)
           return 1;
      if(type==INTTYPE){
         if(IntValue<L.IntValue)
	    return -1;
	 if(IntValue>L.IntValue)
	     return 1;
	 return 0;
      }

      if(type==REALTYPE){
          if(DoubleValue < L.DoubleValue)
	     return -1;
	  if(DoubleValue > L.DoubleValue)
	     return 1;
	   return 0;
      }

      if(type==BOOLTYPE){
          if(!BoolValue & L.BoolValue)
	      return -1;
	  if(BoolValue & !L.BoolValue)
	      return 1;
	  return 0;
      }

      if(type==STRINGTYPE){
        return StringValue.compareTo(L.StringValue);
      }
    return -100;  // this point should never be reached
   }
} // class Link


private class LinkComparator implements Comparator{
  public int compare(Object o1,Object o2){
     if( (o1 instanceof Link ) & (o2 instanceof Link))
        return ((Link)o1).compareTo((Link)o2);
     else return -1;
  }
}

private class StringComparator implements Comparator{
  public int compare(Object o1,Object o2){
     if( (o1 instanceof String) && (o2 instanceof String))
        return ((String)o1).compareTo((String)o2);
     else return -1;
  }
}

private final static int REALTYPE = 0;
private final static int STRINGTYPE =1;
private final static int INTTYPE = 2;
private final static int BOOLTYPE = 3;


} // class
