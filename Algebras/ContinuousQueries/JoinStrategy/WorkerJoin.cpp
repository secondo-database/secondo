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

[1] Implementation of the loop worker.

[toc]

1 WorkerJoin class implementation

*/

#include "WorkerJoin.h"
#include <boost/algorithm/string.hpp>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace continuousqueries {

/*
1.1 Constructor

Creates a new WorkerJoin object.

*/


WorkerJoin::WorkerJoin(int id, std::string attrliststr, 
    TcpClient* coordinationClient, std::string joincondition):
    WorkerGen::WorkerGen(id, attrliststr, coordinationClient)
{
    LOG << "WorlerJoin::Constructor" << ENDL;

    _type = "join";
    _joinCondition = joincondition;
    _queryattrlist = ProtocolHelpers::getQueryAttributes(attrliststr);
    _sc = SecondoSystem::GetCatalog();

    ListExpr tplattrs;
    nl->ReadFromString(attrliststr, tplattrs);
    _tupleattrlist = tplattrs;

    // Reset queries map to 0
    int count = _queryattrlist.length();
    std::string name = "";
    for(int i = 1; i <= count; i++)
    {
        name = _queryattrlist.elem(i).first().convertToString();
        _queryparts.push_back(std::make_pair(
            name,
            _queryattrlist.elem(i).second().convertToString()
        ));
        _queries[name] = 0;
    }
}

WorkerJoin::~WorkerJoin()
{
    // if (_tuplett) _tuplett->DeleteIfAllowed();
    // if (_querytt) _querytt->DeleteIfAllowed();
    // if (_queryrel) _tuplerel->DeleteAndTruncate();
    // if (_queryrel) _queryrel->DeleteAndTruncate();
}

void WorkerJoin::TightLoop()
{
    LOG << "WorkerJoin::TightLoop" << ENDL;
    
    // wait for new tuple
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    while (_running) {
        std::unique_lock<std::mutex> lock(_tupleServer.mqMutex);

        hasMsg = _tupleServer.mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_tupleServer.messages.empty();
        });

        if (!_running) {
            lock.unlock();
            continue;
        }
        
        if (hasMsg)
        {
            msg = ProtocolHelpers::decodeMessage(
                    _tupleServer.messages.front()
                );
            _tupleServer.messages.pop();
        } else {
            msg.valid = false;
        }
        
        lock.unlock();

        if (hasMsg && msg.valid && msg.cmd==StSuGenP::tuple()) {
            // extract informations
            int tupleId = 0;
            std::string tupleString = "";

            std::vector<std::string> parts;

            boost::split(parts, msg.params, boost::is_any_of(
                std::string(1, ProtocolHelpers::seperator)
            ));
            
            try
            {
                tupleId     = std::stoi(parts[0]);
                tupleString = parts[1];
            }
            catch(...)
            {
                tupleId = 0;
                LOG << "failed to extract id or tuple" << ENDL;
            }

            if (!tupleId) continue;

            LOG << tupleId << "|" << tupleString << ENDL;


            // Funktioniert und Shutdown geht.
            // Word w;
            // CcInt* val;

        // Funktioniert inkl Shutdown, wenn _querystring nicht ausgeführt wurde
            (void) executeQueryString("query tpl clear;");
            (void) executeQueryString("query relFromTupleBinStr('" + 
                _tupleattrlist.convertToString() + "', '" + tupleString + 
                "') feed tpl insert count");

            // (void) executeQueryString(_querystring);
            
            
            // w = 
            // val = (CcInt*) w.addr;
            // LOG << "add " << val->GetValue() << ENDL;
            // val->DeleteIfAllowed();
            // w.setAddr(0);

            // w = 
            // val = (CcInt*) w.addr;
            // LOG << "qs " << val->GetValue() << ENDL;
            // val->DeleteIfAllowed();
            // w.setAddr(0);

    // executeQueryString("query tpl feed count;");
    // LOG << "feed count geht" << ENDL;
    // executeQueryString("query tpl feed extract[I];");
    // LOG << "feed extract geht" << ENDL;

    // Funktioniert nicht mehr!
    // Relation* result = (Relation*) executeQueryString(_querystring).addr;
    // LOG << "Number of hits: " << result->GetNoTuples() << ENDL;

    //         // create tuple and insert it into tpl relation
    //         // Tuple* tuple  = new Tuple(_tuplett);
    //         // tuple->ReadFromBinStr(0, tupleString);
            
           
    //         // _tuplerel->AppendTuple(tuple);

    //         // Word tupleRelWord;
    //         // tupleRelWord.setAddr(_tuplerel);

    //         // _sc->UpdateObject("tpl", tupleRelWord);

    //         // execute query string
            

    //         // LOG << "Number of hits: " << result->GetNoTuples() << ENDL;

    //         std::string hitlist = "";
    //         bool anyhit = false;

    //         // tuple->DeleteIfAllowed();

    //         hitlist = hitlist.substr(0, hitlist.size()-1);

    //         // notify all nomos
    //         if (anyhit) notifyAllNoMos(tupleId, tupleString, hitlist);
        }
    }
}

