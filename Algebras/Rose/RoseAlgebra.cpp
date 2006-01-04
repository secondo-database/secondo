/*


//[_] [\_]
//[TOC] [\tableofcontents]
//[Title] [ \title{ROSEAlgebra} \author{Mirco Guenster, Dirk Ansorge} \maketitle]
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
#include "LogMsg.h"
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

 //Pointers to the JVM Environment and JVM.
static JNIEnv *env;
static JavaVM *jvm;

 //The deviation value.
static double DEVIATION_VALUE;

 //Pointers to some 2DSACK classes.
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
static jclass clsRect;

 //Pointers to some 2DSACK methods.
static jmethodID midRationalGetDouble;
static jmethodID midPMSToArray;
static jmethodID midSegmentGetStartpoint;
static jmethodID midSegmentGetEndpoint;
static jmethodID midSMSToArray;
static jmethodID midLLToArray;
static jmethodID midCLToArray;
static jmethodID midCLLPToArray;
static jmethodID midRegionsCyclesPoints;
static jmethodID midRFConstRational;
static jmethodID midPointConstII;
static jmethodID midPointConstDD;
static jmethodID midPointConstRR;
static jmethodID midPointsConst;
static jmethodID midPointsAddPoint;
static jmethodID midSegmentConstIIII;
static jmethodID midSegmentConstDDDD;
static jmethodID midSegmentConstRRRR;
static jmethodID midLinesConst;
static jmethodID midLinesAddSegment;
static jmethodID midSCConst;
static jmethodID midSMSConst;
static jmethodID midSMSAdd;
static jmethodID midSegmentConstPP;
static jmethodID midRegionsConstVoid;
static jmethodID midRegionsConstSMS;
static jmethodID midPointsCompare;
static jmethodID midPointsWriteToByteArray;
static jmethodID midPointsReadFrom;
static jmethodID midPointsPrint;
static jmethodID midPointsRect;
static jmethodID midLinesCompare;
static jmethodID midLinesWriteToByteArray;
static jmethodID midLinesReadFrom;
static jmethodID midLinesPrint;
static jmethodID midLinesRect;
static jmethodID midRegionsCompare;
static jmethodID midRegionsWriteToByteArray;
static jmethodID midRegionsReadFrom;
static jmethodID midRegionsPrint;
static jmethodID midRationalGetNumerator;
static jmethodID midRationalGetDenominator;
static jmethodID midROSESetDeviationValue;
static jmethodID midROSEChooseTriangulator;
static jmethodID midRegionsRect;
static jmethodID midRectGetTopLeftX;
static jmethodID midRectGetTopLeftY;
static jmethodID midRectGetBottomRightX;
static jmethodID midRectGetBottomRightY;

 //Pointers to field IDs of 2DSACK. 
static jfieldID fidRationalX;
static jfieldID fidRationalY;
static jfieldID fidPointsPointSet;
static jfieldID fidLinesSegmentSet;

static jclass clsRoseError;
static jfieldID fidROSEError;

 //Pointers to ROSE operations.
static jmethodID midROSEp_diameter;
static jmethodID midROSEp_no_of_components;

static jmethodID midROSEl_diameter;
static jmethodID midROSEl_interior;
static jmethodID midROSEl_length;
static jmethodID midROSEl_no_of_components;
static jmethodID midROSEl_vertices;

static jmethodID midROSEr_area;
static jmethodID midROSEr_contour;
static jmethodID midROSEr_diameter;
static jmethodID midROSEr_no_of_components;
static jmethodID midROSEr_perimeter;
static jmethodID midROSEr_vertices;

static jmethodID midROSEpp_disjoint;
static jmethodID midROSEpp_dist;
static jmethodID midROSEpp_equal;
static jmethodID midROSEpp_plus;
static jmethodID midROSEpp_minus;
static jmethodID midROSEpp_unequal;
static jmethodID midROSEpp_intersection;

static jmethodID midROSEll_border_in_common;
static jmethodID midROSEll_common_border;
static jmethodID midROSEll_disjoint;
static jmethodID midROSEll_dist;
static jmethodID midROSEll_equal;
static jmethodID midROSEll_intersection;
static jmethodID midROSEll_intersects;
static jmethodID midROSEll_meets;
static jmethodID midROSEll_minus;
static jmethodID midROSEll_plus;
static jmethodID midROSEll_unequal;

static jmethodID midROSErr_adjacent;
static jmethodID midROSErr_area_disjoint;
static jmethodID midROSErr_border_in_common;
static jmethodID midROSErr_common_border;
static jmethodID midROSErr_disjoint;
static jmethodID midROSErr_dist;
static jmethodID midROSErr_edge_disjoint;
static jmethodID midROSErr_edge_inside;
static jmethodID midROSErr_encloses;
static jmethodID midROSErr_equal;
static jmethodID midROSErr_inside;
static jmethodID midROSErr_intersection;
static jmethodID midROSErr_intersects;
static jmethodID midROSErr_meets;
static jmethodID midROSErr_minus;
static jmethodID midROSErr_plus;
static jmethodID midROSErr_unequal;
static jmethodID midROSErr_vertex_inside;

static jmethodID midROSEpl_dist;
static jmethodID midROSEpl_on_border_of;

static jmethodID midROSEpr_dist;
static jmethodID midROSEpr_inside;
static jmethodID midROSEpr_on_border_of;

static jmethodID midROSElp_dist;

static jmethodID midROSErp_dist;

static jmethodID midROSElr_border_in_common;
static jmethodID midROSElr_common_border;
static jmethodID midROSElr_dist;
static jmethodID midROSElr_inside;
static jmethodID midROSElr_intersects;
static jmethodID midROSElr_meets;

static jmethodID midROSErl_border_in_common;
static jmethodID midROSErl_common_border;
static jmethodID midROSErl_dist;
static jmethodID midROSErl_intersection;
static jmethodID midROSErl_intersects;
static jmethodID midROSErl_meets;

static const bool DEBUG = false;
static const bool DEBUG_CONVERT = false;

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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  cerr << "Error in " << name << " in line: " << line << "." << endl;
  if(env->ExceptionOccurred())
    env->ExceptionDescribe();
  exit(1);
}

static void RoseError() {
  jthrowable exec = env->ExceptionOccurred();
  env->ExceptionClear();
  const char* errMsgC = env->GetStringUTFChars((jstring)env->GetObjectField(exec,fidROSEError),0);
  cmsg.error() << errMsgC << endl;
  cmsg.send();
}


/* 
 this function prints an error message including the line
   where the error occured. This method is used for calls
   to Java methods. The name of the Java method which 
   should invoked is also printed. 

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
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jdouble value = env->CallDoubleMethod(jRational, midRationalGetDouble);

  env->DeleteLocalRef(jRational);
  jRational = 0;
  return nl->RealAtom(value);
}

/* 
 The following function takes a java object of type Point
   and returns a suitable ListExpr 

*/

static ListExpr Convert_JavaToC_Point(jobject jPoint) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jobject X = env->GetObjectField(jPoint, fidRationalX);
  if (X == 0) error(__FILE__,__LINE__);

  jobject Y = env->GetObjectField(jPoint, fidRationalY);
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
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jobject jpointlist = env->GetObjectField(jPoints, fidPointsPointSet);
  if (jpointlist == 0) error(__FILE__,__LINE__);

  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jpointlist, midPMSToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  //Determine the length of the result array. 
  int oarrlen = (int)env->GetArrayLength(oarr);

  //Collect all calculated subnodes in a stack first. 
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  // Then assemble the result.
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_Point(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
    tmp = 0;
  } 

  env->DeleteLocalRef(jpointlist);
  jpointlist = 0;
  env->DeleteLocalRef(oarr);
  oarr = 0;
  env->DeleteLocalRef(jPoints); 
  jPoints = 0;

  return result;
}

/*
 The following function takes a java object of type Segment
   and returns a suitable ListExpr 

*/

static ListExpr Convert_JavaToC_Segment(jobject jSegment) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Get the start Point itself 
  jobject startP = env->CallObjectMethod(jSegment, midSegmentGetStartpoint);
  if (startP == 0) error(__FILE__,__LINE__);

  // Get the end Point itself 
  jobject endP = env->CallObjectMethod(jSegment, midSegmentGetEndpoint);
  if (endP == 0) error(__FILE__,__LINE__);

  // Get the field x itself (start point)
  //jobject X1 = env->GetObjectField(startP, fidX1);
  jobject X1 = env->GetObjectField(startP, fidRationalX);
  if (X1 == 0) error(__FILE__,__LINE__);

  // Get the field y itself (start point) 
  //jobject Y1 = env->GetObjectField(startP, fidY1);
  jobject Y1 = env->GetObjectField(startP, fidRationalY);
  if (X1 == 0) error(__FILE__,__LINE__);

  // Get the field x itself (end point) 
  //jobject X2 = env->GetObjectField(endP, fidX2);
  jobject X2 = env->GetObjectField(endP, fidRationalX);
  if (X1 == 0) error(__FILE__,__LINE__);

  // Get the field y itself (end point) 
  //jobject Y2 = env->GetObjectField(endP, fidY2);
  jobject Y2 = env->GetObjectField(endP, fidRationalY);
  if (X1 == 0) error(__FILE__,__LINE__);

  env->DeleteLocalRef(startP);
  startP = 0;
  env->DeleteLocalRef(endP);
  endP = 0;

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
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Get the field itself
  jobject jseglist = env->GetObjectField(jLines, fidLinesSegmentSet);
  if (jseglist == 0) error(__FILE__,__LINE__);

  // Call the method itself  
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jseglist, midSMSToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  // Determine the length of the result array. 
  int oarrlen = (int)env->GetArrayLength(oarr);

  // Collect all calculated subnodes in a stack first. 
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  // Then assemble the result. 
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_Segment(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
    tmp = 0;
  } 

  env->DeleteLocalRef(oarr);
  oarr = 0;
  env->DeleteLocalRef(jseglist);
  jseglist = 0;

  return result;
}

/*
 The following function takes a java object of type ElemList
   and returns a suitable ListExpr. 

*/

static ListExpr Convert_JavaToC_ElemList(jobject jpointList) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Call the method itself  
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jpointList, midLLToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  // Determine the length of the result array. 
  int oarrlen = (int)env->GetArrayLength(oarr);

  // Collect all calculated subnodes in a stack first. 
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  //Then assemble the result. 
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_Point(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
    tmp = 0;
  } 

  env->DeleteLocalRef(oarr);
  oarr = 0;

  return result;
}

/*
 The following function takes a java object of type ElemListList
   and returns a suitable ListExpr. 

*/

static ListExpr Convert_JavaToC_ElemListList(jobject jCycleList) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Call the method itself  
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jCycleList, midCLToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  // Determine the length of the result array. 
  int oarrlen = (int)env->GetArrayLength(oarr);

  // Collect all calculated subnodes in a stack first.
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  // Then assemble the result. 
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_ElemList(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
    tmp = 0;
  } 

  env->DeleteLocalRef(oarr);
  oarr = 0;

  return result;
}



/* 
 The following function takes a java object of type ElemListListList
   and returns a suitable ListExpr. 

*/

static ListExpr Convert_JavaToC_ElemListListList(jobject jCycleListListPoints) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Call the method itself  
  jobjectArray oarr = (jobjectArray)env->CallObjectMethod(jCycleListListPoints, midCLLPToArray);
  if (oarr == 0) error(__FILE__,__LINE__);

  // Determine the length of the result array. 
  int oarrlen = (int)env->GetArrayLength(oarr);

  // Collect all calculated subnodes in a stack first. 
  stack<jobject> jstack;
  for (int i = 0; i < oarrlen; i++) {
    jstack.push(env->GetObjectArrayElement(oarr, i));
  }

  // Then assemble the result. 
  ListExpr result = nl->TheEmptyList();
  while (!jstack.empty()) {
    result = nl->Cons(Convert_JavaToC_ElemListList(jstack.top()),result);
    jobject tmp = jstack.top();
    jstack.pop();
    env->DeleteLocalRef(tmp);
    tmp = 0;
  } 

  env->DeleteLocalRef(oarr);
  oarr = 0;
  
  return result;
}

/* 
 The following function takes a java object of type Regions
   and returns a suitable ListExpr. 

*/

static ListExpr Convert_JavaToC_Regions(jobject jRegions) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  if (!jRegions) cerr << "jRegions is NULL!" << endl;
  
  // Call the method itself  
  jobject cycles = env->CallObjectMethod(jRegions, midRegionsCyclesPoints);
  if (cycles == 0) error(__FILE__,__LINE__);

  ListExpr res = Convert_JavaToC_ElemListListList(cycles);
  env->DeleteLocalRef(cycles);
  cycles = 0;
  return res;
}


/*
 The following function takes a ListExpr of an Rational object and creates
   the suitable Java object from it. 

*/
 
static jobject Convert_CToJava_Rational(const ListExpr &le) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Check whether six elements are in le. 
  if (nl->ListLength(le) != 6) error(__FILE__,__LINE__);

  // Now we calculate the necessary data for creating a Rational object.
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

  jobject result = env->CallStaticObjectMethod(clsRationalFactory, midRFConstRational, 
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
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Check whether two elements are in le. 
  if (nl->ListLength(le) != 2) error(__FILE__,__LINE__);

  // Now we calculate the necessary data for creating a Point object. 
  ListExpr e1 = nl->First(le);
  ListExpr e2 = nl->Second(le);

  if (
      (nl->IsAtom(e1)) && 
      (nl->IsAtom(e2)) && 
      (nl->AtomType(e1) == IntType) &&
      (nl->AtomType(e2) == IntType)) {
    // Both coordinates are integers 
    int intValue1 = nl->IntValue(e1);
    int intValue2 = nl->IntValue(e2);
    
    jobject result = env->NewObject(clsPoint, midPointConstII, intValue1, intValue2);
    if (result == 0) error(__FILE__,__LINE__);

    return result;
  }
  else if 
    (
     (nl->IsAtom(e1)) && 
     (nl->IsAtom(e2)) && 
     (nl->AtomType(e1) == RealType) && 
     (nl->AtomType(e2) == RealType)) {
    // Both coordinates are reals 
    //jfloat realValue1 = nl->RealValue(e1);
    jdouble realValue1 = nl->RealValue(e1);
    //jfloat realValue2 = nl->RealValue(e2);
    jdouble realValue2 = nl->RealValue(e2);
        
    jobject result = env->NewObject(clsPoint, midPointConstDD, (jfloat)realValue1, (jfloat)realValue2);
    //jobject result = env->NewObject(clsPoint, midPointConstDD, realValue1, realValue2);
    if (result == 0) error(__FILE__,__LINE__);

    return result;
  } else {
    // Both coordinates are Rationals 
    //cout << "rationals" << endl;
    jobject num1 = Convert_CToJava_Rational(e1);
    jobject num2 = Convert_CToJava_Rational(e2);

    jobject result = env->NewObject(clsPoint, midPointConstRR,num1,num2);
    env->DeleteLocalRef(num1);
    num1 = 0;
    env->DeleteLocalRef(num2);
    num2 = 0;

    return result;
  }
}

/*

 The following function takes a ListExpr of a Points object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Points(const ListExpr &le) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jobject points = env->NewObject(clsPoints, midPointsConst);
  if (points == 0) error(__FILE__,__LINE__);

  // Now we detect the length of le. 
  int ll = nl->ListLength(le);

  if (ll == 0) {
    //return empty points value
    return points;
  }
  
  bool isPoints = true;

  ListExpr first = nl->First(le);
  if (nl->IsAtom(first)) isPoints = false;
  else {
    isPoints = nl->ListLength(first) == 2;
  }

  if (isPoints) {
    // Now we insert in a for-loop all points into the Points object. 
    ListExpr restlist = le;
    for (int i = 0; i < ll; i++) {
      ListExpr first = nl->First(restlist);
      restlist = nl->Rest(restlist);
      // create a java object points 
      jobject jfirst = Convert_CToJava_Point(first);
      if (jfirst == 0) error(__FILE__,__LINE__);
      env->CallVoidMethod(points, midPointsAddPoint, jfirst);
      env->DeleteLocalRef(jfirst);
      jfirst = 0;
    }
  }
  else {
    jobject jfirst = Convert_CToJava_Point(le);
    if (jfirst == 0) error(__FILE__,__LINE__);
    env->CallVoidMethod(points, midPointsAddPoint, jfirst);
    env->DeleteLocalRef(jfirst);
    jfirst = 0;
  }
  return points;
}

/*
 The following function takes a ListExpr of a Segment object and creates
   the suitable Java object from it. 

*/

static jobject Convert_CToJava_Segment(const ListExpr &le) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // Check whether four elements are in le. 
  if (nl->ListLength(le) != 4) error(__FILE__,__LINE__);

  // Now we calculate the necessary data for creating a Point object. 
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

    // All coordinates are integers 
    int intValue1 = nl->IntValue(e1);
    int intValue2 = nl->IntValue(e2);
    int intValue3 = nl->IntValue(e3);
    int intValue4 = nl->IntValue(e4);
    
    jobject result = env->NewObject(clsSegment, midSegmentConstIIII, 
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

    // All coordinates are reals 
    jdouble realValue1 = nl->RealValue(e1);
    jdouble realValue2 = nl->RealValue(e2);
    jdouble realValue3 = nl->RealValue(e3);
    jdouble realValue4 = nl->RealValue(e4);

    jobject result = env->NewObject(clsSegment, midSegmentConstDDDD, (jfloat)realValue1, (jfloat)realValue2, (jfloat)realValue3,  (jfloat)realValue4);
    //jobject result = env->NewObject(clsSegment, midSegmentConstDDDD, realValue1, realValue2, realValue3,  realValue4);
    if (result == 0) error(__FILE__,__LINE__);

    return result;
  } else {
    jobject num1 = Convert_CToJava_Rational(e1);
    jobject num2 = Convert_CToJava_Rational(e2);
    jobject num3 = Convert_CToJava_Rational(e3);
    jobject num4 = Convert_CToJava_Rational(e4);

    jobject result = env->NewObject(clsSegment, midSegmentConstRRRR, num1,num2,num3,num4);
    env->DeleteLocalRef(num1);
    num1 = 0;
    env->DeleteLocalRef(num2);
    num2 = 0;
    env->DeleteLocalRef(num3);
    num3 = 0;
    env->DeleteLocalRef(num4);
    num4 = 0;

    return result;
  }
}

