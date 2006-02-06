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

//paragraph [1] title: [{\Large \bf ]	[}]


[1] JBBox Algebra

2004-04-22 Thomas Behr

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

\abstract{
This algebra is an example to demonstrate how algebras written in
Java can be used in {\textsc{Secondo}}. This algebra supports also
the persistent storing mechanism.
}

\parindent0mm

\tableofcontents


0 General Notes

0.1 Requirements for the Java-Classes

To store a Java-instance into a database, it has to provide
several methods. First the Java-class must implement the
interface ~Serializable~ which don't need any methods.
Furthermore the class should provide the following methods:
\begin{description}
   \item[compareTo] compares this instance with another one with
                    the familiar return values
   \item[copy] This methods must create a proper copy (not a flat copy)
               of this object.
   \item[readFrom(byte[])] This is a static methods reading the object from
              the argument. Examples can be found in the Java-classes of this
	      algebra (see appendix).
   \item[writeToByteArray] The is the counterpart of the previous method.
   \item[getHashValue] Returns a HashValue for the instance
\end{description}


0.2 Calling Java-Functions from C++

Java-functions are called in the JNI standard way described here shortly.
All Java-algebras run in the same environment which is provided by the
SECONDO system.  Each JNI-algebra holds a  pointer to this JNIEnv
(see section \ref{JNI_Instances}). This variable is defined in the
~Initialize~ function of the algebra (see section \ref{initialization}).

The call of a Java method follows the schema:
\begin{enumerate}
   \item Get the methodID of the method to call using the member cls
   \item Check for success.
   \item Call the method.
   \item Depending on the result value the following steps are to do
         \begin{itemize}
           \item if the result type is primitive (bool, int, double ...) \\
	         just return the result
	   \item if the result type is a non-array object\\
	         create a C++ instance from the result and return it
           \item if the result is an array (also an object) \\
	         create a C++ array of the corresponding C++ class and
		 fill it with the converted objects.
	 \end{itemize}
\end{enumerate}


0.3 How To Store a Java-Object into a Database

A Java-Object can't be stored directly because the use of pointers.
Hence the corresponding C++ class contains this object twice.
The first representation is the Java-Object itself. It is used to
make computations on it. The second one is the byte representation
stored in a FLOB. While storing the object, only the byte representation
is written into the database. After reading out this object from the database
the Java-Object is undefined. We recreate it from its byte representation in
the Initialize() function.


1 Preliminaries

1.1 Includes

~Note~

  Additionally to the familiar includes we need to include
  further headers for using JNI.


*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StandardAttribute.h"
#include "FLOB.h"
#include "Attribute.h"
#include <jni.h>
#include <JVMInit.h>

namespace jbbox {


static NestedList* nl;
static QueryProcessor* qp;

/*
1.2 Needed Instances

\label{JNI_Instances}
The following lines are needed for using JNI. Here a few variables are
declared which contain the Java environment. All variables are defined
in the Initialize function of this algebra.

*/
static JVMInitializer *jvminit=0;
static JNIEnv *env;
static JavaVM *jvm;

/*

1.2 Error Function.

Prints a short message and the line number of an occurred error
referring to problems with JNI to stderr.
If this function is invoked please check your local installation of
the Java Development Kit (JDK).

*/

static void error(int line) {
  cerr << "Error in JBBoxAlgebra.cpp in line: " << line;
  cerr << "." << endl;
  if(env->ExceptionOccurred())
     env->ExceptionDescribe();
  exit(1);
}

/*

2 The Classes

All classes contain the managed Java object as well as the
class of this objects. All operations are realized by invokation of
the corresponding Java methods using JNI. Because we want to use the
classes in the persistent version of the relation algebra we have to
implement several functions to support this. Because the Java object
can't be handled by the relation algebra directly, we have to store the
object data twice. Once as Java object and once as its byte representation
in a FLOB. The counterpart will be computet at the right places.

2.1 The Declaration of the Class JPoint

*/
class JPoint: public StandardAttribute{
   public:
      JPoint();
      JPoint(const jobject jobj);
      JPoint(const int size);
      ~JPoint();
      void Destroy();
      JPoint* Clone() const;
      bool IsDefined() const;
      void SetDefined(bool b);
      size_t HashValue() const;
      void CopyFrom(const StandardAttribute* right);
      int Compare(const Attribute *arg) const;
      bool Adjacent(const Attribute *arg) const;
      int NumOfFLOBs() const;
      FLOB *GetFLOB(const int i);
      // will be invoked if the Java object must
      // be reconstructed from a FLOB
      void Initialize();
      jobject GetObject() const;
      // the function for the algebra operator
      bool Equals(JPoint* P);
      // restores the Java Object from its
      // byte representation
      void RestoreJavaObjectFromFLOB();
  private:
      jclass cls; // pointer to the Java class Point
      jobject obj; // pointer to the managed instance
      // the byte representation of the Java object
      FLOB objectData;
      bool canDelete;
      bool defined;
      // write the Java object in to a FLOB
      void RestoreFLOBFromJavaObject();
};

/*

2.1 The Declaration of the Class JBox

*/
class JBox: public StandardAttribute{
   public:
      JBox();
      JBox(const jobject jobj);
      JBox(const int size);
      ~JBox();
      void Destroy();
      JBox* Clone() const;
      void Write() const;
      bool IsDefined() const;
      void SetDefined(bool b);
      size_t HashValue() const;
      void CopyFrom(const StandardAttribute* right);
      int Compare(const Attribute *arg) const;
      bool Adjacent(const Attribute *arg) const;
      int NumOfFLOBs() const;
      FLOB *GetFLOB(const int i);
      // will be invoked if the Java object must be
      // reconstructed from a FLOB
      void Initialize();
      jobject GetObject() const;
      // the functions for the algebra operators
      bool Equals(const JBox* P) const;
      bool Contains(const JPoint* P) const;
      JBox* Union(const JBox* BB) const;
      JBox* Union(const JPoint* P) const;
      JBox* Intersection(const JBox* BB) const;
      bool  Intersects(const JBox* BB) const;
      float Size() const;
      bool IsEmpty() const;
      // restores the Java Object from its byte representation
      void RestoreJavaObjectFromFLOB();
  private:
      jclass cls; // pointer to the Java class Point
      jobject obj; // pointer to the managed instance
      // the byte representation of the Java object
      FLOB objectData;
      bool canDelete;
      bool defined;
      // write the Java object in to a FLOB
      void RestoreFLOBFromJavaObject();
};

/*

2.2 Definition of the Class Functions


2.2.1 Definitions of the Functions of the Class JPoint


 ~Standard constructor~

 This constructor should never be used directly. In this
 constructor can't be any code. It's strong recommend that
 this constructor makes nothing because it's used in
 a non standard way for persistent storing.

*/
  JPoint::JPoint(){}

/*

 ~Another constructor~

 This constructor should only used in the Open Method
 because it is used in a non-standard way. It have to
 create the FLOB and to initialize the canDelete member.
 No Java object should be constructed here.

*/

