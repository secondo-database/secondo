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



