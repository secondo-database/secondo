/* This module initializes the JNI Environment. */

#ifndef JVMINIT_H
#define JVMINIT_H

#include <jni.h>

class JVMInitializer {
 public:
  JVMInitializer();
  JNIEnv *getEnv();
  JavaVM *getJVM();
  bool isInitialized();
 private:
  static bool initialized;
  static JNIEnv* env;
  static JavaVM* jvm;

 };

#endif