/* 
 The following function takes a ListExpr of a Lines object and creates
   the suitable Java object from it. 

*/

static jobject Convert_CToJava_Lines(const ListExpr &le) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jobject lines = env->NewObject(clsLines, midLinesConst);
  if (lines == 0) error(__FILE__,__LINE__);

  // Now we detect the length of le. 
  int ll = nl->ListLength(le);
  
  // Now we insert in a for-loop all points into the Lines object. 
  ListExpr restlist = le;
  for (int i = 0; i < ll; i++) {
    ListExpr first = nl->First(restlist);
    restlist = nl->Rest(restlist);
    // create a java object segment 
    jobject jfirst = Convert_CToJava_Segment(first);
    if (jfirst == 0) error(__FILE__,__LINE__);
    env->CallVoidMethod(lines, midLinesAddSegment, jfirst);
    env->DeleteLocalRef(jfirst);
    jfirst = 0;
  }

  return lines;
}

/*
 The following function takes a ListExpr of a Regions object and creates
   the suitable Java object from it. 

*/
static jobject Convert_CToJava_Regions(const ListExpr &le) {
  if (DEBUG_CONVERT) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  // We have to collect all segments into a segment list first 

  // Create a new SegmentComparator 
  jobject jSC = env->NewObject(clsSegmentComparator, midSCConst);
  if (jSC == 0) error(__FILE__,__LINE__);

  // Create a SMS object. 
  jobject segMS = env->NewObject(clsSegMultiSet, midSMSConst, jSC);
  if (segMS == 0) error(__FILE__,__LINE__);

 
  // Determine how many faces are in le. 
  int nllfaces = nl->ListLength(le);
  
  // Now we put all segments of the faces into the seglist. 
  ListExpr restFaceList = le;
  for (int i = 0; i < nllfaces; i++) {
    ListExpr firstFace = nl->First(restFaceList);
    restFaceList = nl->Rest(restFaceList);
    // Determine how many cycles are in firstFace. 
    int nllcycles = nl->ListLength(firstFace);

    // Now we put all segments of the cycles into the seglist. 
    ListExpr restCycleList = firstFace;
    for (int j = 0; j < nllcycles; j++) {
      ListExpr firstCycle = nl->First(restCycleList);
      restCycleList = nl->Rest(restCycleList);
      // Determine how many vertices are in firstCycle. 
      int nllvertex = nl->ListLength(firstCycle);
      // Calculate all vertices first. 
      jobject vertex[nllvertex];
      ListExpr restVertexList = firstCycle;
      for (int k = 0; k < nllvertex; k++) {
	ListExpr firstVertex = nl->First(restVertexList);
	restVertexList = nl->Rest(restVertexList);
	vertex[k] = Convert_CToJava_Point(firstVertex);
      }
      // Connect the kth and the (k+1)th Point to a segment. 
      jobject segment[nllvertex];
      for (int k = 0; k < nllvertex - 1; k++) {
	segment[k] = env->NewObject(clsSegment, midSegmentConstPP, vertex[k], vertex[k+1]);
	if (segment[k] == 0) error(__FILE__,__LINE__);
      }
	
      segment[nllvertex-1] = env->NewObject(clsSegment, midSegmentConstPP, vertex[nllvertex-1], vertex[0]);
      if (segment[nllvertex-1] == 0) error(__FILE__,__LINE__);

      //clear memory used for vertices
      for (int k = 0; k < nllvertex; k++) {
	env->DeleteLocalRef(vertex[k]);
	vertex[k] = 0;
      }

      // Now add all segments to the SegMultiSet 
      for (int k = 0; k < nllvertex; k++) {
	env->CallVoidMethod(segMS, midSMSAdd, segment[k]);
      }
      
      //clear memory used for segments
      for (int k = 0; k < nllvertex-1; k++) {
	env->DeleteLocalRef(segment[k]);
	segment[k] = 0;
      }
    }
  }
  
  jobject result = env->NewObject(clsRegions, midRegionsConstSMS, segMS);
  if (result == 0) error(__FILE__,__LINE__);

  //free memory used by Java objects
  env->DeleteLocalRef(jSC);
  jSC = 0;
  env->DeleteLocalRef(segMS);
  segMS = 0;
  
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
  void SyncBboxData();
  
public:
  // Bounding Box data 
  double BboxTopLeftX;
  double BboxTopLeftY;
  double BboxBottomRightX;
  double BboxBottomRightY;

  // Inherited methods of Attribute 
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined() const;
  int Sizeof();

  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  void Destroy();
  jobject GetObject() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    return obj; }
  bool Adjacent(Attribute *arg);
  int NumOfFLOBs();
  FLOB *GetFLOB(const int i);
  void Initialize();
  void Finalize() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  void RestoreJavaObjectFromFLOB();
  size_t HashValue();
  
 
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
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    this->obj = obj;
  }
  
  bool GetNL(ListExpr &le);
  /* Destructor of CcPoints. This destructor destroys also the 
     object inside the JVM. */
  
  ~CcPoints() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *P = (CcPoints *) attr;
  return env->CallByteMethod(obj,midPointsCompare,P->obj);
}

/* 
 Inherited method of StandardAttribute 

*/

Attribute *CcPoints::Clone() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* res = new CcPoints((size_t)objectData.Size());
  res->CopyFrom(this);
  return res;
}

/* 
 Inherited method of StandardAttribute 

*/

bool CcPoints::IsDefined() const {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return true;
}

/* 
Inherited method of StandardAttribute 

*/

int CcPoints::Sizeof() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return sizeof(CcPoints);
}

/* 
Inherited method of StandardAttribute 

*/

/* 
 Inherited method of StandardAttribute 

*/

void CcPoints::SetDefined(bool Defined) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  this->Defined = Defined;
}

/* 
 Inherited method of StandardAttribute 

*/

void CcPoints::CopyFrom(StandardAttribute* right) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *P = (CcPoints *)right;
  objectData.Resize(P->objectData.Size());
  char *data = new char[P->objectData.Size()];
  P->objectData.Get(0,P->objectData.Size(),data);
  objectData.Put(0,P->objectData.Size(),data);
  delete [] data;
  data = 0;
  BboxTopLeftX = P->BboxTopLeftX;
  BboxTopLeftY = P->BboxTopLeftY;
  BboxBottomRightX = P->BboxBottomRightX;
  BboxBottomRightY = P->BboxBottomRightY;
  obj=0;
}

/* 
 Inherited method of StandardAttribute 

*/

size_t CcPoints::HashValue() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return 0;
}

bool CcPoints::Adjacent(Attribute * arg) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return false;
}

int CcPoints::NumOfFLOBs(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return 1;
}

FLOB *CcPoints::GetFLOB(const int i){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
   assert(i==0);
   return &objectData;
}

void CcPoints::Initialize() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  obj = 0;
  Defined = true;
}


/*
 Constructors

*/

CcPoints::CcPoints(size_t size):objectData(size),canDelete(false) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  SetDefined(true);
  obj = env->NewObject(clsPoints,midPointsConst);
  if (obj == 0) error(__FILE__,__LINE__);
}

/* 
 This constructor takes the nested list representation
   of CcPoints and recovers the underlying java object with
   help of this data. 

*/
 
CcPoints::CcPoints(const ListExpr &le):objectData(1) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  obj = Convert_CToJava_Points(le);
  canDelete = false;
  SyncBboxData();
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
 This constructor takes a pointer to a java object which is
   already created. 

*/

CcPoints::CcPoints(const jobject jobj):objectData(1) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  canDelete = false;
  obj = jobj;
  SyncBboxData();
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
 This constructor creates an empty CcPoints object. 

*/	

CcPoints::CcPoints() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
}


/* 
 retrieves the nested list representation of the underlying
     java object. 

*/

bool CcPoints::GetNL(ListExpr& le) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  if (!Defined) {
    le = nl->TheEmptyList();
    return true;
  }
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
  ~Destroy~
  
  The Destroy function as known from other (non-JNI) algebras.

*/

void CcPoints::Destroy(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  canDelete=true;
}

/* 
 Returns the pointer to the proper java objet. 

*/

jobject CcPoints::GetObj() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return obj;
}


/* 
 restores the java object from FLOB
   the FLOB must exists

*/

void CcPoints::RestoreFLOBFromJavaObject(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jbyteArray jbytes = (jbyteArray) env->CallObjectMethod(obj,midPointsWriteToByteArray);
  if(jbytes == 0) error(__FILE__,__LINE__);
  
  int size = env->GetArrayLength(jbytes);
  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,JNI_ABORT);
  env->DeleteLocalRef(jbytes);
  jbytes = 0;
 }


/* 
 creates the content of a FLOB from the given Java-Object

*/

void CcPoints::RestoreJavaObjectFromFLOB(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  if (obj) return;
  // read the data from flob
  if(&objectData == 0){
    return;
  }
  int size = objectData.Size();
  char *bytes = new char[size];
  objectData.Get(0,size,bytes);
  // copy the data into a java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);
  
  obj = env->CallStaticObjectMethod(clsPoints,midPointsReadFrom,jbytes);
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


void CcPoints::Print() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //jmethodID mid = env->GetMethodID(clsPoints,"print","()V");
  env->CallVoidMethod(obj,midPointsPrint);
}


void CcPoints::SyncBboxData() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //restore object if not present
  if (!obj) RestoreJavaObjectFromFLOB();
  //get bounding box of the Java Points object
  jobject bbox = env->CallObjectMethod(obj,midPointsRect);
  if (bbox == 0) error(__FILE__,__LINE__);

  //get the coordinates of the bounding box
  BboxTopLeftX = (double)env->CallDoubleMethod(bbox,midRectGetTopLeftX);
  BboxTopLeftY = (double)env->CallDoubleMethod(bbox,midRectGetTopLeftY);
  BboxBottomRightX = (double)env->CallDoubleMethod(bbox,midRectGetBottomRightX);
  BboxBottomRightY = (double)env->CallDoubleMethod(bbox,midRectGetBottomRightY);
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* newpoints;

  correct = true;
  newpoints = new CcPoints(instance);
  
  return SetWord(newpoints);
}

/* 
 Creation of a CcPoints object. 

*/
static Word CreateCcPoints(const ListExpr typeInfo) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* res = new CcPoints((size_t)1);
  res->SetObject(0);
  return SetWord(res);
}

/* 
 Deletion of a CcPoints object. 

*/

static void DeleteCcPoints(Word &w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "++++++++++++++++++++ called Delete of CcPointss +++++++++++++++++++++++++" << endl; 
  //((CcPoints*)w.addr)->Finalize();
  delete ((CcPoints *)w.addr);
  w.addr = 0;
}

/* 
 Close a CcPoints object. 

*/

static void CloseCcPoints(Word & w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "++++++++++++++++++++ called Close of CcPoints +++++++++++++++++++++++++" << endl; 
  delete (CcPoints *)w.addr;
  w.addr = 0;
}

/* 
 Clone a CcPoints object. 

*/

static Word CloneCcPoints(const Word &w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return SetWord(((CcPoints *)w.addr)->Clone());
}

static void* CastCcPoints( void* addr ) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return new (addr) CcPoints;
}

bool OpenCcPoints(SmiRecord& valueRecord,
                  size_t& offset,
		  const ListExpr typeInfo,
		  Word& value){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* P = (CcPoints*)TupleElement::Open(valueRecord,offset, typeInfo);
  //P->RestoreJavaObjectFromFLOB();
  P->SetObject(0);
  value = SetWord(P);
  return true;
}

bool SaveCcPoints( SmiRecord& valueRecord,
                   size_t& offset,
		   const ListExpr typeInfo,
		   Word& value) { 
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* P = (CcPoints*) value.addr;
  TupleElement::Save(valueRecord,offset,typeInfo,P);
  return true;
}

/*
 2.4 Function Describing the Signature of the Type Constructor

*/

static ListExpr PointsProperty() {

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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  bool res = (nl->IsEqual(type, "points"));
  return res;
}


int
SizeOfCcPoints()
{
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  void SyncBboxData();

public:
  // Bounding Box data 
  double BboxTopLeftX;
  double BboxTopLeftY;
  double BboxBottomRightX;
  double BboxBottomRightY;

  // Inherited methods of Attribute 
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined() const;
  int Sizeof();

  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  void Destroy();
  jobject GetObject() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    return obj; }
  bool Adjacent(Attribute *arg);
  int NumOfFLOBs();
  FLOB *GetFLOB(const int i);
  void Initialize();
  void Finalize() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  void RestoreJavaObjectFromFLOB();
  size_t HashValue();


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
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    this->obj = obj;
  }

  /* Destructor of CcLines. This destructor destroys also the 
     object inside the JVM. */
  ~CcLines() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *L = (CcLines *) attr;
  return env->CallByteMethod(obj,midLinesCompare,L->obj);
}

/* 
 Inherited method of StandardAttribute 

*/

Attribute *CcLines::Clone() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* res = new CcLines((size_t)objectData.Size());
  res->CopyFrom(this);
  return res;
}

/* 
 Inherited method of StandardAttribute 

*/

bool CcLines::IsDefined() const {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return true;
}

/* 
 Inherited method of StandardAttribute 

*/

int CcLines::Sizeof() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return sizeof(CcLines);
}

/* 
 Inherited method of StandardAttribute 

*/

/* 
 Inherited method of StandardAttribute 

*/

void CcLines::SetDefined(bool Defined) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  this->Defined = Defined;
}

/* 
 Inherited method of StandardAttribute 

*/

void CcLines::CopyFrom(StandardAttribute* right) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *L = (CcLines *)right;
  objectData.Resize(L->objectData.Size());
  char *data = new char[L->objectData.Size()];
  L->objectData.Get(0,L->objectData.Size(),data);
  objectData.Put(0,L->objectData.Size(),data);
  delete [] data;
  data = 0;
  BboxTopLeftX = L->BboxTopLeftX;
  BboxTopLeftY = L->BboxTopLeftY;
  BboxBottomRightX = L->BboxBottomRightX;
  BboxBottomRightY = L->BboxBottomRightY;
  obj=0;
}

/* 
 Inherited method of StandardAttribute 

*/

size_t CcLines::HashValue() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return 0;
}

bool CcLines::Adjacent(Attribute * arg) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return false;
}

int CcLines::NumOfFLOBs(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return 1;
}

FLOB *CcLines::GetFLOB(const int i) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  assert(i==0);
  return &objectData;
}

void CcLines::Initialize() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  obj = 0;
  Defined = true;
}

/*
 Constructors

*/

CcLines::CcLines(size_t size):objectData(size),canDelete(false) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  SetDefined(true);
  obj = env->NewObject(clsLines,midLinesConst);
  if (obj == 0) error(__FILE__,__LINE__);
}

/*
 This constructor takes the nested list representation
   of CcLines and recovers the underlying java object with
   help of this data. 

*/

CcLines::CcLines(const ListExpr &le):objectData(1) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  obj = Convert_CToJava_Lines(le);
  canDelete = false;
  SyncBboxData();
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
 This constructor takes a pointer to a java object which is
   already created. 

*/
 
CcLines::CcLines(const jobject jobj) : objectData(1) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  canDelete = false;
  obj = jobj;
  SyncBboxData();
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
 This constructor creates an empty CcPoints object. 

*/	

CcLines::CcLines() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
}

/* 
 retrieves the nested list representation of the underlying
     java object. 

*/

bool CcLines::GetNL(ListExpr& le) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  if (!Defined) {
    le = nl->TheEmptyList();
    return true;
  }
  if (!obj) RestoreJavaObjectFromFLOB();
  le = Convert_JavaToC_Lines(obj);
  return true;
}

/* 
 Destructor of CcLines. This destructor destroys also the 
     object inside the JVM. 

*/
 
/*
  ~Destroy~

  The Destroy function as known from other (non-JNI) algebras.

*/

void CcLines::Destroy(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  canDelete=true;
}

/* 
 Returns the pointer to the proper java objet. 

*/

jobject CcLines::GetObj() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return obj;
}

/* 
 restores the java object from FLOB
   the FLOB must exist

*/

void CcLines::RestoreFLOBFromJavaObject(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jbyteArray jbytes = (jbyteArray) env->CallObjectMethod(obj,midLinesWriteToByteArray);
  if(jbytes == 0) error(__FILE__,__LINE__);
  
  int size = env->GetArrayLength(jbytes);

  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,JNI_ABORT);
  env->DeleteLocalRef(jbytes);
  jbytes = 0;
 }

/* 
 creates the content of a FLOB from the given Java-Object

*/

