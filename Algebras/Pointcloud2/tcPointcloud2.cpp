/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 The Pointcloud Type Constructor

*/
#include "tcPointcloud2.h"

#include "SecondoSystem.h"
#include "ListUtils.h"

using namespace pointcloud2;

extern NestedList *nl;


double Pointcloud2::CELL_SIZE_IN_M = 0.15;
double Pointcloud2::DELTA_ALT_IN_M = 5.0;
bool Pointcloud2::NEIGHBOUR_CELLS = false;
double Pointcloud2::THRESHOLD_MERGE = 0.5;
double Pointcloud2::DISTANCE_SIG_MAXIMA = 0.3;
double Pointcloud2::OVERLAP_RASTERS = 0.1;
size_t Pointcloud2::MIN_GROUND_CELLS = 100;
size_t Pointcloud2::MAX_CELLS_AT_EDGE = 3;
bool Pointcloud2::SPLIT = true;
size_t Pointcloud2::MIN_OBJ_SIZE = 20;
size_t Pointcloud2::MIN_LOC_MAX_SIZE = 5;
double Pointcloud2::RASTER_CLASSIFY_EPSILON = 0.5;
size_t Pointcloud2::RASTER_CLASSIFY_MINPTS = 2;
size_t Pointcloud2::RASTER_CLASSIFY_SCANSERIES = 5;
double Pointcloud2::RASTER_CLASSIFY_SCANSERIES_ADD = 0.5;
size_t Pointcloud2::RASTER_CLASSIFY_NORMALIZE = 3;
bool Pointcloud2::GET_RASTER_NOT_PC = false;
size_t Pointcloud2::SWITCH_FEATURES = 127;

double Pointcloud2::MINIMUM_OBJECT_EXTENT = 2.0;
size_t Pointcloud2::NEIGHBORHOOD_SIZE = 100;
double Pointcloud2::SOFT_DIFFUSION = 0.67;
double Pointcloud2::ROUGH_DIFFUSION = 0.25;

/* min and max values can be entered in the following function: */
std::unique_ptr<Params> Pointcloud2::params = Pointcloud2::initParams();

/*
2 Secondo TypeConstrucor Interface

*/
TypeConstructor Pointcloud2::typeConstructor(
        "pointcloud2", // name of the type in SECONDO
        Pointcloud2::Property, // property function describing signature
        Pointcloud2::Out, Pointcloud2::In, // Out and In functions
        0, 0, // SaveToList, RestoreFromList functions (forget them)
        Pointcloud2::Create, Pointcloud2::Delete, // object creation
                                                        // and deletion
        Pointcloud2::Open, Pointcloud2::Save, // object open, save
        Pointcloud2::Close, Pointcloud2::Clone, // close, and clone
        Pointcloud2::Cast,
        Pointcloud2::SizeOf,
        Pointcloud2::TypeCheck);

ListExpr Pointcloud2::Property() {
   
    return (nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("->SIMPLE"),
                nl->StringAtom("(" + BasicType() + " (RefSys TupType))" ),
                nl->StringAtom("((x_1 y_1 z_1 (tup_1))*)"),
                nl->StringAtom("((1 1 1))"))));
}


ListExpr Pointcloud2::Out(ListExpr typeInfo, Word value) {
    Pointcloud2* pc2 = static_cast<Pointcloud2*>(value.addr);

    if (!pc2->isDefined()) {
        return listutils::getUndefined();
    }

    const auto secondTypeInfo = nl->Second(typeInfo);
    const ListExpr typeInfo2 = listutils::isSymbol(secondTypeInfo) ?
                nl->OneElemList(secondTypeInfo)
                        : secondTypeInfo;

    const bool hasTuple = nl->HasLength(typeInfo2, 2);

    const ListExpr tupleTypeInfo = hasTuple ?
            nl->OneElemList(nl->Second(typeInfo2)) : nl->Empty();
    // this is how tuple->Out(...) needs tupleTypeInfo
    // (see example in Pointcloud2::In function)

    ListExpr result = nl->TheEmptyList();
    ListExpr last;

    SmiRecordFileIterator it;
    pc2->_pointFile->SelectAll(it);

    SmiRecord record;
    while(it.Next(record))
    {   
        PcPoint point;
        const int read = record.Read(&point, sizeof(PcPoint), 0);
        assert(read == sizeof(PcPoint));
        assert(!(hasTuple && point._tupleId == 0));

        ListExpr pcPointList;
        if(hasTuple)
        {
            Tuple * tuple = pc2->_relation->GetTuple(point._tupleId, false);
            const ListExpr tupleList = tuple->Out(tupleTypeInfo);
            delete tuple;

            pcPointList = nl->FourElemList(
                nl->RealAtom(point._x),
                nl->RealAtom(point._y),
                nl->RealAtom(point._z),
                tupleList
            );
        }else{
            pcPointList = nl->ThreeElemList(
                nl->RealAtom(point._x),
                nl->RealAtom(point._y),
                nl->RealAtom(point._z)
            );
        }

        if(nl->IsEmpty(result))
        {
            result = nl->OneElemList(pcPointList);
            last = result;
        }else{
            last = nl->Append(last, pcPointList);
        }
    }
    return result;
}

Word Pointcloud2::In(ListExpr typeInfo, ListExpr value, int errorPos,
        ListExpr& errorInfo, bool& correct) {
    correct = true;
    Word w;

    const auto secondTypeInfo = nl->Second(typeInfo);
    const ListExpr typeInfo2 = listutils::isSymbol(secondTypeInfo) ?
                nl->OneElemList(secondTypeInfo)
                        : secondTypeInfo;

    const bool hasTuple = nl->HasLength(typeInfo2, 2);
    const int expectedLength = hasTuple ? 4 : 3;
    const ListExpr tupleTypeInfo = hasTuple ?
            nl->OneElemList(nl->Second(typeInfo2)) : nl->Empty();
    // this is how Tuple::In(...) needs tupleTypeInfo!
    // Example: if the following Pointcloud2 is created:
    // [const (pointcloud2(EUCLID (tuple((Num int)(Val real)(Name string)))))
    //   value ((1 2 3 (10 3.1415 "Anton")))];
    // then the variables are:
    // typeInfo:
    //        ((152 0) (EUCLID ((3 0) ((Num (1 0)) (Val (1 1)) (Name (1 3))))))
    // typeInfo2:      (EUCLID ((3 0) ((Num (1 0)) (Val (1 1)) (Name (1 3)))))
    // tupleTypeInfo:         (((3 0) ((Num (1 0)) (Val (1 1)) (Name (1 3)))))
    // without the extra brackets, tupleTypeInfo will not be accepted

    if(nl->AtomType(value) != NoAtom){
        correct = false;
        return w;
    }

    Pointcloud2* pc2 = new Pointcloud2(typeInfo2);
    w.addr = pc2;

    if (!nl->IsEmpty(value) && listutils::isSymbolUndefined(nl->First(value))){
        pc2->setDefined(false);
        return w;
    }

    pc2->startInsert();
    while(!nl->IsEmpty(value))
    {
        ListExpr first = nl->First(value);
        value = nl->Rest(value);

        if(nl->HasLength(first, expectedLength)
            && listutils::isNumeric(nl->First(first))
            && listutils::isNumeric(nl->Second(first))
            && listutils::isNumeric(nl->Third(first)))
        {
            TupleId tupleId = 0;
            if(hasTuple)
            {
                Tuple* tuple = Tuple::In(tupleTypeInfo,
                        nl->Fourth(first), errorPos, errorInfo, correct);
                if(!correct){
                    Pointcloud2::Delete(typeInfo, w);
                    return w;
                }else{
                    tupleId = pc2->insert(tuple);
                }
            }

            PcPoint p {
                listutils::getNumValue(nl->First(first)),
                listutils::getNumValue(nl->Second(first)),
                listutils::getNumValue(nl->Third(first)),
                tupleId
            };


            pc2->insert(p);
        }
    }
    pc2->finalizeInsert();
    
    return w;
}

