/*
implementation of MPoint2

*/

#include "MPoint2.h"
#include "SecondoSMI.h"
#include "MovingCalculations.h"
#include "MemStorageManager.h"
#include "GenericTC.h"
#include "TemporalAlgebraFunctions.h"

namespace temporal2algebra {

using temporalalgebra::UPoint;
using temporalalgebra::IPoint;

TypeConstructor* getMPoint2TypePtr() {
    TypeConstructor* type = new GenTC<MPoint2>;
    type->AssociateKind( Kind::TEMPORAL() );
    type->AssociateKind( Kind::DATA() );
    return type;
}


bool CheckMPoint2( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, MPoint2::BasicType() ));
}

ListExpr MPoint2Property() {
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                    nl->StringAtom("Example Type List"),
                    nl->StringAtom("List Rep"),
                    nl->StringAtom("Example List")),
                    nl->FourElemList(nl->StringAtom("-> MAPPING"),
                            nl->StringAtom("(mpoint2) "),
                            nl->StringAtom("( u1 ... un ) "),
                            nl->StringAtom("(((i1 i2 TRUE FALSE)"
                                    " (1.0 2.2 2.5 2.1)) ...)"))));
}


void MPoint2::memClear() {
    cout << "MPoint2::memClear["<< id << "]()\n";
    if (hasMemoryUnits()) {
        MemStorageManager* storage = MemStorageManager::getInstance();
        storage->clear(id);
        id = 0;
    } else {
        //TODO: only allow memClear if we have memUnits???
        cout << "no MemoryUnits present\n";
        //assert(false);
    }
}

// required in OpAppendPositions to explicitly write to sharedMem:
// if data is pushed to mem, no commit on the containing tuple and relation
// is required.
void MPoint2::memAppend(const temporalalgebra::UPoint& upoint) {
    cout << "MPoint2::memAppend[" << id << "](" << upoint << ")\n";
    assert(hasMemoryUnits());
    MemStorageManager* storage = MemStorageManager::getInstance();
    storage->append(id, upoint);
}

void MPoint2::SetBackReference(const BackReference& backReference) {
    assert(!hasMemoryUnits());
    MemStorageManager* storage = MemStorageManager::getInstance();
    assert(id==0);
    id = storage->createId();
    int indexLastUnit = units2.Size() -1;

    Unit finalUnit(0);
    if (indexLastUnit >= 0)
        units2.Get(indexLastUnit, finalUnit);
    else
        finalUnit = Unit(0);
    storage->setBackRef(id, backReference, finalUnit);
}

bool MPoint2::hasMemoryUnits() const {
    return id > 0;
}

/*static*/ bool MPoint2::hasMemoryUnits(const BackReference& backRef) {
    return findMemStorageId(backRef) > 0;
}

/*static*/ MemStorageId MPoint2::findMemStorageId(const BackReference& backRef) {
    MemStorageManager* storage = MemStorageManager::getInstance();
    MemStorageId my_id = storage->getId(backRef);
    cout << " MPoint2::findMemStorageId(" << backRef << "): "
            <<my_id << endl;
    return my_id;
}


/*static*/ Unit MPoint2::getFinalUnit(const MemStorageId& id) {
    MemStorageManager* storage = MemStorageManager::getInstance();
    Unit finalUnit = storage->getFinalUnit(id);
    cout << "MPoint2::getFinalUnit(id: " << id << "): "
            << finalUnit << endl;
    return finalUnit;
}
/*static*/ void MPoint2::appendUnit(const MemStorageId& id, const Unit& unit){
    MemStorageManager* storage = MemStorageManager::getInstance();
    cout << "MPoint2::appendUnit(id: " << id
            << ", unit: " << unit << ")" << endl;
    storage->append(id, unit);
}

const MemStorageId MPoint2::getMemId() const {
    return id;
}

void MPoint2::setMemId(const MemStorageId new_id) {
    id = new_id;
}

/*virtual*/ void MPoint2::Initialize( SmiFileId fileId,
        TupleId tupleId,
        int attrno) {
    cout << *this << "::Initialize("
            << "fileId:" << fileId
            << ", tupleId: " << tupleId
            << ", attrno: " << attrno
            << ")\n";
}

/*virtual*/ void MPoint2::Finalize() {
    cout << *this << "::Finalize()\n";
}