  JPoint::JPoint(const int size):
             objectData(size),
             canDelete(false),
	     defined(true)
	     {}

/*

 ~RestoreFLOBFromJavaObject~

 This function writes the Java object in  the FLOB using the
 serialization mechanism of Java.

*/
void JPoint::RestoreFLOBFromJavaObject(){
  // get the needed methodid
  jmethodID mid;
  mid = env->GetMethodID(cls,"writeToByteArray","()[B");
  // no success
  if(mid == 0){
     error(__LINE__);
  }
  // call the method
  jbyteArray jbytes;
  jbytes = (jbyteArray) env->CallObjectMethod(obj,mid);
  // no success
  if(jbytes == 0){
       error(__LINE__);
  }
  // store the Java result (an array of bytes) in to the FLOB
  int size = env->GetArrayLength(jbytes);
  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,0);
}

/*

 ~RestoreJavaObjectFromFLOB~

 Creates the Java object from a byte sequence stored in the
 FLOB using the serialization mechanism of Java.

*/
void JPoint::RestoreJavaObjectFromFLOB(){
   // get the Java class Point
   cls = env->FindClass("bbox/Point");
   // error: class not found
   if (cls == 0) {
    error(__LINE__);
   }
   // error FLOB is not initialized
   if(&objectData == 0){
      error(__LINE__);
   }

   int size = objectData.Size();
   const char *bytes;
   objectData.Get(0,&bytes);
   // copy the data into a Java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);
  jmethodID mid;
  mid = env->GetStaticMethodID(cls,"readFrom",
                               "([B)Lbbox/Point;");
  if(mid == 0){
     error(__LINE__);
  }
  jobject jres = env->CallStaticObjectMethod(cls,mid,jbytes);
  if(jres == 0){
     error(__LINE__);
  }
  obj = jres;
  jbyte* elems = env->GetByteArrayElements(jbytes,0);
  env->ReleaseByteArrayElements(jbytes,elems,0);
 }

/*

 ~Constructor~

 This constructor takes a Java object and creates the
 corresponding C++ object from it. This includes the
 creation of the FLOB.

*/

JPoint::JPoint(const jobject jobj):objectData(1){
  /* Find the class Point. */
  canDelete = false;
  cls = env->FindClass("bbox/Point");
  if (cls == 0) { // class not found
    error(__LINE__);
  }
  obj = jobj;
  // create the corresponding FLOB
  RestoreFLOBFromJavaObject();
  defined=true;
 }

/*

  ~Destructor~

  Destructor of a JPoint object. This destructor deletes the
  contained Java object if possible.

*/
JPoint::~JPoint(){
  if(canDelete){
     env->DeleteLocalRef(obj);
     Destroy();
  }
}

/*

 ~Destroy~

 The familiar Destroy method known from other (non-JNI) algebras.

*/
void JPoint::Destroy(){
   canDelete=true;
}

/*

 ~HashValue~

 This function returns the HashValue of a JPoint using the
 corresponding Java method.

*/
size_t JPoint::HashValue() const{
  jmethodID mid = env->GetMethodID(cls,"getHashValue","()I");
  if(mid == 0){
     error(__LINE__);
  }
  return (size_t) env->CallIntMethod(obj,mid);
}

/*

~CopyFrom~

This function creates a copy of this JPoint. To realize it,
the FLOB is copied and the Javaobject is reconstructed from it.

*/
void JPoint::CopyFrom(const StandardAttribute* right){
   const JPoint *P = (const JPoint *)right;
   cls = env->FindClass("bbox/Point");
   objectData.Resize(P->objectData.Size());
   const char *data;
   P->objectData.Get(0,&data);
   objectData.Put(0,P->objectData.Size(),data);
   RestoreJavaObjectFromFLOB();
}

/*

~Compare~

This function compares two JPoints using JNI

*/
int JPoint::Compare(const Attribute * arg) const{
  jmethodID mid;
  mid = env->GetMethodID(cls,"compareTo","(Lbbox/Point;)I");
  if(mid == 0){
      error(__LINE__);
  }
  const JPoint *P = (const JPoint *) arg;
  return env->CallIntMethod(obj,mid,P->obj);
}

