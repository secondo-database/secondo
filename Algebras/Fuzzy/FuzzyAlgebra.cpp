/*
//paragraph [1] title: [{\Large \bf ]	[}]

[1] Fuzzy Algebra

2003-06-10 Thomas Behr

This class provides three datatypes CcFPoint, CcFLine and CcFRegion
and a lot of operators. The datatypes are described in
'Modellierung, Implementierung und Visualisierung unscharfer
 r"aumlicher Objekte'.

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


static JVMInitializer *jvminit=0;

static JNIEnv *env;
static JavaVM *jvm;

static jclass PointCls;
static jclass SimplePointCls;

/*
1.2 Error Function.

Prints a short message and the line number of an occurred error
referring to problems with JNI to stderr.
If this function is invoked please check your local installation of
the Java Development Kit (JDK).
*/

static void error(int line) {
  cerr << "Error in FuzzyAlgebra.cpp in line: " << line << "." << endl;
  env->ExceptionDescribe();
  exit(1);
}

/*
 2.
   The wrapper classes. The follow three classes are implemented in
   java programming language. The C++ function invoke the corresponding
   java methods.
*/

/* 2.1 Declaration of the fuzzy object classes. */

/* 2.1.1 The wrapper class for java implementation of a fuzzy point */
class CcFPoint{
public:
   CcFPoint();
   CcFPoint(const jobject jobj);
   ~CcFPoint();
   CcFPoint* Clone();
   bool IsDefined();
   void SetDefined(bool d);
   jobject GetObject();
   // the methods for operators
   CcFPoint* Add(CcFPoint* P);
   CcFPoint* AlphaCut(double alpha, bool strong);
   double BasicCard();
   double BasicSimilar(CcFPoint* P);
   double Cardinality();
   CcFPoint* Difference(CcFPoint* P);
   double ScaleFactor();
   CcFPoint* Intersection(CcFPoint* P);
   bool IsEmpty();
   double MaxValue();
   double MinValue();
   double MaxValueAt(double x, double y);
   double MidValueAt(double x, double y);
   double MinValueAt(double x, double y);
   CcFPoint* ScaledAdd(CcFPoint* P);
   CcFPoint* ScaledDifference(CcFPoint* P);
   CcFPoint* ScaledIntersection(CcFPoint* P);
   CcFPoint* ScaledUnion(CcFPoint* P);
   CcFPoint* Sharp();
   double Similar(CcFPoint* P);
   CcFPoint* Union(CcFPoint* P);
private:
  jclass cls;  // pointer to the corresponding java class Point.
  jobject obj; // pointer to the corresponding instance
  bool defined;
};


/*
  2.1.2 A class for fuzzy Lines based on  a FLine Java implementation */
class CcFLine{
  public:
     CcFLine();
     CcFLine(const jobject jobj);
     ~CcFLine();
     CcFLine* Clone();
     bool IsDefined();
     void SetDefined(bool d);
     jobject GetObject();
     ListExpr toListExpr();
     bool readFromListExpr(ListExpr LE);

     // functions for operators
     CcFLine* Add(CcFLine* L);
     CcFLine* AlphaCut(double alpha,bool strong);
     double BasicLength();
     double BasicSimilar(CcFLine* L);
     CcFPoint* Boundary();
     CcFPoint* CommonPoints(CcFLine * L);
     CcFLine* Difference(CcFLine* L);
     //CcFLine*[] Faces(); 
     double ScaleFactor();
     CcFLine* Intersection(CcFLine* L);
     bool IsEmpty();
     double Length3D();
     double Length();
     double MaxValue();
     double MaxValueAt(double x, double y);
     double MidValueAt(double x, double y);
     double MinValueAt(double x, double y);
     double MinValue();
     CcFLine* ScaledAdd(CcFLine* L);
     CcFLine* ScaledDifference(CcFLine* L);
     CcFLine* ScaledIntersection(CcFLine* L);
     CcFLine* ScaledUnion(CcFLine* L);
     CcFLine* Sharp();
     double Similar(CcFLine* L);
     CcFLine* Union(CcFLine* L);
  private:
     jclass cls;
     jobject obj;
     bool defined;
};

/* 2.1.3 The wrapper class for java implementation of a fuzzy region */
class CcFRegion{
public:
   CcFRegion();
   CcFRegion(const jobject jobj);
   ~CcFRegion();
   CcFRegion* Clone();
   bool IsDefined();
   void SetDefined(bool d);
   jobject GetObject();
   ListExpr toListExpr();
   bool readFromListExpr(ListExpr LE);


   // the methods for algebra operators

   CcFRegion* Add(const CcFRegion* R);
   CcFRegion* AlphaCut(double alpha, bool strong);
   double Area();
   double Area3D();
   double BasicArea();
   double BasicSimilar(const CcFRegion* R);
   CcFLine* Boundary();
   CcFLine* Contour();
   CcFLine* CommonLines(const CcFRegion* R); 
   CcFPoint* CommonPoints(const CcFRegion* R);
   CcFRegion* Difference(const CcFRegion* R);
   //bool Equals(CcFRegion* R);
   //CcFRegion[] faces(); // I don't know how to implement it
   double GetScaleFactor();
   CcFRegion* Holes();
   CcFRegion* Intersection(const CcFRegion* R);
   bool IsEmpty();
   double MaxZ();
   double MaxZfkt(double x, double y);
   double MidZfkt(double x, double y);
   double MinZ();
   double MinZfkt(double x, double y);
   CcFRegion* ScaledAdd(const CcFRegion* R);
   CcFRegion* ScaledDifference(const CcFRegion* R);
   CcFRegion* ScaledIntersection(const CcFRegion* R);
   CcFRegion* ScaledUnion(const CcFRegion* R);
   CcFRegion* Sharp();
   double Similar(const CcFRegion* R);
   // M9Int TopolRelation ???
   CcFRegion* Union(CcFRegion* R);
  
private:
  jclass cls;  // pointer to the corresponding java class FRegion.
  jobject obj; // pointer to the corresponding instance 
  bool defined;
};


/* 2.2 Definition of the class functions.

 */


/* 2.2.1 Definition of Point functions */

/* Standard constructor. */
CcFPoint::CcFPoint() {
  jmethodID mid;

  /* Find the class Point. */
  cls = env->FindClass("fuzzyobjects/composite/FPoint");
  if (cls == 0) {
    error(__LINE__);
  }

  /* Get the method ID of the constructor which takes no parameters. */
  mid = env->GetMethodID(cls, "<init>", "()V");
  if (mid == 0) {
    error(__LINE__);
  }
  
  /* Create a Java-object FPoint. */
  obj = env->NewObject(cls, mid);
  if (obj == 0) {
    error(__LINE__);
  }  
  defined = true;
}

/* construct a new CcFpoint from a given jobject */
CcFPoint::CcFPoint(const jobject jobj){
  /* Find the class Point. */
  cls = env->FindClass("fuzzyobjects/composite/FPoint");  
  if (cls == 0) {
    error(__LINE__);
  }
  obj = jobj;
  defined=true;
}


/* Destructor of a CcFPoint. Deletes the corresponding java object. */ 
CcFPoint::~CcFPoint(){
  env->DeleteLocalRef(obj);
}

/* returns the jobject to manage */
jobject CcFPoint::GetObject(){
  return obj;
}

bool CcFPoint::IsDefined(){
  return defined;
}

void CcFPoint::SetDefined(bool d){
  defined=d;
}


