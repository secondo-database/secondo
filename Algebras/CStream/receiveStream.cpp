/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of operation receiveStream.

[toc]

1 Operation receiveStream implementation

*/


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "RTuple.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "JTree.h"

#include "Algebras/IMEX/aisdecode.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "TestTuples.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

namespace cstream {
using namespace std;

/*
1.1.0 LocalInfoBase
The LocalInfo-Classes implement the receiveStream operator gets the data.
At the moment, only JSON and AIS data sources are supported but this 
modular approach would make wasy to implement other sources, like CSV or
XML files.
For implementing a new data source, the new LocalInfo<...> class mus only
offer a getNext() method. Also, the Value Mapping function would need 
small changes.

*/
class LocalInfoBase {
    public:
        virtual ~LocalInfoBase() {};
        virtual VTuple* getNext() = 0;
};

/*
1.1.1 LocalInfoJSON
Implements the use of JSON files as data sources.
The LocalInfoJSON class only splits a given file into single JSON-Objects.
The conversion is done by the JNode class, which builds a tree structure.
This decoupled approach allows the re-use of parsing an JSON-string for
Secondo.

*/
class LocalInfoJSON : public LocalInfoBase {
    public:
        LocalInfoJSON(CcString* arg): filename("") {
            if(arg->IsDefined()) {
                filename = arg->GetValue();
                fin.open(filename, ios::in);
            }
        }
        ~LocalInfoJSON() {}

        VTuple* getNext() {
            VTuple* res;

            try {

                if (!fin.is_open()) return NULL;

                std::string jsonString = getNextJsonString();

                LOG << ENDL << ENDL << jsonString << ENDL << ENDL;

                if (jsonString == "") return NULL;

                JNode* root = new JNode(jROOT, "", jsonString, NULL);
                root->buildTree();

                std::string tds = root->createTupleDescrString();

                LOG << ENDL << tds << ENDL;
                root->print();
                
                TupleDescr* td = NULL;
                try {
                    td = new TupleDescr(tds);
                } catch (SecondoException e) {
                    LOG << "LocalInfoJSON: getNext() - Error: " 
                        << e.msg() << ENDL;
                    return NULL;
                }

                Tuple* t = root->buildTupleTree();

#ifdef DEBUG_OUTPUT
t->Print(cout);
#endif

                res = new VTuple(t, td);

                root->deleteRec();
                delete root;
                root = NULL;
            } catch (int e) {
                LOG << "CStream::JsonGetNext - an exception # " 
                    << e << " # occurred." << ENDL;
                res = NULL;
            }

            return res;
        }

        std::string getNextJsonString() {
            std::string res = "";
            int bracketsCount = 0;
            bool inText = false;
            bool doEscape = false;
            char c;

            while (!fin.eof() ) {
                fin.get(c);

                // Every json object has to start with {.
                if ((c != '{') && (res.length()==0)) continue;

                switch (c) {
                    case '\\': {
                        doEscape = !doEscape;
                        break;
                    }
                    case '"': {
                        if (!(doEscape)) inText = !inText;
                        doEscape = false;
                        break;
                    }
                    case '{': {
                        if (!doEscape && !inText) bracketsCount++;
                        doEscape = false;
                        break;
                    }
                    case '}': {
                        if (!doEscape && !inText) bracketsCount--;
                        doEscape = false;
                        break;
                    }
                    default: {
                        doEscape = false;
                    }
                }

                res += c;
                
                if ((res.length() > 0) && (bracketsCount == 0)) break;
            }

            return res;
        }

    private:
        std::string filename;
        ifstream fin;
};

/*
1.1.2 LocalInfoAIS
Implements the use of AIS files as data sources.
Heavily builds on top of aisdecode.h.

*/
class LocalInfoAIS : public LocalInfoBase {
    public: 
        LocalInfoAIS(CcString* arg): filename("") {
            if(arg->IsDefined()) {
                filename = arg->GetValue();
                
                ifstream f(filename.c_str());
                if (f.good()) {
                    f.close();
                    aisd = new aisdecode::aisdecoder(filename);
                } else {
                    f.close();
                    aisd = NULL;
                }
            } else {
                aisd = NULL;
            }
            for(int i=0;i<27;i++){
               stdTupleTypes[i] = 0;
            }
        }
        ~LocalInfoAIS() {
            if (aisd) {
              delete aisd;
            }
            for( int i=0;i<27;i++) {
               if(stdTupleTypes[i])
                  stdTupleTypes[i]->DeleteIfAllowed();
            }
        }

