/*


//[_] [\_]
//[TOC] [\tableofcontents]
//[Title] [ \title{DateTime Algebra} \author{Thomas Behr} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]



[1] Rose Algebra

[TOC]

  
January 3rd, 2003 Mirco G[ue]nster and Ismail Zerrad

This algebra implements an interface between the Rose Algebra which was
implemented in the Java programming language and Secondo.
This means that this module is an algebra in terms of Secondo but the 
proper functionality is implemented in Java. Therefore most of the
functions here are wrapper functions which call with help of the
JNI (Java Native Interface) according java methods.
This algebra provides three type constructors ~ccpoints~, ~cclines~
and ~ccregions~ with its operations.
For more details of the "Rose"-Algebra in common see the paper:
"Realm-Based Spatial Data Types - The ROSE Algebra" 
by Ralf Hartmut Gueting and Markus Schneider.
For more details of the JNI (Java Native Interface) see 
"The Java (tm) Native Interface Programmmers Guide and Specification"
by Sheng Liang. This book is online available as PDF file at www.sun.com.

1 Preliminaries

1.1 Includes and global declarations

*/

using namespace std;

#include "Algebra.h"
#include "Application.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"	//needed because we return a CcBool in an op.
#include "StandardAttribute.h"
#include <stack>
#include <time.h>

extern NestedList* nl;
static QueryProcessor* qp;

#include "Attribute.h"
#include <jni.h>
#include <JVMInit.h>

/*
 The JVMInitializer provides access to a pointer to
   the JNI Environment and to a pointer to the 
   Java Virtual Machine. 

*/
static JVMInitializer *jvminit = 0;

/* Pointer to the JVM Environment and JVM. */
static JNIEnv *env;
static JavaVM *jvm;
static jclass clsPoints;
static jclass clsLines;
static jclass clsRegions;
static jclass clsRational;
static jclass clsPoint;
static jclass clsPointMultiSet;
static jclass clsSegment;
static jclass clsSegMultiSet;
static jclass clsLinkedList;
static jclass clsCycleList;
static jclass clsCycleListListPoints;
static jclass clsRationalFactory;
static jclass clsSegmentComparator;
static jclass clsROSEAlgebra;


/*
1.2 Error functions.

This error function is called if an error referring to the
JNI interface occurs. In most cases this indicates that 
there are problems which the installed JDK. 
In this case please check the following items first:

a) Under Windows the jvm.dll must be in your PATH.
b) All paths of used java classes must have an entry in the 
   JNIPath.ini file.
c) Used JAR archives must also have an entry in that file.


*/

/* 
this function prints an error message including the line 
	where the error occured. Should never invoked normally. 

*/
static void error(char *name, int line) {
  cerr << "Error in " << name << " in line: " << line << "." << endl;
  if(env->ExceptionOccurred())
    env->ExceptionDescribe();
  exit(1);
}

/* 
this function prints an error message including the line
   where the error occured. This method is used for calls
   to Java methods. The name of the Java method which 
   should invoked is also printed. 

*/
static void error(char *filename, char *classname, int line) {
  cerr << "Error in " << filename << " in line: " << line 
       << ". " << endl << "Requested Java method couldn't invoked: " 
       << classname << "." << endl;
  if(env->ExceptionOccurred())
    env->ExceptionDescribe();
  exit(1);
}

/* 
these functions are used for debugging and could removed in the 
   final version of this algebra. Just print a message to stdout. 

*/
static void debug(int line) {
  /*
  FILE *debugFile = fopen("DEBUG.TXT", "a");
  cout << "\t***************************" << endl;
  cout << "\t" << "Line No. " << line 
       << " in RoseAlgebra.cpp was tangented." 
       << " For debug purposes." << endl;
  cout << "\t***************************" << endl;
  fprintf(debugFile, "Line: %d\n", line); 
  fclose(debugFile);
  */
}

/*
1.3 Dummy Functions

Not interesting, but needed in the definition of a type constructor.

*/

/*
1.4 Conversion Functions

These functions convert a C++-Nested List into a Java object and
vice versa. 

*/


/* 
The following function takes a java object of type Rational
   and returns a suitable ListExpr 

*/
static ListExpr Convert_JavaToC_Rational(jobject jRational) {
  /* Get method ID of getDouble. */
  jmethodID midGetDouble = env->GetMethodID(clsRational, "getDouble", "()D");

  jdouble value = env->CallDoubleMethod(jRational, midGetDouble);

  env->DeleteLocalRef(jRational);

  return nl->RealAtom(value);
}

/* 
The following function takes a java object of type Point
   and returns a suitable ListExpr 

*/
static ListExpr Convert_JavaToC_Point(jobject jPoint) {
  /* Get the field ID of x */
  jfieldID fidX = env->GetFieldID(clsPoint, "x", "Ltwodsack/util/number/Rational;");
  if (fidX == 0) error(__FILE__,__LINE__);

  /* Get the field ID of y. */
  jfieldID fidY = env->GetFieldID(clsPoint, "y", "Ltwodsack/util/number/Rational;");
  if (fidY == 0) error(__FILE__,__LINE__);

  /* Get x itself */
  jobject X = env->GetObjectField(jPoint, fidX);
  if (X == 0) error(__FILE__,__LINE__);

  /* Get y itself */
  jobject Y = env->GetObjectField(jPoint, fidY);
  if (Y == 0) error(__FILE__,__LINE__);

  return nl->Cons
    (
     Convert_JavaToC_Rational(X), 
     nl->Cons
     (
      Convert_JavaToC_Rational(Y), 
      nl->TheEmptyList()
      )
     );
}


/*
 The following function takes a java object of type Points
   and returns a suitable ListExpr 

*/
static ListExpr Convert_JavaToC_Points(jobject jPoints) {
  /* Get the class */
  jclass cls = clsPoints;

  /* Get the field ID of pointlist */
  jfieldID fid = env->GetFieldID(cls, "pointset", "Ltwodsack/set/PointMultiSet;");
  if (fid == 0) error(__FILE__,__LINE__);
  
  /* Get the field itself */
  jobject jpointlist = env->GetObjectField(jPoints, fid);
  if (jpointlist == 0) error(__FILE__,__LINE__);

  /* Get the class PointMultiSet */
  jclass clsLL = clsPointMultiSet;

  /* Get the method ID of toArrray */
  jmethodID midToArray = env->GetMethodID(clsLL, "toArray", "()[Ljava/lang/Object;");
  if (midToArray == 0) error(__FILE__,__LINE__);

  /* Call the method itself */ 
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jpointlist, midToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  /* Determine the length of the result array. */
  int oarrlen = (int)env->GetArrayLength(oarr);

  /* Collect all calculated subnodes in a stack first. */
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  /* Then assemble the result. */
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_Point(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
  } 

  env->DeleteLocalRef(jpointlist);
  env->DeleteLocalRef(oarr);
  env->DeleteLocalRef(jPoints); 

  return result;
}

/*
 The following function takes a java object of type Segment
   and returns a suitable ListExpr 

*/
static ListExpr Convert_JavaToC_Segment(jobject jSegment) {
  /* Get the class */
  jclass cls = clsSegment;

  /* Get the method ID of startPoint */
  jmethodID midStart = env->GetMethodID(cls, "getStartpoint", "()Ltwodsack/setelement/datatype/basicdatatype/Point;");
  if (midStart == 0) error(__FILE__,__LINE__);

  /* Get the method ID of endPoint */
  jmethodID midEnd = env->GetMethodID(cls, "getEndpoint", "()Ltwodsack/setelement/datatype/basicdatatype/Point;");
  if (midEnd == 0) error(__FILE__,__LINE__);

  /* Get the start Point itself */
  jobject startP = env->CallObjectMethod(jSegment, midStart);
  if (startP == 0) error(__FILE__,__LINE__);

  /* Get the end Point itself */
  jobject endP = env->CallObjectMethod(jSegment, midEnd);
  if (endP == 0) error(__FILE__,__LINE__);

  /* Get the field ID of x (start point) */
  jfieldID fidX1 = env->GetFieldID(clsPoint, "x", "Ltwodsack/util/number/Rational;");
  if (fidX1 == 0) error(__FILE__,__LINE__);

  /* Get the field ID of y (start point) */
  jfieldID fidY1 = env->GetFieldID(clsPoint, "y", "Ltwodsack/util/number/Rational;");
  if (fidY1 == 0) error(__FILE__,__LINE__); 

  /* Get the field ID of x (end point) */
  jfieldID fidX2 = env->GetFieldID(clsPoint, "x", "Ltwodsack/util/number/Rational;");
  if (fidX2 == 0) error(__FILE__,__LINE__);

  /* Get the field ID of y (end point) */
  jfieldID fidY2 = env->GetFieldID(clsPoint, "y", "Ltwodsack/util/number/Rational;");
  if (fidY2 == 0) error(__FILE__,__LINE__);

  /* Get the field x itself (start point) */
  jobject X1 = env->GetObjectField(startP, fidX1);
  if (X1 == 0) error(__FILE__,__LINE__);

  /* Get the field y itself (start point) */
  jobject Y1 = env->GetObjectField(startP, fidY1);
  if (X1 == 0) error(__FILE__,__LINE__);

  /* Get the field x itself (end point) */
  jobject X2 = env->GetObjectField(endP, fidX2);
  if (X1 == 0) error(__FILE__,__LINE__);

  /* Get the field y itself (end point) */
  jobject Y2 = env->GetObjectField(endP, fidY2);
  if (X1 == 0) error(__FILE__,__LINE__);

  env->DeleteLocalRef(startP);
  env->DeleteLocalRef(endP);

  return 
    nl->Cons
    (
     Convert_JavaToC_Rational(X1), 
     nl->Cons
     (
      Convert_JavaToC_Rational(Y1), 
      nl->Cons
      (
       Convert_JavaToC_Rational(X2), 
       nl->Cons
       (
	Convert_JavaToC_Rational(Y2), 
	nl->TheEmptyList()
	)
       )
      )
     );

}

/* 

The following function takes a java object of type Lines
   and returns a suitable ListExpr. 

*/
static ListExpr Convert_JavaToC_Lines(jobject jLines) {

   /* Get the class */
  jclass cls = clsLines;
  
  /* Get the field ID of seglist */
  jfieldID fid = env->GetFieldID(cls, "segset", "Ltwodsack/set/SegMultiSet;");
  if (fid == 0) error(__FILE__,__LINE__);

  /* Get the field itself */
  jobject jseglist = env->GetObjectField(jLines, fid);
  if (jseglist == 0) error(__FILE__,__LINE__);

  /* Get the class SegMultiSet */
  jclass clsLL = clsSegMultiSet;

  /* Get the method ID of toArrray */
  jmethodID midToArray = env->GetMethodID(clsLL, "toArray", "()[Ljava/lang/Object;");
  if (midToArray == 0) error(__FILE__,__LINE__);

  /* Call the method itself */ 
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jseglist, midToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  /* Determine the length of the result array. */
  int oarrlen = (int)env->GetArrayLength(oarr);

  /* Collect all calculated subnodes in a stack first. */
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  /* Then assemble the result. */
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_Segment(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
  } 

  env->DeleteLocalRef(oarr);
  env->DeleteLocalRef(jseglist);

  return result;
}

/*
 The following function takes a java object of type ElemList
   and returns a suitable ListExpr. 

*/
static ListExpr Convert_JavaToC_ElemList(jobject jpointList) {
  /* Get the class */
  jclass cls = clsLinkedList;
  
  /* Get the method ID of toArrray */
  jmethodID midToArray = env->GetMethodID(cls, "toArray", "()[Ljava/lang/Object;");
  if (midToArray == 0) error(__FILE__,__LINE__);

  /* Call the method itself */ 
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jpointList, midToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  /* Determine the length of the result array. */
  int oarrlen = (int)env->GetArrayLength(oarr);

  /* Collect all calculated subnodes in a stack first. */
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  /* Then assemble the result. */
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_Point(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
  } 

  env->DeleteLocalRef(oarr);

  return result;
}

/*
 The following function takes a java object of type ElemListList
   and returns a suitable ListExpr. 

*/
static ListExpr Convert_JavaToC_ElemListList(jobject jCycleList) {
  /* Get the class */
  jclass cls = clsCycleList;
  
  /* Get the method ID of toArrray */
  jmethodID midToArray = env->GetMethodID(cls, "toArray", "()[Ljava/lang/Object;");
  if (midToArray == 0) error(__FILE__,__LINE__);

  /* Call the method itself */ 
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jCycleList, midToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  /* Determine the length of the result array. */
  int oarrlen = (int)env->GetArrayLength(oarr);

  /* Collect all calculated subnodes in a stack first. */
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  /* Then assemble the result. */
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_ElemList(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
  } 

  env->DeleteLocalRef(oarr);

  return result;
}



/* 
The following function takes a java object of type ElemListListList
   and returns a suitable ListExpr. 

*/
static ListExpr Convert_JavaToC_ElemListListList(jobject jCycleListListPoints) {
  /* Get the class */
  jclass cls = clsCycleListListPoints;
  
  /* Get the method ID of toArrray */
  jmethodID midToArray = env->GetMethodID(cls, "toArray", "()[Ljava/lang/Object;");
  if (midToArray == 0) error(__FILE__,__LINE__);

  /* Call the method itself */ 
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jCycleListListPoints, midToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  /* Determine the length of the result array. */
  int oarrlen = (int)env->GetArrayLength(oarr);

  /* Collect all calculated subnodes in a stack first. */
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  /* Then assemble the result. */
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_ElemListList(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
  } 

  env->DeleteLocalRef(oarr);
  
  return result;
}

/* 
The following function takes a java object of type Regions
   and returns a suitable ListExpr. 

*/
static ListExpr Convert_JavaToC_Regions(jobject jRegions) {

  if (!jRegions) cerr << "jRegions is NULL!" << endl;
  
  /* Get the method ID of cyclesPoints */
  jmethodID midCycles = env->GetMethodID(clsRegions, "cyclesPoints", "()Ltwodsack/util/collection/CycleListListPoints;");
  if (midCycles == 0) error(__FILE__,__LINE__);

  /* Call the method itself */ 
  jobject cycles = env->CallObjectMethod(jRegions, midCycles);
  if (cycles == 0) error(__FILE__,__LINE__);

  ListExpr res = Convert_JavaToC_ElemListListList(cycles);
  env->DeleteLocalRef(cycles);
  return res;
}


/*
 The following function takes a ListExpr of an Rational object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Rational(const ListExpr &le) {
  /* Check whether six elements are in le. */
  if (nl->ListLength(le) != 6) error(__FILE__,__LINE__);

  /* Now we calculate the necessary data for creating a Rational object. */
  ListExpr e2 = nl->Second(le);
  ListExpr e3 = nl->Third(le);
  ListExpr e4 = nl->Fourth(le);
  ListExpr e6 = nl->Sixth(le);

  string value2 = nl->SymbolValue(e2);
  long value3 = nl->IntValue(e3);
  long value4 = nl->IntValue(e4);
  long value6 = nl->IntValue(e6);

  int Rat_sign = (value2 == "-") ? -1 : 1;
  int Rat_intPart = value3 * Rat_sign;
  int Rat_numDec = value4 * Rat_sign;
  int Rat_dnmDec = value6;

  /* Get the method ID of constRational */
  jmethodID mid = env->GetStaticMethodID(clsRationalFactory, "constRational", "(II)Ltwodsack/util/number/Rational;");
  if (mid == 0) error(__FILE__,__LINE__);

  jobject result = env->CallStaticObjectMethod(clsRationalFactory, mid, 
					 Rat_intPart * Rat_dnmDec + Rat_numDec,
					 Rat_dnmDec);
  if (result == 0) error(__FILE__,__LINE__);
  
  return result;
}

/*
 The following function takes a ListExpr of a Point object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Point(const ListExpr &le) {
  /* Check whether two elements are in le. */
  if (nl->ListLength(le) != 2) error(__FILE__,__LINE__);

  /* Now we calculate the necessary data for creating a Point object. */
  ListExpr e1 = nl->First(le);
  ListExpr e2 = nl->Second(le);

  if (
      (nl->IsAtom(e1)) && 
      (nl->IsAtom(e2)) && 
      (nl->AtomType(e1) == IntType) 
      && (nl->AtomType(e2) == IntType)) {
    debug(__LINE__);
    /* Both coordinates are integers */
    int intValue1 = nl->IntValue(e1);
    int intValue2 = nl->IntValue(e2);
    
    /* Get the method ID of the constructor which takes two ints. */
    jmethodID mid = env->GetMethodID(clsPoint, "<init>", "(II)V");
    if (mid == 0) error(__FILE__,__LINE__);
    
    jobject result = env->NewObject(clsPoint, mid, intValue1, intValue2);
    if (result == 0) error(__FILE__,__LINE__);

    return result;
  }
  else if 
    (
     (nl->IsAtom(e1)) && 
     (nl->IsAtom(e2)) && 
     (nl->AtomType(e1) == RealType) && 
     (nl->AtomType(e2) == RealType)) {
    debug(__LINE__);
    /* Both coordinates are reals */
    jfloat realValue1 = nl->RealValue(e1);
    jfloat realValue2 = nl->RealValue(e2);
        
    /* Get the method ID of the constructor which takes two float. */
    jmethodID mid = env->GetMethodID(clsPoint, "<init>", "(DD)V");
    if (mid == 0) error(__FILE__,__LINE__);
    
    jobject result = env->NewObject(clsPoint, mid, 
				    (jfloat)realValue1, (jfloat)realValue2);
    if (result == 0) error(__FILE__,__LINE__);

    return result;
  } else {
    /* Both coordinates are Rationals */

    /* Get the method ID of the constructor which takes two Rationals. */
    jmethodID mid = env->GetMethodID(clsPoint, "<init>", 
				     "(Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;)V");
    if (mid == 0) error(__FILE__,__LINE__);
    
    jobject num1 = Convert_CToJava_Rational(e1);
    jobject num2 = Convert_CToJava_Rational(e2);

    jobject result = env->NewObject(clsPoint, mid,num1,num2);
    env->DeleteLocalRef(num1);
    env->DeleteLocalRef(num2);

    return result;
  }
}

