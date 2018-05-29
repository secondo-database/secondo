/*
MMPoint.h
Created on: 03.05.2018
    Author: simon

*/

#ifndef ALGEBRAS_TEMPORAL2_MMPOINT_H_
#define ALGEBRAS_TEMPORAL2_MMPOINT_H_


#include "MemStorageManager.h"
#include "Attribute.h"
#include "GenericTC.h"

// should be removed once we do no longer depend on MPoint for In-/Out
#include "Algebras/Temporal/TemporalAlgebra.h"


namespace temporal2algebra {
using namespace std;

class MMPoint: public Attribute {

public:
    void memClear();
    void memAppend(const temporalalgebra::UPoint& upoint);
    Units memGet() const;

    MMPoint() {} // should do nothing

    MMPoint( const MMPoint& rhs ) : Attribute(false), id(0) {
        cout << "MMPoint(MMPoint& [" << rhs.id << "])\n";

        if (!rhs.IsDefined()) {
            cout << "Undefined\n";
            SetDefined(false);
            return;
        }

        SetDefined(true);
        Units units = rhs.memGet();
        if (units.empty()) {
           cout << "Defined, but empty - no id\n";
           return;       // defined, but empty. Check if in line with MPoint
        }

        MemStorageManager* storage = MemStorageManager::getInstance();
        id = storage->createId();
        cout << "MMPoint(MMPoint&)[" << id << "]\n";


        if (units.empty()) {
            SetDefined(false);
        } else {
            Units::iterator it;
            for (it=units.begin(); it != units.end(); ++it) {
                memAppend(*it);
            }
            SetDefined(true);
        }
    }

    explicit MMPoint(const bool defined): Attribute(defined), id(0) {
        cout << "MMPoint(" << defined << ")\n";
   // Do not allocate ID/Mem slot if no units are present:
   // - work around issue that first MMPoint is created without open DB
   // - save some resources

   //     MemStorageManager* storage = MemStorageManager::getInstance();
   //     id = storage->createId();
   //     cout << "MMPoint(bool).id:" << id << endl;
    }



    ~MMPoint() {
        cout << "~MMPoint["<< id << "]()\n";
       //This will not work! we need to overwrite Delete and Close function:
       // memClear();
    }

   MMPoint& operator=(const MMPoint& rhs){
      cout << "operator= [" << id << "], rhs[" << rhs.id << "]\n";

      if (&rhs == this) {
          return *this;
      }

      memClear();

      if( !rhs.IsDefined() ){
        SetDefined(false);
        return *this;
      }

      SetDefined(true);

      const Units& memUnits = rhs.memGet();
      if (memUnits.empty()) {
          return *this;
      }

      if (id==0) {
          MemStorageManager* storage = MemStorageManager::getInstance();
          id = storage->createId();
      }

      Units::const_iterator it;
      for (it = memUnits.begin(); it != memUnits.end(); ++it) {
         memAppend(*it);
      }

      return *this;
   }



   ListExpr ToListExpr(const ListExpr& typeInfo) const{

       cout << "MMPoint::ToListExpr[" << id << "]( .. )\n";
       cout << "IsDefined()=" << IsDefined() << endl;
       temporalalgebra::MPoint* mpoint = new temporalalgebra::MPoint(0);

       if (IsDefined()) {
             mpoint->Clear();
             mpoint->SetDefined(true);
             mpoint->StartBulkLoad();

             const Units& memUnits
                 = memGet();
             Units::const_iterator it;
             for (it = memUnits.begin(); it != memUnits.end(); ++it) {
                 mpoint->Add(*it);
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

   bool ReadFrom(const ListExpr LE, const ListExpr& typeInfo) {
     cout << "MMPoint::ReadFrom[" << id << "]( "
             + nl->ToString(LE) + ", .. )\n";
     if (listutils::isSymbolUndefined(LE)) {
       SetDefined(false);
       memClear();
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
     cout << "MMPoint::ReadFrom( .. )" << *mpoint << endl;
     if (mpoint->IsDefined()) {
         temporalalgebra::UPoint unit(false);

         int noOfComponents = mpoint->GetNoComponents();
         if (noOfComponents > 0 && id == 0) {
             MemStorageManager* storage = MemStorageManager::getInstance();
             id = storage->createId();
         }

         for ( int i = 0; i < noOfComponents; i++) {
             mpoint->Get(i, unit);
             memAppend(unit);
         }
         SetDefined(true);
     } else {
         SetDefined(false);
     }
     mpoint->DeleteIfAllowed();
     cout << "IsDefined()=" << IsDefined() << endl;
     return true;
   }

   std::string toString() const{
      std::stringstream ss;
      ss << id;
      return ss.str();
   }

   int Compare(const Attribute* rhs1) const{
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
      const MMPoint* rhs = static_cast<const MMPoint*>(rhs1);
      if(id < rhs->id){
         return -1;
      } else if(id  > rhs->id){
         return 1;
      }
      return 0;
   }

   bool Adjacent(const Attribute* rhs) const{
     return false;
   }

   size_t HashValue() const{
     return id;
   }

   void CopyFrom(const Attribute* attr) {
       cout << "MMPoint::CopyFrom[" << id
               <<"](" << (static_cast<const MMPoint*>(attr))->id << ")\n";
       operator=( *((MMPoint*) attr));
   }


   MMPoint* Clone() const{
     cout << "MMPoint::Clone[" << id <<"]()\n";
     return new MMPoint(*this);
   }

    size_t Sizeof() const { return sizeof(*this); }

    static const std::string BasicType(){
           return "mmpoint";
    }

    static const bool checkType(const ListExpr type){
         return listutils::isSymbol(type, BasicType());
    }

    static ListExpr Property(){
          return gentc::GenProperty("-> MAPPING",
                                    BasicType(),
                                   "( u1 ... un )",
                           "(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)) ...)");
    }

    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
       return nl->IsEqual(type,BasicType());
    }

private:
   MemStorageId id;
};


} /* namespace temporal2algebra */

namespace gentc {

    template<>
    void Delete<temporal2algebra::MMPoint>(const ListExpr typeInfo,Word &w);

    template<>
    void Close<temporal2algebra::MMPoint>(const ListExpr typeInfo, Word& w );
} /* namespace gentc */

#endif /* ALGEBRAS_TEMPORAL2_MMPOINT_H_ */