/*

~Adjacent~

Because two points of the $I\hspace{-0.3em}R^2$ can't be
adjacent we return just false.

*/
 bool JPoint::Adjacent(const Attribute* arg) const {
    return false;
 }

/*

~NumOfFLOBs~

Each JNI algebra manages one Java objects which is additionally
stored in a single FLOB. For this reason we return 1.

*/
 int JPoint::NumOfFLOBs() const{
    return 1;
 }

/*

~GetFLOB~

First is checked whether the given index is correct. After that the
contained FLOB is returned.

*/
 FLOB* JPoint::GetFLOB(const int i){
     assert(i==0);
     return &objectData;
 }

/*

~Initialize~

This function is invoked when the object is readed from disk.
This means, the contained FLOB is created but the pointer structure
for the Java object must be created from this FLOB.

*/
void JPoint::Initialize(){
  RestoreJavaObjectFromFLOB();
}

/*

~GetObject~

This function just returns the managed Java object.

*/
 jobject JPoint::GetObject() const {
    return obj;
 }

/*

~IsDefined~

This function checks if this JPoint is in a defined state.

*/
bool JPoint::IsDefined() const{
  return defined;
}

/*

~SetDefined~

sets the defined state of this JPoint.

*/
void JPoint::SetDefined(bool b) {
  defined=b;
}


/*

~Clone~

The Clone function returns a proper copy of the JPoint.
Here the corresponding Java function is called. Alternatively
we can clone the FLOB and reconstruct the Java object from it.

*/
JPoint* JPoint::Clone() const {
  jmethodID mid;
  jobject jobj;
  mid=env->GetMethodID(cls,"copy","()Lbbox/Point;");
  if(mid==0) error(__LINE__);
  jobj = env->CallObjectMethod(obj,mid);
  if(jobj==0) error(__LINE__);
  return new JPoint(jobj);
}

/*

~Equals~

This is the only operator function for JPoint.

*/
bool JPoint::Equals(JPoint* P){
  jmethodID mid;
  mid = env->GetMethodID(cls,"equals","(Ljava/lang/Object;)Z");
  if(mid==0) error(__LINE__);
  bool eq = env->CallBooleanMethod(obj,mid,P->obj);
  if(env->ExceptionOccurred()) error(__LINE__);
  return eq;
}



/*

2.2.2 Definitions of the functions of the class JBox

~Standard Constructor~

This constructor should never be used directly. In this
constructor can't be any code. It's strong recommend that
this constructor makes nothing because it's used in
a non standard way for persistent storing.

*/
  JBox::JBox(){}

/*

 ~Another constructor~

This constructor should only used in the Open function
because it is used in a non-standard way. It have to
create the FLOB and to initialize the canDelete member.
No Java object should be constructed here.

*/
  JBox::JBox(const int size):
        objectData(size),
	canDelete(false),
	defined(true)
	{}

/*

 ~RestoreFLOBFromJavaObject~

 This function writes the Java object in  the FLOB using the
 serialization mechanism of Java.

*/
void JBox::RestoreFLOBFromJavaObject(){
  // get the needed methodid
  jmethodID mid;
  mid = env->GetMethodID(cls,"writeToByteArray","()[B");
  // no success
  if(mid == 0){
     error(__LINE__);
  }
  // call the method
  jbyteArray jbytes;
  jbytes = (jbyteArray) env->CallObjectMethod(obj,mid);
  // no success
  if(jbytes == 0){
       error(__LINE__);
  }
  // store the Java result (an array of bytes) in to the FLOB
  int size = env->GetArrayLength(jbytes);
  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,0);
}

/*

 ~RestoreJavaObjectFromFLOB~

Creates the Java object from a byte sequence stored in the
FLOB using the serialization mechanism of Java.

*/
void JBox::RestoreJavaObjectFromFLOB(){
   // get the Java class Point
   cls = env->FindClass("bbox/BBox");
   // error: class not found
   if (cls == 0) {
    error(__LINE__);
   }
   // error FLOB is not initialized
   if(&objectData == 0){
      error(__LINE__);
   }

   int size = objectData.Size();
   const char *bytes;
   objectData.Get(0,&bytes);
   // copy the data into a Java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);
  jmethodID mid;
  mid = env->GetStaticMethodID(cls,"readFrom",
                               "([B)Lbbox/BBox;");
  if(mid == 0){
     error(__LINE__);
  }
  jobject jres = env->CallStaticObjectMethod(cls,mid,jbytes);
  if(jres == 0){
     error(__LINE__);
  }
  obj = jres;
  jbyte* elems = env->GetByteArrayElements(jbytes,0);
  env->ReleaseByteArrayElements(jbytes,elems,0);
 }

/*

~Constructor~

This constructor takes a Java object and creates the
corresponding C++ object from it. This includes the
creation of the FLOB.

*/
JBox::JBox(const jobject jobj):objectData(1){
  /* Find the class Point. */
  canDelete = false;
  cls = env->FindClass("bbox/BBox");
  if (cls == 0) { // class not found
    error(__LINE__);
  }
  obj = jobj;
  // create the corresponding FLOB
  RestoreFLOBFromJavaObject();
  defined=true;
 }

/*

 ~Destructor~

Destructor of a JBox object. This destructor deletes the
contained Java object if possible.

*/
JBox::~JBox(){
  if(canDelete){
     env->DeleteLocalRef(obj);
     Destroy();
  }
}

/*

~Destroy~

The familiar Destroy method known from other (non-JNI) algebras

*/
void JBox::Destroy(){
   canDelete=true;
}

