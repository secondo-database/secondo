/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

1 Overview

This class provides some functions useful in jni calls.

*/

#include "NestedList.h"
#include "jni.h"
#include <JVMInit.h>
#include "JNITool.h"


bool JNITool::initialized=false;



/*

1.0 The Constructor

This constructor sets the JavaEnvironment and gets some
constants from the Java implementation of nested lists.

*/

JNITool::JNITool(JNIEnv *env, NestedList *nl){
   this->env = env;
   this->nl = nl;

   // store some data to avoid frequently recomputation
   nlclass = env->FindClass("sj/lang/ListExpr");
   assert(nlclass!=0);
   jfieldID fid;
   fid = env->GetStaticFieldID(nlclass,"SYMBOL_ATOM","I");
   assert(fid!=0);
   jsymbol_atom = env->GetStaticIntField(nlclass,fid);  
   
   fid = env->GetStaticFieldID(nlclass,"STRING_ATOM","I");
   assert(fid!=0);
   jstring_atom = env->GetStaticIntField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"TEXT_ATOM","I");
   assert(fid!=0);
   jtext_atom = env->GetStaticIntField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"REAL_ATOM","I");
   assert(fid!=0);
   jreal_atom = env->GetStaticIntField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"INT_ATOM","I");
   assert(fid!=0);
   jint_atom = env->GetStaticIntField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"NO_ATOM","I");
   assert(fid!=0);
   jno_atom = env->GetStaticIntField(nlclass,fid);  
}




/*
1.1 The ~GetJavaList~ Function

This function converts a C++-ListExpr value into a Java-ListExpr value. 

*/
jobject JNITool::GetJavaList(JNIEnv * env, ListExpr LE){
   jmethodID mid;
   jobject res;
   // first process the atoms

   // real 
   if(nl->AtomType(LE)== RealType){
      double rvalue = nl->RealValue(LE);
      mid = env->GetStaticMethodID(nlclass,"realAtom","(D)Lsj/lang/ListExpr;");
      assert(mid!=0);
      res = env->CallStaticObjectMethod(nlclass,mid,rvalue);
      assert(res!=0);
      return res; 
   }

   // integer
   if(nl->AtomType(LE)==IntType){
      int ivalue = nl->IntValue(LE);
      mid = env->GetStaticMethodID(nlclass,"intAtom","(I)Lsj/lang/ListExpr;");
      assert(mid!=0);
      res = env->CallStaticObjectMethod(nlclass,mid,ivalue);
      assert(res!=0);
      return res;
   }

   // symbol
   if(nl->AtomType(LE)==SymbolType){
     string svalue = nl->SymbolValue(LE);
     const char* cstr = svalue.c_str();
     jstring jstr = env->NewStringUTF(cstr);
     assert(jstr!=0);
     mid =  env->GetStaticMethodID(nlclass,"symbolAtom",
                        "(Ljava/lang/String;)Lsj/lang/ListExpr;");
     assert(mid!=0);
     res = env->CallStaticObjectMethod(nlclass,mid,jstr);
     assert(res!=0);
     return res;
   }

   // string
   if(nl->AtomType(LE)==StringType){
     string svalue = nl->StringValue(LE);
     const char* cstr = svalue.c_str();
     jstring jstr = env->NewStringUTF(cstr);
     assert(jstr!=0);
     mid =  env->GetStaticMethodID(nlclass,"stringAtom",
                          "(Ljava/lang/String;)Lsj/lang/ListExpr;");
     assert(mid!=0);
     res = env->CallStaticObjectMethod(nlclass,mid,jstr);
     assert(res!=0);
     return res;
   }

   // text
   if(nl->AtomType(LE)== TextType){
      string tvalue;
      nl->Text2String(LE,tvalue);
      const char* cstr = tvalue.c_str();
      jstring jstr = env->NewStringUTF(cstr);
      assert(jstr!=0);
      mid =  env->GetStaticMethodID(nlclass,"textAtom",
                           "(Ljava/lang/String;)Lsj/lang/ListExpr;");
      assert(mid!=0);
      res = env->CallStaticObjectMethod(nlclass,mid,jstr);
      assert(res!=0);
      return res;
   }

   // empty list
   if(nl->IsEmpty(LE)){
       mid = env->GetStaticMethodID(nlclass,"theEmptyList",
                           "()Lsj/lang/ListExpr;");
       assert(mid!=0);
       res = env->CallStaticObjectMethod(nlclass,mid);
       assert(res!=0);
       return res;
   }

   // non-empty list
   if(nl->AtomType(LE)==NoAtom){
      // the list has at least one element because the empty list is
      // processed before
      ListExpr F = nl->First(LE);
      ListExpr R = nl->Rest(LE);
      jobject elem1 = GetJavaList(env,F);
      jobject last = elem1;
      // put the first element into a java list
      mid = env->GetStaticMethodID(nlclass,"oneElemList",
                          "(Lsj/lang/ListExpr;)Lsj/lang/ListExpr;");
      assert(mid!=0);
      res = env->CallStaticObjectMethod(nlclass,mid,elem1);
      assert(res!=0);
      last = res;
      mid = env->GetStaticMethodID(nlclass,"append",
                  "(Lsj/lang/ListExpr;Lsj/lang/ListExpr;)Lsj/lang/ListExpr;");
      assert(mid!=0);
      while(!nl->IsEmpty(R)){
           jobject next = GetJavaList(env, nl->First(R));
           last = env->CallStaticObjectMethod(nlclass,mid,last,next);
           assert(last!=0); 
           R = nl->Rest(R);
      } 
      return res;
   } 

   // unknow atomtype
   assert(false);

}