        VTuple* getNext() {
            // No filename provided or file does not exist.
            if (aisd == NULL) return NULL;

            aisdecode::MessageBase* msg = aisd->getNextMessage();

            if (msg==0) return NULL;

            std::string tupleDescr = getTupleDescription(msg);

            TupleDescr* td = NULL;
            try {
                td = new TupleDescr(tupleDescr);
            } catch (SecondoException e) {
                LOG << "LocalInfoAIS: getNext() - Error: " << e.msg() << ENDL;
                return NULL;
            }

            Tuple* t = getTuple(td, msg);

            VTuple* vt = new VTuple(t, td);
            delete msg;

            return vt;
        }

        Tuple* getTuple(TupleDescr* td, aisdecode::MessageBase* msg) {

            int mt = msg->getType();
            if(!stdTupleTypes[mt-1]){
              stdTupleTypes[mt-1] = td->CreateTupleType();
            }

            TupleType* tt = stdTupleTypes[mt-1];
            Tuple* t = new Tuple(tt);

switch(msg->getType()) {
  case 1:
  case 2:
  case 3:{
      LOG << "Type 123" << ENDL;
      aisdecode::Message1_3* msg1_3 = static_cast<aisdecode::Message1_3*>(msg);
      t->PutAttribute(0, new CcInt(true, msg1_3->messageType));
      t->PutAttribute(1, new CcInt(true, msg1_3->repeatIndicator));
      t->PutAttribute(2, new CcInt(true, msg1_3->mmsi));
      t->PutAttribute(3, new CcInt(true, msg1_3->status));
      t->PutAttribute(4, new CcInt(true, msg1_3->rot));
      t->PutAttribute(5, new CcInt(true, msg1_3->sog));
      t->PutAttribute(6, new CcInt(true, msg1_3->accuracy));
      t->PutAttribute(7, new CcReal(true, msg1_3->longitude));
      t->PutAttribute(8, new CcReal(true, msg1_3->latitude));
      t->PutAttribute(9, new CcInt(true, msg1_3->cog));
      t->PutAttribute(10, new CcInt(true, msg1_3->heading));
      t->PutAttribute(11, new CcInt(true, msg1_3->second));
      t->PutAttribute(12, new CcInt(true, msg1_3->maneuver));
      t->PutAttribute(13, new CcInt(true, msg1_3->spare));
      t->PutAttribute(14, new CcInt(true, msg1_3->raim));
      t->PutAttribute(15, new CcInt(true, msg1_3->rstatus));
      break;}
  case 4:{
      LOG << "Type 4" << ENDL;
      aisdecode::Message4* msg4 = static_cast<aisdecode::Message4*>(msg);
      t->PutAttribute(0,  new CcInt(true, msg4->type));
      t->PutAttribute(1,  new CcInt(true, msg4->repeat));
      t->PutAttribute(2,  new CcInt(true, msg4->mmsi));
      t->PutAttribute(3,  new CcInt(true, msg4->year));
      t->PutAttribute(4,  new CcInt(true, msg4->month));
      t->PutAttribute(5,  new CcInt(true, msg4->day));
      t->PutAttribute(6,  new CcInt(true, msg4->hour));
      t->PutAttribute(7,  new CcInt(true, msg4->minute));
      t->PutAttribute(8,  new CcInt(true, msg4->second));
      t->PutAttribute(9,  new CcInt(true, msg4->fix));
      t->PutAttribute(10, new CcReal(true, msg4->longitude));
      t->PutAttribute(11, new CcReal(true, msg4->latitude));
      t->PutAttribute(12, new CcInt(true, msg4->epfd));
      t->PutAttribute(13, new CcInt(true, msg4->spare));
      t->PutAttribute(14, new CcInt(true, msg4->raim));
      t->PutAttribute(15, new CcInt(true, msg4->sotdma));
      break;}
  case 5:{
      LOG << "Type 5" << ENDL;
      aisdecode::Message5* msg5 = static_cast<aisdecode::Message5*>(msg);
      t->PutAttribute(0,  new CcInt(true, msg5->type));
      t->PutAttribute(1,  new CcInt(true, msg5->repeat));
      t->PutAttribute(2,  new CcInt(true, msg5->mmsi));
      t->PutAttribute(3,  new CcInt(true, msg5->ais_version));
      t->PutAttribute(4,  new CcInt(true, msg5->imo));
      t->PutAttribute(5,  new CcString(true, msg5->callSign));
      t->PutAttribute(6,  new CcString(true, msg5->vesselName));
      t->PutAttribute(7,  new CcInt(true, msg5->shipType));
      t->PutAttribute(8,  new CcInt(true, msg5->dimToBow));
      t->PutAttribute(9,  new CcInt(true, msg5->dimToStern));
      t->PutAttribute(10, new CcInt(true, msg5->dimToPort));
      t->PutAttribute(11, new CcInt(true, msg5->dimToStarboard));
      t->PutAttribute(12, new CcInt(true, msg5->epfd));
      t->PutAttribute(13, new CcInt(true, msg5->month));
      t->PutAttribute(14, new CcInt(true, msg5->day));
      t->PutAttribute(15, new CcInt(true, msg5->hour));
      t->PutAttribute(16, new CcInt(true, msg5->minute));
      t->PutAttribute(17, new CcInt(true, msg5->draught));
      t->PutAttribute(18, new CcString(true, msg5->destination));
      t->PutAttribute(19, new CcInt(true, msg5->dte));
      break;}
  case 9:{
      LOG << "Type 9" << ENDL;
      aisdecode::Message9* msg9 = static_cast<aisdecode::Message9*>(msg);
      t->PutAttribute(0, new CcInt(true, msg9->type));
      t->PutAttribute(1, new CcInt(true, msg9->repeat));
      t->PutAttribute(2, new CcInt(true, msg9->mmsi));
      t->PutAttribute(3, new CcInt(true, msg9->alt));
      t->PutAttribute(4, new CcInt(true, msg9->sog));
      t->PutAttribute(5, new CcInt(true, msg9->accuracy));
      t->PutAttribute(6, new CcReal(true, msg9->longitude));
      t->PutAttribute(7, new CcReal(true, msg9->latitude));
      t->PutAttribute(8, new CcInt(true, msg9->cog));
      t->PutAttribute(9, new CcInt(true, msg9->second));
      t->PutAttribute(10, new CcInt(true, msg9->reserved));
      t->PutAttribute(11, new CcInt(true, msg9->dte));
      t->PutAttribute(12, new CcInt(true, msg9->assigned));
      t->PutAttribute(13, new CcInt(true, msg9->raim));
      t->PutAttribute(14, new CcInt(true, msg9->radio));                    
      break;}
  case 12:{
      LOG << "Type 12" << ENDL;
      aisdecode::Message12* msg12 = static_cast<aisdecode::Message12*>(msg);
      t->PutAttribute(0, new CcString(true, msg12->omsg));
      t->PutAttribute(1, new CcInt(true, msg12->type));
      t->PutAttribute(2, new CcInt(true, msg12->repeat));
      t->PutAttribute(3, new CcInt(true, msg12->source_mmsi));
      t->PutAttribute(4, new CcInt(true, msg12->sequence_number));
      t->PutAttribute(5, new CcInt(true, msg12->dest_mmsi));
      t->PutAttribute(6, new CcInt(true, msg12->retransmit));
      t->PutAttribute(7, new CcString(true, msg12->text));          
      break;}
  case 14:{
      LOG << "Type 14" << ENDL;
      aisdecode::Message14* msg14 = static_cast<aisdecode::Message14*>(msg);
      t->PutAttribute(0, new CcString(true, msg14->omsg));
      t->PutAttribute(1, new CcInt(true, msg14->type));
      t->PutAttribute(2, new CcInt(true, msg14->repeat));
      t->PutAttribute(3, new CcInt(true, msg14->mmsi));
      t->PutAttribute(4, new CcString(true, msg14->text));          
      break;}
  case 18:{
      LOG << "Type 18" << ENDL;
      aisdecode::Message18* msg18 = static_cast<aisdecode::Message18*>(msg);
      t->PutAttribute(0,  new CcInt(true, msg18->type));
      t->PutAttribute(1,  new CcInt(true, msg18->repeat));
      t->PutAttribute(2,  new CcInt(true, msg18->mmsi));
      t->PutAttribute(3,  new CcInt(true, msg18->reserved1));
      t->PutAttribute(4,  new CcInt(true, msg18->sog));
      t->PutAttribute(5,  new CcInt(true, msg18->accuracy));
      t->PutAttribute(6,  new CcReal(true, msg18->longitude));
      t->PutAttribute(7,  new CcReal(true, msg18->latitude));
      t->PutAttribute(8,  new CcInt(true, msg18->cog));
      t->PutAttribute(9,  new CcInt(true, msg18->heading));
      t->PutAttribute(10, new CcInt(true, msg18->second));
      t->PutAttribute(11, new CcInt(true, msg18->reserved2));
      t->PutAttribute(12, new CcInt(true, msg18->cs));
      t->PutAttribute(13, new CcInt(true, msg18->display));
      t->PutAttribute(14, new CcInt(true, msg18->dsc));
      t->PutAttribute(15, new CcInt(true, msg18->band));
      t->PutAttribute(16, new CcInt(true, msg18->msg22));
      t->PutAttribute(17, new CcInt(true, msg18->assigned));
      t->PutAttribute(18, new CcInt(true, msg18->raim));
      t->PutAttribute(19, new CcInt(true, msg18->radio));
      break;}
  case 19:{
      LOG << "Type 19" << ENDL;
      aisdecode::Message19* msg19 = static_cast<aisdecode::Message19*>(msg);
      t->PutAttribute(0,  new CcInt(true, msg19->type));
      t->PutAttribute(1,  new CcInt(true, msg19->repeat));
      t->PutAttribute(2,  new CcInt(true, msg19->mmsi));
      t->PutAttribute(3,  new CcInt(true, msg19->reserved));
      t->PutAttribute(4,  new CcInt(true, msg19->sog));
      t->PutAttribute(5,  new CcInt(true, msg19->accuracy));
      t->PutAttribute(6,  new CcReal(true, msg19->longitude));
      t->PutAttribute(7,  new CcReal(true, msg19->latitude));
      t->PutAttribute(8,  new CcInt(true, msg19->cog));
      t->PutAttribute(9,  new CcInt(true, msg19->heading));
      t->PutAttribute(10, new CcInt(true, msg19->second));
      t->PutAttribute(11, new CcInt(true, msg19->reserved2));
      t->PutAttribute(12, new CcString(true, msg19->name));
      t->PutAttribute(13, new CcInt(true, msg19->shiptype));
      t->PutAttribute(14, new CcInt(true, msg19->dimToBow));
      t->PutAttribute(15, new CcInt(true, msg19->dimToStern));
      t->PutAttribute(16, new CcInt(true, msg19->dimToPort));
      t->PutAttribute(17, new CcInt(true, msg19->dimToStarboard));
      t->PutAttribute(18, new CcInt(true, msg19->epfd));
      t->PutAttribute(19, new CcInt(true, msg19->raim));
      t->PutAttribute(20, new CcInt(true, msg19->dte));
      t->PutAttribute(21, new CcInt(true, msg19->assigned));
      break;}
  case 24:{
      LOG << "Type 24" << ENDL;
      aisdecode::Message24* msg24 = static_cast<aisdecode::Message24*>(msg);
      t->PutAttribute(0,  new CcInt(true, msg24->type));
      t->PutAttribute(1,  new CcInt(true, msg24->repeat));
      t->PutAttribute(2,  new CcInt(true, msg24->mmsi));
      t->PutAttribute(3,  new CcInt(true, msg24->partno));
      t->PutAttribute(4,  new CcString(true, msg24->shipsname));
      t->PutAttribute(5,  new CcInt(true, msg24->shiptype));
      t->PutAttribute(6,  new CcInt(true, msg24->vendorid));
      t->PutAttribute(7,  new CcInt(true, msg24->model));
      t->PutAttribute(8,  new CcInt(true, msg24->serial));
      t->PutAttribute(9,  new CcString(true, msg24->callsign));
      t->PutAttribute(10, new CcInt(true, msg24->dimToBow));
      t->PutAttribute(11, new CcInt(true, msg24->dimToStern));
      t->PutAttribute(12, new CcInt(true, msg24->dimToPort));
      t->PutAttribute(13, new CcInt(true, msg24->dimToStarboard));
      t->PutAttribute(14, new CcInt(true, msg24->mothership_mmsi));
      break;}
}

            return t;
        }

