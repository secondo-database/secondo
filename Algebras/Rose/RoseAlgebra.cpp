/*
[1] Rose Algebra

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

1 Preliminaries

1.1 Includes and global declarations

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"	//needed because we return a CcBool in an op.
#include "StandardAttribute.h"

static NestedList* nl;
static QueryProcessor* qp;

#include "Attribute.h"
#include <jni.h>
#include <JVMInit.h>

/* The JVMInitializer provides access to a pointer to
   the JNI Environment and to a pointer to the
   Java Virtual Machine. */
static JVMInitializer *jvminit = 0;

/* Pointer to the JVM Environment and JVM. */
static JNIEnv *env;
static JavaVM *jvm;

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

/* this function prints an error message including the line
	where the error occured. Should never invoked normally. */
static void error(char *name, int line) {
  cerr << "Error in " << name << " in line: " << line << "." << endl;
  exit(1);
}

/* this function prints an error message including the line
   where the error occured. This method is used for calls
   to Java methods. The name of the Java method which
   should invoked is also printed. */
static void error(char *filename, char *classname, int line) {
  cerr << "Error in " << filename << " in line: " << line
       << ". " << endl << "Requested Java method couldn't invoked: "
       << classname << "." << endl;
  exit(1);
}

/*
1.3 Dummy Functions

Not interesting, but needed in the definition of a type constructor.

*/

static void *DummyCast(void *addr) {
  return 0;
}

/*
2 Type Constructor ~Points~

2.1 Data Structure - Class ~CcPoints~

*/

class CcPoints {
private:
  jclass cls;
  jobject obj;

public:
  /* Inherited methods of Attribute */
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined();
  int Sizeof();
  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  size_t HashValue();

  /* This contructor takes just one serial string
     of the underlying java object and its length. */
  CcPoints(char *serial, int len);
  /* This constructor takes the nested list representation
     of CcPoints and recovers the underlying java object with
     help of this data. */
  CcPoints(const ListExpr &le);
  /* This constructor takes a pointer to a java object which is
     already created. */
  CcPoints(const jobject jobj);
  /* This constructor creates an empty CcPoints object. */
  CcPoints();
  /* retrieves the nested list representation of the underlying
     java object. */
  bool GetNL(ListExpr &le);
  /* Destructor of CcPoints. This destructor destroys also the
     object inside the JVM. */
  ~CcPoints();
  /* Returns the pointer to the proper java objet. */
  jobject GetObj();

};

/* Inherited method of StandardAttribute */
int CcPoints::Compare(Attribute *attr) {
  return 0;
}

/* Inherited method of StandardAttribute */
Attribute *CcPoints::Clone() {
  return 0;
}

/* Inherited method of StandardAttribute */
bool CcPoints::IsDefined() {
  return true;
}

/* Inherited method of StandardAttribute */
int CcPoints::Sizeof() {
  return 0;
}

/* Inherited method of StandardAttribute */
void CcPoints::SetDefined(bool Defined) {
}

/* Inherited method of StandardAttribute */
void CcPoints::CopyFrom(StandardAttribute* right) {
}

/* Inherited method of StandardAttribute */
size_t CcPoints::HashValue() {
  return 0;
}

/* This contructor takes just one serial string
   of the underlying java object and its length. */
CcPoints::CcPoints(char *serial, int len) {
  jbyteArray jbarr = 0;
  jmethodID mid_Rose;
  jclass cls_Rose;

  /* Get the Class */
  cls = env->FindClass("Points");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Allocate a new byte array inside the VM. */
  jbarr = env->NewByteArray(len);
  if (jbarr == 0) error(__FILE__, __LINE__);

  /* Store the data into this array. */
  env->SetByteArrayRegion(jbarr, 0, len, (jbyte *)serial);

  /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  /* Get the method ID of imprt_arr */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "imprt_arr",
				    "([B)Ljava/lang/Object;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Call the static imprt_arr method */
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jbarr);
  if (obj == 0) error(__FILE__, __LINE__);

}

/* This constructor takes the nested list representation
   of CcPoints and recovers the underlying java object with
   help of this data. */

CcPoints::CcPoints(const ListExpr &le) {
  char *nlchar;
  string nlstr;
  bool succ;
  int len;
  jstring jstr;
  jobject jnl;
  jclass cls_Rose;
  jmethodID mid_Rose;
  jmethodID mid_Rose2;

  /* Get the Class */
  cls = env->FindClass("Points");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Get the c_string of le */
  succ = nl->WriteToString(nlstr, le);
  if (succ == false) error(__FILE__, __LINE__);

  nlchar = (char *)nlstr.c_str();
  len = strlen(nlchar);

  jstr = env->NewStringUTF(nlchar);
  if (jstr == 0) error(__FILE__, __LINE__);

  /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  /* Get the method ID of StringToListExpr */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "StringToListExpr",
				    "(Ljava/lang/String;)Lsj/lang/ListExpr;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Call the static StringToListExpr method */
  jnl = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jstr);
  if (jnl == 0) error(__FILE__, __LINE__);

  /* Get the method ID of imprt_Points */
  mid_Rose2 = env->GetStaticMethodID(cls_Rose, "imprt_Points",
				     "(Lsj/lang/ListExpr;)LPoints;");
  if (mid_Rose2 == 0) error(__FILE__, __LINE__);

  /* recreate the Points object from the (Java-) ListExpr */
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose2, jnl);
  if (obj == 0) error(__FILE__, __LINE__);
}

/* This constructor takes a pointer to a java object which is
   already created. */

CcPoints::CcPoints(const jobject jobj) {
  /* Get the Class Points */
  cls = env->FindClass("Points");
  if (cls == 0) error(__FILE__, __LINE__);
  obj = jobj;
}

/* This constructor creates an empty CcPoints object. */
CcPoints::CcPoints() {
  jmethodID mid;

  /* Get the Class */
  cls = env->FindClass("Points");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(cls, "<init>", "()V");
  if (mid == 0) error(__FILE__,__LINE__);

  /* Create a Java-object Point. */
  obj = env->NewObject(cls, mid);
  if (obj == 0) error(__FILE__,__LINE__);
}


/* retrieves the nested list representation of the underlying
     java object. */

bool CcPoints::GetNL(ListExpr& le) {
  jclass cls_Rose;
  jmethodID mid_Rose;
  jmethodID mid_Rose2;
  jobject jnl;
  jstring jnljstr;
  char *jnlstr;
  string cppstr;

 /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  mid_Rose = env->GetStaticMethodID(cls_Rose, "StringToListExpr",
				    "(Ljava/lang/String;)Lsj/lang/ListExpr;");
  /* Get the method ID of exprt_nl */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "exprt_nl",
				    "(Ljava/lang/Object;)Lsj/lang/ListExpr;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Get the java nested list of this object */
  jnl = env->CallStaticObjectMethod(cls_Rose, mid_Rose, obj);
  if (jnl == 0) error(__FILE__, __LINE__);

  /* Get the method ID of ListExprToString */
  mid_Rose2 = env->GetStaticMethodID(cls_Rose, "ListExprToString",
				     "(Lsj/lang/ListExpr;)Ljava/lang/String;");
  if (mid_Rose2 == 0) error(__FILE__, __LINE__);

  /* Retrieve the java string representation of the nested list. */
  jnljstr = (jstring)env->CallStaticObjectMethod(cls_Rose, mid_Rose2, jnl);
  if (jnlstr == 0) error(__FILE__, __LINE__);

  /* Get the c string of above java string */
  jnlstr = (char *)env->GetStringUTFChars(jnljstr, 0);
  if (jnlstr == 0) error(__FILE__, __LINE__);

  cppstr = jnlstr;

  return nl->ReadFromString(cppstr, le);
}

/* Destructor of CcPoints. This destructor destroys also the
     object inside the JVM. */
CcPoints::~CcPoints() {
  //env->DeleteLocalRef(obj);
}

/* Returns the pointer to the proper java objet. */