/*

~HashValue~

This function returns the HashValue of a JBox using the
corresponding Java method.

*/
size_t JBox::HashValue() const{
  jmethodID mid = env->GetMethodID(cls,"getHashValue","()I");
  if(mid == 0){
     error(__LINE__);
  }
  return (size_t) env->CallIntMethod(obj,mid);
}

/*

~CopyFrom~

This function creates a copy of this JBox. To realize it,
the FLOB is copied and the Javaobject is reconstructed from it.

*/
void JBox::CopyFrom(const StandardAttribute* right){
   const JBox *P = (const JBox *)right;
   cls = env->FindClass("bbox/BBox");
   objectData.Resize(P->objectData.Size());
   const char *data;
   P->objectData.Get(0,&data);
   objectData.Put(0,P->objectData.Size(),data);
   RestoreJavaObjectFromFLOB();
}

/*

~Compare~

This function compares two JPoints using JNI

*/
int JBox::Compare(const Attribute * arg) const{
  jmethodID mid;
  mid = env->GetMethodID(cls,"compareTo","(Lbbox/BBox;)I");
  if(mid == 0){
      error(__LINE__);
  }
  const JBox *P = (const JBox *) arg;
  return env->CallIntMethod(obj,mid,P->obj);
}

/*

~Adjacent~

Returns just false.

*/
 bool JBox::Adjacent(const Attribute* arg) const{
    return false;
 }

/*

~NumOfFLOBs~

Each JNI algebra manages one Java objects which is additionally
stored in a single FLOB. For this reason we return 1.

*/
 int JBox::NumOfFLOBs() const{
    return 1;
 }

/*

~GetFLOB~

First is checked whether the given index is correct. After that the
contained FLOB is returned.

*/
 FLOB* JBox::GetFLOB(const int i){
     assert(i==0);
     return &objectData;
 }

/*

~Initialize~

This function is invoked when the object is readed from disk.
This means, the contained FLOB is created and the pointer structure
for the Java object must be created from this FLOB.

*/
void JBox::Initialize(){
  RestoreJavaObjectFromFLOB();
}

/*

~GetObject~

This function just returns the managed Java object.

*/
 jobject JBox::GetObject() const{
    return obj;
 }

/*

~IsDefined~

This function checks if this JPoint is in a defined state.

*/
bool JBox::IsDefined() const{
   return defined;
}

/*

  ~SetDefined~

  sets the defined state of this JBox

*/
void JBox::SetDefined(bool b){
   defined=b;
}


/*

~Clone~

The Clone function returns a proper copy of the JPoint.
Here the corresponding Java function is called. Alternatively
we can clone the FLOB and reconstruct the Java object from it.

*/
JBox* JBox::Clone() const{
  jmethodID mid;
  jobject jobj;
  mid=env->GetMethodID(cls,"copy","()Lbbox/BBox;");
  if(mid==0) error(__LINE__);
  jobj = env->CallObjectMethod(obj,mid);
  if(jobj==0) error(__LINE__);
  return new JBox(jobj);
}

/*

~Write~

The ~Write~ function is for debugging purposes only.

*/
void JBox::Write() const{
   jmethodID mid = env->GetMethodID(cls,"write","()V");
   if(mid==0)
     error(__LINE__);
   env->CallVoidMethod(obj,mid);
}


/*

~Equals~

This is the a operator function for JBox.

*/
bool JBox::Equals(const JBox* P) const {
  jmethodID mid;
  mid = env->GetMethodID(cls,"equals","(Ljava/lang/Object;)Z");
  if(mid==0) error(__LINE__);
  bool eq = env->CallBooleanMethod(obj,mid,P->obj);
  if(env->ExceptionOccurred()) error(__LINE__);
  return eq;
}

/*
~Contains~

This function checks whether P is contained in this jbox.

*/

bool JBox::Contains(const JPoint* P) const{
  jmethodID mid;
  mid = env->GetMethodID(cls,"contains","(Lbbox/Point;)Z");
  if(mid==0) error(__LINE__);
  bool co = env->CallBooleanMethod(obj,mid,P->GetObject());
  if(env->ExceptionOccurred()) error(__LINE__);
  return co;
}

/*
~Union~

This function extends this JBox to contain the given Point.

*/

