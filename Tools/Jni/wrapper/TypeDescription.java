package wrapper;

/** A class representing the type description for an
  * algebra types.
  **/
public class TypeDescription{

/** Creates a new instance with given name. */
public TypeDescription(String name){
     this.name =name;
     signature = "";
     typeList="";
     valueList ="";
     valueListExample="";
}

/** Sets the name of this operator. */
public void setName(String name){
    this.name = name;
}

/** Sets the signature. */
public void setSignature(String signature){
    this.signature = signature;
}

/** Sets the decriptions of the type list. */
public void setTypeList(String typeList){
    this.typeList = typeList;
}

/** Sets the descriiption of the value list. */
public void setValueList(String valueList){
   this.valueList = valueList;
}

/** Sets an example of a value list. */
public void setValueListExample(String valueListExample){
   this.valueListExample = valueListExample;
}

/** Returns the name of this type. */
public String getName(){
    return name;
}

/** Returns the Signature. */
public String getSignature(){
    return signature;
}

/** Returns the TypeList **/
public String getTypeList(){
   return typeList;
}

/** Returns the ValueList. **/
public String getValueList(){
   return valueList;
}

/** Returns an example value list. */
public String getValueListExample(){
   return valueListExample;
}


/** returns the string creating the listexpr for 
  * this type dscriction when compiled via cpp.
  **/
public String toSecondoSpec(boolean printWarnings){
   boolean Warning=false;
   if(printWarnings){
      if(name.equals("")){
         System.err.println("Warning: empty name in type description");
         Warning = true;
      }
      if(signature.equals("")){
         System.err.println("Warning: empty Signature in type description of"+name);
         Warning = true;
      }
      if(typeList.equals("")){
         System.err.println("Warning: empty typelist in type description of"+name);
         Warning=true;
      }
      if(valueList.equals("")){
         System.err.println("Warning: empty valuelist in type description of"+name);
         Warning = true;
      }
      if(valueListExample.equals("")){
         System.err.println("Warning: empty value list example in type description of"+name);
         Warning =true;
      }
      if(Warning){
         System.err.println("The Type is"+ this);
      }
   }
   

   String res = "static ListExpr "+name+"Property(){\n";
   res += "   ListExpr typeList,signature,valueList,exampleList;\n";

   if(typeList.length()>Environment.MAXSTRINGLENGTH ||
      typeList.indexOf('\"')>=0){
        res += "   typeList = nl->TextAtom();\n";
        res += "   nl->AppendText(typeList,\""+getTextString(typeList)+"\");\n";
   } else{
        res += "   typeList = nl->StringAtom(\""+typeList+"\");\n";
   }

   if(signature.length()>Environment.MAXSTRINGLENGTH ||
      signature.indexOf('\"')>=0){
        res += "   signature = nl->TextAtom();\n";
        res += "   nl->AppendText(signature,\""+getTextString(signature)+"\");\n";
   } else{
        res += "   signature = nl->StringAtom(\""+signature+"\");\n";
   }

   if(valueList.length()>Environment.MAXSTRINGLENGTH ||
      valueList.indexOf('\"')>=0 ){
    
        res += "   valueList = nl->TextAtom();\n";
        res += "   nl->AppendText(valueList,\""+getTextString(valueList)+"\");\n";
   } else{
        res += "   valueList = nl->StringAtom(\""+valueList+"\");\n";
   }


   if(valueListExample.length()>Environment.MAXSTRINGLENGTH ||
      valueListExample.indexOf('\"')>=0 ){
        res += "   exampleList = nl->TextAtom();\n";
        res += "   nl->AppendText(exampleList,\""+getTextString(valueListExample)+"\");\n";
   } else{
        res += "   exampleList = nl->StringAtom(\""+valueListExample+"\");\n";
   }
   res += " return  (nl->TwoElemList(\n";
   res += "                 nl->FourElemList(\n";
   res += "                 nl->StringAtom(\"Signature\"),\n";
   res += "                 nl->StringAtom(\"Example Type List\"),\n";
   res += "                 nl->StringAtom(\"List Representation\"),\n";
   res += "                 nl->StringAtom(\"Example List\")),\n";
   res += "              nl->FourElemList(\n";
   res += "                 signature, \n";
   res += "                 typeList, \n"; 
   res += "                 valueList, \n";
   res += "                 exampleList )));"; 
   res += "}";
   return res;
}

/** returns the String which can be used to build a text atom 
  * containing a.
  */
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

/** Returns a string representation of this. */
public String toString(){
   return "TD["+name+", "+signature+", "+typeList+", "+valueList+", "+valueListExample+"]";

}



private String name;
private String signature;
private String typeList;
private String valueList;
private String valueListExample;


}