jobject CcPoints::GetObj() {
  return obj;
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

static Word CreateCcPoints(const ListExpr typeInfo) {
  return (SetWord(new CcPoints()));
}

static void DeleteCcPoints(Word &w) {
  delete ((CcPoints *)w.addr);
  w.addr = 0;
}

static void CloseCcPoints(Word & w) {
  delete (CcPoints *)w.addr;
  w.addr = 0;
}

static Word CloneCcPoints(const Word &w) {
  return SetWord(((CcPoints *)w.addr)->Clone());
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
       nl->StringAtom("a ccpoints is a set of points which coordinates are rational numbers."))));
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~ccpoints~ does not have arguments, this is trivial.

*/

static bool CheckCcPoints( ListExpr type, ListExpr& errorInfo ) {
  return (nl->IsEqual(type, "ccpoints"));
}

/*
2.6 Creation of the Type Constructor Instance

*/

TypeConstructor ccpoints
(
 "ccpoints",
 // name
 PointsProperty,
 // property function describing the signature
 OutCcPoints,
 // out function
 InCcPoints,
 // in function
 CreateCcPoints,
 // object creation
 DeleteCcPoints,
 // object deletion
 0,
 // object open
 0,
 // object save
 CloseCcPoints,
 // object close
 CloneCcPoints,
 // object clone
 DummyCast,
 // cast function
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

*/

class CcLines {
private:
  jclass cls;
  jobject obj;

public:
  /* Inherited methods of Attribute */
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined();
  int Sizeof();
  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  size_t HashValue();

  /* This contructor takes just one serial string
     of the underlying java object and its length. */
  CcLines(char *serial, int len);
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
  bool GetNL(ListExpr &le);
  /* Destructor of CcLines. This destructor destroys also the
     object inside the JVM. */
  ~CcLines();
  /* Returns the pointer to the proper java objet. */
  jobject GetObj();
};

/* Inherited method of StandardAttribute */
int CcLines::Compare(Attribute *attr) {
  return 0;
}

/* Inherited method of StandardAttribute */
Attribute *CcLines::Clone() {
  return 0;
}

/* Inherited method of StandardAttribute */
bool CcLines::IsDefined() {
  return true;
}

/* Inherited method of StandardAttribute */
int CcLines::Sizeof() {
  return 0;
}

/* Inherited method of StandardAttribute */
void CcLines::SetDefined(bool Defined) {
}

/* Inherited method of StandardAttribute */
void CcLines::CopyFrom(StandardAttribute* right) {
}

/* Inherited method of StandardAttribute */
size_t CcLines::HashValue() {
  return 0;
}

/* This contructor takes just one serial string
   of the underlying java object and its length. */
CcLines::CcLines(char *serial, int len) {
  jbyteArray jbarr = 0;
  jmethodID mid_Rose;
  jclass cls_Rose;

  /* Get the Class */
  cls = env->FindClass("Lines");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Allocate a new byte array inside the VM. */
  jbarr = env->NewByteArray(len);
  if (jbarr == 0) error(__FILE__, __LINE__);

  /* Store the data into this array. */
  env->SetByteArrayRegion(jbarr, 0, len, (jbyte *)serial);

  /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  /* Get the method ID of imprt_arr */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "imprt_arr",
				    "([B)Ljava/lang/Object;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Call the static imprt_arr method */
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jbarr);
  if (obj == 0) error(__FILE__, __LINE__);

}

/* This constructor takes the nested list representation
   of CcLines and recovers the underlying java object with
   help of this data. */

CcLines::CcLines(const ListExpr &le) {
  char *nlchar;
  string nlstr;
  bool succ;
  int len;
  jstring jstr;
  jobject jnl;
  jclass cls_Rose;
  jmethodID mid_Rose;
  jmethodID mid_Rose2;

  /* Get the Class */
  cls = env->FindClass("Lines");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Get the c_string of le */
  succ = nl->WriteToString(nlstr, le);
  if (succ == false) error(__FILE__, __LINE__);

  nlchar = (char *)nlstr.c_str();
  len = strlen(nlchar);

  jstr = env->NewStringUTF(nlchar);
  if (jstr == 0) error(__FILE__, __LINE__);

  /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  /* Get the method ID of StringToListExpr */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "StringToListExpr",
				    "(Ljava/lang/String;)Lsj/lang/ListExpr;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Call the static StringToListExpr method */
  jnl = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jstr);
  if (jnl == 0) error(__FILE__, __LINE__);

  /* Get the method ID of imprt_Lines */
  mid_Rose2 = env->GetStaticMethodID(cls_Rose, "imprt_Lines",
				     "(Lsj/lang/ListExpr;)LLines;");
  if (mid_Rose2 == 0) error(__FILE__, __LINE__);

  /* recreate the Lines object from the (Java-) ListExpr */
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose2, jnl);
  if (obj == 0) error(__FILE__, __LINE__);
}

/* This constructor takes a pointer to a java object which is
   already created. */

CcLines::CcLines(const jobject jobj) {
  /* Get the Class RoseImExport */
  cls = env->FindClass("Lines");
  if (cls == 0) error(__FILE__, __LINE__);
  obj = jobj;
}

/* This constructor creates an empty CcPoints object. */
CcLines::CcLines() {
  jmethodID mid;

  /* Get the Class */
  cls = env->FindClass("Lines");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(cls, "<init>", "()V");
  if (mid == 0) error(__FILE__,__LINE__);

  /* Create a Java-object Point. */
  obj = env->NewObject(cls, mid);
  if (obj == 0) error(__FILE__,__LINE__);
}

/* retrieves the nested list representation of the underlying
     java object. */

bool CcLines::GetNL(ListExpr& le) {
  jclass cls_Rose;
  jmethodID mid_Rose;
  jmethodID mid_Rose2;
  jobject jnl;
  jstring jnljstr;
  char *jnlstr;
  string cppstr;

 /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  mid_Rose = env->GetStaticMethodID(cls_Rose, "StringToListExpr",
				    "(Ljava/lang/String;)Lsj/lang/ListExpr;");
  /* Get the method ID of exprt_nl */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "exprt_nl",
				    "(Ljava/lang/Object;)Lsj/lang/ListExpr;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Get the java nested list of this object */
  jnl = env->CallStaticObjectMethod(cls_Rose, mid_Rose, obj);
  if (jnl == 0) error(__FILE__, __LINE__);

  /* Get the method ID of ListExprToString */
  mid_Rose2 = env->GetStaticMethodID(cls_Rose, "ListExprToString",
				     "(Lsj/lang/ListExpr;)Ljava/lang/String;");
  if (mid_Rose2 == 0) error(__FILE__, __LINE__);

  /* Retrieve the java string representation of the nested list. */
  jnljstr = (jstring)env->CallStaticObjectMethod(cls_Rose, mid_Rose2, jnl);
  if (jnlstr == 0) error(__FILE__, __LINE__);

  /* Get the c string of above java string */
  jnlstr = (char *)env->GetStringUTFChars(jnljstr, 0);
  if (jnlstr == 0) error(__FILE__, __LINE__);

  cppstr = jnlstr;

  return nl->ReadFromString(cppstr, le);
}

/* Destructor of CcLines. This destructor destroys also the
     object inside the JVM. */
CcLines::~CcLines() {
  //env->DeleteLocalRef(obj);
}

/* Returns the pointer to the proper java objet. */