/* returns a clone of this CcFPoint */
CcFPoint* CcFPoint::Clone(){
  jmethodID mid;
  jobject jobj; 

  mid=env->GetMethodID(cls,"copy","()Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  
  jobj = env->CallObjectMethod(obj,mid);
  if(jobj==0) error(__LINE__);

  return new CcFPoint(jobj);
}


/* The operator functions for a fpoint.
   The documentation for this operators you can find
   in the java implementation or in 
   Modellierung,Implementierung und Visualisierung unscharfer
   r"aumlicher Objekte. */

CcFPoint* CcFPoint::Add(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"add",
                       "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  
}

CcFPoint* CcFPoint::AlphaCut(double alpha, bool strong){
  jmethodID mid = env->GetMethodID(cls,"alphaCut","(DZ)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,alpha,strong);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);                                    
}

double CcFPoint::BasicCard(){
  jmethodID mid = env->GetMethodID(cls,"basicCard","()D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid);
  return res; 
}

double CcFPoint::BasicSimilar(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"basicSimilar","(Lfuzzyobjects/composite/FPoint;)D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid,P->obj);
  return res;
}

double CcFPoint::Cardinality(){
  jmethodID mid = env->GetMethodID(cls,"card","()D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid);
  return res;
}

CcFPoint* CcFPoint::Difference(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"difference",
                          "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  
}

double CcFPoint::ScaleFactor(){
   jmethodID mid = env->GetMethodID(cls,"getSF","()D");
   if(mid==0) error(__LINE__);
   jdouble res = env->CallDoubleMethod(obj,mid);
   return res;
}

CcFPoint* CcFPoint::Intersection(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"intersection",
                          "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  
}

bool CcFPoint::IsEmpty(){
  jmethodID mid = env->GetMethodID(cls,"isEmpty","()Z");
  if(mid==0) error(__LINE__);
  jboolean res = env->CallBooleanMethod(obj,mid);
  return res;
}

double CcFPoint::MaxValue(){
  jmethodID mid = env->GetMethodID(cls,"maxZ","()D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid);
  return res; 
}


double CcFPoint::MinValue(){
  jmethodID mid = env->GetMethodID(cls,"minZ","()D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid);
  return res; 
}


double CcFPoint::MaxValueAt(double x, double y){
  jmethodID mid = env->GetMethodID(cls,"maxZfkt","(DD)D");
  if(mid==0) error(__LINE__);
  return env->CallDoubleMethod(obj,mid,x,y);
}


double CcFPoint::MidValueAt(double x, double y){
  jmethodID mid = env->GetMethodID(cls,"midZfkt","(DD)D");
  if(mid==0) error(__LINE__);
  return env->CallDoubleMethod(obj,mid,x,y);
}


double CcFPoint::MinValueAt(double x, double y){
  jmethodID mid = env->GetMethodID(cls,"minZfkt","(DD)D");
  if(mid==0) error(__LINE__);
  return env->CallDoubleMethod(obj,mid,x,y);
}


CcFPoint* CcFPoint::ScaledAdd(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"scaledAdd",
                          "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  
}

CcFPoint* CcFPoint::ScaledDifference(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"scaledDifference",
                          "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  
}


CcFPoint* CcFPoint::ScaledIntersection(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"scaledIntersection",
                          "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  
}

CcFPoint* CcFPoint::ScaledUnion(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"scaledUnion",
                          "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  
}

CcFPoint* CcFPoint::Sharp(){
  jmethodID mid = env->GetMethodID(cls,"sharp","()Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);
}

double CcFPoint::Similar(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"similar","(Lfuzzyobjects/composite/FPoint;)D");
  if(mid==0) error(__LINE__);
  return env->CallDoubleMethod(obj,mid,P->obj);
}

CcFPoint* CcFPoint::Union(CcFPoint* P){
  jmethodID mid = env->GetMethodID(cls,"union",
                          "(Lfuzzyobjects/composite/FPoint;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,P->obj);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);  

}




/* Definition of functions for FLines  */
CcFLine::CcFLine(){
  jmethodID mid;
  cls = env->FindClass("fuzzyobjects/composite/FLine");
  if(cls==0) error(__LINE__);

  mid=env->GetMethodID(cls,"<init>","()V");
  if(mid==0) error(__LINE__);

  obj = env->NewObject(cls,mid);
  if(obj==0) error(__LINE__);

  defined=true;
}

/*create a CcFLine from  a given Java object */
CcFLine::CcFLine(const jobject jobj){
   cls = env->FindClass("fuzzyobjects/composite/FLine");  
   if(cls==0) error(__LINE__);
   obj=jobj;
   defined=true;
}
 
/* the standard destructor */
CcFLine::~CcFLine(){
  env->DeleteLocalRef(obj);
}


/* Convert simple elementary objects from and to lists */
/* convert a fuzzy elementary point to ListExpr,
   no check wether p is really a FPoint */
static ListExpr fEPointToListExpr(jobject FEP){
     jclass fepclass = env->GetObjectClass(FEP);
     jmethodID mid = env->GetMethodID(fepclass,"getX","()I");
     if(mid==0) error(__LINE__);
     int x = env->CallIntMethod(FEP,mid);
     mid = env->GetMethodID(fepclass,"getY","()I");
     int y = env->CallIntMethod(FEP,mid);
     mid = env->GetMethodID(fepclass,"getZ","()D");
     float z =(float) env->CallDoubleMethod(FEP,mid);
     return nl->ThreeElemList( nl->IntAtom(x), nl->IntAtom(y),nl->RealAtom(z));
}

/* read a FEPoint from a ListExpr,
   if LE is not a valid representation of a fepoint then null is returned*/
static jobject ListExprTofEPoint(ListExpr &LE){
   if( (nl->ListLength(LE))!=3)
      return 0;
   ListExpr a1 = nl->First(LE);
   ListExpr a2 = nl->Second(LE);
   ListExpr a3 = nl->Third(LE);
   int x,y;
   double z;
   if(nl->AtomType(a1)!=IntType)
      return 0;
   x = nl->IntValue(a1);
   if(nl->AtomType(a2)!=IntType)
      return 0;
   y =(nl->IntValue(a2));
   if(nl->AtomType(a3)!=RealType)
      return 0;
   z = (nl->RealValue(a3));
   jclass cls = env->FindClass("fuzzyobjects/simple/fEPoint");
   if(cls==0) error(__LINE__);
   jmethodID mid = env->GetMethodID(cls,"<init>","(IID)V");
   if(mid==0) error(__LINE__);
   jobject res= env->NewObject(cls,mid,x,y,z);
   if(res==0) error(__LINE__);
   return res;
}

/** returns the list representation for a given fuzzy segment */
static ListExpr FSegmentToListExpr(jobject obj){
  jclass cls = env->GetObjectClass(obj);
  jmethodID mid1 = env->GetMethodID(cls,"getP1","()Lfuzzyobjects/simple/fEPoint;");
  if(mid1==0) error(__LINE__);
  jmethodID mid2 = env->GetMethodID(cls,"getP2","()Lfuzzyobjects/simple/fEPoint;");
  if(mid2==0) error(__LINE__);
  jobject P1 = env->CallObjectMethod(obj,mid1);
  jobject P2 = env->CallObjectMethod(obj,mid2);
  ListExpr res = nl->TwoElemList( fEPointToListExpr(P1),fEPointToListExpr(P2));
  return res;
}

/** convert a ListExpr to a fuzzy segment,
    if the given ListExpr is not a valid representation of a 
    fuzzy segment, then null is returned */
static jobject ListExprToFSegment(ListExpr LE){
  if(nl->ListLength(LE)!=2)
    return 0;
  jclass cls = env->FindClass("fuzzyobjects/simple/fSegment");
  if(cls==0) error(__LINE__);
  jmethodID mid = env->GetMethodID(cls,"<init>","(Lfuzzyobjects/simple/fEPoint;Lfuzzyobjects/simple/fEPoint;)V");
  if(mid==0) error(__LINE__);
  ListExpr P1L = nl->First(LE);
  jobject P1 = ListExprTofEPoint(P1L);
  if(P1==0) return 0;
  ListExpr P2L = nl->Second(LE);
  jobject P2 = ListExprTofEPoint(P2L);
  if(P2==0) return 0;
  return env->NewObject(cls,mid,P1,P2);
}

/** convert a ListExpr to a fuzzy triangle,
    if the given ListExpr is not a valid representation of a 
    fuzzy triangle, then null is returned */
static jobject ListExprToFTriangle(ListExpr LE){
  if(nl->ListLength(LE)!=3)
    return 0;
  jclass cls = env->FindClass("fuzzyobjects/simple/fTriangle");
  if(cls==0) error(__LINE__);
  jmethodID mid = env->GetMethodID(cls,"<init>","(Lfuzzyobjects/simple/fEPoint;Lfuzzyobjects/simple/fEPoint;Lfuzzyobjects/simple/fEPoint;)V");
  if(mid==0) error(__LINE__);
  ListExpr P1L = nl->First(LE);
  jobject P1 = ListExprTofEPoint(P1L);
  if(P1==0) return 0;
  ListExpr P2L = nl->Second(LE);
  jobject P2 = ListExprTofEPoint(P2L);
  if(P2==0) return 0;
  ListExpr P3L = nl->Third(LE);
  jobject P3 = ListExprTofEPoint(P3L);
  if(P3==0) return 0; 
  return env->NewObject(cls,mid,P1,P2,P3);
}

/** returns the list representation for a given fuzzy segment */
static ListExpr FTriangleToListExpr(jobject obj){
  jclass cls = env->GetObjectClass(obj);
  jmethodID mid1 = env->GetMethodID(cls,"getP1","()Lfuzzyobjects/simple/fEPoint;");
  if(mid1==0) error(__LINE__);
  jmethodID mid2 = env->GetMethodID(cls,"getP2","()Lfuzzyobjects/simple/fEPoint;");
  if(mid2==0) error(__LINE__);
  jmethodID mid3 = env->GetMethodID(cls,"getP3","()Lfuzzyobjects/simple/fEPoint;");
  jobject P1 = env->CallObjectMethod(obj,mid1);
  jobject P2 = env->CallObjectMethod(obj,mid2);
  jobject P3 = env->CallObjectMethod(obj,mid3);
  ListExpr res = nl->ThreeElemList( fEPointToListExpr(P1),fEPointToListExpr(P2),fEPointToListExpr(P3));
  return res;
}


/* convert the CcFLine to ListExpr */
ListExpr CcFLine::toListExpr(){
 /*
   // the old implementation using java conversion 
   jmethodID mid;
   char* jnlstr;
   jstring jnljstr;
   string cppstr;
   ListExpr le;
   bool success;
   mid=env->GetMethodID(cls,"toListString","()Ljava/lang/String;");
   if(mid==0) error(__LINE__);
   jnljstr =(jstring) env->CallObjectMethod(obj,mid);
   if(jnljstr==0) error(__LINE__);
   jnlstr=(char *)env->GetStringUTFChars(jnljstr,0);
   if(jnlstr==0) error(__LINE__);
   cppstr=jnlstr;
   success=nl->ReadFromString(cppstr,le);
   if(!success) error(__LINE__);
   return le; 
 */
  // the new cpp implementation
   jmethodID mid;
   mid = env->GetMethodID(cls,"getSF","()D");
   if(mid==0) error(__LINE__);
   float Z = (float) env->CallDoubleMethod(obj,mid);

   mid = env->GetMethodID(cls,"getSize","()I");
   if(mid==0) error(__LINE__);
   int size = env->CallIntMethod(obj,mid);

   mid = env->GetMethodID(cls,"getSegmentAt","(I)Lfuzzyobjects/simple/fSegment;");
   if(mid==0) error(__LINE__);

   ListExpr Segments;
   ListExpr Last;
   if(size==0)
      Segments = nl->TheEmptyList();
   else{
      jobject S = env->CallObjectMethod(obj,mid,0);
      if(S==0) error(__LINE__);
      Segments = nl->OneElemList(FSegmentToListExpr(S));
      Last = Segments;
   }
   jobject NextSegment;
   for(int i=1;i<size;i++){
       NextSegment = env->CallObjectMethod(obj,mid,i);
       if(NextSegment==0) error(__LINE__);
       Last = nl->Append(Last,FSegmentToListExpr(NextSegment));
  }
   return nl->TwoElemList(nl->RealAtom(Z),Segments);
  }

/* read the CcFLine from given ListExpr */
bool CcFLine::readFromListExpr(ListExpr LE){
  if (nl->ListLength(LE)!=2){  // error  (scaledfactor <Segmentlist>)
     return false;
  }
  ListExpr Factor = nl->First(LE);
  ListExpr Segments = nl->Second(LE);
  if(nl->AtomType(Factor)!=RealType){
      return false;
  }
  double z = nl->RealValue(Factor);
  // remove all objects from the Fline
  jmethodID mid = env->GetMethodID(cls,"clear","()V");
  if(mid==0) error(__LINE__);
  env->CallVoidMethod(obj,mid);  
  
  // set the factor
  mid = env->GetMethodID(cls,"setSF","(D)Z");
  if(mid==0) error(__LINE__);
  bool ok = env->CallBooleanMethod(obj,mid,z);

  // add the Objects
  mid = env->GetMethodID(cls,"add","(Lfuzzyobjects/simple/fSegment;)Z");
  if(mid==0) error(__LINE__);
  jobject NextSegment;
  ListExpr SL;  // list for a single segment
  int count =0;
  while(ok & !nl->IsEmpty(Segments)){
        SL = nl->First(Segments);
        NextSegment = ListExprToFSegment(SL);
	count++;
	if(NextSegment==0){
	    ok = false;
	}
	else{
	   ok = env->CallBooleanMethod(obj,mid,NextSegment);
	}
	Segments = nl->Rest(Segments);
    }
  return ok;
}



/* returns the java obkject from this cpp class */
jobject CcFLine::GetObject(){
   return obj;
}

bool CcFLine::IsDefined(){
  return IsDefined();
}

void CcFLine::SetDefined(bool d){
   defined=d;
}

/* get a clone of this CcFLine */
CcFLine* CcFLine::Clone(){
   jmethodID mid;
   jobject jobj;
   
   mid = env->GetMethodID(cls,"copy","()Lfuzzyobjects/composite/FLine;");
   if(mid==0) error(__LINE__);
   
   jobj = env->CallObjectMethod(obj,mid);
   if(jobj==0) error(__LINE__);

   return new CcFLine(jobj);
}


/* The functions to define operators of Fuzzy Lines.
   A description for the functions you can find in the
   documentation of the java class FLine 
*/

CcFLine* CcFLine::Add(CcFLine* L){
   jmethodID mid;
   mid = env->GetMethodID(cls,"add",
           "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
   if(mid==0) error(__LINE__);
   jobject res = env->CallObjectMethod(obj,mid,L->obj);
   if(res==0) error(__LINE__);
   return new CcFLine(res);
}

CcFLine* CcFLine::AlphaCut(double alpha,bool strong){
   jmethodID mid = env->GetMethodID(cls,"alphaCut","(DZ)Lfuzzyobjects/composite/FLine;");
   if(mid==0) error(__LINE__);
   jobject res = env->CallObjectMethod(obj,mid,alpha,strong);
   if(res==0) error(__LINE__);
   return new CcFLine(res);
}

double CcFLine::BasicLength(){
   jmethodID mid = env->GetMethodID(cls,"basicLen","()D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}

double CcFLine::BasicSimilar(CcFLine* L){
  jmethodID mid = env->GetMethodID(cls,"basicSimilar","(Lfuzzyobjects/composite/FLine;)D");
  if(mid==0) error(__LINE__);
  return env->CallDoubleMethod(obj,mid,L->obj);
}

CcFPoint* CcFLine::Boundary(){
  jmethodID mid = env->GetMethodID(cls,"boundary","()Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid);
  if(res==0) error(__LINE__);
  return new CcFPoint(res);
}

CcFPoint* CcFLine::CommonPoints(CcFLine* L){
   jmethodID mid;
   mid = env->GetMethodID(cls,"commonPoints",
                  "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FPoint;");
   if(mid==0) error(__LINE__);
   jobject res = env->CallObjectMethod(obj,mid,L->obj);
   if(res==0) error(__LINE__);
   return new CcFPoint(res);
}

CcFLine* CcFLine::Difference(CcFLine* L){
  jmethodID mid;
  mid = env->GetMethodID(cls,"difference",
               "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,L->obj);
  if(res==0) error(__LINE__);
  return new CcFLine(res);   
}

double CcFLine::ScaleFactor(){
   jmethodID mid = env->GetMethodID(cls,"getSF","()D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}

CcFLine* CcFLine::Intersection(CcFLine* L){
  jmethodID mid;
  mid = env->GetMethodID(cls,"intersection",
               "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,L->obj);
  if(res==0) error(__LINE__);
  return new CcFLine(res);
}


bool CcFLine::IsEmpty(){
   jmethodID mid = env->GetMethodID(cls,"isEmpty","()Z");
   if(mid==0) error(__LINE__);
   return env->CallBooleanMethod(obj,mid);
}

double CcFLine::Length3D(){
   jmethodID mid = env->GetMethodID(cls,"len3D","()D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}

double CcFLine::Length(){
   jmethodID mid = env->GetMethodID(cls,"length","()D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}

double CcFLine::MaxValue(){
   jmethodID mid = env->GetMethodID(cls,"maxZ","()D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}

double CcFLine::MaxValueAt(double x, double y){
   jmethodID mid = env->GetMethodID(cls,"maxZfkt","(DD)D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}

double CcFLine::MidValueAt(double x, double y){
   jmethodID mid = env->GetMethodID(cls,"midZfkt","(DD)D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}

double CcFLine::MinValueAt(double x, double y){
   jmethodID mid = env->GetMethodID(cls,"minZfkt","(DD)D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid);
}


double CcFLine::MinValue(){
  jmethodID mid = env->GetMethodID(cls,"minZ","()D");
  if(mid==0) error(__LINE__);
  return env->CallDoubleMethod(obj,mid); 
}

CcFLine* CcFLine::ScaledAdd(CcFLine* L){
  jmethodID mid;
  mid = env->GetMethodID(cls,"scaledAdd",
               "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,L->obj);
  if(res==0) error(__LINE__);
  return new CcFLine(res);   
}

CcFLine* CcFLine::ScaledDifference(CcFLine* L){
  jmethodID mid;
  mid = env->GetMethodID(cls,"scaledDifference",
               "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,L->obj);
  if(res==0) error(__LINE__);
  return new CcFLine(res);   
}

CcFLine* CcFLine::ScaledIntersection(CcFLine* L){
  jmethodID mid;
  mid = env->GetMethodID(cls,"scaledIntersection",
               "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,L->obj);
  if(res==0) error(__LINE__);
  return new CcFLine(res);   
}

CcFLine* CcFLine::ScaledUnion(CcFLine* L){
  jmethodID mid;
  mid = env->GetMethodID(cls,"scaledUnion",
               "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,L->obj);
  if(res==0) error(__LINE__);
  return new CcFLine(res);   
}

CcFLine* CcFLine::Sharp(){
  jmethodID mid;
  mid = env->GetMethodID(cls,"sharp","()Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid);
  if(res==0) error(__LINE__);
  return new CcFLine(res);   
}

double CcFLine::Similar(CcFLine* L){
   jmethodID mid = env->GetMethodID(cls,"similar","(Lfuzzyobjects/composite/FLine;)D");
   if(mid==0) error(__LINE__);
   return env->CallDoubleMethod(obj,mid,L->obj);
}

CcFLine* CcFLine::Union(CcFLine* L){
  jmethodID mid;
  mid = env->GetMethodID(cls,"union",
               "(Lfuzzyobjects/composite/FLine;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,L->obj);
  if(res==0) error(__LINE__);
  return new CcFLine(res);   
}



/* The definition of  ccFRegion  functions*/

/* the standard constructor for fuzzy regions */
CcFRegion::CcFRegion(){
  jmethodID mid;
  /*find the fregion class */
   cls = env->FindClass("fuzzyobjects/composite/FRegion");
  if(cls==0) error(__LINE__);

  /* search the java standard constructor */
  mid=env->GetMethodID(cls,"<init>","()V");
  if(mid==0) error(__LINE__);

  /* create the java object */
  obj = env->NewObject(cls,mid);
  if(obj==0) error(__LINE__);
  defined=true;
}


/* constructor with given jobject */
CcFRegion::CcFRegion(const jobject jobj){
   cls = env->GetObjectClass(jobj);
   if(cls==0) error(__LINE__);
   obj = jobj;
   defined=true;
}

/* destructor for ccfregions*/
CcFRegion::~CcFRegion(){
  env->DeleteLocalRef(obj);
}


/*transform to a ListExpr*/
ListExpr CcFRegion::toListExpr(){
   jmethodID mid;
   mid = env->GetMethodID(cls,"getSF","()D");
   if(mid==0) error(__LINE__);
   float Z = (float) env->CallDoubleMethod(obj,mid);

   mid = env->GetMethodID(cls,"getSize","()I");
   if(mid==0) error(__LINE__);
   int size = env->CallIntMethod(obj,mid);

   mid = env->GetMethodID(cls,"getTriangleAt","(I)Lfuzzyobjects/simple/fTriangle;");
   if(mid==0) error(__LINE__);

   ListExpr Triangles;
   ListExpr Last;
   if(size==0)
      Triangles = nl->TheEmptyList();
   else{
      jobject T = env->CallObjectMethod(obj,mid,0);
      if(T==0) error(__LINE__);
      Triangles = nl->OneElemList(FTriangleToListExpr(T));
      Last = Triangles;
   }
   jobject NextTriangle;
   for(int i=1;i<size;i++){
       NextTriangle = env->CallObjectMethod(obj,mid,i);
       if(NextTriangle==0) error(__LINE__);
       Last = nl->Append(Last,FTriangleToListExpr(NextTriangle));
  }
   return nl->TwoElemList(nl->RealAtom(Z),Triangles);

}

/* reads a CcFRegion from a given ListExpr*/
bool CcFRegion::readFromListExpr(ListExpr LE){
  if (nl->ListLength(LE)!=2){  // error  (scaledfactor <Trianglelist>)
     return false;
  }
  ListExpr Factor = nl->First(LE);
  ListExpr Triangles = nl->Second(LE);
  if(nl->AtomType(Factor)!=RealType){
      return false;
  }
  double z = nl->RealValue(Factor);
  // remove all objects from the FRegion
  jmethodID mid = env->GetMethodID(cls,"clear","()V");
  if(mid==0) error(__LINE__);
  env->CallVoidMethod(obj,mid);  
  
  // set the factor
  mid = env->GetMethodID(cls,"setSF","(D)Z");
  if(mid==0) error(__LINE__);
  bool ok = env->CallBooleanMethod(obj,mid,z);

  // add the Objects
  mid = env->GetMethodID(cls,"add","(Lfuzzyobjects/simple/fTriangle;)Z");
  if(mid==0) error(__LINE__);
  jobject NextTriangle;
  ListExpr TL;  // list for a single triangle
  int count =0;
  while(ok & !nl->IsEmpty(Triangles)){
        TL = nl->First(Triangles);
        NextTriangle = ListExprToFTriangle(TL);
	count++;
	if(NextTriangle==0){
	    ok = false;
	}
	else{
	   ok = env->CallBooleanMethod(obj,mid,NextTriangle);
	}
	Triangles = nl->Rest(Triangles);
    }
  return ok;
}

/* get the java object */
jobject CcFRegion::GetObject(){
  return obj;
}

/*return true if the CcFRegion is defined*/
bool CcFRegion::IsDefined(){
   return defined;
}

/*set the defined flag for a CcFRegion */
void CcFRegion::SetDefined(bool d){
  defined=d;
}

/* make a clone from this CcFRegion */
CcFRegion* CcFRegion::Clone(){
  jmethodID mid;
  jobject jobj;
  mid = env->GetMethodID(cls,"copy","()Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);

  jobj = env->CallObjectMethod(obj,mid);
  if(jobj==0) error(__LINE__);

  return new CcFRegion(jobj);
}

/* The Methods for Algebra-Operators */

CcFRegion* CcFRegion::Add(const CcFRegion* R){
  jmethodID mid = env->GetMethodID(cls,"add","(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,R->obj);
  if(jobj==0) error(__LINE__);
  return new CcFRegion(jobj);
}

CcFRegion* CcFRegion::AlphaCut(double alpha, bool strong){
  jmethodID mid=env->GetMethodID(cls,"alphaCut","(DZ)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,alpha,strong);
  if(jobj==0) error(__LINE__);
  return new CcFRegion(jobj);
}

double CcFRegion::Area(){
  jmethodID mid = env->GetMethodID(cls,"area","()D");
  if(mid==0) error(__LINE__);
  jdouble a = env->CallDoubleMethod(obj,mid);
  return a;
}

double CcFRegion::Area3D(){
  jmethodID mid= env->GetMethodID(cls,"area3D","()D");
  if(mid==0) error(__LINE__);
  jdouble d = env->CallDoubleMethod(obj,mid);
  return d;
}

double CcFRegion::BasicArea(){
  jmethodID mid = env->GetMethodID(cls,"basicArea","()D");
  if(mid==0) error(__LINE__);
  jdouble d = env->CallDoubleMethod(obj,mid);
  return d;
}

double CcFRegion::BasicSimilar(const CcFRegion* R){
  jmethodID mid = env->GetMethodID(cls,"basicSimilar","(Lfuzzyobjects/composite/FRegion;)D");
  if(mid==0) error(__LINE__);
  jdouble d = env->CallDoubleMethod(obj,mid,R->obj);
  return d;
}

CcFLine* CcFRegion::Boundary(){
  jmethodID mid = env->GetMethodID(cls,"boundary","()Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid);
  if(jobj==0) error(__LINE__);
  CcFLine* L = new CcFLine(jobj);
  return L;
}

CcFLine* CcFRegion::Contour(){
  jmethodID mid= env->GetMethodID(cls,"contour","()Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid);
  if(jobj==0) error(__LINE__);
  return new CcFLine(jobj);
}

CcFLine* CcFRegion::CommonLines(const CcFRegion* R){
  jmethodID mid;
  mid = env->GetMethodID(cls,"commonLines",
               "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FLine;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,R->obj);
  if(jobj==0) error(__LINE__);
  return new CcFLine(jobj);
}

CcFPoint* CcFRegion::CommonPoints(const CcFRegion* R){
  jmethodID mid;
  mid = env->GetMethodID(cls,"commonPoints",
             "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FPoint;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,R->obj);
  if(jobj==0) error(__LINE__);
  return new CcFPoint(jobj);
}

CcFRegion* CcFRegion::Difference(const CcFRegion* R){
  jmethodID mid;
  mid= env->GetMethodID(cls,"difference",
              "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,R->obj);
  if(jobj==0) error(__LINE__);
  return new CcFRegion(jobj); 
}

/*
bool CcFRegion::Equals(CcFRegion* R){
  jmethodID mid = env->GetMethodID(cls,"equals","(Lfuzzyobjects/composite/FRegion;)Z");
  if(mid==0) error(__LINE__);
  jboolean res = env->CallBooleanMethod(obj,mid,R->obj);
  return res;
}
*/

double CcFRegion::GetScaleFactor(){
   jmethodID mid = env->GetMethodID(cls,"getSF","()D");
   if(mid==0) error(__LINE__);
   jdouble res = env->CallDoubleMethod(obj,mid);
   return res; 
}

CcFRegion* CcFRegion::Holes(){
  jmethodID mid = env->GetMethodID(cls,"holes","()Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid);
  if(jobj==0) error(__LINE__);
  return new CcFRegion(jobj);
}


CcFRegion* CcFRegion::Intersection(const CcFRegion* R){
  jmethodID mid;
  mid =env->GetMethodID(cls,"intersection",
             "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject jobj = env->CallObjectMethod(obj,mid,R->obj);
  if(jobj==0) error(__LINE__);
  return new CcFRegion(jobj); 
}

bool CcFRegion::IsEmpty(){
  jmethodID mid = env->GetMethodID(cls,"isEmpty","()Z");
  if(mid==0) error(__LINE__);
  jboolean res = env->CallBooleanMethod(obj,mid);
  return res;
}

double CcFRegion::MaxZ(){
   jmethodID mid = env->GetMethodID(cls,"maxZ","()D");
   if(mid==0) error(__LINE__);
   jdouble res = env->CallDoubleMethod(obj,mid);
   return res;
}

double CcFRegion::MaxZfkt(double x, double y){
   jmethodID mid = env->GetMethodID(cls,"maxZfkt","(DD)D");
   if(mid==0) error(__LINE__);
   jdouble res = env->CallDoubleMethod(obj,mid,x,y);
   return res;
}

double CcFRegion::MidZfkt(double x, double y){
  jmethodID mid = env->GetMethodID(cls,"midZfkt","(DD)D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid,x,y);
  return res;
}

double CcFRegion::MinZ(){
  jmethodID mid = env->GetMethodID(cls,"minZ","()D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid);
  return res;
}

double CcFRegion::MinZfkt(double x, double y){
  jmethodID mid = env->GetMethodID(cls,"minZfkt","(DD)D");
  if(mid==0) error(__LINE__);
  jdouble res = env->CallDoubleMethod(obj,mid,x,y);
  return res;
}

CcFRegion* CcFRegion::ScaledAdd(const CcFRegion* R){
  jmethodID mid;
  mid= env->GetMethodID(cls,"scaledAdd",
               "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,R->obj);
  if(res==0) error(__LINE__);
  return  new CcFRegion(res);
}


CcFRegion* CcFRegion::ScaledDifference(const CcFRegion* R){
  jmethodID mid;
  mid= env->GetMethodID(cls,"scaledDifference",
               "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,R->obj);
  if(res==0) error(__LINE__);
  return  new CcFRegion(res);
}

CcFRegion* CcFRegion::ScaledIntersection(const CcFRegion* R){
  jmethodID mid;
  mid= env->GetMethodID(cls,"scaledIntersection",
               "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,R->obj);
  if(res==0) error(__LINE__);
  return  new CcFRegion(res);
}


CcFRegion* CcFRegion::ScaledUnion(const CcFRegion* R){
  jmethodID mid;
  mid= env->GetMethodID(cls,"scaledUnion",
               "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,R->obj);
  if(res==0) error(__LINE__);
  return  new CcFRegion(res);
}

CcFRegion* CcFRegion::Sharp(){
  jmethodID mid = env->GetMethodID(cls,"sharp","()Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid);
  if(res==0) error(__LINE__);
  return new CcFRegion(res);
}

double CcFRegion::Similar(const CcFRegion* R){
   jmethodID mid = env->GetMethodID(cls,"similar","(Lfuzzyobjects/composite/FRegion;)D");
   if(mid==0) error(__LINE__);
   jdouble res = env->CallDoubleMethod(obj,mid,R->obj);
   return res;
}


CcFRegion* CcFRegion::Union(CcFRegion* R){
 jmethodID mid;
  mid= env->GetMethodID(cls,"union",
               "(Lfuzzyobjects/composite/FRegion;)Lfuzzyobjects/composite/FRegion;");
  if(mid==0) error(__LINE__);
  jobject res = env->CallObjectMethod(obj,mid,R->obj);
  if(res==0) error(__LINE__);
  return  new CcFRegion(res);
}



/* static functions to define typeconstructors and operators */



/* get the list representation of a FPoint */
static ListExpr OutFPoint( ListExpr typeInfo, Word value ){
/*
   // use java function
  CcFPoint* ccfpoint;
  ccfpoint = (CcFPoint*)(value.addr);
  jclass cls;
  jmethodID mid;
  char* jnlstr;
  jstring jnljstr;
  string cppstr;
  ListExpr le;
  bool success;

  cls = env->FindClass("fuzzyobjects/composite/FPoint");
  if (cls == 0) error(__LINE__);

  mid = env->GetMethodID(cls,"toListString","()Ljava/lang/String;");
  if(mid==0) error(__LINE__);

  jnljstr = (jstring) env->CallObjectMethod(ccfpoint->GetObject(),mid);
  if(jnljstr==0) error(__LINE__);

  // Get the c string of above java string
  jnlstr = (char *)env->GetStringUTFChars(jnljstr, 0);
  if (jnlstr == 0) error(__LINE__);

  cppstr = jnlstr;

  success=nl->ReadFromString(cppstr, le);

  if(success==false) error(__LINE__);
  return le;
*/

  // converting in c++

  CcFPoint* ccfpoint;
  ccfpoint = (CcFPoint*)(value.addr);
  jmethodID mid = env->GetMethodID(PointCls,"getSize","()I");
  if(mid==0) error(__LINE__);

  jobject obj = ccfpoint->GetObject();

  int size = env->CallIntMethod(obj,mid);

  mid = env->GetMethodID(PointCls,"getSF","()D");
  if(mid==0) error(__LINE__);
  float z = env->CallDoubleMethod(obj,mid);

  mid = env->GetMethodID(PointCls,"getPointAt","(I)Lfuzzyobjects/simple/fEPoint;");
  if(mid==0) error(__LINE__);

   ListExpr Points;
   ListExpr Last;

   if(size==0)
      Points = nl->TheEmptyList();
   else{
      jobject p = env->CallObjectMethod(obj,mid,0);
      if(p==0) error(__LINE__);
      Points = nl->OneElemList(fEPointToListExpr(p));
      Last = Points;
   }
   jobject NextPoint;
   for(int i=1;i<size;i++){
       NextPoint = env->CallObjectMethod(obj,mid,i);
       if(NextPoint==0) error(__LINE__);
       Last = nl->Append(Last,fEPointToListExpr(NextPoint));
  }
   return nl->TwoElemList(nl->RealAtom(z),Points);
  
}



static Word InFPoint (const ListExpr typeInfo,
		     const ListExpr instance,
		     const int errorPos,
		     ListExpr& errorInfo,
		     bool& correct ) {

/*
  // a implementation using java function to convert
  CcFPoint* newfpoint;
  string  nlstr;
  jstring jstr;
  jmethodID mid;
  jmethodID cons;
  jclass cls;
  jobject jfpoint;
  jboolean success;
  char* nlchar;

  // we need a string representation for the ListExpr instance
  if(nl->WriteToString(nlstr,instance)==false)
    error(__LINE__);

  nlchar = (char *)nlstr.c_str();

  jstr = env->NewStringUTF(nlchar);
  if (jstr == 0) error(__LINE__);


  // Find the class FPoint.
  cls = env->FindClass("fuzzyobjects/composite/FPoint");
  if (cls == 0) {
    error(__LINE__);
  }

  // Get the method ID of the constructor which takes no parameters.
  cons = env->GetMethodID(cls, "<init>", "()V");
  if (cons == 0) {
    error(__LINE__);
  }

  // Create a Java-object FPoint.
  jfpoint = env->NewObject(cls, cons);
  if (jfpoint == 0) {
    error(__LINE__);
  }

  // get the method id to read in a FPoint from a String
  mid = env->GetMethodID(cls,"readFromListString","(Ljava/lang/String;)Z");
  if(mid==0) error(__LINE__);

  success=env->CallBooleanMethod(jfpoint,mid,jstr);

  if(success==false){
     // destroy the fpoint
     env->DeleteLocalRef(jfpoint);
     correct = false;
     return SetWord(Address(0));
  } else{
    correct=true;
    newfpoint = new CcFPoint(jfpoint);
    return SetWord(newfpoint);
  }

*/  

// converting in  cpp
  if (nl->ListLength(instance)!=2){  // error
     return SetWord(Address(0));
     correct = false;
  }

  ListExpr Factor = nl->First(instance);
  ListExpr Points = nl->Second(instance);
 
 if(nl->AtomType(Factor)!=RealType){
       correct=false;
       return SetWord(Address(0));
  }
  double z = nl->RealValue(Factor);

  // create a new FPoint
  jmethodID mid = env->GetMethodID(PointCls,"<init>","()V");
  if(mid==0) error(__LINE__);
  jobject FP = env->NewObject(PointCls,mid);
  if(FP==0) error(__LINE__);
  
  mid = env->GetMethodID(PointCls,"setSF","(D)Z");
  if(mid==0) error(__LINE__);
  bool ok = env->CallBooleanMethod(FP,mid,z);
   
   cout<<"scalefactor is set to "<<z<<endl;
   

   mid = env->GetMethodID(PointCls,"add","(Lfuzzyobjects/simple/fEPoint;)Z");
   if(mid==0) error(__LINE__);

   cout<<"add methodid "<<mid<<endl;

   jobject NextPoint;
   ListExpr PL;  // list for a singe point
   cout<<"start conversion of points "<<endl;
   int count =0;
   while(ok & !nl->IsEmpty(Points)){
        PL = nl->First(Points);
        NextPoint = ListExprTofEPoint(PL);
	count++;
	if(NextPoint==0){
	    ok = false;
	    cout <<"error in ListExprTofEPoint in object number :"<<count<<endl;
	}
	else{
	   ok = env->CallBooleanMethod(FP,mid,NextPoint);
	   if(!ok)
	       cout<<"error in converting point no "<<count<<endl;
	}
	Points = nl->Rest(Points);
    }


    if(!ok){
       env->DeleteLocalRef(FP);
       correct=false;
       return SetWord(Address(0));
    } else{
      cout<<"conversion successful"<<endl;
      correct=true;
      CcFPoint* newFPoint = new CcFPoint(FP);
      return SetWord(newFPoint);
    }
}


static Word CreateFPoint(const ListExpr typeInfo) {
  return (SetWord(new CcFPoint()));
}

static void DeleteFPoint(Word &w) {
  delete (CcFPoint *)w.addr;
  w.addr = 0;
}

static void CloseFPoint(Word &w) {
  delete (CcFPoint *)w.addr;
  w.addr = 0;
}

static Word CloneFPoint(const Word &w) {
  return SetWord(((CcFPoint *)w.addr)->Clone());
}

/* static functions needed by typconstructor for a Fline */

static ListExpr OutFLine(ListExpr typeInfo,Word value){
  CcFLine* L;
  L = (CcFLine*)(value.addr);
  return L->toListExpr();  
}


static Word InFLine( const ListExpr typeInfo,
			   const ListExpr instance,
			   const int errorPos,
			   ListExpr& errorInfo,
			   bool& correct){
  CcFLine * L;
  L = new CcFLine();
  bool success= L->readFromListExpr(instance);
  if(success){
     correct=true;
     return SetWord(L);
  } else{
     correct=false;
     return SetWord(Address(0));
  }
}

static Word CreateFLine(const ListExpr typeInfo){
   return (SetWord(new CcFLine()));
}

static void DeleteFLine(Word &w){
   delete (CcFLine *)w.addr;
   w.addr = 0;
}

static void CloseFLine(Word &w){
   delete (CcFLine *)w.addr;
   w.addr = 0;
}

static Word CloneFLine(const Word &w){
  return SetWord(((CcFLine *)w.addr)->Clone());
}



/* the static methods for FRegions needed by type constructor */

/* the out function for a fregion */
static ListExpr OutFRegion(ListExpr typeInfo,Word value){
   CcFRegion* R;
   R = (CcFRegion*)(value.addr);
   return R->toListExpr();
}

/* the in function for a fregion */
static Word InFRegion(const ListExpr typeInfo,
                     const ListExpr instance,
                     const int errorPos,
                     ListExpr& errorInfo,
                     bool& correct){
  CcFRegion* R;
  R = new CcFRegion();
  bool success=R->readFromListExpr(instance);
  if(success){
     correct=true;
     return SetWord(R);
  } else{
    correct=false;
    delete R;
    return SetWord(Address(0)); 
  }
}


/* creation of a fuzzy region */
static Word CreateFRegion(const ListExpr typeInfo){
   return (SetWord(new CcFRegion()));
}

/* delete a fuzzy region */
static void DeleteFRegion(Word &w){
  delete (CcFRegion *)w.addr;
  w.addr=0;
}

/* close a fuzzy region */
static void CloseFRegion(Word &w){
  delete (CcFRegion *)w.addr;
  w.addr = 0;
}

/* clone a fuzzy Region */
static Word CloneFRegion(const Word &w){
  return SetWord(((CcFRegion *)w.addr)->Clone());
}



/* the signature functions */
/* the signature of a Fpoint */

/*
 Function Describing the Signature of the Type Constructor
*/

static ListExpr FPointProperty() {
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
	    nl->StringAtom("fpoint"),
	    nl->StringAtom("factor <fepointlist>"),
	    nl->StringAtom("(20.4 ((0,0,0.5)(20,30,0.1)(-20,60,1.0))"))));
}


static ListExpr FLineProperty(){
   return (nl->TwoElemList
            (nl->FourElemList
               ( nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"),
                 nl->StringAtom("List Rep"),
		     nl->StringAtom("Example List")),
             nl->FourElemList
               ( nl->StringAtom("->DATA"),
		     nl->StringAtom("fline"),
		     nl->StringAtom("(scale <fuzzy segment list>)"),
		     nl->StringAtom("(100.0 (((0 0 1.0) (20 30 0.5))((0 0 0.8)(-20 -30 0.0))))"))));
}


static ListExpr FRegionProperty(){
  return ( nl->TwoElemList
             (nl->FourElemList
                 (
                   nl->StringAtom("Signature"),
                   nl->StringAtom("Example Type List"),
                   nl->StringAtom("List Rep"),
                   nl->StringAtom("Example List")),
              nl->FourElemList
                  (
                    nl->StringAtom("->DATA"),
                    nl->StringAtom("fregion"),
                    nl->StringAtom("(scale <fuzzy triangle list>)"),
                    nl->StringAtom("(100.0 ( ( (0 0 1.0)(20 30 0.5)(-20 -30 0.0))))"))));
}




/* a type checking function */

/*
Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
all type constructors  does not have arguments, this is trivial.
*/

static bool CheckPoint( ListExpr type, ListExpr& errorInfo ) {
	return (nl->IsEqual(type, "fpoint"));
}

static bool CheckFLine(ListExpr type, ListExpr& errorInfo){
  return (nl->IsEqual(type,"fline"));
}

static bool CheckFRegion(ListExpr type, ListExpr& errorInfo){
  return (nl->IsEqual(type,"fregion"));
}



/*
 Dummy Functions
 Not interesting, but needed in the definition of a type constructor.
*/

static void* DummyCast( void* addr ) {
  return (0);
}


/* sizeOf functions */
int SizeOfFPoint(){ return sizeof(CcFPoint);}

int SizeOfFLine(){ return sizeof(CcFLine);}

int SizeOfFRegion(){ return sizeof(CcFRegion);}

/*
Creation of the Type Constructor Instance
*/
TypeConstructor ccfpoint
(
 "fpoint",         // name
 FPointProperty,   // property function describing signature
 OutFPoint,        // Out function
 InFPoint,         // In function
 0,        0,      //SaveToList and RestoreFromList functions
 CreateFPoint,     // object creation
 DeleteFPoint,     // object deletion
 0,                // object open
 0,                // object save
 CloseFPoint,       // object close
 CloneFPoint,       // object clone
 DummyCast,        // cast function
 SizeOfFPoint,     // Size of a point
 CheckPoint,       // kind checking function
 0,                // predef. pers. function for model
 TypeConstructor::DummyInModel,	
 TypeConstructor::DummyOutModel,
 TypeConstructor::DummyValueToModel,
 TypeConstructor::DummyValueListToModel
);


/* the typeconstructor for fuzzy regions */
TypeConstructor ccfregion
(
   "fregion", // name
   FRegionProperty, // signature
   OutFRegion,
   InFRegion,
   0,        0,                        //SaveToList and RestoreFromList functions
   CreateFRegion,
   DeleteFRegion,
   0,
   0,
   CloseFRegion,
   CloneFRegion,
   DummyCast,
   SizeOfFRegion,     // Size of a point
   CheckFRegion,
   0,
   TypeConstructor::DummyInModel, 
   TypeConstructor::DummyOutModel, 
   TypeConstructor::DummyValueToModel,
   TypeConstructor::DummyValueListToModel
);


/* the typeconstructor for a fLine */
TypeConstructor ccfline
(
  "fline", FLineProperty,OutFLine,InFLine,0,0,
  CreateFLine,DeleteFLine,0,0,CloseFLine,
  CloneFLine,DummyCast,SizeOfFLine,CheckFLine,0,
  TypeConstructor::DummyInModel,
  TypeConstructor::DummyOutModel,
  TypeConstructor::DummyValueToModel,
  TypeConstructor::DummyValueListToModel
);





/* type mapping functions */

static ListExpr FOiFOiFOi(ListExpr args){
  if(nl->ListLength(args)==2){
      ListExpr arg1=nl->First(args);
      ListExpr arg2=nl->Second(args);
      if( nl->IsEqual(arg1,"fpoint") && nl->IsEqual(arg2,"fpoint"))
         return nl->SymbolAtom("fpoint");
      if( nl->IsEqual(arg1,"fline") && nl->IsEqual(arg2,"fline"))
         return nl->SymbolAtom("fline");
      if( nl->IsEqual(arg1,"fregion") && nl->IsEqual(arg2,"fregion"))
         return nl->SymbolAtom("fregion");
   }
   return nl->SymbolAtom("typeerror");
}

static ListExpr CommonPointsTypeMap(ListExpr args){
   if(nl->ListLength(args)==2){
     ListExpr arg1 = nl->First(args);
     ListExpr arg2 = nl->Second(args);
     if(nl->IsEqual(arg1,"fregion") && nl->IsEqual(arg2,"fregion"))
        return  nl->SymbolAtom("fpoint");
     if(nl->IsEqual(arg1,"fline") && nl->IsEqual(arg2,"fline"))
        return  nl->SymbolAtom("fpoint");
  }
  return nl->SymbolAtom("typeerror");
}

static ListExpr BoundaryTypeMap(ListExpr args){
  if(nl->ListLength(args)==1){
     ListExpr arg1 = nl->First(args);
     if(nl->IsEqual(arg1,"fregion"))
        return  nl->SymbolAtom("fline");
     if(nl->IsEqual(arg1,"fline"))
        return  nl->SymbolAtom("fpoint");
  }
  return nl->SymbolAtom("typeerror");
}

static ListExpr FOiRealBoolFOi(ListExpr args){
  if(nl->ListLength(args)==3){
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    if( nl->IsEqual(arg1,"fpoint") &&
        nl->IsEqual(arg2,"real") &&
        nl->IsEqual(arg3,"bool"))
           return nl->SymbolAtom("fpoint");
    if( nl->IsEqual(arg1,"fregion") &&
        nl->IsEqual(arg2,"real") &&
        nl->IsEqual(arg3,"bool"))
           return nl->SymbolAtom("fregion");
    if( nl->IsEqual(arg1,"fline") &&
        nl->IsEqual(arg2,"real") &&
        nl->IsEqual(arg3,"bool"))
           return nl->SymbolAtom("fline");
  }
  return nl->SymbolAtom("typeerror");
}


static ListExpr FPointReal(ListExpr args){
  if(nl->ListLength(args)==1){
     ListExpr arg1 = nl->First(args);
     if(nl->IsEqual(arg1,"fpoint"))
        return nl->SymbolAtom("real");
  } 
  return nl->SymbolAtom("typeerror");
}

static ListExpr FLineReal(ListExpr args){
  if(nl->ListLength(args)==1){
     ListExpr arg1 = nl->First(args);
     if(nl->IsEqual(arg1,"fline"))
        return nl->SymbolAtom("real");
  } 
  return nl->SymbolAtom("typeerror");
}


static ListExpr FOiFOiReal(ListExpr args){
  if(nl->ListLength(args)==2){
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if( nl->IsEqual(arg1,"fpoint") &&
        nl->IsEqual(arg2,"fpoint"))
         return nl->SymbolAtom("real");
    if( nl->IsEqual(arg1,"fregion") &&
        nl->IsEqual(arg2,"fregion"))
         return nl->SymbolAtom("real");
    if( nl->IsEqual(arg1,"fline") &&
        nl->IsEqual(arg2,"fline"))
         return nl->SymbolAtom("real");
   }
   return nl->SymbolAtom("typeerror");
}

static ListExpr FOBool(ListExpr args){
   if(nl->ListLength(args)==1){
     ListExpr arg1= nl->First(args);
     if( nl->IsEqual(arg1,"fpoint"))
       return nl->SymbolAtom("bool");
     if( nl->IsEqual(arg1,"fregion"))
       return nl->SymbolAtom("bool");
     if( nl->IsEqual(arg1,"fline"))
       return nl->SymbolAtom("bool");
   }
   return nl->SymbolAtom("typeerror");
}

static ListExpr FOReal(ListExpr args){
   if(nl->ListLength(args)==1){
     ListExpr arg1= nl->First(args);
     if( nl->IsEqual(arg1,"fpoint"))
       return nl->SymbolAtom("real");
     if( nl->IsEqual(arg1,"fregion"))
       return nl->SymbolAtom("real");
     if( nl->IsEqual(arg1,"fline"))
       return nl->SymbolAtom("real");

   }
   return nl->SymbolAtom("typeerror");
}


static ListExpr FORealRealReal(ListExpr args){
   if(nl->ListLength(args)==3){
       ListExpr arg1 = nl->First(args);
       ListExpr arg2 = nl->Second(args);
       ListExpr arg3 = nl->Third(args);
       if( nl->IsEqual(arg1,"fpoint") &&
           nl->IsEqual(arg2,"real") &&
           nl->IsEqual(arg3,"real"))
             return nl->SymbolAtom("real");
       if( nl->IsEqual(arg1,"fregion") &&
           nl->IsEqual(arg2,"real") &&
           nl->IsEqual(arg3,"real"))
             return nl->SymbolAtom("real");
       if( nl->IsEqual(arg1,"fline") &&
           nl->IsEqual(arg2,"real") &&
           nl->IsEqual(arg3,"real"))
             return nl->SymbolAtom("real");
   }
   return nl->SymbolAtom("typeerror");
}


static ListExpr FOiFOi(ListExpr args){
   if(nl->ListLength(args)==1){
      ListExpr arg1= nl->First(args);
      if(nl->IsEqual(arg1,"fpoint"))
         return nl->SymbolAtom("fpoint");
      if(nl->IsEqual(arg1,"fregion"))
         return nl->SymbolAtom("fregion");
      if(nl->IsEqual(arg1,"fline"))
         return nl->SymbolAtom("fline");

   }
   return nl->SymbolAtom("typeerror");
}

static ListExpr FRegionFRegion(ListExpr args){
   if(nl->ListLength(args)==1){
      ListExpr arg1= nl->First(args);
      if(nl->IsEqual(arg1,"fregion"))
         return nl->SymbolAtom("fregion");
   }
   return nl->SymbolAtom("typeerror");
}



static ListExpr FRegionReal(ListExpr args){
  if(nl->ListLength(args)==1){
    ListExpr arg = nl->First(args); 
    if(nl->IsEqual(arg,"fregion"))
       return nl->SymbolAtom("real");
  }
  return nl->SymbolAtom("typeerror");   
}

static ListExpr FRegionFLine(ListExpr args){
  if(nl->ListLength(args)==1){
     ListExpr arg = nl->First(args);
     if (nl->IsEqual(arg,"fregion"))
       return nl->SymbolAtom("fline");
  }
  return nl->SymbolAtom("typeerror");   
}

static ListExpr FRegionFRegionFLine(ListExpr args){
  if(nl->ListLength(args)==2){
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if( nl->IsEqual(arg1,"fregion")
        && nl->IsEqual(arg2,"fregion"))
          return nl->SymbolAtom("fline");
  }
  return nl->SymbolAtom("typeerror");   
}


/*
    Value mapping Functions
*/


/* Value mapping functions for fpoints */

static int Add_PP(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFPoint* P1 = (CcFPoint*) args[0].addr;
  CcFPoint* P2 = (CcFPoint*) args[1].addr;
  // get the result
  result.addr = P1->Add(P2);  
  return 0;
}

static int AlphaCut_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   float Alpha = ((CcReal*) args[1].addr)->GetRealval();
   bool Strong = ((CcBool*) args[2].addr)->GetBoolval();
   result.addr = P->AlphaCut(Alpha,Strong);
   return  0;
}

static int BasicCard_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   ((CcReal*)result.addr)->Set(true,(float)P->BasicCard());
   return 0;
}

static int BasicSimilar_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   ((CcReal*)result.addr)->Set(true,(float) P1->BasicSimilar(P2));
   return 0;
}

static int Cardinality_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   ((CcReal*)result.addr)->Set(true,(float)P->Cardinality());
   return 0;
}

static int Difference_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   result.addr= P1->Difference(P2);
   return 0; 
}

static int ScaleFactor_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   ((CcReal*)result.addr)->Set(true,(float) P->ScaleFactor());
   return 0;
}

static int Intersection_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   result.addr= P1->Intersection(P2);
   return 0; 
}

static int IsEmpty_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   ((CcBool*)result.addr)->Set(true,P->IsEmpty());
   return 0;
}

static int MaxValue_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   ((CcReal*)result.addr)->Set(true,(float)P->MaxValue());
   return 0;
}


static int MinValue_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   ((CcReal*)result.addr)->Set(true,(float)P->MinValue());
   return 0;
}


static int MaxValueAt_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   double x = ((CcReal*)args[1].addr)->GetRealval();
   double y = ((CcReal*)args[2].addr)->GetRealval();
   ((CcReal*)result.addr)->Set(true,(float)P->MaxValueAt(x,y));
   return 0;
}

static int MidValueAt_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   double x = ((CcReal*)args[1].addr)->GetRealval();
   double y = ((CcReal*)args[2].addr)->GetRealval();
   ((CcReal*)result.addr)->Set(true,(float)P->MidValueAt(x,y));
   return 0;
}

static int MinValueAt_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P = (CcFPoint*) args[0].addr;
   double x = ((CcReal*)args[1].addr)->GetRealval();
   double y = ((CcReal*)args[2].addr)->GetRealval();
   ((CcReal*)result.addr)->Set(true,(float)P->MinValueAt(x,y));
   return 0;
}

static int ScaledAdd_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   result.addr= P1->ScaledAdd(P2);
   return 0; 
}

static int ScaledDifference_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   result.addr= P1->ScaledDifference(P2);
   return 0; 
}

static int ScaledIntersection_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   result.addr= P1->ScaledIntersection(P2);
   return 0; 
}

static int ScaledUnion_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   result.addr= P1->ScaledUnion(P2);
   return 0; 
}

static int Sharp_P(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   result.addr= P1->Sharp();
   return 0; 
}


static int Union_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   result.addr= P1->Union(P2);
   return 0; 
}

static int Similar_PP(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFPoint* P1 = (CcFPoint*) args[0].addr;
   CcFPoint* P2 = (CcFPoint*) args[1].addr;
   ((CcReal*)result.addr)->Set(true,P1->Similar(P2));   
   return 0; 
}


/* value mappings for Flines */

static int Add_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*) args[1].addr;
  result.addr = L1->Add(L2);
  return 0;
}

static int AlphaCut_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  double alpha = ((CcReal*)args[1].addr)->GetRealval();
  bool strong = ((CcBool*)args[2].addr)->GetBoolval();
  result.addr = L1->AlphaCut(alpha,strong);
  return 0; 
}

static int BasicLength_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  ((CcReal*)result.addr)->Set(true,L1->BasicLength());
  return 0;
}

static int BasicSimilar_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*) args[1].addr;
  ((CcReal*)result.addr)->Set(true,L1->BasicSimilar(L2));
  return 0;
}

static int Boundary_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  result.addr = L1->Boundary();
  return  0;
} 

static int CommonPoints_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->CommonPoints(L2);
  return  0;
} 


static int Difference_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->Difference(L2);
  return  0;
} 

static int ScaleFactor_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  ((CcReal*)result.addr)->Set(true,L1->ScaleFactor());
  return 0;
}

static int Intersection_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->Intersection(L2);
  return  0;
} 

static int IsEmpty_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  ((CcBool*)result.addr)->Set(true,L1->IsEmpty());
  return 0;
}

static int Length3D_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  ((CcReal*)result.addr)->Set(true,L1->Length3D());
  return 0;
}

static int Length_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  ((CcReal*)result.addr)->Set(true,L1->Length());
  return 0;
}

static int MaxValue_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  ((CcReal*)result.addr)->Set(true,L1->MaxValue());
  return 0;
}

static int MinValue_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  ((CcReal*)result.addr)->Set(true,L1->MinValue());
  return 0;
}

static int MaxValueAt_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  double x = ((CcReal*)args[1].addr)->GetRealval();
  double y = ((CcReal*)args[2].addr)->GetRealval();
  ((CcReal*)result.addr)->Set(true,L1->MaxValueAt(x,y));
  return 0;
}

static int MidValueAt_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  double x = ((CcReal*)args[1].addr)->GetRealval();
  double y = ((CcReal*)args[2].addr)->GetRealval();
  ((CcReal*)result.addr)->Set(true,L1->MidValueAt(x,y));
  return 0;
}

static int MinValueAt_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  double x = ((CcReal*)args[1].addr)->GetRealval();
  double y = ((CcReal*)args[2].addr)->GetRealval();
  ((CcReal*)result.addr)->Set(true,L1->MinValueAt(x,y));
  return 0;
}


static int ScaledAdd_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->ScaledAdd(L2);
  return  0;
} 

static int ScaledDifference_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->ScaledDifference(L2);
  return  0;
} 

static int ScaledIntersection_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->ScaledIntersection(L2);
  return  0;
} 

static int ScaledUnion_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->ScaledUnion(L2);
  return  0;
} 

static int Union_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  result.addr = L1->Union(L2);
  return  0;
} 

static int Similar_LL(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  CcFLine* L2 = (CcFLine*)args[1].addr;
  ((CcReal*)result.addr)->Set(true,L1->Similar(L2));
  return  0;
} 
 
static int Sharp_L(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFLine* L1 = (CcFLine*) args[0].addr;
  result.addr = L1->Sharp();
  return 0;
}



/* the value mapping functions for regions */



static int Add_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  // initialize result
  result = qp->ResultStorage(s);
  // get arguments
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  // get the result
  result.addr = R1->Add(R2);  
  return 0;
}

static int AlphaCut_R (Word* args, Word& result, int message, Word& local, Supplier s){
   // initialize result
   result = qp->ResultStorage(s);
   // get arguments
   CcFRegion* R = (CcFRegion *) args[0].addr;
   double alpha = ((CcReal*) args[1].addr)->GetRealval();
   bool strong = ((CcBool*) args[2].addr)->GetBoolval();
   result.addr = R->AlphaCut(alpha,strong);
   return 0;
}

static int Area_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double res = R->Area();
  ((CcReal*)result.addr)->Set(true,(float)res);
  return 0;
}

static int Area3D_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double res = R->Area3D();
  ((CcReal*)result.addr)->Set(true,(float)res);
  return 0;
}

static int BasicArea_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double res = R->BasicArea();
  ((CcReal*)result.addr)->Set(true,(float)res);
  return 0;
}

