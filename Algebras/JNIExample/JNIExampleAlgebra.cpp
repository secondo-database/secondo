/*
//paragraph [1] title: [{\Large \bf ]	[}]

[1] JNIExample Algebra

July, 3rd, 2003 Mirco Guenster: port to new JNI invoke mechanism.
May, 21st, 2003 Mirco Guenster: adoption to new algebra model.
Janary, 28th, 2003 Mirco Guenster and Ismail Zerrad.

This little example algebra demonstrates how to implement an algebra in
the Java programming language and link it into Secondo by the 
Java Native Interface (JNI).

It just provides a data type ~ccpoint~ and two operators:
(i) ~equals~ which checks the equality of two points and 
(ii) ~dist~ which calculates the euklidian distance between 
two of those points. 

1 Preliminaries

1.1 Includes


*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"	//needed because we return a CcBool in an op.

static NestedList* nl;
static QueryProcessor* qp;


#include "Attribute.h"
#include <jni.h>
#include <JVMInit.h>

static JVMInitializer *jvminit = 0;

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
  cerr << "Error in JNIExampleAlgebra.cpp in line: " << line << "." << endl;
  exit(1);
}

/*
1.2 Dummy Functions

Not interesting, but needed in the definition of a type constructor.

*/

static void* DummyCast( void* addr ) {
  return (0);
}

/* 
2 Type Constructor ~ccpoint~

2.1 Data Structure - Class ~CcPoint~

*/

/* This class is just a wrapper to the according java class Point. */
class CcPoint {
public:
  CcPoint();
  CcPoint(jint nom_x, jint denom_x, jint nom_y, jint denom_y);
  ~CcPoint();
  void print();
  jint GetXNominator();
  jint GetXDenominator();
  jint GetYNominator();
  jint GetYDenominator();
  void SetXNominator(jint nom_x);
  void SetXDenominator(jint denom_x);
  void SetYNominator(jint nom_y);
  void SetYDenominator(jint denom_y);
  bool equals(CcPoint &p);
  double distance(CcPoint &p);
  CcPoint *Clone();
  
private:
  jclass cls;  // pointer to the corresponding java class Point.
  jobject obj; // pointer to the corresponding instance 
};

/* Standard constructor. */
CcPoint::CcPoint() {
  jmethodID mid;
  
  /* Find the class Point. */
  cls = env->FindClass("PointJNITest");  
  if (cls == 0) {
    error(__LINE__);
  }
  
  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(cls, "<init>", "()V");
  if (mid == 0) {
    error(__LINE__);
  }
  
  /* Create a Java-object Point. */
  obj = env->NewObject(cls, mid);
  if (obj == 0) {
    error(__LINE__);
  }  
}

/* Constructor which takes the point attributes. */
CcPoint::CcPoint(jint nom_x, jint denom_x, jint nom_y, jint denom_y) {
  jclass clsRational;
  jobject x;
  jobject y;
  jmethodID mid;
  
  /* Find the class Point. */
  cls = env->FindClass("PointJNITest");  
  if (cls == 0) {
    error(__LINE__);
  }
  
  /* Find the class Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the method ID of the constructor of Rational 
     which takes two integers. */
  mid = env->GetMethodID(clsRational, "<init>", "(II)V");
  if (mid == 0) {
    error(__LINE__);
  }
  
  /* Create the first Java-object of type Rational for the x value. */
  x = env->NewObject(clsRational, mid, nom_x, denom_x);
  if (x == 0) {
    error(__LINE__);
  }
  
  /* Create the second Java-object of type Rational for the y value. */
  y = env->NewObject(clsRational, mid, nom_y, denom_y);
  if (y == 0) {
    error(__LINE__);
  }
  
  /* Get the method ID of the constructor of Point
     which takes two Rational objects. */
  mid = env->GetMethodID(cls, "<init>", 
			 "(LRationalJNITest;LRationalJNITest;)V");
  if (mid == 0) {
    error(__LINE__);
  }
  
  /* Create a new Java-object of type Point. */
  obj = env->NewObject(cls, mid, x, y);
  if (obj == 0) {
    error(__LINE__);
  }
}

/* Destructor of a CcPoint. Deletes the corresponding java object. */ 
CcPoint::~CcPoint(){
  env->DeleteLocalRef(obj);		
}