jobject CcLines::GetObj() {
  return obj;
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

static Word CreateCcLines(const ListExpr typeInfo) {
  return (SetWord(new CcLines()));
}

static void DeleteCcLines(Word &w) {
  delete ((CcLines *)w.addr);
  w.addr = 0;
}

static void CloseCcLines(Word & w) {
  delete (CcLines *)w.addr;
  w.addr = 0;
}

static Word CloneCcLines(const Word &w) {
  return SetWord(((CcLines *)w.addr)->Clone());
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
  return (nl->IsEqual(type, "cclines"));
}

/*
3.6 Creation of the Type Constructor Instance

*/

TypeConstructor cclines (
 "cclines",
 // name
 LinesProperty,
 // property function describing the signature
 OutCcLines,
 // out function
 InCcLines,
 // in function
 CreateCcLines,
 // object creation
 DeleteCcLines,
 // object deletion
 0,
 // object open
 0,
 // object save
 CloseCcLines,
 // object close
 CloneCcLines,
 // object clone
 DummyCast,
 // cast function
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

*/

class CcRegions {
private:
  jclass cls;
  jobject obj;

public:
  /* Inherited methods of Attribute */
  int Compare(Attribute *attr);
  Attribute *Clone();
  bool IsDefined();
  int Sizeof();
  void SetDefined(bool Defined);
  void CopyFrom(StandardAttribute* right);
  size_t HashValue();

  /* This contructor takes just one serial string
     of the underlying java object and its length. */
  CcRegions(char *serial, int len);
  /* This constructor takes the nested list representation
     of CcRegions and recovers the underlying java object with
     help of this data. */
  CcRegions(const ListExpr &le);
 /* This constructor takes a pointer to a java object which is
     already created. */
  CcRegions(const jobject jobj);
  /* This constructor creates an empty CcRegions object. */
  CcRegions();
 /* retrieves the nested list representation of the underlying
     java object. */
  bool GetNL(ListExpr &le);
  /* Destructor of CcRegions. This destructor destroys also the
     object inside the JVM. */
  ~CcRegions();
  /* Returns the pointer to the proper java objet. */
  jobject GetObj();
};

/* Inherited method of StandardAttribute */
int CcRegions::Compare(Attribute *attr) {
  return 0;
}

/* Inherited method of StandardAttribute */
Attribute *CcRegions::Clone() {
  return 0;
}

/* Inherited method of StandardAttribute */
bool CcRegions::IsDefined() {
  return true;
}

/* Inherited method of StandardAttribute */
int CcRegions::Sizeof() {
  return 0;
}

/* Inherited method of StandardAttribute */
void CcRegions::SetDefined(bool Defined) {
}

/* Inherited method of StandardAttribute */
void CcRegions::CopyFrom(StandardAttribute* right) {
}

/* Inherited method of StandardAttribute */
size_t CcRegions::HashValue() {
  return 0;
}

/* This contructor takes just one serial string
   of the underlying java object and its length. */
CcRegions::CcRegions(char *serial, int len) {
  jbyteArray jbarr = 0;
  jmethodID mid_Rose;
  jclass cls_Rose;

  /* Get the Class */
  cls = env->FindClass("Regions");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Allocate a new byte array inside the VM. */
  jbarr = env->NewByteArray(len);
  if (jbarr == 0) error(__FILE__, __LINE__);

  /* Store the data into this array. */
  env->SetByteArrayRegion(jbarr, 0, len, (jbyte *)serial);

  /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  /* Get the method ID of imprt_arr */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "imprt_arr",
				    "([B)Ljava/lang/Object;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Call the static imprt_arr method */
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jbarr);
  if (obj == 0) error(__FILE__, __LINE__);
}

/* This constructor takes the nested list representation
   of CcRegions and recovers the underlying java object with
   help of this data. */

CcRegions::CcRegions(const ListExpr &le) {
  char *nlchar;
  string nlstr;
  bool succ;
  int len;
  jstring jstr;
  jobject jnl;
  jclass cls_Rose;
  jmethodID mid_Rose;
  jmethodID mid_Rose2;

  /* Get the Class */
  cls = env->FindClass("Regions");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Get the c_string of le */
  succ = nl->WriteToString(nlstr, le);
  if (succ == false) error(__FILE__, __LINE__);

  nlchar = (char *)nlstr.c_str();
  len = strlen(nlchar);

  jstr = env->NewStringUTF(nlchar);
  if (jstr == 0) error(__FILE__, __LINE__);

  /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  /* Get the method ID of StringToListExpr */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "StringToListExpr",
				    "(Ljava/lang/String;)Lsj/lang/ListExpr;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Call the static StringToListExpr method */
  jnl = env->CallStaticObjectMethod(cls_Rose, mid_Rose, jstr);
  if (jnl == 0) error(__FILE__, __LINE__);

  /* Get the method ID of imprt_Regions */
  mid_Rose2 = env->GetStaticMethodID(cls_Rose, "imprt_Regions",
				     "(Lsj/lang/ListExpr;)LRegions;");
  if (mid_Rose2 == 0) error(__FILE__, __LINE__);

  /* recreate the Regions object from the (Java-) ListExpr */
  obj = env->CallStaticObjectMethod(cls_Rose, mid_Rose2, jnl);
  if (obj == 0) error(__FILE__, __LINE__);
}

/* This constructor takes a pointer to a java object which is
   already created. */

CcRegions::CcRegions(const jobject jobj) {
  /* Get the Class RoseImExport */
  cls = env->FindClass("Regions");
  if (cls == 0) error(__FILE__, __LINE__);
  obj = jobj;
}

/* This constructor creates an empty CcRegions object. */
CcRegions::CcRegions() {
  jmethodID mid;

  /* Get the Class */
  cls = env->FindClass("Regions");
  if (cls == 0) error(__FILE__, __LINE__);

  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(cls, "<init>", "()V");
  if (mid == 0) error(__FILE__,__LINE__);

  /* Create a Java-object Point. */
  obj = env->NewObject(cls, mid);
  if (obj == 0) error(__FILE__,__LINE__);
}

/* retrieves the nested list representation of the underlying
     java object. */

bool CcRegions::GetNL(ListExpr& le) {
  jclass cls_Rose;
  jmethodID mid_Rose;
  jmethodID mid_Rose2;
  jobject jnl;
  jstring jnljstr;
  char *jnlstr;
  string cppstr;

 /* Get the Class RoseImExport */
  cls_Rose = env->FindClass("RoseImExport");
  if (cls_Rose == 0) error(__FILE__, __LINE__);

  mid_Rose = env->GetStaticMethodID(cls_Rose, "StringToListExpr",
				    "(Ljava/lang/String;)Lsj/lang/ListExpr;");
  /* Get the method ID of exprt_nl */
  mid_Rose = env->GetStaticMethodID(cls_Rose, "exprt_nl",
				    "(Ljava/lang/Object;)Lsj/lang/ListExpr;");
  if (mid_Rose == 0) error(__FILE__, __LINE__);

  /* Get the java nested list of this object */
  jnl = env->CallStaticObjectMethod(cls_Rose, mid_Rose, obj);
  if (jnl == 0) error(__FILE__, __LINE__);

  /* Get the method ID of ListExprToString */
  mid_Rose2 = env->GetStaticMethodID(cls_Rose, "ListExprToString",
				     "(Lsj/lang/ListExpr;)Ljava/lang/String;");
  if (mid_Rose2 == 0) error(__FILE__, __LINE__);

  /* Retrieve the java string representation of the nested list. */
  jnljstr = (jstring)env->CallStaticObjectMethod(cls_Rose, mid_Rose2, jnl);
  if (jnlstr == 0) error(__FILE__, __LINE__);

  /* Get the c string of above java string */
  jnlstr = (char *)env->GetStringUTFChars(jnljstr, 0);
  if (jnlstr == 0) error(__FILE__, __LINE__);

  cppstr = jnlstr;

  return nl->ReadFromString(cppstr, le);
}

/* Destructor of CcRegions. This destructor destroys also the
     object inside the JVM. */
CcRegions::~CcRegions() {
  //env->DeleteLocalRef(obj);
}

/* Returns the pointer to the proper java objet. */

jobject CcRegions::GetObj() {
  return obj;
}

/*
4.2 List Representation

The list representation of a CcRegions object is

----	(face1 face2 ... facen)
----

whereas the (internal) list representation of a face is

----	(outer_cycle hole_cycle1 hole_cycle2 ... hole_cyclem)
----

whereas the (internal) list representation of a cycle
(outer_cycle or hole_cycle) is

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

static Word CreateCcRegions(const ListExpr typeInfo) {
  return (SetWord(new CcRegions()));
}

static void DeleteCcRegions(Word &w) {
  delete ((CcRegions *)w.addr);
  w.addr = 0;
}

static void CloseCcRegions(Word & w) {
  delete (CcRegions *)w.addr;
  w.addr = 0;
}

static Word CloneCcRegions(const Word &w) {
  return SetWord(((CcRegions *)w.addr)->Clone());
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
       nl->StringAtom("(<outer_cycle> <hole_cycle1> <hole cycle2> ... <hole cycleN>)"),
       nl->StringAtom("..."),
       nl->StringAtom("missing."))));
}


/*

4.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~cclines~ does not have arguments, this is trivial.

*/

static bool CheckCcRegions(ListExpr type, ListExpr& errorInfo ) {
  return (nl->IsEqual(type, "ccregions"));
}

/*
4.6 Creation of the Type Constructor Instance

*/

TypeConstructor ccregions (
 "ccregions",
 // name
 RegionsProperty,
 // property function describing the signature
 OutCcRegions,
 // out function
 InCcRegions,
 // in function
 CreateCcRegions,
 // object creation
 DeleteCcRegions,
 // object deletion
 0,
 // object open
 0,
 // object save
 CloseCcRegions,
 // object close
 CloneCcRegions,
 // object clone
 DummyCast,
 // cast function
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

/* This is a general type mapping function for all Rose methods
   which take two parameters. */

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

/* This is a general type mapping function for all Rose methods
   which take one parameter. */

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


static ListExpr ccpointsccpointsBool(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "ccpoints", "bool");
}


static ListExpr cclinescclinesBool(ListExpr args) {
  return typeMappingRose(args, "cclines", "cclines", "bool");
}


static ListExpr ccregionsccregionsBool(ListExpr args) {
  return typeMappingRose(args, "ccregions", "ccregions", "bool");
}


static ListExpr ccpointsccregionsBool(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "ccregions", "bool");
}