/*

1.2 The ~GetCppList~ function

This function converts a java ListExpr gives as the argument obj 
into its c++ counterpart.

*/

ListExpr JNITool::GetCppList(JNIEnv* env, jobject obj){
  jmethodID mid = env->GetMethodID(nlclass,"atomType","()I");
  assert(mid!=0);
  int type = env->CallIntMethod(obj,mid);

  if(type == jsymbol_atom){
      mid = env->GetMethodID(nlclass,"symbolValue","()Ljava/lang/String;");
      assert(mid!=0);
      jobject jstr = env->CallObjectMethod(obj,mid);
      assert(jstr!=0);
      const char* cstr;
      cstr = env->GetStringUTFChars((jstring) jstr,NULL);
      assert(cstr!=0);
      return nl->SymbolAtom(cstr);
  }

  if(type == jstring_atom){
      mid = env->GetMethodID(nlclass,"stringValue","()Ljava/lang/String;");
      assert(mid!=0);
      jstring jstr = (jstring) env->CallObjectMethod(obj,mid);
      assert(jstr!=0);
      const char* cstr;
      cstr = env->GetStringUTFChars(jstr,NULL);
      assert(cstr!=0);
      return nl->StringAtom(cstr);
  }

  if(type == jtext_atom){
      mid = env->GetMethodID(nlclass,"textValue","()Ljava/lang/String;");
      assert(mid!=0);
      jstring jstr = (jstring) env->CallObjectMethod(obj,mid);
      assert(jstr!=0);
      const char* cstr;
      cstr = env->GetStringUTFChars(jstr,NULL);
      assert(cstr!=0);
      ListExpr res = nl->TextAtom();
      nl->AppendText(res,cstr);
      return res;
  }

  if(type == jreal_atom){
     mid = env->GetMethodID(nlclass,"realValue","()D");
     assert(mid!=0);
     return nl->RealAtom(env->CallDoubleMethod(obj,mid));
  }

  if(type == jint_atom){
     mid = env->GetMethodID(nlclass,"intValue","()I");
     assert(mid!=0);
     return nl->IntAtom(env->CallIntMethod(obj,mid));
  }
  if(type ==jno_atom){
     // check for emptyness
     jmethodID mid_isEmpty = env->GetMethodID(nlclass,"isEmpty","()Z");
     assert(mid_isEmpty!=0);
     if(env->CallBooleanMethod(obj,mid_isEmpty))
        return nl->TheEmptyList();
     // we have at least one element
     jmethodID mid_first;
     mid_first = env->GetMethodID(nlclass,"first","()Lsj/lang/ListExpr;");
     assert(mid_first!=0);
     jmethodID mid_rest;
     mid_rest = env->GetMethodID(nlclass,"rest","()Lsj/lang/ListExpr;"); 
     assert(mid_rest!=0);
     jmethodID mid_append = env->GetStaticMethodID(nlclass,"append",
                  "(Lsj/lang/ListExpr;Lsj/lang/ListExpr;)Lsj/lang/ListExpr;");
     assert(mid_append!=0);
     // process the first element
     jobject elem1 = env->CallObjectMethod(obj,mid_first);
     assert(elem1!=0);
     ListExpr CppElem1 = GetCppList(env,elem1);
     ListExpr Last = CppElem1;
     ListExpr result = nl->OneElemList(CppElem1);
     Last = result;
     obj = env->CallObjectMethod(obj,mid_rest);
     assert(obj!=0);
     while(!(env->CallBooleanMethod(obj,mid_isEmpty))){
         elem1 = env->CallObjectMethod(obj,mid_first); 
         assert(elem1!=0);
         CppElem1 = GetCppList(env,elem1);  // convert to cpp format
         Last = nl->Append(Last,CppElem1); // append to result 
         obj = env->CallObjectMethod(obj,mid_rest);
         assert(obj!=0);
     } 
     return result;
  }

  assert(false); // unknow list type

}