/* Get the nominator of the x component of a point. */
jint CcPoint::GetXNominator() {
  jfieldID fid;
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of x component. */
  fid = env->GetFieldID(cls, "x", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value x itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the field ID of the nominator in Rational */
  fidRational = env->GetFieldID(clsRational, "n", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }
  
  /* Return the nominator of x. */
  return env->GetIntField(rational, fidRational);
}

/* Get the denominator of the x component in a point. */ 
jint CcPoint::GetXDenominator() {	
  jfieldID fid;
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of x component. */
  fid = env->GetFieldID(cls, "x", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value x itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the field ID of the denominator in Rational */
  fidRational = env->GetFieldID(clsRational, "d", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }
  
  /* Return the denominator of x. */ 
  return env->GetIntField(rational, fidRational);
}

/* Get the nominator of the y component in a point. */
jint CcPoint::GetYNominator() {
  jfieldID fid;
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of y component. */
  fid = env->GetFieldID(cls, "y", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value y itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the field ID of the nominator in Rational. */
  fidRational = env->GetFieldID(clsRational, "n", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }
  
  /* Return the nominator of y. */
  return env->GetIntField(rational, fidRational);
}

/* Get the denominator of the y component in a point. */
jint CcPoint::GetYDenominator() {
  jfieldID fid;
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of y component in a point. */
  fid = env->GetFieldID(cls, "y", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value y itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }

  /* Get the field ID of the denominator in Rational. */
  fidRational = env->GetFieldID(clsRational, "d", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }

  /* Return the denominator of y. */
  return env->GetIntField(rational, fidRational);
}

/* Set the nominator of the x component in a point to nom_x. */
void CcPoint::SetXNominator(jint nom_x) {
  jfieldID fid;
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of x component. */
  fid = env->GetFieldID(cls, "x", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value x itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the field ID of the nominator in Rational. */
  fidRational = env->GetFieldID(clsRational, "n", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }
  
  /* Set the nominator of the x component to nom_x. */
  env->SetIntField(rational, fidRational, nom_x);
}

/* Set the denominator of the x component in a point to denom_x */
void CcPoint::SetXDenominator(jint denom_x) {
  jfieldID fid;
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of x component. */
  fid = env->GetFieldID(cls, "x", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value x itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the field ID of the denominator in Rational. */
  fidRational = env->GetFieldID(clsRational, "d", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }
  
  /* Set the denominator of the x component to denom_x */
  env->SetIntField(rational, fidRational, denom_x);
}

/* Set the nominator of the y component in a point to nom_y. */
void CcPoint::SetYNominator(jint nom_y) {
  jfieldID fid;
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of y component. */
  fid = env->GetFieldID(cls, "y", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value y itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the field ID of the nominator in Rational. */
  fidRational = env->GetFieldID(clsRational, "n", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }
  
  /* Set the nominator of the y component to nom_y */
  env->SetIntField(rational, fidRational, nom_y);
}

/* Set the denominator of the y component in a point to denom_y. */
void CcPoint::SetYDenominator(jint denom_y) {	
  jfieldID fid;
  
  jobject rational;
  jfieldID fidRational;
  jclass clsRational;
  
  /* Get the field ID of y component. */
  fid = env->GetFieldID(cls, "y", "LRationalJNITest;");
  if (fid == 0) {
    error(__LINE__);
  }
  
  /* Get the field value y itself. */
  rational = env->GetObjectField(obj, fid);
  if (rational == 0) {
    error(__LINE__);
  }
  
  /* Get the class handle of Rational. */
  clsRational = env->FindClass("RationalJNITest");
  if (clsRational == 0) {
    error(__LINE__);
  }
  
  /* Get the field ID of the denominator in Rational. */
  fidRational = env->GetFieldID(clsRational, "d", "I");
  if (fidRational == 0) {
    error(__LINE__);
  }
  
  /* Set the denominator of the y component in a point to denom_y. */
  env->SetIntField(rational, fidRational, denom_y);
}

/* Call the print (Java-) method of a Point. For debugging purposes. */
void CcPoint::print() {
  jmethodID mid;
  
  /* Get the method ID of the print method in a Point class. */
  mid = env->GetMethodID(cls, "print", "()V");
  if (mid == 0) {
    error(__LINE__);
  }
  
  /* Call the print method. */
  env->CallVoidMethod(obj, mid);
}

/* Check the equality of p and this point. */
bool CcPoint::equals(CcPoint &p) {
  jmethodID mid;
  bool rc;
  
  /* Get the method ID of the equal method in Point. */
  mid = env->GetMethodID(cls, "equal", "(LPointJNITest;)Z");
  if (mid == 0) {
    error(__LINE__);
  }
  
  /* Call the equal method in Point. */
  rc = env->CallBooleanMethod(obj, mid, p.obj);
  
  return rc;
}

/* Calculate the distance of p and this point. */
double CcPoint::distance(CcPoint &p) {
  jmethodID mid;
  double rc;
  jobject rat_obj;
  jclass rat_cls;
  jmethodID rat_getDID;
  
  /* Get the method ID of the dist method in Point. */
  mid = env->GetMethodID(cls, "dist", "(LPointJNITest;)LRationalJNITest;");
  if (mid == 0) {
    error(__LINE__);
  }

  /* Call the dist method of Point. This method returns 
     an instance of Rational which can be interpreted 
     as a difference vector in the mathematical manner. */
  rat_obj = env->CallObjectMethod(obj, mid, p.obj);
  if (rat_obj == 0) {
    error(__LINE__);
  }

  /* Get the class handle of Rational */
  rat_cls = env->GetObjectClass(rat_obj);
  if (rat_cls == 0) {
    error(__LINE__);
  }
  
  /* Get the method ID of the getDouble method in Point. 
     This method returns the length of our "vector" */
  rat_getDID = env->GetMethodID(rat_cls, "getDouble", "()D");
  if (rat_getDID == 0) {
    error(__LINE__);
  }

  /* Call above method. */
  rc = env->CallDoubleMethod(rat_obj, rat_getDID);
  
  return rc;
}

CcPoint *CcPoint::Clone() {
  return new CcPoint(*this);
}

/*
2.2 List Representation

The list representation of a point is

----	((x_nominator x_denominator) (y_nominator y_denominator))
----

2.3 ~In~ and ~Out~ Functions

*/

static ListExpr OutPoint( ListExpr typeInfo, Word value ) {
  CcPoint* ccpoint;
  ccpoint = (CcPoint*)(value.addr);
  
  return nl->TwoElemList
    (
     nl->TwoElemList
     (
      nl->IntAtom(ccpoint->GetXNominator()),
      nl->IntAtom(ccpoint->GetXDenominator())),
     nl->TwoElemList
     (
      nl->IntAtom(ccpoint->GetYNominator()),
      nl->IntAtom(ccpoint->GetYDenominator())));
}

static Word InPoint (const ListExpr typeInfo, 
		     const ListExpr instance, 
		     const int errorPos, 
		     ListExpr& errorInfo, 
		     bool& correct ) {
  CcPoint* newpoint;

  if (nl->ListLength(instance) == 2) {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);
    ListExpr Nom_x;
    ListExpr Denom_x;
    ListExpr Nom_y;
    ListExpr Denom_y;
		
    if (!nl->IsAtom(First) && !nl->IsAtom(Second)) {
      Nom_x = nl->First(First);
      Denom_x = nl->Second(First);
      Nom_y = nl->First(Second);
      Denom_y = nl->Second(Second);
      
      if (
	  nl->IsAtom(Nom_x) && nl->AtomType(Nom_x) == IntType
	  && 	nl->IsAtom(Denom_x) && nl->AtomType(Denom_x) == IntType
	  && 	nl->IsAtom(Nom_y) && nl->AtomType(Nom_y) == IntType
	  && 	nl->IsAtom(Denom_y) && nl->AtomType(Denom_y) == IntType
	  ) {
	correct = true;
	newpoint = new CcPoint
	  (
	   nl->IntValue(Nom_x),
	   nl->IntValue(Denom_x),
	   nl->IntValue(Nom_y),
	   nl->IntValue(Denom_y)
	   );
	return SetWord(newpoint);
      }
    }
  }
  correct = false;
  
  return SetWord(Address(0));
}

static Word CreatePoint(const ListExpr typeInfo) {
  return (SetWord(new CcPoint()));
}

static void DeletePoint(Word &w) {
  delete (CcPoint *)w.addr;
  w.addr = 0;
}

static void ClosePoint(Word &w) {
  delete (CcPoint *)w.addr;
  w.addr = 0;
}

static Word ClonePoint(const Word &w) {
  return SetWord(((CcPoint *)w.addr)->Clone());
}

/*
2.4 Function Describing the Signature of the Type Constructor

*/

static ListExpr JNIExampleProperty() {
  return (nl->TwoElemList
	  (
	   nl->FourElemList
	   (
	    nl->StringAtom("Signature"),
	    nl->StringAtom("Example Type List"),
	    nl->StringAtom("List Rep"),
	    nl->StringAtom("Example List")),
	   nl->FourElemList
	   (
	    nl->StringAtom("-> DATA"),
	    nl->StringAtom("ccpointjnitest"),
	    nl->StringAtom("\"((<n1> <d1>) (<n2> <d2>))\""),
	    nl->StringAtom("\"((1 2) (3 4))\""))));
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~ccpoint~ does not have arguments, this is trivial.

*/


static bool CheckPoint( ListExpr type, ListExpr& errorInfo ) {
	return (nl->IsEqual(type, "ccpointjnitest"));
}

/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor ccpoint
(
 "ccpointjnitest",
 // name

 JNIExampleProperty,
 // property function describing signature

 OutPoint,
 // Out function

 InPoint,
 // In function

 CreatePoint,
 // object creation

 DeletePoint,
 // object deletion

 0,
 // object open

 0, 
 // object save

 ClosePoint,
 // object close

 ClonePoint,
 // object clone

 DummyCast,
 // cast function

 CheckPoint,
 // kind checking function

 0,
 // predef. pers. function for model
 TypeConstructor::DummyInModel,	
 TypeConstructor::DummyOutModel,
 TypeConstructor::DummyValueToModel,
 TypeConstructor::DummyValueListToModel

);

/*
3 Creating Operators

3.1 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/
static ListExpr ccpointccpointBool( ListExpr args ) {
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 ) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (    nl->IsEqual(arg1, "ccpointjnitest") 
	 && nl->IsEqual(arg2, "ccpointjnitest") )  
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

static ListExpr ccpointccpointDouble(ListExpr args) {
  ListExpr arg1, arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if (   nl->IsEqual(arg1, "ccpointjnitest") 
	&& nl->IsEqual(arg2, "ccpointjnitest"))
      return nl->SymbolAtom("real");
  }
  return nl->SymbolAtom("typeerror");
}
	
/*
3.2 Selection Function
  
Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/

static int simpleSelect (ListExpr args ){
  return 0; 
}

/*
3.3 Value Mapping Function

*/
static int equalsFun (Word* args, Word& result, 
		      int message, Word& local, 
		      Supplier s)
/* Equals predicate for two ccpoints. */
{
  CcPoint* p1;
  CcPoint* p2;
  
  p1 = ((CcPoint *)args[0].addr);
  p2 = ((CcPoint *)args[1].addr);
  
  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result
  
  ((CcBool *)result.addr)->Set(true, p1->equals(*p2));
  //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)
  
  return 0;
}

static int distanceFun(Word *args, Word &result, 
		       int message, Word &local, 
		       Supplier s) {
  /* distance predicate for two ccpoints. */
  CcPoint *p1;
  CcPoint *p2;

  p1 = ((CcPoint *)args[0].addr);
  p2 = ((CcPoint *)args[1].addr);

  result = qp->ResultStorage(s);	
  //query processor has provided
  //a CcBool instance to take the result																
  ((CcReal *)result.addr)->Set(true, p1->distance(*p2));
  //the first argument says the boolean
  //value is defined, the second is the
  //real double value)
  
  return 0;
}

/*

3.4 Definition of Operators

*/

const string equalsSpec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccpointjniexample ccpointjniexample)"
"-> bool</text--->"
"<text>equals(p1, p2) where"
" p1 and p2 are of type ccpointjniexample"
"</text--->"
"<text>gets the identity of points.</text--->"
"<text>equals(p1,p2))</text--->"
") )";

const string distanceSpec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" )"
"( <text>(ccpointjniexample ccpointjniexample)" 
"-> double</text--->"
"<text>distance(p1, p2) where"
" p1 and p2 are of type ccpointjniexample"
"</text--->"
"<text>get the distance between two points. </text--->"
"<text>distance(p1,p2))</text--->"
") )";

/*
Used to explain the signature and the meaning of the ~equals~ and ~dist~ operators.

*/

Operator equals (
	"equals", 		//name
	equalsSpec,  		//specification ....
	equalsFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	ccpointccpointBool	//type mapping 
);	
    

Operator dist(
	"dist",			//name
	distanceSpec, 		//specification ...
	distanceFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function
	ccpointccpointDouble	//type mapping
);


/*
4 Creating the Algebra

*/

class JNIExampleAlgebra : public Algebra {
 public:
  JNIExampleAlgebra() : Algebra() {
    AddTypeConstructor( &ccpoint );

    ccpoint.AssociateKind("DATA");
    /* this means that ccpoint can be used in places 
    	where types of DATA are expected, e. g. in tuples. */
    AddOperator(&equals);
    AddOperator(&dist);
  }
  ~JNIExampleAlgebra() {};
};

JNIExampleAlgebra jniexampleAlgebra; 

/*

5 Initialization

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
InitializeJNIExampleAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  jvminit = new JVMInitializer();
  env = jvminit->getEnv();
  jvm = jvminit->getJVM();

  nl = nlRef;
  qp = qpRef;
  return (&jniexampleAlgebra);
}