static int BasicSimilar_RR (Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFRegion* R1 = (CcFRegion*) args[0].addr;
   CcFRegion* R2 = (CcFRegion*) args[1].addr;
   double res = R1->BasicSimilar(R2);
   ((CcReal*)result.addr)->Set(true,(float)res);
   return 0;
}

static int Boundary_R (Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFRegion* R = (CcFRegion*) args[0].addr;
   CcFLine* L = R->Boundary();
   result.addr=L;
   return 0;
}

static int Contour_R (Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFRegion* R = (CcFRegion*) args[0].addr;
   CcFLine* L = R->Contour();
   result.addr=L;
   return 0;
}

static int CommonLines_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFLine* L = R1->CommonLines(R2); 
  result.addr=L;
  return 0;
}

static int CommonPoints_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFPoint* L = R1->CommonPoints(R2); 
  result.addr=L;
  return 0;
}

static int Difference_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFRegion* R3 = R1->Difference(R2);
  result.addr=R3;
  return 0; 
}

/*
static int Equals_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  bool res = R1->Equals(R2);
  ((CcBool*)result.addr)->Set(true,res);
  return 0;
}
*/

static int ScaleFactor_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double res = R->GetScaleFactor();
  ((CcReal*)result.addr)->Set(true,res);
  return 0;
}

