/*

This class provides functions for converting nested
lists from the Cpp instances to Java instances and vice versa.

*/

#include "NestedList.h"
#include "jni.h"
#include <JVMInit.h>

class JNITool{
   public:
      JNITool(JNIEnv *env,NestedList* nl);
      jobject GetJavaList(JNIEnv *env,ListExpr LE);
      ListExpr GetCppList(JNIEnv* env, jobject obj);
   private:
      JNIEnv *env;
      NestedList *nl;
      jclass nlclass;
      static bool initialized;
      int jsymbol_atom;
      int jstring_atom;
      int jtext_atom;
      int jreal_atom;
      int jint_atom;
      int jno_atom;
}; 