Word Pointcloud2::Create(const ListExpr typeInfo) {
    const auto secondTypeInfo = nl->Second(typeInfo);
    const ListExpr typeInfo2 = listutils::isSymbol(secondTypeInfo) ?
                nl->OneElemList(secondTypeInfo)
                        : secondTypeInfo;

    Pointcloud2* pc2 = new Pointcloud2(typeInfo2);

    return SetWord(pc2);
}

void Pointcloud2::Close(const ListExpr typeInfo, Word& w) {
    Pointcloud2* pc2 = static_cast<Pointcloud2*>(w.addr);
    delete pc2;
}

Word Pointcloud2::Clone(const ListExpr typeInfo, const Word& w) {
    Pointcloud2* original = static_cast<Pointcloud2*>(w.addr);

    const auto secondTypeInfo = nl->Second(typeInfo);
    const ListExpr typeInfo2 = listutils::isSymbol(secondTypeInfo) ?
                nl->OneElemList(secondTypeInfo)
                        : secondTypeInfo;

    Pointcloud2* clone = new Pointcloud2(typeInfo2);
    clone->copyAllFrom(original);

    return Word(clone);
}

void Pointcloud2::Delete(const ListExpr typeInfo, Word& w) {
    Pointcloud2* pc2 = static_cast<Pointcloud2*>(w.addr);

    if (pc2->_pointFile) {
        pc2->_pointFile->Drop();
    }

    if (pc2->_rtree) {
        pc2->_rtree->DeleteFile();
    }

    if (pc2->_relation) {
        pc2->_relation->Delete(true);
    }

    delete pc2;
}

bool Pointcloud2::Open(SmiRecord& valueRecord, size_t& offset,
        const ListExpr typeInfo, Word& value) {
    const auto secondTypeInfo = nl->Second(typeInfo);
    const ListExpr typeInfo2 = listutils::isSymbol(secondTypeInfo) ?
                nl->OneElemList(secondTypeInfo)
                        : secondTypeInfo;
    try{
        Pointcloud2* pc2 = new Pointcloud2(valueRecord, offset, 
                                        typeInfo2, value);
        value = SetWord(pc2);
    } catch(std::invalid_argument&)
    {
        return false;
    }

    return true;
}

bool Pointcloud2::Save(SmiRecord& valueRecord, size_t& offset,
        const ListExpr typeInfo, Word& value) {

    Pointcloud2* pc2 = static_cast<Pointcloud2*>(value.addr);

    // save whether the pointcloud is defined
    valueRecord.Write(&pc2->_isDefined, sizeof(bool), offset);
    offset += sizeof(bool);

    // save number of Points
    valueRecord.Write(&pc2->_pointCount, sizeof(size_t), offset);
    offset += sizeof(size_t);

    // save pointFileID
    SmiFileId pointFileId = 0;
    if (pc2->hasValidPointFile()) {
        pointFileId = pc2->GetPointFileId();
    }
    valueRecord.Write(&pointFileId, sizeof(SmiFileId), offset);
    offset += sizeof(SmiFileId);


    if(!pc2->_rtree->Save(valueRecord, offset)) {
        return false;
    }
    
    if(pc2->_relation) {
        if(!pc2->_relation->Save(valueRecord, offset, 
            pc2->getRelationTypeInfo(typeInfo))) {
            return false;
        }
    }

    return true;
}

bool Pointcloud2::TypeCheck(ListExpr type) {
    ListExpr dummy = nl->TheEmptyList();
    return TypeCheck(type, dummy);
}

bool Pointcloud2::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    if (nl->HasLength(type, 2)
        && listutils::isSymbol(nl->First(type), BasicType())) {

        const auto secondTypeInfo = nl->Second(type);
/*
At this point we deal with the problem that if you construct
a pointcloud2-object in Secondo with the expression
"const pointcloud2(RefSys)"
the resulting type list will be flat, meaning
(pointcloud2 RefSys) with NO nesting.
However, if the cloud is constructed with an expression like
"const pointcloud2(RefSys TupType)"
the resulting list IS nested:
(pointcloud2 (RefSys TupType)).
Since there is no straightforward way to deal with this behaviour,
the approach taken for now is to explicitly deal with this distinction
in the type-related utility functions in the Pointcloud2::-namespace,
and thus try to hide it from operator type-mappings.

*/
        const ListExpr typeParams =
                        listutils::isSymbol(secondTypeInfo) ?
                                nl->OneElemList(secondTypeInfo)
                                : secondTypeInfo;

        // no symbol, but still an atom
        if (!nl->AtomType(typeParams == NoAtom)) {
                return false;
        }

        if (nl->IsEmpty(typeParams)) {
                return false;
        }

        const bool isSymbol = listutils::isSymbol(nl->First(typeParams));
        if (!isSymbol) {
                return false;
        }

        try {
            Referencesystem::toEnum(nl->SymbolValue(nl->First(typeParams)));
        } catch (std::invalid_argument&) {
                return false;
        }

        if (nl->HasLength(typeParams, 1)) {
                return true;
        }

        if (nl->HasLength(typeParams, 2)
            && Tuple::checkType(nl->Second(typeParams))) {
                return true;
        }
    }

    return false;
}

/*
This function is only meant to be used in Operator Type Mappings.
It does not actually test whether the argument is a Pointcloud,
it assumes it is.

*/
bool Pointcloud2::isTupleCloud(const ListExpr cloudType ) {
        // (? ?)
    if (nl->HasLength(cloudType, 2)
                //(? (? ?))
        && !listutils::isSymbol(nl->Second(cloudType))
        && nl->HasLength(nl->Second(cloudType), 2)) {
        return true;
    }
    return false;
}

/*
Returns a tuple type-list, meaning of the form:
(tuple ( (AttrName type) ... ))
It assumes a Cloud isTupleCloud and will certainly crash otherwise.

*/
ListExpr Pointcloud2::getTupleType(const ListExpr cloudType) {
    return nl->Second(nl->Second(cloudType));
}

