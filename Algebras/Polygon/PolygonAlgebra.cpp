/*
1 Implementation Module PolygonAlgebra

October 30th, 2002 Mirco G[ue]nster

The sole purpose of this little algebra is to
demonstrate the use of FLOBs and DBArrays.


2 Includes and global declarations

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"

static NestedList* nl;
static QueryProcessor* qp;

#include "Tuple.h"
#include "Attribute.h"
#include "DBArray.h"

struct CcPoint { int x; int y; };

/*

3 Implementation of class CcPolygon

*/
class CcPolygon : public Attribute {
private:
  	friend void SecondoMain(int, char *[]); /* Simplify writing test programs. */
	// the number of points in the polygon.
	int numberOfPoints;
	// the polygon points are stored in two DBArrays.
  	DBArray X;
	DBArray Y;
	
public:
	
/* 

3.1 Constructor.

recfile must be a file in which the points of a polygon are stored if 
the polygon size exceeds a specific value. (threshold size for FLOBs)
The parameters inX and inY must be pointers to arrays of ints with capacity of inNumberPoints. 

*/
	CcPolygon(SmiRecordFile *recfile, int inNumberOfPoints, int *inX, int *inY) 
		: X(recfile, sizeof(int), inNumberOfPoints), Y(recfile, sizeof(int), inNumberOfPoints) {
		numberOfPoints = inNumberOfPoints;
		
		// Store the projections of the points into the DBArrays.
		for (int i = 0; i < numberOfPoints; i++) {
			X.Put(i, (char *)(&(inX[i])));
			Y.Put(i, (char *)(&(inY[i])));
		}
	}

/*

3.2 Destructor.

*/
    ~CcPolygon(){};
    
/*
3.3 NumOfFLOBs.

returns the number of FLOBs; here 2. (X and Y component)

*/
  	int NumOfFLOBs() {
		return 2; 
	};
	
/*
3.4 GetFLOB

returns the FLOB with number i if exists.

*/
  	FLOB *GetFLOB(int i) { 
      switch (i) {
		case 0: return &X;
		case 1: return &Y;
		default: return 0;
      }
    }
    
/*
3.5 Compare

not implemented.

*/
   	int Compare(Attribute*){ return 0; };
	
/*

3.6 Sizeof

returns the size of a CcPolygon.

*/
   	int Sizeof(){ 
		return sizeof(CcPolygon); 
	};
	
/*
3.7 Clone

not implemented.

*/
   	class Attribute *Clone() {
		return 0;
	};
	
/*
3.8 IsDefined

*/
   	bool IsDefined(){
		return true;
	};

/*
3.9 Print

*/
  	ostream &Print(ostream &os);

/*

Here should be declared whatever Polygon access operation
you might consider useful. In our context, we omit those definitions.
Instead, member variables will be modified directly by the
Main function.
  
*/
};

/*

3.10 cout-Operator for CcPoints.

*/
ostream &operator<<(ostream &os, CcPoint &p){ 
	return os << "(" << p.x << ", " << p.y << ")";
};

/*

3.11 Implementation of the Print method for CcPolygon.

*/
ostream &CcPolygon::Print(ostream &os) {
	int value;
	
	os << "{POLYGON: ";
	
	for (int i = 0; i < numberOfPoints; i++) {
		X.Get(i, (char *)&value);
		os << "(" << value << ", ";
		Y.Get(i, (char *)&value);
		os  << value << ")";
		if (i < numberOfPoints - 1) os << ", ";
	}		
	os << "}";
	return os;
}

/*

4 Help functions to build an algebra.

4.1 CastPolygon

This function is needed to build an algebra.

*/
void *CastPolygon(void *addr) {
	return (new (addr) CcPolygon(*((CcPolygon *)addr)));
}

/*

4.2 FunctionProperty 

This function is needed to build an algebra.

*/
static ListExpr FunctionProperty() {
	return (nl->TheEmptyList());
}


/*

4.3 DummyInModel

This function is needed to build an algebra.

*/
static Word DummyInModel(ListExpr typeExpr, ListExpr list, int objNo) {
	return (SetWord(Address(0)));
}

/*

4.4 DummyOutModel

This function is needed to build an algebra.

*/
static ListExpr DummyOutModel(ListExpr typeExpr, Word model) {
	return 0;
}

/*

4.5 DummyValueToModel

This function is needed to build an algebra.

*/
static Word DummyValueToModel(ListExpr typeExpr, Word value) {
	return (SetWord(Address(0)));
}

/*

4.5 DummyValueListToModel

This function is needed to build an algebra.

*/
static Word DummyValueListToModel(const ListExpr typeExpr, const ListExpr valueList, const int errorPos, ListExpr& errorInfo, bool& correct) {
	correct = true;
	errorInfo = 0;
	return (SetWord(Address(0)));
}

/*

4.6 NoSpace

This function is needed to build an algebra.

*/
static Word NoSpace(int size) {
	return (SetWord(Address(0)));
}

/*

4.7 DoNothing

This function is needed to build an algebra.

*/
static void DoNothing(Word& w) {
	w.addr = 0;
}

/*

4.8 InMap

This function is needed to build an algebra.

*/
static Word InMap(const ListExpr typeInfo, const ListExpr instance, 
				  const int errorPos, ListExpr& errorInfo, 
				  bool& correct) {
	/*  We don't do any checks here; any list expression will be accepted.
		Errors will be found when the function is used, i.e., sent 
		to the query processor.
	*/
  	return (SetWord( instance ));
}

/*

4.9 OutMap

This function is needed to build an algebra.

*/
static ListExpr OutMap(ListExpr typeInfo, Word value) {
  return (value.list);
}

/*

4.10 CheckMap

This function is needed to build an algebra.

*/
static bool CheckMap(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(nl->First(type), "map"));
}

/*

5 The type constructor for the PolygonAlgebra.

*/
TypeConstructor PolygonTC(
	// name
	"polygon",
	
	// Type property
	FunctionProperty,
	
	// OutObject
	OutMap,
	
	// InObject
	InMap,
	
	// ObjectCreation
	NoSpace,
	
	// ObjectDeletion
	DoNothing,

	// ObjectCast
	CastPolygon,

	// TypeCheckFunction
	CheckMap,

	// PersistFunction pvf
	0,

	// PersistFunction pmf
	0,

	// InModelFunction
	DummyInModel,

	// OutModelFunction
	DummyOutModel,

	// ValueToModelFunction
	DummyValueToModel,
	
	// ValueListToModelFunction
	DummyValueListToModel
);

/*

6 PolygonAlgebra

*/
class PolygonAlgebra : public Algebra {
public:
  PolygonAlgebra() : Algebra() {
      AddTypeConstructor(&PolygonTC);
   }
  ~PolygonAlgebra(){};
};

PolygonAlgebra polygonalgebra;

/* 

7 Initialization

*/

extern "C"
Algebra*
InitializePolygonAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
	nl = nlRef;
	qp = qpRef;
	return (&polygonalgebra);
}