void WorkerJoin::Initialize()
{
    LOG << "WorkerJoin::Initialize" << ENDL;

    Word tupleRelWord;
    Word queryRelWord;
    Word btreeWord;
    ListExpr typeInfo;
    ListExpr relType;

    _sc->DeleteObject("tpl");
    _sc->DeleteObject("queries");
    _sc->DeleteObject("qbtree");

    // Create the tpl object
    typeInfo = nl->TwoElemList(
        listutils::basicSymbol<Tuple>(), 
        _tupleattrlist.listExpr()
    );

    relType = nl->TwoElemList(listutils::basicSymbol<Relation>(), typeInfo);

    _tuplett = new TupleType(_sc->NumericType(typeInfo));
    _tuplerel = new Relation(_tuplett);
    
    tupleRelWord.setAddr(_tuplerel);
    _sc->InsertObject("tpl", "", relType, tupleRelWord, true);

    // Create the queries object
    typeInfo = nl->TwoElemList(
        listutils::basicSymbol<Tuple>(), 
        _queryattrlist.listExpr()
    );

    relType = nl->TwoElemList(listutils::basicSymbol<Relation>(), typeInfo);

    _querytt  = new TupleType(_sc->NumericType(typeInfo));
    _queryrel = new Relation(_querytt);

    queryRelWord.setAddr(_queryrel);
    _sc->InsertObject("queries", "", relType, queryRelWord, true);

    // Create the btree object
    relType = nl->ThreeElemList(
        nl->SymbolAtom(BTree::BasicType()), 
        typeInfo,
        nl->SymbolAtom(CcString::BasicType())
    );

    _qbtree = new BTree(SmiKey::String);

    btreeWord.setAddr(_qbtree);
    _sc->InsertObject("qbtree", "", relType, btreeWord, true);

    buildQueryString();
}

// Query handling
void WorkerJoin::addQuery(int id, std::string function)
{
    LOG << "addQuery " << id << ": " << function << ENDL;
    
    std::map<std::string, std::string> protofunction;

    for (std::vector<std::pair <std::string, std::string>>::iterator 
        it = _queryparts.begin(); 
        it != _queryparts.end(); it++)
    {
        protofunction[it->first] = "[const "+ it->second +" value undefined]";
    }

    ListExpr funlistexpr;
    nl->ReadFromString(function, funlistexpr);
    
    NList funlist = funlistexpr;
    
    protofunction["QID"] = std::to_string(id);

    for(int i = 1; i <= (int) funlist.length(); i++)
    {
        _queries[funlist.elem(i).first().convertToString()]++;
        protofunction[funlist.elem(i).first().convertToString()] =
            funlist.elem(i).second().convertToString();
    }

    std::string qs = "query queries inserttuple[";

    for (std::vector<std::pair <std::string, std::string>>::iterator 
        it = _queryparts.begin(); 
        it != _queryparts.end(); it++)
    {
        if (it != _queryparts.begin()) qs += ", ";
        qs += protofunction[it->first];
    }

    qs += "]";

    if (_joinCondition != "none")
        qs += " qbtree insertbtree[" + _joinCondition + "]";
    
    qs += " count";

    (void) executeQueryString(qs);

    buildQueryString();
}

Word WorkerJoin::executeQueryString(std::string querystring)
{
    LOG << "Executing: " << querystring << ENDL;

    Word resultword;
    SecParser parser;
    std::string exestring;
    int parseRes = 0;
    
    try
    {
        if (querystring.substr(0,1) != "(")
            parser.Text2List(querystring, exestring);

        LOG << exestring << ENDL;

        //  0 = success, 1 = error, 2 = stack overflow
        if (parseRes == 0) 
        {
            if (querystring.substr(0,1) == "q")
            exestring = exestring.substr(7, exestring.length() - 9);

            if ( !QueryProcessor::ExecuteQuery(exestring, resultword) ) 
            {   
                LOG << "Error while executing query." << ENDL;
                resultword.setAddr(0);
            }
        } else {
            LOG << "Error while parsing query: " << parseRes << ENDL;
            resultword.setAddr(0);
        }

        if (resultword.addr != 0) LOG << "Query success!!!" << ENDL;
    }
    catch(const std::exception& e)
    {
        LOG << "Catched an error while executing query..." << ENDL;
        resultword.setAddr(0);
    }

    return resultword;
}

std::string WorkerJoin::getRelationDescription(NList attrlist) 
{
    std::string result = "rel(tuple([";   
    int count = attrlist.length();
    
    for(int i = 1; i <= count; i++)
    {
        result += attrlist.elem(i).first().convertToString();
        result += " : ";
        result += attrlist.elem(i).second().convertToString();
        if (i != count) result += ", ";
    }

    result += "]))";

    return result;
}

