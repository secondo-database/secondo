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

import java.io.*;
import java.net.*;
import java.util.*;

import jpl.JPL;
import jpl.Atom;
import jpl.Query;
import jpl.Term;
import jpl.Variable;
import jpl.fli.*;
import jpl.Compound;




 class NamedVariable{
      /* creates a new Named Variable */
      public NamedVariable(String Name){
        varValue = new Variable();
	name = Name;
      }

      /** returns the variable value
        * if it's a atom null is returned
	*/
      public Variable getVariable(){
         return varValue;
      }


      /** returns the name of this variable*/
      public String getName(){
         return name;
      }

      /** returns true if the Name is equal
        */
      public boolean equals(Object o){
         if(o==null | !(o instanceof NamedVariable))
	    return false;
	 return name.equals( ((NamedVariable) o).name);
      }

      /** returns the variable-value if it's a variable,
        * otherwise the atom-value is returned
	*/
      public Term getTerm(){
         return varValue;
      }

      public String toString(){
         return name;
      }

      private String name;
      private Variable varValue;

   }



