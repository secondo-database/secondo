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
#include <android/log.h>
#include <unistd.h>
#include <string>

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
   try {
   nlclass = env->FindClass("sj/lang/ListExpr");
   assert(nlclass!=0);
   }catch(...) {

   }
   //env->ExceptionClear();
   jfieldID fid;

   fid = env->GetStaticFieldID(nlclass,"SYMBOL_ATOM","B");
   assert(fid!=0);
   jsymbol_atom = env->GetStaticByteField(nlclass,fid);

   

   fid = env->GetStaticFieldID(nlclass,"STRING_ATOM","B");
   assert(fid!=0);
   jstring_atom = env->GetStaticByteField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"TEXT_ATOM","B");
   assert(fid!=0);
   jtext_atom = env->GetStaticByteField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"REAL_ATOM","B");
   assert(fid!=0);
   jreal_atom = env->GetStaticByteField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"INT_ATOM","B");
   assert(fid!=0);
   jint_atom = env->GetStaticByteField(nlclass,fid);  

   fid = env->GetStaticFieldID(nlclass,"BOOL_ATOM","B");
   assert(fid!=0);
   jbool_atom = env->GetStaticByteField(nlclass,fid);


   fid = env->GetStaticFieldID(nlclass,"NO_ATOM","B");
   assert(fid!=0);
   jno_atom = env->GetStaticByteField(nlclass,fid); 

   realAtomID = env->GetStaticMethodID(nlclass,"realAtom",
                                       "(D)Lsj/lang/ListExpr;");
   assert(realAtomID);

   intAtomID = env->GetStaticMethodID(nlclass,"intAtom",
                                      "(I)Lsj/lang/ListExpr;");
   assert(intAtomID);
  
   symbolAtomID = env->GetStaticMethodID(nlclass,"symbolAtom",
                             "([B)Lsj/lang/ListExpr;");
   assert(symbolAtomID);

   stringAtomID =env->GetStaticMethodID(nlclass,"stringAtom",
                            "([B)Lsj/lang/ListExpr;");
   assert(stringAtomID);

   textAtomID = env->GetStaticMethodID(nlclass,"textAtom",
                             "([B)Lsj/lang/ListExpr;");
   assert(textAtomID);

   boolAtomID = env->GetStaticMethodID(nlclass,"boolAtom",
                             "(Z)Lsj/lang/ListExpr;");
   assert(boolAtomID);

   theEmptyListID = env->GetStaticMethodID(nlclass,"theEmptyList",
                           "()Lsj/lang/ListExpr;");
   assert(theEmptyListID);

   oneElemListID = env->GetStaticMethodID(nlclass,"oneElemList",
                              "(Lsj/lang/ListExpr;)Lsj/lang/ListExpr;");
   assert(oneElemListID);

   appendID = env->GetStaticMethodID(nlclass,"append",
                "(Lsj/lang/ListExpr;Lsj/lang/ListExpr;)Lsj/lang/ListExpr;");
   assert(appendID);
   
   atomTypeID = env->GetMethodID(nlclass,"atomType","()I");
   assert(atomTypeID);

   symbolValueID = env->GetMethodID(nlclass,"symbolValue",
                                            "()Ljava/lang/String;");
   assert(symbolValueID);

   stringValueID = env->GetMethodID(nlclass,"stringValue",
                                            "()Ljava/lang/String;");
   assert(stringValueID);

   textValueID =  env->GetMethodID(nlclass,"textValue","()Ljava/lang/String;");
   assert(textValueID);
  
   realValueID = env->GetMethodID(nlclass,"realValue","()D");
   assert(realValueID);

   boolValueID = env->GetMethodID(nlclass,"boolValue","()Z");
   assert(boolValueID);

   intValueID = env->GetMethodID(nlclass,"intValue","()I");
   assert(intValueID);

   isEmptyID = env->GetMethodID(nlclass,"isEmpty","()Z");
   assert(isEmptyID);
 
   firstID = env->GetMethodID(nlclass,"first","()Lsj/lang/ListExpr;");
   assert(firstID);

   restID = env->GetMethodID(nlclass,"rest","()Lsj/lang/ListExpr;");
   assert(restID);

   systemclass = env->FindClass("java/lang/System");
   assert(systemclass);
   gcID = env->GetStaticMethodID(systemclass,"gc","()V");
   assert(gcID);

}




