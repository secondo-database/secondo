/*

//paragraph [1] Title: [{\large\bf ] [}]

[1] JVMInitializer

\begin{center}
   Last Change: May 2004
\end{center}

Algebras written in Java use JNI to call the
Java methods from C/C++. To realize that it is
needed to provide a Java Virtual Maschine
(JVM for short). All Code has to use the same
JVM. It is not possible to use several instances
of the JVM in a single program. Each code which
want to use JNI has to create a JVMInitializer.
The first call of the constructor creates the Java
environment. The file JNI.ini is readed for
initialization of the JVM. After this, the constructor
will make nothing when it called. 


*/

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