/*

 The following function takes a ListExpr of a Points object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Points(const ListExpr &le) {

  /* Get the method ID of the constructor of Points */
  jmethodID midPoints = env->GetMethodID(clsPoints, "<init>", "()V");
  if (midPoints == 0) error(__FILE__,__LINE__);

  jobject points = env->NewObject(clsPoints, midPoints);
  if (points == 0) error(__FILE__,__LINE__);

  /* Now we detect the length of le. */
  int ll = nl->ListLength(le);
  
  /* Get the method ID of add */
  jmethodID midAdd = env->GetMethodID(clsPoints, "add", "(Ltwodsack/setelement/datatype/basicdatatype/Point;)V");
  if (midAdd == 0) error(__FILE__,__LINE__);

  bool isPoints = true;

  ListExpr first = nl->First(le);
  if (nl->IsAtom(first)) isPoints = false;
  else {
    isPoints = nl->ListLength(first) == 2;
  }

  if (isPoints) {
    /* Now we insert in a for-loop all points into the Points object. */
    ListExpr restlist = le;
    for (int i = 0; i < ll; i++) {
      ListExpr first = nl->First(restlist);
      restlist = nl->Rest(restlist);
      /* create a java object points */
      jobject jfirst = Convert_CToJava_Point(first);
      if (jfirst == 0) error(__FILE__,__LINE__);
      env->CallVoidMethod(points, midAdd, jfirst);
      env->DeleteLocalRef(jfirst);
    }
  }
  else {
    jobject jfirst = Convert_CToJava_Point(le);
    if (jfirst == 0) error(__FILE__,__LINE__);
    env->CallVoidMethod(points, midAdd, jfirst);
    env->DeleteLocalRef(jfirst);
  }
  return points;
}

/*
 The following function takes a ListExpr of a Segment object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Segment(const ListExpr &le) {
  /* Check wheather four elements are in le. */
  if (nl->ListLength(le) != 4) error(__FILE__,__LINE__);

  /* Now we calculate the necessary data for creating a Point object. */
  ListExpr e1 = nl->First(le);
  ListExpr e2 = nl->Second(le);
  ListExpr e3 = nl->Third(le);
  ListExpr e4 = nl->Fourth(le);

  if 
    (
     (nl->IsAtom(e1)) && 
     (nl->IsAtom(e2)) && 
     (nl->IsAtom(e3)) && 
     (nl->IsAtom(e4)) && 
     (nl->AtomType(e1) == IntType) && 
     (nl->AtomType(e2) == IntType) && 
     (nl->AtomType(e3) == IntType) && 
     (nl->AtomType(e4) == IntType)) {
    debug(__LINE__);
    /* All coordinates are integers */
    int intValue1 = nl->IntValue(e1);
    int intValue2 = nl->IntValue(e2);
    int intValue3 = nl->IntValue(e3);
    int intValue4 = nl->IntValue(e4);
    
    /* Get the method ID of the constructor which takes four ints. */
    jmethodID mid = env->GetMethodID(clsSegment, "<init>", "(IIII)Ltwodsack/setelement/datatype/basicdatatype/Segment;");
    if (mid == 0) error(__FILE__,__LINE__);
    
    jobject result = env->NewObject(clsSegment, mid, 
				    intValue1, intValue2, 
				    intValue3, intValue4);
    if (result == 0) error(__FILE__,__LINE__);

    return result;
  }
  else if 
    (
     (nl->IsAtom(e1)) && 
     (nl->IsAtom(e2)) &&
     (nl->IsAtom(e3)) &&
     (nl->IsAtom(e4)) &&
     (nl->AtomType(e1) == RealType) && 
     (nl->AtomType(e2) == RealType) &&
     (nl->AtomType(e3) == RealType) &&
     (nl->AtomType(e4) == RealType)) {
    debug(__LINE__);
    /* All coordinates are reals */
    double realValue1 = nl->RealValue(e1);
    double realValue2 = nl->RealValue(e2);
    double realValue3 = nl->RealValue(e3);
    double realValue4 = nl->RealValue(e4);

    /* Get the method ID of the constructor which takes four float. */
    jmethodID mid = env->GetMethodID(clsSegment, "<init>", "(DDDD)V");
    if (mid == 0) error(__FILE__,__LINE__);
    
    jobject result = env->NewObject(clsSegment, mid, 
				     realValue1,  realValue2,
				     realValue3,  realValue4);
    if (result == 0) error(__FILE__,__LINE__);

    return result;
  } else {
    /* Get the method ID of the constructor which takes four Rationals. */
    jmethodID mid = env->GetMethodID
      (clsSegment, "<init>", 
       "(Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;)V");
    if (mid == 0) error(__FILE__,__LINE__);
    
    jobject num1 = Convert_CToJava_Rational(e1);
    jobject num2 = Convert_CToJava_Rational(e2);
    jobject num3 = Convert_CToJava_Rational(e3);
    jobject num4 = Convert_CToJava_Rational(e4);

    jobject result = env->NewObject(clsSegment, mid, num1,num2,num3,num4);
    env->DeleteLocalRef(num1);
    env->DeleteLocalRef(num2);
    env->DeleteLocalRef(num3);
    env->DeleteLocalRef(num4);

    return result;
  }
}

/* 
The following function takes a ListExpr of a Lines object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Lines(const ListExpr &le) {

  /* Get the method ID of the constructor of Lines */
  jmethodID midLines = env->GetMethodID(clsLines, "<init>", "()V");
  if (midLines == 0) error(__FILE__,__LINE__);

  jobject lines = env->NewObject(clsLines, midLines);
  if (lines == 0) error(__FILE__,__LINE__);

  /* Now we detect the length of le. */
  int ll = nl->ListLength(le);
  
  /* Get the method ID of add */
  jmethodID midAdd = env->GetMethodID(clsLines, "add", "(Ltwodsack/setelement/datatype/basicdatatype/Segment;)V");
  if (midAdd == 0) error(__FILE__,__LINE__);

  /* Now we insert in a for-loop all points into the Lines object. */
  ListExpr restlist = le;
  for (int i = 0; i < ll; i++) {
    ListExpr first = nl->First(restlist);
    restlist = nl->Rest(restlist);
    /* create a java object segment */
    jobject jfirst = Convert_CToJava_Segment(first);
    if (jfirst == 0) error(__FILE__,__LINE__);
    env->CallVoidMethod(lines, midAdd, jfirst);
    env->DeleteLocalRef(jfirst);
  }

  return lines;
}

/*
 The following function takes a ListExpr of a Regions object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Regions(const ListExpr &le) {
  /* We have to collect all segments into a segment list first */
  
  /* get method ID of constructor */
  jmethodID midSC = env->GetMethodID(clsSegmentComparator, "<init>", "()V");
  if (midSC == 0) error(__FILE__,__LINE__);

  /* Get the method ID of the constructor */
  jmethodID midSMS = env->GetMethodID(clsSegMultiSet, "<init>", "(Ltwodsack/util/comparator/SegmentComparator;)V");
  if (midSMS == 0) error(__FILE__,__LINE__);

  /* Get the method ID of add */
  jmethodID midSMSAdd = env->GetMethodID(clsSegMultiSet, "add", "(Ltwodsack/setelement/datatype/basicdatatype/Segment;)V");
  if (midSMSAdd == 0) error(__FILE__,__LINE__);

  /* Create a new SegmentComparator */
  jobject jSC = env->NewObject(clsSegmentComparator, midSC);
  if (jSC == 0) error(__FILE__,__LINE__);

  /* Create a SMS object. */
  jobject segMS = env->NewObject(clsSegMultiSet, midSMS, jSC);
  if (segMS == 0) error(__FILE__,__LINE__);

  /* Get the method ID of the constructor */
  jmethodID midSegment = env->GetMethodID
    (clsSegment, "<init>", "(Ltwodsack/setelement/datatype/basicdatatype/Point;Ltwodsack/setelement/datatype/basicdatatype/Point;)V");
  if (midSegment == 0) error(__FILE__,__LINE__);

  /* Determine how many faces are in le. */
  int nllfaces = nl->ListLength(le);
  
 /* Now we put all segments of the faces into the seglist. */
  ListExpr restFaceList = le;
  for (int i = 0; i < nllfaces; i++) {
    ListExpr firstFace = nl->First(restFaceList);
    restFaceList = nl->Rest(restFaceList);
    /* Determine how many cycles are in firstFace. */
    int nllcycles = nl->ListLength(firstFace);

   /* Now we put all segments of the cycles into the seglist. */
    ListExpr restCycleList = firstFace;
    for (int j = 0; j < nllcycles; j++) {
      ListExpr firstCycle = nl->First(restCycleList);
      restCycleList = nl->Rest(restCycleList);
      /* Determine how many vertices are in firstCycle. */
      int nllvertex = nl->ListLength(firstCycle);
      /* Calculate all vertices first. */
      jobject vertex[nllvertex];
      ListExpr restVertexList = firstCycle;
      for (int k = 0; k < nllvertex; k++) {
	ListExpr firstVertex = nl->First(restVertexList);
	restVertexList = nl->Rest(restVertexList);
	vertex[k] = Convert_CToJava_Point(firstVertex);
      }
      /* Connect the kth and the (k+1)th Point to a segment. */
      jobject segment[nllvertex];
      for (int k = 0; k < nllvertex - 1; k++) {
	segment[k] = env->NewObject(clsSegment, midSegment, vertex[k], vertex[k+1]);
	if (segment[k] == 0) error(__FILE__,__LINE__);
      }
	
      segment[nllvertex-1] = env->NewObject(clsSegment, midSegment, vertex[nllvertex-1], vertex[0]);
      if (segment[nllvertex-1] == 0) error(__FILE__,__LINE__);

      //clear memory used for vertices
      for (int k = 0; k < nllvertex; k++)
	env->DeleteLocalRef(vertex[k]);

      /* Now add all segments to the SegMultiSet */
      for (int k = 0; k < nllvertex; k++) {
	env->CallVoidMethod(segMS, midSMSAdd, segment[k]);
      }
      
      //clear memory used for segments
      for (int k = 0; k < nllvertex-1; k++)
	env->DeleteLocalRef(segment[k]);
    }
  }
  
  /* Get the methodID of the constructor. */
  jmethodID midRegions = env->GetMethodID(clsRegions, "<init>", 
					  "(Ltwodsack/set/SegMultiSet;)V");
  if (midRegions == 0) error(__FILE__,__LINE__);
  
  jobject result = env->NewObject(clsRegions, midRegions, segMS);
  if (result == 0) error(__FILE__,__LINE__);

  //free memory used by Java objects
  env->DeleteLocalRef(jSC);
  env->DeleteLocalRef(segMS);
  
  return result;
}

/* 
2 Type Constructor ~Points~

2.1 Data Structure - Class ~CcPoints~

The class CcPoints is just a wrapper to the 
according Points Java class. 
The cls attribute holds a pointer to the 
Java class Points and the obj attribute 
holds a pointer to the java object itself.
These pointers are used to invoke according 
java methods.

*/

