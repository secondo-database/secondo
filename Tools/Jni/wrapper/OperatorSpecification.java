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

package wrapper;
/** Class modelling a specification of an operator */
public class OperatorSpecification{

/** Creates a new instance with empty descriptions. */
public OperatorSpecification(){
    OperatorName = "";
    Signature = "";
    Syntax = "";
    Example = "";
}

/** Creates a ne instance with given name and with empty descriptions. */
public OperatorSpecification(String OperatorName){
   this();
   setName(OperatorName);
}

/** Sets the operators name. */
public void setName(String OpName){
   this.OperatorName = OpName;
}

/** Sets the signatur of this operator. */
public void setSignature(String S){
   this.Signature = S;
}

/** Sets the syntax of this operator. */
public void setSyntax(String S){
   this.Syntax = S;
}

/** Sets the example query. **/ 
public void setExample(String E){
   this.Example = E;
}

/** Returns a String representation of this. */
public String toString(){
   return "Spec["+OperatorName+" , "+Signature+", "+Syntax+", "+Example+"]";
}

/** Returns the content of an textatom with represents the given String. */
private String getTextString(String a){
   String res = "";
   int pos;
   while((pos =a.indexOf('\"'))>=0){
      res += a.substring(0,pos);
      a = a.substring(pos+1);
      res += "\\\"";
   }
   res += a;
   return res;
}

/** Returns the SecondoSpecification of this operator 
  **/
public String toSecondoSpec(boolean printWarnings){
   if(printWarnings){
      if(OperatorName.equals(""))
         System.err.println("Warning: empty name in operator");
      if(Signature.equals(""))
         System.err.println("Warning: empty signature in operator"+OperatorName);
      if(Syntax.equals(""))
         System.err.println("Warning: empty syntax in operator"+OperatorName);
      if(Example.equals(""))
         System.err.println("Warning: empty example in operator"+OperatorName);
   } 
   String res = "const string "+OperatorName+"_spec=\n";
          res += ("     \"( ( \\\"Signature\\\" \\\"Syntax\\\" \\\"Example\\\")\"\n");
          res += "      \"(\"\n";
          int max = Environment.MAXSTRINGLENGTH;

          if(Signature.length()>max || Signature.indexOf('\"')>=0 )
              res += "    \"<text>"+getTextString(Signature)+"</text--->\"\n";    
          else
              res += "    \"\\\""+Signature+"\\\"\"\n";

          if(Syntax.length()>max || Syntax.indexOf('\"')>=0 )
              res += "     \"<text>"+getTextString(Syntax)+"</text--->\"\n";    
          else
              res += "     \"\\\""+Syntax+"\\\"\"\n";

          if(Example.length()>max || Example.indexOf('\"')>=0 )
              res += "     \"<text>"+getTextString(Example)+"</text--->\"\n";    
          else
              res += "     \"\\\""+Example+"\\\"\"\n";
          res += "\"))\";";
          return res;

}

/** Returns the name of this operator. **/
public String getOperatorName(){ return OperatorName;}

/** Returns the signature of this operator. */
public String getSignature(){ return Signature; }
  
/** returns the syntax of this operator. **/
public String getSyntax(){ return Syntax; }

/** Returns an example use of this operator. */
public String getExample(){ return Example; }


private String OperatorName;
private String Signature;
private String Syntax;
private String Example;

}
