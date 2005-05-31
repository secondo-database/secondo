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
import java.io.*;
import java.lang.reflect.*;
import java.util.Vector;


/**
  * A class for writing a Cpp wrapper for an Java algebra. 
  */
public class AlgebraWriter{

/** writes the algebra build from the given methods and classed
  * to out. 
  */
public static boolean writeAlgebra(PrintStream out,
                                   String AlgebraName,
                                   String AuthorName,
                                   Class[] Classes,
                                   ClassMethod[] Methods,
                                   OperatorSpecification[] Specs,
                                   TypeDescription[] Descs
                                   ){

  // first: extract the classes from the classmethods
  Vector classV = new Vector();
  for(int i=0;i<Methods.length;i++){
       // set the number
       Methods[i].number=i;
       Class C = Methods[i].c;
       if(!classV.contains(C)) 
          classV.add(C);
  }

  
  // append classes without algebra methods
  for(int i=0;i<Classes.length;i++){
       Class C = Classes[i];
       if(!classV.contains(C))
          classV.add(C);
  }

  // copy the classes into an array  
  Class[] classes = new Class[classV.size()];
  for(int i=0;i<classes.length;i++)
      classes[i] = (Class) classV.get(i);

  // print out the header (includes and so on )
  printHeader(out,AlgebraName,AuthorName,classes,Methods);
 
  // print forward declarations for the classes
  out.println("/*\n");
  out.println("1.0 Forward Declarations for Algebra Classes \n\n");
  out.println("*/");
   
  for(int i=0;i<classes.length;i++)
     if(canBeWrappedAsType(classes[i]))
        out.println("class "+getCppName(classes[i])+";");

  out.println("\n/*\n");
  out.println("1.1 Method IDs for some functions \n\n");
  out.println("*/");
  for(int i=0;i<classes.length;i++)
      if(canBeWrappedAsType(classes[i])){
          String clsname = getShortString(classes[i]);
          out.println("jmethodID "+clsname+"_getHashValue_ID;");
          out.println("jmethodID "+clsname+"_compareTo_ID;");
          out.println("jmethodID "+clsname+"_toListExpr_ID;");
          out.println("jmethodID "+clsname+"_std_constr_ID;");
          out.println("jmethodID "+clsname+"_loadFrom_ID;");
      }
     

  // print out a C++ class for each java class which can be wrapped 
  printWrapper(out,classes,Methods,AlgebraName,AuthorName);
  printSignatures(out,classes,Descs);  
  printKindCheckings(out,classes);
  printTypeConstructors(out,classes);

  printOperators(out,Methods,Specs);

 
  printAlgebra(out,classes,Methods,AlgebraName);
  printInitialization(out,AlgebraName,Classes,Methods);  
  


  return true;
}


/** returns the names of the operators from the given array
  * of ClassMethods.
  */
public static String[] getOperatorNames(ClassMethod[] CMs){
    Vector V = new Vector();
    for(int i=0;i<CMs.length;i++){
        String Name = CMs[i].m.getName();
        if(!V.contains(Name))
           V.add(Name);
    }
    String[] res = new String[V.size()];
    for( int i=0;i<res.length;i++)
        res[i] = (String) V.get(i);
    return res;
}

/** return the c++ name of M 
  * In general just the java name of the Method is
  * returned. In cases of conflicts with cpp keyword,
  * the name is changed by adding a _cpp. 
  */

private static String getFunctionName(Method M){
  String r = M.getName();
  if(r.equals("union"))
     r += CPP;
  if(r.equals("struct"))
     r+= CPP;
  // add further keywords here
  return r; 
}	

/** Returns true if C can be wrapped.
  * Because i have not found any restructions for the classes,
  * just true is returned.
  **/
public static boolean canBeWrapped(Class C){
    return true;
}


/** Returns true if C can be used as an algebra type.
  * This requires that the class implements the AlgebraType interface
  * and that the class contains a constructor without any argument.
  */
public static boolean canBeWrappedAsType(Class C){
  // class must implement the AlgebraType interface
  // and a constructor whithout any argument must exists
  if(!C.isPrimitive()){
     try{
       Object o = C.newInstance(); // throws an expetion if no empty constructor exists
       return (o instanceof wrapper.AlgebraType);  
     } catch(Exception e){
        return false;
     }
   }else{ // C is primitive
     // only the types from the standard algebra can be used
     // please uncomment the types implemented in the StandardAlgebra
     boolean res =  
               C.equals(Boolean.TYPE) 
            || C.equals(Integer.TYPE) 
            || C.equals(Long.TYPE)    
            || C.equals(Float.TYPE)   
            || C.equals(Double.TYPE)  
         // || C.equals(Byte.TYPE)    
         // || C.equals(Char.TYPE)    
         // || C.equals(Void.TYPE)
            ;
     return res;
   }
}

/** Returns true if M can be used as an operator. 
  * This mthod checks wether the return type as well as
  * all aparameter types can be wrapped.
  **/
public static boolean canBeWrapped(Method M){
   boolean ok = canBeWrappedAsType(M.getReturnType());
   if(!ok) return false;
   Class[] args = M.getParameterTypes();
   ok = canBeWrappedAsType(M.getReturnType());
   for(int i=0;i<args.length && ok; i++)
      ok = canBeWrappedAsType(args[i]);
   return ok;
}

/** Returns true if CM can be wrapped. 
  * This means, the method of CM must be wrappable and
  * in the case of a non-statc method also the class of CM
  * must be able to be wrapped.
  **/
public static boolean canBeWrapped(ClassMethod CM){
   boolean ok = true;
   if(!Modifier.isStatic(CM.m.getModifiers())){
      ok = canBeWrappedAsType(CM.c);
   }
   return ok && canBeWrapped(CM.m);   
}

/* checks whether CM can be wrapped in the context of classes;*/
public static boolean canBeWrapped(ClassMethod CM, Class[] Classes){
   boolean ok = true;
   if(!Modifier.isStatic(CM.m.getModifiers())){
     ok = canBeWrapped(CM.c) && contains(Classes,CM.c);
   }
   Class RT =  CM.m.getReturnType();
   ok = canBeWrappedAsType(RT) && (contains(Classes,RT)||RT.isPrimitive());
 
   Class[] args = CM.m.getParameterTypes();
   for(int i=0;ok&& i<args.length;i++){
      ok = ok && canBeWrappedAsType(args[i])  && 
                 (contains(Classes,args[i]) || args[i].isPrimitive());
   }
   return  ok; 
}

/** returns true if C in element of classes */
private static boolean contains(Class[] Classes, Class C){
   for(int i=0;i<Classes.length;i++)
      if(Classes[i].equals(C))
         return true;
   return false;
}



/* returns all classes from classes which can be wrapped
 * as an algebra types;
 */
public static Class[] getWrapableClasses(Class[] Classes){
   Vector TMP = new Vector(Classes.length);
   for(int i=0;i<Classes.length;i++)
     if(!TMP.contains(Classes[i]))
        if(canBeWrappedAsType(Classes[i]))
            TMP.add(Classes[i]);
   Class[] result = new Class[TMP.size()];
   for(int i=0;i<result.length;i++)
       result[i] = (Class) TMP.get(i);

   return result;
}


/* returns the all wrapable methods for the given classes */
public static ClassMethod[] getWrapableMethods(Class[] Classes,boolean inherited){
   Vector tmp = new Vector();
   Method[] Ms;
   Class cls;
   boolean isType;
   for(int i=0;i<Classes.length;i++){
       cls = Classes[i];
       isType = canBeWrappedAsType(cls);
       if(inherited)
          Ms = cls.getMethods();
       else
          Ms = cls.getDeclaredMethods();
       for(int j=0;j<Ms.length;j++){
          if(isType || Modifier.isStatic(Ms[j].getModifiers())){
             ClassMethod CM = new ClassMethod(Classes[i],Ms[j]);
             if(canBeWrapped(CM,Classes)) 
                 if(!tmp.contains(CM))
                     tmp.add(CM);
          }
       }
         
   }
   ClassMethod[] result = new ClassMethod[tmp.size()];
   for(int i=0;i<result.length;i++)
       result[i] = (ClassMethod) tmp.get(i);
   return result;  
}


/** check wether the resulting types are unique 
  *  the twice StringBuffer will contain all classNames which are 
  * not unique
  **/ 
public static boolean unique(Class[] Classes,StringBuffer Twices){
   if(AlgebraTypeClass==null){
      try{
         AlgebraTypeClass = Class.forName("wrapper.AlgebraType");
      }catch(Exception e){
         Twices.append("error in loading AlgebraType class ");
         return false;
      }
   } 
   Vector Names = new Vector();
   Vector TwicedNames = new Vector();
   for(int i=0;i<Classes.length;i++){
       String CurrentName = Classes[i].getName();
       // check for AlgebraType
       Class[] Interfaces = Classes[i].getInterfaces();
       boolean found = false;
       for(int k=0;!found && k<Interfaces.length;k++)
          if(Interfaces[k].equals(AlgebraTypeClass))
             found = true;
      if(found){
         if(!Names.contains(CurrentName))
             Names.add(CurrentName);
         else if(!TwicedNames.contains(CurrentName))
             TwicedNames.add(CurrentName);
      }
   }
   Twices.delete(0,Twices.length());
   if(TwicedNames.size()==0)
        return true;
   for(int i=0;i<TwicedNames.size();i++){
      if (i>0) Twices.append(", ");
      Twices.append((String) TwicedNames.get(i)); 
   }
   return false;
}

/** checks whether the resulting operators are unique */
public static boolean unique(ClassMethod[] CMs, StringBuffer Twices){
     Vector Names = new Vector();
     Vector TwicedNames = new Vector();
     Twices.delete(0,Twices.length());
     for(int i=0;i<CMs.length;i++){
        ClassMethod CM = CMs[i];
        String Signature =  getSignature(CM);
        if(!Names.contains(Signature))
           Names.add(Signature);
        else
           if(!TwicedNames.contains(Signature))
              TwicedNames.add(Signature);
     }
    if(TwicedNames.size()==0)
      return true;
    for(int i=0;i<TwicedNames.size();i++){
       if (i>0) Twices.append(", ");
       Twices.append((String) TwicedNames.get(i)); 
    }
    return false;
}
  
/** Returns the signature of the operator derived from CM.
  */
private static String getSignature(ClassMethod CM){
   Class RT = CM.m.getReturnType();
   Class[] TMPParams = CM.m.getParameterTypes();
   Class[] Params;
   if(!Modifier.isStatic(CM.m.getModifiers())){
      Params  = new Class[TMPParams.length+1];
      Params[0] = CM.c;
      for(int i=0;i<TMPParams.length;i++)
         Params[i+1] = TMPParams[i];
   }else
      Params = TMPParams;


   String result = getCppName(RT)+" (";
   for(int i=0;i<Params.length;i++){
       if(i>0) result += ", ";
       result += getCppName(Params[i]);
   }
   result += ");";
   return result;
}


/* Returns the method without packagenames for uses classes */
public static String getShortString(Method M){
  Class RT = M.getReturnType();
  String res = getShortString(RT)+" ";
  res += M.getName()+"(";
  Class[] args = M.getParameterTypes();
  for(int i=0;i<args.length;i++){
     if(i>0) res += ", ";
     res += getShortString(args[i]);
  }
  res +=")";
  return res;

}

/** Returns the name of the class without the packageName **/
public static String getShortString(Class C){
   String res =  C.getName();
   if(C.isPrimitive())
      return res;
   if(C.getPackage()==null)
      return res;
   String p = C.getPackage().getName();
   return res.substring(p.length()+1);
}



/** This function writes the includes, the class declarations,
  * and the definitions to out.
  */
private static void printWrapper(PrintStream out,Class[] Cs,ClassMethod[] Cms,
                                 String AlgebraName, String AuthorName){
   printClassDeclarations(out,Cs,Cms);
   printDefinitions(out,Cs,Cms);
   printStandardFunctions(out,Cs);
}



/** Prints the wrapper for all given methods to out **/
private static void printDefinitions(PrintStream out,
                                     Class[] classes,
                                     ClassMethod[] methods){
   out.println("/*");
   out.println("1.5 Class Definitions\n");
   out.println("*/");
   Vector Methods = new Vector();
   for(int i=0;i<classes.length;i++){
       if(canBeWrappedAsType(classes[i])){ 
          Methods.clear();
          for(int j=0;j<methods.length;j++)
              if(methods[j].c.equals(classes[i]))
                  Methods.add(new MethodWithIDNumber(methods[j].m,methods[j].number));
          printDefinitions(out,classes[i],Methods);
       }
   }
}


/** Prints the method wrappers for the given class to out. **/
private static  void printDefinitions(PrintStream out,Class cls,Vector Ms){

   // at first, we print out the "standard" constructors and functions for this class
   String Name = getShortString(cls);
   out.println("/*\n");
   out.println(" 1.2.3 Constructor building the cpp object from the java one ");
   out.println("\n*/");
   out.println(Name+"::"+Name+"(const jobject jobj):objectData(1){");
   out.println("   __TRACE__");
   out.println("   canDelete = false;");
   String FullName = getString(cls);
   out.println("   obj = jobj; // create the corresponding FLOB ");
   out.println("   RestoreFLOBFromJavaObject(); ");
   out.println("   defined = true; ");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.4 Constructor taking the initial size for the FLOB\n");
   out.println("*/");
   out.println(Name+"::"+Name+"(const int size):");
   out.println("     objectData(size), ");
   out.println("     canDelete(false), ");
   out.println("     defined(false) ");
   out.println("    {obj=0;}\n\n");

   out.println("/*\n");
   out.println("1.2.5 Destructor for "+Name +"\n");
   out.println("*/");
   out.println(Name+"::~"+Name+"(){");
   out.println("   __TRACE__");
   out.println("   if(canDelete){");
   out.println("      objectData.Destroy();");
   out.println("   }");
   out.println("   if(obj) ");
   out.println("      env->DeleteLocalRef(obj);");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.6 Clone function \n");
   out.println("*/");
   out.println(Name+"* "+Name+"::Clone(){");
   out.println("   __TRACE__");
   out.println("   "+Name+"* res = new "+Name+"(objectData.Size());");
   out.println("   res->CopyFrom(this); ");
   out.println("   return res;");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.7 The ~HashValue~ Function \n");
   out.println("*/");
   out.println("size_t "+Name+"::HashValue(){ ");
   out.println("   __TRACE__");
   out.println("   if(!obj)");
   out.println("       RestoreJavaObjectFromFLOB();");
   out.println("   jmethodID mid = "+getShortString(cls)+"_getHashValue_ID;");
   out.println("   return (size_t) env->CallIntMethod(obj,mid);");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.8 The ~CopyFrom~ function \n");
   out.println("*/");
   out.println("void "+Name+"::CopyFrom(StandardAttribute* right){");
   out.println("   __TRACE__");
   out.println("   "+Name +"* R = ("+Name+"*) right;");
   out.println("   objectData.Resize(R->objectData.Size());");
   out.println("   char* data = new char[R->objectData.Size()];");
   out.println("   R->objectData.Get(0,R->objectData.Size(),data);");
   out.println("   objectData.Put(0,R->objectData.Size(),data);");
   out.println("   delete [] data;");
   out.println("   if(obj)");
   out.println("      env->DeleteLocalRef(obj);"); 
   out.println("   obj=0;");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.9 The ~Compare~ function \n");
   out.println("*/");
   out.println("int "+Name+"::Compare(Attribute* arg){");
   out.println("   __TRACE__");
   out.println("   jmethodID mid; ");
   out.println("   mid = "+getShortString(cls)+"_compareTo_ID;");
   out.println("   if(mid==0) error(__LINE__);");
   out.println("   if(!obj)");
   out.println("     RestoreJavaObjectFromFLOB();");
   out.println("   "+Name+" *C = ("+Name+" *)arg;");
   out.println("   return env->CallIntMethod(obj,mid,C->GetObject());  ");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.10 The ~Adjacent~ function \n");
   out.println("*/");
   out.println("bool "+Name+"::Adjacent(Attribute* arg){");
   out.println("   __TRACE__");
   out.println("   return  false; ");
   out.println("}\n\n");


   out.println("/*\n");
   out.println("1.2.10 The ~NumOfFLOBs~ function \n");
   out.println("*/");
   out.println("int "+Name+"::NumOfFLOBs(){");
   out.println("   __TRACE__");
   out.println("   return  1; ");
   out.println("}\n\n");


   out.println("/*\n");
   out.println("1.2.10 The ~GetFLOB~ function \n");
   out.println("*/");
   out.println("FLOB* "+Name+"::GetFLOB(const int i){");
   out.println("   __TRACE__");
   out.println("   assert(i==0);");
   out.println("   return  &objectData; ");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.11 The ~RestoreFLOBFromJavaObject~ function \n");
   out.println("This function fills the content of the FLOB with the");
   out.println("data from the java object.\n");
   out.println("*/");
   out.println("void "+Name+"::RestoreFLOBFromJavaObject(){");
   out.println("   __TRACE__");
   out.println("   // call the method ");
   out.println("   jbyteArray jbytes;");
   out.println("   jbytes = (jbyteArray) env->CallStaticObjectMethod(Serializer_class,");
   out.println("                  Serializer_writeToByteArray_ID,obj);");
   out.println("   if(jbytes==0) error(__LINE__);");
   out.println("   int size = env->GetArrayLength(jbytes);");
   out.println("   char *bytes = (char *) env->GetByteArrayElements(jbytes,0);");
   out.println("   objectData.Resize(size);");
   out.println("   objectData.Put(0,size,bytes);");
   out.println("   env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,0);");
   out.println("   env->DeleteLocalRef(jbytes); ");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("1.2.12 The ~RestoreJavaObjectFromFLOB~ function \n");
   out.println("This functions takes the content of the FLOB and ");
   out.println("creates the Java object from it. \n");
   out.println("*/");
   out.println("void "+Name+"::RestoreJavaObjectFromFLOB(){");
   out.println("   __TRACE__");
   out.println("   // get the jaca class ");
   out.println("   if(&objectData == 0) error(__LINE__); ");
   out.println("   int size = objectData.Size(); ");
   out.println("   if(size==0) error(__LINE__);");
   out.println("   char *bytes = new char[size]; ");
   out.println("   objectData.Get(0,size,bytes);" );
   out.println("   // copy the data into a java array ");
   out.println("   jbyteArray jbytes = env->NewByteArray(size);");
   out.println("   env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);");
   out.println("   obj = env->CallStaticObjectMethod(Serializer_class,");
   out.println("               Serializer_readFrom_ID ,jbytes);");
   out.println("   if(obj==0) error(__LINE__); ");
   out.println("   jbyte* elems = env->GetByteArrayElements(jbytes,0);");
   out.println("   env->ReleaseByteArrayElements(jbytes,elems,0);");
   out.println("   delete [] bytes; ");
   out.println("   bytes=0; ");
   out.println("   env->DeleteLocalRef(jbytes); ");
   out.println("}\n\n");


   out.println("/*\n");
   out.println("1.2.13 The ~ToListExpr~ Function \n");
   out.println("*/");
   out.println("ListExpr "+Name+"::ToListExpr(ListExpr typeInfo){");
   out.println("   __TRACE__"); 
   out.println("   jmethodID mid ="+ getShortString(cls)+"_toListExpr_ID;");
   //out.println("   jobject jtype = jnitool->GetJavaList(env,typeInfo);");
   out.println("   if(!obj)");
   out.println("      RestoreJavaObjectFromFLOB();");
   out.println("   jobject jres = env->CallObjectMethod(obj,mid,0);");
   out.println("   if(jres==0) error(__LINE__);");
   out.println("   ListExpr res = jnitool->GetCppList(env,jres);");
   out.println("   env->DeleteLocalRef(jres); ");
//   out.println("   env->DeleteLocalRef(jtype); ");
   out.println("   return res; ");
   out.println("}");

     

   // write Wrapper functions for the operators
   for(int i=0;i<Ms.size();i++){
      MethodWithIDNumber  mwidn = (MethodWithIDNumber) Ms.get(i);
      
      if(!Modifier.isStatic(mwidn.m.getModifiers())){ 
         out.println("/*\n");
         out.println("1.2.1 Wrapper Function for "+mwidn.m);
         out.println("\n*/");
         out.println(getCppMethod(cls,mwidn.m,mwidn.idnumber));
      }
   }
}


/** Returns the Wrapper for a given functions as String **/

private static String getCppMethod(Class cls, Method M,int methodidnumber){
   String res ;
   String Name = getShortString(cls);
   Class RT = M.getReturnType();
   res = getShortString(RT);
   if(res.equals("boolean"))
      res = "bool";
   if(!RT.isPrimitive())
       res+="*";
   res += " ";
   res += getShortString(cls)+"::";
   res += getFunctionName(M)+"(";
   Class[] args = M.getParameterTypes();
   for(int i=0;i<args.length;i++){
       if(i>0) res+=", ";
       String Param = getShortString(args[i]);
       if(Param.equals("boolean"))
          Param = "bool";
       res += Param;
       if(!args[i].isPrimitive())
           res +="*";    
       res += "  P"+i;
   }
   res += ")  { \n";
   res += "  __TRACE__\n";
   res +="  jmethodID mid=theMethodIDs["+methodidnumber+"];\n";

   // creating the objects if needed
   res += "  if(!obj)\n";
   res += "     RestoreJavaObjectFromFLOB();\n";  
   String Call = getJNICall(M);
   RT = M.getReturnType();
   String CallParams="";
   for(int i=0;i<args.length;i++){
      if(args[i].isPrimitive())
         CallParams += ", P"+i;
      else
         CallParams += ", P"+i+"->GetObject()";
   }
   String O = Modifier.isStatic(M.getModifiers())?"cls":"obj";

   if(RT.isPrimitive()){
      if(!RT.equals(Void.TYPE)){
            res += "  return env->"+Call+"("+O+",mid"+CallParams+");\n";
      }
      else{
         res += "   env->"+Call+"("+O+",mid"+CallParams+");\n";
      }
   }
   else{
     res += "  jobject jobj = env->"+Call+"("+O+",mid"+CallParams+");\n";
     res += "  if(jobj==0) error(__LINE__);\n";
     res += "  return new "+getShortString(RT)+"(jobj);\n";
   }
   res +="}\n";
   return res;
}

/** Returns the Name and Signature of a Method
  * in jvm readable format
  */
private static String[] getSignature(Method M){
  String[] result = new String[2];
  result[0] = M.getName();
  Class ReturnClass = M.getReturnType();
  String ReturnString = getJNIString(ReturnClass);
  String ParamString = "(";
  Class[] ParamClasses = M.getParameterTypes();
  for(int i=0;i<ParamClasses.length;i++)
     ParamString += getJNIString(ParamClasses[i]);
  ParamString += ")";
  result[1]=ParamString+ReturnString;
  return result;
}


/** returns the correct jni call **/
private static String getJNICall(Method M){
   Class C = M.getReturnType();
   String Pre = Modifier.isStatic(M.getModifiers())?"CallStatic":"Call";
   String type="";
   if(C.isPrimitive()){
     if(C.equals(Boolean.TYPE))
        type = "Boolean";
     if(C.equals(Character.TYPE))
        type ="Char";
     if(C.equals(Byte.TYPE))
        type="Byte";
     if(C.equals(Short.TYPE))
        type ="Short";
     if(C.equals(Integer.TYPE))
        type="Int";
     if(C.equals(Long.TYPE))
        type="Long";
     if(C.equals(Float.TYPE))
        type="Float";
     if(C.equals(Double.TYPE))
        type="Double";
     if(C.equals(Void.TYPE))
        type="Void";
  } else
  if( C.isArray())
     type ="Array";
  else
    type ="Object";

  return Pre+type+"Method";
}



/** returns the jni code for the given class */
private static  String getJNIString(Class C){
  if(C.isPrimitive()){
     if(C.equals(Boolean.TYPE))
        return "Z";
     if(C.equals(Character.TYPE))
        return "C";

     if(C.equals(Byte.TYPE))
        return "B";

     if(C.equals(Short.TYPE))
        return "S";

     if(C.equals(Integer.TYPE))
        return "I";

     if(C.equals(Long.TYPE))
        return "J";

     if(C.equals(Float.TYPE))
        return "F";

     if(C.equals(Double.TYPE))
        return "D";

     if(C.equals(Void.TYPE))
        return  "";
     return"Error in  getJNIString";
  } else

  if( C.isArray())  {
    return "["+getJNIString(C.getComponentType());
  }
  String PackageN=null;
  if(C.getPackage()!=null) 
     PackageN = C.getPackage().getName().replace('.','/');
  if(PackageN==null || PackageN.length()<1)
     return "L"+getShortString(C)+";";
  else
     return "L"+PackageN+"/"+getShortString(C)+";";

}

/** This method returns the full qualified classname withouth a leading class. */
private static String getString(Class cls){
    return (""+cls).substring(6);

}

/** Write the required includes and so on to out **/
private static void printHeader(PrintStream out,String AlgebraName, String Author,
                                Class[] Classes,ClassMethod[] Methods){

// first we write some outputs for the pd-system
out.println("/*");
out.println("1 "+ AlgebraName);
out.println("\n written by "+Author);
out.println("\n This file was generated automatically, please edit it");
out.println("only if  you know what you do.");
out.println("");
out.println("1.1 Includes ");
out.println("Additional to the includes required by all algebras in secondo");
out.println("some special includes for using jni is maked here");
out.println("\n*/\n");
out.println(" using namespace std;\n");
out.println("#include \"Algebra.h\"");
out.println("#include \"NestedList.h\"");
out.println("#include \"QueryProcessor.h\"");
out.println("#include \"StandardTypes.h\"");
out.println("#include \"StandardAttribute.h\"");
out.println("#include \"FLOB.h\"");
out.println("#include \"Attribute.h\"");
out.println("#include <jni.h>");
out.println("#include <JVMInit.h>\n");
out.println("#include \"JNITool.h\"");
out.println(" namespace "+AlgebraName+" { \n\n");
out.println("/*\n1.2 Some required instances\n\n*/\n");
out.println("static NestedList* nl;");
out.println("static QueryProcessor* qp;");
out.println("static JVMInitializer *jvminit=0;");
out.println("static JNIEnv *env;");
out.println("static JavaVM *jvm;");
out.println("static JNITool *jnitool;\n");
// static classes for easy access 
for(int i=0;i<Classes.length;i++){
       out.println("static jclass "+getShortString(Classes[i])+"_class;");
}
out.println("static jclass Serializer_class;");

// static method ids
out.println("\n// forward declarations for method ids");
out.println("static jmethodID Serializer_writeToByteArray_ID;");
out.println("static jmethodID Serializer_readFrom_ID;");

out.println("static jmethodID theMethodIDs["+Methods.length+"]; "); 

out.println("/*\n1.3 Definitions for easy tracing \n\n*/\n");
out.println("//#define __TRACE_ON__");
out.println("#ifdef __TRACE_ON__");
out.println("#define __TRACE__ cout <<__FILE__ << \"..\" << __PRETTY_FUNCTION__ << \"@\" << __LINE__ << endl;");
out.println("#else");
out.println("#define __TRACE__");
out.println("#endif\n");


out.println("/*\n1.3 An error function \n");
out.println("This function is called wheneven an error in the java code occurs.");
out.println("It writes the line number as well as a possible java exception ");
out.println("to the standard output and exists the secondo system ");
out.println("\n*/");
out.println(" static void error(int line) {");
out.println("   cerr << \"Error in \" << __FILE__ << \" in line: \" << line;");
out.println("   cerr << \".\" << endl;");
out.println("   if(env->ExceptionOccurred()) ");
out.println("        env->ExceptionDescribe();");
out.println("   assert(false);");
out.println(" }\n\n");
}


/** This function calls the print declaration method for each class.
**/
private static void printClassDeclarations(PrintStream out,
                                           Class[] classes,
                                           ClassMethod[] methods){

   out.println("/*");
   out.println("1.4 Class Declarations\n");
   out.println("*/");
   Vector Methods = new Vector();
   for(int i=0;i<classes.length;i++){
       if(canBeWrappedAsType(classes[i])){
          Methods.clear();
          for(int j=0;j<methods.length;j++)
              if(methods[j].c.equals(classes[i]))
                  Methods.add(methods[j].m);
          printClassDeclaration(out,classes[i],Methods);
      }
   }

}

/** Prints the Declaration for the Cpp class for the given Java class. **/
private static void printClassDeclaration(PrintStream out, Class C, Vector methods){
   out.println("/*");
   String Name = getShortString(C);
   out.println("1.4.1 Declaration of the class "+Name);
   out.println("\n*/\n");
   out.println("class "+Name+": public StandardAttribute{\n");
   out.println("  public :");
   // the empty standard constructor
   out.println("     "+Name+"(){}");
   // constructor taking an java object
   out.println("     "+Name+"(const jobject jobj);");
   // constructor with size definition
   out.println("     "+Name+"(const int size);");
   // the destructor
   out.println("     ~"+Name+"();");
   // some functions required for the use as an attribute type
   out.println("     void Destroy();");
   out.println("     "+Name+"* Clone();");
   out.println("     bool IsDefined() const{ __TRACE__ return defined;}");
   out.println("     void SetDefined(bool b){__TRACE__ defined =b;}");
   out.println("     size_t HashValue();");
   out.println("     void CopyFrom(StandardAttribute* right);");
   out.println("     int Compare(Attribute* arg);");
   out.println("     bool Adjacent(Attribute* arg);");
   out.println("     int NumOfFLOBs();");
   out.println("     FLOB* GetFLOB(const int i);");
   out.println("     jobject GetObject(){if(!obj)");
   out.println("                            RestoreJavaObjectFromFLOB();");
   out.println("                         return obj;}");
   out.println("     void SetObject(jobject obj){ __TRACE__ ");
   out.println("                                  this->obj=obj;}");
   out.println("     // functions required for persistent storing");
   out.println("     void RestoreJavaObjectFromFLOB();");
   out.println("     void RestoreFLOBFromJavaObject();");
   out.println("     void Initialize(){__TRACE__ obj=0;}");
   out.println("     void Finalize(){__TRACE__ ");
   out.println("                     if(obj) ");
   out.println("                       env->DeleteLocalRef(obj);");
   out.println("                     obj=0;};");
   out.println("     ListExpr ToListExpr(ListExpr typeInfo);");
   out.println("     // the next functions are the algebra operators");

   for(int i=0;i<methods.size();i++){
       Method m = (Method) methods.get(i);
       if(!Modifier.isStatic(m.getModifiers()))
           out.println("     "+ getMethodDeclaration(C,m)+";");
   }

   out.println("  private:");
   out.println("     jobject obj; // the wrapped instance");
   out.println("     // byte representation of the java object");
   out.println("     FLOB objectData;");
   out.println("     bool canDelete;");
   out.println("     bool defined;");
   out.println("};\n"); // end of the declaration
}

/**returns  a single function declaration for the specified method **/
private static String getMethodDeclaration(Class C, Method M){
   String res;
   Class RT = M.getReturnType();
   res = getShortString(RT);
   if(res.equals("boolean"))
      res = "bool";
   if(!RT.isPrimitive())
      res+="*";
   res +=(" "+getFunctionName(M)+"(");
   Class[] args = M.getParameterTypes();
   for(int i=0;i<args.length;i++) {
      if(i>0) res+=(", ");
      String Param = (getShortString(args[i]));
      if(Param.equals("boolean"))
           Param = "bool";
      res += Param;
      if(!args[i].isPrimitive())
        res +=("*");
      res += " P"+i;
   }
   res += ")";
   return res;
}

/** prints the standard functions required by type constructors **/
private static void printStandardFunctions(PrintStream out,Class[] classes){
    
    for(int i=0;i<classes.length;i++)
        if(canBeWrappedAsType(classes[i]))
           printStandardFunctions(out,classes[i]);   

}


/** prints out the standardfunctions for a single class **/
private static void printStandardFunctions(PrintStream out,Class cls){
   String Name = getShortString(cls);
   String FullName = getString(cls);
   out.println("/*\n");
   out.println("1.2.5 The standard functions required for type constructors\n"); 
   out.println(" The ~In~ function for "+Name+"\n");
   out.println("*/");
   out.println(" static Word In"+Name+"(const ListExpr typeInfo, ");
   out.println("                        const ListExpr instance, ");
   out.println("                        const int errorPos, ");
   out.println("                        ListExpr& errorInfo, ");
   out.println("                        bool& correct){");
   out.println("   __TRACE__");
   out.println("   jmethodID mid; ");
   out.println("   jobject obj; ");
   out.println("   jclass cls = "+Name+"_class;");
   out.println("   jobject jtype = jnitool->GetJavaList(env,typeInfo);");
   out.println("   jobject jinstance = jnitool->GetJavaList(env,instance);");
   out.println("   // Create an Object with the standard constructor ");
   out.println("   mid = "+Name+"_std_constr_ID;");
   out.println("   if(mid==0) error(__LINE__);");
   out.println("   obj = env->NewObject(cls,mid);");
   out.println("   if(obj==0) error(__LINE__);");
   out.println("   mid = "+Name+"_loadFrom_ID;");
   out.println("   correct = env->CallBooleanMethod(obj,mid,jtype,jinstance);");
   out.println("   env->DeleteLocalRef(jtype);");
   out.println("   env->DeleteLocalRef(jinstance);");
   out.println("   if(correct) ");
   out.println("        return SetWord(new "+Name+"(obj));");
   out.println("   // not successful ");
   out.println("   env->DeleteLocalRef(obj);");
   out.println("   return SetWord(Address(0));");
   out.println("}\n\n");

   out.println("/*");
   out.println("The ~Out~ function for "+Name+"\n");
   out.println("*/");
   out.println("static ListExpr Out"+Name+"(ListExpr typeInfo,Word value){");
   out.println("   __TRACE__");
   out.println("   "+Name+"* O = ("+Name+"*) (value.addr);");
   out.println("   ListExpr res = O->ToListExpr(typeInfo);");
   out.println("   O->Finalize(); ");
   out.println("   return res; ");
   out.println(" }\n\n");

   out.println("/*\n");
   out.println("The ~Create~ function for "+Name+"\n");
   out.println("*/");
   out.println("static Word Create"+Name+"(const ListExpr typeInfo){");
   out.println("   __TRACE__");
   out.println("   "+Name+"* res = new "+Name+"(1);");
   out.println("   res->SetObject(0);");
   out.println("   return SetWord(res);");
   out.println("}\n\n");

   out.println("/*\n");
   out.println(" The ~Delete~ Function for "+Name+"\n");
   out.println("*/");
   out.println("static void Delete"+Name+"(Word &w){");
   out.println("  __TRACE__");
   out.println("  delete ("+Name+" *)w.addr; ");
   out.println("  w.addr = 0; ");
   out.println("}\n\n");

      
   out.println("/*\n");
   out.println(" The ~Close~ Function for "+Name+"\n");
   out.println("*/");
   out.println("static void Close"+Name+"(Word &w){");
   out.println("  __TRACE__");
   out.println("  delete ("+Name+" *)w.addr; ");
   out.println("  w.addr = 0; ");
   out.println("}\n\n");

   
   out.println("/*\n");
   out.println(" The ~Clone~ Function for "+Name+"\n");
   out.println("*/");
   out.println("static Word Clone"+Name+"(const Word &w){");
   out.println("   __TRACE__");
   out.println("   return SetWord(( ("+Name+"*)w.addr)->Clone());");
   out.println("}\n\n");


   out.println("/*\n");
   out.println(" The ~Open~ Function for "+Name+"\n");
   out.println("*/");
   out.println("bool Open"+Name+"(SmiRecord& valueRecord,");
   out.println("                  size_t& offset,");
   out.println("                  const ListExpr typeInfo,");
   out.println("                  Word& value){");
   out.println("   __TRACE__");
   out.println("   "+Name+"* P = ("+Name+"*) TupleElement::Open(valueRecord,offset,typeInfo);");
   out.println("   P->SetObject(0); ");
   out.println("   value = SetWord(P); ");
   out.println("   return true; ");
   out.println("}\n\n");

   
   out.println("/*\n");
   out.println(" The ~Save~ Function for "+Name+"\n");
   out.println("*/");
   out.println("bool Save"+Name+"(SmiRecord& valueRecord,");
   out.println("                  size_t& offset,");
   out.println("                  const ListExpr typeInfo,");
   out.println("                  Word& value){");
   out.println("   __TRACE__");
   out.println("   "+Name+"* P = ("+Name+"*) value.addr;");
   out.println("   TupleElement::Save(valueRecord,offset,typeInfo,P);");
   out.println("   return true; ");
   out.println("}\n\n");

   out.println("/*\n");
   out.println("The ~SizeOf~ Function for "+Name +"\n");
   out.println("*/");
   out.println("int SizeOf"+Name+"(){\n   __TRACE__\n   return sizeof("+Name+");\n}\n\n");

   out.println("/*\n");
   out.println("The ~Cast~ Function for "+Name +"\n");
   out.println("*/");
   out.println("void* Cast"+Name+"(void* addr){");
   out.println("   __TRACE__");
   out.println("   return new (addr) "+Name+";");
   out.println("}\n\n");
}

/** Prints the property function for the gives classes to out. */
private static void printSignatures(PrintStream out, Class[] classes,TypeDescription[] Descs){
   out.println("/*\n");
   out.println("1.2.6 Signatures for the classes \n");
   out.println("*/\n");
   for(int i=0;i<classes.length;i++)
      if(canBeWrappedAsType(classes[i]))
         printSignature(out,classes[i],Descs);
}

/** Prints the property function for a given class. */
private static void printSignature(PrintStream out, Class cls,TypeDescription[] Descs){
  String Name = getSecondoType(cls);
  int pos = -1;
  for(int i=0;i<Descs.length&&pos<0;i++)
     if(Name.equals(Descs[i].getName()))
          pos = i;
  if(pos <0){
     System.err.println("Warning: No TypeDescription for type "+Name+" found"); 
     out.println(" static ListExpr "+Name+"Property(){ ");
     out.println("    return (nl->TwoElemList(");
     out.println("              nl->FourElemList(");
     out.println("                  nl->StringAtom(\"Signature\"),");
     out.println("                  nl->StringAtom(\"Example Type List\"),");
     out.println("                  nl->StringAtom(\"List Representation\"),");
     out.println("                  nl->StringAtom(\"Example List\")),");
     out.println("              nl->FourElemList(");
     out.println("                  nl->StringAtom(\"->Data\"),");
     out.println("                  nl->StringAtom(\"unknow\"),");
     out.println("                  nl->StringAtom(\"unknow\"),");
     out.println("                  nl->StringAtom(\"unknow\"))));");
     out.println(" }\n\n");
  } else{
     out.println(Descs[pos].toSecondoSpec(true));
  }
}

/** Prints the kind checking functions for all classes in classes to out. **/
private static void printKindCheckings(PrintStream out,Class[] classes){
   
   out.println("/*\n");
   out.println("1.2.6  Kind checking for the classes \n");
   out.println("*/\n");
   for(int i=0;i<classes.length;i++)
     if(canBeWrappedAsType(classes[i]))
        printKindChecking(out,classes[i]); 
}

/** Prints the kind checking function for cls to out. */
private static void printKindChecking(PrintStream out, Class cls){
    String Name = getShortString(cls);
    out.println(" static bool Check"+Name+"(ListExpr type, ListExpr& errorInfo){");
   /*
    // the following class uses the type check of algebra type
    // because on other places code exists which excludes the use
    // of composite types, we can make it simpler and
    // remove the checkType method from the AlgebraType interface
    //
    out.println("    jclass cls = "+getShortString(cls)+"_class;");
    out.println("    jmethodID mid = env->GetMethodID(cls,\"<init>\",\"()V\");");
    out.println("    if(mid==0) error(__LINE__);");
    out.println("    jobject obj = env->NewObject(cls,mid);");
    out.println("    if(obj==0) error(__LINE__);");
    out.println("    jobject tlist = jnitool->GetJavaList(env,type);");
    out.println("    mid = env->GetMethodID(cls,\"checkType\",\"(Lsj/lang/ListExpr;)Z\");");
    out.println("    if(mid==0) error(__LINE__);");
    out.println("    bool res = env->CallObjectMethod(obj,mid,tlist);");
    out.println("    env->DeleteLocalRef(tlist);");
    out.println("    env->DeleteLocalRef(obj);");
    out.println("    return res; ");
   */
   // if composite type are not allowed, use the following code:
    String SName = getSecondoType(cls);
    out.println("   __TRACE__");
    out.println("   return nl->IsEqual(type,\""+SName+"\");");

    // common code for both versions
    out.println("}\n\n");
}

/* Prints the instances for the type constructors. */
private static void printTypeConstructors(PrintStream out, Class[] classes){
   out.println("/*\n");
   out.println("1.2  Type Constructors \n");
   out.println("*/\n");
   for(int i=0;i<classes.length;i++)
       if(canBeWrappedAsType(classes[i]))
          printTypeConstructor(out,classes[i]);
}   

/** Prints the creation of the type constructor for cls to out. **/
private static void printTypeConstructor(PrintStream out,Class cls){
   String Name = getShortString(cls);
   String LoName =Name.toLowerCase();
   out.println("TypeConstructor "+LoName);
   out.println("(");
   out.println("  \""+LoName+"\",   ");
   out.println("  "+getSecondoType(cls)+"Property,  ");
   out.println("  Out"+Name+",       ");
   out.println("  In"+Name+",        ");
   out.println("  0,0,               // save and restore from list");
   out.println("  Create"+Name+",    ");
   out.println("  Delete"+Name+",    ");
   out.println("  Open"+Name+",      ");
   out.println("  Save"+Name+",      ");
   out.println("  Close"+Name+",     ");
   out.println("  Clone"+Name+",     ");
   out.println("  Cast"+Name+",      ");
   out.println("  SizeOf"+Name+",    ");
   out.println("  Check"+Name+",     ");
   out.println("  0,               // model");
   out.println("  TypeConstructor::DummyInModel,");
   out.println("  TypeConstructor::DummyOutModel,");
   out.println("  TypeConstructor::DummyValueToModel,");
   out.println("  TypeConstructor::DummyValueListToModel");
   out.println(");");                                

}

/** Prints the creation of the algebra to out. */
private static void printAlgebra(PrintStream out,Class[] classes,
                                 ClassMethod[] Methods,String AlgebraName){
   out.println("/*\n");
   out.println("1 Creating the Algebra\n");
   out.println("*/");
   out.println("class "+AlgebraName+"Algebra : public Algebra{");
   out.println("  public: ");
   out.println("    "+AlgebraName+"Algebra() : Algebra(){");
   for(int i=0;i<classes.length;i++){
      if(canBeWrappedAsType(classes[i])){
         String CN = getShortString(classes[i]).toLowerCase();
         out.println("        AddTypeConstructor( &"+CN+");");
         out.println("        "+CN+".AssociateKind(\"DATA\");");
      }
   }
   Vector UsedOps = new Vector();
   for(int i=0;i<Methods.length;i++){
      String ON = Methods[i].m.getName();
      if(!UsedOps.contains(ON)){
          out.println("        AddOperator(&"+ON+"_op);");
          UsedOps.add(ON);
      }
   }
   // add operators here
   out.println("   }");
   out.println("  ~"+AlgebraName+"Algebra(){};");
   out.println("};\n\n");
   out.println(AlgebraName+"Algebra "+ (AlgebraName+"Algebra").toLowerCase()+";");
}

/** Prints the initialization of the algebra to out. */
private static void printInitialization(PrintStream out,String AlgebraName,Class[] Classes,ClassMethod[] Methods){
    out.println("/*\n");
    out.println("3 Initialization \n");
    out.println(" We have to initialize the algebra as well as the ");
    out.println(" Java Virtual Machine.\n ");
    out.println("*/");
    out.println(" extern \"C\"");
    out.println(" Algebra* ");
    out.println(" Initialize"+AlgebraName+"Algebra( NestedList* nlRef,");
    out.println("           QueryProcessor* qpRef){");

    out.println(" \n// initialize the java environment ");
    out.println("   jvminit = new JVMInitializer(); ");
    out.println("   env = jvminit->getEnv(); ");
    out.println("   jvm = jvminit->getJVM(); ");
    out.println("   jnitool = new JNITool(env,nlRef);");
    out.println(" \n// get the used java classes  ");
    for(int i=0;i<Classes.length;i++){
        String PackageN=null;
        String FindName;
        if(Classes[i].getPackage()!=null) 
           PackageN = Classes[i].getPackage().getName().replace('.','/');
        if(PackageN==null || PackageN.length()<1)
            FindName = getShortString(Classes[i]);
        else
            FindName =  PackageN+"/"+getShortString(Classes[i]);
       out.print("   "+getShortString(Classes[i])+"_class = env->FindClass(\"");
       out.println( FindName + "\");");
       out.println("   if("+getShortString(Classes[i])+"_class==0) error(__LINE__);");
    }
    // get the standard functions for each class to wrapped
    for(int i=0;i<Classes.length;i++){
        if(canBeWrappedAsType(Classes[i])){
           String cn = getShortString(Classes[i]); 
           out.println("   "+cn+"_getHashValue_ID =");
           out.println("             env->GetMethodID(+"+cn+"_class,\"getHashValue\",\"()I\");");
           out.println("   if(!"+cn+"_getHashValue_ID) error(__LINE__);\n");
           out.println("   "+cn+"_compareTo_ID =" );
           out.println("            env->GetMethodID("+cn+"_class,\"compareTo\",\"(Ljava/lang/Object;)I\");");
           out.println("   if(!"+cn+"_compareTo_ID) error(__LINE__);\n");
           out.println("   "+cn+"_toListExpr_ID = ");
           out.println("             env->GetMethodID("+cn+"_class,\"toListExpr\",");
           out.println("                       \"(Lsj/lang/ListExpr;)Lsj/lang/ListExpr;\");");
           out.println("   if(!"+cn+"_toListExpr_ID) error(__LINE__); \n");
           out.println("   "+cn+"_loadFrom_ID =");
           out.println("             env->GetMethodID("+cn+"_class,\"loadFrom\",");
           out.println("                        \"(Lsj/lang/ListExpr;Lsj/lang/ListExpr;)Z\");");
           out.println("   if(!"+cn+"_loadFrom_ID) error(__LINE__);\n");
           out.println("   "+cn+"_std_constr_ID ="); 
           out.println("             env->GetMethodID("+cn+"_class,\"<init>\",\"()V\");");
           out.println("   if(!"+cn+"_std_constr_ID) error(__LINE__);\n");
        }
    }
    out.println("   Serializer_class = env->FindClass(\"wrapper/Serializer\");");
    out.println("   if(Serializer_class==0) error(__LINE__);"); 
    
    out.println(" \n// get the used methodids "); 

    out.println("   Serializer_writeToByteArray_ID =");
    out.println("       env->GetStaticMethodID(Serializer_class,");
    out.println("           \"writeToByteArray\",\"(Ljava/io/Serializable;)[B\");");
    out.println("   if(! Serializer_writeToByteArray_ID )");
    out.println("       error(__LINE__);\n");
    
    out.println("   Serializer_readFrom_ID =");
    out.println("       env->GetStaticMethodID(Serializer_class,\"readFrom\",");
    out.println("                            \"([B)Ljava/lang/Object;\");");
    out.println("   if(!Serializer_readFrom_ID )");
    out.println("       error(__LINE__);\n");

    // Initialize the method ids for operators
    for(int i=0;i<Methods.length;i++){
       out.println("   theMethodIDs["+i+"] =");
       out.print("          env->Get"); 
       if(Modifier.isStatic(Methods[i].m.getModifiers())){
           out.print("Static");
       }
       out.println("MethodID("+getShortString(Methods[i].c)+"_class, ");
       String[] Ps = getSignature(Methods[i].m);
       out.println("             \""+Ps[0]+"\",\""+Ps[1]+"\");");
       out.println("   if(!theMethodIDs["+i+"])");
       out.println("       error(__LINE__);");
    }



    out.println("   nl = nlRef; ");
    out.println("   qp = qpRef; ");
    out.println("   return (&" + (AlgebraName+"algebra").toLowerCase()+");");
    out.println(" }");
    out.println("} // end namespace "+AlgebraName);

}


/** Prints operators for the given methods 
  **/
private static void printOperators(PrintStream out, ClassMethod[] Methods,
                                   OperatorSpecification[] Specs){

  // print pd comment
  out.println("/*\n");
  out.println("4 Operators \n");
  out.println("*/\n");


  // first we scan all operator names and collect them in a vector
  Vector OperatorNames = new Vector();
  String CurrentName;
  for(int i=0;i<Methods.length;i++){
     CurrentName = Methods[i].m.getName();
     if(!OperatorNames.contains(CurrentName))
         OperatorNames.add(CurrentName);
  }

  // count the number of overloads
  int[] count = new int[OperatorNames.size()];
  for(int i=0;i<count.length;i++) 
     count[i]=0;

  int maxOverloads = 0;
  for(int i=0;i<Methods.length;i++){
     CurrentName=Methods[i].m.getName();
     int pos = OperatorNames.indexOf(CurrentName);
     count[pos]++;
     if(count[pos]>maxOverloads)
         maxOverloads=count[pos];
  }

  if(maxOverloads>1){  // they are overloaded operators-> create an array of dummymodels
     out.println("/*\n");
     out.println("4.0 Array of DummyModels for overloaded operators \n");
     out.println("*/");
     out.println("ModelMapping DummyModelArray[] = { ");
     for(int i=0;i<maxOverloads;i++){
          if(i>0) out.println(",");
          out.print("     Operator::DummyModel"); 
     }
     out.println("\n };"); 

  } 
 
  
  // for each Operatorname, we collect the set of methods 
  Vector ValueMappings = new Vector();
  for(int i=0;i<OperatorNames.size();i++){
      CurrentName = (String) OperatorNames.get(i);
      // get all appropriate mappings
      ValueMappings.clear(); 
      for(int j=0;j<Methods.length;j++)
         if(Methods[j].m.getName().equals(CurrentName))
             ValueMappings.add(Methods[j]);
      printOperator(i,out,CurrentName,ValueMappings,Specs);
  }
}


/** Prints all requirements for the given operator.
  */
private static void printOperator(int no,PrintStream out,  String Name, Vector ClassMethods,
                         OperatorSpecification[] Specs){

  // print pd comment
  out.println("/*\n");
  out.println("4."+no+" Operator ~"+Name+"~\n");
  out.println("*/");

  printTypeMapping(no,out,Name,ClassMethods);
  printValueMappings(no,out,Name,ClassMethods);
  printSelectionFunction(no,out,Name,ClassMethods);
  printValueMappingArray(no,out,Name,ClassMethods);
  printSpecification(no,out,Name,ClassMethods,Specs);
  printOperatorInstance(no,out,Name,ClassMethods);

} 

/** Prints the type mapping for the given operator */

private static void printTypeMapping(int no,PrintStream out, String Name, Vector Methods){
  out.println("/*\n");
  out.println("4."+no+".1 Type Mapping Function for Operator ~"+Name+"~\n");
  out.println("*/");
  out.println("static ListExpr "+Name+"_TypeMap(ListExpr args){");

  // check wether at least on value mapping requieres an parameter
  boolean param_required = false;
  for(int i=0;i<Methods.size()&&!param_required;i++){
      Method M = ((ClassMethod)Methods.get(i)).m;
      if(!Modifier.isStatic(M.getModifiers()))
         param_required=true;
      if(M.getParameterTypes().length>0)
         param_required=true;
  } 
  if(param_required){
      out.println("   bool ok=true;");
      out.println("   ListExpr arg;");
      out.println("   ListExpr tmpargs;");
  }
  for(int i=0;i<Methods.size();i++){
     printTypeMapping(out,(ClassMethod) Methods.get(i));
  }
  // no typemapping was successfully
  out.println("   return nl->SymbolAtom(\"typeerror\");");   
  out.println("}");
}

/** prints the type mapping for CM o out. **/
private static void printTypeMapping(PrintStream out,ClassMethod CM){
   Class[] Secondargs = CM.m.getParameterTypes(); 
   Class[] args;
   if(!Modifier.isStatic(CM.m.getModifiers())){
      args = new Class[Secondargs.length+1];
      args[0] = CM.c;
      for(int i=1;i<args.length;i++)
         args[i] = Secondargs[i-1];
   } else { // static function
       args = Secondargs;
   }
  
   if(args.length==0){ // without parameters
      out.println("   if(nl->ListLength(args)==0){ ");
      out.println("      return nl->SymbolAtom(\""+getSecondoType(CM.m.getReturnType())+"\");");
      out.println("   }");
   } else{
      out.println("    if(nl->ListLength(args)=="+(args.length)+"){");
      out.println("        tmpargs = args; ");
      out.println("        ok = true;");
      
      for(int i=0;i<args.length;i++){
         out.println("        arg = nl->First(tmpargs);");
         out.println("        tmpargs = nl->Rest(tmpargs);");
         out.println("        ok = ok && nl->IsEqual(arg,\""+getSecondoType(args[i])+"\");");  
      }
      out.println("        if(ok)");
      out.println("           return nl->SymbolAtom(\""+getSecondoType(CM.m.getReturnType())+"\");");
      out.println("    }");
   }
}

/** Returns the C++ Type for primitive types and the
  * lowercase classname of an object type. **/
public static String getSecondoType(Class C){
   if(!C.isPrimitive())
      return getShortString(C).toLowerCase();
   if(C.equals(Boolean.TYPE))
      return "bool";
   if(C.equals(Byte.TYPE)){
       return "byte";
   }
   if(C.equals(Character.TYPE)){
       return "char";
   }       
   if(C.equals(Short.TYPE)){
       return "int";
   }
   if(C.equals(Integer.TYPE)){
       return "int";
   }
   if(C.equals(Long.TYPE)){
       return "int";
   }
   if(C.equals(Float.TYPE)){
       return "real";
   }
   if(C.equals(Double.TYPE)){
       return "real";
   }
   if(C.equals(Void.TYPE)){
      return "void";
   }
   System.err.println("Unknow Class type detected");
   return "CcError";
}

/** Returns the cpp name of C. **/
private static String getCppName(Class C){
   if(!C.isPrimitive())
      return getShortString(C);
   String Res = C.getName();
   if(Res.equals("boolean"))
      Res = "bool";
   return Res;
}


/** Returns the standard type of C, e.g. CcReal. **/
private static String getSecondoName(Class C){
   if(!C.isPrimitive())
      return getShortString(C);
   if(C.equals(Boolean.TYPE))
      return "CcBool";
   if(C.equals(Byte.TYPE)){
       return "CcByte";
   }
   if(C.equals(Character.TYPE)){
       return "CcChar";
   }       
   if(C.equals(Short.TYPE)){
       return "CcInt";
   }
   if(C.equals(Integer.TYPE)){
       return "CcInt";
   }
   if(C.equals(Long.TYPE)){
       return "CcInt";
   }
   if(C.equals(Float.TYPE)){
       return "CcReal";
   }
   if(C.equals(Double.TYPE)){
       return "CcReal";
   }
   if(C.equals(Void.TYPE)){
      return "CcVoid";
   }
   System.err.println("Unknow Class type detected");
   return "CcError";
}


/** Prints all Value Mappingd for methods. */
private static void printValueMappings(int no,PrintStream out, String Name, Vector Methods){

   out.println("/*\n");
   out.println("4."+no+".2 Value Mappings for Operator ~"+Name+"~\n");
   out.println("*/");
   for(int i=0;i<Methods.size();i++){
        ClassMethod CM = (ClassMethod) Methods.get(i);
        if(Modifier.isStatic(CM.m.getModifiers()))
            printValueMappingStatic(out,Name,CM,CM.number);
        else
            printValueMappingNonStatic(out,Name,CM);
   }
}

/** Prints a single value maping of an operator.
  * The method in CM must be static.
  **/
private static void printValueMappingStatic(PrintStream out, String Name,
                                            ClassMethod CM,int methodidnumber){
    String VMName = getValueMappingName(CM);
    out.println("static int "+VMName+"(");
    out.println("            Word* args, Word& result, int message,");
    out.println("            Word& local, Supplier s){");
    out.println("   __TRACE__");
    out.println("   result = qp->ResultStorage(s);");
    Class[] args = CM.m.getParameterTypes();
    Class RT = CM.m.getReturnType();
    Method M = CM.m;
    // get the parameter
    for(int i=0;i<args.length;i++){
       String AType = getSecondoName(args[i]);
       out.println("   "+AType+"* arg"+(i)+" = ("+AType+"*) args["+(i)+"].addr;"); 
    }
    // print the Wrapping
    out.println("  jclass cls = "+getShortString(CM.c)+"_class;");
    out.println("  jmethodID mid = theMethodIDs["+methodidnumber+"];");
    String Call = getJNICall(M);
    RT = M.getReturnType();
    String CallParams="";
    for(int i=0;i<args.length;i++){
      if(args[i].isPrimitive())
         CallParams += ", arg"+i+getSecondoValAccess(args[i]);
      else
         CallParams += ", arg"+i+"->GetObject()";
    }
    String O = Modifier.isStatic(M.getModifiers())?"cls":"obj";
    String CRT = getCppName(RT);
    String SRT = getSecondoName(RT);
    if(RT.isPrimitive()){
        out.println("   "+CRT +" res = env->"+Call+"(cls,mid"+CallParams+");");
        out.println("   (("+SRT +"*)result.addr)->Set(true,res);");
        out.println("   return 0;");
    } else { // non primitive return value
        out.println("   jobject obj = env->"+Call+"(cls,mid"+CallParams+");");
        out.println("   "+CRT+"*res = new "+CRT+"(obj);");
        out.println("   (("+SRT+"*)result.addr)->CopyFrom(res);");
        out.println("   delete res; ");
        out.println("   return 0;"); 
    }
    out.println("}");
}


/** Prints out a value mapping for a non-static method. */
private static void printValueMappingNonStatic(PrintStream out,String Name,ClassMethod CM){
     String VMName = getValueMappingName(CM);
     out.println("static int "+VMName+"(");
     out.println("            Word* args, Word& result, int message,");
     out.println("            Word& local, Supplier s){");
     out.println("   __TRACE__");
     out.println("   result = qp->ResultStorage(s);");
     // get all parameters
     Class[] args = CM.m.getParameterTypes();
     Class RT = CM.m.getReturnType();
     String SName = getSecondoName(RT);
     String CName = getSecondoName(CM.c);
     out.println("   "+CName+"* arg0 = ("+CName+"*) args[0].addr;");  
     // get the parameter named arg0 ... argn
     for(int i=0;i<args.length;i++){
       String AType = getSecondoName(args[i]);
       out.println("   "+AType+"* arg"+(i+1)+" = ("+AType+"*) args["+(i+1)+"].addr;"); 
     }
     // print the function call
     if(!RT.isPrimitive()){ // wrapped 
        // create the new by calling the function
        out.print("   "+SName+"*  r = arg0->"+getFunctionName(CM.m)+"(");
        // printparameters
        for(int i=0;i<args.length;i++){
           if(i>0) out.print(", ");
           out.print(getSecondoCast(args[i])+"arg"+(i+1)+getSecondoValAccess(args[i]));

        }
        out.println(");");
        // copy r into result   
        out.println("   (("+SName+"*)result.addr)->CopyFrom(r);");
        out.println("   delete r; ");
        out.println("   return 0;");
     }else{ // a type from the standard algebra
        out.print("   (("+SName+"*)result.addr)->Set(true,");
        out.print("    arg0->"+getFunctionName(CM.m)+"(");
        // print parameter
        for(int i=0;i<args.length;i++){
          if(i>0) out.print(", ");
           out.print(getSecondoCast(args[i])+"arg"+(i+1)+getSecondoValAccess(args[i]));
        }
        out.println("));");
        out.println("   return 0;");
     }

     out.println("}"); 
}

/** Prints out the value access for types of the StandardAlgebra. */
private static String getSecondoValAccess(Class C){
   if(!C.isPrimitive())
      return "";
   if(C.equals(Boolean.TYPE))
      return "->GetBoolval()";
   if(C.equals(Byte.TYPE)){
       return "->GetByteval()";
   }
   if(C.equals(Character.TYPE)){
       return "->GetCharval()";
   }       
   if(C.equals(Short.TYPE)){
       return "->GetIntval()";
   }
   if(C.equals(Integer.TYPE)){
       return "->GetIntval()";
   }
   if(C.equals(Long.TYPE)){
       return "->GetIntval()";
   }
   if(C.equals(Float.TYPE)){
       return "->GetRealval()";
   }
   if(C.equals(Double.TYPE)){
       return "->GetRealval()";
   }
   if(C.equals(Void.TYPE)){
      return "->GetVoidval()";
   }
   System.err.println("Unknow Class type detected");
   return "CcError";
}

/** Returns the required cast for some types, e.g. short -> int **/
private static String getSecondoCast(Class C){
   if(!C.isPrimitive())
      return "";
   if(C.equals(Short.TYPE)){
       return "(int) ";
   }
   if(C.equals(Long.TYPE)){
       return "(int) ";
   }
   return "";
}


/** Returns the name of the valueMapping for CM. */
private static String getValueMappingName(ClassMethod CM){
    String res = CM.m.getName()+"_";
    if(!Modifier.isStatic(CM.m.getModifiers()))
           res += getCppName(CM.c)+"_";
    Class[] args = CM.m.getParameterTypes();
    for(int i=0;i<args.length;i++)
       res += getCppName(args[i])+"_";
    res += getCppName(CM.m.getReturnType());
    return res;
}

/** prints the selection function for the given operator **/
private static void  printSelectionFunction(int no,PrintStream out,String Name,Vector Methods){
   
   out.println("/*\n");
   out.println("4."+no+".3 Selection Functions for Operator ~"+Name+"~\n");
   out.println("*/");
   out.println("static int "+Name+"Select(ListExpr args){");
   if(Methods.size()<2){ // non overloaded -> simple Select
       out.println("   return 0;");
   }else{ // overloaded operator
       out.println("   bool ok;");
       out.println("   ListExpr arg;");
       out.println("   ListExpr tmpargs;");
       for(int i=0;i<Methods.size();i++){
          printSelection(i,out,(ClassMethod) Methods.get(i));
       }
       out.println("   return -1; // should never be reached");   
   }
   out.println("}");
}

/** prints the selection of a single value mapping. */
private static void printSelection(int no, PrintStream out,ClassMethod CM){
   Class[] Secondargs = CM.m.getParameterTypes(); 
   Class[] args;
   if(!Modifier.isStatic(CM.m.getModifiers())){
       args = new Class[Secondargs.length+1];
       args[0] = CM.c;
       for(int i=1;i<args.length;i++)
           args[i] = Secondargs[i-1];
   } else { // non-static class
       args = Secondargs; 
   }
   out.println("    if(nl->ListLength(args)=="+(args.length)+"){");
   out.println("        ok = true;");
   out.println("        tmpargs = args;");
      
   for(int i=0;i<args.length;i++){
      out.println("        arg = nl->First(tmpargs);");
      out.println("        tmpargs = nl->Rest(tmpargs);");
      out.println("        ok = ok && nl->IsEqual(arg,\""+getCppName(args[i]).toLowerCase()+"\");");  
   }
   out.println("        if(ok)");
   out.println("           return "+no+";");
   out.println("    }");
   
}

/** prints the valuemapping array required by overloaded operators. **/
private static void printValueMappingArray(int no,PrintStream out, String Name,Vector Methods){
    if(Methods.size()<2) return; // non-overloaded operator
    out.println("/*\n");
    out.println("4."+no+".5  Value Mapping Array  for overloaded Operator ~"+Name+"~\n");
    out.println("*/");
    out.println(" ValueMapping "+Name+"map[] = {");
    for(int i=0;i<Methods.size();i++){
        if(i>0) out.println(",");
        out.print(getValueMappingName( ((ClassMethod)Methods.get(i)))); 
    }
    out.println("};");
}

/** Prints the specifications for the operator name **/
private static void printSpecification(int no,PrintStream out,String Name,Vector Methods,
        OperatorSpecification[] Specs
    ){
    out.println("/*\n");
    out.println("4."+no+".6  Specification of Operator ~"+Name+"~\n");
    out.println("*/");
  
    // find the specification
    int pos = -1;
    for(int i=0;i<Specs.length&&pos<0;i++)
         if(Specs[i].getOperatorName().equals(Name))
            pos=i;

    if(pos<0){ // no specification found
       System.err.println("Warning: Missing specification for operator"+Name);
       out.println("const string "+Name+"_spec=");
       out.println("  \"( ( \\\"Signature\\\" \\\"Syntax\\\" \\\"Example\\\")\"");
       out.println("  \"    ( \\\"unknow\\\" \\\"unknow\\\" \\\"unknow\\\"))\";");
    } else{
       out.println(Specs[pos].toSecondoSpec(true));
    }
}

/** Prints the creation of a operator instance. **/
private static void printOperatorInstance(int no, PrintStream out, String Name,Vector Methods){
    
    out.println("/*\n");
    out.println("4."+no+".7  Create instance of  Operator ~"+Name+"~\n");
    out.println("*/");
    if(Methods.size()>1){ // overloaded operator
       out.println("Operator "+Name+"_op");
       out.println("  (");
       out.println("   \""+Name+"\",     // Name ");
       out.println("   "+Name+"_spec,   // specification");
       out.println("   "+Methods.size()+",   // number of function ");
       out.println("    "+Name+"map,   // Value mapping array ");
       out.println("   DummyModelArray,  // model mapping");
       out.println("   "+Name+"Select, // selection function ");
       out.println("   "+Name+"_TypeMap // type mapping fucntion");
       out.println(");");
     } else{ // non overloaded operator
       out.println("Operator "+Name+"_op");
       out.println("  (");
       out.println("   \""+Name+"\",     // Name ");
       out.println("   "+Name+"_spec,   // specification");
       out.println("    "+getValueMappingName((ClassMethod)Methods.get(0))+",   // Value mapping function ");
       out.println("   Operator::DummyModel,  // model mapping");
       out.println("   "+Name+"Select, // selection function ");
       out.println("   "+Name+"_TypeMap // type mapping fucntion");
       out.println(");");
     }

}



static final String CPP = "_cpp"; 
static Class AlgebraTypeClass = null;

private static class MethodWithIDNumber{

  MethodWithIDNumber(Method m, int idnumber){
     this.m = m; 
     this.idnumber = idnumber;
  }
  Method m;
  int idnumber;
}

}