/*
1.1 The ~GetJavaList~ Function

This function converts a C++-ListExpr value into a Java-ListExpr value.
The result is computed via an JNI call. For this reason, the
caller has to ensure, to tell the java environment , that the result
can be destroyed via the DeleteLocalRef-Command. 

*/
jobject JNITool::GetJavaList(JNIEnv * env, ListExpr LE){
   jobject res;
   // first process the atoms

   // real 

   if(nl->AtomType(LE)== RealType){
      double rvalue = nl->RealValue(LE);

      res = env->CallStaticObjectMethod(nlclass,realAtomID,rvalue);
      if(!res) Error(__LINE__);
      return res; 
   }

   // integer
   if(nl->AtomType(LE)==IntType){
      int ivalue = nl->IntValue(LE);


      res = env->CallStaticObjectMethod(nlclass,intAtomID,ivalue);
      if(!res) Error(__LINE__);
      return res;
   }
   // boolean
   if(nl->AtomType(LE)==BoolType){
      int bvalue = nl->BoolValue(LE);


      res = env->CallStaticObjectMethod(nlclass,boolAtomID,bvalue);
      if(!res) Error(__LINE__);
      return res;
   }

   // symbol
   if(nl->AtomType(LE)==SymbolType){
	   string svalue = nl->SymbolValue(LE);
	   const char* cstr = svalue.c_str();
   	  //__android_log_write(ANDROID_LOG_INFO, "FU", cstr);
   	  const int cstrlen = strlen(cstr);
	   //jstring jstr = env->NewStringUTF(cstr);
	   jbyteArray arr = env->NewByteArray(cstrlen);
	   env->SetByteArrayRegion(arr, 0, cstrlen,(jbyte*) cstr);
	   char tempstring[100];
	   //sprintf(tempstring, "Laenge des Strings = %d", cstrlen);
	   //__android_log_write(ANDROID_LOG_INFO, "FU", tempstring);
//	   sprintf(tempstring, "Bytes = %d %d %d", arr[0], arr[1], arr[2]);
//	   __android_log_write(ANDROID_LOG_INFO, "FU", tempstring);

	   //if(!jstr) Error(__LINE__);
	   if (!arr) Error(__LINE__);
	   res = env->CallStaticObjectMethod(nlclass,symbolAtomID,arr);
	   //res = env->CallStaticObjectMethod(nlclass,symbolAtomID,jstr);
	   //env->DeleteLocalRef(jstr);
	   env->DeleteLocalRef(arr);
	   if(!res) Error(__LINE__);

	   return res;
   }

   // string
   if(nl->AtomType(LE)==StringType){
	   string svalue = nl->StringValue(LE);
	   const char* cstr = svalue.c_str();
	   //jstring jstr = env->NewStringUTF(cstr);
	   jbyteArray arr = env->NewByteArray((int)strlen(cstr));
	   env->SetByteArrayRegion(arr, 0, (int)strlen(cstr),(jbyte*) cstr);

	   //if(!jstr) Error(__LINE__);
	   if (!arr) Error(__LINE__);
	   res = env->CallStaticObjectMethod(nlclass,stringAtomID,arr);
	   //res = env->CallStaticObjectMethod(nlclass,stringAtomID,jstr);
	   //env->DeleteLocalRef(jstr);
	   env->DeleteLocalRef(arr);
	   if(!res) Error(__LINE__);

	   return res;
   }

   // text
   if(nl->AtomType(LE)== TextType){
      string tvalue;
      nl->Text2String(LE,tvalue);
      const char* cstr = tvalue.c_str();
      //jstring jstr = env->NewStringUTF(cstr);
	   jbyteArray arr = env->NewByteArray((int)strlen(cstr));
	   env->SetByteArrayRegion(arr, 0, (int)strlen(cstr),(jbyte*) cstr);

      //if(!jstr) Error(__LINE__);
	   if (!arr) Error(__LINE__);
	   res = env->CallStaticObjectMethod(nlclass,stringAtomID,arr);
//      res = env->CallStaticObjectMethod(nlclass,textAtomID,jstr);
//      env->DeleteLocalRef(jstr);
	   env->DeleteLocalRef(arr);
      if(!res) Error(__LINE__);
      return res;
   }

   // empty list
   if(nl->IsEmpty(LE)){
	   res = env->CallStaticObjectMethod(nlclass,theEmptyListID);
       if(!res) Error(__LINE__);
       return res;
   }

   // non-empty list
   if(nl->AtomType(LE)==NoAtom){

      // the list has at least one element because the empty list is
      // processed before
      ListExpr F = nl->First(LE);
      ListExpr R = nl->Rest(LE);
      jobject elem1 = GetJavaList(env,F);
      // put the first element into a java list
      res = env->CallStaticObjectMethod(nlclass,oneElemListID,elem1);
      env->DeleteLocalRef(elem1);
 
      if(!res) Error(__LINE__);
      jobject last =  res;
	  char memText[100];

      while(!nl->IsEmpty(R)){
//    	  sprintf(memText, "Memory: sizeof(res) %d",sizeof(*res) );
//    	  __android_log_write(ANDROID_LOG_INFO, "FU", memText);

           jobject next = GetJavaList(env, nl->First(R));
           jobject oldlast = last;
           if(next!=NULL) {
        	   last = env->CallStaticObjectMethod(
			nlclass,appendID,last,next);
        	   env->DeleteLocalRef(next);

        	   if(oldlast!=res)
        		   env->DeleteLocalRef(oldlast);
        	   if(!last) Error(__LINE__);

           }
           R = nl->Rest(R);
      }
      if(last!=res)
         env->DeleteLocalRef(last);

      return res;
   } 
   // unknow atomtype

//   Error(__LINE__);

   return 0;

}


