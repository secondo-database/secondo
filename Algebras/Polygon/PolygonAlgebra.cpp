#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"

static NestedList* nl;
static QueryProcessor* qp;

#include "Tuple.h"
#include "Attribute.h"
#include "FLOB.h"

struct CcPoint { int x; int y; };
struct CcRectangle { CcPoint ur; CcPoint ul; CcPoint ll; CcPoint lr; };

class CcPolygon : public Attribute {
  	friend void SecondoMain(int, char *[]); /* Simplify writing test programs. */
	friend void *CastPolygon(void *addr); // this method needs access to convexHull.
  	int numberOfPoints;
  	FLOB X;
	FLOB Y;
	
public:
	CcPolygon(SmiRecordFile *recfile) : X(recfile), Y(recfile) {};
	CcPolygon(SmiRecordFile *recfile, int inNumberOfPoints, char *inX, char *inY) 
		: X(recfile, inNumberOfPoints * sizeof(int)), Y(recfile, inNumberOfPoints * sizeof(int)) {
		numberOfPoints = inNumberOfPoints;
		X.Write(0, inNumberOfPoints * sizeof(int), inX);
		Y.Write(0, inNumberOfPoints * sizeof(int), inY);
	}

    ~CcPolygon(){};
  	int NumOfFLOBs() { return 2; };
  	FLOB *GetFLOB(int i) { 
      switch (i) {
		case 0: return &X;
		case 1: return &Y;
		default: return 0;
      }
    }
   	int Compare(Attribute*){ return 0; };
   	int Sizeof(){ 
		return sizeof(CcPolygon); 
	};
   	class Attribute * Clone() {return NULL;};
   	bool IsDefined(){return true;};

  	ostream &Print(ostream &os);

  /*
    .
    .
    Here should be declared whatever Polygon access operation
    you might consider useful. In our context, we omit those definitions.
    Instead, member variables will be modified directly by the
    Main function.
    .
    .
  */
};

ostream &operator<<(ostream &os, CcPoint &p){ 
	return os << "(" << p.x << ", " << p.y << ")";
};

ostream &operator<<(ostream &os, CcRectangle &r){ 
	return os << "(" << r.ur << ", " << r.ul << ", " << r.ll << ", " << r.lr << ")";
};

ostream &CcPolygon::Print(ostream &os) {
	os << "{POLYGON: ";
	
	int *xArray = new int[numberOfPoints];
	int *yArray = new int[numberOfPoints];
	
	X.Get(0, numberOfPoints * sizeof(int), (char *)xArray);
	Y.Get(0, numberOfPoints * sizeof(int), (char *)yArray);
	
	for (int i = 0; i < numberOfPoints; i++) {
		os << "(" << xArray[i] << ", " << yArray[i] << ")";
		if (i < numberOfPoints - 1) os << ", ";
	}		
	os << "}";
	return os;
}

void *CastPolygon(void *addr) {
	//return new (addr) Polygon(((Polygon *)addr)->convexHull.lobFile);
	return addr;
}

static ListExpr FunctionProperty() {
	return (nl->TheEmptyList());
}

static Word DummyInModel(ListExpr typeExpr, ListExpr list, int objNo) {
	return (SetWord(Address(0)));
}

static ListExpr DummyOutModel(ListExpr typeExpr, Word model) {
	return 0;
}

static Word DummyValueToModel(ListExpr typeExpr, Word value) {
	return (SetWord(Address(0)));
}

static Word DummyValueListToModel(const ListExpr typeExpr, const ListExpr valueList, const int errorPos, ListExpr& errorInfo, bool& correct) {
	correct = true;
	errorInfo = 0;
	return (SetWord(Address(0)));
}

static Word NoSpace(int size) {
	return (SetWord(Address(0)));
}

static void DoNothing(Word& w) {
	w.addr = 0;
}

static Word InMap(const ListExpr typeInfo, const ListExpr instance, const int errorPos, ListExpr& errorInfo, bool& correct) {
	/*  We don't do any checks here; any list expression will be accepted.
		Errors will be found when the function is used, i.e., sent to the
		query processor.
	*/

  	return (SetWord( instance ));
}

static ListExpr OutMap(ListExpr typeInfo, Word value) {
  return (value.list);
}

static bool CheckMap(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(nl->First(type), "map"));
}

TypeConstructor PolygonTC(
	"polygon",									// name
	FunctionProperty,							// TypeProperty
	OutMap,										// OutObject
	InMap, 										// InObject
	NoSpace, 									// ObjectCreation
	DoNothing,									// ObjectDeletion
	CastPolygon,							 	// ObjectCast
	CheckMap,									// TypeCheckFunction
	0,											// PersistFunction pvf
	0, 											// PersistFunction pmf
	DummyInModel,			 					// InModelFunction
	DummyOutModel,								// OutModelFunction
	DummyValueToModel,							// ValueToModelFunction
	DummyValueListToModel						// ValueListToModelFunction
);


class PolygonAlgebra : public Algebra {
public:
  PolygonAlgebra() : Algebra() {
      AddTypeConstructor(&PolygonTC);
   }
  ~PolygonAlgebra(){};
};

PolygonAlgebra polygonalgebra;

/* Initialization */

extern "C"
Algebra*
InitializePolygonAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
	nl = nlRef;
	qp = qpRef;
	return (&polygonalgebra);
}