void CcLines::RestoreJavaObjectFromFLOB(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  if (obj) return;
  
  if(&objectData == 0){
    return;
  }
  int size = objectData.Size();

  char *bytes = new char[size];
  objectData.Get(0,size,bytes);
  // copy the data into a java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);

  obj = env->CallStaticObjectMethod(clsLines,midLinesReadFrom,jbytes);
  if(obj == 0) error(__FILE__,__LINE__);

  //obj = jres;
  jbyte* elems = env->GetByteArrayElements(jbytes,0);
  env->ReleaseByteArrayElements(jbytes,elems,JNI_ABORT);
  delete [] bytes;
  bytes = NULL;
  env->DeleteLocalRef(jbytes);
  jbytes = 0;
}


void CcLines::Print() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //jmethodID mid = env->GetMethodID(clsLines,"print","()V");
  env->CallVoidMethod(obj,midLinesPrint);
}

void CcLines::SyncBboxData() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //restore object if not present
  if (!obj) RestoreJavaObjectFromFLOB();
  //get bounding box of the Java Lines object
  jobject bbox = env->CallObjectMethod(obj,midLinesRect);
  if (bbox == 0) error(__FILE__,__LINE__);

  //get the coordinates of the bounding box
  BboxTopLeftX = (double)env->CallDoubleMethod(bbox,midRectGetTopLeftX);
  BboxTopLeftY = (double)env->CallDoubleMethod(bbox,midRectGetTopLeftY);
  BboxBottomRightX = (double)env->CallDoubleMethod(bbox,midRectGetBottomRightX);
  BboxBottomRightY = (double)env->CallDoubleMethod(bbox,midRectGetBottomRightY);
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* newlines;

  correct = true;
  newlines = new CcLines(instance);
  
  return SetWord(newlines);
}

/* 
 Creation of a CcLines object. 

*/

static Word CreateCcLines(const ListExpr typeInfo) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "CreateCcLines" << endl;

  CcLines* res = new CcLines((size_t)1);
  res->SetObject(0);
  return SetWord(res);
}

/* 
 Deletion of a CcLines object. 

*/
 static void DeleteCcLines(Word &w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "++++++++++++++++++++ called Delete of CcLines +++++++++++++++++++++++++" << endl;
  //((CcLines*)w.addr)->Finalize();
  delete ((CcLines *)w.addr);
  w.addr = 0;
}

/* 
 Close a CcLines object. 

*/
static void CloseCcLines(Word & w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "++++++++++++++++++++ called Close of CcLines +++++++++++++++++++++++++" << endl;
  delete (CcLines *)w.addr;
  w.addr = 0;
}

/* 
 Clone a CcLines object. 

*/

static Word CloneCcLines(const Word &w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return SetWord(((CcLines *)w.addr)->Clone());
}

static void* CastCcLines( void* addr ) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return new (addr) CcLines;
}

bool OpenCcLines(SmiRecord& valueRecord,
                 size_t& offset,
		 const ListExpr typeInfo,
		 Word& value) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* L = (CcLines*) value.addr;
  TupleElement::Save(valueRecord,offset,typeInfo,L);
  return true;
}

/*
 3.4 Function Describing the Signature of the Type Constructor
 
*/

static ListExpr LinesProperty() {

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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return (nl->IsEqual(type, "line"));
}

int
SizeOfCcLines()
{
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  void SyncBboxData();

public:
  // Bounding Box data 
  double BboxTopLeftX;
  double BboxTopLeftY;
  double BboxBottomRightX;
  double BboxBottomRightY;

  // Inherited methods of Attribute 
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined() const;
  int Sizeof();
  //void *GetValue();
  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  void Destroy();
  jobject GetObject() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    return obj; }
  bool Adjacent(Attribute *arg);
  int NumOfFLOBs();
  FLOB *GetFLOB(const int i);
  void Initialize();
  void Finalize() {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  void RestoreJavaObjectFromFLOB();
  size_t HashValue();


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
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    this->obj = obj;
  }

  /* Destructor of CcRegions. This destructor destroys also the 
     object inside the JVM. */
  ~CcRegions() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
    //cout << "++++++++++++++++++++ called destructor of CcRegions +++++++++++++++++++++++++" << endl;
    env->DeleteLocalRef(obj);
    obj = 0;
  }
  // Returns the pointer to the proper java objet. 
  jobject GetObj();
  void Print();
};

/* 
 Inherited method of StandardAttribute 

*/

int CcRegions::Compare(Attribute *attr) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *R = (CcRegions *) attr;
  return env->CallByteMethod(obj,midRegionsCompare,R->obj);
}

/* 
 Inherited method of StandardAttribute 

*/

Attribute *CcRegions::Clone() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* res = new CcRegions((size_t)objectData.Size());
  res->CopyFrom(this);
  return res;
}

/* 
 Inherited method of StandardAttribute 

*/

bool CcRegions::IsDefined() const {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return true;
}

/* 
 Inherited method of StandardAttribute 

*/

int CcRegions::Sizeof() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return sizeof(CcRegions);
}

/* 
 Inherited method of StandardAttribute 

*/
 
/* 
 Inherited method of StandardAttribute 

*/

void CcRegions::SetDefined(bool Defined) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  this->Defined = Defined;
}

/* 
 Inherited method of StandardAttribute 

*/

void CcRegions::CopyFrom(StandardAttribute* right) { 
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *R = (CcRegions *)right;
  objectData.Resize(R->objectData.Size());
  char *data = new char[R->objectData.Size()];
  R->objectData.Get(0,R->objectData.Size(),data);
  objectData.Put(0,R->objectData.Size(),data);
  delete [] data;
  data = 0;
  BboxTopLeftX = R->BboxTopLeftX;
  BboxTopLeftY = R->BboxTopLeftY;
  BboxBottomRightX = R->BboxBottomRightX;
  BboxBottomRightY = R->BboxBottomRightY;
  obj=0;
}

/* 
 Inherited method of StandardAttribute 

*/

size_t CcRegions::HashValue() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return 0;
}

bool CcRegions::Adjacent(Attribute * arg) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return false;
}

int CcRegions::NumOfFLOBs(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return 1;
}

FLOB *CcRegions::GetFLOB(const int i){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  assert(i==0);
  return &objectData;
}

void CcRegions::Initialize() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  obj = 0;
  Defined=true;
}

/*
 Constructors

*/

CcRegions::CcRegions(size_t size):objectData(size),canDelete(false) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  SetDefined(true);
  obj = env->NewObject(clsRegions, midRegionsConstVoid);
  if (obj == 0) error(__FILE__,__LINE__);
}

/* 
 This constructor takes the nested list representation
   of CcRegions and recovers the underlying java object with
   help of this data. 

*/

CcRegions::CcRegions(const ListExpr &le):objectData(1) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  obj = Convert_CToJava_Regions(le);
  canDelete = false;
  SyncBboxData();
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
 This constructor takes a pointer to a java object which is
   already created. 

*/

CcRegions::CcRegions(const jobject jobj):objectData(1) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  canDelete = false;
  obj = jobj;
  SyncBboxData();
  RestoreFLOBFromJavaObject();
  SetDefined(true);
}

/* 
 This constructor creates an empty CcRegions object.

*/	

CcRegions::CcRegions() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
}

/* 
 retrieves the nested list representation of the underlying
     java object. 

*/

bool CcRegions::GetNL(ListExpr& le) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  ~Destroy~

  The Destroy function as kinown from other (non-JNI) algebras.

*/

void CcRegions::Destroy(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  canDelete=true;
}

/* 
 Returns the pointer to the proper java object. 

*/

jobject CcRegions::GetObj() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return obj;
}

/* 
 restores the java object from FLOB
   the FLOB must exists

*/

void CcRegions::RestoreFLOBFromJavaObject(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  jbyteArray jbytes = (jbyteArray) env->CallObjectMethod(obj,midRegionsWriteToByteArray);
  if(jbytes == 0) error(__FILE__,__LINE__);
  
  int size = env->GetArrayLength(jbytes);
  char *bytes = (char*) env->GetByteArrayElements(jbytes,0);
  objectData.Resize(size);
  objectData.Put(0,size,bytes);
  env->ReleaseByteArrayElements(jbytes,(jbyte*)bytes,JNI_ABORT);
  env->DeleteLocalRef(jbytes);
  jbytes = 0;
}

/* 
 creates the content of a FLOB from the given Java-Object

*/

void CcRegions::RestoreJavaObjectFromFLOB(){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  if (obj) return;
  // read the data from flob

  if(&objectData == 0){
    return;
  }
  int size = objectData.Size();
  char *bytes = new char[size];
  objectData.Get(0,size,bytes);
  // copy the data into a java-array
  jbyteArray jbytes = env->NewByteArray(size);
  env->SetByteArrayRegion(jbytes,0,size,(jbyte*)bytes);

  obj = env->CallStaticObjectMethod(clsRegions,midRegionsReadFrom,jbytes);
  if(obj == 0){
    error(__FILE__,__LINE__);
  }
  //obj = jres;
  jbyte* elems = env->GetByteArrayElements(jbytes,0);
  env->ReleaseByteArrayElements(jbytes,elems,JNI_ABORT);
  delete [] bytes;
  bytes = NULL;
  env->DeleteLocalRef(jbytes);
  jbytes = 0;
  }

void CcRegions::Print() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //jmethodID mid = env->GetMethodID(clsRegions,"print","()V");
  env->CallVoidMethod(obj,midRegionsPrint);
}

void CcRegions::SyncBboxData() {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //restore object if not present
  if (!obj) RestoreJavaObjectFromFLOB();
  //get bounding box of the Java Regions object
  jobject bbox = env->CallObjectMethod(obj,midRegionsRect);
  if (bbox == 0) error(__FILE__,__LINE__);

  //jmethodID mid = env->GetMethodID(clsRect,"print","()V");
  //if (mid == 0) error (__FILE__,__LINE__);
  //env->CallVoidMethod(bbox,mid);

  //get the coordinates of the bounding box
  BboxTopLeftX = (double)env->CallDoubleMethod(bbox,midRectGetTopLeftX);
  BboxTopLeftY = (double)env->CallDoubleMethod(bbox,midRectGetTopLeftY);
  BboxBottomRightX = (double)env->CallDoubleMethod(bbox,midRectGetBottomRightX);
  BboxBottomRightY = (double)env->CallDoubleMethod(bbox,midRectGetBottomRightY);
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* newregions;
  correct = true;
  newregions = new CcRegions(instance);
  
  return SetWord(newregions);
}

/* 
 Creation of a CcRegions object. 

*/

static Word CreateCcRegions(const ListExpr typeInfo) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* res = new CcRegions((size_t)1);
  res->SetObject(0);
  return SetWord(res);
}

/* 
 Deletion of a CcRegions object. 

*/

static void DeleteCcRegions(Word &w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "++++++++++++++++++++ called Delete of CcRegions +++++++++++++++++++++++++" << endl; 
  //((CcRegions*)w.addr)->Finalize();
  delete ((CcRegions *)w.addr);
  w.addr = 0;
}

/* 
 Close a CcRegions object. 

*/

static void CloseCcRegions(Word & w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "++++++++++++++++++++ called Close of CcRegions +++++++++++++++++++++++++" << endl;
  delete (CcRegions *)w.addr;
  w.addr = 0;
}

/* 
 Clone a CcRegions object. 

*/

static Word CloneCcRegions(const Word &w) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return SetWord(((CcRegions *)w.addr)->Clone());
}

static void* CastCcRegions( void* addr ){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return new (addr) CcRegions;
}

bool OpenCcRegions(SmiRecord& valueRecord,size_t& offset,const ListExpr typeInfo,Word& value){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* R = (CcRegions*) TupleElement::Open(valueRecord,offset, typeInfo);
  R->SetObject(0);
  value = SetWord(R);
  return true;
}

bool SaveCcRegions(SmiRecord& valueRecord,size_t& offset,const ListExpr typeInfo,Word& value) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* R = (CcRegions*) value.addr;
  TupleElement::Save(valueRecord,offset,typeInfo,R); 
  return true;
}


/*
 4.4 Function Describing the Signature of the Type Constructor

*/

static ListExpr RegionsProperty() {

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
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return (nl->IsEqual(type, "region"));
}

int
SizeOfCcRegions()
{
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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

static ListExpr equalTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr unequalTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr disjointTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr insideTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr intersectsTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr meetsTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr borderInCommonTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr onBorderOfTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("bool");
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("bool");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr intersectionTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("points");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("points");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("region");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("line");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr plusTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("points");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("line");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("region");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr minusTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("points");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("line");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("region");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr commonBorderTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("line");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("line");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("line");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("line");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr verticesTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 1) {
    arg1 = nl->First(args);
    if (nl->IsEqual(arg1,"line"))
      return nl->SymbolAtom("points");
    if (nl->IsEqual(arg1,"region"))
      return nl->SymbolAtom("points");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameter of type "+nl->SymbolValue(arg1));
    else
      ErrorReporter::ReportError("Type mapping function got wrong type as parameter.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 1.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr noOfComponentsTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 1) {
    arg1 = nl->First(args);
    if (nl->IsEqual(arg1,"points"))
      return nl->SymbolAtom("int");
    if (nl->IsEqual(arg1,"line"))
      return nl->SymbolAtom("int");
    if (nl->IsEqual(arg1,"region"))
      return nl->SymbolAtom("int");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameter of type "+nl->SymbolValue(arg1));
    else
      ErrorReporter::ReportError("Type mapping function got wrong type as parameter.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 1.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr distTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"points"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
      return nl->SymbolAtom("real");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameters of type "+nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types as parameters.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 2.");
  return nl->SymbolAtom("typeerror");
}

static ListExpr diameterTypeMap (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1,arg2;
  if (nl->ListLength(args) == 1) {
    arg1 = nl->First(args);
    if (nl->IsEqual(arg1,"points"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"line"))
      return nl->SymbolAtom("real");
    if (nl->IsEqual(arg1,"region"))
      return nl->SymbolAtom("real");
    if ((nl->AtomType(arg1) == SymbolType) && (nl->AtomType(arg2) == SymbolType)) 
      ErrorReporter::ReportError("Type mapping function got parameter of type "+nl->SymbolValue(arg1));
    else
      ErrorReporter::ReportError("Type mapping function got wrong type as parameter.");
  } 
  ErrorReporter::ReportError("Type mapping function got a parameter of lengh != 1.");
  return nl->SymbolAtom("typeerror");
}

/* 
This is a general type mapping function for all Rose methods
   which take two parameters. 

*/
static ListExpr typeMappingRose (ListExpr args, char *type1, char *type2, char *resulttype) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
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

static ListExpr typeMappingRose (ListExpr args, char *type1, char *resulttype) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1;

  if (nl->ListLength(args) == 1) {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, type1))
      return nl->SymbolAtom(resulttype);
  }
  return nl->SymbolAtom("typeerror");
}


/* 
 type mapping function: ccregions x ccregions -> bool 

*/

static ListExpr ccregionsccregionsBool(ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return typeMappingRose(args, "region", "region", "bool");
}


/* 
 type mapping function: cclines -> ccregions 

*/

static ListExpr cclinesccregions(ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return typeMappingRose(args, "line", "region");
}


/* 
 type mapping function: ccregions -> cclines 

*/

static ListExpr ccregionscclines(ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return typeMappingRose(args, "region", "line");
}


/* 
 type mapping function: ccregions -> real 

*/

static ListExpr ccregionsDouble(ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return typeMappingRose(args, "region", "real");
}

/* 
 type mapping function: cclines -> real 

*/

static ListExpr cclinesDouble(ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return typeMappingRose(args, "line", "real");
}

/*
 type mapping function: int -> bool

*/

static ListExpr intBool (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1;
  if (nl->ListLength(args) == 1) {
    arg1 = nl->First(args);
    if (nl->IsEqual(arg1, "int"))
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
 type mapping function: double -> bool

*/

static ListExpr doubleBool (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1;
  if (nl->ListLength(args) == 1) {
    arg1 = nl->First(args);
    if (nl->IsEqual(arg1, "real"))
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
 5.2 Selection Function

Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/

static int simpleSelect (ListExpr args ){
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  return 0; 
}

static int equalSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 2;
  return -1;
}

static int unequalSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 2;
  return -1;
}

static int disjointSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 2;
  return -1;
}

static int insideSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"region"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 2;
  return -1;
}

static int intersectsSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
    return 2;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 3;
  return -1;
}

static int meetsSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
    return 2;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 3;
  return -1;
}

static int borderInCommonSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
    return 2;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 3;
  return -1;
}

static int onBorderOfSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"line"))
    return 0;
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"region"))
    return 1;
  return -1;
}

static int intersectionSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 2;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
    return 3;
  return -1;
}

static int plusSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 2;
  return -1;
}

static int minusSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 2;
  return -1;
}

static int commonBorderSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 0;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
    return 1;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
    return 2;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 3;
  return -1;
}

static int verticesSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  if (nl->IsEqual(arg1,"line"))
    return 0;
  if (nl->IsEqual(arg1,"region"))
    return 1;
  return -1;
}

static int noOfComponentsSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  if (nl->IsEqual(arg1,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region"))
    return 2;
  return -1;
}

static int distSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"points"))
    return 0;
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"line"))
    return 1;
  if (nl->IsEqual(arg1,"points") && nl->IsEqual(arg2,"region"))
    return 2;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"points"))
    return 3;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line"))
    return 4;
  if (nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region"))
    return 5;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"points"))
    return 6;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line"))
    return 7;
  if (nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region"))
    return 8;
  return -1;
}