class CcPoints: public StandardAttribute {
private:
  jobject obj;
  FLOB objectData;
  bool canDelete;
  bool Defined;
  void RestoreFLOBFromJavaObject();
  
public:
  /* Inherited methods of Attribute */
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined() const;
  int Sizeof();
  //void *GetValue();
  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  void Destroy();
  jobject GetObject() {
    return obj; }
  bool Adjacent(Attribute *arg);
  int NumOfFLOBs();
  FLOB *GetFLOB(const int i);
  void Initialize();
  void Finalize() {
    //cout << "++++++++++++++++++++ called Finalize of CcPoints +++++++++++++++++++++++++" << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  void RestoreJavaObjectFromFLOB();
  size_t HashValue();
  
  /* This contructor takes just one serial string 
     of the underlying java object and its length. */
  //CcPoints(char *serial, int len);
  /* This constructor takes the nested list representation
     of CcPoints and recovers the underlying java object with
     help of this data. */
  CcPoints(const ListExpr &le);
  /* This constructor takes a pointer to a java object which is
     already created. */
  CcPoints(const jobject jobj);
  /* This constructor creates an empty CcPoints object. */	
  CcPoints();
  CcPoints(size_t size);
  /* retrieves the nested list representation of the underlying
     java object. */

    void SetObject(jobject obj) {
    this->obj = obj;
  }

  bool GetNL(ListExpr &le);
  /* Destructor of CcPoints. This destructor destroys also the 
     object inside the JVM. */

  ~CcPoints() {
    //cout << "++++++++++++++++++++ called destructor of CcPoints +++++++++++++++++++++++++" << endl;    
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  /* Returns the pointer to the proper java objet. */
  jobject GetObj();
  void Print();
  
};

/* 
Inherited method of StandardAttribute 

*/
int CcPoints::Compare(Attribute *attr) {
  jmethodID mid = env->GetMethodID(clsPoints,"compare","(LPoints;)I");
  if (mid == 0) error(__FILE__,__LINE__);

  CcPoints *P = (CcPoints *) attr;
  return env->CallByteMethod(obj,mid,P->obj);
}

/* 
Inherited method of StandardAttribute 

*/
Attribute *CcPoints::Clone() {
  CcPoints* res = new CcPoints(objectData.Size());
  res->CopyFrom(this);
  return res;
}

/* 
Inherited method of StandardAttribute 

*/
bool CcPoints::IsDefined() const {
  return true;
}

/* 
Inherited method of StandardAttribute 

*/
int CcPoints::Sizeof() {
  return sizeof(CcPoints);
}

/* 
Inherited method of StandardAttribute 

*/
 /*
void *CcPoints::GetValue() {
  return (void *)-1;
}
 */

/* 
Inherited method of StandardAttribute 

*/
void CcPoints::SetDefined(bool Defined) {
  this->Defined = Defined;
}

/* 
Inherited method of StandardAttribute 

*/
void CcPoints::CopyFrom(StandardAttribute* right) {
  CcPoints *P = (CcPoints *)right;
  objectData.Resize(P->objectData.Size());
  char *data = new char[P->objectData.Size()];
  P->objectData.Get(0,P->objectData.Size(),data);
  objectData.Put(0,P->objectData.Size(),data);
  delete [] data;
  //RestoreJavaObjectFromFLOB();
  //if (obj)
  // env->DeleteLocalRef(obj);
  obj=0;
}

/* 
Inherited method of StandardAttribute 

*/
size_t CcPoints::HashValue() {
  return 0;
}

bool CcPoints::Adjacent(Attribute * arg) {
  return false;
}

int CcPoints::NumOfFLOBs(){
  return 1;
}

FLOB *CcPoints::GetFLOB(const int i){
   assert(i==0);
   return &objectData;
}

void CcPoints::Initialize() {
  //RestoreJavaObjectFromFLOB();
  obj = 0;
}



/* 
This contructor takes just one serial string 
   of the underlying java object and its length. 

*/
 /*
CcPoints::CcPoints(char *serial, int len) {
  debug(__LINE__);
  jbyteArray jbarr = 0;
  jmethodID mid_Rose;
  jclass cls_Rose;

  // Get the Class 
  cls = env->FindClass("Points");
  if (cls == 0) error(__FILE__, __LINE__);

  // Allocate a new byte array inside the VM.
  jbarr = env->NewByteArray(len);
  if (jbarr == 0) error(__FILE__, __LINE__);

  // Store the data into this array. 
  env->SetByteArrayRegion(jbarr, 0, len, (jbyte *)serial);

  // Get the Class RoseImExport
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  // Get the method ID of imprt_arr 
  mid_Rose = env->GetStaticMethodID(cls_Rose, "imprt_arr", 
				    "([B)Ljava/lang/Object;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);
  
  // Call the static imprt_arr method
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jbarr);
  if (obj == 0) error(__FILE__, __LINE__);  

}
 */

CcPoints::CcPoints(size_t size):objectData(size),canDelete(false) {
  SetDefined(true);
}

/* 
This constructor takes the nested list representation
   of CcPoints and recovers the underlying java object with
   help of this data. 

*/
CcPoints::CcPoints(const ListExpr &le):objectData(1) {
  obj = Convert_CToJava_Points(le);
  canDelete = false;
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
This constructor takes a pointer to a java object which is
   already created. 

*/
CcPoints::CcPoints(const jobject jobj):objectData(1) {
  /* Get the Class Points */
  canDelete = false;
  obj = jobj;
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
This constructor creates an empty CcPoints object. 

*/	
CcPoints::CcPoints() {
  jmethodID mid;

  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(clsPoints, "<init>", "()V");
  if (mid == 0) error(__FILE__,__LINE__);
  
  /* Create a Java-object Point. */
  obj = env->NewObject(clsPoints, mid);
  if (obj == 0) error(__FILE__,__LINE__);
}


/* 
retrieves the nested list representation of the underlying
     java object. 

*/
bool CcPoints::GetNL(ListExpr& le) {
  if (!obj) RestoreJavaObjectFromFLOB();
  assert(obj != 0);
  le = Convert_JavaToC_Points(obj);
  return true;
}

/* 
Destructor of CcPoints. This destructor destroys also the 
     object inside the JVM. 

*/

  /*
    CcPoints::~CcPoints() {
    //if(canDelete) {
    env->DeleteLocalRef(obj);
    //Destroy();
    //}
    }
  */

/*
  ~Destroy~
  
  The Destroy function as known from other (non-JNI) algebras.

*/
void CcPoints::Destroy(){
  //cout << "++++++++++++++++++++ called destroy of CcPoints +++++++++++++++++++++++++" << endl;
  canDelete=true;
}

/* 
Returns the pointer to the proper java objet. 

*/
jobject CcPoints::GetObj() {
  return obj;
}


/* 
restores the java object from FLOB
   the FLOB must exists

*/
void CcPoints::RestoreFLOBFromJavaObject(){
  jmethodID mid = env->GetMethodID(clsPoints,"writeToByteArray","()[B");
  if(mid == 0) error(__FILE__,__LINE__);
  
  jbyteArray jbytes = (jbyteArray) env->CallObjectMethod(obj,mid);
  if(jbytes == 0) error(__FILE__,__LINE__);
  
  int size = env->GetArrayLength(jbytes);
  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,JNI_ABORT);
  env->DeleteLocalRef(jbytes);
 }


/* 
creates the content of a FLOB from the given Java-Object

*/
void CcPoints::RestoreJavaObjectFromFLOB(){
  if (obj) return;
  // read the data from flob
  if(&objectData == 0){
    return;
  }
  int size = objectData.Size();
  cout << "going to read " << size << " bytes" << endl;
  char *bytes = new char[size];
  objectData.Get(0,size,bytes);
  // copy the data into a java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);
  jmethodID mid = env->GetStaticMethodID(clsPoints,"readFrom","([B)LPoints;");
  if(mid == 0){
    error(__FILE__,__LINE__);
  }
  jobject jres = env->CallStaticObjectMethod(clsPoints,mid,jbytes);
  if(jres == 0){
    error(__FILE__,__LINE__);
  }
  obj = jres;
  jbyte* elems = env->GetByteArrayElements(jbytes,0);
  env->ReleaseByteArrayElements(jbytes,elems,JNI_ABORT);
  delete [] bytes;
  bytes = NULL;
  env->DeleteLocalRef(jbytes);
}


void CcPoints::Print() {
  jmethodID mid = env->GetMethodID(clsPoints,"print","()V");
  env->CallVoidMethod(obj,mid);
}


/*
2.2 List Representation

The list representation of a CcPoints object is

----	(point1 point2 ... pointn)
----

whereas the (internal) list representation of a point is

----	(xCoord yCoord)
----

whereas the (internal) list representation of a coordinate is

----	(rat <sign> <intPart> <numDecimal> / <denomDecimal>)
----




2.3 ~In~ and ~Out~ Functions for CcPoints

*/

static ListExpr OutCcPoints( ListExpr typeInfo, Word value ) {
  CcPoints* ccpoints = (CcPoints*)(value.addr);
  ListExpr le;

  if (ccpoints->GetNL(le) == false) error(__FILE__, __LINE__);
  return le;
}


static Word InCcPoints(const ListExpr typeInfo, 
		       const ListExpr instance,
		       const int errorPos, 
		       ListExpr& errorInfo, 
		       bool& correct ) {
  CcPoints* newpoints;

  correct = true;
  newpoints = new CcPoints(instance);
  
  return SetWord(newpoints);
}

/* 
Creation of a CcPoints object. 

*/
static Word CreateCcPoints(const ListExpr typeInfo) {
  CcPoints* res = new CcPoints((size_t)1);
  res->SetObject(0);
  return SetWord(res);
}

/* 
Deletion of a CcPoints object. 

*/
static void DeleteCcPoints(Word &w) {
  //cout << "++++++++++++++++++++ called Delete of CcPointss +++++++++++++++++++++++++" << endl; 
  delete ((CcPoints *)w.addr);
  w.addr = 0;
}

/* 
Close a CcPoints object. 

*/
static void CloseCcPoints(Word & w) {
  //cout << "++++++++++++++++++++ called Close of CcPoints +++++++++++++++++++++++++" << endl; 
  delete (CcPoints *)w.addr;
  w.addr = 0;
}

/* 
Clone a CcPoints object. 

*/
static Word CloneCcPoints(const Word &w) {
  debug(__LINE__);
  return SetWord(((CcPoints *)w.addr)->Clone());
}

static void* CastCcPoints( void* addr ) {
  return new (addr) CcPoints;
}

bool OpenCcPoints(SmiRecord& valueRecord,
                  size_t& offset,
		  const ListExpr typeInfo,
		  Word& value){
  CcPoints* P = (CcPoints*)TupleElement::Open(valueRecord,offset, typeInfo);
  //P->RestoreJavaObjectFromFLOB();
  P->SetObject(0);
  value = SetWord(P);
  return true;
}

bool SaveCcPoints( SmiRecord& valueRecord,
                   size_t& offset,
		   const ListExpr typeInfo,
		   Word& value)
{ CcPoints* P = (CcPoints*) value.addr;
  TupleElement::Save(valueRecord,offset,typeInfo,P);
  return true;
}

/*
2.4 Function Describing the Signature of the Type Constructor

*/

static ListExpr PointsProperty() {
  debug(__LINE__);
  return 
    (nl->TwoElemList
     (
      nl->FiveElemList
      (nl->StringAtom("Signature"),
       nl->StringAtom("Example Type List"),
       nl->StringAtom("List Rep"),
       nl->StringAtom("Example List"),
       nl->StringAtom("Remarks")),
      nl->FiveElemList
      (nl->StringAtom("-> DATA"),
       nl->StringAtom("ccpoints"),
       nl->StringAtom("(<p1> <p2> ... <pN>)"),
       nl->StringAtom("..."),
       nl->StringAtom("a set of points "))));
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~ccpoints~ does not have arguments, this is trivial.

*/

static bool CheckCcPoints( ListExpr type, ListExpr& errorInfo ) {
  bool res = (nl->IsEqual(type, "points"));
  return res;
}


int
SizeOfCcPoints()
{
  return sizeof(CcPoints);
}

/*
2.6 Creation of the Type Constructor Instance

*/

TypeConstructor ccpoints
(
 "points",
 // name
 PointsProperty,
 // property function describing the signature
 OutCcPoints,
 // out function
 InCcPoints,
 // in function
 0, 0,
 //SaveToList and RestoreFromList functions
 CreateCcPoints,
 // object creation
 DeleteCcPoints,
 // object deletion
 OpenCcPoints,
 // object open
 SaveCcPoints,
 // object save
 CloseCcPoints,
 // object close
 CloneCcPoints,
 // object clone
 CastCcPoints,
 // cast function
 SizeOfCcPoints,
 //sizeof function
 CheckCcPoints,
 // kind checking function
 0,
 // predefined persistence function for model.
 TypeConstructor::DummyInModel,
 //TypeConstructor::DummyInModel, 	
 TypeConstructor::DummyOutModel,
 //TypeConstructor::DummyOutModel,
 TypeConstructor::DummyValueToModel,
 //TypeConstructor::DummyValueToModel,
 TypeConstructor::DummyValueListToModel
 //TypeConstructor::DummyValueListToModel 
 );


/* 
3 Type Constructor ~Lines~

3.1 Data Structure - Class ~Lines~

The class CcLines is just a wrapper to the 
according Lines Java class. 
The cls attribute holds a pointer to the 
Java class Lines and the obj attribute 
holds a pointer to the java object itself.
These pointers are used to invoke according 
java methods.

*/

class CcLines: public StandardAttribute {
private:
  jobject obj;
  FLOB objectData;
  bool canDelete;
  bool Defined;
  void RestoreFLOBFromJavaObject();

public:
  /* Inherited methods of Attribute */
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined() const;
  int Sizeof();
  //void *GetValue();
  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  void Destroy();
  jobject GetObject() {
    return obj; }
  bool Adjacent(Attribute *arg);
  int NumOfFLOBs();
  FLOB *GetFLOB(const int i);
  void Initialize();
  void Finalize() {
    //cout << "++++++++++++++++++++ called Finalize of CcLines +++++++++++++++++++++++++" << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  void RestoreJavaObjectFromFLOB();
  size_t HashValue();

  /* This contructor takes just one serial string 
     of the underlying java object and its length. */
  //CcLines(char *serial, int len);
  /* This constructor takes the nested list representation
     of CcLines and recovers the underlying java object with
     help of this data. */
  CcLines(const ListExpr &le);
  /* This constructor takes a pointer to a java object which is
     already created. */
  CcLines(const jobject jobj);
  /* retrieves the nested list representation of the underlying
     java object. */
  /* This constructor creates an empty CcLines object. */	
  CcLines();
  CcLines(size_t size);
  bool GetNL(ListExpr &le);
  
  void SetObject(jobject obj) {
    this->obj = obj;
  }

  /* Destructor of CcLines. This destructor destroys also the 
     object inside the JVM. */
  ~CcLines() {
    //cout << "++++++++++++++++++++ called destructor of CcLines +++++++++++++++++++++++++" << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  /* Returns the pointer to the proper java objet. */
  jobject GetObj();

  void Print();
};

/* 
Inherited method of StandardAttribute 

*/
int CcLines::Compare(Attribute *attr) {
  jmethodID mid = env->GetMethodID(clsLines,"compare","(LLines;)I");
  if (mid == 0) error(__FILE__,__LINE__);
  
  CcLines *L = (CcLines *) attr;
  return env->CallByteMethod(obj,mid,L->obj);
}

/* 
Inherited method of StandardAttribute 

*/
Attribute *CcLines::Clone() {
  CcLines* res = new CcLines(objectData.Size());
  res->CopyFrom(this);
  return res;
}

/* 
Inherited method of StandardAttribute 

*/
bool CcLines::IsDefined() const {
  return true;
}

/* 
Inherited method of StandardAttribute 

*/
int CcLines::Sizeof() {
  return sizeof(CcLines);
}

/* 
Inherited method of StandardAttribute 

*/
 /*
void *CcLines::GetValue() {
  return (void *)-1;
}
 */

/* 
Inherited method of StandardAttribute 

*/
void CcLines::SetDefined(bool Defined) {
  this->Defined = Defined;
}

/* 
Inherited method of StandardAttribute 

*/
void CcLines::CopyFrom(StandardAttribute* right) {
  CcLines *L = (CcLines *)right;
  objectData.Resize(L->objectData.Size());
  char *data = new char[L->objectData.Size()];
  L->objectData.Get(0,L->objectData.Size(),data);
  objectData.Put(0,L->objectData.Size(),data);
  delete [] data;
  //RestoreJavaObjectFromFLOB();
  //if (obj)
  //  env->DeleteLocalRef(obj);
  obj=0;
}

/* 
Inherited method of StandardAttribute 

*/
size_t CcLines::HashValue() {
  return 0;
}

bool CcLines::Adjacent(Attribute * arg) {
  return false;
}

int CcLines::NumOfFLOBs(){
  return 1;
}

FLOB *CcLines::GetFLOB(const int i) {
  assert(i==0);
  return &objectData;
}

void CcLines::Initialize() {
  //RestoreJavaObjectFromFLOB();
  obj = 0;
}

/*

 This contructor takes just one serial string 
   of the underlying java object and its length. 

*/

 /*
CcLines::CcLines(char *serial, int len) {
  debug(__LINE__);
  jbyteArray jbarr = 0;
  jmethodID mid_Rose;
  jclass cls_Rose;

  // Get the Class
  cls = env->FindClass("Lines");
  if (cls == 0) error(__FILE__, __LINE__);

  // Allocate a new byte array inside the VM.
  jbarr = env->NewByteArray(len);
  if (jbarr == 0) error(__FILE__, __LINE__);

  // Store the data into this array. 
  env->SetByteArrayRegion(jbarr, 0, len, (jbyte *)serial);

  // Get the Class RoseImExport
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  // Get the method ID of imprt_arr 
  mid_Rose = env->GetStaticMethodID(cls_Rose, "imprt_arr", 
				    "([B)Ljava/lang/Object;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  // Call the static imprt_arr method
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jbarr);
  if (obj == 0) error(__FILE__, __LINE__);
}
  */

CcLines::CcLines(size_t size):objectData(size),canDelete(false) {
  SetDefined(true);
}

/*
This constructor takes the nested list representation
   of CcLines and recovers the underlying java object with
   help of this data. 

*/

CcLines::CcLines(const ListExpr &le):objectData(1) {
  obj = Convert_CToJava_Lines(le);
  canDelete = false;
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
This constructor takes a pointer to a java object which is
   already created. 

*/
CcLines::CcLines(const jobject jobj) : objectData(1) {
  /* Get the Class Lines */
  canDelete = false;
  obj = jobj;
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
This constructor creates an empty CcPoints object. 

*/	
CcLines::CcLines() {
  jmethodID mid;

  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(clsLines, "<init>", "()V");
  if (mid == 0) error(__FILE__,__LINE__);
  
  /* Create a Java-object Point. */
  obj = env->NewObject(clsLines, mid);
  if (obj == 0) error(__FILE__,__LINE__);
}

/* 
retrieves the nested list representation of the underlying
     java object. 

*/
bool CcLines::GetNL(ListExpr& le) {
  //SetObject(0);
  if (!obj) RestoreJavaObjectFromFLOB();
  le = Convert_JavaToC_Lines(obj);
  return true;
}

/* 
Destructor of CcLines. This destructor destroys also the 
     object inside the JVM. 

*/
  /*
    CcLines::~CcLines() {
  //if (canDelete) {
  env->DeleteLocalRef(obj);
  //Destroy();
  //}
  }
  */

/*
  ~Destroy~

  The Destroy function as known from other (non-JNI) algebras.

*/
void CcLines::Destroy(){
  //cout << "++++++++++++++++++++ called destroy of CcLines +++++++++++++++++++++++++" << endl;
  canDelete=true;
}

/* 
Returns the pointer to the proper java objet. 

*/

jobject CcLines::GetObj() {
  return obj;
}

/* 
restores the java object from FLOB
   the FLOB must exist

*/
void CcLines::RestoreFLOBFromJavaObject(){
  jmethodID mid = env->GetMethodID(clsLines,"writeToByteArray","()[B");
  if(mid == 0) error(__FILE__,__LINE__);
  
  jbyteArray jbytes = (jbyteArray) env->CallObjectMethod(obj,mid);
  if(jbytes == 0) error(__FILE__,__LINE__);
  
  int size = env->GetArrayLength(jbytes);

  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,JNI_ABORT);
  env->DeleteLocalRef(jbytes);
 }

/* 
creates the content of a FLOB from the given Java-Object

*/
void CcLines::RestoreJavaObjectFromFLOB(){
  if (obj) return;
  
  if(&objectData == 0){
    return;
  }
  int size = objectData.Size();
  
  cout << "size of CcLines = " << size << endl;

  char *bytes = new char[size];
  objectData.Get(0,size,bytes);
  // copy the data into a java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);
  jmethodID mid = env->GetStaticMethodID(clsLines,"readFrom","([B)LLines;");
  if(mid == 0){
    error(__FILE__,__LINE__);
  }
  obj = env->CallStaticObjectMethod(clsLines,mid,jbytes);
  if(obj == 0){
    error(__FILE__,__LINE__);
  }
  //obj = jres;
  jbyte* elems = env->GetByteArrayElements(jbytes,0);
  env->ReleaseByteArrayElements(jbytes,elems,JNI_ABORT);
  delete [] bytes;
  bytes = NULL;
  env->DeleteLocalRef(jbytes);
}


void CcLines::Print() {
  jmethodID mid = env->GetMethodID(clsLines,"print","()V");
  env->CallVoidMethod(obj,mid);
}


/*
3.2 List Representation

The list representation of a CcLines object is

----	(segment1 segment2 ... segmentn)
----

whereas the (internal) list representation of a segment is

----	(x1 y1 x2 y2)
----

whereas x1, y1, x2 and y2 are (internal) list representations
of rational numbers

----	(x1 y1 x2 y2)
----

whereas the (internal) list representation of a rational number is

----	(rat <sign> <intPart> <numDecimal> / <denomDecimal>)
----


3.3 ~In~ and ~Out~ Functions for CcLines

*/

static ListExpr OutCcLines( ListExpr typeInfo, Word value ) {
  CcLines* cclines = (CcLines*)(value.addr);
  ListExpr le;

  if (cclines->GetNL(le) == false) error(__FILE__, __LINE__);
  return le;
}

static Word InCcLines(const ListExpr typeInfo, 
		      const ListExpr instance,
		      const int errorPos, 
		      ListExpr& errorInfo, 
		      bool& correct ) {
  CcLines* newlines;

  correct = true;
  newlines = new CcLines(instance);
  
  return SetWord(newlines);
}

/* 
Creation of a CcLines object. 

*/
static Word CreateCcLines(const ListExpr typeInfo) {
  cout << "CreateCcLines" << endl;

  CcLines* res = new CcLines((size_t)1);
  res->SetObject(0);
  return SetWord(res);
}

/* 
Deletion of a CcLines object. 

*/
static void DeleteCcLines(Word &w) {
  //cout << "++++++++++++++++++++ called Delete of CcLines +++++++++++++++++++++++++" << endl;
  delete ((CcLines *)w.addr);
  w.addr = 0;
}

/* 
Close a CcLines object. 

*/
static void CloseCcLines(Word & w) {
  //cout << "++++++++++++++++++++ called Close of CcLines +++++++++++++++++++++++++" << endl;
  delete (CcLines *)w.addr;
  w.addr = 0;
}

/* 
Clone a CcLines object. 

*/
static Word CloneCcLines(const Word &w) {
  debug(__LINE__);
  return SetWord(((CcLines *)w.addr)->Clone());
}

static void* CastCcLines( void* addr ) {
  return new (addr) CcLines;
}

bool OpenCcLines(SmiRecord& valueRecord,
                 size_t& offset,
		 const ListExpr typeInfo,
		 Word& value){
  CcLines* L = (CcLines*) TupleElement::Open(valueRecord,offset, typeInfo);
  //L->RestoreJavaObjectFromFLOB();
  L->SetObject(0);
  value = SetWord(L);
  return true;
}

bool SaveCcLines( SmiRecord& valueRecord ,
                  size_t& offset,
		  const ListExpr typeInfo,
		  Word& value){
  CcLines* L = (CcLines*) value.addr;
  TupleElement::Save(valueRecord,offset,typeInfo,L);
  return true;
}

/*
3.4 Function Describing the Signature of the Type Constructor

*/

static ListExpr LinesProperty() {
  debug(__LINE__);
  return 
    (nl->TwoElemList
     (
      nl->FiveElemList
      (nl->StringAtom("Signature"),
       nl->StringAtom("Example Type List"),
       nl->StringAtom("List Rep"),
       nl->StringAtom("Example List"),
       nl->StringAtom("Remarks")),
      nl->FiveElemList
      (nl->StringAtom("-> DATA"),
       nl->StringAtom("cclines"),
       nl->StringAtom("(<seg1> <seg2> ... <segN>)"),
       nl->StringAtom("..."),
       nl->StringAtom("missing."))));
}

/*
3.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~cclines~ does not have arguments, this is trivial.

*/

static bool CheckCcLines(ListExpr type, ListExpr& errorInfo ) {
  return (nl->IsEqual(type, "line"));
}

int
SizeOfCcLines()
{
  return sizeof(CcLines);
}


/*
3.6 Creation of the Type Constructor Instance

*/

TypeConstructor cclines (
 "line",
 // name
 LinesProperty,
 // property function describing the signature
 OutCcLines,
 // out function
 InCcLines,
 // in function
 0, 0,
 //SaveToList and RestoreFromList functions
 CreateCcLines,
 // object creation
 DeleteCcLines,
 // object deletion
 OpenCcLines,
 // object open
 SaveCcLines,
 // object save
 CloseCcLines,
 // object close
 CloneCcLines,
 // object clone
 CastCcLines,
 // cast function
 SizeOfCcLines,
 //sizeof function
 CheckCcLines,
 // kind checking function
 0,
 // predefined persistence function for model.
 TypeConstructor::DummyInModel,
 //TypeConstructor::DummyInModel, 	
 TypeConstructor::DummyOutModel,
 //TypeConstructor::DummyOutModel,
 TypeConstructor::DummyValueToModel,
 //TypeConstructor::DummyValueToModel,
 TypeConstructor::DummyValueListToModel
 //TypeConstructor::DummyValueListToModel
 );

/* 
4 Type Constructor ~Regions~

4.1 Data Structure - Class ~Regions~

The class CcRegions is just a wrapper to the 
according Regions Java class. 
The cls attribute holds a pointer to the 
Java class Lines and the obj attribute 
holds a pointer to the java object itself.
These pointers are used to invoke according 
java methods.

*/

class CcRegions: public StandardAttribute {
private:
  jobject obj;
  FLOB objectData;
  bool canDelete;
  bool Defined;
  void RestoreFLOBFromJavaObject();

public:
  /* Inherited methods of Attribute */
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined() const;
  int Sizeof();
  //void *GetValue();
  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  void Destroy();
  jobject GetObject() {
    return obj; }
  bool Adjacent(Attribute *arg);
  int NumOfFLOBs();
  FLOB *GetFLOB(const int i);
  void Initialize();
  void Finalize() {
    //cout << "++++++++++++++++++++ called Finalize of CcRegions +++++++++++++++++++++++++" << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  void RestoreJavaObjectFromFLOB();
  size_t HashValue();

  /* This contructor takes just one serial string 
     of the underlying java object and its length. */
  //CcRegions(char *serial, int len);
  /* This constructor takes the nested list representation
     of CcRegions and recovers the underlying java object with
     help of this data. */
  CcRegions(const ListExpr &le);
 /* This constructor takes a pointer to a java object which is
     already created. */
  CcRegions(const jobject jobj);
  /* This constructor creates an empty CcRegions object. */	
  CcRegions();
  CcRegions(size_t size);
 /* retrieves the nested list representation of the underlying
     java object. */
  bool GetNL(ListExpr &le);
  
  void SetObject(jobject obj) {
    this->obj = obj;
  }

  /* Destructor of CcRegions. This destructor destroys also the 
     object inside the JVM. */
  ~CcRegions() {
    //cout << "++++++++++++++++++++ called destructor of CcRegions +++++++++++++++++++++++++" << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  /* Returns the pointer to the proper java objet. */
  jobject GetObj();
  void Print();
};

/* 
Inherited method of StandardAttribute 

*/
int CcRegions::Compare(Attribute *attr) {
  jmethodID mid = env->GetMethodID(clsRegions,"compare","(LRegions;)I");
  if (mid == 0) error (__FILE__,__LINE__);
  
  CcRegions *R = (CcRegions *) attr;
  return env->CallByteMethod(obj,mid,R->obj);
}

/* 
Inherited method of StandardAttribute 

*/
Attribute *CcRegions::Clone() {
  CcRegions* res = new CcRegions(objectData.Size());
  res->CopyFrom(this);
  return res;
}

/* 
Inherited method of StandardAttribute 

*/
bool CcRegions::IsDefined() const {
  return true;
}

/* 
Inherited method of StandardAttribute 

*/
int CcRegions::Sizeof() {
  return sizeof(CcRegions);
}

/* 
Inherited method of StandardAttribute 

*/
 /*
void *CcRegions::GetValue() {
  return (void *)-1;
}
 */

/* 
Inherited method of StandardAttribute 

*/
void CcRegions::SetDefined(bool Defined) {
  this->Defined = Defined;
}

/* 
Inherited method of StandardAttribute 

*/
void CcRegions::CopyFrom(StandardAttribute* right) { 
  cout << "CcRegions::CopyFrom." << endl;
  CcRegions *R = (CcRegions *)right;
  objectData.Resize(R->objectData.Size());
  cout << "objectData.Size: " << R->objectData.Size() << endl;
  char *data = new char[R->objectData.Size()];
  R->objectData.Get(0,R->objectData.Size(),data);
  objectData.Put(0,R->objectData.Size(),data);
  delete [] data;
  cout << "objectData.Size" << objectData.Size() << endl;
  //RestoreJavaObjectFromFLOB();
  //if(obj)
  //  env->DeleteLocalRef(obj);
  obj=0;
}

/* 
Inherited method of StandardAttribute 

*/
size_t CcRegions::HashValue() {
  return 0;
}

bool CcRegions::Adjacent(Attribute * arg) {
  return false;
}

int CcRegions::NumOfFLOBs(){
  return 1;
}

FLOB *CcRegions::GetFLOB(const int i){
  assert(i==0);
  return &objectData;
}

void CcRegions::Initialize() {
  //RestoreJavaObjectFromFLOB();
  obj = 0;
  Defined=true;
}


/* 
This contructor takes just one serial string 
   of the underlying java object and its length. 

*/
 /*
CcRegions::CcRegions(char *serial, int len) {
  debug(__LINE__);
  jbyteArray jbarr = 0;
  jmethodID mid_Rose;
  jclass cls_Rose;

  // Get the Class
  cls = env->FindClass("Regions");
  if (cls == 0) error(__FILE__, __LINE__);

  // Allocate a new byte array inside the VM.
  jbarr = env->NewByteArray(len);
  if (jbarr == 0) error(__FILE__, __LINE__);

  // Store the data into this array.
  env->SetByteArrayRegion(jbarr, 0, len, (jbyte *)serial);

  // Get the Class RoseImExport
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  // Get the method ID of imprt_arr
  mid_Rose = env->GetStaticMethodID(cls_Rose, "imprt_arr", 
				    "([B)Ljava/lang/Object;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  // Call the static imprt_arr method
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jbarr);
  if (obj == 0) error(__FILE__, __LINE__);  
}
 */

CcRegions::CcRegions(size_t size):objectData(size),canDelete(false) {
  SetDefined(true);
}

/* 
This constructor takes the nested list representation
   of CcRegions and recovers the underlying java object with
   help of this data. 

*/

CcRegions::CcRegions(const ListExpr &le):objectData(1) {
  obj = Convert_CToJava_Regions(le);
  canDelete = false;
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
This constructor takes a pointer to a java object which is
   already created. 

*/
CcRegions::CcRegions(const jobject jobj):objectData(1) {
  /* Get the Class RoseImExport */
  //cout << "CcRegions::constructor(jobject) called" << endl;
  //Application::PrintStacktrace();
  canDelete = false;
  obj = jobj;
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
This constructor creates an empty CcRegions object.

*/	
CcRegions::CcRegions() {
  jmethodID mid;

  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(clsRegions, "<init>", "()V");
  if (mid == 0) error(__FILE__,__LINE__);
  
  /* Create a Java-object Point. */
  obj = env->NewObject(clsRegions, mid);
  if (obj == 0) error(__FILE__,__LINE__);
}

/* 
retrieves the nested list representation of the underlying
     java object. 

*/
bool CcRegions::GetNL(ListExpr& le) {
  if (!Defined) {
    le = nl->TheEmptyList();
    return true;
  }
  if (!obj) RestoreJavaObjectFromFLOB();
  le = Convert_JavaToC_Regions(obj);
  return true;
}

/* 
Destructor of CcRegions. This destructor destroys also the 
   object inside the JVM. 

*/
  /*
    CcRegions::~CcRegions() {
    cout << "++++++++++++++++++++ called destructor of CcRegions +++++++++++++++++++++++++" << endl;
    //if (canDelete) {
    env->DeleteLocalRef(obj);
    env->DeleteLocalRef(cls);
    //Destroy();
    //}
    }
  */

/*
  ~Destroy~

  The Destroy function as kinown from other (non-JNI) algebras.

*/
void CcRegions::Destroy(){
  //cout << "++++++++++++++++++++ called destroy of CcRegions +++++++++++++++++++++++++" << endl;
  canDelete=true;
}

/* 
Returns the pointer to the proper java object. 

*/
jobject CcRegions::GetObj() {
  return obj;
}

/* 
restores the java object from FLOB
   the FLOB must exists

*/
void CcRegions::RestoreFLOBFromJavaObject(){
  //cout << "RestoreFLOBFromJavaObject called" << endl;
  jmethodID mid = env->GetMethodID(clsRegions,"writeToByteArray","()[B");
  if(mid == 0) error(__FILE__,__LINE__);
  
  jbyteArray jbytes = (jbyteArray) env->CallObjectMethod(obj,mid);
  if(jbytes == 0) error(__FILE__,__LINE__);
  
  int size = env->GetArrayLength(jbytes);
  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,JNI_ABORT);
  env->DeleteLocalRef(jbytes);
}

/* 
creates the content of a FLOB from the given Java-Object

*/
void CcRegions::RestoreJavaObjectFromFLOB(){
  if (obj) return;
  // read the data from flob

  if(&objectData == 0){
    return;
  }
  int size = objectData.Size();
  cout << "going to read " << size << " bytes" << endl;
  char *bytes = new char[size];
  objectData.Get(0,size,bytes);
  // copy the data into a java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);
  jmethodID mid = env->GetStaticMethodID(clsRegions,"readFrom","([B)LRegions;");
  if(mid == 0){
    error(__FILE__,__LINE__);
  }
  obj = env->CallStaticObjectMethod(clsRegions,mid,jbytes);
  if(obj == 0){
    error(__FILE__,__LINE__);
  }
  //obj = jres;
  jbyte* elems = env->GetByteArrayElements(jbytes,0);
  env->ReleaseByteArrayElements(jbytes,elems,JNI_ABORT);
  delete [] bytes;
  bytes = NULL;
  env->DeleteLocalRef(jbytes);
  }

void CcRegions::Print() {
  jmethodID mid = env->GetMethodID(clsRegions,"print","()V");
  env->CallVoidMethod(obj,mid);
}

/*
4.2 List Representation

The list representation of a CcRegions object is

----	(face1 face2 ... facen)
----

whereas the (internal) list representation of a face is 

----	(outer[_]cycle hole[_]cycle1 hole[_]cycle2 ... hole[_]cyclem)
----

whereas the (internal) list representation of a cycle 
(outer[_]cycle or hole[_]cycle) is  

----	(vertex1 vertex2 ... vertexk)
----

whereas the (internal) list representation of a vertex is 
the same as for a point:

----	(xCoord yCoord)
----

whereas the (internal) list representation of a coordinate is

----	(rat <sign> <intPart> <numDecimal> / <denomDecimal>)
----


4.3 ~In~ and ~Out~ Functions for CcRegions

*/

static ListExpr OutCcRegions( ListExpr typeInfo, Word value ) {
  CcRegions* ccregions = (CcRegions*)(value.addr);
  ListExpr le;

  if (ccregions->GetNL(le) == false) error(__FILE__, __LINE__);

  return le;
}


static Word InCcRegions(const ListExpr typeInfo, 
			const ListExpr instance,
			const int errorPos, 
			ListExpr& errorInfo, 
			bool& correct ) {
  CcRegions* newregions;
  correct = true;
  newregions = new CcRegions(instance);
  
  return SetWord(newregions);
}

/* 
Creation of a CcRegions object. 

*/
static Word CreateCcRegions(const ListExpr typeInfo) {
  CcRegions* res = new CcRegions((size_t)1);
  res->SetObject(0);
  return SetWord(res);
}

/* 
Deletion of a CcRegions object. 

*/
static void DeleteCcRegions(Word &w) {
  //cout << "++++++++++++++++++++ called Delete of CcRegions +++++++++++++++++++++++++" << endl; 
  delete ((CcRegions *)w.addr);
  w.addr = 0;
}

/* 
Close a CcRegions object. 

*/
static void CloseCcRegions(Word & w) {
  //cout << "++++++++++++++++++++ called Close of CcRegions +++++++++++++++++++++++++" << endl;
  delete (CcRegions *)w.addr;
  w.addr = 0;
}

/* 
Clone a CcRegions object. 

*/
static Word CloneCcRegions(const Word &w) {
  debug(__LINE__);
  return SetWord(((CcRegions *)w.addr)->Clone());
}

static void* CastCcRegions( void* addr ){
  return new (addr) CcRegions;
}

bool OpenCcRegions(SmiRecord& valueRecord,
                   size_t& offset,
		   const ListExpr typeInfo,
		   Word& value){
  CcRegions* R = (CcRegions*) TupleElement::Open(valueRecord,offset, typeInfo);
  R->SetObject(0);
  value = SetWord(R);
  return true;
}

bool SaveCcRegions( SmiRecord& valueRecord,
                    size_t& offset,
		    const ListExpr typeInfo,
		    Word& value) {
  CcRegions* R = (CcRegions*) value.addr;
  TupleElement::Save(valueRecord,offset,typeInfo,R);
  return true;
}


/*
4.4 Function Describing the Signature of the Type Constructor

*/

static ListExpr RegionsProperty() {
  debug(__LINE__);
  return 
    (nl->TwoElemList
     (
      nl->FiveElemList
      (nl->StringAtom("Signature"),
       nl->StringAtom("Example Type List"),
       nl->StringAtom("List Rep"),
       nl->StringAtom("Example List"),
       nl->StringAtom("Remarks")),
      nl->FiveElemList
      (nl->StringAtom("-> DATA"),
       nl->StringAtom("ccregions"),
       nl->StringAtom("(<outer_cycle> <hole_cycle1>... <hole cycleN>)"),
       nl->StringAtom("..."),
       nl->StringAtom("missing."))));
}


/*

4.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~cclines~ does not have arguments, this is trivial.

*/

static bool CheckCcRegions(ListExpr type, ListExpr& errorInfo ) {
  return (nl->IsEqual(type, "region"));
}

int
SizeOfCcRegions()
{
  return sizeof(CcRegions);
}


/*
4.6 Creation of the Type Constructor Instance

*/

TypeConstructor ccregions (
 "region",
 // name
 RegionsProperty,
 // property function describing the signature
 OutCcRegions,
 // out function
 InCcRegions,
 // in function
 0, 0,
 //SaveToList and RestoreFromList functions
 CreateCcRegions,
 // object creation
 DeleteCcRegions,
 // object deletion
 OpenCcRegions,
 // object open
 SaveCcRegions,
 // object save
 CloseCcRegions,
 // object close
 CloneCcRegions,
 // object clone
 CastCcRegions,
 // cast function
 SizeOfCcRegions,
 //sizeof function
 CheckCcRegions,
 // kind checking function
 0,
 // predefined persistence function for model.
 TypeConstructor::DummyInModel,
 //TypeConstructor::DummyInModel, 	
 TypeConstructor::DummyOutModel,
 //TypeConstructor::DummyOutModel,
 TypeConstructor::DummyValueToModel,
 //TypeConstructor::DummyValueToModel,
 TypeConstructor::DummyValueListToModel
 //TypeConstructor::DummyValueListToModel
 );
 
/*
5 Creating Operators

5.1 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

/* 
This is a general type mapping function for all Rose methods
   which take two parameters. 

*/
static ListExpr typeMappingRose
(ListExpr args, char *type1, char *type2, char *resulttype) {
  ListExpr arg1, arg2;

  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, type1) 
	 && nl->IsEqual(arg2, type2))  
      return nl->SymbolAtom(resulttype);
  }
  return nl->SymbolAtom("typeerror");
}

/* 
This is a general type mapping function for all Rose methods
   which take one parameter. 

*/
static ListExpr typeMappingRose
(ListExpr args, char *type1, char *resulttype) {
  ListExpr arg1;

  if (nl->ListLength(args) == 1) {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, type1))
      return nl->SymbolAtom(resulttype);
  }
  return nl->SymbolAtom("typeerror");
}

/* 
type mapping function: ccpoints x ccpoints -> bool 

*/
static ListExpr ccpointsccpointsBool(ListExpr args) {
  return typeMappingRose(args, "points", "points", "bool");
}

/* 
type mapping function: cclines x cclines -> bool 

*/
static ListExpr cclinescclinesBool(ListExpr args) {
  return typeMappingRose(args, "line", "line", "bool");
}

/* 
type mapping function: ccregions x ccregions -> bool 

*/
static ListExpr ccregionsccregionsBool(ListExpr args) {
  return typeMappingRose(args, "region", "region", "bool");
}

/* 
type mapping function: ccpoints x ccregions -> bool 

*/
static ListExpr ccpointsccregionsBool(ListExpr args) {
  return typeMappingRose(args, "points", "region", "bool");
}

/* 
type mapping function: cclines x ccregions -> bool 

*/
static ListExpr cclinesccregionsBool(ListExpr args) {
  return typeMappingRose(args, "line", "region", "bool");
}

/* 
type mapping function: ccregions x cclines -> bool 

*/
static ListExpr ccregionscclinesBool(ListExpr args) {
  return typeMappingRose(args, "region", "line", "bool");
}

/* 
type mapping function: ccregions x ccregions -> ccregions 

*/
static ListExpr ccregionsccregionsccregions(ListExpr args) {
  return typeMappingRose(args, "region", "region", "region");
}

/* 
type mapping function: cclines x cclines -> cclines 

*/
static ListExpr ccregionscclinescclines(ListExpr args) {
  return typeMappingRose(args, "region", "line", "line");
}

/* 
type mapping function: ccregions x ccregions -> cclines 

*/
static ListExpr ccregionsccregionscclines(ListExpr args) {
  return typeMappingRose(args, "region", "region", "line");
}

/* 
type mapping function: cclines x cclines -> cclines 

*/
static ListExpr cclinescclinescclines(ListExpr args) {
  return typeMappingRose(args, "line", "line", "line");
}

/* 
type mapping function: cclines x ccregions -> cclines 

*/
static ListExpr cclinesccregionscclines(ListExpr args) {
  return typeMappingRose(args, "line", "region", "line");
}

/* 
type mapping function: cclines x cclines -> ccpoints 

*/
static ListExpr cclinescclinesccpoints(ListExpr args) {
  return typeMappingRose(args, "line", "line", "points");
}

/* 
type mapping function: ccpoints x cclines -> bool 

*/
static ListExpr ccpointscclinesBool(ListExpr args) {
  return typeMappingRose(args, "points", "line", "bool");
}

/* 
type mapping function: ccpoints x ccpoints -> ccpoints 

*/
static ListExpr ccpointsccpointsccpoints(ListExpr args) {
  return typeMappingRose(args, "points", "points", "points");
}

/* 
type mapping function: cclines -> ccpoints 

*/
static ListExpr cclinesccpoints(ListExpr args) {
  return typeMappingRose(args, "line", "points");
}

/* 
type mapping function: cclines -> ccregions 

*/
static ListExpr cclinesccregions(ListExpr args) {
  return typeMappingRose(args, "line", "region");
}

/* 
type mapping function: ccregions -> ccpoints 

*/
static ListExpr ccregionsccpoints(ListExpr args) {
  return typeMappingRose(args, "region", "points");
}

/* 
type mapping function: ccregions -> cclines 

*/
static ListExpr ccregionscclines(ListExpr args) {
  return typeMappingRose(args, "region", "line");
}

/* 
type mapping function: ccpoints -> int 

*/
static ListExpr ccpointsInt(ListExpr args) {
  return typeMappingRose(args, "points", "int");
}

/* 
type mapping function: cclines -> int 

*/
static ListExpr cclinesInt(ListExpr args) { 
  return typeMappingRose(args, "line", "int");
}

/* 
type mapping function: ccregions -> int 

*/
static ListExpr ccregionsInt(ListExpr args) { 
  return typeMappingRose(args, "region", "int");
}

/* 
type mapping function: ccpoints x ccpoints -> real 

*/
static ListExpr ccpointsccpointsReal(ListExpr args) { 
  return typeMappingRose(args, "points", "points", "real");
}

/* 
type mapping function: ccpoints x cclines -> real 

*/
static ListExpr ccpointscclinesReal(ListExpr args) { 
  return typeMappingRose(args, "points", "line", "real");
}

/* 
type mapping function: ccpoints x ccregions -> real 

*/
static ListExpr ccpointsccregionsReal(ListExpr args) { 
  return typeMappingRose(args, "points", "region", "real");
}

/* 
type mapping function: cclines x ccpoints -> real 

*/
static ListExpr cclinesccpointsReal(ListExpr args) { 
  return typeMappingRose(args, "line", "points", "real");
} 
/* 
type mapping function: cclines x cclines -> real 

*/
static ListExpr cclinescclinesReal(ListExpr args) { 
  return typeMappingRose(args, "line", "line", "real");
}

/* 
type mapping function: cclines x ccregions -> real 

*/
static ListExpr cclinesccregionsReal(ListExpr args) { 
  return typeMappingRose(args, "line", "region", "real");
}

/* 
type mapping function: ccregions x ccpoints -> real 

*/
static ListExpr ccregionsccpointsReal(ListExpr args) { 
  return typeMappingRose(args, "region", "points", "real");
}

/* 
type mapping function: ccregions x cclines -> real 

*/
static ListExpr ccregionscclinesReal(ListExpr args) { 
  return typeMappingRose(args, "region", "line", "real");
}

/* 
type mapping function: ccregions x ccregions -> real 

*/
static ListExpr ccregionsccregionsReal(ListExpr args) { 
  debug(__LINE__);
  return typeMappingRose(args, "region", "region", "real");
}

/* 
type mapping function: ccpoints -> real 

*/
static ListExpr ccpointsReal(ListExpr args) {
  debug(__LINE__);
  return typeMappingRose(args, "points", "real");
}

/* 
type mapping function: cclines -> real 

*/
static ListExpr cclinesReal(ListExpr args) {
  debug(__LINE__);
  return typeMappingRose(args, "line", "real");
}

/* 
type mapping function: ccregions -> real 

*/
static ListExpr ccregionsReal(ListExpr args) {
  debug(__LINE__);
  return typeMappingRose(args, "region", "real");
}

/* 
type mapping function: ccregions -> real 

*/
static ListExpr ccregionsDouble(ListExpr args) {
  debug(__LINE__);
  return typeMappingRose(args, "region", "real");
}

/* 
type mapping function: cclines -> real 

*/
static ListExpr cclinesDouble(ListExpr args) {
  debug(__LINE__);
  return typeMappingRose(args, "line", "real");
}

/*
5.2 Selection Function

Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/

static int simpleSelect (ListExpr args ){
  return 0; 
}

/*
5.3 Value Mapping Functions. 

*/

/*
5.3.1 General Wrapper functions for the Java Methods. These methods 
invoke specific Java methods with the given name and signature.

*/

/*
5.3.1.1 Template Wrapper functions

*/

/* 
this is a template function which calls a method of the
Java class ROSEAlgebra with two parameters of type CcPoints,
CcLines or CcRegions and returns a bool value. The signature
must correspond to both types and return type.

*/
template <class Type1, class Type2>
static bool callBooleanJMethod(char *name, Type1 *t1, Type2 *t2, char *signature) {
  /* Get the method ID of the java function */
  jmethodID mid_ROSE = env->GetStaticMethodID(clsROSEAlgebra, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  bool result = env->CallStaticBooleanMethod(clsROSEAlgebra, mid_ROSE, t1->GetObj(), t2->GetObj());
  return result;
}

/* 
this is a template function which calls a method of the
Java class ROSEAlgebra with one parameter of type CcPoints,
CcLines or CcRegions and returns an integer value. The signature
must correspond to the type and return type.

*/
template <class Type1>
static int callIntegerJMethod(char *name, Type1 *t1, char *signature) {
  int result;

  /* Get the method ID of the java function */
  jmethodID mid_ROSE = env->GetStaticMethodID(clsROSEAlgebra, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  result = env->CallStaticIntMethod(clsROSEAlgebra, mid_ROSE, t1->GetObj());
  return result;
}

/* 
this is a template function which calls a method of the
Java class ROSEAlgebra with two parameters of type CcPoints,
CcLines or CcRegions and returns a Rational value. This value
is transformed into a double value. The signature
must correspond to the type and return type.

*/
template <class Type1, class Type2>
static double callRationalJMethod(char *name, Type1 *t1, Type2 *t2, 
				  char *signature) {

  /* Get the method ID of the java function */
  jmethodID mid_ROSE = env->GetStaticMethodID(clsROSEAlgebra, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  jobject result =  env->CallStaticObjectMethod(clsROSEAlgebra, mid_ROSE, t1->GetObj(), t2->GetObj());
  if (result == 0) error (__FILE__, __LINE__);

  /* Get the method ID of the java function. */
  jmethodID mid_Rational1 = env->GetMethodID(clsRational, "getNumerator", "()I");
  if (mid_Rational1 == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function. */
  jmethodID mid_Rational2 = env->GetMethodID(clsRational, "getDenominator", "()I");
  if (mid_Rational2 == 0) error(__FILE__, name, __LINE__);

  /* Calculate the numerator and denominator of the result. */
  int numerator = env->CallIntMethod(result, mid_Rational1);
  int denominator = env->CallIntMethod(result, mid_Rational2);
  
  return (double)numerator / (double)denominator;
}

/* 
this is a template function which calls a method of the
Java class ROSEAlgebra with one parameter of type CcPoints,
CcLines or CcRegions and returns a Rational value. This value
is transformed into a double value. The signature
must correspond to the type and return type.

*/
template <class Type1>
static double callRationalJMethod(char *name, Type1 *t1, char *signature) {
  debug(__LINE__);
  /* Get the method ID of the java function */
  jmethodID mid_ROSE = env->GetStaticMethodID(clsROSEAlgebra, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  jobject result =  env->CallStaticObjectMethod(clsROSEAlgebra, mid_ROSE, t1->GetObj());
  if (result == 0) error (__FILE__, name, __LINE__);

  /* Get the method ID of the java function. */
  jmethodID mid_Rational1 = env->GetMethodID(clsRational, "getNumerator", "()I");
  if (mid_Rational1 == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function. */
  jmethodID mid_Rational2 = env->GetMethodID(clsRational, "getDenominator", "()I");
  if (mid_Rational2 == 0) error(__FILE__, name, __LINE__);

  /* Calculate the numerator and denominator of the result */
  int numerator = env->CallIntMethod(result, mid_Rational1);
  int denominator = env->CallIntMethod(result, mid_Rational2);
  
  return (double)numerator / (double)denominator;
}

/* 
this is a template function which calls a method of the
Java class ROSEAlgebra with a parameter of type CcPoints,
CcLines or CcRegions and returns a double value. The signature
must correspond to the type and return type.

*/
template <class Type1>
static double callDoubleJMethod(char *name, Type1 *t1, char *signature) {
  debug(__LINE__);

  /* Get the method ID of the java function */
  jmethodID mid_ROSE = env->GetStaticMethodID(clsROSEAlgebra, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  double result = (double)env->CallStaticDoubleMethod(clsROSEAlgebra, mid_ROSE, t1->GetObj());

  return result;
}


/*
 this is a template function which calls a method of the
Java class ROSEAlgebra with two parameters of type CcPoints,
CcLines or CcRegions and returns another Rose type, like 
CcPoints, CcLines or CcRegions. The signature
must correspond two both types and return type.

*/
template <class Type1, class Type2, class ReturnType>
static ReturnType *callObjectJMethod
(char *name, Type1 *t1, Type2 *t2, char *signature) {

  /* Get the method ID of the java function */
  jmethodID mid_ROSE = env->GetStaticMethodID(clsROSEAlgebra, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  if(env->ExceptionOccurred())
    env->ExceptionDescribe();
  jobject result = env->CallStaticObjectMethod(clsROSEAlgebra, mid_ROSE, t1->GetObj(), t2->GetObj());
  if (result == 0) error (__FILE__, name, __LINE__);

  return new ReturnType(result);
}

/* 
this is a template function which calls a method of the
Java class ROSEAlgebra with one parameter of type CcPoints,
CcLines or CcRegions and returns another Rose type, like 
CcPoints, CcLines or CcRegions. The signature
must correspond two both types and return type.

*/
template <class Type1, class ReturnType>
static ReturnType *callObjectJMethod
(char *name, Type1 *t1, char *signature) {

  /* Get the method ID of the java function */
  jmethodID mid_ROSE = env->GetStaticMethodID(clsROSEAlgebra, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  jobject result = env->CallStaticObjectMethod(clsROSEAlgebra, mid_ROSE, t1->GetObj());
  if (result == 0) error (__FILE__, name, __LINE__);

  return new ReturnType(result);
}

/*

5.3.1.2 General Wrapper functions for the Java Methods. 

These methods 
invoke specific Java methods with the given name and signature with 
help of above template functions. These functions are provided to make
the algebra more clearly arranged.
The names of these functions all begin with the prefix callJMethod[_] 
followed by two or three letters in their suffix which stand for the
types.
If a suffix consists of three letters the according Java method
takes two parameters. The first two letters correspond to the type
of these parameters whereby the last letter correspond to the 
return type.
If a suffix consists of two letters the according Java method 
takes one paramter only. The first letter corresponds to the type
of this parameter whereby the second (the last) letter correspond
to the return type.

The letters have the following meaning:

- P  Points
- L  Lines
- R  Regions
- B  boolean
- I  integer
- D  double (converted into ccreal)
- d  double (the result of the java function is Rational - conv. into ccreal)

*/


/* 
Call a Java method with the given name which takes
   two CcPoints and returns a boolean value. 

*/
static bool callJMethod_PPB
(char *name, CcPoints *ccp1, CcPoints *ccp2) {
  return callBooleanJMethod<CcPoints, CcPoints>
    (name, ccp1, ccp2, "(LPoints;LPoints;)Z");
}

/* 
Call a Java method with the given name which takes
   two CcLines and returns a boolean value. 

*/
static bool callJMethod_LLB
(char *name, CcLines *ccl1, CcLines *ccl2) {
  return callBooleanJMethod<CcLines, CcLines>
    (name, ccl1, ccl2, "(LLines;LLines;)Z");
}

/*
 Call a Java method with the given name which takes
   two CcRegions and returns a boolean value. 

*/
static bool callJMethod_RRB
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return callBooleanJMethod<CcRegions, CcRegions>
    (name, ccr1, ccr2, "(LRegions;LRegions;)Z");
}


/*
 Call a Java method with the given name which takes
   a CcPoints and a CcRegions object and returns a boolean value. 

*/
static bool callJMethod_PRB
(char *name, CcPoints *ccp, CcRegions *ccr) {
  return callBooleanJMethod<CcPoints, CcRegions>
    (name, ccp, ccr, "(LPoints;LRegions;)Z");
}

/* 
Call a Java method with the given name which takes
   a CcLines and a CcRegions object and returns a boolean value. 

*/
static bool callJMethod_LRB
(char *name, CcLines *ccl, CcRegions *ccr) {
  return callBooleanJMethod<CcLines, CcRegions>
    (name, ccl, ccr, "(LLines;LRegions;)Z");
}
/*
 Call a Java method with the given name which takes
   two CcRegions and returns a CcRegions value. 

*/
static CcRegions *callJMethod_RRR
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return 
   callObjectJMethod<CcRegions, CcRegions, CcRegions>
   (name, ccr1, ccr2, "(LRegions;LRegions;)LRegions;");
}

/*
 Call a Java method with the given name which takes
   two CcRegions and returns a CcLines value. 

*/
static CcLines *callJMethod_RRL
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return
    callObjectJMethod<CcRegions, CcRegions, CcLines>
    (name, ccr1, ccr2, "(LRegions;LRegions;)LLines;");
}

/*
 Call a Java method with the given name which takes
   a CcRegions and a CcLines object and returns a boolean value. 

*/
static bool callJMethod_RLB
(char *name, CcRegions *ccr, CcLines *ccl) {
  return 
    callBooleanJMethod<CcRegions, CcLines>
    (name, ccr, ccl, "(LRegions;LLines;)Z");
}

/*
 Call a Java method with the given name which takes
   a CcPoints and a CcLines object and returns a boolean value. 

*/
static bool callJMethod_PLB
(char *name, CcPoints *ccp, CcLines *ccl) {
  return 
    callBooleanJMethod<CcPoints, CcLines>
    (name, ccp, ccl, "(LPoints;LLines;)Z");
}
/*
 Call a Java method with the given name which takes
   two CcLines and returns a CcPoints value. 

*/

static CcPoints *callJMethod_LLP
(char *name, CcLines *ccl1, CcLines *ccl2) {
  return 
    callObjectJMethod<CcLines, CcLines, CcPoints>
    (name, ccl1, ccl2, "(LLines;LLines;)LPoints;");
}

/*
 Call a Java method with the given name which takes
   a CcRegions and a CcLines object and returns a CcLines value. 

*/
static CcLines *callJMethod_RLL
(char *name, CcRegions *ccr, CcLines *ccl) {
  return 
    callObjectJMethod<CcRegions, CcLines, CcLines>
    (name, ccr, ccl, "(LRegions;LLines;)LLines;");
}

/*
 Call a Java method with the given name which takes
   two CcPoints and returns a CcPoints value. 

*/
static CcPoints *callJMethod_PPP
(char *name, CcPoints *ccp1, CcPoints *ccp2) {
  return
    callObjectJMethod<CcPoints, CcPoints, CcPoints>
    (name, ccp1, ccp2, "(LPoints;LPoints;)LPoints;");
}

/*
 Call a Java method with the given name which takes
   two CcLines and returns a CcLines value. 

*/
static CcLines *callJMethod_LLL
(char *name, CcLines *ccl1, CcLines *ccl2) {
  return 
    callObjectJMethod<CcLines, CcLines, CcLines>
    (name, ccl1, ccl2, "(LLines;LLines;)LLines;");
}

/*
 Call a Java method with the given name which takes
   a CcLines and a CcRegions object and returns a CcLines value. 

*/
static CcLines *callJMethod_LRL
(char *name, CcLines *ccl, CcRegions *ccr) {
  return 
    callObjectJMethod<CcLines, CcRegions, CcLines>
    (name, ccl, ccr, "(LLines;LRegions;)LLines;");
}

/*
 Call a Java method with the given name which takes
   a CcLines object and returns a CcLines value. 

*/
static CcPoints *callJMethod_LP
(char *name, CcLines *ccl) {
  return
    callObjectJMethod<CcLines, CcPoints>
    (name, ccl, "(LLines;)LPoints;");
}

/*
 Call a Java method with the given name which takes
   a CcLines object and returns a CcRegions value. 

*/
static CcRegions *callJMethod_LR
(char *name, CcLines *ccl) {
  return 
    callObjectJMethod<CcLines, CcRegions>
    (name, ccl, "(LLines;)LRegions;");
}

/*
 Call a Java method with the given name which takes
   a CcRegions object and returns a CcPoints value. 

*/
static CcPoints *callJMethod_RP
(char *name, CcRegions *ccr) {
  return 
    callObjectJMethod<CcRegions, CcPoints>
    (name, ccr, "(LRegions;)LPoints;");
}

/*
 Call a Java method with the given name which takes
   a CcRegions object and returns a CcLines value. 

*/
static CcLines *callJMethod_RL
(char *name, CcRegions *ccr) {
  return 
    callObjectJMethod<CcRegions, CcLines>
    (name, ccr, "(LRegions;)LLines;");
}

/*
 Call a Java method with given name which takes
   a CcPoints object and returns an Integer value.

*/
static int callJMethod_PI
(char *name, CcPoints *ccp) {
  return  
    callIntegerJMethod<CcPoints>(name, ccp, "(LPoints;)I");
}

/*
 Call a Java method with given name which takes
   a CcLines object and returns an Integer value. 

*/
static int callJMethod_LI
(char *name, CcLines *ccl) {
  return
    callIntegerJMethod<CcLines>(name, ccl, "(LLines;)I");
}

/*
 Call a Java method with given name which takes
   a CcRegions object and returns an Integer value. 

*/
static int callJMethod_RI
(char *name, CcRegions *ccr) {
  return
    callIntegerJMethod<CcRegions>(name, ccr, "(LRegions;)I");
}

/*
 Call a Java method with given name which takes
   two CcPoints and returns a double value. 

*/
static double callJMethod_PPd
(char *name, CcPoints *ccp1, CcPoints *ccp2) {
  return
    callRationalJMethod<CcPoints, CcPoints>
    (name, ccp1, ccp2, "(LPoints;LPoints;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   a CcPoints and CcLines object and returns a double value. 

*/
static double callJMethod_PLd
(char *name, CcPoints *ccp, CcLines *ccl) {
  return
    callRationalJMethod<CcPoints, CcLines>
    (name, ccp, ccl, "(LPoints;LLines;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   a CcPoints and CcRegions object and returns a double value. 

*/
static double callJMethod_PRd
(char *name, CcPoints *ccp, CcRegions *ccr) {
  return
    callRationalJMethod<CcPoints, CcRegions>
    (name, ccp, ccr, "(LPoints;LRegions;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   a CcLines and CcPoints object and returns a double value. 

*/
static double callJMethod_LPd
(char *name, CcLines *ccl, CcPoints *ccp) {
  return
    callRationalJMethod<CcLines, CcPoints>
    (name, ccl, ccp, "(LLines;LPoints;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   two CcLines and returns a double value. 

*/
static double callJMethod_LLd
(char *name, CcLines *ccl1, CcLines *ccl2) {
  clock_t time1 = clock();
  double result = 
    callRationalJMethod<CcLines, CcLines>
    (name, ccl1, ccl2, "(LLines;LLines;)Ltwodsack/util/number/Rational;");
  
  clock_t time2 = clock();
  cout << "HINWEIS: Verbrauchte Zeit in C++, callJMethod_LLd:" 
       << (time2 - time1) << endl;

  return result;
}

/*
 Call a Java method with given name which takes
   a CcLines and CcRegions object and returns a double value. 

*/
static double callJMethod_LRd
(char *name, CcLines *ccl, CcRegions *ccr) {
  return
    callRationalJMethod<CcLines, CcRegions>
    (name, ccl, ccr, "(LLines;LRegions;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   a CcRegions and CcPoints object and returns a double value. 

*/
static double callJMethod_RPd
(char *name, CcRegions *ccr, CcPoints *ccp) {
  return
    callRationalJMethod<CcRegions, CcPoints>
    (name, ccr, ccp, "(LRegions;LPoints;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   a CcRegions and CcLines object and returns a double value. 

*/
static double callJMethod_RLd
(char *name, CcRegions *ccr, CcLines *ccl) {
  return
    callRationalJMethod<CcRegions, CcLines>
    (name, ccr, ccl, "(LRegions;LLines;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   two CcRegions and returns a double value. 

*/
static double callJMethod_RRd
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return
    callRationalJMethod<CcRegions, CcRegions>
    (name, ccr1, ccr2, "(LRegions;LRegions;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   one CcPoints and returns a double value. 

*/
static double callJMethod_Pd(char *name, CcPoints *ccp) {
  debug(__LINE__);
  return
    callRationalJMethod<CcPoints>
    (name, ccp, "(LPoints;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   one CcLines and returns a double value. 

*/
static double callJMethod_Ld(char *name, CcLines *ccl) {
  debug(__LINE__);
  return
    callRationalJMethod<CcLines>
    (name, ccl, "(LLines;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   one CcRegions and returns a double value. 

*/
static double callJMethod_Rd(char *name, CcRegions *ccr) {
  debug(__LINE__);
  return
    callRationalJMethod<CcRegions>
    (name, ccr, "(LRegions;)Ltwodsack/util/number/Rational;");
}

/*
 Call a Java method with given name which takes
   a CcLines and returns a double value. 

*/
static double callJMethod_LD(char *name, CcLines *ccl) {
  debug(__LINE__);
  return
    callDoubleJMethod<CcLines>(name, ccl, "(LLines;)D");
}

/*
 Call a Java method with given name which takes
   a CcRegions and returns a double value. 

*/
static double callJMethod_RD(char *name, CcRegions *ccr) {
  debug(__LINE__);
  return
    callDoubleJMethod<CcRegions>(name, ccr, "(LRegions;)D");
}

/*
5.3.2 The proper Value Mapping Functions. 

*/

/* 
Equals predicate for two ccpoints. 

*/
static int pp_equalFun(Word* args, Word& result, int message, 
			       Word& local, Supplier s) {

  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);
  
  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_PPB
     ("pp_equal", ccp1, ccp2));

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)
  
  return 0;
}

/*
 Equals predicate for two cclines. 

*/
static int ll_equalFun(Word* args, Word& result, int message, 
				       Word& local, Supplier s) {

  CcLines* ls1 = ((CcLines *)args[0].addr);
  CcLines* ls2 = ((CcLines *)args[1].addr);

  if (!ls1->GetObject()) ls1->RestoreJavaObjectFromFLOB();
  if (!ls2->GetObject()) ls2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LLB
     ("ll_equal", ls1, ls2));

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
Equal predicate for two ccregions. 

*/
static int rr_equalFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s) {

  CcRegions* rs1 = ((CcRegions *)args[0].addr);
  CcRegions* rs2 = ((CcRegions *)args[1].addr);

  if (!rs1->GetObject()) rs1->RestoreJavaObjectFromFLOB();
  if (!rs2->GetObject()) rs2->RestoreJavaObjectFromFLOB();  
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result

  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_equal", rs1, rs2));

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/*
 pp[_]unequal predicate for two ccpoints. 

*/
static int pp_unequalFun(Word* args, Word& result, int message, 
			 Word& local, Supplier s) {

  CcPoints* ccps1 = ((CcPoints *)args[0].addr);
  CcPoints* ccps2 = ((CcPoints *)args[1].addr);
  
  if (!ccps1->GetObject()) ccps1->RestoreJavaObjectFromFLOB();
  if (!ccps2->GetObject()) ccps2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_PPB
     ("pp_unequal", ccps1, ccps2));
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]nequal predicate for two cclines. 

*/
static int ll_unequalFun(Word* args, Word& result, int message, 
			 Word& local, Supplier s) {

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  if (!ccl1 ->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2 ->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LLB
     ("ll_unequal", ccl1, ccl2));
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/*

 rr[_]unequal predicate for two ccregions 

*/
static int rr_unequalFun(Word* args, Word& result, int message, 
				       Word& local, Supplier s) {

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
	
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_unequal", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pp[_]disjoint predicate for two CcPoints 

*/
static int pp_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {

  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);
	
  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_PPB
     ("pp_disjoint", ccp1, ccp2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]disjoint predicate for two CcLines 

*/
static int ll_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LLB
     ("ll_disjoint", ccl1, ccl2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]disjoint predicate for two CcRegions 

*/
static int rr_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
	
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_disjoint", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pr[_]inside predicate for CcPoints and CcRegions 

*/
static int pr_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {

  CcPoints* ccp = ((CcPoints *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
	
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_PRB
     ("pr_inside",ccp, ccr)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
lr[_]inside predicate for CcLines and CcRegions 

*/
static int lr_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {

  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
	
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LRB
     ("lr_inside", ccl, ccr)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)
  return 0;
}

/* 
rr[_]inside predicate for two CcRegions 

*/
static int rr_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
	
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_inside",ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]area[_]disjoint predicate for two CcRegions 

*/
static int rr_area_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_area_disjoint", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]edge[_]disjoint predicate for two CcRegions 

*/
static int rr_edge_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
	
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_edge_disjoint", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]edge[_]inside predicate for two CcRegions 

*/
static int rr_edge_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_edge_inside", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]vertex[_]inside predicate for two CcRegions 

*/
static int rr_vertex_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_vertex_inside", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]intersects predicate for two CcRegions 

*/
static int rr_intersectsFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_intersects", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]meets predicate for two CcRegions 

*/
static int rr_meetsFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_meets", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]border[_]in[_]common predicate for two CcRegions 

*/
static int rr_border_in_commonFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_border_in_common", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]adjacent predicate for two CcRegions 

*/
static int rr_adjacentFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_adjacent", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]encloses predicate for two CcRegions 

*/
static int rr_enclosesFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
	
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RRB
     ("rr_encloses", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]intersection predicate for two CcRegions 

*/
static int rr_intersectionFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions *ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_RRR("rr_intersection", ccr1, ccr2);
  if(env->ExceptionOccurred())
    env->ExceptionDescribe();
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]plus predicate for two CcRegions 

*/
static int rr_plusFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  CcRegions *ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_RRR("rr_plus", ccr1, ccr2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]minus predicate for two CcRegions 

*/
static int rr_minusFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s)
{
  CcRegions *ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
	
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  jmethodID mid = env->GetStaticMethodID(clsROSEAlgebra,"rr_minus","(LRegions;LRegions;)LRegions;");
  if (mid == 0) error(__FILE__, __LINE__);

  jobject res = env->CallStaticObjectMethod(clsROSEAlgebra,mid,ccr1->GetObj(),ccr2->GetObj());
  if (res == 0) error(__FILE__, __LINE__);

  ccresult = new CcRegions(res);

  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)
  //cout << "HERE" << endl;
  //exit(1);

  return 0;
}

/* 
rr[_]common[_]border predicate for two CcRegions 

*/
static int rr_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  CcLines *ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_RRL("rr_common_border", ccr1, ccr2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
ll[_]intersects predicate for two CcLines 

*/
static int ll_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LLB
     ("ll_intersects", ccl1, ccl2));
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
lr[_]intersects predicate for CcLines and CcRegions 

*/ 
static int lr_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {
  
  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LRB
     ("lr_intersects", ccl, ccr));
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rl[_]intersects predicate for CcRegions and CcLines 

*/
static int rl_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {

  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);
  
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RLB
     ("rl_intersects", ccr, ccl));
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]meets predicate for two cclines.

*/
static int ll_meetsFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s)
{
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
	
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LLB
     ("ll_meets", ccl1, ccl2));
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)
  
  return 0;
}

/* 
lr[_]meets predicate for cclines and ccregions. 

*/
static int lr_meetsFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s)
{
  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
	
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LRB
     ("lr_meets", ccl, ccr));
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)
  
  return 0;
}

/* 
rl[_]meets predicate for ccregions and cclines 

*/
static int rl_meetsFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s)
{
  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);
	
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RLB
     ("rl_meets", ccr, ccl));
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)
  
  return 0;
}

/* 
ll[_]border[_]in[_]common predicate for two CcLines. 

*/
static int ll_border_in_commonFun(Word* args, Word& result, int message, 
				  Word& local, Supplier s)
{
  
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LLB
     ("ll_border_in_common", ccl1, ccl2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
lr[_]border[_]in[_]common predicate for CcLines and CcRegions. 

*/
static int lr_border_in_commonFun(Word* args, Word& result, int message, 
				  Word& local, Supplier s)
{
  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
  
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_LRB
     ("lr_border_in_common", ccl, ccr)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rl[_]border[_]in[_]common predicate for CcRegions and CcLines. 

*/
static int rl_border_in_commonFun(Word* args, Word& result, int message, 
				  Word& local, Supplier s)
{
  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);
  
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_RLB
     ("rl_border_in_common", ccr, ccl)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}


/* 
pl[_]on[_]border[_]of predicate for CcPoints and CcLines 

*/
static int pl_on_border_ofFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s)
{

  CcPoints* ccp = ((CcPoints *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);
  
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_PLB
     ("pl_on_border_of", ccp, ccl)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pr[_]on[_]border[_]of predicate for CcPoints and CcRegions 

*/
static int pr_on_border_ofFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s)
{
  CcPoints* ccp = ((CcPoints *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
  
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set
    (true, callJMethod_PRB
     ("pr_on_border_of", ccp, ccr)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pp[_]intersection predicate for two CcPoints.

*/
static int pp_intersectionFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s)
{
  CcPoints *ccresult;

  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);
  
  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
 
  ccresult = callJMethod_PPP("pp_intersection", ccp1, ccp2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]intersection predicate for two CcLines. 

*/
static int ll_intersectionFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s)
{
  CcPoints *ccresult;

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_LLP("ll_intersection", ccl1, ccl2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rl[_]intersection predicate for CcRegions and CcLines 

*/
static int rl_intersectionFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s)
{
  cout << "rl_intersection" << endl;
  
  CcLines *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  cout << "rl_intersection2" << endl;

  ccresult = callJMethod_RLL("rl_intersection", ccr, ccl);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  cout << "rl_intersection3" << endl;

  return 0;
}

/*
pp[_]plus predicate for two CcPoints. 

*/
static int pp_plusFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s)
{
  CcPoints *ccresult;

  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);

  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_PPP("pp_plus", ccp1, ccp2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]plus predicate for two CcPoints. 

*/
static int ll_plusFun(Word* args, Word& result, int message, 
		      Word& local, Supplier s)
{
  CcLines *ccresult;
  
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_LLL("ll_plus", ccl1, ccl2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pp[_]minus predicate for two CcPoints. 

*/
static int pp_minusFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s)
{
  CcPoints *ccp1;
  CcPoints *ccp2;
  CcPoints *ccresult;

  ccp1 = ((CcPoints *)args[0].addr);
  ccp2 = ((CcPoints *)args[1].addr);

  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_PPP("pp_minus", ccp1, ccp2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]minus predicate for two CcPoints. 

*/
static int ll_minusFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s)
{
  CcLines *ccresult;

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_LLL("ll_minus", ccl1, ccl2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]common[_]border predicate for two CcLines. 

*/
static int ll_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  CcLines *ccresult;

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_LLL("ll_common_border", ccl1, ccl2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
lr[_]common[_]border predicate for CcLines and CcRegions. 

*/
static int lr_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  CcLines *ccresult;

  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_LRL("lr_common_border", ccl, ccr);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
rl[_]common[_]border predicate for CcLines and CcRegions. 

*/
static int rl_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  CcLines *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_RLL("rl_common_border", ccr, ccl);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
l[_]vertices predicate for CcLines. 

*/
static int l_verticesFun(Word *args, Word& result, 
			 int message, Word& local, Supplier s) {
  CcPoints *ccresult;

  CcLines* ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_LP("l_vertices", ccl);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
r[_]vertices predicate for CcRegions. 

*/
static int r_verticesFun(Word *args, Word& result, 
			 int message, Word& local, Supplier s) {
  CcPoints *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_RP("r_vertices", ccr);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
l[_]interior predicate for CcLines. 

*/
static int l_interiorFun(Word *args, Word& result, 
			 int message, Word& local, Supplier s) {
  CcRegions *ccresult;

  CcLines* ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_LR("l_interior", ccl);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
r[_]contour predicate for CcRegions. 

*/
static int r_contourFun(Word *args, Word& result, 
			int message, Word& local, Supplier s) {
  CcLines *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result
  
  ccresult = callJMethod_RL("r_contour", ccr);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value
  
  return 0;
}

/* 
p[_]no[_]of[_]components predicate for CcPoints. 

*/
static int p_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  CcPoints *ccp = ((CcPoints *)args[0].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcInt *)result.addr)->Set
    (true, callJMethod_PI
     ("p_no_of_components", ccp)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
l[_]no[_]of[_]components predicate for CcLines. 

*/
static int l_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  CcLines *ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcInt *)result.addr)->Set
    (true, callJMethod_LI
     ("l_no_of_components", ccl)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
r[_]no[_]of[_]components predicate for CcPoints. 

*/
static int r_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  CcRegions *ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcInt *)result.addr)->Set
    (true, callJMethod_RI
     ("r_no_of_components", ccr)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pp[_]dist predicate for two CcPoints 

*/
static int pp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcPoints *ccp1 = ((CcPoints *)args[0].addr);
  CcPoints *ccp2 = ((CcPoints *)args[1].addr);

  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_PPd ("pp_dist", ccp1, ccp2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pl[_]dist predicate for CcPoints and CcLines 

*/
static int pl_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcPoints *ccp = ((CcPoints *)args[0].addr);
  CcLines *ccl = ((CcLines *)args[1].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_PLd
     ("pl_dist", ccp, ccl)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
pr[_]dist predicate for CcPoints and CcRegions 

*/
static int pr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcPoints *ccp = ((CcPoints *)args[0].addr);
  CcRegions *ccr = ((CcRegions *)args[1].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_PRd
     ("pr_dist", ccp, ccr)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
lp[_]dist predicate for CcLines and CcPoints 

*/
static int lp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);
  CcPoints *ccp = ((CcPoints *)args[1].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_LPd
     ("lp_dist", ccl, ccp)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
ll[_]dist predicate for two CcLines. 

*/
static int ll_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcLines *ccl1 = ((CcLines *)args[0].addr);
  CcLines *ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_LLd
     ("ll_dist", ccl1, ccl2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
lr[_]dist predicate for CcLines and CcRegions. 

*/
static int lr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);
  CcRegions *ccr = ((CcRegions *)args[1].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_LRd
     ("lr_dist", ccl, ccr)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rp[_]dist predicate for CcRegions and CcPoints. 

*/
static int rp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);
  CcPoints *ccp = ((CcPoints *)args[1].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_RPd
     ("rp_dist", ccr, ccp)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rl[_]dist predicate for CcRegions and CcLines. 

*/
static int rl_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);
  CcLines *ccl = ((CcLines *)args[1].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_RLd
     ("rl_dist", ccr, ccl)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
rr[_]dist predicate for two CcRegions. 

*/
static int rr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcRegions *ccr1 = ((CcRegions *)args[0].addr);
  CcRegions *ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_RRd("rr_dist", ccr1, ccr2)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
p[_]diameter predicate for a CcPoints. 

*/
static int p_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {

  CcPoints *ccp = ((CcPoints *)args[0].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();

  debug(__LINE__);
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_Pd
     ("p_diameter", ccp)); 

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
l[_]diameter predicate for a CcLines. 

*/
static int l_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  debug(__LINE__);
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_Ld
     ("l_diameter", ccl)); 
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
r[_]diameter predicate for a CcRegions. 

*/
static int r_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);
  
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  debug(__LINE__);
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_Rd
     ("r_diameter", ccr)); 
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
l[_]ength predicate for a CcLines. 

*/
static int l_lengthFun(Word *args, Word &result, int message,
		       Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  debug(__LINE__);
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_LD
     ("l_length", ccl)); 
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
r[_]aera predicate for a CcRegions. 

*/
static int r_areaFun(Word *args, Word &result, int message,
		     Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  debug(__LINE__);
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_RD
     ("r_area", ccr)); 
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* 
r[_]perimeter predicate for a CcRegions. 

*/
static int r_perimeterFun(Word *args, Word &result, int message,
			  Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  debug(__LINE__);
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_RD
     ("r_perimeter", ccr)); 
  
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/*

5.4 Definition of Operators

*/

const string pp_equalSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccpoints ccpoints) -> bool</text--->"
"<text>pp_equal(p1 , p2) where"
" p1, p2 are of type ccpoints"
"</text--->"
"<text>pp_equal predicate.</text--->"
"<text>pp_equal(p1,p2)</text--->"
") )";

const string ll_equalSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(cclines cclines) -> bool</text--->"
"<text>ll_equal(l1 , l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_equal predicate.</text--->"
"<text>ll_equal(l1,l2)</text--->"
") )";

const string rr_equalSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_equal(r1 , r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_equal predicate.</text--->"
"<text>rr_equal(p1,p2)</text--->"
") )";

const string pp_unequalSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccpoints ccpoints) -> bool</text--->"
"<text>pp_unequal(p1 , p2) where"
" p1, p2 are of type ccpoints"
"</text--->"
"<text>pp_unequal predicate.</text--->"
"<text>pp_unequal(p1,p2)</text--->"
") )";

const string ll_unequalSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(cclines cclines) -> bool</text--->"
"<text>ll_unequal(l1 , l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_unequal predicate.</text--->"
"<text>ll_unequal(l1,l2)</text--->"
") )";

const string rr_unequalSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_unequal(r1 , r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>pp_unequal predicate.</text--->"
"<text>pp_unequal(p1,p2)</text--->"
") )";

const string pp_disjointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccpoints ccpoints) -> bool</text--->"
"<text>pp_disjoint(p1 , p2) where"
" p1, p2 are of type ccpoints"
"</text--->"
"<text>pp_disjoint predicate.</text--->"
"<text>pp_disjoint(p1,p2)</text--->"
") )";

const string ll_disjointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(cclines cclines) -> bool</text--->"
"<text>ll_disjoint(l1 , l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_disjoint predicate.</text--->"
"<text>ll_disjoint(l1,l2)</text--->"
") )";

const string rr_disjointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_disjoint(r1 , r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_disjoint predicate.</text--->"
"<text>rr_disjoint(p1,p2)</text--->"
") )";

const string pr_insideSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccpoints ccregions) -> bool</text--->"
"<text>pr_inside(p, r) where"
" p is of type ccpoints and r is of type ccregions."
"</text--->"
"<text>pr_inside predicate.</text--->"
"<text>pr_inside(p,r)</text--->"
") )";

const string lr_insideSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(cclines ccregions) -> bool</text--->"
"<text>lr_inside(l , r) where"
" l is of type cclines and r is of type ccregions"
"</text--->"
"<text>lr_inside predicate.</text--->"
"<text>lr_inside(l,r)</text--->"
") )";

const string rr_insideSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_inside(r1 , r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_inside predicate.</text--->"
"<text>rr_inside(r1,r2)</text--->"
") )";

const string rr_area_disjointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_area_disjoint(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_area_disjoint predicate.</text--->"
"<text>rr_area_disjoint(r1,r2)</text--->"
") )";

const string rr_edge_disjointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_edge_disjoint(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_edge_disjoint predicate.</text--->"
"<text>rr_edge_disjoint(r1,r2)</text--->"
") )";

const string rr_edge_insideSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_edge_inside(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_edge_inside predicate.</text--->"
"<text>rr_edge_inside(r1,r2)</text--->"
") )";

const string rr_vertex_insideSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_vertex_inside(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_vertex_inside predicate.</text--->"
"<text>rr_vertex_inside(r1,r2)</text--->"
") )";

const string rr_intersectsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_intersects(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_intersects predicate.</text--->"
"<text>rr_intersects(r1,r2)</text--->"
") )";

const string rr_meetsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_meets(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_meets predicate.</text--->"
"<text>rr_meets(r1,r2)</text--->"
") )";

const string rr_border_in_commonSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_border_in_common(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_border_in_common predicate.</text--->"
"<text>rr_border_in_common(r1,r2)</text--->"
") )";

const string rr_adjacentSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_adjacent(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_adjacent predicate.</text--->"
"<text>rr_adjacent(r1,r2)</text--->"
") )";

const string rr_enclosesSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> bool</text--->"
"<text>rr_encloses(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_encloses predicate.</text--->"
"<text>rr_encloses(r1,r2)</text--->"
") )";

const string rr_intersectionSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> ccregions</text--->"
"<text>rr_intersection(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_intersection predicate.</text--->"
"<text>rr_intersection(r1,r2)</text--->"
") )";

const string rr_plusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> ccregions</text--->"
"<text>rr_plus(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_plus predicate.</text--->"
"<text>rr_plus(r1,r2)</text--->"
") )";

const string rr_minusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> ccregions</text--->"
"<text>rr_minus(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_minus predicate.</text--->"
"<text>rr_minus(r1,r2)</text--->"
") )";

const string rr_common_borderSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> cclines</text--->"
"<text>rr_common_border(r1, r2) where"
" r1, r2 are of type ccregions"
"</text--->"
"<text>rr_common_border predicate.</text--->"
"<text>rr_common_border(r1,r2)</text--->"
") )";

const string ll_intersectsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> bool</text--->"
"<text>ll_intersects(l1, l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_intersects predicate.</text--->"
"<text>ll_intersects(r1,r2)</text--->"
") )";

const string ll_meetsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> bool</text--->"
"<text>ll_meets(l1, l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_meets predicate.</text--->"
"<text>ll_meets(l1,l2)</text--->"
") )";

const string ll_border_in_commonSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> bool</text--->"
"<text>ll_border_in_common(l1, l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_border_in_common predicate.</text--->"
"<text>ll_border_in_common(l1,l2)</text--->"
") )";

const string ll_intersectionSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> ccpoints</text--->"
"<text>ll_intersection(l1, l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_intersection predicate.</text--->"
"<text>ll_intersection(l1,l2)</text--->"
") )";

const string ll_plusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> cclines</text--->"
"<text>ll_plus(l1, l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_plus predicate.</text--->"
"<text>ll_plus(l1,l2)</text--->"
") )";

const string ll_minusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> cclines</text--->"
"<text>ll_minus(l1, l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_minus predicate.</text--->"
"<text>ll_minus(l1,l2)</text--->"
") )";

const string ll_common_borderSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> cclines</text--->"
"<text>ll_common_border(l1, l2) where"
" l1, l2 are of type cclines"
"</text--->"
"<text>ll_common_border predicate.</text--->"
"<text>ll_common_border(l1,l2)</text--->"
") )";

const string lr_intersectsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines ccregions) -> bool</text--->"
"<text>lr_intersects(l, r) where"
" l is of type cclines and r is of type ccregions"
"</text--->"
"<text>lr_intersects predicate.</text--->"
"<text>lr_intersects(l,r)</text--->"
") )";

const string lr_meetsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines ccregions) -> bool</text--->"
"<text>lr_meets(l, r) where"
" l is of type cclines and r is of type ccregions"
"</text--->"
"<text>lr_meets predicate.</text--->"
"<text>lr_meets(l,r)</text--->"
") )";

const string lr_border_in_commonSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines ccregions) -> bool</text--->"
"<text>lr_border_in_common(l, r) where"
" l is of type cclines and r is of type cclines"
"</text--->"
"<text>lr_border_in_common predicate.</text--->"
"<text>lr_border_in_common(l, r)</text--->"
") )";

const string lr_common_borderSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines ccregions) -> cclines</text--->"
"<text>lr_common_border(l, r) where"
" l is of type cclines and r is of type ccregions."
"</text--->"
"<text>lr_common_border predicate.</text--->"
"<text>ll_common_border(l,r)</text--->"
") )";

const string rl_intersectsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions cclines) -> bool</text--->"
"<text>rl_intersects(r, l) where"
" l is of type cclines and r is of type ccregions"
"</text--->"
"<text>rl_intersects predicate.</text--->"
"<text>rl_intersects(r, l)</text--->"
") )";

const string rl_meetsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions cclines) -> bool</text--->"
"<text>rl_meets(r, l) where"
" r is of type ccregions and l is of type cclines"
"</text--->"
"<text>rl_meets predicate.</text--->"
"<text>rl_meets(r,l)</text--->"
") )";

const string rl_border_in_commonSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions cclines) -> bool</text--->"
"<text>rl_border_in_common(r, l) where"
" r is of type ccregions and l is of type cclines"
"</text--->"
"<text>rl_border_in_common predicate.</text--->"
"<text>rl_border_in_common(r, l)</text--->"
") )";

const string rl_intersectionSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions cclines) -> cclines</text--->"
"<text>rl_intersection(r, l) where"
" r is of type ccregions and l is of type cclines"
"</text--->"
"<text>rl_intersection predicate.</text--->"
"<text>rl_intersection(r, l)</text--->"
") )";

const string rl_common_borderSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions cclines) -> cclines</text--->"
"<text>rl_common_border(r, l) where"
" r is of type ccregions and l is of type cclines"
"</text--->"
"<text>rl_common_border predicate.</text--->"
"<text>rl_common_border(r, l)</text--->"
") )";

const string pl_on_border_ofSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints cclines) -> bool</text--->"
"<text>pl_on_border_ofSpec(p, l) where"
" p is of type ccpoints and l is of type cclines"
"</text--->"
"<text>pl_on_border_of predicate.</text--->"
"<text>pl_on_border_of(p, l)</text--->"
") )";

const string pr_on_border_ofSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints ccregions) -> bool</text--->"
"<text>pr_on_border_ofSpec(p, r) where"
" p is of type ccpoints and r is of type ccregions"
"</text--->"
"<text>pr_on_border_of predicate.</text--->"
"<text>pr_on_border_of(p, r)</text--->"
") )";

const string pp_intersectionSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints ccpoints) -> bool</text--->"
"<text>pp_intersection(p1, p2) where"
" p1 and p2 are of type ccpoints"
"</text--->"
"<text>pp_intersection predicate.</text--->"
"<text>pp_intersection(p1, p2)</text--->"
") )";

const string pp_plusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints ccpoints) -> ccpoints</text--->"
"<text>pp_plus(p1, p2) where"
" p1 and p2 are of type ccpoints"
"</text--->"
"<text>pp_plus predicate.</text--->"
"<text>pp_plus(p1, p2)</text--->"
") )";

const string pp_minusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints ccpoints) -> ccpoints</text--->"
"<text>pp_minus(p1, p2) where"
" p1 and p2 are of type ccpoints"
"</text--->"
"<text>pp_minus predicate.</text--->"
"<text>pp_minus(p1, p2)</text--->"
") )";

const string l_verticesSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines) -> ccpoints</text--->"
"<text>l_vertices(l) where"
" l is of type cclines"
"</text--->"
"<text>l_vertices predicate.</text--->"
"<text>l_vertices(l)</text--->"
") )";

const string r_verticesSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions) -> ccpoints</text--->"
"<text>r_vertices(r) where"
" r is of type ccregions"
"</text--->"
"<text>r_vertices predicate.</text--->"
"<text>r_vertices(r)</text--->"
") )";

const string l_interiorSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines) -> ccregions</text--->"
"<text>l_interior(l) where"
" l is of type cclines"
"</text--->"
"<text>l_interior predicate.</text--->"
"<text>l_interior(l)</text--->"
") )";

const string r_contourSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions) -> cclines</text--->"
"<text>r_contour(r) where"
" r is of type ccregions"
"</text--->"
"<text>r_contour predicate.</text--->"
"<text>r_contour(r)</text--->"
") )";

const string p_no_of_componentsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints) -> int</text--->"
"<text>p_no_of_components(p) where"
" p is of type ccpoints"
"</text--->"
"<text>p_no_of_components predicate.</text--->"
"<text>p_no_of_components(p)</text--->"
") )";

const string l_no_of_componentsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines) -> int</text--->"
"<text>l_no_of_components(l) where"
" l is of type cclines"
"</text--->"
"<text>l_no_of_components predicate.</text--->"
"<text>l_no_of_components(l)</text--->"
") )";

const string r_no_of_componentsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions) -> int</text--->"
"<text>r_no_of_components(r) where"
" r is of type ccregions"
"</text--->"
"<text>r_no_of_components predicate.</text--->"
"<text>r_no_of_components(r)</text--->"
") )";

const string pp_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints ccpoints) -> real</text--->"
"<text>pp_dist(p1, p2) where"
" p1 and p2 are of type ccpoints"
"</text--->"
"<text>pp_dist predicate.</text--->"
"<text>pp_dist(p1, p2)</text--->"
") )";

const string pl_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints cclines) -> real</text--->"
"<text>pl_dist(p, l) where"
" p is of type ccpoints and l is of type cclines"
"</text--->"
"<text>pl_dist predicate.</text--->"
"<text>pl_dist(p, l)</text--->"
") )";

const string pr_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints ccregions) -> real</text--->"
"<text>pr_dist(p, r) where"
" p is of type ccpoints and r is of type ccregions"
"</text--->"
"<text>pr_dist predicate.</text--->"
"<text>pr_dist(p, r)</text--->"
") )";

const string lp_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines ccpoints) -> real</text--->"
"<text>pp_intersection(l, p) where"
" p is of type ccpoints and l is of type cclines"
"</text--->"
"<text>lp_dist predicate.</text--->"
"<text>lp_dist(l, p)</text--->"
") )";

const string ll_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines cclines) -> real</text--->"
"<text>ll_dist(l1, l2) where"
" l1 and l2 are of type cclines"
"</text--->"
"<text>ll_dist predicate.</text--->"
"<text>ll_dist(l1, l2)</text--->"
") )";

const string lr_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines ccregions) -> real</text--->"
"<text>lr_intersection(l, r) where"
" l is of type cclines and r is of type ccregions"
"</text--->"
"<text>lr_dist predicate.</text--->"
"<text>lr_dist(l, r)</text--->"
") )";

const string rp_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccpoints) -> real</text--->"
"<text>rp_dist(r, p) where"
" r is of type ccregions and p is of type ccpoints"
"</text--->"
"<text>rp_dist predicate.</text--->"
"<text>rp_dist(r, p)</text--->"
") )";

const string rl_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions cclines) -> real</text--->"
"<text>rl_dist(r, l) where"
" r is of type ccregions and l is of type cclines"
"</text--->"
"<text>rl_dist predicate.</text--->"
"<text>rl_dist(r, l)</text--->"
") )";

const string rr_distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions ccregions) -> real</text--->"
"<text>rr_dist(r1, r2) where"
" r1 and r2 are of type ccregions"
"</text--->"
"<text>rr_dist predicate.</text--->"
"<text>rr_dist(r1, r2)</text--->"
") )";

const string p_diameterSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccpoints) -> real</text--->"
"<text>p_diameter(p) where"
" p is of type ccpoints"
"</text--->"
"<text>p_diameter predicate.</text--->"
"<text>p_diameter(p)</text--->"
") )";

const string l_diameterSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines) -> real</text--->"
"<text>l_diameter(l) where"
" l is of type cclines"
"</text--->"
"<text>l_diameter predicate.</text--->"
"<text>l_diameter(l)</text--->"
") )";

const string l_lengthSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(cclines) -> real</text--->"
"<text>l_length(l) where"
" l is of type cclines"
"</text--->"
"<text>l_length predicate.</text--->"
"<text>l_length(l)</text--->"
") )";

const string r_diameterSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions) -> real</text--->"
"<text>r_diameter(r) where"
" r is of type ccregions"
"</text--->"
"<text>r_diameter predicate.</text--->"
"<text>r_diameter(r)</text--->"
") )";