/*

1.2 The ~GetCppList~ function

This function converts a java ListExpr gives as the argument obj 
into its c++ counterpart.

*/

ListExpr JNITool::GetCppList(JNIEnv* env, jobject obj){

  int type =  env->CallIntMethod(obj,atomTypeID);
  if(type == jsymbol_atom){
      jobject jstr = env->CallObjectMethod(obj,symbolValueID);
      if(!jstr) Error(__LINE__);
      const char* cstr;
      cstr = env->GetStringUTFChars((jstring) jstr,NULL);
      if(!cstr) Error(__LINE__);
      ListExpr res =  nl->SymbolAtom(cstr);
      env->ReleaseStringUTFChars((jstring)jstr,cstr);
      env->DeleteLocalRef(jstr);
      return res;
  }

  if(type == jstring_atom){
      jstring jstr = (jstring) env->CallObjectMethod(obj,stringValueID);
      if(!jstr) Error(__LINE__);
      const char* cstr;
      cstr = env->GetStringUTFChars(jstr,NULL);
      if(!cstr) Error(__LINE__);
      ListExpr res = nl->StringAtom(cstr);
      env->ReleaseStringUTFChars((jstring)jstr,cstr);
      env->DeleteLocalRef(jstr);
      return res;
  }

  if(type == jtext_atom){
      jstring jstr = (jstring) env->CallObjectMethod(obj,textValueID);
      if(!jstr) Error(__LINE__);
      const char* cstr;
      cstr = env->GetStringUTFChars(jstr,NULL);
      if(!cstr) Error(__LINE__);
      ListExpr res = nl->TextAtom();
      nl->AppendText(res,cstr);
      env->ReleaseStringUTFChars(jstr,cstr);
      env->DeleteLocalRef(jstr);
      return res;
  }

  if(type == jreal_atom){
     return nl->RealAtom(env->CallDoubleMethod(obj,realValueID));
  }

  if(type == jint_atom){
     return nl->IntAtom(env->CallIntMethod(obj,intValueID));
  }
  if(type == jbool_atom){
     return nl->BoolAtom(env->CallBooleanMethod(obj,boolValueID));
  }

  if(type ==jno_atom){
     // check for emptyness
     if(env->CallBooleanMethod(obj,isEmptyID))
        return nl->TheEmptyList();
     // we have at least one element
     // process the first element
     jobject elem1 = env->CallObjectMethod(obj,firstID);
     if(!elem1) Error(__LINE__);
     ListExpr CppElem1 = GetCppList(env,elem1);
     env->DeleteLocalRef(elem1);
     ListExpr Last = CppElem1;
     ListExpr result = nl->OneElemList(CppElem1);
     Last = result;
     jobject o = obj;
     o = env->CallObjectMethod(obj,restID);
     if(!o) Error(__LINE__);
     while(!(env->CallBooleanMethod(o,isEmptyID))){
         elem1 = env->CallObjectMethod(o,firstID); 
         if(!elem1) Error(__LINE__);
         CppElem1 = GetCppList(env,elem1);  // convert to cpp format
         env->DeleteLocalRef(elem1); 
         Last = nl->Append(Last,CppElem1); // append to result 
         jobject oldo = o;
         o = env->CallObjectMethod(oldo,restID);
         env->DeleteLocalRef(oldo);
         if(!o) Error(__LINE__);
     } 
     if(o!=obj)
        env->DeleteLocalRef(o);
     return result;
  }

  Error(__LINE__); // unknow list type
  return 0; 

}

/*
1.4 PrintJavaList

Simple function for printing out a nested list java-instance.

*/
void JNITool::PrintJavaList(jobject list){
    jmethodID mid = env->GetMethodID(nlclass,"writeListExpr","()V");
    if(!mid) Error(__LINE__);
    env->CallVoidMethod(list,mid);
}



/*
1.5 Error-Function

This function prints out the last exception occurred in the Java VM and
exits the program via an assert call.

*/
void JNITool::Error(int line){
   __android_log_print(ANDROID_LOG_ERROR,
	"FU","Error occured in file %s at line %d\n",__FILE__,line);
   if(env->ExceptionOccurred())
     env->ExceptionDescribe();
   assert(0);

}