/*
This function works on Clouds having Attributes already,
as well as on Clouds having none.

*/
ListExpr Pointcloud2::appendAttributesToCloud(const ListExpr cloudType,
        ListExpr appendage) {
    ListExpr typeParams;
    // see TypeCheck for an explanation of this differentiation
    if (isTupleCloud(cloudType)) {
        typeParams = nl->TwoElemList(
                // RefSys
                nl->First(nl->Second(cloudType)),
                // Tuple Type is of the form (tuple ( (AttrName type){*} ))
                nl->TwoElemList(
                    nl->SymbolAtom(Tuple::BasicType()),
                    listutils::concat(
                        nl->Second(getTupleType(cloudType)),
                        appendage)));
    } else {
        typeParams = nl->TwoElemList(
                // RefSys
                nl->Second(cloudType),
                // construct a Tuple Type
                nl->TwoElemList(
                        nl->SymbolAtom(Tuple::BasicType()),
                        appendage));
    }
    return nl->TwoElemList(
            nl->SymbolAtom(Pointcloud2::BasicType()),
            typeParams);
}

ListExpr Pointcloud2::cloudTypeWithParams(ListExpr refSys) {
    return nl->TwoElemList(
            nl->SymbolAtom(Pointcloud2::BasicType()),
            refSys);
}
ListExpr Pointcloud2::cloudTypeWithParams(ListExpr refSys, ListExpr tupType) {
    return nl->TwoElemList(
            nl->SymbolAtom(Pointcloud2::BasicType()),
            nl->TwoElemList(
                    refSys,
                    tupType));
}

/*
This function is intended to be used for return values of
GetTupleResultType(Supplier s) from RelationCommon in VM.
It takes a name and a list and returns the 0-based index
of the Attribute, and -1 if the name was not found.
(Note: type-values as returned from the supplier are
have such shape:
((152 0) (EUCLID ((3 0) ((ObjID (1 0)) (CatID (1 0))))))
First of each thing might either represent a number in
Secondo Catalogue or a Symbol. But who knows.
Second might represent value - note that VMs typically
return 0 and value is probably nothing of particular
use to us in VM.)

*/
int Pointcloud2::findAttributeIndexInVM(std::string name, 
                                        ListExpr attribute_list) {
    int j = -1; // Tuple::PutAttribute etc. expect 0-based indices
    ListExpr rest = attribute_list;
    while (!nl->IsEmpty(rest)) {
        ListExpr current = nl->First(rest);
        j++;
        if (nl->IsEqual(nl->First(current), name)) {
            return j;
        }
        rest = nl->Rest(rest);
    }
    return -1;
}

void* Pointcloud2::Cast(void* addr) {
    return addr;
}

int Pointcloud2::SizeOf() {
    //Size of relation type is missing
    return sizeof(bool) + 
            sizeof(size_t) + 
            sizeof(SmiFileId) +
            SizeOfRTree<DIMENSIONS>();
}


/*
3 Utilities
3.1 Constructors

*/
Pointcloud2::Pointcloud2(const ListExpr& typeInfo) : _pointCount(0)
{
    setReferencesystem(nl->First(typeInfo));

    _pointFile = RecordFilePtr(new SmiRecordFile(true, 
                                                sizeof(PcPoint), 
                                                false));
    _pointFile->Create();

    _rtree = RTreePtr(new RTreeType(PAGE_SIZE, false));

    //Relation is optional => no exception throwing
    if( nl->HasLength(typeInfo, 2) ) {
        createRelationpointer(nl->Second(typeInfo));
    }
}

Pointcloud2::Pointcloud2(SmiRecord &valueRecord, size_t &offset,
                      const ListExpr typeInfo, Word &value) {

    setReferencesystem(nl->First(typeInfo));

    // read whether pointcloud is defined
    valueRecord.Read(&_isDefined, sizeof(bool), offset);
    offset += sizeof(bool);

    // read number of Points
    valueRecord.Read(&_pointCount, sizeof(size_t), offset);
    offset += sizeof(size_t);

    // read pointFileID
    SmiFileId pointFileId;
    valueRecord.Read(&pointFileId, sizeof(SmiFileId), offset);
    offset += sizeof(SmiFileId);

    _pointFile = RecordFilePtr(
            new SmiRecordFile(true, sizeof(PcPoint), false));
    if (pointFileId != 0) {
        _pointFile->Open(pointFileId);
    }

    if(!pointcloud2::UnbrokenOpenRTree<DIMENSIONS>(
        valueRecord, offset, typeInfo, value)) {
        throw std::invalid_argument("Could not restore RTree.");
    }
    _rtree = RTreePtr(static_cast<RTreeType*>(value.addr));

    //Relation is optional => no exception thrPointcloud2Pointcloud2owing
    if( nl->HasLength(typeInfo, 2) ){
        setRelationpointer(valueRecord, offset, nl->Second(typeInfo));
    }
}

/*
3.2 Getters and Setters

*/
PcPointPtr Pointcloud2::getPoint(const size_t pointId) const {
    PcPointPtr point = std::make_shared<PcPoint>();
    getPoint(pointId, point.get());
    return point;
}

void Pointcloud2::getPoint(const size_t pointId, PcPoint* pcPoint) const {
    SmiRecord record;
    int RecordSelected = _pointFile->SelectRecord(pointId, record,
            SmiFile::ReadOnly);
    assert(RecordSelected); // pointId is 1-based!

    const int read = record.Read(pcPoint, sizeof(PcPoint), 0);

    assert(read == sizeof(PcPoint));
}

SmiRecordFileIterator* Pointcloud2::getFileIterator() const {
        SmiRecordFileIterator* it = new SmiRecordFileIterator();
        _pointFile->SelectAll(*it);
        return it;
}

Tuple* Pointcloud2::getTuple(const TupleId& tupleId) const {
    return _relation->GetTuple(tupleId, true);
}

TupleType* Pointcloud2::getTupleType() const {
    return _relation->GetTupleType();
}

void Pointcloud2::updateTuple(Tuple *tuple,
        const std::vector<int>& changedIndices,
        const std::vector<Attribute*>& newAttrs) {
    _relation->UpdateTuple(tuple, changedIndices, newAttrs);
}

int Pointcloud2::getMaxValueOfIntAttr(const int attrIndex,
        const int defaultValue, std::shared_ptr<BitArray> bitMask) const {

    if (!hasRelation())
        return defaultValue;

    int maxValue = defaultValue;

    // iterate over the points in the SmiRecordFile (we cannot iterate
    // directly over the tuples since we need the SmiRecordIds)
    SmiRecordFileIterator it;
    _pointFile->SelectAll(it);
    SmiRecord record;
    while(it.Next(record)) {
        // get the next point
        PcPoint point;
        const size_t read = record.Read(&point, sizeof(PcPoint));
        assert(read == sizeof(PcPoint));

        // get the corresponding tuple
        Tuple* tuple = _relation->GetTuple(point._tupleId, true);

        CcInt* attrValueRef = (CcInt*)tuple->GetAttribute(attrIndex);
        if (attrValueRef->IsDefined()) {
            // if the attribute value differs from the default,
            // set the bit in the bitMask to false
            int attrValue = attrValueRef->GetIntval();
            if (attrValue != defaultValue)
                bitMask->set(record.GetId(), false);
            // update maxValue
            maxValue = MAX(maxValue, attrValue);
        }
        tuple->DeleteIfAllowed();
    }
    return maxValue;
}