const string r_areaSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions) -> real</text--->"
"<text>r_area(r) where"
" r is of type ccregions"
"</text--->"
"<text>r_area predicate.</text--->"
"<text>r_area(r)</text--->"
") )";

const string r_perimeterSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>(ccregions) -> real</text--->"
"<text>r_perimeter(r) where"
" r is of type ccregions"
"</text--->"
"<text>r_perimeter predicate.</text--->"
"<text>r_perimeter(r)</text--->"
") )";


/*
Used to explain the signature and the meaning of the implemented operators.

*/

Operator pp_equal 
(
 "pp_equal", 			//name
 pp_equalSpec,  		//specification ....
 pp_equalFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccpointsBool		//type mapping 
 );

Operator ll_equal 
(
 "ll_equal", 			//name
 ll_equalSpec,  		//specification ....
 ll_equalFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesBool		//type mapping 
 );

Operator rr_equal 
(
 "rr_equal", 			//name
 rr_equalSpec,  		//specification ....
 rr_equalFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool		//type mapping 
 );

Operator pp_unequal 
(
 "pp_unequal", 			//name
 pp_unequalSpec,  		//specification ....
 pp_unequalFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccpointsBool		//type mapping 
 );

Operator ll_unequal 
(
 "ll_unequal", 			//name
 ll_unequalSpec,       		//specification ....
 ll_unequalFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesBool		//type mapping 
 );