JBox* JBox::Union(const JPoint* P) const{
  jmethodID mid;
  mid = env->GetMethodID(cls,"union",
                         "(Lbbox/Point;)Lbbox/BBox;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,P->GetObject());
  if(jobj==0) error(__LINE__);
  return new JBox(jobj);
}

/*
~Union~

This function creates a new JBox which contains all points in this
JBox and in B.  

*/

JBox* JBox::Union(const JBox* B) const{
  jmethodID mid;
  mid = env->GetMethodID(cls,"union",
                         "(Lbbox/BBox;)Lbbox/BBox;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,B->obj);
  if(jobj==0) error(__LINE__);
  return new JBox(jobj);
}

/*
~Intersection~

This function creates a new JBox which only contains such points 
which are contained in both in this JBox and in B.

*/

JBox* JBox::Intersection(const JBox* B) const{
  jmethodID mid;
  mid = env->GetMethodID(cls,"intersection",
                         "(Lbbox/BBox;)Lbbox/BBox;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,B->obj);
  if(jobj==0) error(__LINE__);
  return new JBox(jobj);
}

/*
~Intersects~

This function checks wether this JBx and B have a common point.

*/

bool JBox::Intersects(const JBox* B) const{
  jmethodID mid;
  mid = env->GetMethodID(cls,"intersects","(Lbbox/BBox;)Z");
  if(mid==0) error(__LINE__);
  bool res =  env->CallObjectMethod(obj,mid,B->obj);
  if(env->ExceptionOccurred())
     error(__LINE__);
  return res;
}

/*
~Size~

This function  computes the size of the area covered by this JBox.

*/

float JBox::Size() const{
  jmethodID mid = env->GetMethodID(cls,"size","()D");
  if(mid==0) error(__LINE__);
  float res = (float) env->CallDoubleMethod(obj,mid);
  if(env->ExceptionOccurred()) error(__LINE__);
  return res;
}

/*
~IsEmpty~

The ~IsEmpty~ function checks whether this JBox is empty.

*/

bool JBox::IsEmpty() const{
  jmethodID mid = env->GetMethodID(cls,"isEmpty","()Z");
  if(mid==0) error(__LINE__);
  bool res = env->CallBooleanMethod(obj,mid);
  if(env->ExceptionOccurred()) error(__LINE__);
  return res;
}


/*

3 The Definition of the Type Constructors

3.1 Needed Functions

 ~Out~ Function for a JPoint

*/
static ListExpr OutJPoint(ListExpr typeInfo, Word value){
   JPoint* P = (JPoint*) (value.addr);
   jclass cls = env->FindClass("bbox/Point");
   jmethodID mid = env->GetMethodID(cls,"getX","()D");
   if(mid==0) error(__LINE__);
   float x;
   x = (float) env->CallDoubleMethod(P->GetObject(),mid);
   mid = env->GetMethodID(cls,"getY","()D");
   if(mid==0) error(__LINE__);
   float y;
   y = (float) env->CallDoubleMethod(P->GetObject(),mid);
   if(env->ExceptionOccurred())
      error(__LINE__);
   return nl->TwoElemList( nl->RealAtom(x), nl->RealAtom(y));
}

/*
 ~In~ Function for a JPoint

*/
static Word InJPoint(const ListExpr typeInfo,
		     const ListExpr instance,
		     const int errorPos,
		     ListExpr& errorInfo,
		     bool& correct ) {

  if(nl->ListLength(instance)!=2){ // error
     correct=false;
     return SetWord(Address(0));
  }
  ListExpr F = nl->First(instance);
  ListExpr S = nl->Second(instance);
  if(nl->AtomType(F)!=RealType){
     correct=false;
     return SetWord(Address(0));
  }
  if(nl->AtomType(S)!=RealType){
     correct=false;
     return SetWord(Address(0));
  }
  jclass cls = env->FindClass("bbox/Point");
  if(cls==0)error(__LINE__);
  jmethodID mid = env->GetMethodID(cls,"<init>","(DD)V");
  if(mid==0) error(__LINE__);
  jobject obj;
  obj = env->NewObject(cls,mid,nl->RealValue(F),
                       nl->RealValue(S));
  if(obj==0) error(__LINE__);
  correct=true;
  JPoint* P = new JPoint(obj);
  return SetWord(P);
}

/*

~Out~ Function for a JBox

*/
static ListExpr OutJBox(ListExpr typeInfo, Word value){
  JBox* B = (JBox*) (value.addr);
  if(B->IsEmpty()){
     return nl->SymbolAtom("empty");
  }
  jclass cls = env->FindClass("bbox/BBox");
  jfieldID fid = env->GetFieldID(cls,"minX","D");
  float minX;
  minX = (float) env->GetDoubleField(B->GetObject(),fid);
  fid = env->GetFieldID(cls,"maxX","D");
  float maxX;
  maxX = (float) env->GetDoubleField(B->GetObject(),fid);
  fid = env->GetFieldID(cls,"minY","D");
  float minY;
  minY = (float) env->GetDoubleField(B->GetObject(),fid);
  fid = env->GetFieldID(cls,"maxY","D");
  float maxY;
  maxY = (float) env->GetDoubleField(B->GetObject(),fid);
  ListExpr res;
  res = nl->FourElemList(nl->RealAtom(minX), nl->RealAtom(maxX),
                         nl->RealAtom(minY), nl->RealAtom(maxY));
  return res;
}

/*

~In~ Function for a JBox

*/
static Word InJBox(const ListExpr typeInfo,
		   const ListExpr instance,
		   const int errorPos,
		   ListExpr& errorInfo,
		   bool& correct ) {
  jmethodID mid;
  jobject obj;
  jclass cls = env->FindClass("bbox/BBox");
  if(cls==0) error(__LINE__);
  if(nl->IsEqual(instance,"empty")){
     mid = env->GetMethodID(cls,"<init>","()V");
     if(mid==0) error(__LINE__);
     obj = env->NewObject(cls,mid);
     if(obj==0) error(__LINE__);
     correct=true;
     return SetWord(new JBox(obj));
  }
  if(nl->ListLength(instance)!=4){
    correct=false;
    return SetWord(Address(0));
  }
  ListExpr e1 = nl->First(instance);
  ListExpr e2 = nl->Second(instance);
  ListExpr e3 = nl->Third(instance);
  ListExpr e4 = nl->Fourth(instance);
  if(nl->AtomType(e1)!=RealType  ||
     nl->AtomType(e2)!=RealType  ||
     nl->AtomType(e3)!=RealType  ||
     nl->AtomType(e4)!=RealType){
     correct=false;
     return SetWord(Address(0));
  }
  mid = env->GetMethodID(cls,"<init>","(DDDD)V");
  if(mid==0) error(__LINE__);
  obj = env->NewObject(cls,mid,nl->RealValue(e1),
                       nl->RealValue(e2), nl->RealValue(e3),
		       nl->RealValue(e4));

  if(obj==0) error(__LINE__);
  correct=true;
  return SetWord(new JBox(obj));
}

/*

~Create~ Functions

*/
static Word CreateJPoint(const ListExpr typeInfo){
  jclass cls = env->FindClass("bbox/Point");
  if(cls==0) error(__LINE__);
  jmethodID mid = env->GetMethodID(cls,"<init>","(DD)V");
  if(mid==0) error(__LINE__);
  jobject obj = env->NewObject(cls,mid,0.0,0.0);
  if(obj==0) error(__LINE__);
  return SetWord(new JPoint(obj));
}

static Word CreateJBox(const ListExpr typeInfo){
  jclass cls = env->FindClass("bbox/BBox");
  if(cls==0) error(__LINE__);
  jmethodID mid = env->GetMethodID(cls,"<init>","(DDDD)V");
  if(mid==0) error(__LINE__);
  jobject obj = env->NewObject(cls,mid,0.0,0.0,0.0,0.0);
  if(obj==0) error(__LINE__);
  return SetWord(new JBox(obj));
}

/*

~Delete~ Functions

*/

static void DeleteJPoint(const ListExpr typeInfo, Word &w) {
  delete (JPoint *)w.addr;
  w.addr = 0;
}

static void DeleteJBox(const ListExpr typeInfo, Word &w) {
  delete (JBox *)w.addr;
  w.addr = 0;
}

/*

~Close~ Functions

*/
static void CloseJPoint(const ListExpr typeInfo, Word &w) {
  delete (JPoint *)w.addr;
  w.addr = 0;
}

static void CloseJBox(const ListExpr typeInfo, Word &w) {
  delete (JBox *)w.addr;
  w.addr = 0;
}

/*

~Clone~ Functions

*/
static Word CloneJPoint(const ListExpr typeInfo, const Word &w) {
  return SetWord(((JPoint *)w.addr)->Clone());
}

static Word CloneJBox(const ListExpr typeInfo, const Word &w) {
  return SetWord(((JBox *)w.addr)->Clone());
}

/*

 ~Open~ Functions

*/
bool OpenJPoint(SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
		Word& value){
   JPoint* P = (JPoint*)Attribute::Open(valueRecord,offset, typeInfo);
   P->RestoreJavaObjectFromFLOB();
   value = SetWord(P);
   return true;
}

bool OpenJBox(SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
		Word& value){
   JBox* B = (JBox*) Attribute::Open(valueRecord,offset, typeInfo);
   B->RestoreJavaObjectFromFLOB();
   value = SetWord(B);
   return true;
}

/*

~Save~ functions

*/
bool SaveJPoint( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
		 Word& value)
{ JPoint* P = (JPoint*) value.addr;
  Attribute::Save(valueRecord,offset,typeInfo,P);
  return true;
}

bool SaveJBox( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
		 Word& value)
{ JBox* B = (JBox*) value.addr;
  Attribute::Save(valueRecord,offset,typeInfo,B);
  return true;
}


/*

~SizeOf~ Functions

*/
int SizeOfJPoint(){ return sizeof(JPoint);}
int SizeOfJBox(){ return sizeof(JBox);}


/*

~Cast~ Functions

*/
static void* CastJPoint( void* addr ) {
  return new (addr) JPoint;
}

static void* CastJBox( void* addr ) {
  return new (addr) JBox;
}

/*

3.2 The Signatures of the Type Constructors

*/
static ListExpr JPointProperty(){
   return (nl->TwoElemList(
              nl->FourElemList(
	        nl->StringAtom("Signature"),
	        nl->StringAtom("Example Type List"),
	        nl->StringAtom("List Representation"),
	        nl->StringAtom("Example List")),
	      nl->FourElemList(
	        nl->StringAtom("->DATA"),
	        nl->StringAtom("jpoint"),
		nl->StringAtom("(real real)"),
		nl->StringAtom("(1.0 2.7)"))));
}


static ListExpr JBoxProperty(){
   return (nl->TwoElemList(
              nl->FourElemList(
	        nl->StringAtom("Signature"),
	        nl->StringAtom("Example Type List"),
	        nl->StringAtom("List Representation"),
	        nl->StringAtom("Example List")),
	      nl->FourElemList(
	        nl->StringAtom("->DATA"),
	        nl->StringAtom("jbox"),
		nl->StringAtom("(minX maxX minY maxY)"),
		nl->StringAtom("(1.0 2.7 37.9 90.8)"))));
}

/*

3.3 Kind checking Functions

*/

static bool CheckJPoint( ListExpr type, ListExpr& errorInfo ) {
	return (nl->IsEqual(type, "jpoint"));
}
static bool CheckJBox( ListExpr type, ListExpr& errorInfo ) {
	return (nl->IsEqual(type, "jbox"));
}

/*

3.4 Creation of the Type Constructor Instance

*/
TypeConstructor jpoint
(
 "jpoint",         // name
 JPointProperty,   // property function describing signature
 OutJPoint,        // Out function
 InJPoint,         // In function
 0,        0,      // SaveToList and RestoreFromList functions
 CreateJPoint,     // object creation
 DeleteJPoint,     // object deletion
 0, //OpenJPoint,       // object open
 0, //SaveJPoint,       // object save
 CloseJPoint,      // object close
 CloneJPoint,      // object clone
 CastJPoint,       // cast function
 SizeOfJPoint,     // Size of a point
 CheckJPoint       // kind checking function
);


TypeConstructor jbox
(
 "jbox",         // name
 JBoxProperty,   // property function describing signature
 OutJBox,        // Out function
 InJBox,         // In function
 0,        0,      // SaveToList and RestoreFromList functions
 CreateJBox,     // object creation
 DeleteJBox,     // object deletion
 0, //OpenJBox,       // object open
 0, //SaveJBox,       // object save
 CloseJBox,      // object close
 CloneJBox,      // object clone
 CastJBox,       // cast function
 SizeOfJBox,     // Size of a point
 CheckJBox       // kind checking function
);


/*

4 Operators

4.1 Type Mapping Functions

*/

static ListExpr OiOiBool(ListExpr arg){
   if(nl->ListLength(arg)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(arg),"jpoint") &&
      nl->IsEqual(nl->Second(arg),"jpoint"))
         return nl->SymbolAtom("bool");
   if(nl->IsEqual(nl->First(arg),"jbox") &&
      nl->IsEqual(nl->Second(arg),"jbox"))
         return nl->SymbolAtom("bool");
   return nl->SymbolAtom("typeerror");
}

static ListExpr BoxPointBool(ListExpr arg){
   if(nl->ListLength(arg)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(arg),"jbox") &&
      nl->IsEqual(nl->Second(arg),"jpoint"))
        return nl->SymbolAtom("bool");
   return nl->SymbolAtom("typeerror");
}

static ListExpr BoxBoxBox(ListExpr arg){
   if(nl->ListLength(arg)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(arg),"jbox") &&
      nl->IsEqual(nl->Second(arg),"jbox"))
        return nl->SymbolAtom("jbox");
   return nl->SymbolAtom("typeerror");
}

