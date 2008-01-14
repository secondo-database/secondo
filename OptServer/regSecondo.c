
#include <jni.h>


#include <SecondoPL.h>


/*
 * Class:     OptimizerServer 
 * Method:    registerSecondo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_OptimizerServer_registerSecondo(
                  JNIEnv * env , 
                  jclass cl){
   int res = registerSecondo();
   return res;
   
}