Operator rr_unequal 
(
 "rr_unequal", 			//name
 rr_unequalSpec,       		//specification ....
 rr_unequalFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool		//type mapping 
 );

Operator pp_disjoint 
(
 "pp_disjoint", 		//name
 pp_disjointSpec,  		//specification ....
 pp_disjointFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccpointsBool		//type mapping 
 );

Operator ll_disjoint 
(
 "ll_disjoint", 		//name
 ll_disjointSpec,  		//specification ....
 ll_disjointFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesBool		//type mapping 
 );

Operator rr_disjoint 
(
 "rr_disjoint", 		//name
 rr_disjointSpec,  		//specification ....
 rr_disjointFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool		//type mapping 
 );

Operator pr_inside 
(
 "pr_inside", 			//name
 pr_insideSpec,  		//specification ....
 pr_insideFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccregionsBool		//type mapping 
 );

Operator lr_inside 
(
 "lr_inside", 			//name
 lr_insideSpec,		  	//specification ....
 lr_insideFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccregionsBool   	//type mapping 
 );

Operator rr_inside 
(
 "rr_inside", 			//name
 rr_insideSpec,  		//specification ....
 rr_insideFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool 	//type mapping 
 );

Operator rr_area_disjoint 
(
 "rr_area_disjoint", 		//name
 rr_area_disjointSpec,  	//specification ....
 rr_area_disjointFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool 	//type mapping 
 );