static ListExpr cclinesccregionsBool(ListExpr args) {
  return typeMappingRose(args, "cclines", "ccregions", "bool");
}


static ListExpr ccregionscclinesBool(ListExpr args) {
  return typeMappingRose(args, "ccregions", "cclines", "bool");
}


static ListExpr ccregionsccregionsccregions(ListExpr args) {
  return typeMappingRose(args, "ccregions", "ccregions", "ccregions");
}


static ListExpr ccregionscclinescclines(ListExpr args) {
  return typeMappingRose(args, "ccregions", "cclines", "cclines");
}


static ListExpr ccregionsccregionscclines(ListExpr args) {
  return typeMappingRose(args, "ccregions", "ccregions", "cclines");
}


static ListExpr cclinescclinescclines(ListExpr args) {
  return typeMappingRose(args, "cclines", "cclines", "cclines");
}


static ListExpr cclinesccregionscclines(ListExpr args) {
  return typeMappingRose(args, "cclines", "ccregions", "cclines");
}


static ListExpr cclinescclinesccpoints(ListExpr args) {
  return typeMappingRose(args, "cclines", "cclines", "ccpoints");
}

static ListExpr ccpointscclinesBool(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "cclines", "bool");
}


static ListExpr ccpointsccpointsccpoints(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "ccpoints", "ccpoints");
}


static ListExpr cclinesccpoints(ListExpr args) {
  return typeMappingRose(args, "cclines", "ccpoints");
}


static ListExpr cclinesccregions(ListExpr args) {
  return typeMappingRose(args, "cclines", "ccregions");
}


static ListExpr ccregionsccpoints(ListExpr args) {
  return typeMappingRose(args, "ccregions", "ccpoints");
}


static ListExpr ccregionscclines(ListExpr args) {
  return typeMappingRose(args, "ccregions", "cclines");
}


static ListExpr ccpointsInt(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "int");
}


static ListExpr cclinesInt(ListExpr args) {
  return typeMappingRose(args, "cclines", "int");
}


static ListExpr ccregionsInt(ListExpr args) {
  return typeMappingRose(args, "ccregions", "int");
}


static ListExpr ccpointsccpointsReal(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "ccpoints", "real");
}


static ListExpr ccpointscclinesReal(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "cclines", "real");
}


static ListExpr ccpointsccregionsReal(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "ccregions", "real");
}


static ListExpr cclinesccpointsReal(ListExpr args) {
  return typeMappingRose(args, "cclines", "ccpoints", "real");
}


static ListExpr cclinescclinesReal(ListExpr args) {
  return typeMappingRose(args, "cclines", "cclines", "real");
}


static ListExpr cclinesccregionsReal(ListExpr args) {
  return typeMappingRose(args, "cclines", "ccregions", "real");
}


static ListExpr ccregionsccpointsReal(ListExpr args) {
  return typeMappingRose(args, "ccregions", "ccpoints", "real");
}


static ListExpr ccregionscclinesReal(ListExpr args) {
  return typeMappingRose(args, "ccregions", "cclines", "real");
}


static ListExpr ccregionsccregionsReal(ListExpr args) {
  return typeMappingRose(args, "ccregions", "ccregions", "real");
}


static ListExpr ccpointsReal(ListExpr args) {
  return typeMappingRose(args, "ccpoints", "real");
}


static ListExpr cclinesReal(ListExpr args) {
  return typeMappingRose(args, "cclines", "real");
}


static ListExpr ccregionsReal(ListExpr args) {
  return typeMappingRose(args, "ccregions", "real");
}


static ListExpr ccregionsDouble(ListExpr args) {
  return typeMappingRose(args, "ccregions", "real");
}


