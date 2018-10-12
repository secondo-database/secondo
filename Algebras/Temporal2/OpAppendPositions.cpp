/*
implementation of operator appendpositions

*/

#include "OpAppendPositions.h"

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "MPoint2.h"

#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/OrderedRelation/OrderedRelationAlgebra.h"

#include "MovingCalculations.h"

#include <sstream>


extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace temporalalgebra;

namespace temporal2algebra{

struct AppendPositionsInfo : OperatorInfo {
    AppendPositionsInfo() : OperatorInfo() {
        name =      "appendpositions";
        signature = "stream(tuple1) x relation(tuple2)"
                "x text x text x text (x int)"
                " -> stream(tuple)";
        syntax =    "<stream(tuple1)> x <relation(tuple2)> appendpositions"
                " ['<tid-src-attr>', '<pos-src-attr>', '<pos-dst-attr>',"
                " <transaction_mode>]";
        meaning =   "Appends the positions from tuple1.pos-src-attr to"
                " tuple2.pos-dst-attr where tuple2.tid = tuple1.tid-src-attr\n"
                "Choose transaction mode 1 for subcommits. ATTENTION: this will"
                " crash secondo when using the feed operator.";
    }
};

ListExpr AppendPositions_tm( ListExpr args ) {
    int numArgs = nl->ListLength(args);
    if (numArgs < 5 || numArgs > 6 ) {
        stringstream s;
        s << "expected 5 or 6 arguments, but got " << numArgs;
        return listutils::typeError(s.str());
    }

    ListExpr arg_tuplestreamType = nl->First(nl->First(args));
    ListExpr arg_relationName = nl->Second(args);
    ListExpr arg_srcTidAttr   = nl->Third(args);
    ListExpr arg_srcElemAttr  = nl->Fourth(args);
    ListExpr arg_dstM2Attr    = nl->Fifth(args);



    if(!listutils::isTupleStream(arg_tuplestreamType)){
        return listutils::typeError("first argument must be a tuple stream"
                ", but got " + nl->ToString(arg_tuplestreamType));
    }
    ListExpr tupAttributes = nl->Second(nl->Second(arg_tuplestreamType));

    if(!FText::checkType(nl->First(arg_relationName))) {
        return listutils::typeError("second argument must be of type "
                + FText::BasicType()
        + ", but got " + nl->ToString(nl->First(arg_srcElemAttr)));
    }

    std::string relationName = nl->Text2String(nl->Second(arg_relationName));

    SecondoCatalog* catalog = SecondoSystem::GetCatalog();
    if (!catalog->IsObjectName(relationName)) {
        return listutils::typeError("No secondo object found with name '"
                + relationName + "'");
    }

    ListExpr relationType = catalog->GetObjectTypeExpr(relationName);

    if(!listutils::isRelDescription(relationType) &&
            !listutils::isOrelDescription(relationType)){
        return listutils::typeError("second argument must be of type "
                + Relation::BasicType() + " or "
                + OrderedRelation::BasicType()
        + ", but got " + nl->ToString(relationType));
    }
    ListExpr relAttributes = nl->Second(nl->Second(relationType));

    if(!FText::checkType(nl->First(arg_srcTidAttr))) {
        return listutils::typeError("third argument must be of type "
                + FText::BasicType()
        + ", but got " + nl->ToString(nl->First(arg_srcTidAttr)));
    }
    std::string tupTidAttrToFindName =
            nl->Text2String(nl->Second(arg_srcTidAttr));

    if(!FText::checkType(nl->First(arg_srcElemAttr))) {
        return listutils::typeError("fourth argument must be of type "
                + FText::BasicType()
        + ", but got " + nl->ToString(nl->First(arg_srcElemAttr)));
    }
    std::string tupAttrToFindName =
            nl->Text2String(nl->Second(arg_srcElemAttr));

    if(!FText::checkType(nl->First(arg_dstM2Attr))) {
        return listutils::typeError("fifth argument must be of type "
                + FText::BasicType()
        + ", but got " + nl->ToString(nl->First(arg_dstM2Attr)));
    }
    std::string relAttrToFindName = nl->Text2String(nl->Second(arg_dstM2Attr));


    // find tid attr in incoming stream
    ListExpr tupTidAttrType;
    int tupTidAttrPos = listutils::findAttribute(
            tupAttributes , tupTidAttrToFindName , tupTidAttrType);

    if (tupTidAttrPos == 0) {
        return listutils::typeError("incoming stream does not "
                "contain attribute with name '" + tupTidAttrToFindName +"'");
    }

    if (TupleIdentifier::checkType(tupTidAttrType)) {
        cout << "this is what we really need\n";
    } else if (CcInt::checkType(tupTidAttrType)) {
        cout << "only temporarily accept int\n";
    } else {
        return listutils::typeError("expected attribute's '"
                + tupTidAttrToFindName
                + "' type in incoming stream to be "
                + TupleIdentifier::BasicType()
        + ", but was " + nl->ToString(tupTidAttrType));
    }

    // find unit/intime attr in incoming stream
    ListExpr tupAttrType;
    int tupAttrPos = listutils::findAttribute(
            tupAttributes , tupAttrToFindName , tupAttrType);

    if (tupAttrPos == 0) {
        return listutils::typeError("incoming stream does not "
                "contain attribute with name '" + tupAttrToFindName +"'");
    }

    std::string expectedDestAttrType;
    if (IPoint::checkType(tupAttrType)) {
        expectedDestAttrType = MPoint2::BasicType();
    } else if (CcInt::checkType(tupAttrType)) {
        expectedDestAttrType = CcInt::BasicType();
    } else {
        return listutils::typeError("expected attribute's '"
                + tupAttrToFindName
                + "' type in incoming stream to be any of "
                + IPoint::BasicType()
        + ", " + CcInt::BasicType()
        + ", but was " + nl->ToString(tupAttrType));
    }

    // find moving2 attr in relation
    ListExpr relAttrType;
    int relAttrPos = listutils::findAttribute(
            relAttributes , relAttrToFindName , relAttrType);

    if (relAttrPos == 0) {
        return listutils::typeError("target relation does not "
                "contain attribute with name '" + relAttrToFindName +"'");
    }

    if (nl->ToString(relAttrType) != expectedDestAttrType){
        return listutils::typeError("attribute '"
                + tupAttrToFindName + "' of type "
                + nl->ToString(tupAttrType)
                + " in stream requires attribute of type "
                + expectedDestAttrType + " in target relation, but '"
                + relAttrToFindName + "' is of type "
                + nl->ToString(relAttrType));
    }

    if (numArgs == 5) {
        return nl->ThreeElemList (
                nl->SymbolAtom ( Symbols::APPEND ()) ,
                nl->FourElemList (
                        nl->IntAtom(0), // dummy
                        nl->IntAtom(tupTidAttrPos),
                        nl->IntAtom(tupAttrPos),
                        nl->IntAtom(relAttrPos)) ,
                        arg_tuplestreamType);
    }

    // numArgs == 6
    if(!CcInt::checkType(nl->First(nl->Sixth(args)))) {
        return listutils::typeError("(optional) sixth argument must be of type "
                + CcInt::BasicType()
        + ", but got " + nl->ToString(nl->First(nl->Sixth(args))));
    }

    return nl->ThreeElemList (
            nl->SymbolAtom ( Symbols::APPEND ()) ,
            nl->ThreeElemList (
                    nl->IntAtom(tupTidAttrPos),
                    nl->IntAtom(tupAttrPos),
                    nl->IntAtom(relAttrPos)) ,
                    arg_tuplestreamType);
}


int AppendPositions_sf( ListExpr args ) {
    // TODO: is there a way to pass information from the type mapping?
    // How to select the correct version
    cout << "AppendPositions_sf: " << nl->ToString(args) << endl;
    return 0;
}

void sleepMode(int mode, int mode2, std::string message) {
    if (mode == mode2) {
        cout << "sleeping: '" << message << "'\n";
        sleep (10);
        cout << "sleeping done\n";
    }
}

void handleIpoint(const BackReference& backRef,
        Relation* relation,
        const IPoint* ipoint,
        const int transactionMode) {

    Word word;

    MemStorageId destMemId = MPoint2::findMemStorageId(backRef);

    // TODO: refactor to:
    // MemStorageId MPoint2::bringToMemory(backRef, commit_mode)
    if (destMemId <= 0) { // no such Moving im Memory, check relation
        Tuple* destTuple = relation->GetTuple(backRef.tupleId, true);
        if (destTuple != 0) {
            cout << "found tuple in relation: " << *destTuple  << endl;
            MPoint2* moving =  static_cast<MPoint2*>(
                    destTuple->GetAttribute(backRef.attrPos));
            cout << "also found MPoint2: " << *moving << endl;

            MPoint2* moving_for_update = moving->Clone();
            moving_for_update->SetBackReference(backRef);
            vector<int> changedIndices(1);
            changedIndices[0] = backRef.attrPos;
            vector<Attribute*> newAttrs(1);
            newAttrs[0] = moving_for_update;
            // it seems that UpdateTuple does not incr ref count
            // for newAttrs -> so do not delete(IfAllowed) moving_for_update.
            relation->UpdateTuple(destTuple,
                    changedIndices,
                    newAttrs);
            destTuple->DeleteIfAllowed();

            if(transactionMode == 1) {
                cout << "commiting....\n";
                SmiEnvironment::CommitTransaction();
                SmiEnvironment::BeginTransaction();
                cout << "...done\n";
            }

        }
    }

    if (destMemId <= 0) {
        cout << "no tuple found for '"
                << backRef << "'\n";
        return;
    }

    Unit finalUnit = MPoint2::getFinalUnit(destMemId);
    UPoint newUnit(0);
    CreateExtensionUnit<IPoint, UPoint>(
            &finalUnit,
            ipoint,
            &newUnit);
    if (newUnit.IsDefined()) {
        MPoint2::appendUnit(destMemId, newUnit);
    } else {
        cout << "couldn't create valid extension unit\n";
    }

}

// return copy of original tuple
int AppendPositionsStreamRegular_vm
( Word* args, Word& result, int message, Word& local, Supplier s )
{
    // 0:srcStream(tuple1) x 1:dstRelation(tuple2) x
    // 2:srcTidName x 3:srcUnitName x 4:dstMovingName x
    // 5:srcTidPos x 6:srcUnitPos x 7:dstMovingPos

    cout << "int AppendPositionsStreamRegular_vm\n";

    struct AppendPositions_LocalInfo{
        AppendPositions_LocalInfo():
            mode(0),
            srcTidPos(0),
            srcUnitPos(0),
            dstMovingPos(0),
            relationName(),
            relation(0) {};
        int mode;
        int srcTidPos;
        int srcUnitPos;
        int dstMovingPos;
        std::string relationName;
        Relation* relation;
    };

    Word elem;
    AppendPositions_LocalInfo *localInfo =
            static_cast<AppendPositions_LocalInfo*>(local.addr);
    Word t;

    bool defined; // only needed in OPEN
    Word word; // only needed in OPEN
    SecondoCatalog* catalog; // only needed in OPEN
    switch (message)
    {
    case OPEN :
        localInfo = new AppendPositions_LocalInfo();
        // we get indexes 1 based, but need them 0 based
        localInfo->mode =
                (static_cast<CcInt*>(args[5].addr))->GetIntval();
        localInfo->srcTidPos    =
                (static_cast<CcInt*>(args[6].addr))->GetIntval() -1;
        localInfo->srcUnitPos   =
                (static_cast<CcInt*>(args[7].addr))->GetIntval() -1;
        localInfo->dstMovingPos =
                (static_cast<CcInt*>(args[8].addr))->GetIntval() -1;

        // we checked the presence of the relation during TypeMapping
        // TODO: check if there is a way to ensure the Relation* stays valid
        localInfo->relationName =
                static_cast<FText*>(args[1].addr)->GetValue();
        catalog = SecondoSystem::GetCatalog();
        catalog->GetObject(localInfo->relationName, word, defined);
        localInfo->relation = static_cast<Relation*>(word.addr);

        local.setAddr(localInfo);

        qp->Open(args[0].addr);
        return 0;

    case REQUEST :
        if(!localInfo){ return CANCEL; }
        qp->Request(args[0].addr,t);
        if (qp->Received(args[0].addr))
        {

            Tuple* src_tuple = static_cast<Tuple*>(t.addr);
            cout << "src_tuple: " << src_tuple
                    << ", *src_tuple: " << *src_tuple << endl;

            TupleIdentifier* srcTid_ptr =
                    static_cast<TupleIdentifier*>(
                            src_tuple->GetAttribute(localInfo->srcTidPos));
            cout << "got Tuple identifier: " << srcTid_ptr->GetTid() << endl;

            IPoint* srcIPoint_ptr =
                    static_cast<IPoint*>(
                            src_tuple->GetAttribute(localInfo->srcUnitPos));
            cout << "got Unit identifier: " << *srcIPoint_ptr << endl;


            BackReference backRef = BackReference(
                    localInfo->relationName,
                    srcTid_ptr->GetTid(),
                    localInfo->dstMovingPos
            );
            cout << "backRef: " << backRef << endl;

            handleIpoint(backRef,
                    localInfo->relation,
                    srcIPoint_ptr,
                    localInfo->mode);

            //TODO: maybe it makes more sense to pass on the updated tuple
            // or even the original extended with the changes
            // (as in UpdateRelationAlgebra)

            // src_tup->DeleteIfAllowed(); // don't delete but - we pass it on
            result.setAddr(src_tuple);
            return YIELD;
        }
        else
            return CANCEL;

    case CLOSE :
        qp->Close(args[0].addr);
        if(localInfo)
        {
            delete localInfo;
            local.setAddr(0);
        }
        return 0;

    }
    cout << "unhandled message: " << message << endl;
    assert(false);
    return -1;
}


ValueMapping AppendPositions_vms[] =
{
        AppendPositionsStreamRegular_vm

        //  AppendPositions_vm<MBool, IBool>,
        //  AppendPositions_vm<MString, IString>,
        //  AppendPositions_vm<MInt, IInt>
        //  AppendPositions_vm<MReal, IReal>,
};

Operator* getAppendPositionsOpPtr() {
    Operator* op = new Operator(
            AppendPositionsInfo(),
            AppendPositions_vms,
            AppendPositions_sf,
            AppendPositions_tm
    );
    op->SetUsesArgsInTypeMapping();
    return op;
}

} // end of namespace temporal2algebra