Operator rr_edge_disjoint 
(
 "rr_edge_disjoint", 		//name
 rr_edge_disjointSpec,  	//specification ....
 rr_edge_disjointFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool 	//type mapping 
 );

Operator rr_edge_inside 
(
 "rr_edge_inside", 		//name
 rr_edge_insideSpec,  		//specification ....
 rr_edge_insideFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool 	//type mapping 
 );

Operator rr_vertex_inside 
(
 "rr_vertex_inside", 		//name
 rr_vertex_insideSpec,  	//specification ....
 rr_vertex_insideFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool	 	//type mapping 
 );

Operator rr_intersects 
(
 "rr_intersects", 		//name
 rr_intersectsSpec,   	      	//specification ....
 rr_intersectsFun,	      	//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,		      	//trivial selection function 
 ccregionsccregionsBool       	//type mapping 
 );

Operator rr_meets 
(
 "rr_meets", 			//name
 rr_meetsSpec,  	      	//specification ....
 rr_meetsFun,		      	//value mapping
 Operator::DummyModel,	      	//dummy model mapping, defined in Algebra.h
 simpleSelect,		      	//trivial selection function 
 ccregionsccregionsBool       	//type mapping 
 );

Operator rr_border_in_common 
(
 "rr_border_in_common",       	//name
 rr_border_in_commonSpec,     	//specification ....
 rr_border_in_commonFun,      	//value mapping
 Operator::DummyModel,	      	//dummy model mapping, defined in Algebra.h
 simpleSelect,		      	//trivial selection function 
 ccregionsccregionsBool       	//type mapping 
 );

