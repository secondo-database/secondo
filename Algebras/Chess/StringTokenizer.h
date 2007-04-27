/*
---- 
This file is part of SECONDO.

Copyright (C) 2006, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


*/

#include <string>

class StringTokenizer 
{
private:
    vector<string> elements;
    vector<string>::iterator iterelements;
    int index;
    int count;
public:
    StringTokenizer(string,string);
    ~StringTokenizer() { elements.clear();};
    int countElements();
    string elementAt(int);
    
    string getNextElement();
};



  // Constructor that takes 2 arguments 
  // first argument is of string type that to be tokenized.
  // second argument is of string type that is used as token seperator
 // and default seperator is space
 
StringTokenizer::StringTokenizer(string str,string sep=" ")
{
    index=0;
    count=0;
    string str1="";
    for(int i=0;i< (int) str.length() && sep.length()== 1;i++)
    {
        if(str[i]==sep[0])
        {
            elements.push_back(str1);
            
            str1="";
        }
        else
        {
            str1+=str[i]; 
        }
    }
    elements.push_back(str1);
    count=elements.size ();

}

  // Method is used to fetch the tokens.
string StringTokenizer::getNextElement()
{
    index++;
    if(index==count)
    {
        throw string("Index out of Bounds");
    }
    return elements[index-1];    
}
  //method used to fetch the count of tokens from the string
int StringTokenizer::countElements()
{
    return count;
}

  //fetch the elements at given position 
string StringTokenizer::elementAt(int index)
{
    if(index>=count ||index<0)
    {
        throw string("Index out of Bounds");
    }
    else
        return elements[index];    
}