std::string WorkerJoin::getUniqueId(int len)
{
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz";

    std::string s;

    for (int i = 0; i < len; ++i) {
        s = s + alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return s;
}

void WorkerJoin::showStatus()
{
    LOG << "WorkerJoin::Status" << ENDL;

    LOG << "Query Parts and their number in queries: " << ENDL;

    for (std::vector<std::pair <std::string, std::string>>::iterator 
        it = _queryparts.begin(); 
        it != _queryparts.end(); it++)
    {
        LOG << it->first << ": " << _queries[it->first] << ENDL;
    }

    LOG << ENDL << "***************************************" << ENDL << ENDL;
    LOG << "Current Query:" << ENDL << _querystring << ENDL;
}

void WorkerJoin::buildQueryString()
{
    LOG << "WorkerJoin::buildQueryString" << ENDL;

    std::vector<structQuerysort> qs;
    int grpcount1 = 0;
    int grpcount2 = 0;
    int grpcount3 = 0;
    int sum = 0;

    for (std::vector<std::pair <std::string, std::string>>::iterator 
        it = _queryparts.begin(); 
        it != _queryparts.end(); it++)
    {
        std::string first = it->first;

        if (first == _joinCondition) continue;
        if (first == "QID") continue;
        if (!_queries[first]) continue;

        structQuerysort elem;
        elem.tname = first.substr(0, first.length()-3);
        elem.qname = first;
        elem.count = _queries[first];
        elem.type = it->second;
        elem.unused = true;

        if (first.substr(first.length()-2) == "eq") {
            elem.comp = "=";
            if (it->second == "bool") {
                elem.group = 3;
                grpcount3++;
            } else {
                elem.group = 1;
                grpcount1++;
            }
        }
        if (first.substr(first.length()-2) == "gt") {
            elem.comp = "<";
            elem.group = 2;
            grpcount2++;
        }
        if (first.substr(first.length()-2) == "lt") {
            elem.comp = ">";
            elem.group = 2;
            grpcount2++;
        }

        LOG << "tname:" << elem.tname << " qname:" << elem.qname << " comp:" 
            << elem.comp << " type:" << elem.type 
            << " group:" << elem.group << " count:" << elem.count << ENDL;
        
        qs.push_back(elem);
    }

    LOG << "***" << grpcount1 << " " << grpcount2 << " " << grpcount3 << ENDL;

    std::string filterstring = "";
    std::vector<structQuerysort>::iterator index;
    sum = grpcount1 + grpcount2 + grpcount3;

    while (grpcount1>0) 
    {
        LOG << "1" << ENDL;
        index = qs.end();
        for (std::vector<structQuerysort>::iterator it = qs.begin(); 
             it != qs.end(); it++)
        {
            if (it->group == 1 && it->unused) {
                if (index == qs.end()) {
                    index = it;
                } else {
                    if (it->count > index->count) index = it;
                }
            }
        }

        filterstring += getFilterStringPart(*index) + " and ";
        index->unused = false;
        grpcount1--;
    }

    while (grpcount2>0) 
    {
        LOG << "2" << ENDL;
        index = qs.end();
        for (std::vector<structQuerysort>::iterator it = qs.begin(); 
             it != qs.end(); it++)
        {
            if (it->group == 2 && it->unused) {
                if (index == qs.end()) {
                    index = it;
                } else {
                    if (it->count > index->count) index = it;
                }
            }
        }

        filterstring += getFilterStringPart(*index) + " and ";
        index->unused = false;
        grpcount2--;
    }

    while (grpcount3>0) 
    {
        index = qs.end();
        LOG << "3" << ENDL;
        for (std::vector<structQuerysort>::iterator it = qs.begin(); 
             it != qs.end(); it++)
        {
            if (it->group == 3 && it->unused) {
                if (index == qs.end()) {
                    index = it;
                } else {
                    if (it->count > index->count) index = it;
                }
            }
        }

        filterstring += getFilterStringPart(*index) + " and ";
        index->unused = false;
        grpcount3--;
    }

    filterstring = filterstring.substr(0, filterstring.length()-5);

    _querystring = "query " + getJoinStringPart();

    if (sum>0) _querystring += " filter[" + filterstring + "]";

    _querystring += " consume;";

    // _querystring = "query qbtree queries exactmatch[\"January\"] consume;";
    // _querystring  = "query qbtree queries exactmatch[tpl ";
    // _querystring += "feed extract[S]] count;";
    LOG << ENDL << ENDL << "QueryString: " << _querystring << ENDL << ENDL;
}

std::string WorkerJoin::getFilterStringPart(structQuerysort elem)
{
    return "((." + elem.qname + " = [const " + elem.type +" value undefined])"+
        " or (." + elem.qname + " " + elem.comp + 
        " tpl feed extract[" + elem.tname + "]))";
}

std::string WorkerJoin::getJoinStringPart()
{
    if (_joinCondition == "none") return "queries";

    std::string tname = _joinCondition.substr(0, _joinCondition.length()-3);
    std::string comp = _joinCondition.substr(_joinCondition.length()-2);
    
    if (comp == "eq") comp = "exactmatch";
    // if (comp == "gt") comp = "<";
    // if (comp == "lt") comp = ">";

    return "qbtree queries " + comp + 
        "[tpl feed extract[" + tname + "]]";
}


} /* namespace */