Operator rr_adjacent 
(
 "rr_adjacent", 		//name
 rr_adjacentSpec,  		//specification ....
 rr_adjacentFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool 	//type mapping 
 );

Operator rr_encloses 
(
 "rr_encloses", 		//name
 rr_enclosesSpec,  		//specification ....
 rr_enclosesFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsBool 	//type mapping 
 );

Operator rr_intersection 
(
 "rr_intersection", 		//name
 rr_intersectionSpec,  		//specification ....
 rr_intersectionFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsccregions 	//type mapping 
 );

Operator rr_plus 
(
 "rr_plus", 			//name
 rr_plusSpec,  			//specification ....
 rr_plusFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsccregions 	//type mapping 
 );

Operator rr_minus 
(
 "rr_minus", 			//name
 rr_minusSpec,  		//specification ....
 rr_minusFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsccregions 	//type mapping 
 );

Operator rr_common_border 
(
 "rr_common_border", 		//name
 rr_common_borderSpec,  	//specification ....
 rr_common_borderFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionscclines 	//type mapping 
 );		   

Operator ll_intersects 
(
 "ll_intersects", 		//name
 ll_intersectsSpec,  		//specification ....
 ll_intersectsFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesBool 		//type mapping 
);

Operator pl_on_border_of 
(
 "pl_on_border_of", 		//name
 pl_on_border_ofSpec,  		//specification ....
 pl_on_border_ofFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointscclinesBool 		//type mapping 
);