static ListExpr BoxBoxBool(ListExpr arg){
   if(nl->ListLength(arg)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(arg),"jbox") &&
      nl->IsEqual(nl->Second(arg),"jbox"))
        return nl->SymbolAtom("bool");
   return nl->SymbolAtom("typeerror");
}


static ListExpr BoxReal(ListExpr arg){
   if(nl->ListLength(arg)!=1)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(arg),"jbox"))
      return nl->SymbolAtom("real");
   return nl->SymbolAtom("typeerror");
}

static ListExpr PointBoxBool(ListExpr arg){
   if(nl->ListLength(arg)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(arg),"jpoint") &&
      nl->IsEqual(nl->Second(arg),"jbox"))
      return nl->SymbolAtom("bool");
   return nl->SymbolAtom("typeerror");
}

static ListExpr BoxBool(ListExpr arg){
   if(nl->ListLength(arg)!=1)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(arg),"jbox"))
      return nl->SymbolAtom("bool");
   return nl->SymbolAtom("typeerror");
}


static ListExpr UnionTypeMap(ListExpr arg){
   if(nl->ListLength(arg)!=2)
     return nl->SymbolAtom("typeerror");

   ListExpr F = nl->First(arg);
   ListExpr S = nl->Second(arg);
   if( nl->IsEqual(F,"jpoint") && nl->IsEqual(S,"jbox"))
      return nl->SymbolAtom("jbox");
   if( nl->IsEqual(F,"jbox") && nl->IsEqual(S,"jbox"))
      return nl->SymbolAtom("jbox");
   if( nl->IsEqual(F,"jbox") && nl->IsEqual(S,"jpoint"))
      return nl->SymbolAtom("jbox");
   return nl->SymbolAtom("typeerror");
}