static int Holes_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  CcFRegion* res = R->Holes();
  result.addr=res;
  return 0;
}

static int Intersection_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFRegion* R3 = R1->Intersection(R2);
  result.addr=R3;
  return 0;
}

static int IsEmpty_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  bool res = R->IsEmpty();
  ((CcBool*)result.addr)->Set(true,res);
  return 0;
}


static int MaxValue_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double Z = R->MaxZ();
  ((CcReal*)result.addr)->Set(true,(float)Z);
  return 0;
}

static int MinValue_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double Z = R->MinZ();
  ((CcReal*)result.addr)->Set(true,(float)Z);
  return 0;
}


static int MaxValueAt_R(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double x = ((CcReal*)args[1].addr)->GetRealval();
  double y = ((CcReal*)args[2].addr)->GetRealval();
  double Z = R->MaxZfkt(x,y);
  ((CcReal*)result.addr)->Set(true,Z);
  return 0;
}

static int MidValueAt_R(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double x = ((CcReal*)args[1].addr)->GetRealval();
  double y = ((CcReal*)args[2].addr)->GetRealval();
  double Z = R->MidZfkt(x,y);
  ((CcReal*)result.addr)->Set(true,Z);
  return 0;
}

static int MinValueAt_R(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R = (CcFRegion*) args[0].addr;
  double x = ((CcReal*)args[1].addr)->GetRealval();
  double y = ((CcReal*)args[2].addr)->GetRealval();
  double Z = R->MinZfkt(x,y);
  ((CcReal*)result.addr)->Set(true,Z);
  return 0;
}