        std::string getTupleDescription(aisdecode::MessageBase* msg) {
switch(msg->getType()) {
    case 1:
    case 2:
    case 3: return "((Type int)(RepeatIndicator int)(Mmsi int)"
        "(Status int)(Rot int)(Sog int)(Accuracy int)(Longitude real)"
        "(Latitude real)(Cog int)(Heading int)(Second int)(Maneuver int)"
        "(Spare int)(Raim int)(Rstatus int))";
    case 4: return "((Type int)(Repeat int)(Mmsi int)(Year int)(Month int)"
        "(Day int)(Hour int)(Minute int)(Second int)(Fix int)(Longitude real)"
        "(Latitude real)(Epfd int)(Spare int)(Raim int)(Sotdma int))";
    case 5: return "((Type int)(Repeat int)(Mmsi int)(Ais_version int)(Imo int)"
        "(CallSign string)(VesselName string)(ShipType int)(DimToBow int)"
        "(DimToStern int)(DimToPort int)(DimToStarboard int)"
        "(Epfd int)(Month int)(Day int)(Hour int)(Minute int)"
        "(Draught int)(Destination string)(Dte int))";
    case 9: return "((Type int)(Repeat int)(Mmsi int)(Alt int)(Sog int)"
        "(Accuracy int)(Longitude real)(Latitude real)(Cog int)(Second int)"
        "(Reserved int)(Dte int)(Assigned int)(Raim int)(Radio int))";
    case 12: return "((Omsg string)(Type int)(Repeat int)(Source_mmsi int)"
        "(Sequence_number int)(Dest_mmsi int)(Retransmit int)(Text string))";
    case 14: return "((Type int)(Repeat int)(Source_mmsi int)"
        "(Sequence_number int)(Dest_mmsi int)(Retransmit int)(Text string))"; 
    case 18: return "((Type int)(Repeat int)(Mmsi int)(Reserved1 int)(Sog int)"
        "(Accuracy int)(Longitude real)(Latitude real)(Cog int)(Heading int)"
        "(Second int)(Reserved2 int)(Cs int)(Display int)(Dsc int)(Band int)"
        "(Msg22 int)(Assigned int)(Raim int)(Radio int))";
    case 19: return "((Type int)(Repeat int)(Mmsi int)(Reserved int)(Sog int)"
        "(Accuracy int)(Longitude real)(Latitude real)(Cog int)(Heading int)"
        "(Second int)(Reserved2 int)(Name string)(Shiptype int)(DimToBow int)"
        "(DimToStern int)(DimToPort int)(DimToStarboard int)(Epfd int)"
        "(Raim int)(Dte int)(Assigned int))";
    case 24: return "((Type int)(Repeat int)(Mmsi int)(Partno int)"
        "(Shipsname string)(Shiptype int)(Vendorid int)(Model int)(Serial int)"
        "(Callsign string)(DimToBow int)(DimToStern int)(DimToPort int)"
        "(DimToStarboard int)(Mothership_mmsi int))";
    default: return "";
}
        }