/*

4.2 Value Mapping Functions

*/
static int Equals_PP(Word* args, Word& result, int message,
                     Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JPoint* P1 = (JPoint*) args[0].addr;
  JPoint* P2 = (JPoint*) args[1].addr;
  ((CcBool*)result.addr)->Set(true,P1->Equals(P2));
  return 0;
}

static int Equals_BB(Word* args, Word& result, int message,
                     Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B1 = (JBox*) args[0].addr;
  JBox* B2 = (JBox*) args[1].addr;
  ((CcBool*)result.addr)->Set(true,B1->Equals(B2));
  return 0;
}

static int Contains_BP(Word* args, Word& result, int message,
                       Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B = (JBox*) args[0].addr;
  JPoint* P = (JPoint*) args[1].addr;
  ((CcBool*)result.addr)->Set(true,B->Contains(P));
  return 0;
}

static int Inside_PB(Word* args, Word& result, int message,
                     Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JPoint* P = (JPoint*) args[0].addr;
  JBox* B = (JBox*) args[1].addr;
  ((CcBool*)result.addr)->Set(true,B->Contains(P));
  return 0;
}


static int Union_BB(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B1 = (JBox*) args[0].addr;
  JBox* B2 = (JBox*) args[1].addr;
  JBox* r = B1->Union(B2);
  ((JBox*)result.addr)->CopyFrom(r);
  delete r;
  return 0;
}

static int Union_BP(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B = (JBox*) args[0].addr;
  JPoint* P = (JPoint*) args[1].addr;
  JBox* r = B->Union(P);
  ((JBox*)result.addr)->CopyFrom(r);
  delete r;
  return 0;
}

static int Union_PB(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JPoint* P = (JPoint*) args[0].addr;
  JBox* B = (JBox*) args[1].addr;
  JBox* r = B->Union(P);
  ((JBox*)result.addr)->CopyFrom(r);
  delete r;
  return 0;
}


static int Intersection_BB(Word* args, Word& result,
                      int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B1 = (JBox*) args[0].addr;
  JBox* B2 = (JBox*) args[1].addr;
  JBox* r;
  ((JBox*)result.addr)->CopyFrom(r = B1->Intersection(B2));
  delete r;
  return 0;
}

static int Intersects_BB(Word* args, Word& result,
                       int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B1 = (JBox*) args[0].addr;
  JBox* B2 = (JBox*) args[1].addr;
  ((CcBool*) result.addr)->Set(true,B1->Intersects(B2));
  return 0;
}