static int diameterSelect (ListExpr args) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  ListExpr arg1 = nl->First(args);
  if (nl->IsEqual(arg1,"points"))
    return 0;
  if (nl->IsEqual(arg1,"line"))
    return 1;
  if (nl->IsEqual(arg1,"region"))
    return 2;
  return -1;
}


/*
  This method gets a Java object, which has to be a Rational instance, and returns a double value.

*/

static double convertRational(jobject jrat) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  int numerator = env->CallIntMethod(jrat,midRationalGetNumerator);
  int denominator = env->CallIntMethod(jrat,midRationalGetDenominator);

  return (double)numerator/(double)denominator;
}


/*
 Call the setDeviationValue Java function.

*/

static int callSetDeviationValue(CcReal *ccr) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //set deviation value to ccr
  //jmethodID midSetDeviation = env->GetStaticMethodID(clsROSEAlgebra, "setDeviationValue", "(D)V");
  //if (midSetDeviation == 0) error(__FILE__,__LINE__);
  double v = ccr->GetRealval();
  DEVIATION_VALUE = v;
  env->CallStaticVoidMethod(clsROSEAlgebra, midROSESetDeviationValue, v);

  return 0;
}

/*
  Call the chooseTriangulator Java function

*/

static int callChooseTriangulator(CcInt *cci) {
    if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //set the triangulator
  //jmethodID midChooseTri = env->GetStaticMethodID(clsROSEAlgebra, "chooseTriangulator", "(I)V");
  //if (midChooseTri == 0) error(__FILE__,__LINE__);
  env->CallStaticVoidMethod(clsROSEAlgebra, midROSEChooseTriangulator, cci->GetIntval());

  return 0;
}

/*
 5.3.2 The proper Value Mapping Functions. 

*/

/*
 Function to compute bounding box intersection. It returns true, if the bounding boxes intersect. This function
 takes the DEVIATION VALUE into account.

*/

static bool bboxesIntersect(double o1tlx, double o1tly, double o1brx, double o1bry,
			    double o2tlx, double o2tly, double o2brx, double o2bry) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //cout << "bboxesIntersect" << endl;
  bool xcomm = false;
  bool ycomm = false;
    
  //    if (o1tlx == o2tlx || o1brx == o2brx || o1tlx == o2brx || o1brx == o2tlx ||
  //    (o1tlx < o2tlx && o1brx > o2tlx) || (o1tlx < o2brx && o1brx > o2brx) ||
  //    (o2tlx < o1tlx && o2brx > o1tlx) || (o2tlx < o1brx && o2brx > o1brx))
  //    xcomm = true;
      
  
  if (abs(o1tlx - o2tlx) < DEVIATION_VALUE ||
      abs(o1brx - o2brx) < DEVIATION_VALUE ||
      abs(o1tlx - o2brx) < DEVIATION_VALUE ||
      abs(o1brx - o2tlx) < DEVIATION_VALUE ||
      (o1tlx < o2tlx && o1brx > o2tlx) || (o1tlx < o2brx && o1brx > o2brx) ||
      (o2tlx < o1tlx && o2brx > o1tlx) || (o2tlx < o1brx && o2brx > o1brx))
    xcomm = true;
  
  
  //  if (o1tly == o2tly || o1bry == o2bry || o1tly == o2bry || o1bry == o1tly ||
  //  (o1tly > o2tly && o1bry < o2tly) || (o1tly > o2bry && o1bry < o2bry) ||
  //  (o2tly > o1tly && o2bry < o1tly) || (o2tly > o1bry && o2bry < o1bry))
  //  ycomm = true;
    
  
  if (abs(o1tly - o2tly) < DEVIATION_VALUE ||
      abs(o1bry - o2bry) < DEVIATION_VALUE ||
      abs(o1tly - o2bry) < DEVIATION_VALUE ||
      abs(o1bry - o2tly) < DEVIATION_VALUE ||
      (o1tly > o2tly && o1bry < o2tly) || (o1tly > o2bry && o1bry < o2bry) ||
      (o2tly > o1tly && o2bry < o1tly) || (o2tly > o1bry && o2bry < o1bry))
    ycomm = true;
  if (xcomm && ycomm) 
    return true;
  else
    return false;
}

/*
 Function to compute bounding box equality. It returns true, if the bounding boxes are equal. This function
 takes the DEVIATION VALUE into account.

*/

static bool bboxesEqual(double o1tlx, double o1tly, double o1brx, double o1bry,
			double o2tlx, double o2tly, double o2brx, double o2bry) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  //if (o1tlx == o2tlx && o1tly == o2tly && o1brx == o2brx && o1bry == o2bry)
  if (abs(o1tlx - o2tlx) < DEVIATION_VALUE &&
      abs(o1tly - o2tly) < DEVIATION_VALUE &&
      abs(o1brx - o2brx) < DEVIATION_VALUE &&
      abs(o1bry - o2bry) < DEVIATION_VALUE)
    return true;
  else 
    return false;
}

/* 
 Equals predicate for two ccpoints. 

*/

static int pp_equalFun(Word* args, Word& result, int message, 
			       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);
  
  //if bboxes aren't equal, return false
  if (!bboxesEqual(ccp1->BboxTopLeftX,ccp1->BboxTopLeftY,ccp1->BboxBottomRightX,ccp1->BboxBottomRightY,
		   ccp2->BboxTopLeftX,ccp2->BboxTopLeftY,ccp2->BboxBottomRightX,ccp2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }//if

  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);
  //((CcBool *)result.addr)->Set(true, callJMethod_PPB("pp_equal", ccp1, ccp2));
  jboolean obj = env->CallStaticBooleanMethod(clsROSEAlgebra,midROSEpp_equal,ccp1->GetObj(),ccp2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_equal,ccp1->GetObj(),ccp2->GetObj()));
  
  return 0;
}

/*
 Equals predicate for two cclines. 

*/

static int ll_equalFun(Word* args, Word& result, int message, 
				       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ls1 = ((CcLines *)args[0].addr);
  CcLines* ls2 = ((CcLines *)args[1].addr);

  //if bboxes aren't equal, return false
  if (!bboxesEqual(ls1->BboxTopLeftX,ls1->BboxTopLeftY,ls1->BboxBottomRightX,ls1->BboxBottomRightY,
		   ls2->BboxTopLeftX,ls2->BboxTopLeftY,ls2->BboxBottomRightX,ls2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }//if

  if (!ls1->GetObject()) ls1->RestoreJavaObjectFromFLOB();
  if (!ls2->GetObject()) ls2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //((CcBool *)result.addr)->Set(true, callJMethod_LLB("ll_equal", ls1, ls2));
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_equal,ls1->GetObj(),ls2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_equal,ls1->GetObj(),ls2->GetObj()));

  return 0;
}

/* 
 Equal predicate for two ccregions. 

*/

static int rr_equalFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* rs1 = ((CcRegions *)args[0].addr);
  CcRegions* rs2 = ((CcRegions *)args[1].addr);

  //if bboxes aren't equal, return false
  if (!bboxesEqual(rs1->BboxTopLeftX,rs1->BboxTopLeftY,rs1->BboxBottomRightX,rs1->BboxBottomRightY,
		       rs2->BboxTopLeftX,rs2->BboxTopLeftY,rs2->BboxBottomRightX,rs2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }//if

  //bboxes intersect, so prepare to invoke the Java method
  if (!rs1->GetObject()) rs1->RestoreJavaObjectFromFLOB();
  if (!rs2->GetObject()) rs2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  //((CcBool *)result.addr)->Set(true, callJMethod_RRB("rr_equal", rs1, rs2));

  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_equal,rs1->GetObj(),rs2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_equal,rs1->GetObj(),rs2->GetObj()));

  return 0;
}

/*
 pp[_]unequal predicate for two ccpoints. 

*/

static int pp_unequalFun(Word* args, Word& result, int message, 
			 Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* ccps1 = ((CcPoints *)args[0].addr);
  CcPoints* ccps2 = ((CcPoints *)args[1].addr);

  //if bboxes aren't equal, return true
  if (!bboxesEqual(ccps1->BboxTopLeftX,ccps1->BboxTopLeftY,ccps1->BboxBottomRightX,ccps1->BboxBottomRightY,
		   ccps2->BboxTopLeftX,ccps2->BboxTopLeftY,ccps2->BboxBottomRightX,ccps2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }
  //bboxes are equal, so prepare to invoke Java method
  if (!ccps1->GetObject()) ccps1->RestoreJavaObjectFromFLOB();
  if (!ccps2->GetObject()) ccps2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_unequal,ccps1->GetObj(),ccps2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_unequal,ccps1->GetObj(),ccps2->GetObj()));
  
  return 0;
}

/* 
 ll[_]nequal predicate for two cclines. 
 
*/

static int ll_unequalFun(Word* args, Word& result, int message, 
			 Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  //if bboxes aren't equal, return true
  if (!bboxesEqual(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		   ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }

  //bboxes are equal, prepare to invoke the Java method
  if (!ccl1 ->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2 ->GetObject()) ccl2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_unequal,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_unequal,ccl1->GetObj(),ccl2->GetObj()));
  
  return 0;
}

/*

 rr[_]unequal predicate for two ccregions 

*/

static int rr_unequalFun(Word* args, Word& result, int message, 
				       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
	
  //if bboxes aren't equal, return true
  if (!bboxesEqual(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		   ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }
  
  //bboxes are equal, so prepare to invoke the Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	 
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_unequal,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_unequal,ccr1->GetObj(),ccr2->GetObj()));
  
  return 0;
}

/* 
 pp[_]disjoint predicate for two CcPoints 

*/

static int pp_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);
	
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccp1->BboxTopLeftX,ccp1->BboxTopLeftY,ccp1->BboxBottomRightX,ccp1->BboxBottomRightY,
		       ccp2->BboxTopLeftX,ccp2->BboxTopLeftY,ccp2->BboxBottomRightX,ccp2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }

  //bboxes are equal, prepare to invoke the Java method
  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_disjoint,ccp1->GetObj(),ccp2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_disjoint,ccp1->GetObj(),ccp2->GetObj()));

  return 0;
}

/* 
 ll[_]disjoint predicate for two CcLines 

*/

static int ll_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		       ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }
  
  //bboxes intersect, so prepare to invoke the Java method
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_disjoint,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_disjoint,ccl1->GetObj(),ccl2->GetObj()));
  return 0;
}

/* 
 rr[_]disjoint predicate for two CcRegions 

*/

static int rr_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		   ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_disjoint,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_disjoint,ccr1->GetObj(),ccr2->GetObj()));
  
  return 0;
}

/* 
 pr[_]inside predicate for CcPoints and CcRegions 

*/

static int pr_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* ccp = ((CcPoints *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccp->BboxTopLeftX,ccp->BboxTopLeftY,ccp->BboxBottomRightX,ccp->BboxBottomRightY,
		       ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpr_inside,ccp->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpr_inside,ccp->GetObj(),ccr->GetObj()));
  
  return 0;
}

/* 
 lr[_]inside predicate for CcLines and CcRegions 

*/

static int lr_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
	
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY,
		       ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_inside,ccl->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_inside,ccl->GetObj(),ccr->GetObj()));
  
  return 0;
}

/* 
 rr[_]inside predicate for two CcRegions 

*/

static int rr_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_inside,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_inside,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]area[_]disjoint predicate for two CcRegions 

*/

static int rr_area_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return true
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_area_disjoint,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_area_disjoint,ccr1->GetObj(),ccr2->GetObj()));
  
  return 0;
}

/* 
 rr[_]edge[_]disjoint predicate for two CcRegions 

*/

static int rr_edge_disjointFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return true
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_edge_disjoint,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_edge_disjoint,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]edge[_]inside predicate for two CcRegions 

*/

static int rr_edge_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_edge_inside,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_edge_inside,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]vertex[_]inside predicate for two CcRegions 

*/

static int rr_vertex_insideFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_vertex_inside,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_vertex_inside,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]intersects predicate for two CcRegions 

*/

static int rr_intersectsFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_intersects,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_intersects,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]meets predicate for two CcRegions 

*/

static int rr_meetsFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_meets,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_meets,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]border[_]in[_]common predicate for two CcRegions 

*/

static int rr_border_in_commonFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_border_in_common,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_border_in_common,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]adjacent predicate for two CcRegions 

*/

static int rr_adjacentFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
	
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_adjacent,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_adjacent,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]encloses predicate for two CcRegions 
 
*/

static int rr_enclosesFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_encloses,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_encloses,ccr1->GetObj(),ccr2->GetObj()));

  return 0;
}

/* 
 rr[_]intersection predicate for two CcRegions 

*/

static int rr_intersectionFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    //((CcRegions*)result.addr) = new CcRegions(env->NewObject(clsRegions,midRegionsConstVoid));
    result = qp->ResultStorage(s);
    //delete (CcRegions*)result.addr;
    //qp->ResultStorage(s,SetWord(new CcRegions(env->NewObject(clsRegions,midRegionsConstVoid))));
    ccresult = new CcRegions(env->NewObject(clsRegions,midRegionsConstVoid));
    ((CcRegions*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    return 0;
  }
  
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);
  //delete (CcRegions*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_intersection,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRegions,midRegionsConstVoid);
  }
  ccresult = new CcRegions(obj);
  //ccresult = new CcRegions(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_intersection,ccr1->GetObj(),ccr2->GetObj()));
  ((CcRegions*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));
  //ccresult = callJMethod_RRR("rr_intersection", ccr1, ccr2);
  //if(env->ExceptionOccurred())
  //  env->ExceptionDescribe();
  //result.addr = ccresult;

  return 0;
}

/* 
 rr[_]plus predicate for two CcRegions 

*/

static int rr_plusFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcRegions*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_plus,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRegions,midRegionsConstVoid);
  }
  ccresult = new CcRegions(obj);
  //ccresult = new CcRegions(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_plus,ccr1->GetObj(),ccr2->GetObj()));
  ((CcRegions*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));  
  //ccresult = callJMethod_RRR("rr_plus", ccr1, ccr2);
  //result.addr = ccresult;

  return 0;
}

/* 
 rr[_]minus predicate for two CcRegions 

*/

static int rr_minusFun(Word* args, Word& result, int message, 
			  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return first object
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    //delete (CcRegions*)result.addr;
    //((CcRegions*)result.addr) = ccr1;
    //qp->ResultStorage(s,SetWord(ccr1));
    ((CcRegions*)result.addr)->CopyFrom(ccr1);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcRegions*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_minus,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRegions,midRegionsConstVoid);
  }
  ccresult = new CcRegions(obj);
  //ccresult = new CcRegions(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_minus,ccr1->GetObj(),ccr2->GetObj()));
  ((CcRegions*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));
  //jobject res = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_minus,ccr1->GetObj(),ccr2->GetObj());
  //if (res == 0) error(__FILE__, __LINE__);
  //ccresult = new CcRegions(res);
  //result.addr = ccresult;

  return 0;
}

/* 
 rr[_]common[_]border predicate for two CcRegions 

*/

static int rr_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;

  CcRegions* ccr1 = ((CcRegions *)args[0].addr);
  CcRegions* ccr2 = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccr1->BboxTopLeftX,ccr1->BboxTopLeftY,ccr1->BboxBottomRightX,ccr1->BboxBottomRightY,
		       ccr2->BboxTopLeftX,ccr2->BboxTopLeftY,ccr2->BboxBottomRightX,ccr2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ccresult = new CcLines(env->NewObject(clsLines,midLinesConst));
    ((CcLines*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    //delete (CcLines*)result.addr;
    //qp->ResultStorage(s,SetWord(new CcLines(env->NewObject(clsLines,midLinesConst))));
    //((CcRegions*)result.addr) = new CcRegions(env->NewObject(clsRegions,midRegionsConstVoid));
    return 0;
  }

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //delete (CcLines*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_common_border,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //ccresult = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_common_border,ccr1->GetObj(),ccr2->GetObj()));
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //ccresult = callJMethod_RRL("rr_common_border", ccr1, ccr2);
  //result.addr = ccresult;

  return 0;
}

/* 
 ll[_]intersects predicate for two CcLines 

*/

static int ll_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		       ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to call Java method
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_intersects,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_intersects,ccl1->GetObj(),ccl2->GetObj()));
  
  return 0;
}

/* 
 lr[_]intersects predicate for CcLines and CcRegions 

*/ 

static int lr_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY,
		       ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY)) {  
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_intersects,ccl->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_intersects,ccl->GetObj(),ccr->GetObj()));
  
  return 0;
}

/* 
 rl[_]intersects predicate for CcRegions and CcLines 

*/

static int rl_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY,
		       ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_intersects,ccr->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_intersects,ccr->GetObj(),ccl->GetObj()));
  
  return 0;
}

/* 
 ll[_]meets predicate for two cclines.

*/

static int ll_meetsFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		       ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_meets,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_meets,ccl1->GetObj(),ccl2->GetObj()));

  return 0;
}

/* 
 lr[_]meets predicate for cclines and ccregions. 

*/

static int lr_meetsFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
	
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY,
		       ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_meets,ccl->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_meets,ccl->GetObj(),ccr->GetObj()));
  
  return 0;
}

/* 
 rl[_]meets predicate for ccregions and cclines 

*/

static int rl_meetsFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);

  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY,
		       ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_meets,ccr->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_meets,ccr->GetObj(),ccl->GetObj()));
  
  return 0;
}

/* 
 ll[_]border[_]in[_]common predicate for two CcLines. 

*/

static int ll_border_in_commonFun(Word* args, Word& result, int message, 
				  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		       ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersects, so prepare to invoke Java method
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_border_in_common,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_border_in_common,ccl1->GetObj(),ccl2->GetObj()));

  return 0;
}