void Pointcloud2::setReferencesystem(const ListExpr& expr)
{
    const std::string referenceSytemAsString = nl->SymbolValue(expr);
    _referencesystem = Referencesystem::toEnum(referenceSytemAsString);
}

void Pointcloud2::setRelationpointer(SmiRecord &valueRecord, size_t &offset, 
                                    const ListExpr& expr)
{
    const ListExpr relType = getRelationTypeInfo(expr);
    _relation = RelationPtr(Relation::Open(valueRecord, offset, relType ));
}

void Pointcloud2::createRelationpointer(const ListExpr& expr)
{
    const ListExpr relType = getRelationTypeInfo(expr);
    _relation = RelationPtr(new Relation(relType));
}


Rect3 Pointcloud2::getBoundingBox() const
{
    if (_isDefined){
        return _rtree->BoundingBox(); // returns undefined BoundingBox if empty
    }else{
        return BBox<DIMENSIONS>(false);
    }
}


size_t Pointcloud2::getPointCount() const
{
    return _pointCount;
}

ListExpr Pointcloud2::getRelationTypeInfo(const ListExpr& tupleTypeInfo) const
{
    return nl->TwoElemList(nl->SymbolAtom("rel"), tupleTypeInfo);
}


/*
3.3 Insert Functions

Starts the RTree bulk load mode, if possible (i.e. if the RTree is empty).
After calling insert(...) multiple times, bulk load must be finalized by
calling finalizeInsert().

*/
void Pointcloud2::startInsert() {
    _mayStartBulkMode = (_pointCount == 0);
}

/*
Adds the given point and its tuple to the pointcloud. The tuple is being
consumed (i.e. tuple->DeleteIfAllowed() is called), so any subsequent use of
the tuple is forbidden (especially, tuple->DeleteIfAllowed() must not be
called again!)

*/
size_t Pointcloud2::insert(PcPoint& point, Tuple* const tuple) {
    const TupleId tupleId = insert(tuple);
    point._tupleId = tupleId;
    return insert(point);
}

/*
Adds the given tuple to the relation. The tuple is being
consumed (i.e. tuple->DeleteIfAllowed() is called), so any subsequent use of
the tuple is forbidden (especially, tuple->DeleteIfAllowed() must not be
called again!)

*/
TupleId Pointcloud2::insert(Tuple* const tuple) {
    _relation->AppendTuple(tuple);
    TupleId id = tuple->GetTupleId();
    tuple->DeleteIfAllowed();
    return id;
}

size_t Pointcloud2::insert(const PcPoint& point) {
    SmiRecord record;
    SmiRecordId pointId; // AppendRecord sets pointId to a free, 1-based ID

    bool recordSelected = _pointFile->AppendRecord(pointId, record);
    assert(recordSelected);

    const int written = record.Write(&point, sizeof(PcPoint), 0);
    assert(written == sizeof(PcPoint));

    R_TreeLeafEntry<DIMENSIONS,SmiRecordId> entry(
            point.getBoundingBox(), pointId);

    // if this is the first insert(const PcPoint&) after startInsert() was
    // called, try to start the RTree bulk load
    if (_mayStartBulkMode) {
        _bulkMode = _rtree->InitializeBulkLoad();
        _mayStartBulkMode = false; // do not try again
    }

    if (_bulkMode)
        _rtree->InsertBulkLoad(entry);
    else
        _rtree->Insert(entry);

    ++_pointCount;
    return pointId;
}

void Pointcloud2::insertTupleForExistingPoint(Tuple* tuple, PcPoint& point,
        SmiRecordId smiRecordId) {
    assert (point._tupleId == 0);

    TupleId tupleId = insert(tuple);
    point._tupleId = tupleId;

    SmiRecord record;
    bool recordSelected = _pointFile->SelectRecord(
            smiRecordId, record, SmiFile::Update);
    assert(recordSelected);

    int offset = 0;
    int written = record.Write(&point, sizeof(PcPoint), offset);
    assert(written == sizeof(PcPoint));
}


/*
Copies a point and a tuple (if there is any) from source to this pointcloud2.
The source pointcloud2 must have the same structure as this pointcloud2.

*/
void Pointcloud2::insert(PcPoint& point, const Pointcloud2* source) {
    if (_relation && (point._tupleId != 0)) {
        Tuple* tuple = source->getTuple(point._tupleId);
        insert(point, tuple); // gets new tupleId
    } else {
        insert(point);
    }
}

/*
Finalizes the RTree bulk load mode, making the RTree persistent.

*/
void Pointcloud2::finalizeInsert() {
    if (_bulkMode)
        _rtree->FinalizeBulkLoad();
    _bulkMode = false;
    _mayStartBulkMode = false;
}

/*
3.4 Copy functions for restrict operators

3.4.1 copyAllFrom source

Copies all points and tuples (if there is any) from source to this pointcloud2.
The source pointcloud2 must have the same structure as this pointcloud2.

*/
void Pointcloud2::copyAllFrom(const Pointcloud2* source) {
    _isDefined = source->_isDefined;
    if (!_isDefined)
        return; // no need to copy anything else

    // assert that this pointcloud2 is empty (otherwise we would need
    // to adjust the point._tupleId values!)
    assert(_pointCount == 0);
    _rtree->Clone(source->_rtree.get());

    copy_file(source);

    if (source->_relation) { // relation is optional
        Tuple *t;
        GenericRelationIterator *iter = source->_relation->MakeScan();
        while ((t = iter->GetNextTuple()) != 0) {
            _relation->AppendTuple(t);
            t->DeleteIfAllowed();
        }
        delete iter;
    }
}