ostream& MPoint2::Print(ostream &os) const {
    os << "[mpoint2 id: " << id << ", del.refs: " << del.refs
            << ", this: " << this
            << "]";
    return os;
}


MPoint2::MPoint2() {
    cout << "MPoint2(): " << *this << endl;
} // should do nothing

MPoint2::MPoint2( const MPoint2& rhs ) :
             Attribute(rhs.IsDefined()),
             id(0),
             units2(0),
             bbox2(rhs.bbox2.IsDefined())
{
    if (!rhs.IsDefined()) {
        cout << "Undefined\n";
        return;
    }

    cout << "MPoint2( "<< rhs << " )\n";

    //optimized Flob-Copy for persistent data
    // TODO: check if resizing to total size makes sense
    units2.copyFrom(rhs.units2);

    //Add Memory Units:
    if (rhs.hasMemoryUnits()) {
        size_t firstMemIndex = rhs.units2.Size();
        size_t numComps = (size_t)rhs.GetNoComponents();
        for (size_t i = firstMemIndex; i < numComps; ++i) {
            UPoint unit;
            rhs.Get(i, unit);
            units2.Append(unit);
        }
    }

    bbox2 = rhs.GetBBox();

    cout << "MPoint2("<< rhs << "): " <<  *this << endl;
}

/*explicit*/ MPoint2::MPoint2(const bool defined):
                 Attribute(defined),
                 id(0),
                 units2(0),
                 bbox2(false)
{
    cout << "MPoint2(defined: " << defined << "): " << *this << endl;

    // Do not allocate ID/Mem slot if no units2 are present:
    // - work around issue that first MPoint2 is created without open DB
    // - save some resources
    // - not all instances allow for backreferences (eg. temp objects)

}


/*explicit*/ MPoint2::MPoint2(const int dummy):
                 Attribute(true),
                 id(0),
                 units2(0),
                 bbox2(false)
{
    cout << "MPoint2(dummy: " << dummy << "): " << *this << endl;
}


MPoint2::~MPoint2() {
    cout << "~MPoint2(): " << *this << endl;
}

MPoint2& MPoint2::operator=(const MPoint2& rhs){
    cout << *this << ".operator= (" << rhs << ")\n";

    if (&rhs == this) {
        return *this;
    }

    Clear();

    if( !rhs.IsDefined() ){
        SetDefined(false);
        return *this;
    }

    SetDefined(true);

    //optimized Flob-Copy for persistent data
    units2.copyFrom(rhs.units2);

    //Add Memory Units:
    if (rhs.hasMemoryUnits()) {
        size_t firstMemIndex = rhs.units2.Size();
        size_t numComps = (size_t)rhs.GetNoComponents();
        for (size_t i = firstMemIndex; i < numComps; ++i) {
            UPoint unit;
            rhs.Get(i, unit);
            units2.Append(unit);
        }
    }

    bbox2 = rhs.GetBBox();

    cout << "operator=(): " << *this << endl;
    return *this;
}



ListExpr MPoint2::ToListExpr(const ListExpr& typeInfo) const{

    cout << "MPoint2::ToListExpr[" << id << "]( .. )\n";
    cout << "IsDefined()=" << IsDefined() << endl;
    temporalalgebra::MPoint* mpoint = new temporalalgebra::MPoint(0);

    if (IsDefined()) {
        mpoint->Clear();
        mpoint->SetDefined(true);
        mpoint->StartBulkLoad();

        size_t numComps = (size_t)GetNoComponents();
        for (size_t i = 0; i < numComps; ++i) {
            UPoint unit;
            Get(i, unit);
            mpoint->Add(unit);
        }

        mpoint->EndBulkLoad(false);
        mpoint->SetDefined(true);
        cout << *mpoint << endl;
    } else {
        mpoint->SetDefined(false);
    }

    ListExpr res = temporalalgebra::OutMapping<
            temporalalgebra::MPoint,
            temporalalgebra::UPoint,
            temporalalgebra::OutUPoint>
    ( typeInfo, SetWord( mpoint ) );
    mpoint->DeleteIfAllowed();
    return res;
}