static int Size_B(Word* args, Word& result, int message,
                  Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B = (JBox*) args[0].addr;
  ((CcReal*)result.addr)->Set(true,B->Size());
  return 0;
}

static int IsEmpty_B(Word* args, Word& result, int message,
                     Word& local, Supplier s){
  result = qp->ResultStorage(s);
  JBox* B = (JBox*) args[0].addr;
  ((CcBool*)result.addr)->Set(true,B->IsEmpty());
  return 0;
}


/*

4.3 Selection Functions

*/
static int EqualsSelect(ListExpr args){
  if(nl->ListLength(args)!=2) return -1; // error
  if(nl->IsEqual(nl->First(args),"jpoint")) return 0;
  if(nl->IsEqual(nl->First(args),"jbox")) return 1;
  return -1;
}

static int UnionSelect(ListExpr args){
   if(nl->ListLength(args)!=2) return -1;
   if(nl->IsEqual(nl->First(args),"jbox")){
       if(nl->IsEqual(nl->Second(args),"jpoint"))
           return 0;
       else
           return 2;
   }
   if(nl->IsEqual(nl->First(args),"jpoint"))
      return 1;
   return -1;
}

/*

4.4 Value Mappings for Overloaded Operators

*/
ValueMapping EqualsMap[] = {Equals_PP,Equals_BB};

ValueMapping JUnionMap[] = {Union_BP,Union_PB,Union_BB};

/*

4.5 Specification of Operators

*/
const string equals_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" object_i x object_i -> bool \" "
	    "    \" _ = _ \" "
	    "    \" P1 equals P2\"))";


const string contains_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" jbox x jpoint -> bool \" "
	    "    \" _ contains _ \" "
	    "    \" B contains P\"))";

const string inside_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" jpoint x jbox -> bool \" "
	    "    \" _ inside _\" "
            "    \" P inside  B\"))";

const string union_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" object x object -> jbox \" "
	    "    \" _ union _\" "
	    "    \" B1 union B2\"))";

const string intersection_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" jbox x jbox -> jbox \" "
	    "    \" intersection (_, _) \" "
	    "    \" B1 intersection B2\"))";

const string intersects_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" jbox x jbox -> bool \" "
 	    "    \" _ intersects _ \" "
	    "    \" query B1 intersects B2\"))";


const string size_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" jbox -> real \" "
	    "    \" _ size\" "
	    "    \" B size\"))";

const string isempty_spec=
            "( (\"Signature\" \"Syntax\" \"Example\")"
            "  ( \" _ -> bool \" "
	    "    \" jb isempty\" "
	    "    \"query  B isempty\"))";

/*

4.7 Definition of Operators

*/

Operator op_equals
(
 "=", 			//name
 equals_spec,  		//specification ....
 2,                     // number of functions
 EqualsMap,			//value mapping
 EqualsSelect,		//selection function
 OiOiBool		//type mapping
);

Operator op_contains
(
 "contains", 			//name
 contains_spec,  		//specification ....
 Contains_BP,			//value mapping
 Operator::SimpleSelect,			//trivial selection function
 BoxPointBool			//type mapping
);

Operator op_inside
(
 "inside", 			//name
 inside_spec,  		//specification ....
 Inside_PB,			//value mapping
 Operator::SimpleSelect,			//trivial selection function
 PointBoxBool			//type mapping
);

Operator op_intersects
(
 "intersects", 			//name
 intersects_spec,  		//specification ....
 Intersects_BB,			//value mapping
 Operator::SimpleSelect,			//trivial selection function
 BoxBoxBool			//type mapping
);


Operator op_junion
(
 "union", 		//name
 union_spec,  		//specification ....
 3,                     // number of functions
 JUnionMap,		//value mapping
 UnionSelect,		//selection function
 UnionTypeMap		//type mapping
);



Operator op_jintersection
(
 "intersection",		//name
 intersection_spec,  		//specification ....
 Intersection_BB,		//value mapping
 Operator::SimpleSelect,			//trivial selection function
 BoxBoxBox			//type mapping
);

Operator op_size
(
 "size", 			//name
 size_spec,  			//specification
 Size_B,			//value mapping
 Operator::SimpleSelect,			//trivial selection function
 BoxReal			//type mapping
);

Operator op_isempty
(
 "isempty", 			//name
 isempty_spec,  		//specification ....
 IsEmpty_B,			//value mapping
 Operator::SimpleSelect,			//trivial selection function
 BoxBool			//type mapping
);

/*

5 Creating The Algebra

*/
class JBBoxAlgebra : public Algebra
{
 public:
  JBBoxAlgebra() : Algebra()
  {
    AddTypeConstructor( &jpoint );
    jpoint.AssociateKind("DATA");
    AddTypeConstructor(&jbox);
    jbox.AssociateKind("DATA");
    AddOperator(&op_equals);
    AddOperator(&op_contains);
    AddOperator(&op_inside);
    AddOperator(&op_junion);
    AddOperator(&op_jintersection);
    AddOperator(&op_size);
    AddOperator(&op_isempty);
    AddOperator(&op_intersects);
   }
  ~JBBoxAlgebra(){};
};

JBBoxAlgebra jbboxalgebra;


/*

6 Initialization

\label{initialization}
Here we have to initialize the algebra as well as the
Java Virtual Machine.

*/
extern "C"
Algebra*
InitializeJBBoxAlgebra( NestedList* nlRef,
                        QueryProcessor* qpRef )
{
  jvminit = new JVMInitializer();
  env = jvminit->getEnv();
  jvm = jvminit->getJVM();

  nl = nlRef;
  qp = qpRef;
  return (&jbboxalgebra);
}

} // end namespace jbbox
