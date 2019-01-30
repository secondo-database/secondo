/*
moving point implementation with optional in-memory representation
with similar Interface as temporalalgebra::MPoint

*/

#ifndef ALGEBRAS_TEMPORAL2_MPOINT2_H_
#define ALGEBRAS_TEMPORAL2_MPOINT2_H_

#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "Types.h"
#include "Algebras/Temporal/TemporalAlgebra.h" // UPoint
// for relation related pointers:
#include "Algebras/Relation-C++/RelationAlgebra.h"
class TypeConstructor;


namespace temporal2algebra {
TypeConstructor* getMPoint2TypePtr();

class MPoint2: public Attribute {

public:

    void memClear();
    const MemStorageId getMemId() const;
    void setMemId(const MemStorageId new_id);

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

    std::ostream& Print(std::ostream &os) const;

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
    bool ordered; // better: duringBulkProcessing

    // bool canDestroy // used in Delete und In fo  orig. MPoint
    MemStorageId id;
    DbArray <temporalalgebra::UPoint> units2;
    Rectangle<3> bbox2;
};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_MPOINT2_H_ */