/*
This function performs a deep copy of a Pointcloud2. If the source cloud
has a relation already, all tuples in it will be copied. If the given
newAttributes vector contains elements, those attributes will be initialized
in each tuple.

*/
void Pointcloud2::copyAllWithAdditionalAttributes(
        const Pointcloud2* source,
        const std::vector<std::unique_ptr<Attribute>>& newAttributes) {

    // if newAttributes is empty, we can simply use the copyAll method:
    if (newAttributes.size() == 0) {
        this->copyAllFrom(source);
        return;
    }

    _isDefined = source->_isDefined;
    if (!_isDefined)
        return; // no need to copy anything else

    // assert that this pointcloud2 is empty (otherwise we would need
    // to adjust the point._tupleId values!)
    assert(_pointCount == 0);

    // copy the RTree and the SmiFile
    _rtree->Clone(source->_rtree.get());
    copy_file(source);

    // copy the relation only if the source Pointcloud2 has one
    if (source->_relation) {
        int newAttrsStartIndex = _relation->GetTupleType()->GetNoAttributes()
                - newAttributes.size();
        Tuple *sourceTuple;
        GenericRelationIterator *iter = source->_relation->MakeScan();
        while ((sourceTuple = iter->GetNextTuple()) != 0) {
            Tuple* tup = this->createEmptyTuple();
            // copy existing attributes from source tuple
            for (int i = 0; i < sourceTuple->GetNoAttributes(); ++i)
                tup->CopyAttribute(i, sourceTuple, i);
            // add new attributes
            int newAttrsIndex = newAttrsStartIndex;
            for (const std::unique_ptr<Attribute>& newAttribute : newAttributes)
                tup->PutAttribute(newAttrsIndex++, newAttribute->Clone());
            // append tuple to relation. TupleIds must be the same as in
            // the source cloud, otherwise we would need to update the
            // references in the _pointFile!
            _relation->AppendTuple(tup);
            assert (tup->GetTupleId() == sourceTuple->GetTupleId());
            sourceTuple->DeleteIfAllowed();
            tup->DeleteIfAllowed();
        }
        delete iter;
    } else {
        SmiRecordFileIterator it;
        _pointFile->SelectAll(it);

        SmiRecord record;
        while (it.Next(record)) {
            PcPoint point;
            const size_t read = record.Read(&point, sizeof(PcPoint));
            assert(read == sizeof(PcPoint));
            int newAttrsIndex = 0;
            Tuple* tup = this->createEmptyTuple();
            for (const std::unique_ptr<Attribute>& 
                                  newAttribute : newAttributes) {
                tup->PutAttribute(newAttrsIndex++, newAttribute->Clone());
            }
            insertTupleForExistingPoint(tup, point, record.GetId());
            // do NOT call tup->DeleteIfAllowed() here!
        }
    }
}

void Pointcloud2::copy_file(const Pointcloud2* source) {

    SmiRecordFileIterator it;
    source->_pointFile->SelectAll(it);

    SmiRecord record;
    while (it.Next(record)) {
        PcPoint point;
        const size_t read = record.Read(&point, sizeof(PcPoint));
        assert(read == sizeof(PcPoint));
        SmiRecord newRecord;
        SmiRecordId newRecordId; // AppendRecord sets newRecordId to a free,
        // 1-based ID.
        bool recSelected = _pointFile->AppendRecord(newRecordId, newRecord);
        assert (recSelected);
        SmiSize written = newRecord.Write(&point, sizeof(PcPoint), 0);
        assert(written == read);
    }
    _pointCount = source->_pointCount;
}

/*
3.4.2 copySelectionFrom source

*/
size_t Pointcloud2::copySelectionFrom(const Pointcloud2* source,
        const PcBox* bbox) {

    // copying from an undefined source makes this instance undefined, too
    if (!source->isDefined() || !_isDefined || !bbox->IsDefined()) {
        this->setDefined(false);
        return 0; // 0 points were copied
    }

    // calculate the intersection
    const PcBox sourceBbox = source->getBoundingBox();
    const PcBox intersectionBbox = sourceBbox.Intersection(*bbox);

    // are there any source points in the filter bbox?
    if (intersectionBbox.IsEmpty()) {
        return 0; // nothing to copy
    }

    // is the source bbox completely inside the filter bbox?
    if (bbox->Contains(sourceBbox)) {
        copyAllFrom(source);
        return source->_pointCount; // all points were copied
    }

    // determine whether to use the RTree or read the SmiRecordFile:

    // using the RTree to find source points inside the bbox is only
    // worth the effort if the bbox is significantly smaller than the
    // total source bbox (e.g., below 10-20%). Note that reading RTree entries
    // is not alternative, but additional to reading the SmiRecordFile entries
    // and that RTree entries require 6 * 8 + 4 = 52 bytes (as they contain a
    // box rather than a point) while SmiRecords only need 3 * 8 + 4 = 28 bytes
    // (i.e. a given number of points require 85% more RTree blocks than
    // SmiRecordFile blocks!)
    constexpr double INTERSECTION_RATIO_THRESHOLD = 0.1;
    // TODO: in Performancetests einen geeigneten Wert ermitteln: 0.1? 0.2?

    // also, using the RTree is only worth the effort if there are more
    // than just a few blocks in the SmiRecordFile:
    constexpr size_t POINT_COUNT_THRESHOLD = 8 * PAGE_SIZE / sizeof(PcPoint);

    const double intersectionRatio = intersectionBbox.Area()/sourceBbox.Area();
    const bool useRTree = (intersectionRatio < INTERSECTION_RATIO_THRESHOLD
        && source->getPointCount() > POINT_COUNT_THRESHOLD);

    cout << "intersection ratio = " << intersectionRatio;
    cout << ", source point count = " << source->getPointCount();
    cout << " => " << (useRTree ? "traversing RTree." :
            "reading SmiRecordFile") << endl;

    const size_t initialPointCount = _pointCount;
    startInsert();
    if (useRTree) {
        // extend the bbox by the FACTOR which is used for the points
        // stored in the RTree, so wo can determine whether an RTree entry
        // is inside the bbox
        auto bboxExtended = std::unique_ptr<PcBox>(bbox->Clone());
        bboxExtended->Extend(FACTOR);

        // traverse the RTree of the source pointcloud2 to find nodes and
        // points that are located inside the bboxExtended
        copySelectionFromRTree(source, bboxExtended.get(),
                source->_rtree->RootRecordId(), false);

    } else {
        // read points directly from the SmiRecordFile
        SmiRecordFileIterator it;
        source->_pointFile->SelectAll(it);
        SmiRecord record;
        PcPoint point;
        while(it.Next(record)) {
            const size_t read = record.Read(&point, sizeof(PcPoint));
            assert(read == sizeof(PcPoint));

            // no +-FACTOR required here
            const double pointRect[] = { point._x, point._x, 
                                        point._y, point._y,
                                        point._z, point._z }; 

            if (bbox->Contains( {true, pointRect} ))
                insert(point, source);
        }
    }
    finalizeInsert();

    // return the number of points that were added
    return _pointCount - initialPointCount;
}