/* 
 lr[_]border[_]in[_]common predicate for CcLines and CcRegions. 

*/

static int lr_border_in_commonFun(Word* args, Word& result, int message, 
				  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY,
		       ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_border_in_common,ccl->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_border_in_common,ccl->GetObj(),ccr->GetObj()));

  return 0;
}

/* 
 rl[_]border[_]in[_]common predicate for CcRegions and CcLines. 

*/

static int rl_border_in_commonFun(Word* args, Word& result, int message, 
				  Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY,
		       ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_border_in_common,ccr->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_border_in_common,ccr->GetObj(),ccl->GetObj()));

  return 0;
}


/* 
 pl[_]on[_]border[_]of predicate for CcPoints and CcLines 

*/

static int pl_on_border_ofFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* ccp = ((CcPoints *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccp->BboxTopLeftX,ccp->BboxTopLeftY,ccp->BboxBottomRightX,ccp->BboxBottomRightY,
		       ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpl_on_border_of,ccp->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpl_on_border_of,ccp->GetObj(),ccl->GetObj()));

  return 0;
}

/* 
 pr[_]on[_]border[_]of predicate for CcPoints and CcRegions 

*/

static int pr_on_border_ofFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints* ccp = ((CcPoints *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);
  
  //if bboxes don't intersect, return false
  if (!bboxesIntersect(ccp->BboxTopLeftX,ccp->BboxTopLeftY,ccp->BboxBottomRightX,ccp->BboxBottomRightY,
		       ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpr_on_border_of,ccp->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = false;
  }
  ((CcBool*)result.addr)->Set(true,obj);
  //((CcBool*)result.addr)->Set(true,env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpr_on_border_of,ccp->GetObj(),ccr->GetObj()));

  return 0;
}

/* 
 pp[_]intersection predicate for two CcPoints.

*/

static int pp_intersectionFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccresult;

  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);

  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccp1->BboxTopLeftX,ccp1->BboxTopLeftY,ccp1->BboxBottomRightX,ccp1->BboxBottomRightY,
		       ccp2->BboxTopLeftX,ccp2->BboxTopLeftY,ccp2->BboxBottomRightX,ccp2->BboxBottomRightY)) {
    cout << "intersction: don't intersect" << endl;
    result = qp->ResultStorage(s);
    ccresult = new CcPoints(env->NewObject(clsPoints,midPointsConst));
    ((CcPoints*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    //delete (CcPoints*)result.addr;
    //qp->ResultStorage(s,SetWord(new CcPoints(env->NewObject(clsPoints,midPointsConst))));
    return 0;
  }
  cout << "intersection: intersect" << endl;
  //bboxes intersect, prepare to invoke Java method
  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcPoints*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_intersection,ccp1->GetObj(),ccp2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsPoints,midPointsConst);
  }
  ccresult = new CcPoints(obj);
  //ccresult = new CcPoints(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_intersection,ccp1->GetObj(),ccp2->GetObj()));
  //qp->ResultStorage(s, SetWord(ccresult));
  ((CcPoints*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;

  return 0;
}

/* 
 ll[_]intersection predicate for two CcLines. 

*/

static int ll_intersectionFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccresult;

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		       ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    //delete (CcPoints*)result.addr;
    ccresult = new CcPoints(env->NewObject(clsPoints,midPointsConst));
    ((CcPoints*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    //qp->ResultStorage(s,SetWord(new CcPoints(env->NewObject(clsPoints,midPointsConst))));
    //((CcPoints*)result.addr) = new CcPoints(env->NewObject(clsPoints,midRegionsConstVoid));
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcPoints*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_intersection,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsPoints,midPointsConst);
  }
  ccresult = new CcPoints(obj);
  //ccresult = new CcPoints(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_intersection,ccl1->GetObj(),ccl2->GetObj()));
  //qp->ResultStorage(s,SetWord(ccresult));
  ((CcPoints*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //ccresult = callJMethod_LLP("ll_intersection", ccl1, ccl2);
  //result.addr = ccresult;

  return 0;
}

/* 
 rl[_]intersection predicate for CcRegions and CcLines 

*/

static int rl_intersectionFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);

  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY,
		       ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY)) {
    //((CcLines*)result.addr) = new CcLines(env->NewObject(clsLines,midLinesConst));
    result = qp->ResultStorage(s);
    //delete (CcLines*)result.addr;
    //qp->ResultStorage(s,SetWord(new CcLines(env->NewObject(clsLines,midLinesConst))));
    ccresult = new CcLines(env->NewObject(clsLines,midLinesConst));
    ((CcLines*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcLines*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_intersection,ccr->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //result = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_intersection,ccr->GetObj(),ccl->GetObj()));
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));
  //ccresult = callJMethod_RLL("rl_intersection", ccr, ccl);
  //result.addr = ccresult;

  return 0;
}

/*
 pp[_]plus predicate for two CcPoints. 

*/

static int pp_plusFun(Word* args, Word& result, int message, 
			      Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccresult;

  CcPoints* ccp1 = ((CcPoints *)args[0].addr);
  CcPoints* ccp2 = ((CcPoints *)args[1].addr);

  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcPoints*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_plus,ccp1->GetObj(),ccp2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsPoints,midPointsConst);
  }
  ccresult = new CcPoints(obj);
  //ccresult = new CcPoints(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_plus,ccp1->GetObj(),ccp2->GetObj()));
  ((CcPoints*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));
  
  //ccresult = callJMethod_PPP("pp_plus", ccp1, ccp2);
  //result.addr = ccresult;

  return 0;
}

/* 
 ll[_]plus predicate for two CcLines. 

*/

static int ll_plusFun(Word* args, Word& result, int message, 
		      Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;
  
  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcLines*)result.addr;

  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_plus,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //ccresult = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_plus,ccl1->GetObj(),ccl2->GetObj()));
  
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  
  //result.addr = ccresult;
  //qp->ResultStorage(s,SetWord(ccresult));

  return 0;
}

/* 
 pp[_]minus predicate for two CcPoints. 

*/

static int pp_minusFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccp1;
  CcPoints *ccp2;
  CcPoints *ccresult;

  ccp1 = ((CcPoints *)args[0].addr);
  ccp2 = ((CcPoints *)args[1].addr);
  
  //if bboxes don't intersect, return first object
  if (!bboxesIntersect(ccp1->BboxTopLeftX,ccp1->BboxTopLeftY,ccp1->BboxBottomRightX,ccp1->BboxBottomRightY,
		       ccp2->BboxTopLeftX,ccp2->BboxTopLeftY,ccp2->BboxBottomRightX,ccp2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    ((CcPoints*)result.addr)->CopyFrom(ccp1);
    //delete (CcPoints*)result.addr;
    //((CcPoints*)result.addr) = ccp1;
    //qp->ResultStorage(s,SetWord(ccp1));
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcPoints*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_minus,ccp1->GetObj(),ccp2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsPoints,midPointsConst);
  }
  ccresult = new CcPoints(obj);
  //ccresult = new CcPoints(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_minus,ccp1->GetObj(),ccp2->GetObj()));
  ((CcPoints*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));
  //ccresult = callJMethod_PPP("pp_minus", ccp1, ccp2);
  //result.addr = ccresult;

  return 0;
}

/* 
 ll[_]minus predicate for two CcPoints. 

*/

static int ll_minusFun(Word* args, Word& result, int message, 
		       Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return first object
  if (!bboxesIntersect(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		       ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    //delete (CcLines*)result.addr;
    //((CcLines*)result.addr) = ccl1;
    //qp->ResultStorage(s,SetWord(ccl1));
    ((CcPoints*)result.addr)->CopyFrom(ccl1);
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  //delete (CcLines*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_minus,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //ccresult = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_minus,ccl1->GetObj(),ccl2->GetObj()));
  //qp->ResultStorage(s,SetWord(ccresult));
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //ccresult = callJMethod_LLL("ll_minus", ccl1, ccl2);
  //result.addr = ccresult;

  return 0;
}

/* 
 ll[_]common[_]border predicate for two CcLines. 

*/

static int ll_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;

  CcLines* ccl1 = ((CcLines *)args[0].addr);
  CcLines* ccl2 = ((CcLines *)args[1].addr);
  
  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccl1->BboxTopLeftX,ccl1->BboxTopLeftY,ccl1->BboxBottomRightX,ccl1->BboxBottomRightY,
		       ccl2->BboxTopLeftX,ccl2->BboxTopLeftY,ccl2->BboxBottomRightX,ccl2->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    //delete (CcLines*)result.addr;
    //qp->ResultStorage(s,SetWord(new CcLines(env->NewObject(clsLines,midLinesConst))));
    ccresult = new CcLines(env->NewObject(clsLines,midLinesConst));
    ((CcLines*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    //((CcLines*)result.addr) = new CcLines(env->NewObject(clsLines,midLinesConst));
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //delete (CcLines*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_common_border,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //ccresult = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_common_border,ccl1->GetObj(),ccl2->GetObj()));
  //qp->ResultStorage(s,SetWord(ccresult));
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //ccresult = callJMethod_LLL("ll_common_border", ccl1, ccl2);
  //result.addr = ccresult;

  return 0;
}

/* 
 lr[_]common[_]border predicate for CcLines and CcRegions. 

*/

static int lr_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;

  CcLines* ccl = ((CcLines *)args[0].addr);
  CcRegions* ccr = ((CcRegions *)args[1].addr);

  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY,
		       ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY)) {  
    result = qp->ResultStorage(s);
    //delete (CcLines*)result.addr;
    //qp->ResultStorage(s,SetWord(new CcLines(env->NewObject(clsLines,midLinesConst))));
    ccresult = new CcLines(env->NewObject(clsLines,midLinesConst));
    ((CcLines*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    //((CcLines*)result.addr) = new CcLines(env->NewObject(clsLines,midLinesConst));
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //delete (CcLines*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_common_border,ccl->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //ccresult = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_common_border,ccl->GetObj(),ccr->GetObj()));
  //qp->ResultStorage(s,SetWord(ccresult));
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //ccresult = callJMethod_LRL("lr_common_border", ccl, ccr);
  //result.addr = ccresult;

  return 0;
}

/* 
 rl[_]common[_]border predicate for CcLines and CcRegions. 

*/

static int rl_common_borderFun(Word *args, Word& result, 
			       int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);
  CcLines* ccl = ((CcLines *)args[1].addr);

  //if bboxes don't intersect, return empty object
  if (!bboxesIntersect(ccr->BboxTopLeftX,ccr->BboxTopLeftY,ccr->BboxBottomRightX,ccr->BboxBottomRightY,
		       ccl->BboxTopLeftX,ccl->BboxTopLeftY,ccl->BboxBottomRightX,ccl->BboxBottomRightY)) {
    result = qp->ResultStorage(s);
    //delete (CcLines*)result.addr;
    //qp->ResultStorage(s,SetWord(new CcLines(env->NewObject(clsLines,midLinesConst))));
    ccresult = new CcLines(env->NewObject(clsLines,midLinesConst));
    ((CcLines*)result.addr)->CopyFrom(ccresult);
    delete ccresult;
    ccresult = 0;
    //((CcLines*)result.addr) = new CcLines(env->NewObject(clsLines,midLinesConst));
    return 0;
  }
  //bboxes intersect, so prepare to invoke Java method
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //delete (CcLines*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_common_border,ccr->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //ccresult = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_common_border,ccr->GetObj(),ccl->GetObj()));
  //qp->ResultStorage(s,SetWord(ccresult));
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //ccresult = callJMethod_RLL("rl_common_border", ccr, ccl);
  //result.addr = ccresult;

  
  return 0;
}

/* 
 l[_]vertices predicate for CcLines. 

*/

static int l_verticesFun(Word *args, Word& result, 
			 int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccresult;

  CcLines* ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //delete (CcPoints*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEl_vertices,ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsPoints,midPointsConst);
  }
  ccresult = new CcPoints(obj);
  //ccresult = new CcPoints(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEl_vertices,ccl->GetObj()));
  //qp->ResultStorage(s,SetWord(ccresult));
  ((CcLines*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //ccresult = callJMethod_LP("l_vertices", ccl);
  //result.addr = ccresult;

  return 0;
}

/* 
 r[_]vertices predicate for CcRegions. 

*/

static int r_verticesFun(Word *args, Word& result, 
			 int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //delete (CcPoints*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEr_vertices,ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsPoints,midPointsConst);
  }
  ccresult = new CcPoints(obj);
  //ccresult = new CcPoints(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEr_vertices,ccr->GetObj()));
  ((CcPoints*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));
  //ccresult = callJMethod_RP("r_vertices", ccr);
  //result.addr = ccresult;
  
  return 0;
}

/* 
 l[_]interior predicate for CcLines. 

*/

static int l_interiorFun(Word *args, Word& result, 
			 int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccresult;

  CcLines* ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEl_interior,ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRegions,midRegionsConstVoid);
  }
  ccresult = new CcRegions(obj);
  ((CcRegions*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  
  return 0;
}

/* 
 r[_]contour predicate for CcRegions. 

*/

static int r_contourFun(Word *args, Word& result, 
			int message, Word& local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccresult;

  CcRegions* ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);
  //delete (CcLines*)result.addr;
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEr_contour,ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsLines,midLinesConst);
  }
  ccresult = new CcLines(obj);
  //ccresult = new CcLines(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEr_contour,ccr->GetObj()));
  ((CcRegions*)result.addr)->CopyFrom(ccresult);
  delete ccresult;
  ccresult = 0;
  //qp->ResultStorage(s,SetWord(ccresult));

  //ccresult = callJMethod_RL("r_contour", ccr);
  //result.addr = ccresult;

  return 0;
}

/* 
 p[_]no[_]of[_]components predicate for CcPoints. 

*/

static int p_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccp = ((CcPoints *)args[0].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  
  //((CcInt *)result.addr)->Set(true, callJMethod_PI("p_no_of_components", ccp)); 
  jint obj = env->CallStaticIntMethod(clsROSEAlgebra,midROSEp_no_of_components,ccp->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = -1;
  }
  ((CcInt*)result.addr)->Set(true,obj);
  //((CcInt*)result.addr)->Set(true,env->CallStaticIntMethod(clsROSEAlgebra,midROSEp_no_of_components,ccp->GetObj()));


  return 0;
}

/* 
 l[_]no[_]of[_]components predicate for CcLines. 

*/

static int l_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jint obj = env->CallStaticIntMethod(clsROSEAlgebra,midROSEl_no_of_components,ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = -1;
  }
  ((CcInt*)result.addr)->Set(true,obj);
  //((CcInt*)result.addr)->Set(true,env->CallStaticIntMethod(clsROSEAlgebra,midROSEl_no_of_components,ccl->GetObj()));

  return 0;
}

/* 
 r[_]no[_]of[_]components predicate for CcPoints. 

*/

static int r_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jint obj = env->CallStaticIntMethod(clsROSEAlgebra,midROSEr_no_of_components,ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = -1;
  }
  ((CcInt*)result.addr)->Set(true,obj);
  //((CcInt*)result.addr)->Set(true,env->CallStaticIntMethod(clsROSEAlgebra,midROSEr_no_of_components,ccr->GetObj()));

  return 0;
}

/* 
 pp[_]dist predicate for two CcPoints 

*/

static int pp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccp1 = ((CcPoints *)args[0].addr);
  CcPoints *ccp2 = ((CcPoints *)args[1].addr);

  if (!ccp1->GetObject()) ccp1->RestoreJavaObjectFromFLOB();
  if (!ccp2->GetObject()) ccp2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_dist,ccp1->GetObj(),ccp2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  ((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpp_dist,ccp1->GetObj(),ccp2->GetObj())));

  return 0;
}

/* 
 pl[_]dist predicate for CcPoints and CcLines 

*/

static int pl_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccp = ((CcPoints *)args[0].addr);
  CcLines *ccl = ((CcLines *)args[1].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpl_dist,ccp->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpl_dist,ccp->GetObj(),ccl->GetObj())));

  return 0;
}

/* 
 pr[_]dist predicate for CcPoints and CcRegions 
 
*/

static int pr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccp = ((CcPoints *)args[0].addr);
  CcRegions *ccr = ((CcRegions *)args[1].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpr_dist,ccp->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEpr_dist,ccp->GetObj(),ccr->GetObj())));

  return 0;
}

/* 
 lp[_]dist predicate for CcLines and CcPoints 

*/

static int lp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccl = ((CcLines *)args[0].addr);
  CcPoints *ccp = ((CcPoints *)args[1].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();
  
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSElp_dist,ccl->GetObj(),ccp->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSElp_dist,ccl->GetObj(),ccp->GetObj())));

  return 0;
}

/* 
 ll[_]dist predicate for two CcLines. 

*/

static int ll_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccl1 = ((CcLines *)args[0].addr);
  CcLines *ccl2 = ((CcLines *)args[1].addr);

  if (!ccl1->GetObject()) ccl1->RestoreJavaObjectFromFLOB();
  if (!ccl2->GetObject()) ccl2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_dist,ccl1->GetObj(),ccl2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEll_dist,ccl1->GetObj(),ccl2->GetObj())));

  return 0;
}

/* 
 lr[_]dist predicate for CcLines and CcRegions. 

*/

static int lr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccl = ((CcLines *)args[0].addr);
  CcRegions *ccr = ((CcRegions *)args[1].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_dist,ccl->GetObj(),ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSElr_dist,ccl->GetObj(),ccr->GetObj())));

  return 0;
}