static int ScaledAdd_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFRegion* R3 = R1->ScaledAdd(R2);
  result.addr=R3;
  return 0;
}

static int ScaledDifference_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFRegion* R3 = R1->ScaledDifference(R2);
  result.addr=R3;
  return 0;
}

static int ScaledIntersection_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFRegion* R3 = R1->ScaledIntersection(R2);
  result.addr=R3;
  return 0;
}

static int ScaledUnion_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFRegion* R3 = R1->ScaledUnion(R2);
  result.addr=R3;
  return 0;
}

static int Union_RR (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R2 = (CcFRegion*) args[1].addr;
  CcFRegion* R3 = R1->Union(R2);
  result.addr=R3;
  return 0;
}

static int Sharp_R (Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcFRegion* R1 = (CcFRegion*) args[0].addr;
  CcFRegion* R3 = R1->Sharp();
  result.addr=R3;
  return 0;
}

static int Similar_RR (Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcFRegion* R1 = (CcFRegion*) args[0].addr;
   CcFRegion* R2 = (CcFRegion*) args[1].addr;
   double res = R1->Similar(R2);
   ((CcReal*)result.addr)->Set(true,res);
   return 0;
}


/* the selection functions */

/* the selection function for non overloading operators 
   like area length cardinality
*/
static int simpleSelect (ListExpr args ){
  return 0; 
}

