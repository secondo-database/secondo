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

//paragraph [1] Title: [{\large\bf] [}]

[1] JVMInit

This module initializes the JNI Environment. \\
\begin{center}
   March, 25th, 2003 by Mirco Guenster and Ismail Zerrad. \\
   changed 2003-06-17 by Th. Behr \\
      new analyse of the jni.ini-file \\
      make compatible with new java-version 1.4 \\
   changed 2005-04-04 by M. Spiekermann \\
      new parameter M in jni.ini introduced wich \\
      defined the maximum heap size of the VM in megabytes \\
   changed 2005-07-21 by M. Spiekermann \\
      Parameter L will be prefixed by variable J2SDK\_ROOT \\
\end{center}

*/

#include <vector>
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


/*

1 The ~getIniFile~ function

This function returns the content of system variable JNI\_INIT if available.
If this variable is not defined, a warning is printed out and a default
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

/*
2 The ~getAbsolutePath~ function

If the given string begins with "/", the
string  itself is returned. Otherwise
\$SECONDO\_BUILD\_DIR/String is returned.

*/
string getAbsolutePath(string Path){
  if(Path[0]=='/' | Path[0]=='.')
    return Path;
  if(PATH_SEPARATOR==';' && Path.size()>1 && Path[1]==':'){
    // an absolute windows path
    return Path;
  }
  string SecondoHome = getenv("SECONDO_BUILD_DIR");
  return SecondoHome+"/"+Path;
}

/*

3 The ~trim~ function

This function removes spaces at begin and end of the given string

*/
string trim(const string& s){
  if(s.length() == 0)
       return s;
  int start = s.find_first_not_of(" \t");
  int end = s.find_last_not_of(" \t");
  if(start == string::npos) // No non-spaces
     return "";
  return string(s, start, end - start + 1);
}


/*
4 The ~processLine~ function

In this function a single line of the ini file is analyzed.

*/
void processLine(const string& inputLine,string& classpath,
                       string& libdir, string& version, string& heapMem,vector<string>& xoptions){
 // remove comment
 int comment = inputLine.find("#");
 string line;
 if(comment != string::npos){
    line = inputLine.substr(0,comment);
 }else
    line = string(inputLine);

 int len = line.size();
 // find the first not space character
 int i=line.find_first_not_of(" \t");
 if(i==string::npos)
    return;
 if(line[i]!='%') // ignore lines without this switch
   return;

 string line2 = line.substr(i,line.size());
 string line3 = trim(line2.substr(2,line2.size()));

 if(line2[1]=='P'){
   if(classpath.size()>0)
      classpath += PATH_SEPARATOR;
   classpath += getAbsolutePath(line3);
 }
 if(line[1]=='L') {
    string J2SDKRoot = getenv("J2SDK_ROOT");

    if ( ((line3[0] != '.') && (line3[0] != '/')) && (J2SDKRoot != "") ) 
    {
      libdir = J2SDKRoot + "/" + line3;
    } 
    else 
    {
      libdir = line3;
    } 
 }
 if(line[1]=='V')
    version = line3;
 if(line[1]=='M')
    heapMem = line3;
 if(line[1]=='A'){
    while(line3!=""){
        int pos = line3.find(' ');
        if(pos<0){
           xoptions.push_back(line3);
           line3="";
        }else{
           xoptions.push_back(line3.substr(0,pos));
           line3 = trim(line3.substr(pos+1,line3.length()));
        }
	}
 }
}


/*

5 The ~readFile~ function

In this function the file is opened and readed. If the file can't
opened or an error occurs in reading this file, readFile will return
-1 otherwise 0.

*/
int readFile(const string& FileName,string& classpath,
                   string& libpath, string& version, string& heapMem,vector<string>&  xoptions){
  // open the file
  ifstream infile(FileName.c_str());
  if(!infile){ // error in opening file
    return -1;
  }
  string line;
  while(!infile.eof()){
     getline(infile,line);
     processLine(line,classpath,libpath,version,heapMem,xoptions);
  }
  infile.close();
  return 0;
}

/*
6 The ~error~ function

This function reports an error message and exits the program.

*/
static void error(int reason) {
  cerr << "Error during initialization of the JNI Interface: ";
  if (reason == 1) cerr << "Couldn't read the configuration file JNI.ini";
  if (reason == 2) cerr << "Couldn't create the JVM.";
  if (reason == 3) cerr << "wrong java version in JNI.ini";
  cerr << endl;
  exit(1);
}

/*
7 The ~toNumber~ function

This function returns the interger represented by the
argument. if c does not represent a valid digit, -1 is returned.

*/
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


/*
8 The ~Constructor~

If the constructor is called the first time, the file jni.ini
is analyzed and the Java environment is created. All other
calls leads to no action.

*/
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
  string heapMemStr="";
  vector<string> xoptions;  

  string FileName = getIniFile();
  if(readFile(FileName,classpath,libpath,versionstr,heapMemStr,xoptions)!=0)
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

// The following section is taken from the description of
// the JNI Enhancement, Introduced in version 1.2 of the 
// JavaTM 2 SDK.
// Copyright (1995-1999) by Sun Microsystems Inc.
//
// Site: http://java.sun.com/j2se/1.4.2/docs/guide/jni/jni-12.html#invo

  string classoption ="-Djava.class.path="+classpath;
  string liboption = "-Djava.library.path="+libpath;
  string mem_mx_option = "-Xmx" + heapMemStr + "M"; 

  cout<<"set version to " << major<<"."<<minor<<endl;

  int version = 65536*major+minor;

  char co[classoption.size()+1];
  sprintf(co,"%s",classoption.c_str());
  char lo[liboption.size()+1];
  sprintf(lo,"%s",liboption.c_str());
  char mo[mem_mx_option.size()+1];
  sprintf(mo,"%s",mem_mx_option.c_str());
  const unsigned int VM_MAXOPT = 3+xoptions.size();
  #ifdef JNI_VERSION_1_2
     JavaVMInitArgs vm_args;

     JavaVMOption options [VM_MAXOPT];
     options[0].optionString = co;
     options[1].optionString = lo;
     options[2].optionString = mo;
     for(int i=0;i<xoptions.size();i++){
        char* xopt = new char[xoptions[i].size()+1];
        sprintf(xopt,"%s",xoptions[i].c_str());
        options[3+i].optionString = xopt;
     }

     cout << "VM Options: " << endl;
     for (int i=0; i<VM_MAXOPT; i++) 
     {
       cout << "  " << options[i].optionString << endl;
     }
     // vm_args.version =version;
     vm_args.version = version;
     vm_args.options = options;
     vm_args.nOptions = VM_MAXOPT;
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

/*
9 ~getEnv~

Returns the Java environment.

*/
JNIEnv* JVMInitializer::getEnv() {
  return env;
}

/*
~getJVM~

Returns the java virtual machine.

*/
JavaVM* JVMInitializer::getJVM() {
  return jvm;
}

/*
~isInitialized~

Returns the state of the JavaInitializer.

*/
bool JVMInitializer::isInitialized(){
  return initialized;
}