bool MPoint2::ReadFrom(const ListExpr LE, const ListExpr& typeInfo) {
    cout << "MPoint2::ReadFrom[" << id << "]( "
            + nl->ToString(LE) + ", .. )\n";
    Clear();

    if (listutils::isSymbolUndefined(LE)) {
        SetDefined(false);
        return true;
    }

    bool mp_correct = false;
    ListExpr errorInfo;
    Word mp_ptr = temporalalgebra::InMapping
            <temporalalgebra::MPoint,
            temporalalgebra::UPoint,
            temporalalgebra::InUPoint>(
                    typeInfo, LE, 0, errorInfo, mp_correct
            );
    if (!mp_correct) return false;

    temporalalgebra::MPoint* mpoint =
            static_cast<temporalalgebra::MPoint*> (mp_ptr.addr);
    cout << "MPoint2::ReadFrom( .. )" << *mpoint << endl;
    if (mpoint->IsDefined()) {
        temporalalgebra::UPoint unit(false);
        int noOfComponents = mpoint->GetNoComponents();
        for ( int i = 0; i < noOfComponents; i++) {
            mpoint->Get(i, unit);
            units2.Append(unit);
        }
        bbox2 = mpoint->BoundingBox();
        SetDefined(true);
    } else {
        SetDefined(false);
    }
    mpoint->DeleteIfAllowed();
    cout << "IsDefined()=" << IsDefined() << endl;
    return true;
}

std::string MPoint2::toString() const{
    std::stringstream ss;
    ss << id;
    return ss.str();
}

int MPoint2::Compare(const Attribute* rhs1) const{
    if(!IsDefined()){
        if(!rhs1->IsDefined()){
            return 0;
        } else {
            return -1;
        }
    }
    if(!rhs1->IsDefined()){
        return 1;
    }
    const MPoint2* rhs = static_cast<const MPoint2*>(rhs1);
    if(id < rhs->id){
        return -1;
    } else if(id  > rhs->id){
        return 1;
    }
    //TODO: FIX comparison - we no longer always have an Id
    return 0;
}

bool MPoint2::Adjacent(const Attribute* rhs) const{
    return false;
}

size_t MPoint2::HashValue() const{
    //TODO: FIX HashValue - we no longer always have an Id
    return id;
}

void MPoint2::CopyFrom(const Attribute* attr) {
    cout << *this << "::CopyFrom("
            << *(static_cast<const MPoint2*>(attr)) << ")\n";
    operator=( *((MPoint2*) attr));
    cout << *this << "::CopyFrom("
            << *(static_cast<const MPoint2*>(attr)) << ")\n";
}


MPoint2* MPoint2::Clone() const{
    cout << *this << "::Clone()\n";
    MPoint2* res = new MPoint2(*this);
    cout << *this << "::Clone(): " << *res << endl;
    return res;
}

size_t MPoint2::Sizeof() const {
    return sizeof(*this);
}

/*static*/ const std::string MPoint2::BasicType(){
    return "mpoint2";
}

/*static*/ const bool MPoint2::checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
}

/*static*/ ListExpr MPoint2::Property(){
    return gentc::GenProperty("-> MAPPING",
            BasicType(),
            "( u1 ... un )",
            "(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)) ...)");
}

/*static*/ bool MPoint2::CheckKind(ListExpr type, ListExpr& errorInfo){
    return nl->IsEqual(type,BasicType());
}

// NumOfFLOBs
// this class has one FLOB in form of a DbArray
inline /*virtual*/ int MPoint2::NumOfFLOBs () const {
    return 1;
}
// return the flob if index is correct
inline /*virtual*/ Flob* MPoint2::GetFLOB ( const int i ) {
    assert ( i == 0);
    return &units2;
}

// Functions added to replace direct member access from TemporalAlgebra
Rectangle<3> MPoint2::GetBBox() const {
    Rectangle<3> res(bbox2);
    cout << "GetBBox()\n";
    if (hasMemoryUnits()) { // currently this implies #units2 > 0
        MemStorageManager* storage = MemStorageManager::getInstance();

        int storageSize = storage->Size(id);

        for (int pos = 0;
                pos < storageSize;
                ++pos) {
            Rectangle<3> unitBBox = storage->Get(id, pos).BoundingBox();

            cout << "before: res=" << res << endl;
            cout << "adding: bbx=" << unitBBox << endl;
            if (!bbox2.IsDefined()) {
                res = unitBBox;
            } else {
                res.Extend(unitBBox);
            }
            cout << "after:  res=" << res << endl << endl;
        }
    }
    return res;
}


// Functions copied and adapted from temporalalgebra::MPoint