/*
3.4.3 copySelectionFromRTree

traverse the RTree of the source pointcloud2 to find points that are located
inside the given bbox. If isInsideBbox is true, it is clear from the
node's bounding box that every point in the current node (and subtree!) is
inside the given bbox (with no further checks necessary)

*/
void Pointcloud2::copySelectionFromRTree(const Pointcloud2* source, PcBox* bbox,
            SmiRecordId adr, bool isInsideBbox) {

    RTreeNodePtr node = source->_rtree->GetMyNode(adr, false,
            source->_rtree->MinEntries(0), source->_rtree->MaxEntries(0));

    for (int j = 0; j < node->EntryCount(); ++j) {
        if (node->IsLeaf()) {
            const RTreeLeafEntry e = (RTreeLeafEntry&)(*node)[j];
            if (isInsideBbox || bbox->Contains(e.box)) {
                PcPoint point;
                size_t read;
                source->_pointFile->Read(e.info, &point, sizeof(PcPoint),
                        0, read);
                assert(read == sizeof(PcPoint));
                insert(point, source);
            }
        } else {
            RTreeInternalEntry e = (RTreeInternalEntry&)(*node)[j];
            if (bbox->Intersects(e.box)) {
                // determine whether the whole node/subtree is inside the bbox;
                // if so, further bbox->Contains checks can be omitted
                // on this node and its subtree
                const bool subtreeInside=isInsideBbox || bbox->Contains(e.box);
                copySelectionFromRTree(source, bbox, e.pointer, subtreeInside);
            }
        }
    }
    delete node;
}

/*
3.4.2 copySelectionFrom source using a BitArray

*/
size_t Pointcloud2::copySelectionFrom(const Pointcloud2* source,
        const BitArray& bitMap) {

    // copying from an undefined source makes this instance undefined, too
    if (!source->isDefined() || !_isDefined) {
        this->setDefined(false);
        return 0; // 0 points were copied
    }
    assert (source->getPointCount() <= bitMap.getSize());

    const size_t initialPointCount = _pointCount;

    startInsert();
    // read points directly from the SmiRecordFile
    SmiRecordFileIterator it;
    source->_pointFile->SelectAll(it);
    SmiRecord record;
    PcPoint point;
    while(it.Next(record)) {
        if (!bitMap.get(record.GetId()))
            continue;

        const size_t read = record.Read(&point, sizeof(PcPoint));
        assert(read == sizeof(PcPoint));
        insert(point, source);
    }
    finalizeInsert();

    // return the number of points that were added
    return _pointCount - initialPointCount;
}

/*
3.5 Cluster method

Clusters the points in this Pointcloud2 using DbScan and copies the points and
their clusterIds to the given destination Pointcloud2 (destPc2).

For minClusterSizeToCopy, use 0 to copy all points, 1 to eliminate noise,
or any other positive value to remove both noise and small clusters.

For each source attribute index, destAttrIndices[sourceAttrIndex]
returns the corresponding index in destPc2 (or -1 if the attribute should
not be copied to destPc2.

*/
void Pointcloud2::cluster(const double eps, const size_t minPts,
        size_t minClusterSizeToCopy, Pointcloud2* destPc2,
        int clusterAttrIndex, std::vector<int> destAttrIndices) const {
    // TODO: falls this bereits ein "Cluster"-Attribut hat, sollte auch
    // this == destPc2 erlaubt (und dann unten berücksichtigt werden!

    // TODO: offen ist noch, wie in dem Fall vorzugehen ist, dass die Punkte
    // nicht in den Arbeitsspeicher passen: dann soll laut Aufgabenstellung
    // der vorhandene RTree benutzt werden - aber was ist mit der ClusterId?
    // - Passt zumindest die in den Arbeitsspeicher?
    // - Oder soll grundsätzlich eine ClusterId im RTree / im SmiRecordEntry
    //   vorgesehen werden? 
    // und muss die Implementierung dafür dann doppelt vorliegen?

    // DEBUG
    cout << endl << "Point count: " << _pointCount << endl;
    // for(size_t i = 1; i <= _pointCount; ++i) { // getPoint 1-based!
    //     cout << getPoint(i)->toString() << endl;
    // }
    // cout << "Starting DBSCAN ..." << endl;

    // fill the point data allocated in the main memory
    std::shared_ptr<std::vector<DbScanPoint<DIMENSIONS>>> pointSpace =
            std::make_shared<std::vector<DbScanPoint<DIMENSIONS>>>();
    pointSpace.get()->resize(_pointCount + 1);
    std::vector<PointIndex> indexOfSmiId =
            getAllPoints(*(pointSpace.get()), SEQUENCE_LENGTH_MAX);

    // indexOfSmiId now contains index positions with which the points
    // were sorted into the pointSpace in RTree order (i.e. points from
    // the same RTree node have neighbor index positions).
    // For a given SmiRecordId id, indexOfId.get()[id] returns the unique
    // index position in the pointSpace.

    // create a DbScan instance using the constant Pointcloud2::DIMENSIONS
    DbScan<DIMENSIONS> dbScan(eps, minPts, pointSpace, true);
    dbScan.run();

    // -----------------------------------------------------
    // Transfer the result to the destination Pointcloud2

    bool copyAllPoints = (minClusterSizeToCopy == 0) ||
            (minClusterSizeToCopy == 1 && dbScan.getNoiseCount() == 0);
    bool sourceHasTuples = hasRelation();
    bool destHasTuples = destPc2->hasRelation();

    destPc2->_isDefined = _isDefined;
    destPc2->startInsert();
    // as an alternative to inserting points, cloning the RTree was tested:
    // if (copyAllPoints) { destPc2->_rtree->Clone(_rtree.get()); } else { ...
    // (pc. Pointcloud2::copyAllFrom()), however, this does not work -
    // there is a crash when the Pointcloud2 is closed and the RTree performs
    // WriteHeader.

    SmiRecordFileIterator it;
    _pointFile->SelectAll(it); // iterate over source points

    SmiRecord sourceRecord;
    SmiRecordId sourceSmiId;
    while(it.Next(sourceSmiId, sourceRecord)) {
        PcPoint point;
        const size_t read = sourceRecord.Read(&point, sizeof(PcPoint));
        assert(read == sizeof(PcPoint));

        // get the clusterId that DbScan has assigned to this point
        PointIndex pointSpaceIndex = indexOfSmiId[sourceSmiId];
        int clusterId = pointSpace.get()->at(pointSpaceIndex)._clusterId;

        // if noise or small clusters should be filtered out, check if
        // point must be skipped
        if (!copyAllPoints && (clusterId == SCAN_NOISE ||
                dbScan.getClusterSize(clusterId) < minClusterSizeToCopy)) {
            continue;
        }

        if (destHasTuples) {
            // create tuple
            Tuple* destTuple = destPc2->createEmptyTuple();

            if (sourceHasTuples) {
                // copy attributes from source tuple
                Tuple* sourceTuple = getTuple(point._tupleId);
                for (size_t i = 0; i < destAttrIndices.size(); i++) {
                    int destIndex = destAttrIndices[i];
                    if (destIndex >= 0) // otherwise, this attribute is omitted
                        destTuple->CopyAttribute(i, sourceTuple, destIndex);
                }
                sourceTuple->DeleteIfAllowed();
            }
            // put cluster id to attribute and insert tuple to destPc2
            if (clusterAttrIndex >= 0) {
                destTuple->PutAttribute(clusterAttrIndex,
                        new CcInt(true, clusterId));
            }
            // insert tuple to destPc2
            point._tupleId = destPc2->insert(destTuple);
            // do NOT call destTuple->DeleteIfAllowed() here!
        }

        destPc2->insert(point);
        // do NOT call destTuple->DeleteIfAllowed(); here (done in ->insert)
    }
    destPc2->finalizeInsert();
}

