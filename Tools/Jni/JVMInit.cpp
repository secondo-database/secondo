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
#include <iostream>
#include <fstream>
#include <string>

bool JVMInitializer::initialized=false;
JNIEnv* JVMInitializer::env=0;
JavaVM* JVMInitializer::jvm=0;


/* 1.0 The getIniFile function
   This function returns the content of system variable JNI_INIT if available.
   If this variable is not defined, a warning is printted out and a default
   value is returned.
*/
string getIniFile(){
  char* res = getenv("JNI_INIT");
  if(res!=0)
      return res;
  else{
     cerr << "JNI_INIT not defined using default: ./JNI.ini" << endl;
     return "JNI.ini";
  }
}

/* 1.0 The getAbsolutePath function
   If the given string begins with "/", the
   string  self is returned. Otherwise
   $SECONDO_BUILD_DIR/String is returned.
*/
string getAbsolutePath(string Path){
  if(Path[0]=='/' | Path[0]=='.')
     return Path;
  if(PATH_SEPARATOR==';' && Path.size()>1 && Path[1]==':'){ // an absolutte windows path
         return Path;
  }      
  string SecondoHome = getenv("SECONDO_BUILD_DIR");
  return SecondoHome+"/"+Path;
}

/* 1.0 The processLine function
  In this function a single line of the ini file is
  analyzed.
*/
void processLine(const string& inputLine,string& classpath, string& libdir, string& version){
 // remove comment
 int comment = inputLine.find("#");
 string line;
 if(comment != string::npos){
    line = inputLine.substr(0,comment);
 }else
    line = string(inputLine);

 int len = line.size();
 // find the first not space character
 int i=0;
 while(line[i]==' ' && ++i < len);
 if(i>=len){
   return;
 }
 if(line[0]!='%') // ignore lines without this switch
   return;

 string line2 = line.substr(2,line.size());
 // find the first not space character
 i=0;
 len = line2.size();
 while(line2[i]==' ' && ++i < len);
 int i2 = len;
 while(line2[i2]==' ' && --i2 > 0);
 string line3 = line2.substr(i,i2);

 if(line[1]=='P'){
   if(classpath.size()>0)
      classpath += PATH_SEPARATOR;
   classpath += getAbsolutePath(line3);
 }
 if(line[1]=='L')
    libdir = line3;
 if(line[1]=='V')
    version = line3;
}


/* 1.1 The readFile function
   In this function the file is opened and readed. If the file can't
   opened or an error occurs in reading this file, readFile will return
   -1 otherwise 0.
*/
int readFile(const string& FileName,string& classpath, string& libpath, string& version){
  // open the file
  ifstream infile(FileName.c_str());
  if(!infile){ // error in opening file
    return -1;
  }
  string line;
  while(!infile.eof()){
     getline(infile,line);
     processLine(line,classpath,libpath,version);
  }
  infile.close();
  return 0;
}

/* 1.1 The error function
   This function reports an error message and exists the program.
*/

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
  if(initialized) return; // initialize only one time

  jint res;
  cout << "Initializing the JNI. " << endl;

  initialized =false;
  int major=0; // version part
  int minor=0; // version part
  int number=0;

  string classpath="";
  string libpath="";
  string versionstr="";
  string FileName = getIniFile();
  if(readFile(FileName,classpath,libpath,versionstr)!=0)
     error(1);

  if(versionstr.size()==0){
    cout << "no version specified using default 1.4" << endl;
    major = 1;
    minor = 4;
  }else{
    int len = versionstr.size();
    int i=0;
    int digit;
    while(i<len && versionstr[i]!='.'){
       digit = toNumber(versionstr[i]);
       if(digit>=0)
          major = major*10 + digit;
       i++;
    }
    i++; // ignore the dot
    while(i<len){
       digit = toNumber(versionstr[i]);
       if(digit >=0)
          minor = minor*10 + digit;
       i++;
    }
  }

  string classoption ="-Djava.class.path="+classpath;
  string liboption = "-Djava.library.path="+libpath;


  cout<<"set classpath to" << classpath<<endl;
  cout<<"set libdir to " << libpath<<endl;
  cout<<"set version to " << major<<"."<<minor<<endl;

  int version = 65536*major+minor;

  char co[classoption.size()+1];
  sprintf(co,"%s",classoption.c_str());
  char lo[liboption.size()+1];
  sprintf(lo,"%s",liboption.c_str());

  #ifdef JNI_VERSION_1_2
     JavaVMInitArgs vm_args;
     JavaVMOption options [2];
     options[0].optionString =co;
     options[1].optionString =lo;
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
     string cp2 = vm_args.classpath+PATH_SEPARATOR+classpath;
     vm_args.classpath =cp2;
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






