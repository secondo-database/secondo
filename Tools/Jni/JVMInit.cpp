/* This module initializes the JNI Environment. 
   March, 25th, 2003 by Mirco Guenster and Ismail Zerrad.
   changed 2003-06-17 by Th. Behr
      new ananyse of the jni.ini-file
      make compatible with new java-version 1.4
 */

using namespace std;

#ifdef _WIN32
#define PATH_SEPARATOR ';'
#else /* UNIX */
#define PATH_SEPARATOR ':'
#endif

#include<JVMInit.h>
#include<iostream>

const int PATHLENGTH = 1024;

bool JVMInitializer::initialized=false;
JNIEnv* JVMInitializer::env=0;
JavaVM* JVMInitializer::jvm=0;


static void error(int reason) {
  cerr << "Error during initialization of the JNI Interface: ";
  if (reason == 1) cerr << "Couldn't read the configuration file JNI.ini";
  if (reason == 2) cerr << "Couldn't create the JVM.";
  if (reason == 3) cerr << "wrong java version in JNI.ini";
  cerr << endl;
  exit(1);
}

static int toNumber(char c){
  if(c=='0') return 0;
  if(c=='1') return 1;
  if(c=='2') return 2;
  if(c=='3') return 3;
  if(c=='4') return 4;
  if(c=='5') return 5;
  if(c=='6') return 6;
  if(c=='7') return 7;
  if(c=='8') return 8;
  if(c=='9') return 9;
  return -1;
}


JVMInitializer::JVMInitializer() {
  if(initialized) return;

  jint res;

  

  
  char libdir[PATHLENGTH]; // path to lib
  char userclasspath[PATHLENGTH]; 
  char classoption[PATHLENGTH];  
  char liboption[1024];
  char outclasspath[PATHLENGTH]; // the output formatted classpath



  // for information output only.
  int clpos = 0;
  int outpos = 0;
  int libpos =0;
  FILE *file;
  int c = 0;
  int i = 0;
  cout << "Initializing the JNI. " << endl;
  
  initialized =false; 
  int major=0; // version part
  int minor=0; // version part
  int number=0;

  // read the configuration file which contains 
  // the jar files which should provided. 
  file = fopen("JNI.ini", "r");

  if (file == 0) {
    error(1);
  }
  else {
    for (i = 0; i < PATHLENGTH; i++) {
      userclasspath[i] = '\0';
      outclasspath[i] = '\0';
      classoption[i] = '\0';
      liboption[i] = '\0';
      libdir[i] = '\0';
    }

    // contine this while loop until no data can be read from the
    // configuration file.
    c=fgetc(file);
    while (c!=EOF) {
      //ignore space at begin of line
      while((c==' ' || c=='\t' )&& c!=EOF && c!='\n')
          c=fgetc(file);

      // a meanful line (classpath version or libpath) 
      if(c=='%'){
        c=fgetc(file);
        if(c=='L'){ // found the libpath
           c=fgetc(file);
           while(c!=EOF && c!= '\n'){
              libdir[libpos++]=c;
              c=fgetc(file);
           }
        }
        if(c=='V'){
           c=fgetc(file);
           // read the major number
           while(c!=EOF && c!='\n' && c!='.'){
              if(c!=' ' && c!='\t'){ // ignore spaces
                number=toNumber(c);
                if(number<0) error(3);
                major = major*10+number;
              }
              c=fgetc(file);
           }
           // read the minor number
           if(c=='.'){
              c=fgetc(file);
              while(c!=EOF && c!='\n'){
                if(c!=' ' && c!='\t'){
                   number=toNumber(c);
                   if(number<0) error(3);
                   minor = minor*10+number;
                }
                c=fgetc(file);
              }
           }
        } else if(c=='P'){ // found a classpath
          if (clpos>0){ // add a Path separator
             userclasspath[clpos++]=PATH_SEPARATOR;
             outclasspath[outpos++]='\n';
          }
          outclasspath[outpos++]='\t';
          c=fgetc(file);
          //ignore space at begin of line
          while(c==' ' && c!=EOF && c!='\n')
             c=fgetc(file);
          while(c!=EOF && c!='\n' && c!='#'){
             userclasspath[clpos++]=c; 
             outclasspath[outpos++]=c;
             c=fgetc(file);
          }

          if(c=='#'){ // comment after line
             while(c!=EOF && c!='\n')
                c=fgetc(file);
          }

          // remove space at end of classpath
          while(clpos>0 && userclasspath[clpos]==' '){
              userclasspath[clpos]='\0';
              clpos--;
          }

        }
        else // ignore this line
           while(c!=EOF && c!= '\n')
             c=fgetc(file);
      }
      else{ // ignore this line
        while(c!=EOF && c!='\n')
           c=fgetc(file);
      }
      if(c=='\n') c=fgetc(file);
    }
    fclose(file);
    cout << "Searching for JAR archives and class files in:" << endl;
    cout << outclasspath << endl;
  }

  

  sprintf(classoption,"%s%s","-Djava.class.path=",userclasspath);
  sprintf(liboption,"%s%s","-Djava.library.path=",libdir);

  cout<<"set libdir to"<<libdir<<endl;
  cout<<"set version to"<<major<<"."<<minor<<endl;
  
  int version = 65536*major+minor;


  #ifdef JNI_VERSION_1_2 
     JavaVMInitArgs vm_args;
     JavaVMOption options [2];
     options[0].optionString =classoption;
     options[1].optionString =liboption;
     // vm_args.version =version;
     vm_args.version = version;
     vm_args.options =options;
     vm_args.nOptions =2;
     vm_args.ignoreUnrecognized =JNI_TRUE;
     //Create the Java VM 
     res =JNI_CreateJavaVM(&jvm,(void**)&env,&vm_args);
  #else // JNI_Version 1.1
     JDK1_1InitArgs vm_args;
     vm_args.version = version;
     
     JNI_GetDefaultJavaVMInitArgs(&vm_args);
     //Append USER_CLASSPATH to the default system class path 
     char classpath[1024];
     sprintf(classpath,"%s%c%s",
         vm_args.classpath,PATH_SEPARATOR,userclasspath);
     vm_args.classpath =userclasspath;
     //Create the Java VM 
     res =JNI_CreateJavaVM(&jvm,(void**)&env,&vm_args);
  #endif //JNI_VERSION_1_2 


  if (res < 0) {
    error(2);
  }
  

  initialized = true;
  cout << "JNI initialization finished." << endl;
}

JNIEnv* JVMInitializer::getEnv() {
  return env;
}

JavaVM* JVMInitializer::getJVM() {
  return jvm;
}

bool JVMInitializer::isInitialized(){
  return initialized;
}






