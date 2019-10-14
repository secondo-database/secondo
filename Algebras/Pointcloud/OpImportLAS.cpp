/*
----
This file is part of SECONDO.

Copyright (C) 2018, University in Hagen, 
Facualty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include <string>
#include <assert.h>

#include "NestedList.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Stream.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Pointcloud/lasreader/lasreader.h"
#include "Algebras/Pointcloud/lasreader/laspoint.h"
#include "Algebras/Spatial/Point.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "QueryProcessor.h"
#include "Operator.h"

#include "OpImportLAS.h"


extern QueryProcessor* qp;


namespace pointcloud_importLAS {

/*
1 Operator importing a LAS-File

*/
template<class T>
ListExpr getAttr(std::string name){
  
  char f = name[0];
  if(f>='a' && f<='z'){
    f = f + 'A'-'a';
    name[0] = f;
  }

  return nl->TwoElemList( nl->SymbolAtom(name),
                          listutils::basicSymbol<T>());
}

void extendAttrList(ListExpr last, int pointType){


  last = nl->Append( last, getAttr<CcInt>("intensity"));


  // we support point formats 0,...,5
  if(pointType<0 || pointType>5){
    return;
  }  
  // append all members of point format 0
  last = nl->Append(last, getAttr<CcInt>("returnNumber"));
  last = nl->Append(last, getAttr<CcInt>("numberOfReturns"));
  last = nl->Append(last, getAttr<CcBool>("scanDirectionFlag"));
  last = nl->Append(last, getAttr<CcBool>("edgeOfFlightLine"));
  last = nl->Append(last, getAttr<CcInt>("classification"));
  last = nl->Append(last, getAttr<CcInt>("scan_angle_rank"));
  last = nl->Append(last, getAttr<CcInt>("user_data"));
  last = nl->Append(last, getAttr<CcInt>("point_source_id"));

  if(pointType==0){
    return;
  }
  if(pointType == 2){
    last = nl->Append(last, getAttr<CcInt>("red"));
    last = nl->Append(last, getAttr<CcInt>("green"));
    last = nl->Append(last, getAttr<CcInt>("blue"));
    return;
  }
  // append attributes of pointtype 1
  last = nl->Append(last, getAttr<CcReal>("gps_time"));
  if(pointType == 1){
    return;
  }
  if(pointType == 4){
     last = nl->Append(last, getAttr<CcInt>("wavePacketDescriptorIndex"));
     last = nl->Append(last, getAttr<CcInt>("byteOffsetToWaveformData"));
     last = nl->Append(last, getAttr<CcInt>("waveFormPacketSize"));
     last = nl->Append(last, getAttr<CcReal>("returnPointWaveformLocation"));
     last = nl->Append(last, getAttr<CcReal>("x_t"));
     last = nl->Append(last, getAttr<CcReal>("y_t"));
     last = nl->Append(last, getAttr<CcReal>("z_t"));
     return;
  }
  //append member of point type 3
  last = nl->Append(last, getAttr<CcInt>("red"));
  last = nl->Append(last, getAttr<CcInt>("green"));
  last = nl->Append(last, getAttr<CcInt>("blue"));
  if(pointType==3){
    return;
  }
  assert(pointType ==5);
  last = nl->Append(last, getAttr<CcInt>("wavePacketDescriptorIndex"));
  last = nl->Append(last, getAttr<CcInt>("byteOffsetToWaveformData"));
  last = nl->Append(last, getAttr<CcInt>("waveFormPacketSize"));
  last = nl->Append(last, getAttr<CcReal>("returnPointWaveformLocation"));
  last = nl->Append(last, getAttr<CcReal>("x_t"));
  last = nl->Append(last, getAttr<CcReal>("y_t"));
  last = nl->Append(last, getAttr<CcReal>("z_t"));
}


