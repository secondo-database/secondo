
//This file is part of SECONDO.

//Copyright (C) 2009, University in Hagen, 
// Faculty of Mathematics and Computer Science, 
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

package gui;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;
import java.io.*;

public class StoredQueries{

public StoredQueries(){
  table = new Vector(127);
}

public int readFromFile(String fileName){
  table.clear();
  BufferedReader in=null;
  int errors = 0;
  try{
    in = new BufferedReader(new FileReader(fileName));
    if(!in.ready()){
       return 1;
    }
    String line = in.readLine();
    if(!line.equals("##SavedQueries##")){
      return 1;
    }
    while(in.ready()){
       line = in.readLine();
       while(in.ready() && !line.matches("\\[.*\\] *[0-9]+")){
         line = in.readLine();
       }
       if(!line.matches("\\[.*\\] *[0-9]+")){
         return errors;
       } 
       int last = line.lastIndexOf(']');
       String name = line.substring(1,last);

       if(name.length()==0 || containsName(name)){
          errors++;
       } else {
         String num = line.substring(last+1).trim();
         int n = Integer.parseInt(num);
         String query = "";
         for(int i=0;i<n && in.ready();i++) {
            query+=in.readLine()+"\n";
         }
         table.add(new Pair(name,query.trim()));
       }
    }
  }catch(Exception e){
    return errors+1;
  } finally{
    if(in!=null){try { in.close();} catch(Exception e){} }
  }
  return errors;
}



public boolean saveToFile(String fileName){
  PrintStream p = null;
  try{
      File f = new File(fileName);
      p = new PrintStream(new FileOutputStream(f));
      p.println("##SavedQueries##\n");
      for(int i=0;i<table.size();i++){
        Pair pair = (Pair) table.get(i);
        String name = pair.name;
        String query = pair.query;
        int count=0;
        int pos=0;
        while((pos = query.indexOf('\n',pos+1))>=0){
          count++;
        }
        count++;
        p.println("["+name+"] " + count);
        p.println(query);
        p.println("\n\n");
      }
  } catch(Exception e){
    return false;
  } finally {
    if(p!=null){
      try{p.close();}catch(Exception e){}
    }
  }
  return true;
}

public boolean putEntry(String name, String query){
  if(containsName(name)){
     return false;
  }
  table.add(new Pair(name,query.trim()));
  return true;
}

public String getQuery(String name){
  if(!table.contains(name)){
    return null;
  }
  for(int i =0;i<table.size();i++){
     Pair p = (Pair) table.get(i);
     if(p.name.equals(name)){
        return p.query;
     }
  }
  return null;
}

public boolean containsName(String name){
  for(int i=0;i<table.size();i++){
     Pair p = (Pair) table.get(i);
     if(p.name.equals(name)){
       return true;
     }
  };
  return false;
}

public boolean removeName(String name){
  for(int i=0;i<table.size();i++){
     Pair p = (Pair) table.get(i);
     if(p.name.equals(name)){
       table.remove(i);
       return true;
     } 
  }
  return false;
}



public void clear(){
  table.clear();
}

public int getSize(){
   return table.size();
}

public Object getElementAt(int index){
   return table.get(index);
}




private Vector table;


}