/* The selection function for overloading operators.
   All operators are unifique determined by the 
   first argument of this operator. So we contain 
   a simple selection function
 */
static int fuzzySelect(ListExpr args){
  if(nl->ListLength(args)<1) return -1;  // should never occurs
  ListExpr A = nl->First(args);
  if(nl->IsEqual(A,"fpoint")) return 0;
  if(nl->IsEqual(A,"fregion")) return 1;
  if(nl->IsEqual(A,"fline")) return 2;
  return -1;
}
   

/* The selection function for operators which can only applied
   to flines und fregions (i.e. CommonPoints and Boundary).
   The first argument determines the function number 
*/
static int RLSelect(ListExpr args){
  if(nl->ListLength(args)<1) return -1;
  ListExpr First = nl->First(args);
  if(nl->IsEqual(First,"fregion")) return 0;
  if(nl->IsEqual(First,"fline")) return 1;
  return -1; // this point should never be reached
}
   

/* Define ValueMappings for overloaded Operators */

  ValueMapping AddMap[] = {Add_PP,Add_RR,Add_LL};  
  ValueMapping AlphaCutMap[] = {AlphaCut_P,AlphaCut_R,AlphaCut_L};
  ValueMapping BasicSimilarMap[] = {BasicSimilar_PP,BasicSimilar_RR,BasicSimilar_LL};
  ValueMapping DifferenceMap[] = {Difference_PP,Difference_RR,Difference_LL};
  ValueMapping ScaleFactorMap[] = {ScaleFactor_P,ScaleFactor_R,ScaleFactor_L};
  ValueMapping IntersectionMap[] = {Intersection_PP,Intersection_RR,Intersection_LL};
  ValueMapping IsEmptyMap[] = {IsEmpty_P,IsEmpty_R,IsEmpty_L};
  ValueMapping MaxValueMap[] = {MaxValue_P,MaxValue_R,MaxValue_L};
  ValueMapping MinValueMap[] = {MinValue_P,MinValue_R,MinValue_L};
  ValueMapping MaxValueAtMap[] ={MaxValueAt_P,MaxValueAt_R,MaxValueAt_L};
  ValueMapping MidValueAtMap[] ={MidValueAt_P,MidValueAt_R,MidValueAt_L};
  ValueMapping MinValueAtMap[] ={MinValueAt_P,MinValueAt_R,MinValueAt_L};
  ValueMapping ScaledAddMap[] ={ScaledAdd_PP,ScaledAdd_RR,ScaledAdd_LL};
  ValueMapping ScaledDifferenceMap[] ={ScaledDifference_PP,ScaledDifference_RR,ScaledDifference_LL};
  ValueMapping ScaledIntersectionMap[] ={ScaledIntersection_PP,ScaledIntersection_RR,ScaledIntersection_LL};
  ValueMapping ScaledUnionMap[] ={ScaledUnion_PP,ScaledUnion_RR,ScaledUnion_LL};
  ValueMapping SharpMap[] ={Sharp_P,Sharp_R,Sharp_L};
  ValueMapping SimilarMap[] ={Similar_PP,Similar_RR,Similar_LL};
  ValueMapping UnionMap[] ={Union_PP,Union_RR,Union_LL};
  ValueMapping BoundaryMap[] ={Boundary_R,Boundary_L};
  ValueMapping CommonPointsMap[] = {CommonPoints_RR,CommonPoints_LL};



