package wrapper;	

import java.lang.reflect.*;

/** A class containing a class and a method.
  * This is required to handle the wrapping of inherited methods.
  **/

public class ClassMethod{

   /** Creates a new instance of ClassMethod */
   public ClassMethod(Class c,Method M){
       this.c = c;
       this.m = M;
   }

   /** Returns a String representatio of this. */
   public String toString(){
     return "[C:"+c+" , M:"+m+"]";
   }

   /** The managed Class */ 
   public Class c;
   /** The managed Method */
   public Method m;


}