void MPoint2::Get( const int i, Unit &unit ) const
{
    assert( IsDefined() );
    assert(i >= 0);
    assert(i < GetNoComponents());
    int flobSize = units2.Size();

    if ( i >= flobSize) {
        assert(hasMemoryUnits());
        // requested unit is in memory
        int memPos = i - flobSize;
        MemStorageManager* storage = MemStorageManager::getInstance();
        unit = storage->Get(id, memPos);
    } else {
        bool ok = units2.Get( i, unit );

        if(!ok){
            cout << "Problem in getting data from " << units2 << std::endl;
            assert(ok);
        }
    }

    if ( !unit.IsValid() )
    {
        cout << __FILE__ << "," << __LINE__ << ":"
                << __PRETTY_FUNCTION__
                << " Get(" << i << ", Unit): Unit is invalid:";
        unit.Print(cout); cout << std::endl;
        assert( unit.IsValid());
    }
}

int MPoint2::GetNoComponents() const
{
    assert( IsDefined() );
    int res = units2.Size();
    if (hasMemoryUnits()) {
        MemStorageManager* storage = MemStorageManager::getInstance();
        res += storage->Size(id);
    }
    return res;
}

bool MPoint2::IsEmpty() const
{
    return !IsDefined() || (GetNoComponents() == 0);
}

void MPoint2::Clear()
{
    ordered = true;
    units2.clean();
    memClear();
    del.isDefined = true;
}



void MPoint2::StartBulkLoad() //TODO
{
    assert( IsDefined() );
    assert( ordered );
    ordered = false;
}

void MPoint2::Add( const UPoint& unit ) //TODO only during BulkLoad??
{
    assert( unit.IsDefined() );
    assert( unit.IsValid() );
    if(!IsDefined() || !unit.IsDefined()){
        SetDefined( false );
        memClear();
        return;
    }

    if (hasMemoryUnits()) {
        MemStorageManager* storage = MemStorageManager::getInstance();
        storage->append(id, unit);
    } else {
        units2.Append(unit);
        if (bbox2.IsDefined()) {
            bbox2.Extend(unit.BoundingBox());
        } else {
            bbox2 = unit.BoundingBox();
        }
    }

    RestoreBoundingBox(false); // TODO needed? should be class invariant
}

bool MPoint2::EndBulkLoad(const bool sort, const bool checkvalid) //TODO
{
    bool res = true;
    //start: from Mapping::EndBulkLoad
    assert( !ordered ); // check for active bulk load
    if( !IsDefined() ){
        units2.clean();
        memClear();
    } else if( sort ){
        cout << "not fully impelemented yet\n";
        //   assert(false);
        units2.Sort( temporalalgebra::UnitCompare<UPoint> );
        //memSort(UnitCompare<Unit>) //TODO
    }
    ordered = true;
    units2.TrimToSize();
    if( checkvalid && !IsValid(*this) ){
        units2.clean();
        memClear();
        SetDefined( false );
        res = false;
    }

    //end: from Mapping::EndbulkLoad
    if(res){
        RestoreBoundingBox();
    }
    return res;
}

void MPoint2::RestoreBoundingBox(const bool force /*=false*/) // TODO
{
    if(!IsDefined() || units2.Size() == 0)
    { // invalidate bbox
        bbox2.SetDefined(false);
    }
    else if(force || !bbox2.IsDefined())
    { // construct bbox
        UPoint unit;
        int size = units2.Size(); //TODO: bbox in Attribute: only for pers. data
        Get( 0, unit ); // safe, since (this) contains at least 1 unit
        bbox2 = unit.BoundingBox();
        for( int i = 1; i < size; i++ ){
            Get( i, unit );
            bbox2 = bbox2.Union(unit.BoundingBox());
        }
    } // else: bbox unchanged and still correct
}

void MPoint2::Resize(size_t n){

    // TODO: how to Handle invariant?!
    // shrinkToFit may be better??
    // or only reduce size if the existing units fit?
    if (n < (size_t)GetNoComponents()) {
        cout << "cannot Resize() to " << n
                << ", as requested size is smaller than current size "
                << GetNoComponents() << endl;
        assert(false);
        return;
    }

    if (n == 0) {
        Clear();
        return;
    }

    if (hasMemoryUnits())
    {
        // memUnits don't need to be resized
        return;
    }

    units2.resize(n);
}


bool MPoint2::IsOrdered() const {
    return true; //TODO: fix this...
    //return ordered;
}

} /* temporal2algebra */