ListExpr importLASTM(ListExpr args){
   if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected"); 
   }
   ListExpr first = nl->First(args);
   if(!nl->HasLength(args,2)){ // SetUsesArgsInTypeMapping
      return listutils::typeError("internal error");
   }
   if(!FText::checkType(nl->First(first))){
     return listutils::typeError("first argument must be of type text");
   }
   ListExpr second = nl->Second(args);
   if(!nl->HasLength(second,2)){
     return listutils::typeError("internal error"); 
   }
   if(!CcBool::checkType(nl->First(second))){
     return listutils::typeError("second argument must be of type bool");
   }
   
   ListExpr fn = nl->Second(first);
   // support only constant names
   if(nl->AtomType(fn)!=TextType){
     return listutils::typeError("only constant filenames are supported yet");
     // TODO : implement evaluation of fn 
   }
   std::string file = nl->TextValue(fn);
   lasreader reader(file);
   if(!reader.isOk()){
     return listutils::typeError("Could not read file " + file);
   }  
   int pt = reader.getPointFormat();
   ListExpr attrList = nl->OneElemList( getAttr<Point>("Pos"));

   ListExpr last = attrList;
   
   last = nl->Append(last, getAttr<CcReal>("Alt"));

   extendAttrList(last, pt);

   return Stream<Tuple>::wrap( Tuple::wrap(attrList));
}


CcInt* getAttr(int i){ return new CcInt(true,i); }
CcInt* getAttr(uint8_t i){ return new CcInt(true,i); }
CcInt* getAttr(uint16_t i){ return new CcInt(true,i); }
CcInt* getAttr(uint32_t i){ return new CcInt(true,i); }
CcInt* getAttr(uint64_t i){ return new CcInt(true,i); }
CcInt* getAttr(char c){ return new CcInt(true,c); }
CcBool* getAttr(bool b){ return new CcBool(true,b); }
CcReal* getAttr(double d){ return new CcReal(true,d); }

template<typename T>
void extendTuple(Tuple* t, T v, size_t& offset){
   t->PutAttribute(offset, getAttr(v));
   offset++;
}


void fillTuple0( Tuple* t, lasPoint0* p, size_t& offset){
   extendTuple(t,p->returnNumber, offset);
   extendTuple(t,p->numberOfReturns, offset);
   extendTuple(t,p->scanDirectionFlag, offset);
   extendTuple(t,p->edgeOfFlightLine, offset);
   extendTuple(t,p->classification, offset);
   extendTuple(t,p->scan_angle_rank, offset);
   extendTuple(t,p->user_data, offset);
   extendTuple(t,p->point_source_id, offset);
}

void fillTuple2(Tuple* t, lasPoint2* p, size_t& offset){
   fillTuple0(t,p,offset);
   extendTuple(t,p->red, offset);
   extendTuple(t,p->green, offset);
   extendTuple(t,p->blue, offset);
}


void fillTuple1(Tuple* t, lasPoint1* p, size_t& offset){
  fillTuple0(t,p,offset);
  extendTuple(t,p->gps_time, offset);
}


void fillTuple4(Tuple* t, lasPoint4* p, size_t& offset){
     fillTuple1(t,p,offset);
     extendTuple(t,p->wavePacketDescriptorIndex, offset);
     extendTuple(t,p->byteOffsetToWaveformData, offset);
     extendTuple(t,p->waveFormPacketSize, offset);
     extendTuple(t,p->returnPointWaveformLocation, offset);
     extendTuple(t,p->x_t, offset);
     extendTuple(t,p->y_t, offset);
     extendTuple(t,p->z_t, offset);
}


void fillTuple3(Tuple* t, lasPoint3* p, size_t& offset){
   fillTuple1(t,p,offset);
   extendTuple(t,p->red, offset);
   extendTuple(t,p->green, offset);
   extendTuple(t,p->blue, offset);
}


