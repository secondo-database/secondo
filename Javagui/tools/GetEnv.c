
#include <stdlib.h>
#include "GetEnv.h"
#include <string.h>


JNIEXPORT jstring JNICALL Java_tools_GetEnv_getEnvFromC(JNIEnv *env , jclass class, jstring string){
  const char *str = (*env)->GetStringUTFChars(env,string,0);
  char* val = getenv( str );
  (*env)->ReleaseStringUTFChars(env,string,str);
  jstring res = (*env)->NewStringUTF(env,(val));
  return res;
}