/* 
 rp[_]dist predicate for CcRegions and CcPoints. 

*/

static int rp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccr = ((CcRegions *)args[0].addr);
  CcPoints *ccp = ((CcPoints *)args[1].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErp_dist,ccr->GetObj(),ccp->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErp_dist,ccr->GetObj(),ccp->GetObj())));

  return 0;
}

/* 
 rl[_]dist predicate for CcRegions and CcLines. 

*/

static int rl_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccr = ((CcRegions *)args[0].addr);
  CcLines *ccl = ((CcLines *)args[1].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();
  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_dist,ccr->GetObj(),ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErl_dist,ccr->GetObj(),ccl->GetObj())));

  return 0;
}

/* 
 rr[_]dist predicate for two CcRegions. 

*/

static int rr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccr1 = ((CcRegions *)args[0].addr);
  CcRegions *ccr2 = ((CcRegions *)args[1].addr);

  if (!ccr1->GetObject()) ccr1->RestoreJavaObjectFromFLOB();
  if (!ccr2->GetObject()) ccr2->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_dist,ccr1->GetObj(),ccr2->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSErr_dist,ccr1->GetObj(),ccr2->GetObj())));

  return 0;
}

/* 
 p[_]diameter predicate for a CcPoints. 

*/

static int p_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcPoints *ccp = ((CcPoints *)args[0].addr);

  if (!ccp->GetObject()) ccp->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEp_diameter,ccp->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEp_diameter,ccp->GetObj())));

  return 0;
}

/* 
 l[_]diameter predicate for a CcLines. 

*/

static int l_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  //debug(__LINE__);
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEl_diameter,ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEl_diameter,ccl->GetObj())));
  
  return 0;
}

/* 
 r[_]diameter predicate for a CcRegions. 

*/

static int r_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccr = ((CcRegions *)args[0].addr);
  
  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  //debug(__LINE__);
  result = qp->ResultStorage(s);	
  jobject obj = env->CallStaticObjectMethod(clsROSEAlgebra,midROSEr_diameter,ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = env->NewObject(clsRationalFactory,midRFConstRational,-1);
  }
  ((CcReal*)result.addr)->Set(true,convertRational(obj));
  //((CcReal*)result.addr)->Set(true,convertRational(env->CallStaticObjectMethod(clsROSEAlgebra,midROSEr_diameter,ccr->GetObj())));
  
  return 0;
}

/* 
 l[_]ength predicate for a CcLines. 

*/

static int l_lengthFun(Word *args, Word &result, int message,
		       Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcLines *ccl = ((CcLines *)args[0].addr);

  if (!ccl->GetObject()) ccl->RestoreJavaObjectFromFLOB();

  //debug(__LINE__);
  result = qp->ResultStorage(s);	
  jdouble obj = env->CallStaticDoubleMethod(clsROSEAlgebra,midROSEl_length,ccl->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = -1;
  }
  ((CcReal*)result.addr)->Set(true,obj);
  //((CcReal*)result.addr)->Set(true,env->CallStaticDoubleMethod(clsROSEAlgebra,midROSEl_length,ccl->GetObj()));
  
  return 0;
}

/* 
 r[_]aera predicate for a CcRegions. 

*/

static int r_areaFun(Word *args, Word &result, int message,
		     Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jdouble obj = env->CallStaticDoubleMethod(clsROSEAlgebra,midROSEr_area,ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = -1;
  }
  ((CcReal*)result.addr)->Set(true,obj);
  //((CcReal*)result.addr)->Set(true,env->CallStaticDoubleMethod(clsROSEAlgebra,midROSEr_area,ccr->GetObj()));
  
  return 0;
}

/* 
 r[_]perimeter predicate for a CcRegions. 

*/

static int r_perimeterFun(Word *args, Word &result, int message,
			  Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcRegions *ccr = ((CcRegions *)args[0].addr);

  if (!ccr->GetObject()) ccr->RestoreJavaObjectFromFLOB();

  result = qp->ResultStorage(s);	
  jdouble obj = env->CallStaticDoubleMethod(clsROSEAlgebra,midROSEr_perimeter,ccr->GetObj());
  if (env->ExceptionOccurred()) {
    RoseError();
    obj = -1;
  }
  ((CcReal*)result.addr)->Set(true,obj);
  //((CcReal*)result.addr)->Set(true,env->CallStaticDoubleMethod(clsROSEAlgebra,midROSEr_perimeter,ccr->GetObj()));
  
  return 0;
}


/*
 The setDeviationValue operation is used to set the deviation value for the ROSE algebra.

*/

static int setDeviationValueFun(Word *args, Word &result, int message,
				 Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcReal *realval = ((CcReal *)args[0].addr);
  result = qp->ResultStorage(s);
  
  callSetDeviationValue(realval);
  ((CcBool *)result.addr)->Set(true, true);

  return 0;
}
  
/*
 chooseTriangulator can be used to choose between different triangulators.

*/

static int chooseTriangulatorFun(Word *args, Word &result, int message,
			      Word &local, Supplier s) {
  if (DEBUG) cout << "entered " << __PRETTY_FUNCTION__ << endl;
  CcInt *intval = ((CcInt *)args[0].addr);
  result = qp->ResultStorage(s);
  
  callChooseTriangulator(intval);
  ((CcBool *)result.addr)->Set(true, true);

  return 0;
}

/*
 5.4 Definition of Operators

*/

 //predicates
ValueMapping equalMap[] = { pp_equalFun, ll_equalFun, rr_equalFun };
ValueMapping unequalMap[] = { pp_unequalFun, ll_unequalFun, rr_unequalFun };
ValueMapping disjointMap[] = { pp_disjointFun, ll_disjointFun, rr_disjointFun };
ValueMapping insideMap[] = { pr_insideFun, lr_insideFun, rr_insideFun };
ValueMapping intersectsMap[] = { ll_intersectsFun, lr_intersectsFun, rl_intersectsFun, rr_intersectsFun };
ValueMapping meetsMap[] = { ll_meetsFun, lr_meetsFun, rl_meetsFun, rr_meetsFun };
ValueMapping borderInCommonMap[] = { ll_border_in_commonFun, lr_border_in_commonFun, rl_border_in_commonFun, rr_border_in_commonFun };
ValueMapping onBorderOfMap[] = { pl_on_border_ofFun, pr_on_border_ofFun };

 //operations
ValueMapping intersectionMap[] = { pp_intersectionFun, ll_intersectionFun, rr_intersectionFun, rl_intersectionFun };
ValueMapping plusMap[] = { pp_plusFun, ll_plusFun, rr_plusFun };
ValueMapping minusMap[] = { pp_minusFun, ll_minusFun, rr_minusFun };
ValueMapping commonBorderMap[] = { ll_common_borderFun, lr_common_borderFun, rl_common_borderFun, rr_common_borderFun };
ValueMapping verticesMap[] = { l_verticesFun, r_verticesFun };
ValueMapping noOfComponentsMap[] = { p_no_of_componentsFun, l_no_of_componentsFun, r_no_of_componentsFun };
ValueMapping distMap[] = { pp_distFun, pl_distFun, pr_distFun, lp_distFun, ll_distFun, lr_distFun, rp_distFun, rl_distFun, rr_distFun };
ValueMapping diameterMap[] = { p_diameterFun, l_diameterFun, r_diameterFun };

ModelMapping ROSEDummyModel_7[] =
     {Operator::DummyModel,Operator::DummyModel,Operator::DummyModel,
      Operator::DummyModel,Operator::DummyModel,Operator::DummyModel,
      Operator::DummyModel};