static ListExpr cclinesDouble(ListExpr args) {
  return typeMappingRose(args, "cclines", "real");
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

/* this is a template function which calls a method of the
Java class ROSEAlgebra with two parameters of type CcPoints,
CcLines or CcRegions and returns a bool value. The signature
must correspond to both types and return type.
*/

template <class Type1, class Type2>
static bool callBooleanJMethod(char *name, Type1 *t1, Type2 *t2, char *signature) {
  jclass cls_ROSE;
  jmethodID mid_ROSE;

  /* Get the Class ROSEAlgebra */
  cls_ROSE = env->FindClass("ROSEAlgebra");
  if (cls_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function */
  mid_ROSE = env->GetStaticMethodID(cls_ROSE, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  return env->CallStaticBooleanMethod(cls_ROSE, mid_ROSE,
				      t1->GetObj(), t2->GetObj());
}

/* this is a template function which calls a method of the
Java class ROSEAlgebra with one parameter of type CcPoints,
CcLines or CcRegions and returns an integer value. The signature
must correspond to the type and return type.
*/

template <class Type1>
static int callIntegerJMethod(char *name, Type1 *t1, char *signature) {
  jclass cls_ROSE;
  jmethodID mid_ROSE;
  int result;

  /* Get the Class ROSEAlgebra */
  cls_ROSE = env->FindClass("ROSEAlgebra");
  if (cls_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function */
  mid_ROSE = env->GetStaticMethodID(cls_ROSE, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  result = env->CallStaticIntMethod(cls_ROSE, mid_ROSE, t1->GetObj());

  return result;
}

/* this is a template function which calls a method of the
Java class ROSEAlgebra with two parameters of type CcPoints,
CcLines or CcRegions and returns a Rational value. This value
is transformed into a double value. The signature
must correspond to the type and return type.
*/

template <class Type1, class Type2>
static double callRationalJMethod(char *name, Type1 *t1, Type2 *t2,
				  char *signature) {
  jclass cls_ROSE;
  jmethodID mid_ROSE;
  jobject result;
  jclass cls_Rational;
  jmethodID mid_Rational1;
  jmethodID mid_Rational2;
  int numerator;
  int denominator;

  /* Get the Class ROSEAlgebra */
  cls_ROSE = env->FindClass("ROSEAlgebra");
  if (cls_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function */
  mid_ROSE = env->GetStaticMethodID(cls_ROSE, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  result =  env->CallStaticObjectMethod(cls_ROSE, mid_ROSE,
					 t1->GetObj(), t2->GetObj());
  /* Get the class Rational */
  cls_Rational = env->FindClass("Rational");
  if (cls_Rational == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function. */
  mid_Rational1 = env->GetMethodID(cls_Rational, "getNumerator", "()I");
  if (mid_Rational1 == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function. */
  mid_Rational2 = env->GetMethodID(cls_Rational, "getDenominator", "()I");
  if (mid_Rational2 == 0) error(__FILE__, name, __LINE__);

  /* Calculate the numerator and denominator of the result. */
  numerator = env->CallIntMethod(result, mid_Rational1);
  denominator = env->CallIntMethod(result, mid_Rational2);

  return (double)numerator / (double)denominator;
}

/* this is a template function which calls a method of the
Java class ROSEAlgebra with one parameter of type CcPoints,
CcLines or CcRegions and returns a Rational value. This value
is transformed into a double value. The signature
must correspond to the type and return type.
*/

template <class Type1>
static double callRationalJMethod(char *name, Type1 *t1, char *signature) {
  jclass cls_ROSE;
  jmethodID mid_ROSE;
  jobject result;
  jclass cls_Rational;
  jmethodID mid_Rational1;
  jmethodID mid_Rational2;
  int numerator;
  int denominator;

  /* Get the Class ROSEAlgebra */
  cls_ROSE = env->FindClass("ROSEAlgebra");
  if (cls_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function */
  mid_ROSE = env->GetStaticMethodID(cls_ROSE, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  result =  env->CallStaticObjectMethod(cls_ROSE, mid_ROSE, t1->GetObj());

  /* Get the Class Rational */
  cls_Rational = env->FindClass("Rational");
  if (cls_Rational == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function. */
  mid_Rational1 = env->GetMethodID(cls_Rational, "getNumerator", "()I");
  if (mid_Rational1 == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function. */
  mid_Rational2 = env->GetMethodID(cls_Rational, "getDenominator", "()I");
  if (mid_Rational2 == 0) error(__FILE__, name, __LINE__);

  /* Calculate the numerator and denominator of the result */
  numerator = env->CallIntMethod(result, mid_Rational1);
  denominator = env->CallIntMethod(result, mid_Rational2);

  return (double)numerator / (double)denominator;
}

/* this is a template function which calls a method of the
Java class ROSEAlgebra with a parameter of type CcPoints,
CcLines or CcRegions and returns a double value. The signature
must correspond to the type and return type.
*/

template <class Type1>
static double callDoubleJMethod(char *name, Type1 *t1, char *signature) {
  jclass cls_ROSE;
  jmethodID mid_ROSE;

  /* Get the Class ROSEAlgebra */
  cls_ROSE = env->FindClass("ROSEAlgebra");
  if (cls_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Get the method ID of the java function */
  mid_ROSE = env->GetStaticMethodID(cls_ROSE, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  return (double)env->CallStaticDoubleMethod(cls_ROSE, mid_ROSE, t1->GetObj());
}


/* this is a template function which calls a method of the
Java class ROSEAlgebra with two parameters of type CcPoints,
CcLines or CcRegions and returns another Rose type, like
CcPoints, CcLines or CcRegions. The signature
must correspond two both types and return type.
*/

template <class Type1, class Type2, class ReturnType>
static ReturnType *callObjectJMethod
(char *name, Type1 *t1, Type2 *t2, char *signature) {
  jclass cls_ROSE;
  jmethodID mid_ROSE;
  jobject result;

  /* Get the Class ROSEAlgebra */
  cls_ROSE = env->FindClass("ROSEAlgebra");
  if (cls_ROSE == 0) error(__FILE__, name,__LINE__);

  /* Get the method ID of the java function */
  mid_ROSE = env->GetStaticMethodID(cls_ROSE, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  result = env->CallStaticObjectMethod(cls_ROSE, mid_ROSE,
					t1->GetObj(), t2->GetObj());

  return new ReturnType(result);
}

/* this is a template function which calls a method of the
Java class ROSEAlgebra with one parameter of type CcPoints,
CcLines or CcRegions and returns another Rose type, like
CcPoints, CcLines or CcRegions. The signature
must correspond two both types and return type.
*/

template <class Type1, class ReturnType>
static ReturnType *callObjectJMethod
(char *name, Type1 *t1, char *signature) {
  jclass cls_ROSE;
  jmethodID mid_ROSE;
  jobject result;

  /* Get the Class ROSEAlgebra */
  cls_ROSE = env->FindClass("ROSEAlgebra");
  if (cls_ROSE == 0) error(__FILE__, name,__LINE__);

  /* Get the method ID of the java function */
  mid_ROSE = env->GetStaticMethodID(cls_ROSE, name, signature);
  if (mid_ROSE == 0) error(__FILE__, name, __LINE__);

  /* Call the static method with given name */
  result = env->CallStaticObjectMethod(cls_ROSE, mid_ROSE, t1->GetObj());

  return new ReturnType(result);
}

/*

5.3.1.2 General Wrapper functions for the Java Methods. These methods
invoke specific Java methods with the given name and signature with
help of above template functions. These functions are provided to make
the algebra more clearly arranged.
The names of these functions all begin with the prefix callJMethod_
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


/* Call a Java method with the given name which takes
   two CcPoints and returns a boolean value.
*/

static bool callJMethod_PPB
(char *name, CcPoints *ccp1, CcPoints *ccp2) {
  return callBooleanJMethod<CcPoints, CcPoints>
    (name, ccp1, ccp2, "(LPoints;LPoints;)Z");
}

/* Call a Java method with the given name which takes
   two CcLines and returns a boolean value.
*/

static bool callJMethod_LLB
(char *name, CcLines *ccl1, CcLines *ccl2) {
  return callBooleanJMethod<CcLines, CcLines>
    (name, ccl1, ccl2, "(LLines;LLines;)Z");
}

/* Call a Java method with the given name which takes
   two CcRegions and returns a boolean value.
*/

static bool callJMethod_RRB
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return callBooleanJMethod<CcRegions, CcRegions>
    (name, ccr1, ccr2, "(LRegions;LRegions;)Z");
}


/* Call a Java method with the given name which takes
   a CcPoints and a CcRegions object and returns a boolean value.
*/

static bool callJMethod_PRB
(char *name, CcPoints *ccp, CcRegions *ccr) {
  return callBooleanJMethod<CcPoints, CcRegions>
    (name, ccp, ccr, "(LPoints;LRegions;)Z");
}

/* Call a Java method with the given name which takes
   a CcLines and a CcRegions object and returns a boolean value.
*/

static bool callJMethod_LRB
(char *name, CcLines *ccl, CcRegions *ccr) {
  return callBooleanJMethod<CcLines, CcRegions>
    (name, ccl, ccr, "(LLines;LRegions;)Z");
}

/* Call a Java method with the given name which takes
   two CcRegions and returns a CcRegions value.
*/

static CcRegions *callJMethod_RRR
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return
   callObjectJMethod<CcRegions, CcRegions, CcRegions>
   (name, ccr1, ccr2, "(LRegions;LRegions;)LRegions;");
}

/* Call a Java method with the given name which takes
   two CcRegions and returns a CcLines value.
*/

static CcLines *callJMethod_RRL
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return
    callObjectJMethod<CcRegions, CcRegions, CcLines>
    (name, ccr1, ccr2, "(LRegions;LRegions;)LLines;");
}

/* Call a Java method with the given name which takes
   a CcRegions and a CcLines object and returns a boolean value.
*/

static bool callJMethod_RLB
(char *name, CcRegions *ccr, CcLines *ccl) {
  return
    callBooleanJMethod<CcRegions, CcLines>
    (name, ccr, ccl, "(LRegions;LLines;)Z");
}

/* Call a Java method with the given name which takes
   a CcPoints and a CcLines object and returns a boolean value.
*/

static bool callJMethod_PLB
(char *name, CcPoints *ccp, CcLines *ccl) {
  return
    callBooleanJMethod<CcPoints, CcLines>
    (name, ccp, ccl, "(LPoints;LLines;)Z");
}

/* Call a Java method with the given name which takes
   two CcLines and returns a CcPoints value.
*/

static CcPoints *callJMethod_LLP
(char *name, CcLines *ccl1, CcLines *ccl2) {
  return
    callObjectJMethod<CcLines, CcLines, CcPoints>
    (name, ccl1, ccl2, "(LLines;LLines;)LPoints;");
}

/* Call a Java method with the given name which takes
   a CcRegions and a CcLines object and returns a CcLines value.
*/

static CcLines *callJMethod_RLL
(char *name, CcRegions *ccr, CcLines *ccl) {
  return
    callObjectJMethod<CcRegions, CcLines, CcLines>
    (name, ccr, ccl, "(LRegions;LLines;)LLines;");
}

/* Call a Java method with the given name which takes
   two CcPoints and returns a CcPoints value.
*/

static CcPoints *callJMethod_PPP
(char *name, CcPoints *ccp1, CcPoints *ccp2) {
  return
    callObjectJMethod<CcPoints, CcPoints, CcPoints>
    (name, ccp1, ccp2, "(LPoints;LPoints;)LPoints;");
}

/* Call a Java method with the given name which takes
   two CcLines and returns a CcLines value.
*/

static CcLines *callJMethod_LLL
(char *name, CcLines *ccl1, CcLines *ccl2) {
  return
    callObjectJMethod<CcLines, CcLines, CcLines>
    (name, ccl1, ccl2, "(LLines;LLines;)LLines;");
}

/* Call a Java method with the given name which takes
   a CcLines and a CcRegions object and returns a CcLines value.
*/

static CcLines *callJMethod_LRL
(char *name, CcLines *ccl, CcRegions *ccr) {
  return
    callObjectJMethod<CcLines, CcRegions, CcLines>
    (name, ccl, ccr, "(LLines;LRegions;)LLines;");
}

/* Call a Java method with the given name which takes
   a CcLines object and returns a CcLines value.
*/

static CcPoints *callJMethod_LP
(char *name, CcLines *ccl) {
  return
    callObjectJMethod<CcLines, CcPoints>
    (name, ccl, "(LLines;)LPoints;");
}

/* Call a Java method with the given name which takes
   a CcLines object and returns a CcRegions value.
*/

static CcRegions *callJMethod_LR
(char *name, CcLines *ccl) {
  return
    callObjectJMethod<CcLines, CcRegions>
    (name, ccl, "(LLines;)LRegions;");
}

/* Call a Java method with the given name which takes
   a CcRegions object and returns a CcPoints value.
*/

static CcPoints *callJMethod_RP
(char *name, CcRegions *ccr) {
  return
    callObjectJMethod<CcRegions, CcPoints>
    (name, ccr, "(LRegions;)LPoints;");
}

/* Call a Java method with the given name which takes
   a CcRegions object and returns a CcLines value.
*/

static CcLines *callJMethod_RL
(char *name, CcRegions *ccr) {
  return
    callObjectJMethod<CcRegions, CcLines>
    (name, ccr, "(LRegions;)LLines;");
}

/* Call a Java method with given name which takes
   a CcPoints object and returns an Integer value.
*/

static int callJMethod_PI
(char *name, CcPoints *ccp) {
  return
    callIntegerJMethod<CcPoints>(name, ccp, "(LPoints;)I");
}

/* Call a Java method with given name which takes
   a CcLines object and returns an Integer value.
*/

static int callJMethod_LI
(char *name, CcLines *ccl) {
  return
    callIntegerJMethod<CcLines>(name, ccl, "(LLines;)I");
}

/* Call a Java method with given name which takes
   a CcRegions object and returns an Integer value.
*/

static int callJMethod_RI
(char *name, CcRegions *ccr) {
  return
    callIntegerJMethod<CcRegions>(name, ccr, "(LRegions;)I");
}

/* Call a Java method with given name which takes
   two CcPoints and returns a double value.
*/

static double callJMethod_PPd
(char *name, CcPoints *ccp1, CcPoints *ccp2) {
  return
    callRationalJMethod<CcPoints, CcPoints>
    (name, ccp1, ccp2, "(LPoints;LPoints;)LRational;");
}

/* Call a Java method with given name which takes
   a CcPoints and CcLines object and returns a double value.
*/

static double callJMethod_PLd
(char *name, CcPoints *ccp, CcLines *ccl) {
  return
    callRationalJMethod<CcPoints, CcLines>
    (name, ccp, ccl, "(LPoints;LLines;)LRational;");
}

/* Call a Java method with given name which takes
   a CcPoints and CcRegions object and returns a double value.
*/

static double callJMethod_PRd
(char *name, CcPoints *ccp, CcRegions *ccr) {
  return
    callRationalJMethod<CcPoints, CcRegions>
    (name, ccp, ccr, "(LPoints;LRegions;)LRational;");
}

/* Call a Java method with given name which takes
   a CcLines and CcPoints object and returns a double value.
*/

static double callJMethod_LPd
(char *name, CcLines *ccl, CcPoints *ccp) {
  return
    callRationalJMethod<CcLines, CcPoints>
    (name, ccl, ccp, "(LLines;LPoints;)LRational;");
}

/* Call a Java method with given name which takes
   two CcLines and returns a double value.
*/

static double callJMethod_LLd
(char *name, CcLines *ccl1, CcLines *ccl2) {
  return
    callRationalJMethod<CcLines, CcLines>
    (name, ccl1, ccl2, "(LLines;LLines;)LRational;");
}

/* Call a Java method with given name which takes
   a CcLines and CcRegions object and returns a double value.
*/

static double callJMethod_LRd
(char *name, CcLines *ccl, CcRegions *ccr) {
  return
    callRationalJMethod<CcLines, CcRegions>
    (name, ccl, ccr, "(LLines;LRegions;)LRational;");
}

/* Call a Java method with given name which takes
   a CcRegions and CcPoints object and returns a double value.
*/

static double callJMethod_RPd
(char *name, CcRegions *ccr, CcPoints *ccp) {
  return
    callRationalJMethod<CcRegions, CcPoints>
    (name, ccr, ccp, "(LRegions;LPoints;)LRational;");
}

/* Call a Java method with given name which takes
   a CcRegions and CcLines object and returns a double value.
*/

static double callJMethod_RLd
(char *name, CcRegions *ccr, CcLines *ccl) {
  return
    callRationalJMethod<CcRegions, CcLines>
    (name, ccr, ccl, "(LRegions;LLines;)LRational;");
}

/* Call a Java method with given name which takes
   two CcRegions and returns a double value.
*/

static double callJMethod_RRd
(char *name, CcRegions *ccr1, CcRegions *ccr2) {
  return
    callRationalJMethod<CcRegions, CcRegions>
    (name, ccr1, ccr2, "(LRegions;LRegions;)LRational;");
}

/* Call a Java method with given name which takes
   one CcPoints and returns a double value.
*/

static double callJMethod_Pd(char *name, CcPoints *ccp) {
  return
    callRationalJMethod<CcPoints>
    (name, ccp, "(LPoints;)LRational;");
}

/* Call a Java method with given name which takes
   one CcLines and returns a double value.
*/

static double callJMethod_Ld(char *name, CcLines *ccl) {
  return
    callRationalJMethod<CcLines>
    (name, ccl, "(LLines;)LRational;");
}

/* Call a Java method with given name which takes
   one CcRegions and returns a double value.
*/

static double callJMethod_Rd(char *name, CcRegions *ccr) {
  return
    callRationalJMethod<CcRegions>
    (name, ccr, "(LRegions;)LRational;");
}

/* Call a Java method with given name which takes
   a CcLines and returns a double value.
*/

static double callJMethod_LD(char *name, CcLines *ccl) {
  return
    callDoubleJMethod<CcLines>(name, ccl, "(LLines;)D");
}

/* Call a Java method with given name which takes
   a CcRegions and returns a double value.
*/

static double callJMethod_RD(char *name, CcRegions *ccr) {
  return
    callDoubleJMethod<CcRegions>(name, ccr, "(LRegions;)D");
}

/*
5.3.2 The proper Value Mapping Functions.

*/

/* Equals predicate for two ccpoints. */

static int pp_equalFun(Word* args, Word& result, int message,
				       Word& local, Supplier s) {
  CcPoints* ccp1;
  CcPoints* ccp2;

  ccp1 = ((CcPoints *)args[0].addr);
  ccp2 = ((CcPoints *)args[1].addr);

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

/* Equals predicate for two cclines. */

static int ll_equalFun(Word* args, Word& result, int message,
				       Word& local, Supplier s) {
  CcLines* ls1;
  CcLines* ls2;

  ls1 = ((CcLines *)args[0].addr);
  ls2 = ((CcLines *)args[1].addr);

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

/* Equal predicate for two ccregions. */

static int rr_equalFun(Word* args, Word& result, int message,
		       Word& local, Supplier s) {
  CcRegions* rs1;
  CcRegions* rs2;

  rs1 = ((CcRegions *)args[0].addr);
  rs2 = ((CcRegions *)args[1].addr);

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

/* pp_unequal predicate for two ccpoints. */

static int pp_unequalFun(Word* args, Word& result, int message,
			 Word& local, Supplier s)
{
  CcPoints* ccps1;
  CcPoints* ccps2;

  ccps1 = ((CcPoints *)args[0].addr);
  ccps2 = ((CcPoints *)args[1].addr);

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

/* ll_unequal predicate for two cclines. */

static int ll_unequalFun(Word* args, Word& result, int message,
			 Word& local, Supplier s)
{
  CcLines* ccl1;
  CcLines* ccl2;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* rr_unequal predicate for two ccregions */

static int rr_unequalFun(Word* args, Word& result, int message,
				       Word& local, Supplier s)
{
  CcRegions* ccr1;
  CcRegions* ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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


/* pp_disjoint predicate for two CcPoints */

static int pp_disjointFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcPoints* ccp1;
  CcPoints* ccp2;

  ccp1 = ((CcPoints *)args[0].addr);
  ccp2 = ((CcPoints *)args[1].addr);

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

/* ll_disjoint predicate for two CcLines */

static int ll_disjointFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcLines *ccl1;
  CcLines *ccl2;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* rr_disjoint predicate for two CcRegions */

static int rr_disjointFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* pr_inside predicate for CcPoints and CcRegions */

static int pr_insideFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcPoints *ccp;
  CcRegions *ccr;

  ccp = ((CcPoints *)args[0].addr);
  ccr = ((CcRegions *)args[1].addr);

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

/* lr_inside predicate for CcLines and CcRegions */

static int lr_insideFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcLines *ccl;
  CcRegions *ccr;

  ccl = ((CcLines *)args[0].addr);
  ccr = ((CcRegions *)args[1].addr);

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

/* rr_inside predicate for two CcRegions */

static int rr_insideFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_area_disjoint predicate for two CcRegions */

static int rr_area_disjointFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_edge_disjoint predicate for two CcRegions */

static int rr_edge_disjointFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_edge_inside predicate for two CcRegions */

static int rr_edge_insideFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_vertex_inside predicate for two CcRegions */

static int rr_vertex_insideFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_intersects predicate for two CcRegions */

static int rr_intersectsFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_meets predicate for two CcRegions */

static int rr_meetsFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_border_in_common predicate for two CcRegions */

static int rr_border_in_commonFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_adjacent predicate for two CcRegions */

static int rr_adjacentFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_encloses predicate for two CcRegions */

static int rr_enclosesFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_intersection predicate for two CcRegions */

static int rr_intersectionFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;
  CcRegions *ccresult;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result

  ccresult = callJMethod_RRR("rr_intersection", ccr1, ccr2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* rr_plus predicate for two CcRegions */

static int rr_plusFun(Word* args, Word& result, int message,
			  Word& local, Supplier s) {
  CcRegions *ccr1;
  CcRegions *ccr2;
  CcRegions *ccresult;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* rr_minus predicate for two CcRegions */

static int rr_minusFun(Word* args, Word& result, int message,
			  Word& local, Supplier s)
{
  CcRegions *ccr1;
  CcRegions *ccr2;
  CcRegions *ccresult;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result

  ccresult = callJMethod_RRR("rr_minus", ccr1, ccr2);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* rr_common_border predicate for two CcRegions */

static int rr_common_borderFun(Word *args, Word& result,
			       int message, Word& local, Supplier s) {
  CcRegions *ccr1;
  CcRegions *ccr2;
  CcLines *ccresult;

  ccr1 = ((CcRegions *)args[0].addr);
  ccr2 = ((CcRegions *)args[1].addr);

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

/* ll_intersects predicate for two CcLines */

static int ll_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {

  CcLines *ccl1;
  CcLines *ccl2;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* lr_intersects predicate for CcLines and CcRegions */

static int lr_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {
  CcLines *ccl;
  CcRegions *ccr;

  ccl = ((CcLines *)args[0].addr);
  ccr = ((CcRegions *)args[1].addr);

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

/* rl_intersects predicate for CcRegions and CcLines */

static int rl_intersectsFun(Word* args, Word& result, int message,
			    Word& local, Supplier s) {
  CcRegions *ccr;
  CcLines *ccl;

  ccr = ((CcRegions *)args[0].addr);
  ccl = ((CcLines *)args[1].addr);

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

/* ll_meets predicate for two cclines.*/

static int ll_meetsFun(Word* args, Word& result, int message,
		       Word& local, Supplier s)
{
  CcLines* ccl1;
  CcLines* ccl2;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* lr_meets predicate for cclines and ccregions. */

static int lr_meetsFun(Word* args, Word& result, int message,
		       Word& local, Supplier s)
{
  CcLines* ccl;
  CcRegions* ccr;

  ccl = ((CcLines *)args[0].addr);
  ccr = ((CcRegions *)args[1].addr);

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

/* rl_meets predicate for ccregions and cclines */

static int rl_meetsFun(Word* args, Word& result, int message,
		       Word& local, Supplier s)
{
  CcLines* ccl;
  CcRegions* ccr;

  ccr = ((CcRegions *)args[0].addr);
  ccl = ((CcLines *)args[1].addr);

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

/* ll_border_in_common predicate for two CcLines. */

static int ll_border_in_commonFun(Word* args, Word& result, int message,
				  Word& local, Supplier s)
{

  CcLines *ccl1;
  CcLines *ccl2;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* lr_border_in_common predicate for CcLines and CcRegions. */

static int lr_border_in_commonFun(Word* args, Word& result, int message,
				  Word& local, Supplier s)
{

  CcLines *ccl;
  CcRegions *ccr;

  ccl = ((CcLines *)args[0].addr);
  ccr = ((CcRegions *)args[1].addr);

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

/* rl_border_in_common predicate for CcRegions and CcLines. */

static int rl_border_in_commonFun(Word* args, Word& result, int message,
				  Word& local, Supplier s)
{
  CcLines *ccl;
  CcRegions *ccr;

  ccr = ((CcRegions *)args[0].addr);
  ccl = ((CcLines *)args[1].addr);

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


/* pl_on_border_of predicate for CcPoints and CcLines */

static int pl_on_border_ofFun(Word* args, Word& result, int message,
			      Word& local, Supplier s)
{
  CcPoints *ccp;
  CcLines *ccl;

  ccp = ((CcPoints *)args[0].addr);
  ccl = ((CcLines *)args[1].addr);

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

/* pr_on_border_of predicate for CcPoints and CcRegions */

static int pr_on_border_ofFun(Word* args, Word& result, int message,
			      Word& local, Supplier s)
{
  CcPoints *ccp;
  CcRegions *ccr;

  ccp = ((CcPoints *)args[0].addr);
  ccr = ((CcRegions *)args[1].addr);

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

/* pp_intersection predicate for two CcPoints. */

static int pp_intersectionFun(Word* args, Word& result, int message,
			      Word& local, Supplier s)
{
  CcPoints *ccp1;
  CcPoints *ccp2;
  CcPoints *ccresult;

  ccp1 = ((CcPoints *)args[0].addr);
  ccp2 = ((CcPoints *)args[1].addr);

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

/* ll_intersection predicate for two CcLines. */

static int ll_intersectionFun(Word* args, Word& result, int message,
			      Word& local, Supplier s)
{
  CcLines *ccl1;
  CcLines *ccl2;
  CcPoints *ccresult;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* rl_intersection predicate for CcRegions and CcLines */

static int rl_intersectionFun(Word* args, Word& result, int message,
			      Word& local, Supplier s)
{
  CcRegions *ccr;
  CcLines *ccl;
  CcLines *ccresult;

  ccr = ((CcRegions *)args[0].addr);
  ccl = ((CcLines *)args[1].addr);

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result

  ccresult = callJMethod_RLL("rl_intersection", ccr, ccl);
  result.addr = ccresult;

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* pp_plus predicate for two CcPoints. */

static int pp_plusFun(Word* args, Word& result, int message,
			      Word& local, Supplier s)
{
  CcPoints *ccp1;
  CcPoints *ccp2;
  CcPoints *ccresult;

  ccp1 = ((CcPoints *)args[0].addr);
  ccp2 = ((CcPoints *)args[1].addr);

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

/* ll_plus predicate for two CcPoints. */

static int ll_plusFun(Word* args, Word& result, int message,
		      Word& local, Supplier s)
{
  CcLines *ccl1;
  CcLines *ccl2;
  CcLines *ccresult;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* pp_minus predicate for two CcPoints. */

static int pp_minusFun(Word* args, Word& result, int message,
		       Word& local, Supplier s)
{
  CcPoints *ccp1;
  CcPoints *ccp2;
  CcPoints *ccresult;

  ccp1 = ((CcPoints *)args[0].addr);
  ccp2 = ((CcPoints *)args[1].addr);

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

/* ll_minus predicate for two CcPoints. */

static int ll_minusFun(Word* args, Word& result, int message,
		       Word& local, Supplier s)
{
  CcLines *ccl1;
  CcLines *ccl2;
  CcLines *ccresult;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* ll_common_border predicate for two CcLines. */

static int ll_common_borderFun(Word *args, Word& result,
			       int message, Word& local, Supplier s) {
  CcLines *ccl1;
  CcLines *ccl2;
  CcLines *ccresult;

  ccl1 = ((CcLines *)args[0].addr);
  ccl2 = ((CcLines *)args[1].addr);

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

/* lr_common_border predicate for CcLines and CcRegions. */

static int lr_common_borderFun(Word *args, Word& result,
			       int message, Word& local, Supplier s) {
  CcLines *ccl;
  CcRegions *ccr;
  CcLines *ccresult;

  ccl = ((CcLines *)args[0].addr);
  ccr = ((CcRegions *)args[1].addr);

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

/* rl_common_border predicate for CcLines and CcRegions. */

static int rl_common_borderFun(Word *args, Word& result,
			       int message, Word& local, Supplier s) {
  CcRegions *ccr;
  CcLines *ccl;
  CcLines *ccresult;

  ccr = ((CcRegions *)args[0].addr);
  ccl = ((CcLines *)args[1].addr);

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

/* l_vertices predicate for CcLines. */

static int l_verticesFun(Word *args, Word& result,
			 int message, Word& local, Supplier s) {
  CcLines *ccl;
  CcPoints *ccresult;

  ccl = ((CcLines *)args[0].addr);

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

/* r_vertices predicate for CcRegions. */

static int r_verticesFun(Word *args, Word& result,
			 int message, Word& local, Supplier s) {
  CcRegions *ccr;
  CcPoints *ccresult;

  ccr = ((CcRegions *)args[0].addr);

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

/* l_interior predicate for CcLines. */

static int l_interiorFun(Word *args, Word& result,
			 int message, Word& local, Supplier s) {
  CcLines *ccl;
  CcRegions *ccresult;

  ccl = ((CcLines *)args[0].addr);

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

/* r_contour predicate for CcRegions. */

static int r_contourFun(Word *args, Word& result,
			int message, Word& local, Supplier s) {
  CcRegions *ccr;
  CcLines *ccresult;

  ccr = ((CcRegions *)args[0].addr);

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

/* p_no_of_components predicate for CcPoints. */

static int p_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  CcPoints *ccp = ((CcPoints *)args[0].addr);

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

/* l_no_of_components predicate for CcLines. */

static int l_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  CcLines *ccl = ((CcLines *)args[0].addr);

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

/* r_no_of_components predicate for CcPoints. */

static int r_no_of_componentsFun(Word *args, Word &result,
				 int message, Word &local,
				 Supplier s) {
  CcRegions *ccr = ((CcRegions *)args[0].addr);

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

/* pp_dist predicate for two CcPoints */

static int pp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcPoints *ccp1 = ((CcPoints *)args[0].addr);
  CcPoints *ccp2 = ((CcPoints *)args[1].addr);

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result

  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_PPd
     ("pp_dist", ccp1, ccp2));

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* pl_dist predicate for CcPoints and CcLines */

static int pl_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcPoints *ccp = ((CcPoints *)args[0].addr);
  CcLines *ccl = ((CcLines *)args[1].addr);

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

/* pr_dist predicate for CcPoints and CcRegions */

static int pr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcPoints *ccp = ((CcPoints *)args[0].addr);
  CcRegions *ccr = ((CcRegions *)args[1].addr);

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

/* lp_dist predicate for CcLines and CcPoints */

static int lp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);
  CcPoints *ccp = ((CcPoints *)args[1].addr);

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

/* ll_dist predicate for two CcLines. */

static int ll_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcLines *ccl1 = ((CcLines *)args[0].addr);
  CcLines *ccl2 = ((CcLines *)args[1].addr);

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

/* lr_dist predicate for CcLines and CcRegions. */

static int lr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);
  CcRegions *ccr = ((CcRegions *)args[1].addr);

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

/* rp_dist predicate for CcRegions and CcPoints. */

static int rp_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);
  CcPoints *ccp = ((CcPoints *)args[1].addr);

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

/* rl_dist predicate for CcRegions and CcLines. */

static int rl_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);
  CcLines *ccl = ((CcLines *)args[1].addr);

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

/* rr_dist predicate for two CcRegions. */

static int rr_distFun(Word *args, Word &result, int message,
		      Word &local, Supplier s) {

  CcRegions *ccr1 = ((CcRegions *)args[0].addr);
  CcRegions *ccr2 = ((CcRegions *)args[1].addr);

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance to take the result

  ((CcReal *)result.addr)->Set
    (true, (float)callJMethod_RRd
     ("rr_dist", ccr1, ccr2));

  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

/* p_diameter predicate for a CcPoints. */

static int p_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {

  CcPoints *ccp = ((CcPoints *)args[0].addr);

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

/* l_diameter predicate for a CcLines. */

static int l_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);

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

/* r_diameter predicate for a CcRegions. */

static int r_diameterFun(Word *args, Word &result, int message,
			 Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);

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

/* l_length predicate for a CcLines. */

static int l_lengthFun(Word *args, Word &result, int message,
		       Word &local, Supplier s) {

  CcLines *ccl = ((CcLines *)args[0].addr);

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

/* r_aera predicate for a CcRegions. */

static int r_areaFun(Word *args, Word &result, int message,
		     Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);

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

/* r_perimeter predicate for a CcRegions. */

static int r_perimeterFun(Word *args, Word &result, int message,
			  Word &local, Supplier s) {

  CcRegions *ccr = ((CcRegions *)args[0].addr);

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

  nl = nlRef;
  qp = qpRef;
  return (&roseAlgebra);
}
