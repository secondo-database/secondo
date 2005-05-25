/*

This class provides functions for converting nested
lists from the Cpp instances to Java instances and vice versa.

*/
#ifndef JNITOOL_H
#define JNITOOL_H

#include "NestedList.h"
#include "jni.h"
#include <JVMInit.h>

class JNITool{
   public:
      JNITool(JNIEnv *env,NestedList* nl);
      jobject GetJavaList(JNIEnv *env,ListExpr LE);
      ListExpr GetCppList(JNIEnv* env, jobject obj);
      void PrintJavaList(jobject list);
      void Gc();
   private:
      JNIEnv *env;
      NestedList *nl;
      jclass nlclass;
      jclass systemclass;
      static bool initialized;
      int jsymbol_atom;
      int jstring_atom;
      int jtext_atom;
      int jreal_atom;
      int jint_atom;
      int jno_atom;
      jmethodID realAtomID;
      jmethodID intAtomID;
      jmethodID symbolAtomID;
      jmethodID stringAtomID;
      jmethodID textAtomID;
      jmethodID theEmptyListID;
      jmethodID oneElemListID;
      jmethodID appendID;
      jmethodID atomTypeID;
      jmethodID symbolValueID;
      jmethodID stringValueID;
      jmethodID textValueID;
      jmethodID realValueID;
      jmethodID intValueID;
      jmethodID isEmptyID;
      jmethodID firstID;
      jmethodID restID;
      jmethodID gcID;
      void Error(int line);
}; 

#endif