    private:
        std::string filename;
        aisdecode::aisdecoder* aisd;
        TupleType* stdTupleTypes[27];
};


class LocalInfoTestTuple : public LocalInfoBase {
    public:
        LocalInfoTestTuple(CcString* arg) {
            tt = new TestTuples();

            whichTuple = "1";
            maxTuple = 5;
            countTuple = 0;

            if(arg->IsDefined()) {
                whichTuple = arg->GetValue();

                if (whichTuple.find("*") != std::string::npos) {
                    maxTuple = atoi(whichTuple.substr(0, 
                        whichTuple.find("*")).c_str());
                    whichTuple = whichTuple.substr(
                        whichTuple.find("*")+1, whichTuple.length());
                }
            }
        }
        ~LocalInfoTestTuple() {}

        VTuple* getNext() {
            if (countTuple == maxTuple) return NULL;
            countTuple++;

            if (whichTuple == "1") return tt->Create1();
            if (whichTuple == "1_1") return tt->Create1_1();
            if (whichTuple == "2") return tt->Create2();
            if (whichTuple == "2_1") return tt->Create2_1();
            if (whichTuple == "3") return tt->Create3();
            if (whichTuple == "3_1") return tt->Create3_1();
            if (whichTuple == "4") return tt->Create4();
            if (whichTuple == "4_1") return tt->Create4_1();
            if (whichTuple == "5") return tt->Create5();
            if (whichTuple == "5_1") return tt->Create5_1();

            return tt->Create1();
            
        }
    