/* specification of operators */
const string add_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fuzzyobject_i fuzzyobject_i) -> fuzzyobject_i</text--->"
			     "<text>add ( o1 ,o2 ) where"
			     " o1, o2   are of type fline,fpoint or fregion (the same type)"
			     "</text--->"
			     "<text>To add two fuzzy objects</text--->"
			     "<text>add(r1,r2)</text--->"
			      ") )";

const string basiccard_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fpoint) -> real</text--->"
			     "<text>basiccard(p) where"
			     " p is of type fpoint"
			     "</text--->"
			     "<text>get the number of containing coordinates</text--->"
			     "<text>basiccard(p1))</text--->"
			      ") )";

const string cardinality_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fpoint) -> real</text--->"
			     "<text>cardinality(p) where"
			     " p is of type fpoint"
			     "</text--->"
			     "<text>get the weighted sum  of containing coordinates</text--->"
			     "<text>cardinality(p1))</text--->"
			      ") )";

const string length3d_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fline) -> real</text--->"
			     "<text>length3d(l) where"
			     " l is of type fline"
			     "</text--->"
			     "<text>get the fline describing 3d structure</text--->"
			     "<text>length3d(l1))</text--->"
			      ") )";

const string length_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fline) -> real</text--->"
			     "<text>length(l) where"
			     " l is of type fline"
			     "</text--->"
			     "<text>get the weighted length of a fline</text--->"
			     "<text>length(l1))</text--->"
			      ") )";

const string basiclength_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fline) -> real</text--->"
			     "<text>basiclength(l) where"
			     " l is of type fline"
			     "</text--->"
			     "<text>get the length of a fline without membershipvalues</text--->"
			     "<text>basiclength(l1))</text--->"
			      ") )";

const string alphacut_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fobject real bool) -> fregion</text--->"
			     "<text>alphacut(fo,alpha,strong )  where fo is a fuzzy object,\
                        alpha is a real and strong is a bool"
			     "</text--->"
			     "<text>returns the alpha cut of a fobject</text--->"
			     "<text>alphacut(o1,0.8,TRUE)</text--->"
			      ") )";


const string boundary_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fline) -> fpoint , (fregion) -> fline </text--->"
			     "<text>boundary(fo) where fo is of type fline or fregion"
			     "</text--->"
			     "<text>returns the boundary/endpoints of a fregion/fline</text--->"
			     "<text>boundary(l1)</text--->"
			      ") )";


const string commonpoints_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fline,fline) -> fpoint , (fregion,fregion) -> fpoint </text--->"
			     "<text>commonpoints(fo) where fo is of type fline or fregion"
			     "</text--->"
			     "<text> get all common points which are not part of a common segment </text--->"
			     "<text>commonpoints(r1,r2)</text--->"
			      ") )";

const string area_spec= "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion) -> real</text--->"
			     "<text>area(r) where r is of type fregion"
			     "</text--->"
			     "<text> get the weighted area of a fuzzy region </text--->"
			     "<text>area(r1)</text--->"
			      ") )";



const string area3d_spec= "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion) -> real</text--->"
			     "<text>area3d(r) where r is of type fregion"
			     "</text--->"
			     "<text> get the area of the 3d structure \
                               defined by a fregion </text--->"
			     "<text>area3d(r1)</text--->"
			      ") )";


const string basicarea_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion) -> real</text--->"
			     "<text>basicarea(r) where r is of type fregion"
			     "</text--->"
			     "<text> get the area of a fregion excluding membershipvalues </text--->"
			     "<text>basicarea(r1)</text--->"
			      ") )";

const string basicsimilar_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fobject_i,fobject_i) -> real</text--->"
			     "<text>basicsimilar(o1,o2) where o1,o2 are of same fuzzy type "
                       " in{fpoint,fline,fregion} "
			     "</text--->"
			     "<text> get a real value in [0,1] describing the similarity of 2 fuzzy objects </text--->"
			     "<text>basicsimilar(r1,r2)</text--->"
			      ") )";

const string contour_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion) -> fline</text--->"
			     "<text>contour(r) where r is of type fregion "
			     "</text--->"
			     "<text> returns the contour of a fregion this means "
                       " the boundary without boundary of holes from r </text--->"
			     "<text>contour(r1)</text--->"
			      ") )";


const string commonlines_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion,fregion) -> fline</text--->"
			     "<text>commonlines(r1,r2) where o1,o2 are of type fregion "
			     "</text--->"
			     "<text> returns all common segments of r1,r2 which are not a part"
                       " of a common triangle  </text--->"
			     "<text>commonlines(r1,r2)</text--->"
			      ") )";

const string difference_spec= "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion,fregion) -> fregion, (fline,fline) -> fline "
                             " (fpoint,fpoint)-> fpoint</text--->"
			     "<text>difference(o1,o2)"
			     "</text--->"
			     "<text> returns the difference of 2 fuzzy objects excluding the scalefactor"
                       "</text--->"
			     "<text>difference(r1,r2)</text--->"
			      ") )";


const string getscalefactor_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text> fobject -> real</text--->"
			     "<text>scalefactor(o1)"
			     "</text--->"
			     "<text> returns the scalefactor of a fuzzy object"
                       "</text--->"
			     "<text>scalefactor(r1)</text--->"
			      ") )";


const string holes_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion) -> fregion </text--->"
			     "<text>holes(r)"
			     "</text--->"
			     "<text> returns the holes of a fregion described by a fregion"
                       "</text--->"
			     "<text>holes(r1)</text--->"
			      ") )";


const string intersection_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fregion,fregion) -> fregion, (fline,fline) -> fline "
                             " (fpoint,fpoint)-> fpoint</text--->"
			     "<text>intersection(o1,o2)"
			     "</text--->"
			     "<text> returns the intersection of 2 fuzzy objects excluding the scalefactor"
                       "</text--->"
			     "<text>intersection(r1,r2)</text--->"
			      ") )";


const string isempty_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text> fobject -> bool</text--->"
			     "<text>isempty(o1)"
			     "</text--->"
			     "<text> returns true if o1 has no components"
                       "</text--->"
			     "<text>isempty(r1)</text--->"
			      ") )";



const string maxz_spec= "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text> fobject -> bool</text--->"
			     "<text>maxvalue(o1)"
			     "</text--->"
			     "<text> returns the maximum membership value containing in the basic of o1"
                       "</text--->"
			     "<text>maxvalue(r1)</text--->"
			      ") )";

const string minz_spec=  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text> fobject -> bool</text--->"
			     "<text>minvalue(o1)"
			     "</text--->"
			     "<text> returns the minimum membership value containing in the basic of o1"
                       "</text--->"
			     "<text>minvalue(r1)</text--->"
			      ") )";

const string maxzfkt_spec= "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text> (fobject,real,real) -> bool</text--->"
			     "<text>max_value_at(fo,x,y)"
			     "</text--->"
			     "<text> returns the maximum membership value at (x,y)"
                       "</text--->"
			     "<text>max_value_at(r1,898.7,87.0)</text--->"
			      ") )";


const string midzfkt_spec=  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text> (fobject,real,real) -> bool</text--->"
			     "<text>mid_value_at(fo,x,y)"
			     "</text--->"
			     "<text> returns the average membership value at (x,y)"
                       "</text--->"
			     "<text>mid_value_at(r1,898.7,87.0)</text--->"
			      ") )";


const string minzfkt_spec= "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text> (fobject,real,real) -> bool</text--->"
			     "<text>min_value_at(fo,x,y)"
			     "</text--->"
			     "<text> returns the minimum membership value at (x,y)"
                       "</text--->"
			     "<text>min_value_at(r1,898.7,87.0)</text--->"
			      ") )";




