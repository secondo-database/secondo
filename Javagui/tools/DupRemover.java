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

package tools;

import java.util.TreeSet;
import java.io.*;

/** this class read lines from stdin and
    prints out this lines without duplicates */

public class DupRemover{

public static void main(String[] args){
  try{
     BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
     String line;
     TreeSet Set = new TreeSet();
     while(in.ready()){
        line = in.readLine();
	if(!Set.contains(line)){
	    Set.add(line);
	    System.out.print(line+" ");
	}
     }

  } catch(Exception e){
     System.err.println("error in  removing duplicates");
  }
}

}