/*
3.5.1 getAllPoints

Fills the preallocated array pointSpace with the coordinates of all points in
this Pointcloud2. Rather than using the SmiFile order, the persistent RTree is
used to create sequences of points in pointSpace. These sequences can be
refined to a maximum size given by maxSequenceLength. If refineSequneces
is false, the sequences will not be refined.

The return value indexOfSmiId is filled with index positions used to create
this order. For a given SmiRecordId id, the value indexOfId[id]
is a unique index position. All points from the same RTree node are in a
sequence of index positions in pointSpace.

*/
std::vector<PointIndex> Pointcloud2::getAllPoints(
        std::vector<DbScanPoint<DIMENSIONS>>& pointSpace,
        const int maxSequenceLength) const {

    bool REPORT_TO_CONSOLE = false;

    assert (pointSpace.size() == _pointCount + 1); // 1-based

    // create vector to map SmiRecordIds to 1-based index positions
    // (SmiRecordIds are 1-based, therefore "_pointCount + 1");
    // initialize with value 0 in case of unused SmiRecordIds
    std::vector<PointIndex> indexOfSmiId =
            std::vector<PointIndex>( _pointCount + 1, 0 );

    // initialize pointSpace with _isLastInSeq = false; getRTreeOrder()
    // assumes false is set and only stores true where applicable
    for (PointIndex i = 0; i <= _pointCount; ++i)
        pointSpace[i].initialize(false);

    // --------------------------------
    // 1. get the order in which to store the points
    // (points in the same RTree node will be stored sequentially)

    // calculate RTree order; this also sets _isLastInSeq in pointSpace
    // (although the points coordinates will only be read below)
    PointIndex index = 0;
    getRTreeOrder(_rtree->RootRecordId(), indexOfSmiId, index, pointSpace,
            maxSequenceLength);

    if (REPORT_TO_CONSOLE) {
        for (size_t smiId = 1; smiId <= _pointCount; ++smiId) {
            size_t index = indexOfSmiId[smiId];
            cout << "SmiRecId " << smiId << ": index " << index;
            if (pointSpace[index]._isLastInSeq)
                cout << " (last in sequence)";
            cout << endl;
        }
        cout << endl;
    }

    // --------------------------------
    // 2. read points to main memory

    // read the points into the main memory, using the "rtree order"
    // which was stored in indexOfSmiId
    PcPoint point;
    for (size_t smiRecId = 1; smiRecId <= _pointCount; ++smiRecId) {
        // get the index at which to store this point in pointSpace
        PointIndex index = indexOfSmiId[smiRecId];
        DbScanPoint<DIMENSIONS>& destPoint = pointSpace[index];

        // copy this point's coordinates to DbScanPoint (keeping _isLastInSeq
        // which was already calculated by getRTreeOrder above)
        getPoint(smiRecId, &point);
        double coords[DIMENSIONS] {point._x, point._y, point._z};
        destPoint.setCoords(coords);
    }

    if (REPORT_TO_CONSOLE) {
        for(size_t i = 1; i <= _pointCount; ++i) { // pointSpace is 1-based
            cout << pointSpace[i].toString() << endl;
            if (pointSpace[i]._isLastInSeq)
                cout << "----------" << endl;
        }
    }

    return indexOfSmiId;
}

/*
3.5.2 getRTreeOrder

Fills indexOfSmiId with index positions by which the points can be sorted in
RTree order (i.e. points in the same RTree node get neighbor index positions).
For a given SmiRecordId id, indexOfId.get()[id] is a unique
index position.

pointSpace.get()[index]._isLastInSeq is set to true for the last point in
each sequence (note that index is the pointSpace index, not the SmiRecordId!)

*/
void Pointcloud2::getRTreeOrder(SmiRecordId adr,
        std::vector<PointIndex>& indexOfSmiId, PointIndex& index,
        std::vector<DbScanPoint<DIMENSIONS>>& pointSpace,
        const int maxSequenceLength) const {
    RTreeNodePtr node = _rtree->GetMyNode(adr, false,
            _rtree->MinEntries(0), _rtree->MaxEntries(0));
    int entryCount = node->EntryCount();
    if (!node->IsLeaf()) {
        for (int i = 0; i < entryCount; ++i) {
            RTreeInternalEntry e = (RTreeInternalEntry&)(*node)[i];
            getRTreeOrder(e.pointer, indexOfSmiId, index, pointSpace,
                    maxSequenceLength);
        }
    } else if (maxSequenceLength < 0 || entryCount <= maxSequenceLength) {
        // use all (up to 76) entries of the RTree node as one sequence
        for (int i = 0; i < entryCount; ++i) {
            const RTreeLeafEntry e = (RTreeLeafEntry&)(*node)[i];
            ++index;
            indexOfSmiId[e.info] = index;
        }
        pointSpace[index]._isLastInSeq = true;
    } else {
        // insert the points from this RTree leaf into a temporary MMRTree
        // in order to split them into sequences of min..maxSeqLength points
        // (MMRTree expects 1 <= min <= max / 2)
        using MMRTreeT = mmrtree::RtreeT<DIMENSIONS, SmiRecordId>;
        size_t minSeqLength = maxSequenceLength / 2;
        std::unique_ptr<MMRTreeT> mmRTree(
                new MMRTreeT(minSeqLength, maxSequenceLength));
        for (int i = 0; i < entryCount; ++i) {
            const RTreeLeafEntry e = (RTreeLeafEntry&)(*node)[i];
            mmRTree->insert(e.box, e.info);
        }

        // traverse MMRTree, adding entries in indexOfSmiId and marking
        // the sequences in isLastInSeq
        std::unique_ptr<MMRTreeT::iterator> it { mmRTree->entries() };
        SmiRecordId const *smiId;
        uintptr_t lastNodeId = 0;
        while( (smiId = it->next()) != 0) {
            // is this entry from a different MMTree node?
            uintptr_t nodeId = it->getNodeId();
            if (nodeId != lastNodeId) {
                // mark the previous entry as last in sequence
                // (note that this happens before ++index!)
                pointSpace[index]._isLastInSeq = true;
            }
            // add entry to indexOfSmiId
            ++index;
            indexOfSmiId[*smiId] = index;
            lastNodeId = nodeId;
        }
        pointSpace[index]._isLastInSeq = true;
    }
    delete node;
}

/*
3.5 Temporary Fixes

*/
template <unsigned dim>
bool pointcloud2::UnbrokenOpenRTree( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  R_Tree<dim, TupleId> *rtree = new R_Tree<dim, TupleId>( fileid, false );
  value = SetWord( rtree );

  return true;
}