const string scaledadd_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fuzzyobject_i fuzzyobject_i) -> fuzzyobject_i</text--->"
			     "<text>scaled_add ( o1 ,o2 ) where"
			     " o1, o2   are of type fline,fpoint or fregion (the same type)"
			     "</text--->"
			     "<text>To add two fuzzy objects including scaledfactor</text--->"
			     "<text>scaled_add(r1,r2)</text--->"
			      ") )";

const string scaleddifference_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fuzzyobject_i fuzzyobject_i) -> fuzzyobject_i</text--->"
			     "<text>scaled_difference( o1 ,o2 ) where"
			     " o1, o2   are of type fline,fpoint or fregion (the same type)"
			     "</text--->"
			     "<text>returns the difference of two fuzzy objects including scalefactor</text--->"
			     "<text>scaled_difference(r1,r2)</text--->"
			      ") )";


const string scaledintersection_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fuzzyobject_i fuzzyobject_i) -> fuzzyobject_i</text--->"
			     "<text>scaled_intersection( o1 ,o2 ) where"
			     " o1, o2   are of type fline,fpoint or fregion (the same type)"
			     "</text--->"
			     "<text>returns the intersection of two fuzzy objects including scalefactor</text--->"
			     "<text>scaled_intersection(r1,r2)</text--->"
			      ") )";


const string scaledunion_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fuzzyobject_i fuzzyobject_i) -> fuzzyobject_i</text--->"
			     "<text>scaled_union( o1 ,o2 ) where"
			     " o1, o2   are of type fline,fpoint or fregion (the same type)"
			     "</text--->"
			     "<text>returns the union of two fuzzy objects including scalefactor</text--->"
			     "<text>scaled_uion(r1,r2)</text--->"
			      ") )";

const string union_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fuzzyobject_i fuzzyobject_i) -> fuzzyobject_i</text--->"
			     "<text>union( o1 ,o2 ) where"
			     " o1, o2   are of type fline,fpoint or fregion (the same type)"
			     "</text--->"
			     "<text>returns the union of two fuzzy objects excluding scalefactor</text--->"
			     "<text>union(r1,r2)</text--->"
			      ") )";

const string sharp_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fuzzyobject_i) -> fuzzyobject_i</text--->"
			     "<text>sharp( o1) where"
			     " o1  is of type fline,fpoint or fregion"
			     "</text--->"
			     "<text> returns a sharp object, this means a fuzzy object with all"
                       " membership values setted to 1.0</text--->"
			     "<text>sharp(r1,r2)</text--->"
			      ") )";

const string similar_spec="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(fobject_i,fobject_i) -> real</text--->"
			     "<text>similar(o1,o2) where o1,o2 are of same fuzzy type "
                       " in{fpoint,fline,fregion} "
			     "</text--->"
			     "<text> get a real value in [0,1] describing the similarity of 2 fuzzy objects </text--->"
			     "<text>similar(r1,r2)</text--->"
			      ") )";


/*
The dummy model mapping:

*/

static Word
CcNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

ModelMapping Model_2[] = {CcNoModelMapping,CcNoModelMapping};
ModelMapping Model_3[] = {CcNoModelMapping,CcNoModelMapping,CcNoModelMapping};


/* definition of operators */

Operator op_add
(
 "add", 			//name
 add_spec,  		//specification ....
 3,                     // number of functions
 AddMap,			//value mapping
 Model_3,	      	//dummy model mapping
 fuzzySelect,		//selection function 
 FOiFOiFOi		//type mapping 
);


Operator op_alphacut
(
 "alphacut", 	//name
 alphacut_spec,	//specification ....
 3,
 AlphaCutMap,	//value mapping
 Model_3,	  	//dummy model mapping, defined in Algebra.h
 fuzzySelect,	//trivial selection function 
 FOiRealBoolFOi	//type mapping 
);


Operator op_basiccard
(
 "basiccard", 			//name
 basiccard_spec,  		//specification ....
 BasicCard_P,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FPointReal		//type mapping 
);

Operator op_cardinality
(
 "cardinality", 			//name
 cardinality_spec,  		//specification ....
 Cardinality_P,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FPointReal		//type mapping 
);

Operator op_length3d
(
 "length3d", 			//name
 length3d_spec,  		//specification ....
 Length3D_L,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FLineReal		//type mapping 
);

Operator op_length
(
 "length", 			//name
 length_spec,  		//specification ....
 Length_L,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FLineReal		//type mapping 
);

Operator op_basiclength
(
 "basiclength", 			//name
 basiclength_spec,  		//specification ....
 BasicLength_L,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FLineReal		//type mapping 
);

Operator op_area
(
 "area", 			//name
 area_spec,  		//specification ....
 Area_R,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FRegionReal		//type mapping 
);

Operator op_area3d
(
 "area3d", 			//name
 area3d_spec,  		//specification ....
 Area3D_R,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FRegionReal		//type mapping 
);

Operator op_basicarea
(
 "basicarea", 			//name
 basicarea_spec,  		//specification ....
 BasicArea_R,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FRegionReal		//type mapping 
);

Operator op_basicsimilar
(
 "basicsimilar", 			//name
 basicsimilar_spec,  		//specification ....
 3,
 BasicSimilarMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiReal		//type mapping 
);

Operator op_boundary
(
 "boundary", 			//name
 boundary_spec,  		//specification ....
 2,
 BoundaryMap,		//value mapping
 Model_2,	       	//dummy model mapping, defined in Algebra.h
 RLSelect,			//trivial selection function 
 BoundaryTypeMap		//type mapping 
);


Operator op_contour
(
 "contour", 			//name
 contour_spec,  		//specification ....
 Contour_R,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FRegionFLine		//type mapping 
);

Operator op_commonlines
(
 "commonlines", 			//name
 commonlines_spec,  		//specification ....
 CommonLines_RR,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FRegionFRegionFLine		//type mapping 
);

Operator op_commonpoints
(
 "commonpoints", 			//name
 commonpoints_spec,  		//specification ....
 2,
 CommonPointsMap,			//value mapping
 Model_2,		//dummy model mapping, defined in Algebra.h
 RLSelect,			//trivial selection function 
 CommonPointsTypeMap		//type mapping 
);

Operator op_difference
(
 "difference", 			//name
 difference_spec,  		//specification ....
 3,
 DifferenceMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiFOi		//type mapping 
);


Operator op_scalefactor
(
 "scalefactor", 			//name
 getscalefactor_spec,  		//specification ....
 3,
 ScaleFactorMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOReal		//type mapping 
);

Operator op_holes
(
 "holes", 			//name
 holes_spec,  		//specification ....
 Holes_R,			//value mapping
 Operator::DummyModel,		//dummy model mapping, defined in Algebra.h
 simpleSelect,			//trivial selection function 
 FRegionFRegion		//type mapping 
);

Operator op_intersection
(
 "fuzzy_intersection", 			//name
 intersection_spec,  		//specification ....
  3,
 IntersectionMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiFOi		//type mapping 
);

Operator op_isempty
(
 "isempty", 			//name
 isempty_spec,  		//specification ....
 3,
 IsEmptyMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOBool		//type mapping 
);

Operator op_maxv
(
 "maxvalue", 			//name
 maxz_spec,  		//specification ....
 3,
 MaxValueMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOReal		//type mapping 
);


Operator op_minv
(
 "minvalue", 			//name
 minz_spec,  		//specification ....
 3,
 MinValueMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOReal		//type mapping 
);


Operator op_max_value_at
(
 "max_value_at", 			//name
 maxzfkt_spec,  		//specification ....
 3,
 MaxValueAtMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function
 FORealRealReal		//type mapping 
);

Operator op_mid_value_at
(
 "mid_value_at", 			//name
 midzfkt_spec,  		//specification ....
 3,
 MidValueAtMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FORealRealReal		//type mapping 
);

Operator op_min_value_at
(
 "min_value_at", 			//name
 minzfkt_spec,  		//specification ....
 3,
 MinValueAtMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FORealRealReal		//type mapping 
);


Operator op_scaled_add
(
 "scaled_add", 			//name
 scaledadd_spec,  		//specification ....
 3,
 ScaledAddMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function
 FOiFOiFOi		//type mapping 
);

Operator op_scaled_difference
(
 "scaled_difference", 			//name
 scaleddifference_spec,  		//specification ....
 3,
 ScaledDifferenceMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiFOi		//type mapping 
);

Operator op_scaled_intersection
(
 "scaled_intersection", 			//name
 scaledintersection_spec,  		//specification ....
 3,
 ScaledIntersectionMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiFOi		//type mapping 
);

Operator op_scaled_union
(
 "scaled_union", 			//name
 scaledunion_spec,  		//specification ....
 3,
 ScaledUnionMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiFOi		//type mapping 
);

Operator op_union
(
 "fuzzy_union", 			//name
 union_spec,  		//specification ....
 3,
 UnionMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiFOi		//type mapping 
);


Operator op_sharp
(
 "sharp", 			//name
 sharp_spec,  		//specification ....
 3,
 SharpMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOi		//type mapping 
);

Operator op_similar
(
 "similar", 			//name
 similar_spec,  		//specification ....
 3,
 SimilarMap,			//value mapping
 Model_3,		//dummy model mapping, defined in Algebra.h
 fuzzySelect,			//trivial selection function 
 FOiFOiReal		//type mapping 
);


/*
 Creating the Algebra
*/

class FuzzyAlgebra : public Algebra
{
 public:
  FuzzyAlgebra() : Algebra()
  {
    AddTypeConstructor( &ccfpoint );

    ccfpoint.AssociateKind("DATA");  
 
    AddTypeConstructor(&ccfregion);
    ccfregion.AssociateKind("DATA");

    AddTypeConstructor(&ccfline);
    ccfline.AssociateKind("DATA");

    AddOperator(&op_add);
    AddOperator(&op_alphacut);
    AddOperator(&op_area);
    AddOperator(&op_area3d);
    AddOperator(&op_basicarea);
    AddOperator(&op_basicsimilar);
    AddOperator(&op_boundary);
    AddOperator(&op_contour);
    AddOperator(&op_commonlines);
    AddOperator(&op_commonpoints);
    AddOperator(&op_difference);
    AddOperator(&op_scalefactor);
    AddOperator(&op_holes);
    AddOperator(&op_intersection);
    AddOperator(&op_isempty);
    AddOperator(&op_maxv);
    AddOperator(&op_minv);
    AddOperator(&op_max_value_at);
    AddOperator(&op_mid_value_at);
    AddOperator(&op_min_value_at);
    AddOperator(&op_scaled_add);
    AddOperator(&op_scaled_difference);
    AddOperator(&op_scaled_intersection);
    AddOperator(&op_scaled_union);
    AddOperator(&op_union);
    AddOperator(&op_sharp);
    AddOperator(&op_similar);
    AddOperator(&op_basiccard);
    AddOperator(&op_cardinality);
    AddOperator(&op_length);
    AddOperator(&op_length3d);
    AddOperator(&op_basiclength);
  }
  ~FuzzyAlgebra() {};
};

FuzzyAlgebra fuzzyAlgebra; 


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
InitializeFuzzyAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  jvminit = new JVMInitializer();
  env = jvminit->getEnv();
  jvm = jvminit->getJVM();
  PointCls = env->FindClass("fuzzyobjects/composite/FPoint");
  SimplePointCls = env->FindClass("fuzzyobjects/simple/fEPoint");
  if(PointCls==0) error(__LINE__);
  if(SimplePointCls==0) error(__LINE__);


  nl = nlRef;
  qp = qpRef;
  return (&fuzzyAlgebra);
}