Operator pr_on_border_of 
(
 "pr_on_border_of", 		//name
 pr_on_border_ofSpec,  		//specification ....
 pr_on_border_ofFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccregionsBool 		//type mapping 
);

Operator pp_intersection 
(
 "pp_intersection", 		//name
 pp_intersectionSpec,  		//specification ....
 pp_intersectionFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccpointsccpoints 	//type mapping 
);

Operator pp_plus 
(
 "pp_plus", 			//name
 pp_plusSpec,  			//specification ....
 pp_plusFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccpointsccpoints 	//type mapping 
);

Operator pp_minus 
(
 "pp_minus", 			//name
 pp_minusSpec,  		//specification ....
 pp_minusFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccpointsccpoints 	//type mapping 
);

Operator l_vertices 
(
 "l_vertices", 			//name
 l_verticesSpec,  		//specification ....
 l_verticesFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccpoints 		//type mapping 
);

Operator r_vertices 
(
 "r_vertices", 			//name
 r_verticesSpec,  		//specification ....
 r_verticesFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccpoints 		//type mapping 
);

Operator rl_border_in_common 
(
 "rl_border_in_common", 	//name
 rl_border_in_commonSpec,  	//specification ....
 rl_border_in_commonFun,	//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionscclinesBool 		//type mapping 
);

Operator l_interior 
(
 "l_interior", 			//name
 l_interiorSpec,  		//specification ....
 l_interiorFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccregions 		//type mapping 
);

Operator r_contour 
(
 "r_contour", 			//name
 r_contourSpec,  		//specification ....
 r_contourFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionscclines 		//type mapping 
);

Operator rl_intersects 
(
 "rl_intersects", 		//name
 rl_intersectsSpec,  		//specification ....
 rl_intersectsFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionscclinesBool 		//type mapping 
);

Operator rl_meets 
(
 "rl_meets", 			//name
 rl_meetsSpec,  		//specification ....
 rl_meetsFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionscclinesBool 		//type mapping 
);

Operator rl_intersection 
(
 "rl_intersection", 		//name
 rl_intersectionSpec,  		//specification ....
 rl_intersectionFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionscclinescclines 	//type mapping 
);

Operator rl_common_border 
(
 "rl_common_border", 		//name
 rl_common_borderSpec,  	//specification ....
 rl_common_borderFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionscclinescclines 	//type mapping 
);

Operator lr_intersects 
(
 "lr_intersects", 		//name
 lr_intersectsSpec,  		//specification ....
 lr_intersectsFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccregionsBool 		//type mapping 
);

Operator lr_meets 
(
 "lr_meets", 			//name
 lr_meetsSpec,  		//specification ....
 lr_meetsFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccregionsBool 		//type mapping 
);

Operator lr_border_in_common 
(
 "lr_border_in_common", 	//name
 lr_border_in_commonSpec,  	//specification ....
 lr_border_in_commonFun,	//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccregionsBool 		//type mapping 
);

Operator lr_common_border 
(
 "lr_common_border", 		//name
 lr_common_borderSpec,  	//specification ....
 lr_common_borderFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccregionscclines 	//type mapping 
);

Operator ll_meets 
(
 "ll_meets", 			//name
 ll_meetsSpec,  		//specification ....
 ll_meetsFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesBool 		//type mapping 
);

Operator ll_border_in_common 
(
 "ll_border_in_common", 	//name
 ll_border_in_commonSpec,  	//specification ....
 ll_border_in_commonFun,	//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesBool 		//type mapping 
);

Operator ll_intersection 
(
 "ll_intersection", 		//name
 ll_intersectionSpec,  		//specification ....
 ll_intersectionFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesccpoints 	//type mapping 
);

Operator ll_plus 
(
 "ll_plus", 			//name
 ll_plusSpec,  			//specification ....
 ll_plusFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinescclines 		//type mapping 
);

Operator ll_minus 
(
 "ll_minus", 			//name
 ll_minusSpec,  		//specification ....
 ll_minusFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinescclines 		//type mapping 
);

Operator ll_common_border 
(
 "ll_common_border", 		//name
 ll_common_borderSpec,  	//specification ....
 ll_common_borderFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinescclines 		//type mapping 
);

Operator p_no_of_components
(
 "p_no_of_components", 		//name
 p_no_of_componentsSpec,  	//specification ....
 p_no_of_componentsFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsInt 	        	//type mapping 
 );

Operator l_no_of_components
(
 "l_no_of_components", 		//name
 l_no_of_componentsSpec,  	//specification ....
 l_no_of_componentsFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesInt 	        	//type mapping 
 );

Operator r_no_of_components
(
 "r_no_of_components", 		//name
 r_no_of_componentsSpec,  	//specification ....
 r_no_of_componentsFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsInt 	        	//type mapping 
 );

Operator pp_dist
(
 "pp_dist", 			//name
 pp_distSpec,  		        //specification ....
 pp_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccpointsReal   	//type mapping 
 );

Operator pl_dist
(
 "pl_dist", 			//name
 pl_distSpec,  		        //specification ....
 pl_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointscclinesReal   		//type mapping 
 );

Operator pr_dist
(
 "pr_dist", 			//name
 pr_distSpec,  	        	//specification ....
 pr_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsccregionsReal   	//type mapping 
 );

Operator lp_dist
(
 "lp_dist", 			//name
 lp_distSpec,  		        //specification ....
 lp_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccpointsReal   		//type mapping 
 );

Operator ll_dist
(
 "ll_dist", 			//name
 ll_distSpec,  	        	//specification ....
 ll_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinescclinesReal   		//type mapping 
 );

Operator lr_dist
(
 "lr_dist", 			//name
 lr_distSpec,  	        	//specification ....
 lr_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesccregionsReal   	//type mapping 
 );

Operator rp_dist
(
 "rp_dist", 			//name
 rp_distSpec, 	 	        //specification ....
 rp_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccpointsReal  	//type mapping 
 );

Operator rl_dist
(
 "rl_dist",	 		//name
 rl_distSpec,  		        //specification ....
 rl_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionscclinesReal   	//type mapping 
 );

Operator rr_dist
(
 "rr_dist", 			//name
 rr_distSpec,  		        //specification ....
 rr_distFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsccregionsReal   	//type mapping 
 );

Operator p_diameter
(
 "p_diameter", 			//name
 p_diameterSpec,  		//specification ....
 p_diameterFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccpointsReal           	//type mapping 
 );

Operator l_diameter
(
 "l_diameter", 			//name
 l_diameterSpec,  		//specification ....
 l_diameterFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesReal           		//type mapping 
 );

Operator r_diameter
(
 "r_diameter", 			//name
 r_diameterSpec,  		//specification ....
 r_diameterFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsReal           	//type mapping 
 );

Operator l_length
(
 "l_length",	 		//name
 l_lengthSpec,  		//specification ....
 l_lengthFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 cclinesDouble          	//type mapping 
 );

Operator r_area
(
 "r_area", 			//name
 r_areaSpec,  			//specification ....
 r_areaFun,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsDouble          	//type mapping 
 );

Operator r_perimeter
(
 "r_perimeter", 		//name
 r_perimeterSpec,  		//specification ....
 r_perimeterFun,		//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 ccregionsDouble          	//type mapping 
 );


/*
  6 Creating the Algebra
  
*/

class RoseAlgebra : public Algebra {
public:
  RoseAlgebra() : Algebra() {
    AddTypeConstructor(&ccpoints);
    AddTypeConstructor(&cclines);
    AddTypeConstructor(&ccregions);
    ccpoints.AssociateKind("DATA");
    cclines.AssociateKind("DATA");
    ccregions.AssociateKind("DATA");
    /* this means that ccpoints, cclines, ccregions 
       can be used in places where types of DATA are expected, 
       e. g. in tuples. */
    AddOperator(&pp_equal);
    AddOperator(&ll_equal);
    AddOperator(&rr_equal);
    AddOperator(&pp_unequal);
    AddOperator(&ll_unequal);
    AddOperator(&rr_unequal);
    AddOperator(&pp_disjoint);
    AddOperator(&ll_disjoint);
    AddOperator(&rr_disjoint);
    AddOperator(&pr_inside);
    AddOperator(&lr_inside);
    AddOperator(&rr_inside);
    AddOperator(&rr_area_disjoint);
    AddOperator(&rr_edge_disjoint);
    AddOperator(&rr_edge_inside);
    AddOperator(&rr_vertex_inside);
    AddOperator(&rr_intersects);
    AddOperator(&rr_intersection);
    AddOperator(&rr_meets);
    AddOperator(&rr_border_in_common);
    AddOperator(&rr_adjacent);
    AddOperator(&rr_encloses);
    AddOperator(&rr_intersection);
    AddOperator(&rr_plus);
    AddOperator(&rr_minus);
    AddOperator(&rr_common_border);
    AddOperator(&ll_intersects);
    AddOperator(&pl_on_border_of);
    AddOperator(&pr_on_border_of);
    AddOperator(&pp_intersection);
    AddOperator(&pp_plus);
    AddOperator(&pp_minus);
    AddOperator(&l_vertices);
    AddOperator(&r_vertices);
    AddOperator(&l_interior);
    AddOperator(&r_contour);
    AddOperator(&rl_intersects);
    AddOperator(&rl_meets);
    AddOperator(&rl_border_in_common);
    AddOperator(&rl_intersection);
    AddOperator(&rl_common_border);
    AddOperator(&lr_intersects);
    AddOperator(&lr_meets);
    AddOperator(&lr_border_in_common);
    AddOperator(&lr_common_border);
    AddOperator(&ll_meets);
    AddOperator(&ll_border_in_common);
    AddOperator(&ll_intersection);
    AddOperator(&ll_plus);
    AddOperator(&ll_minus);
    AddOperator(&ll_common_border);
    AddOperator(&p_no_of_components);
    AddOperator(&l_no_of_components);
    AddOperator(&r_no_of_components);
    AddOperator(&pp_dist);
    AddOperator(&pl_dist);
    AddOperator(&pr_dist);
    AddOperator(&lp_dist);
    AddOperator(&ll_dist);
    AddOperator(&lr_dist);
    AddOperator(&rp_dist);
    AddOperator(&rl_dist);
    AddOperator(&rr_dist);
    AddOperator(&p_diameter);
    AddOperator(&l_diameter);
    AddOperator(&r_diameter);
    AddOperator(&l_length);
    AddOperator(&r_area);
    AddOperator(&r_perimeter);
  }
  ~RoseAlgebra() {};
};

RoseAlgebra roseAlgebra; 

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeRoseAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  /* Get the pointers env and jvm from 
     the used Java Virtual Machine. */
  jvminit = new JVMInitializer();
  env = jvminit->getEnv();
  jvm = jvminit->getJVM();

  clsRationalFactory = env->FindClass("twodsack/util/number/RationalFactory");
  if (clsRationalFactory == 0) error(__FILE__,__LINE__);

  //set Rational class
  jmethodID midRatFac = env->GetStaticMethodID(clsRationalFactory, "setClass", "(Ljava/lang/String;)V");
  if (midRatFac == 0) error(__FILE__,__LINE__);

  jstring jstr = env->NewStringUTF("twodsack.util.number.RationalDouble");
  if (jstr == 0) error(__FILE__,__LINE__);

  env->CallStaticVoidMethod(clsRationalFactory, midRatFac, jstr);

  jmethodID midPrecision = env->GetStaticMethodID(clsRationalFactory, "setPrecision", "(Z)V");
  if (midPrecision == 0) error(__FILE__,__LINE__);

  env->CallStaticVoidMethod(clsRationalFactory, midPrecision, false);

  jmethodID midSetDeriv = env->GetStaticMethodID(clsRationalFactory, "setDerivDouble", "(D)V");
  if (midSetDeriv == 0) error(__FILE__,__LINE__);

  //set derivation value for 2DSACK package
  env->CallStaticVoidMethod(clsRationalFactory, midSetDeriv, 0.00000001);

  jmethodID midReadDeriv = env->GetStaticMethodID(clsRationalFactory, "readDerivDouble", "()D");
  if (midReadDeriv == 0) error(__FILE__,__LINE__);

  jmethodID midReadDerivN = env->GetStaticMethodID(clsRationalFactory, "readDerivDoubleNeg", "()D");
  if (midReadDerivN == 0) error(__FILE__,__LINE__);
  
  jdouble resD = env->CallStaticDoubleMethod(clsRationalFactory, midReadDeriv);
  jdouble resDN = env->CallStaticDoubleMethod(clsRationalFactory, midReadDerivN);

  cout << "2DSACK algebra: derivation values set to " << resD << "/" << resDN << endl;
  
  //read all the other classes that are needed in here
  clsPoints = env->FindClass("Points");
  if (clsPoints == 0) error(__FILE__,__LINE__);
  
  clsLines = env->FindClass("Lines");
  if (clsLines == 0) error(__FILE__,__LINE__);

  clsRegions = env->FindClass("Regions");
  if (clsRegions == 0) error(__FILE__,__LINE__);

  clsRational = env->FindClass("twodsack/util/number/Rational");
  if (clsRational == 0) error(__FILE__,__LINE__);

  clsPoint = env->FindClass("twodsack/setelement/datatype/basicdatatype/Point");
  if (clsPoint == 0) error(__FILE__,__LINE__);

  clsPointMultiSet = env->FindClass("twodsack/set/PointMultiSet");
  if (clsPointMultiSet == 0) error(__FILE__,__LINE__);

  clsSegment = env->FindClass("twodsack/setelement/datatype/basicdatatype/Segment");
  if (clsSegment == 0) error(__FILE__,__LINE__);

  clsSegMultiSet = env->FindClass("twodsack/set/SegMultiSet");
  if (clsSegMultiSet == 0) error(__FILE__,__LINE__);
  
  clsLinkedList = env->FindClass("java/util/LinkedList");
  if (clsLinkedList == 0) error(__FILE__,__LINE__);
  
  clsCycleList = env->FindClass("twodsack/util/collection/CycleList");
  if (clsCycleList == 0) error(__FILE__,__LINE__);

  clsCycleListListPoints = env->FindClass("twodsack/util/collection/CycleListListPoints");
  if (clsCycleListListPoints == 0) error(__FILE__,__LINE__);

  clsSegmentComparator = env->FindClass("twodsack/util/comparator/SegmentComparator");
  if (clsSegmentComparator == 0) error(__FILE__,__LINE__);

  clsROSEAlgebra = env->FindClass("ROSEAlgebra");
  if (clsROSEAlgebra == 0) error(__FILE__,__LINE__);

  nl = nlRef;
  qp = qpRef;

  return (&roseAlgebra);
}