void fillTuple5(Tuple* t, lasPoint5* p, size_t& offset){
     fillTuple3(t,p,offset);
     extendTuple(t,p->wavePacketDescriptorIndex, offset);
     extendTuple(t,p->byteOffsetToWaveformData, offset);
     extendTuple(t,p->waveFormPacketSize, offset);
     extendTuple(t,p->returnPointWaveformLocation, offset);
     extendTuple(t,p->x_t, offset);
     extendTuple(t,p->y_t, offset);
     extendTuple(t,p->z_t, offset);
}



class importLASInfo{

  public:
     importLASInfo(const std::string& _fn, bool _wgs, ListExpr _tt) :
        wgs(_wgs) {
        reader = new lasreader(_fn);
        tt = new TupleType(_tt);
        pointType = reader->isOk()? reader->getPointFormat():0;
     }

     ~importLASInfo(){
        delete reader;
        tt->DeleteIfAllowed();
     }

     Tuple* next(){
        if(!reader->isOk()){
           return 0;
        }
        lasPoint* p = reader->next();
        if(p==0){
           return 0;
        }
        double x,y,z;
        bool valid;
        if(wgs){
            reader->toLatLon(p,x,y,z,valid);
        } else {
            reader->getCoordinates(p,x,y,z,valid);
        }
        Tuple* res = createTuple(x,y,z,valid, p);
        delete p;
        return res;

     }

     Tuple* createTuple( const double x, const double y, const double z,
                         const bool valid, lasPoint* p){

        Tuple* res = new Tuple(tt);
        if(valid){
           res->PutAttribute(0, new Point(true,x,y));
           res->PutAttribute(1, new CcReal(true,z));
        } else {
           res->PutAttribute(0, new Point(true,x,y));
           res->PutAttribute(1, new CcReal(true,z));
        }
        res->PutAttribute(2, new CcInt(p->Intensity));

        size_t offset = 3; // start filling tuple at offset 2
        switch(pointType){
           case 0 : fillTuple0(res, (lasPoint0*) p, offset); break;
           case 1 : fillTuple1(res, (lasPoint1*) p, offset); break;
           case 2 : fillTuple2(res, (lasPoint2*) p, offset); break;
           case 3 : fillTuple3(res, (lasPoint3*) p, offset); break;
           case 4 : fillTuple4(res, (lasPoint4*) p, offset); break;
           case 5 : fillTuple5(res, (lasPoint5*) p, offset); break;
        }
        return res;
     }

  private:
     bool wgs;
     lasreader* reader;
     int pointType;
     TupleType* tt;
};


int importLASVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {

    importLASInfo* info = (importLASInfo*) local.addr;
    switch(message){
      case OPEN :{ if(info) {
                     delete info;
                     local.addr = 0;
                  }
                  FText* fn = (FText*) args[0].addr;
                  CcBool* wgs = (CcBool*) args[1].addr;
                  if(fn->IsDefined() && wgs->IsDefined()){
                    local.addr = new importLASInfo(fn->GetValue(), 
                                               wgs->GetValue(),
                                               nl->Second(qp->GetNumType(s)));
                  }
                 }
                  return 0;
     case REQUEST: result.addr = info?info->next(): 0;
                   return result.addr?YIELD:CANCEL;
     case CLOSE : if(info){
                    delete info;
                    local.addr = 0;
                  }
                  return 0;
    }
    return -1;
}

OperatorSpec importLASSpec(
   "text x bool -> stream(tuple(X)) ",
   "filename importLAS[ UseWGS ] ",
   "Importes a las file as a tuple stream. "
   "The tuple type depends on the fpoint format "
   "used in the file.",
   "query 'pointcloud.las' importLAS[TRUE] count"
);



Operator importLASOp(
  "importLAS",
  importLASSpec.getStr(),
  importLASVM,
  Operator::SimpleSelect,
  importLASTM
);


} // end of namespace


Operator* getImportLASOp(){
   pointcloud_importLAS::importLASOp.SetUsesArgsInTypeMapping();
   return &pointcloud_importLAS::importLASOp;
}