    private:
        std::string whichTuple;
        int countTuple;
        int maxTuple;
        TestTuples* tt;
};

/*
1.2  TypeMapping of operator ~receiveStream~ 
Expects two strings, first the filename, then the format (currently 
AIS or JSON).

*/
ListExpr ReceiveStream_TM(ListExpr args) {
    // the list is coded as ( (<type> <query part>) (<type> <query part>) )
    #ifdef DEBUG_OUTPUT
    VTHelpers::PrintList("ReceiveStream_TM", args, 2);
    #endif

    if (nl->ListLength(args) != 2)
        return listutils::typeError("two argument expected");

    // 1. argument: filename (STRING)
    ListExpr arg1 = nl->First(nl->First(args));
    
    if (!CcString::checkType(arg1))
        return listutils::typeError("file name as string expected");

    // 2. argument: format (STRING)
    ListExpr arg2 = nl->First(nl->Second(args));

    if (!CcString::checkType(arg2))
        return listutils::typeError("file format as string expected");

    return nl->TwoElemList(listutils::basicSymbol<Stream<VTuple> >(),
                nl->OneElemList(listutils::basicSymbol<VTuple>()));
}

/*
1.2  ValueMapping of operator ~receiveStream~
Plain and simple ValueMapping. All real work is done by the getNext()
function of the corresponding LocalInfo<> Class.

*/
int ReceiveStream_VM(Word* args, Word& result, int message, 
    Word& local, Supplier s) {

    LOG << "ReceiveStream_VM: ";

    LocalInfoBase* li = (LocalInfoBase*) local.addr;

    switch(message) {

        case OPEN: {
            LOG << "OPEN" << ENDL;

            CcString* filename = (CcString*) args[0].addr;
            CcString* datatype = (CcString*) args[1].addr;
            
            std::string type = datatype->GetValue();
            boost::to_upper(type);

            if (li) {
                delete li;
            }

            if (type == "AIS") {
                LOG << "(AIS) File: " << filename->GetValue() << ENDL;
                local.addr = new LocalInfoAIS( filename );

            } else if (type == "JSON") {
                LOG << "(JSON) File: " << filename->GetValue() << ENDL;
                local.addr = new LocalInfoJSON( filename );
            }  else if (type == "TEST") {
                LOG << "(TEST) TestTuple: " << filename->GetValue() << ENDL;
                local.addr = new LocalInfoTestTuple( filename );
            } // else if (type == "CSV") {
              //  LOG << "(JSON) File: " << filename->GetValue() << ENDL;
              //  local.addr = new LocalInfoCSV( filename );
              // } <-- Add new allowed file types like this!
            
            return 0;
        }

        case REQUEST: {
            LOG << "REQUEST";
            if (!li) return CANCEL;

            VTuple* res = li->getNext();

            if (res) {
                LOG << " yield" << ENDL;
                result = res;
                return YIELD;
            }

            LOG << " cancel" << ENDL;
            result.addr = 0;
            return CANCEL;
        }

        case CLOSE: {
            LOG << "CLOSE" << ENDL;
            if (li) {
                delete li;
                local.addr = 0;
            }
            return 0;
        }

    }
    return 0;
}

/*
5.8.3 Specification of operator ~receiveStream~

*/
const std::string ReceiveStream_Spec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(text symbol)"
" -> (stream (vtuple))"
"</text--->"
"<text>receiveStream(_, _)</text--->"
"<text>Converts a file of JSON or AIS data to a stream of VTuples.</text--->"
"<text>receiveStream(\"somefile.json\", \"json\")</text--->"
") )";

/*
5.8.3 Definition of operator ~receiveStream~

*/
Operator receiveStream_Op(
    "receiveStream",
    ReceiveStream_Spec,
    ReceiveStream_VM,
    Operator::SimpleSelect,
    ReceiveStream_TM
);

} /* end of namespace */