const string equalSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>({points,line,region} x {points,line,region} -> bool</text--->"
"<text>o1 equal o2, where o1,o2 are objects of type {points,line,region}</text--->"
"<text>The equal predicate returns true, if both objects are equal.</text--->"
"<text>query Rhein equal Weser</text--->"
") )";

const string unequalSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>({points,line,region} x {points,line,region} -> bool</text--->"
"<text>o1 unequal o2, where o1,o2 are objects of type {points,line,region}</text--->"
"<text>The unequal predicate returns true, if both objects are not equal.</text--->"
"<text>query Rhein unequal Weser</text--->"
") )";

const string disjointSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>({points,line,region} x {points,line,region} -> bool</text--->"
"<text>o1 disjoint o2, where o1,o2 are objects of type {points,line,region}</text--->"
"<text>The disjoint predicate returns true, if both objects have no common points.</text--->"
"<text>query Rhein disjoint Weser</text--->"
") )";

const string insideSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>({points,line,region} x region -> bool</text--->"
"<text>o inside r, where o is an object of type {points,line,region} and r is a region.</text--->"
"<text>The inside predicate returns true, if o lies inside of r</text--->"
"<text>query Rhein inside LKMagdeburg</text--->"
") )";

const string areaDisjointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>region x egion -> bool</text--->"
"<text>r1 area_disjoint r2, where r1,r2 are of type region</text--->"
"<text>The area_disjoint predicate returns true, if r1,r2 have no common area.</text--->"
"<text>query LKMagdeburg area_disjoint SKKiel</text--->"
") )";

const string edgeDisjointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>egion x region -> bool</text--->"
"<text>r1 edge_disjoint r2, where r1,r2 are of type region</text--->"
"<text>The edge_disjoint predicate returns true, if r1,r2 have no common edges.</text--->"
"<text>query LKMagdeburg edge_disjoint SKKiel</text--->"
") )";

const string edgeInsideSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>region x region -> bool</text--->"
"<text>r1 edge_inside r2, where r1,r2 are of type region</text--->"
"<text>The edge_inside predicate returns true, if no edges of r1 are inside of r2.</text--->"
"<text>query LKMagdeburg edge_inside SKKiel</text--->"
") )";

const string vertexInsideSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>region x region -> bool</text--->"
"<text>r1 vertex_inside r2, where r1,r2 are of type region</text--->"
"<text>The vertex_inside predicate returns true, if no vertex of r1 is inside of r2.</text--->"
"<text>query LKMagdeburg vertex_inside SKKiel</text--->"
") )";

const string intersectsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>{line,region} x {line,region} -> bool</text--->"
"<text>o1 intersects o2, where o1,o2 are of type {line,region}</text--->"
"<text>The intersects predicate returns true, if o1,o2 intersect</text--->"
"<text>query Rhein intersects Main</text--->"
") )";

const string meetsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{line,region} x {line,region} -> bool</text--->"
"<text>o1 meets o2, where o1,o2 are of type {line,region}</text--->"
"<text>The meets predicate returns true, if o1,o2 meet in one point</text--->"
"<text>query Rhein meets Weser</text--->"
") )";

const string borderInCommonSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{line,region} x {line,region} -> bool</text--->"
"<text>o1 border_in_common o2, where o1,o2 are of type {line,region}</text--->"
"<text>The border_in_common predicate returns true, if o1,o2 have a common border.</text--->"
"<text>query LKSteinfurt border_in_common LKMagdeburg</text--->"
") )";

const string adjacentSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>region x region -> bool</text--->"
"<text>r1 adjacent r2, where r1,r2 are of type region</text--->"
"<text>The adjacent predicate returns true, if r1,r2 have a common border, but don't have a common area</text--->"
"<text>query LKSteinfurt adjacent LKMagdeburg</text--->"
") )";

const string enclosesSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>region x region -> bool</text--->"
"<text>r1 encloses r2, where r1,r2 are of type region</text--->"
"<text>The encloses predicate returns true, if r2 completely lies in holes of r1</text--->"
"<text>query LKOsnabrueck encloses SKOsnabrueck</text--->"
") )";

const string intersectionSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{points,line,region} x {points,line,region} -> {points,line,region}</text--->"
"<text>intersection(o1,o2), where both objects have to be of the same type. The result type of points x points is points, the result type of line x line is points and the result type of region x region is region.</text--->"
"<text>The intersection operation returns the geometric intersection of two objects.</text--->"
"<text>query intersection(Rhein,Weser)(</text--->"
") )";

const string plusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{points,line,region} x {points,line,region} -> {points,line,region}</text--->"
"<text>o1 plus o2, where o1,o2 and the result are of the same type {points,line,region}</text--->"
"<text>The plus operation returns the geometric sum of two objects</text--->"
"<text>query LKOsnabrueck plus SKOsnabrueck</text--->"
") )";

const string minusSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{points,line,region} x {points,line,region} -> {points,line,regin}</text--->"
"<text>o1 minus o2, where o1,o2 and the result are of the same type {points,line,region}</text--->"
"<text>The minus operation returns the geometric difference of two objects</text--->"
"<text>query LKSteinfurt minus LKOsnabrueck</text--->"
") )";

const string commonBorderSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{line,region} x {line,region} -> line</text--->"
"<text>common_border(o1,o2), where o1,o2 are of type {line,region}</text--->"
"<text>The common_border operation returns the common part of the borders or line objects, resp.</text--->"
"<text>query common_border(LKSteinfurt,SKOsnabrueck)</text--->"
") )";

const string onBorderOfSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>points x {line,region} -> bool</text--->"
"<text>p on_border_of o, where p is a points value and o is of type {line,region}</text--->"
"<text>The on_border_of predicate returns true, if p completely lies on o</text--->"
"<text>query Koeln on_border_of autobahn3</text--->"
") )";

const string verticesSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{line,region} -> points</text--->"
"<text>vertices (o), where o is of type {line,region}</text--->"
"<text>The vertices operation return the vertices of the o value</text--->"
"<text>query vertices (magdeburg)</text--->"
") )";

const string interiorSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>line -> region</text--->"
"<text>interior(l), where l has type line</text--->"
"<text>The interior operation returns the region that is enclosed by l.</text--->"
"<text>qeury interior(borderOfLKMagdeburg)</text--->"
") )";

const string contourSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>region -> line</text--->"
"<text>contour (r), where r has type region</text--->"
"<text>The contour operation returns the border or r without holes.</text--->"
"<text>query contour (magdeburg)</text--->"
") )";

const string noOfComponentsSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{points,line,region} -> int</text--->"
"<text>no_of_components (o), where o is of type {points,line,region}</text--->"
"<text>The no_of_components operation counts the number of connected components and returns that number.</text--->"
"<text>query no_of_components (Rhein)</text--->"
") )";

const string distSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{points,line,region} -> real</text--->"
"<text>dist(o1,o2), where o1,o2 are of type {points,line,region}</text--->"
"<text>The dist operation returns the Eucledian distance between o1,o2.</text--->"
"<text>query dist(LKSteinfurt,Rhein)</text--->"
") )";

const string diameterSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>{points,line,region} -> real</text--->"
"<text>diameter (o), where o is of type {points,line,region}</text--->"
"<text>The diameter operation returns the diameter of o.</text--->"
"<text>query diameter (Rhein)</text--->"
") )";

const string lengthSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>line -> real</text--->"
"<text>length (l), where l is of type line</text--->"
"<text>The length operation return the lenght of a line object.</text--->"
"<text>query length (Rhein)</text--->"
") )";

const string areaSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>region -> real</text--->"
"<text>area (r), where r is of type region</text--->"
"<text>The area operation returns a real value for the area of r.</text--->"
"<text>query area (magdeburg)</text--->"
") )";

const string perimeterSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>region -> real</text--->"
"<text>perimeter (r), where r is of type region</text--->"
"<text>The perimeter operation returns a real value for the perimeter of r.</text--->"
"<text>query perimeter (magdeburg)</text--->"
") )";

const string setDeviationValueSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>double -> bool</text--->"
"<text>setDeviationValue(d), d is a double value</text--->"
"<text>Sets the deviation value for the ROSE-Algebra.</text--->"
"<text>query setDeviationValue(0.000001)</text--->"
") )";

const string chooseTriangulatorSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" " 
"\"Example\" )"
"( <text>int -> bool</text--->"
"<text>chooseTriangulator(i), i is an integer value</text--->"
"<text>Chooses the triangulator of the ROSE algebra. 0 = Mehlhorn (default), 1 = Triangle, 2 = NetGen.</text--->"
"<text>query chooseTriangulator(1)</text--->"
") )";

/*
 Used to explain the signature and the meaning of the implemented operators.

*/

Operator ROSEequal (
		    "=",                  //name
		    equalSpec,            //specification
		    3,                    //number of functions
		    equalMap,             //value mapping 
		    ROSEDummyModel_7,     //model
		    equalSelect,          //selection function
		    equalTypeMap);        //type mapping function
		    
Operator ROSEunequal (
		      "#",                //name
		      unequalSpec,        //specification
		      3,                  //number of functions
		      unequalMap,         //value mapping
		      ROSEDummyModel_7,   //model
		      unequalSelect,       //selection function
		      unequalTypeMap);    //type mapping function

Operator ROSEdisjoint (
		       "disjoint",           //name
		       disjointSpec,         //specification
		       3,                    //number of functions
		       disjointMap,          //value mapping 
		       ROSEDummyModel_7,     //model
		       disjointSelect,       //selection function
		       disjointTypeMap);     //type mapping function

Operator ROSEinside (
		       "inside",             //name
		       insideSpec,           //specification
		       3,                    //number of functions
		       insideMap,            //value mapping 
		       ROSEDummyModel_7,     //model
		       insideSelect,         //selection function
		       insideTypeMap);       //type mapping function

Operator ROSEarea_disjoint (
		       "area_disjoint",          //name
		       areaDisjointSpec,        //specification
		       rr_area_disjointFun,      //value mapping
		       Operator::DummyModel,     //model
		       simpleSelect,             //selection function
		       ccregionsccregionsBool);  //type mapping function

Operator ROSEedge_disjoint (
		       "edge_disjoint",          //name
		       edgeDisjointSpec,        //specification
		       rr_edge_disjointFun,      //value mapping
		       Operator::DummyModel,     //model
		       simpleSelect,             //selection function
		       ccregionsccregionsBool);  //type mapping function

Operator ROSEedge_inside (
		       "edge_inside",            //name
		       edgeInsideSpec,          //specification
		       rr_edge_insideFun,        //value mapping
		       Operator::DummyModel,     //model
		       simpleSelect,             //selection function
		       ccregionsccregionsBool);  //type mapping function

Operator ROSEvertex_inside (
		       "vertex_inside",          //name
		       vertexInsideSpec,        //specification
		       rr_vertex_insideFun,      //value mapping
		       Operator::DummyModel,     //model
		       simpleSelect,             //selection function
		       ccregionsccregionsBool);  //type mapping function

Operator ROSEintersects (
			 "intersects",         //name
			 intersectsSpec,       //specification
			 4,                    //number of functions
			 intersectsMap,        //value mapping 
			 ROSEDummyModel_7,     //model
			 intersectsSelect,     //selection function
			 intersectsTypeMap);   //type mapping function

Operator ROSEmeets (
		    "meets",         //name
		    meetsSpec,       //specification
		    4,               //number of functions
		    meetsMap,        //value mapping 
		    ROSEDummyModel_7,//model
		    meetsSelect,     //selection function
		    meetsTypeMap);   //type mapping function

Operator ROSEborderInCommon (
			     "border_in_common",        //name
			     borderInCommonSpec,        //specification
			     4,                         //number of functions
			     borderInCommonMap,         //value mapping 
			     ROSEDummyModel_7,          //model
			     borderInCommonSelect,      //selection function
			     borderInCommonTypeMap);    //type mapping function

Operator ROSEonBorderOf (
			 "on_border_of",        //name
			 onBorderOfSpec,        //specification
			 2,                     //number of functions
			 onBorderOfMap,         //value mapping 
			 ROSEDummyModel_7,      //model
			 onBorderOfSelect,      //selection function
			 onBorderOfTypeMap);    //type mapping function

Operator ROSEintersection (
			   "intersection",        //name
			   intersectionSpec,      //specification
			   4,                     //number of functions
			   intersectionMap,       //value mapping 
			   ROSEDummyModel_7,      //model
			   intersectionSelect,    //selection function
			   intersectionTypeMap);  //type mapping function

Operator ROSEplus (
		   "plus",           //name
		   plusSpec,         //specification
		   3,                //number of functions
		   plusMap,          //value mapping 
		   ROSEDummyModel_7, //model
		   plusSelect,       //selection function
		   plusTypeMap);     //type mapping function

Operator ROSEminus (
		   "minus",           //name
		   minusSpec,         //specification
		   3,                 //number of functions
		   minusMap,          //value mapping 
		   ROSEDummyModel_7,  //model
		   minusSelect,       //selection function
		   minusTypeMap);     //type mapping function

Operator ROSEcommonBorder (
		   "common_border",     //name
		   commonBorderSpec,    //specification
		   4,                   //number of functions
		   commonBorderMap,     //value mapping 
		   ROSEDummyModel_7,    //model
		   commonBorderSelect,  //selection function
		   commonBorderTypeMap);//type mapping function

Operator ROSEvertices (
		       "vertices",        //name
		       verticesSpec,      //specification
		       2,                 //number of functions
		       verticesMap,       //value mapping 
		       ROSEDummyModel_7,  //model
		       verticesSelect,    //selection function
		       verticesTypeMap);  //type mapping function

Operator ROSEnoOfComponents (
			     "no_of_components",     //name
			     noOfComponentsSpec,     //specification
			     3,                      //number of functions
			     noOfComponentsMap,      //value mapping 
			     ROSEDummyModel_7,       //model
			     noOfComponentsSelect,   //selection function
			     noOfComponentsTypeMap); //type mapping function

Operator ROSEdist (
		   "dist",           //name
		   distSpec,         //specification
		   9,                //number of functions
		   distMap,          //value mapping 
		   ROSEDummyModel_7, //model
		   distSelect,       //selection function
		   distTypeMap);     //type mapping function

Operator ROSEadjacent (
		       "adjacent",               //name
		       adjacentSpec,             //specification
		       rr_adjacentFun,           //value mapping
		       Operator::DummyModel,     //model
		       simpleSelect,             //selection function
		       ccregionsccregionsBool);  //type mapping function

Operator ROSEencloses (
		       "encloses", 		//name
		       enclosesSpec,  		//specification ....
		       rr_enclosesFun,		//value mapping
		       Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
		       simpleSelect,		//trivial selection function 
		       ccregionsccregionsBool 	//type mapping 
		       );



Operator ROSEinterior (
		       "interior", 			//name
		       interiorSpec,  		        //specification ....
		       l_interiorFun,			//value mapping
		       Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
		       simpleSelect,			//trivial selection function 
		       cclinesccregions 		//type mapping 
		       );

Operator ROSEcontour (
		      "contour", 		//name
		      contourSpec,  		//specification ....
		      r_contourFun,		//value mapping
		      Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
		      simpleSelect,		//trivial selection function 
		      ccregionscclines 		//type mapping 
		      );

Operator ROSElength (
		     "length",	 		//name
		     lengthSpec,  		//specification ....
		     l_lengthFun,		//value mapping
		     Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
		     simpleSelect,		//trivial selection function 
		     cclinesDouble          	//type mapping 
		     );

Operator ROSEarea (
		 "area", 			//name
		 areaSpec,  			//specification ....
		 r_areaFun,			//value mapping
		 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
		 simpleSelect,			//trivial selection function 
		 ccregionsDouble          	//type mapping 
		 );

Operator ROSEperimeter (
			"perimeter", 		//name
			perimeterSpec,  	//specification ....
			r_perimeterFun,		//value mapping
			Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
			simpleSelect,		//trivial selection function 
			ccregionsDouble         //type mapping 
			);

Operator ROSEdiameter (
		       "diameter",              //name
		       diameterSpec,            //specification ...
		       3,                       //number of functions
		       diameterMap,             //value mapping
		       ROSEDummyModel_7,        //model
		       diameterSelect,          //selection function
		       diameterTypeMap);        //type mapping function

Operator setDeviationValue (
			    "setDeviationValue", 	//name
			    setDeviationValueSpec, 	//specification ....
			    setDeviationValueFun,	//value mapping
			    Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
			    simpleSelect,		//trivial selection function 
			    doubleBool             	//type mapping 
			    );

Operator chooseTriangulator (
			     "chooseTriangulator", 	//name
			     chooseTriangulatorSpec,	//specification ....
			     chooseTriangulatorFun,	//value mapping
			     Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
			     simpleSelect,		//trivial selection function 
			     intBool                 	//type mapping 
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
    AddOperator(&ROSEequal);
    AddOperator(&ROSEunequal);
    AddOperator(&ROSEdisjoint);
    AddOperator(&ROSEinside);
    AddOperator(&ROSEarea_disjoint);
    AddOperator(&ROSEedge_disjoint);
    AddOperator(&ROSEedge_inside);
    AddOperator(&ROSEvertex_inside);
    AddOperator(&ROSEintersects);
    AddOperator(&ROSEmeets);
    AddOperator(&ROSEborderInCommon);
    AddOperator(&ROSEonBorderOf);
    AddOperator(&ROSEintersection);
    AddOperator(&ROSEplus);
    AddOperator(&ROSEminus);
    AddOperator(&ROSEcommonBorder);
    AddOperator(&ROSEvertices);
    AddOperator(&ROSEnoOfComponents);
    AddOperator(&ROSEdist);
    AddOperator(&ROSEadjacent);
    AddOperator(&ROSEencloses);
    AddOperator(&ROSEinterior);
    AddOperator(&ROSEcontour);
    AddOperator(&ROSElength);
    AddOperator(&ROSEarea);
    AddOperator(&ROSEperimeter);
    AddOperator(&ROSEdiameter);

    AddOperator(&setDeviationValue);
    AddOperator(&chooseTriangulator);
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
  //Get the pointers env and jvm from the used Java Virtual Machine.
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

  jmethodID midSetDeviation = env->GetStaticMethodID(clsRationalFactory, "setDeviationDouble", "(D)V");
  if (midSetDeviation == 0) error(__FILE__,__LINE__);

  //set deviation value for 2DSACK package
  DEVIATION_VALUE = 0.0000000001;
  env->CallStaticVoidMethod(clsRationalFactory, midSetDeviation, DEVIATION_VALUE);

  jmethodID midReadDeviation = env->GetStaticMethodID(clsRationalFactory, "readDeviationDouble", "()D");
  if (midReadDeviation == 0) error(__FILE__,__LINE__);

  jmethodID midReadDeviationN = env->GetStaticMethodID(clsRationalFactory, "readDeviationDoubleNeg", "()D");
  if (midReadDeviationN == 0) error(__FILE__,__LINE__);
  
  jdouble resD = env->CallStaticDoubleMethod(clsRationalFactory, midReadDeviation);
  jdouble resDN = env->CallStaticDoubleMethod(clsRationalFactory, midReadDeviationN);

  cout << "2DSACK algebra: deviation values set to " << resD << "/" << resDN << endl;
  
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

  clsRect = env->FindClass("twodsack/setelement/datatype/basicdatatype/Rect");
  if (clsRect == 0) error(__FILE__,__LINE__);

  //define java methods, that are needed
  midRationalGetDouble = env->GetMethodID(clsRational, "getDouble", "()D");
  if (midRationalGetDouble == 0) error(__FILE__,__LINE__);
  midPMSToArray = env->GetMethodID(clsPointMultiSet, "toArray", "()[Ljava/lang/Object;");
  if (midPMSToArray == 0) error(__FILE__,__LINE__);
  midSegmentGetStartpoint = env->GetMethodID(clsSegment, "getStartpoint", "()Ltwodsack/setelement/datatype/basicdatatype/Point;");
  if (midSegmentGetStartpoint == 0) error(__FILE__,__LINE__);
  midSegmentGetEndpoint = env->GetMethodID(clsSegment, "getEndpoint", "()Ltwodsack/setelement/datatype/basicdatatype/Point;");
  if (midSegmentGetEndpoint == 0) error(__FILE__,__LINE__);
  midSMSToArray = env->GetMethodID(clsSegMultiSet, "toArray", "()[Ljava/lang/Object;");
  if (midSMSToArray == 0) error(__FILE__,__LINE__);
  midLLToArray = env->GetMethodID(clsLinkedList, "toArray", "()[Ljava/lang/Object;");
  if (midLLToArray == 0) error(__FILE__,__LINE__);
  midCLToArray = env->GetMethodID(clsCycleList, "toArray", "()[Ljava/lang/Object;");
  if (midCLToArray == 0) error(__FILE__,__LINE__);
  midCLLPToArray = env->GetMethodID(clsCycleListListPoints, "toArray", "()[Ljava/lang/Object;");
  if (midCLLPToArray == 0) error(__FILE__,__LINE__);
  midRegionsCyclesPoints = env->GetMethodID(clsRegions, "cyclesPoints", "()Ltwodsack/util/collection/CycleListListPoints;");
  if (midRegionsCyclesPoints == 0) error(__FILE__,__LINE__);
  midRFConstRational = env->GetStaticMethodID(clsRationalFactory, "constRational", "(II)Ltwodsack/util/number/Rational;");
  if (midRFConstRational == 0) error(__FILE__,__LINE__);
  midPointConstII = env->GetMethodID(clsPoint, "<init>", "(II)V");
  if (midPointConstII == 0) error(__FILE__,__LINE__);
  midPointConstDD = env->GetMethodID(clsPoint, "<init>", "(DD)V");
  if (midPointConstDD == 0) error(__FILE__,__LINE__);
  midPointConstRR = env->GetMethodID(clsPoint, "<init>", "(Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;)V");
  if (midPointConstRR == 0) error(__FILE__,__LINE__);
  midPointsConst = env->GetMethodID(clsPoints, "<init>", "()V");
  if (midPointsConst == 0) error(__FILE__,__LINE__);
  midPointsAddPoint = env->GetMethodID(clsPoints, "add", "(Ltwodsack/setelement/datatype/basicdatatype/Point;)V");
  if (midPointsAddPoint == 0) error(__FILE__,__LINE__);
  midSegmentConstIIII = env->GetMethodID(clsSegment, "<init>", "(IIII)V");
  if (midSegmentConstIIII == 0) error(__FILE__,__LINE__);
  midSegmentConstDDDD = env->GetMethodID(clsSegment, "<init>", "(DDDD)V");
  if (midSegmentConstDDDD == 0) error(__FILE__,__LINE__);
  midSegmentConstRRRR = env->GetMethodID(clsSegment, "<init>", "(Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;Ltwodsack/util/number/Rational;)V");
  if (midSegmentConstRRRR == 0) error(__FILE__,__LINE__);
  midLinesConst = env->GetMethodID(clsLines, "<init>", "()V");
  if (midLinesConst == 0) error(__FILE__,__LINE__);
  midLinesAddSegment = env->GetMethodID(clsLines, "add", "(Ltwodsack/setelement/datatype/basicdatatype/Segment;)V");
  if (midLinesAddSegment == 0) error(__FILE__,__LINE__);
  midSCConst = env->GetMethodID(clsSegmentComparator, "<init>", "()V");
  if (midSCConst == 0) error(__FILE__,__LINE__);
  midSMSConst = env->GetMethodID(clsSegMultiSet, "<init>", "(Ltwodsack/util/comparator/SegmentComparator;)V");
  if (midSMSConst == 0) error(__FILE__,__LINE__);
  midSMSAdd = env->GetMethodID(clsSegMultiSet, "add", "(Ltwodsack/setelement/datatype/basicdatatype/Segment;)V");
  if (midSMSAdd == 0) error(__FILE__,__LINE__);
  midSegmentConstPP = env->GetMethodID(clsSegment, "<init>", "(Ltwodsack/setelement/datatype/basicdatatype/Point;Ltwodsack/setelement/datatype/basicdatatype/Point;)V");
  if (midSegmentConstPP == 0) error(__FILE__,__LINE__);
  midRegionsConstVoid = env->GetMethodID(clsRegions, "<init>", "()V");
  if (midRegionsConstVoid == 0) error(__FILE__,__LINE__);
  midRegionsConstSMS = env->GetMethodID(clsRegions, "<init>", "(Ltwodsack/set/SegMultiSet;)V");
  if (midRegionsConstSMS == 0) error(__FILE__,__LINE__);
  midPointsCompare = env->GetMethodID(clsPoints,"compare","(LPoints;)I");
  if (midPointsCompare == 0) error(__FILE__,__LINE__);
  midPointsWriteToByteArray = env->GetMethodID(clsPoints,"writeToByteArray","()[B");
  if(midPointsWriteToByteArray == 0) error(__FILE__,__LINE__);
  midPointsReadFrom = env->GetStaticMethodID(clsPoints,"readFrom","([B)LPoints;");
  if(midPointsReadFrom == 0) error(__FILE__,__LINE__);
  midPointsPrint = env->GetMethodID(clsPoints,"print","()V");
  if(midPointsPrint == 0) error(__FILE__,__LINE__);
  midLinesRect = env->GetMethodID(clsLines,"rect","()Ltwodsack/setelement/datatype/basicdatatype/Rect;");
  if(midLinesRect == 0) error(__FILE__,__LINE__);
  midPointsRect = env->GetMethodID(clsPoints,"rect","()Ltwodsack/setelement/datatype/basicdatatype/Rect;");
  if(midPointsRect == 0) error(__FILE__,__LINE__);
  midLinesCompare = env->GetMethodID(clsLines,"compare","(LLines;)I");
  if (midLinesCompare == 0) error(__FILE__,__LINE__);
  midLinesWriteToByteArray = env->GetMethodID(clsLines,"writeToByteArray","()[B");
  if(midLinesWriteToByteArray == 0) error(__FILE__,__LINE__);
  midLinesReadFrom = env->GetStaticMethodID(clsLines,"readFrom","([B)LLines;");
  if(midLinesReadFrom == 0) error(__FILE__,__LINE__);
  midLinesPrint = env->GetMethodID(clsLines,"print","()V");
  if(midLinesPrint == 0) error (__FILE__,__LINE__);
  midRegionsCompare = env->GetMethodID(clsRegions,"compare","(LRegions;)I");
  if (midRegionsCompare == 0) error (__FILE__,__LINE__);
  midRegionsWriteToByteArray = env->GetMethodID(clsRegions,"writeToByteArray","()[B");
  if(midRegionsWriteToByteArray == 0) error(__FILE__,__LINE__);
  midRegionsReadFrom = env->GetStaticMethodID(clsRegions,"readFrom","([B)LRegions;");
  if(midRegionsReadFrom == 0) error(__FILE__,__LINE__);
  midRegionsPrint = env->GetMethodID(clsRegions,"print","()V");
  if(midRegionsPrint == 0) error(__FILE__,__LINE__);
  midRationalGetNumerator = env->GetMethodID(clsRational, "getNumerator", "()I");
  if (midRationalGetNumerator == 0) error(__FILE__, __LINE__);
  midRationalGetDenominator = env->GetMethodID(clsRational, "getDenominator", "()I");
  if (midRationalGetDenominator == 0) error(__FILE__, __LINE__);
  midROSESetDeviationValue = env->GetStaticMethodID(clsROSEAlgebra, "setDeviationValue", "(D)V");
  if (midROSESetDeviationValue == 0) error(__FILE__,__LINE__);
  midROSEChooseTriangulator = env->GetStaticMethodID(clsROSEAlgebra, "chooseTriangulator", "(I)V");
  if (midROSEChooseTriangulator == 0) error(__FILE__,__LINE__);
  midRegionsRect = env->GetMethodID(clsRegions,"rect","()Ltwodsack/setelement/datatype/basicdatatype/Rect;");
  if (midRegionsRect == 0) error(__FILE__,__LINE__);
  midRectGetTopLeftX = env->GetMethodID(clsRect,"getTopLeftX","()D");
  if (midRectGetTopLeftX == 0) error (__FILE__, __LINE__);
  midRectGetTopLeftY = env->GetMethodID(clsRect,"getTopLeftY","()D");
  if (midRectGetTopLeftY == 0) error (__FILE__, __LINE__);
  midRectGetBottomRightX = env->GetMethodID(clsRect,"getBottomRightX","()D");
  if (midRectGetBottomRightX == 0) error (__FILE__, __LINE__);
  midRectGetBottomRightY = env->GetMethodID(clsRect,"getBottomRightY","()D");
  if (midRectGetBottomRightY == 0) error (__FILE__, __LINE__);

  //define ROSE function mid

  midROSEp_diameter = env->GetStaticMethodID(clsROSEAlgebra,"p_diameter","(LPoints;)Ltwodsack/util/number/Rational;");
  if (midROSEp_diameter == 0) error(__FILE__,__LINE__);
  midROSEp_no_of_components = env->GetStaticMethodID(clsROSEAlgebra,"p_no_of_components","(LPoints;)I");
  if (midROSEp_no_of_components == 0) error(__FILE__,__LINE__);

  midROSEl_diameter = env->GetStaticMethodID(clsROSEAlgebra,"l_diameter","(LLines;)Ltwodsack/util/number/Rational;");
  if (midROSEl_diameter == 0) error(__FILE__,__LINE__);
  midROSEl_interior = env->GetStaticMethodID(clsROSEAlgebra,"l_interior","(LLines;)LRegions;");
  if (midROSEl_interior == 0) error(__FILE__,__LINE__);
  midROSEl_length = env->GetStaticMethodID(clsROSEAlgebra,"l_length","(LLines;)D");
  if (midROSEl_length == 0) error(__FILE__,__LINE__);
  midROSEl_no_of_components = env->GetStaticMethodID(clsROSEAlgebra,"l_no_of_components","(LLines;)I");
  if (midROSEl_no_of_components == 0) error(__FILE__,__LINE__);
  midROSEl_vertices = env->GetStaticMethodID(clsROSEAlgebra,"l_vertices","(LLines;)LPoints;");
  if (midROSEl_vertices == 0) error(__FILE__,__LINE__);

  midROSEr_area = env->GetStaticMethodID(clsROSEAlgebra,"r_area","(LRegions;)D");
  if (midROSEr_area == 0) error(__FILE__,__LINE__);
  midROSEr_contour = env->GetStaticMethodID(clsROSEAlgebra,"r_contour","(LRegions;)LLines;");
  if (midROSEr_contour == 0) error(__FILE__,__LINE__);
  midROSEr_diameter = env->GetStaticMethodID(clsROSEAlgebra,"r_diameter","(LRegions;)Ltwodsack/util/number/Rational;");
  if (midROSEr_diameter == 0) error(__FILE__,__LINE__);
  midROSEr_no_of_components = env->GetStaticMethodID(clsROSEAlgebra,"r_no_of_components","(LRegions;)I");
  if (midROSEr_no_of_components == 0) error(__FILE__,__LINE__);
  midROSEr_perimeter = env->GetStaticMethodID(clsROSEAlgebra,"r_perimeter","(LRegions;)D");
  if (midROSEr_perimeter == 0) error(__FILE__,__LINE__);
  midROSEr_vertices = env->GetStaticMethodID(clsROSEAlgebra,"r_vertices","(LRegions;)LPoints;");
  if (midROSEr_vertices == 0) error(__FILE__,__LINE__);

  midROSEpp_disjoint = env->GetStaticMethodID(clsROSEAlgebra,"pp_disjoint","(LPoints;LPoints;)Z");
  if (midROSEpp_disjoint == 0) error(__FILE__,__LINE__);
  midROSEpp_dist = env->GetStaticMethodID(clsROSEAlgebra,"pp_dist","(LPoints;LPoints;)Ltwodsack/util/number/Rational;");
  if (midROSEpp_dist == 0) error(__FILE__,__LINE__);
  midROSEpp_equal = env->GetStaticMethodID(clsROSEAlgebra,"pp_equal","(LPoints;LPoints;)Z");
  if (midROSEpp_equal == 0) error(__FILE__,__LINE__);
  midROSEpp_plus = env->GetStaticMethodID(clsROSEAlgebra,"pp_plus","(LPoints;LPoints;)LPoints;");
  if (midROSEpp_plus == 0) error(__FILE__,__LINE__);
  midROSEpp_minus = env->GetStaticMethodID(clsROSEAlgebra,"pp_minus","(LPoints;LPoints;)LPoints;");
  if (midROSEpp_minus == 0) error(__FILE__,__LINE__);
  midROSEpp_unequal = env->GetStaticMethodID(clsROSEAlgebra,"pp_unequal","(LPoints;LPoints;)Z");
  if (midROSEpp_unequal == 0) error(__FILE__,__LINE__);
  midROSEpp_intersection = env->GetStaticMethodID(clsROSEAlgebra,"pp_intersection","(LPoints;LPoints;)LPoints;");
  if (midROSEpp_intersection == 0) error(__FILE__,__LINE__);

  midROSEll_border_in_common = env->GetStaticMethodID(clsROSEAlgebra,"ll_border_in_common","(LLines;LLines;)Z");
  if (midROSEll_border_in_common == 0) error(__FILE__,__LINE__);
  midROSEll_common_border = env->GetStaticMethodID(clsROSEAlgebra,"ll_common_border","(LLines;LLines;)LLines;");
  if (midROSEll_common_border == 0) error(__FILE__,__LINE__);
  midROSEll_disjoint = env->GetStaticMethodID(clsROSEAlgebra,"ll_disjoint","(LLines;LLines;)Z");
  if (midROSEll_disjoint == 0) error(__FILE__,__LINE__);
  midROSEll_dist = env->GetStaticMethodID(clsROSEAlgebra,"ll_dist","(LLines;LLines;)Ltwodsack/util/number/Rational;");
  if (midROSEll_dist == 0) error(__FILE__,__LINE__);
  midROSEll_equal = env->GetStaticMethodID(clsROSEAlgebra,"ll_equal","(LLines;LLines;)Z");
  if (midROSEll_equal == 0) error(__FILE__,__LINE__);
  midROSEll_intersection = env->GetStaticMethodID(clsROSEAlgebra,"ll_intersection","(LLines;LLines;)LPoints;");
  if (midROSEll_intersection == 0) error(__FILE__,__LINE__);
  midROSEll_intersects = env->GetStaticMethodID(clsROSEAlgebra,"ll_intersects","(LLines;LLines;)Z");
  if (midROSEll_intersects == 0) error(__FILE__,__LINE__);
  midROSEll_meets = env->GetStaticMethodID(clsROSEAlgebra,"ll_meets","(LLines;LLines;)Z");
  if (midROSEll_meets == 0) error(__FILE__,__LINE__);
  midROSEll_minus = env->GetStaticMethodID(clsROSEAlgebra,"ll_minus","(LLines;LLines;)LLines;");
  if (midROSEll_minus == 0) error(__FILE__,__LINE__);
  midROSEll_plus = env->GetStaticMethodID(clsROSEAlgebra,"ll_plus","(LLines;LLines;)LLines;");
  if (midROSEll_plus == 0) error(__FILE__,__LINE__);
  midROSEll_unequal = env->GetStaticMethodID(clsROSEAlgebra,"ll_unequal","(LLines;LLines;)Z");
  if (midROSEll_unequal == 0) error(__FILE__,__LINE__);

  midROSErr_adjacent = env->GetStaticMethodID(clsROSEAlgebra,"rr_adjacent","(LRegions;LRegions;)Z");
  if (midROSErr_adjacent == 0) error(__FILE__,__LINE__);
  midROSErr_area_disjoint = env->GetStaticMethodID(clsROSEAlgebra,"rr_area_disjoint","(LRegions;LRegions;)Z");
  if (midROSErr_area_disjoint == 0) error(__FILE__,__LINE__);
  midROSErr_border_in_common = env->GetStaticMethodID(clsROSEAlgebra,"rr_border_in_common","(LRegions;LRegions;)Z");
  if (midROSErr_border_in_common == 0) error(__FILE__,__LINE__);
  midROSErr_common_border = env->GetStaticMethodID(clsROSEAlgebra,"rr_common_border","(LRegions;LRegions;)LLines;");
  if (midROSErr_common_border == 0) error(__FILE__,__LINE__);
  midROSErr_disjoint = env->GetStaticMethodID(clsROSEAlgebra,"rr_disjoint","(LRegions;LRegions;)Z");
  if (midROSErr_disjoint == 0) error(__FILE__,__LINE__);
  midROSErr_dist = env->GetStaticMethodID(clsROSEAlgebra,"rr_dist","(LRegions;LRegions;)Ltwodsack/util/number/Rational;");
  if (midROSErr_dist == 0) error(__FILE__,__LINE__);
  midROSErr_edge_disjoint = env->GetStaticMethodID(clsROSEAlgebra,"rr_edge_disjoint","(LRegions;LRegions;)Z");
  if (midROSErr_edge_disjoint == 0) error(__FILE__,__LINE__);
  midROSErr_edge_inside = env->GetStaticMethodID(clsROSEAlgebra,"rr_edge_inside","(LRegions;LRegions;)Z");
  if (midROSErr_edge_inside == 0) error(__FILE__,__LINE__);
  midROSErr_encloses = env->GetStaticMethodID(clsROSEAlgebra,"rr_encloses","(LRegions;LRegions;)Z");
  if (midROSErr_encloses == 0) error(__FILE__,__LINE__);
  midROSErr_equal = env->GetStaticMethodID(clsROSEAlgebra,"rr_equal","(LRegions;LRegions;)Z");
  if (midROSErr_equal == 0) error(__FILE__,__LINE__);
  midROSErr_inside = env->GetStaticMethodID(clsROSEAlgebra,"rr_inside","(LRegions;LRegions;)Z");
  if (midROSErr_inside == 0) error(__FILE__,__LINE__);
  midROSErr_intersection = env->GetStaticMethodID(clsROSEAlgebra,"rr_intersection","(LRegions;LRegions;)LRegions;");
  if (midROSErr_intersection == 0) error(__FILE__,__LINE__);
  midROSErr_intersects = env->GetStaticMethodID(clsROSEAlgebra,"rr_intersects","(LRegions;LRegions;)Z");
  if (midROSErr_intersects == 0) error(__FILE__,__LINE__);
  midROSErr_meets = env->GetStaticMethodID(clsROSEAlgebra,"rr_meets","(LRegions;LRegions;)Z");
  if (midROSErr_meets == 0) error(__FILE__,__LINE__);
  midROSErr_minus = env->GetStaticMethodID(clsROSEAlgebra,"rr_minus","(LRegions;LRegions;)LRegions;");
  if (midROSErr_minus == 0) error(__FILE__, __LINE__);
  midROSErr_plus = env->GetStaticMethodID(clsROSEAlgebra,"rr_plus","(LRegions;LRegions;)LRegions;");
  if (midROSErr_plus == 0) error(__FILE__,__LINE__);
  midROSErr_unequal = env->GetStaticMethodID(clsROSEAlgebra,"rr_unequal","(LRegions;LRegions;)Z");
  if (midROSErr_unequal == 0) error(__FILE__,__LINE__);
  midROSErr_vertex_inside = env->GetStaticMethodID(clsROSEAlgebra,"rr_vertex_inside","(LRegions;LRegions;)Z");
  if (midROSErr_vertex_inside == 0) error(__FILE__,__LINE__);

  midROSEpl_dist = env->GetStaticMethodID(clsROSEAlgebra,"pl_dist","(LPoints;LLines;)Ltwodsack/util/number/Rational;");
  if (midROSEpl_dist == 0) error(__FILE__,__LINE__);
  midROSEpl_on_border_of = env->GetStaticMethodID(clsROSEAlgebra,"pl_on_border_of","(LPoints;LLines;)Z");
  if (midROSEpl_on_border_of == 0) error(__FILE__,__LINE__);

  midROSEpr_dist = env->GetStaticMethodID(clsROSEAlgebra,"pr_dist","(LPoints;LRegions;)Ltwodsack/util/number/Rational;");
  if (midROSEpr_dist == 0) error(__FILE__,__LINE__);
  midROSEpr_inside = env->GetStaticMethodID(clsROSEAlgebra,"pr_inside","(LPoints;LRegions;)Z");
  if (midROSEpr_inside == 0) error(__FILE__,__LINE__);
  midROSEpr_on_border_of = env->GetStaticMethodID(clsROSEAlgebra,"pr_on_border_of","(LPoints;LRegions;)Z");
  if (midROSEpr_on_border_of == 0) error(__FILE__,__LINE__);
  
  midROSElp_dist = env->GetStaticMethodID(clsROSEAlgebra,"lp_dist","(LLines;LPoints;)Ltwodsack/util/number/Rational;");
  if (midROSElp_dist == 0) error(__FILE__,__LINE__);

  midROSErp_dist = env->GetStaticMethodID(clsROSEAlgebra,"rp_dist","(LRegions;LPoints;)Ltwodsack/util/number/Rational;");
  if (midROSErp_dist == 0) error(__FILE__,__LINE__);

  midROSElr_border_in_common = env->GetStaticMethodID(clsROSEAlgebra,"lr_border_in_common","(LLines;LRegions;)Z");
  if (midROSElr_border_in_common == 0) error(__FILE__,__LINE__);
  midROSElr_common_border = env->GetStaticMethodID(clsROSEAlgebra,"lr_common_border","(LLines;LRegions;)LLines;");
  if (midROSElr_common_border == 0) error(__FILE__,__LINE__);
  midROSElr_dist = env->GetStaticMethodID(clsROSEAlgebra,"lr_dist","(LLines;LRegions;)Ltwodsack/util/number/Rational;");
  if (midROSElr_dist == 0) error(__FILE__,__LINE__);
  midROSElr_inside = env->GetStaticMethodID(clsROSEAlgebra,"lr_inside","(LLines;LRegions;)Z");
  if (midROSElr_inside == 0) error(__FILE__,__LINE__);
  midROSElr_intersects = env->GetStaticMethodID(clsROSEAlgebra,"lr_intersects","(LLines;LRegions;)Z");
  if (midROSElr_intersects == 0) error(__FILE__,__LINE__);
  midROSElr_meets = env->GetStaticMethodID(clsROSEAlgebra,"lr_meets","(LLines;LRegions;)Z");
  if (midROSElr_meets == 0) error(__FILE__,__LINE__);

  midROSErl_border_in_common = env->GetStaticMethodID(clsROSEAlgebra,"rl_border_in_common","(LRegions;LLines;)Z");
  if (midROSErl_border_in_common == 0) error(__FILE__,__LINE__);
  midROSErl_common_border = env->GetStaticMethodID(clsROSEAlgebra,"rl_common_border","(LRegions;LLines;)LLines;");
  if (midROSErl_common_border == 0) error(__FILE__,__LINE__);
  midROSErl_dist = env->GetStaticMethodID(clsROSEAlgebra,"rl_dist","(LRegions;LLines;)Ltwodsack/util/number/Rational;");
  if (midROSErl_dist == 0) error(__FILE__,__LINE__);
  midROSErl_intersection = env->GetStaticMethodID(clsROSEAlgebra,"rl_intersection","(LRegions;LLines;)LLines;");
  if (midROSErl_intersection == 0) error(__FILE__,__LINE__);
  midROSErl_intersects = env->GetStaticMethodID(clsROSEAlgebra,"rl_intersects","(LRegions;LLines;)Z");
  if (midROSErl_intersects == 0) error(__FILE__,__LINE__);
  midROSErl_meets = env->GetStaticMethodID(clsROSEAlgebra,"rl_meets","(LRegions;LLines;)Z");
  if (midROSErl_meets == 0) error(__FILE__,__LINE__);

  //define java fields, that are needed
  fidRationalX = env->GetFieldID(clsPoint, "x", "Ltwodsack/util/number/Rational;");
  if (fidRationalX == 0) error(__FILE__,__LINE__);
  fidRationalY = env->GetFieldID(clsPoint, "y", "Ltwodsack/util/number/Rational;");
  if (fidRationalY == 0) error(__FILE__,__LINE__);
  fidPointsPointSet = env->GetFieldID(clsPoints, "pointset", "Ltwodsack/set/PointMultiSet;");
  if (fidPointsPointSet == 0) error(__FILE__,__LINE__);
  fidLinesSegmentSet = env->GetFieldID(clsLines, "segset", "Ltwodsack/set/SegMultiSet;");
  if (fidLinesSegmentSet == 0) error(__FILE__,__LINE__);
  
  clsRoseError = env->FindClass("RoseAlgebraError");
  if (clsRoseError == 0) error(__FILE__,__LINE__);
  fidROSEError = env->GetFieldID(clsRoseError, "errorMessage", "Ljava/lang/String;");
  if (fidROSEError == 0) error(__FILE__,__LINE__);


  nl = nlRef;
  qp = qpRef;

  return (&roseAlgebra);
}
