
#include <jni.h>


#include <SecondoPL.h>


/*
 * Class:     OptimizerServer 
 * Method:    registerSecondo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_OptimizerServer_registerSecondo(
                  JNIEnv * env , 
                  jclass cl,
                  jstring user,
                  jstring pswd){
   // convert the jni string to native cpp strings
   const jbyte *cuser;
   cuser = (*env)->GetStringUTFChars(env,user , NULL);
   if (cuser == NULL) {
      return -1; 
   }
   const jbyte *cpswd;
   cpswd = (*env)->GetStringUTFChars(env,pswd , NULL);
   if (cpswd == NULL) {
      return -1; 
   }
   int res = registerSecondo(cuser,cpswd);
   // release native c strings
   (*env)->ReleaseStringUTFChars(env, user, cuser); 
   (*env)->ReleaseStringUTFChars(env, pswd, cpswd); 
   return res;
   
}

