package tools;

//This file is part of SECONDO.

//Copyright (C) 2004i-2007, University in Hagen, 
// Faculty of Mathematics and 
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

/*
 A Stringtokenizer class which also returns empty strings.

*/

public class MyStringTokenizer{

/** Creates a StrigTokenizer for the String S and delimiter delim.
 **/
public MyStringTokenizer(String S,char delim){
  MyString = S;
  this.delim=delim;
}

/** Returns whether more tokens are available **/
public boolean hasMoreTokens(){
  return MyString.length()>0;
}


/** Returns the nextToken in the String. **/
public String nextToken(){
  if(MyString.length()==0) return "";


  int index = MyString.indexOf(delim);
  if (index<0){
    String res = MyString;
    MyString="";
    return res;
  } else{
    String res = MyString.substring(0,index);
    MyString = MyString.substring(index+1);
    return res;
  }

}

/** The (remaining) String. **/
private String MyString;
/** The used delimiter. **/  
private char delim;

}