/*
3.4 merge operator functions

*/
void Pointcloud2::merge(const Pointcloud2* source,
                        const bool hasTuple,
                        const std::vector<int> sourceAttrIndex,
                        const ListExpr tupleTypeInfo) {
    if (!source->_isDefined)
        return;

    const int attrCount = hasTuple ? sourceAttrIndex.size() : 0;

    SmiRecordFileIterator it;
    source->_pointFile->SelectAll(it);
    SmiRecord record;
    // do NOT call startInsert() here, the caller of this method does!
    while(it.Next(record)) {
        PcPoint point;
        const size_t read = record.Read(&point, sizeof(PcPoint));
        assert(read == sizeof(PcPoint));
        if (hasTuple) {
            Tuple * tuplesource = source->_relation->GetTuple(
                                                    point._tupleId, true);
            Tuple * tupletarget = new Tuple(tupleTypeInfo);
            for (int i = 0; i < attrCount; i++) {
               tupletarget->CopyAttribute(sourceAttrIndex[i]-1, tuplesource,i);
            }
            insert(point, tupletarget);
            tuplesource->DeleteIfAllowed();
        } else {
            insert(point);
        }
    }
    // do NOT call finalizeInsert() here, the caller of this method does!
}

std::unique_ptr<Params> Pointcloud2::initParams() {
    std::unique_ptr<Params> res= std::unique_ptr<Params>(new Params());

    // intParam_t MIN_INT = std::numeric_limits<intParam_t>::min();
    intParam_t MAX_INT = std::numeric_limits<intParam_t>::max();
    double MIN_REAL = -std::numeric_limits<double>::max();
    double MAX_REAL = std::numeric_limits<double>::max();


    Param::CONTEXT raster = Param::CONTEXT::analyzeRaster;

    res->add( { "CELL_SIZE_IN_M", &CELL_SIZE_IN_M, raster,
        0.01, 100.0, "Raster: cell size in m" });

    res->add( { "DELTA_ALT_IN_M", &DELTA_ALT_IN_M, raster,
        0.01, 100.0, "Raster: max delta alt for flooding in m" });

    res->add( { "NEIGHBOUR_CELLS", &NEIGHBOUR_CELLS, raster,
        "Raster: using 4 oder 8 neighbor cells" });

    res->add( { "THRESHOLD_MERGE", &THRESHOLD_MERGE, raster,
        0.001, 200.0, "Raster: threshold for merging objects in m" });

    res->add( { "DISTANCE_SIG_MAXIMA", &DISTANCE_SIG_MAXIMA, raster,
        0.01, 200.0, "Raster: dist. to merge local to sign. maxima in m" });

    res->add( { "OVERLAP_RASTERS", &OVERLAP_RASTERS, raster,
        0.0, 50.0, "Raster: overlapping of raster in memory in %" });

    res->add( { "MIN_GROUND_CELLS", &MIN_GROUND_CELLS, raster,
        1L, MAX_INT, "Raster: minimum cells for ground object" });

    res->add( { "MAX_CELLS_AT_EDGE", &MAX_CELLS_AT_EDGE, raster,
        1L, 100L, "Raster: min cells at edge to exclude object"});

    res->add( { "SPLIT", &SPLIT, raster,
        "Raster: true uses split objets by local maxima routine"});

    res->add( { "MIN_OBJ_SIZE", &MIN_OBJ_SIZE, raster,
        1L, 1000L, "Raster: minimum cells of object"});

    res->add( { "MIN_LOC_MAX_SIZE", &MIN_LOC_MAX_SIZE, raster,
        1L, 100L, "Raster: minimum cells of local maximum"});

    res->add( { "RASTER_CLASSIFY_EPSILON", &RASTER_CLASSIFY_EPSILON,
        raster, 0.0, MAX_REAL, "Raster: dbscan epsilon for classify" });

    res->add( { "RASTER_CLASSIFY_MINPTS", &RASTER_CLASSIFY_MINPTS,
        raster, 2, MAX_INT, "Raster: dbscan minimum points for classify" });

    res->add( { "RASTER_CLASSIFY_SCANSERIES", &RASTER_CLASSIFY_SCANSERIES,
            raster, 1, MAX_INT, "Raster: number of varying dbscans" });

    res->add( { "RASTER_CLASSIFY_SCANSERIES_ADD",
                &RASTER_CLASSIFY_SCANSERIES_ADD,
                raster, MIN_REAL, MAX_REAL,
                "Raster: value added to epsilon in each round" });

    res->add( { "RASTER_CLASSIFY_NORMALIZE", &RASTER_CLASSIFY_NORMALIZE,
                raster, 0, 10, "Raster: std-deviation between 1 and this" });

    res->add( { "GET_RASTER_NOT_PC", &GET_RASTER_NOT_PC, raster,
        "set true analyzeRaster returns the raster not the points"});

    res->add( { "SWITCH_FEATURES", &SWITCH_FEATURES, raster,
      1, 127, "switch features for classify as binary number as int"});



    Param::CONTEXT geom = Param::CONTEXT::analyzeGeom;

    // the TYPICAL_POINT_DISTANCE can be retrieved automatically
    // res->add( { "TYPICAL_POINT_DISTANCE", &TYPICAL_POINT_DISTANCE, geom,
    //     0.0, MAX_REAL, "Geom: typical point distance in cloud" });

    res->add( { "MINIMUM_OBJECT_EXTENT", &MINIMUM_OBJECT_EXTENT, geom,
        0.001, MAX_REAL, "Geom: extent of small objects in cloud" });

    res->add( { "NEIGHBORHOOD_SIZE", &NEIGHBORHOOD_SIZE, geom,
        25, 1000, "Geom: size of neighborhoods used for dual point calc." });

    res->add( { "SOFT_DIFFUSION", &SOFT_DIFFUSION, geom,
        0.01, 10.0, "Geom: deviation of points from regular grid order"});

    res->add( { "ROUGH_DIFFUSION", &ROUGH_DIFFUSION, geom,
        0.001, 10.0, "Geom: how far points may stick out from the surface" });

    return res;
}
/*
This function is intended to be used for return values of
GetTupleResultType(Supplier s) from RelationCommon in VM.
It takes a name and a list and returns the 1-based index
of the Attribute, and 0 if the name was not found.
(Note: type-values as returned from the supplier are
have such shape:
((152 0) (EUCLID ((3 0) ((ObjID (1 0)) (CatID (1 0))))))
First of each thing might either represent a number in
Secondo Catalogue or a Symbol. But who knows.
Second might represent value - note that VMs typically
return 0 and value is probably nothing of particular
use to us in VM.)

*/
int findAttributeIndexInVM(std::string name, ListExpr attribute_list) {
    int j = 0;
    ListExpr rest = attribute_list;
    while (!nl->IsEmpty(rest)) {
        ListExpr current = nl->First(rest);
        j++;
        if (nl->IsEqual(nl->First(current), name)) {
            return j;
        }
        rest = nl->Rest(rest);
    }
    return 0;
}
