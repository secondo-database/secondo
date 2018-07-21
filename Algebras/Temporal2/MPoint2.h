/*
MPoint2.h
Created on: 03.05.2018
    Author: simon

*/

#ifndef ALGEBRAS_TEMPORAL2_MPOINT2_H_
#define ALGEBRAS_TEMPORAL2_MPOINT2_H_

#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "Types.h"
#include "Algebras/Temporal/TemporalAlgebra.h" // UPoint
 // for relation related pointers:
#include "Algebras/Relation-C++/RelationAlgebra.h"
class TypeConstructor; // fwd dcl instead of #include TypeConstructor.h


namespace temporal2algebra {
TypeConstructor* getMPoint2TypePtr();

using namespace std;

class MPoint2: public Attribute {

public:

    //TODO: refactor: separate "MPointInterface" methods from
    // Memory related methods -> logging etc. only in memMethods
    // should make handling correctly more easy
    void memClear(); // used in genttc::Delete()
    const MemStorageId getMemId() const; // used in cout and pushToMem

    void memAppend(const temporalalgebra::UPoint& upoint);
    void SetBackReference(const BackReference& backReference);

    // check if it its safe to call memAppend()
    // relevant for OpAppendPositions to optimize inserts
    // if there already exists a memory representation, then just append data
    // without copying the whole MPoint2 Data
    bool hasMemoryUnits() const;
    static bool hasMemoryUnits(const BackReference& backRef);
    static MemStorageId findMemStorageId(const BackReference& backRef);
    static Unit getFinalUnit(const MemStorageId& id);
    static void appendUnit(const MemStorageId& id, const Unit& unit);

    MPoint2();

    MPoint2( const MPoint2& rhs );
    //only used during Algebra init:
    explicit MPoint2(const bool defined);
    explicit MPoint2(const int dummy);
    virtual ~MPoint2();

    MPoint2& operator=(const MPoint2& rhs);



   ListExpr ToListExpr(const ListExpr& typeInfo) const;

   bool ReadFrom(const ListExpr LE, const ListExpr& typeInfo);

   std::string toString() const;

   int Compare(const Attribute* rhs1) const;

   bool Adjacent(const Attribute* rhs) const;

   size_t HashValue() const;

   void CopyFrom(const Attribute* attr);


   MPoint2* Clone() const;

   size_t Sizeof() const;
   static const std::string BasicType();
   static const bool checkType(const ListExpr type);

   static ListExpr Property();

   static bool CheckKind(ListExpr type, ListExpr& errorInfo);

   virtual void Initialize( SmiFileId fileId,
           TupleId tupleId,
           int attrno);
   virtual void Finalize();

   ostream& Print(ostream &os) const;

   inline virtual int NumOfFLOBs () const;
   inline virtual Flob* GetFLOB ( const int i );

   // Functions added to replace direct member access from TemporalAlgebra
   Rectangle<3> GetBBox() const;

// functions from temporalalgebra::Mapping and ::MPoint
// required in Operators copied from TemporalAlgebra

   void Get( const int i, Unit& upi ) const;
   int  GetNoComponents() const;
   bool IsEmpty() const;
   void Clear();
   void StartBulkLoad();
   void Add( const temporalalgebra::UPoint& unit );
   // MergeAdd(smth.);
   bool EndBulkLoad( const bool sort = true,
           const bool checkvalid = false );
   // Destroy();
   bool IsOrdered() const;
   void Resize(size_t n);

private:

   void RestoreBoundingBox(const bool force = false); //class invariant?

private:
   // members
   bool ordered; // better: duringBulkProcessing

   // bool canDestroy // used in Delete und In fo  orig. MPoint
   MemStorageId id;
   DbArray <temporalalgebra::UPoint> units2;
   Rectangle<3> bbox2;
};

//std::ostream &operator<<(std::ostream &os, MPoint2 const &l);



} /* namespace temporal2algebra */


// we need our own versions of Delete to correctly clean up the memory parts
namespace gentc {

    template<>
    void Delete<temporal2algebra::MPoint2>(const ListExpr typeInfo,Word &w);

    template<>
    void Close<temporal2algebra::MPoint2>(const ListExpr typeInfo, Word& w );
} /* namespace gentc */

#endif /* ALGEBRAS_TEMPORAL2_MPOINT2_H_ */
