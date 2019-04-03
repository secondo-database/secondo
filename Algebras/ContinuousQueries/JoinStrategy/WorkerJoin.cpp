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
    _type = "join";
    _joinCondition = joincondition;
    
    _queryattrlist = ProtocolHelpers::getQueryAttributes(attrliststr);
    _sc = SecondoSystem::GetCatalog();

    ListExpr tplattrs;
    nl->ReadFromString(attrliststr, tplattrs);
    _tupleattrlist = tplattrs;

    // Reset queries map to 0
    _noqueries = 0;
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

    _monitor = new Monitor(id, "join", _joinCondition, coordinationClient, 
        2 * 60 * 1000, 100);
}

/*
1.2 Destructor

Destroys the WorkerJoin object.

*/

WorkerJoin::~WorkerJoin()
{
}

/*
1.3 Initialize

Prepares the WorkerJoin object.

*/

void WorkerJoin::Initialize()
{
    Word tupleRelWord;
    Word queryRelWord;
    Word btreeWord;
    ListExpr typeInfo;
    ListExpr relType;

    // Create the tpl object
    _sc->DeleteObject("tpl");
    typeInfo = nl->TwoElemList(
        listutils::basicSymbol<Tuple>(), 
        _tupleattrlist.listExpr()
    );

    relType = nl->TwoElemList(listutils::basicSymbol<Relation>(), typeInfo);

    _tuplett = new TupleType(_sc->NumericType(typeInfo));
    _tuplerel = new Relation(_tuplett);
    
    tupleRelWord.setAddr(_tuplerel);
    _sc->InsertObject("tpl", "", relType, tupleRelWord, true);

bool recover = false;
if (!recover)
{
    // Normally create all elements fresh

    // Create the queries object
    _sc->DeleteObject("queries");
    typeInfo = nl->TwoElemList(
        listutils::basicSymbol<Tuple>(), 
        _queryattrlist.listExpr()
    );

    relType = nl->TwoElemList(listutils::basicSymbol<Relation>(), typeInfo);

    _querytt  = new TupleType(_sc->NumericType(typeInfo));
    _queryrel = new Relation(_querytt);

    queryRelWord.setAddr(_queryrel);
    _sc->InsertObject("queries", "", relType, queryRelWord, true);

    LOG << "***************************" << ENDL;
    LOG << "typeInfo: " << nl->ToString(typeInfo) << ENDL;
    LOG << "JoinCond: " << _joinCondition << ENDL;
    LOG << "***************************" << ENDL;

    // Create the btree object
    _sc->DeleteObject("qbtree");
    size_t jcPos = _queryattrlist.convertToString().find(_joinCondition);
    std::string jcType =  _queryattrlist.convertToString().substr(
        jcPos + _joinCondition.length() + 1,
        1
    );
    
    ListExpr jcTypeAtom = nl->TheEmptyList();
    SmiKey::KeyDataType jcTypeSmi=SmiKey::Unknown;

    if (jcType == "s")
    {
        jcTypeAtom = nl->SymbolAtom(CcString::BasicType());
        jcTypeSmi = SmiKey::String;
    } else
    if (jcType == "i")
    {
        jcTypeAtom = nl->SymbolAtom(CcInt::BasicType());
        jcTypeSmi = SmiKey::Integer;
    } else
    if (jcType == "r")
    {
        jcTypeAtom = nl->SymbolAtom(CcReal::BasicType());
        jcTypeSmi = SmiKey::Float;
    }

    relType = nl->ThreeElemList(
        nl->SymbolAtom(BTree::BasicType()), 
        typeInfo,
        jcTypeAtom
    );
    
    _qbtree = new BTree(jcTypeSmi);

    btreeWord.setAddr(_qbtree);
    _sc->InsertObject("qbtree", "", relType, btreeWord, true);

} else {
    // if recover is true, don't reset the btree and queries
    // use the prepared objects insteald

    // set the counter in the _queries object to the right number
    // iterare over all query parts
    std::string qf = "";
    CcInt* countnos;

    for (std::vector<std::pair <std::string, std::string>>::iterator 
            it = _queryparts.begin(); 
            it != _queryparts.end(); it++)
    {
        if (it->first == "QID") continue;
        
        qf = "query queries feed filter[not(." + it->first + 
                " = [const " + it->second + " value undefined])] count";
        countnos = (CcInt*) executeQueryString(qf).addr;

        _queries[it->first] = countnos->GetValue();
    }

    qf = "query queries count;";
    countnos = (CcInt*) executeQueryString(qf).addr;

    _noqueries = countnos->GetValue();
}

    _sc->CleanUp(false, true);
    
    _tuplerel = (Relation*) executeQueryString("query tpl;").addr;
    
    buildQueryString();
}

/*
1.4 TightLoop

Waits for new tuples and checks all registered queries against them.

*/

void WorkerJoin::TightLoop()
{
    // wait for new tuple
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    _monitor->startBatch();

    while (_running) {
        std::unique_lock<std::mutex> lock(_tupleServer.mqMutex);

        hasMsg = _tupleServer.mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_tupleServer.messages.empty();
        });

        if (!_running) {
            _monitor->checkBatch();
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

            _monitor->startWorkRound();

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
                std::cout << "failed to extract id or tuple" << endl;
            }

            if (!tupleId) {
                _monitor->endWorkRound(0, 0, 0);
                continue;
            }

            // (void) executeQueryString("query tpl clear;");

            // Relation* tplr = (Relation*) executeQueryString(
        //     "query relFromTupleBinStr('" + _tupleattrlist.convertToString()
        //      + "', '" + tupleString + "') feed tpl insert consume;").addr;

            // if (tplr->GetNoTuples() == 0)
            // {
            //     LOG << "No tuple in tpl! relFromTupleBinStr failed!" << ENDL;
            //     _monitor->endWorkRound(0, 0, 0);
            //     continue;
            // }

            // Beginn ALTERNATIVE

                Tuple* t = new Tuple(_tuplett);
                t->ReadFromBinStr(0, tupleString);

        // (void) executeQueryString("query tpl clear;");
        // Relation* trel = (Relation*) executeQueryString("query tpl;").addr;
                _tuplerel->Clear();

                std::this_thread::sleep_for(std::chrono::milliseconds(5));

                _tuplerel->AppendTuple(t);

            // Ende ALTERNATIVE

            Relation* result=(Relation*) executeQueryString(_querystring).addr;

            int hits = result->GetNoTuples();
            std::string hitlist = "";

            for(int i = 1; i <= hits; i++)
            {
                hitlist += result->GetTuple(i, true)->GetAttribute(0)->toText();
                hitlist += ",";
            }
            
            hitlist = hitlist.substr(0, hitlist.size()-1);

            std::cout << "Tpl: " << tupleId << " | Hits: " << hits << endl;

                t->DeleteIfAllowed();
                // result->Close();
                result->DeleteAndTruncate();

            // notify all nomos
            if (hits) notifyAllNoMos(tupleId, tupleString, hitlist);

            _monitor->endWorkRound(1, _noqueries, hits);
        }

        _monitor->checkBatch();
    }
}

/*
1.5 addQuery

Adds a new query.

*/

void WorkerJoin::addQuery(int id, std::string function)
{
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
    _noqueries++;
    buildQueryString();
}

/*
1.6 executeQueryString

Executes a query string. This doesn't work as expected.

*/

Word WorkerJoin::executeQueryString(std::string querystring)
{
    _sc->CleanUp(false, true);
    
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
                std::cout << "Error while executing query." << endl;
                resultword.setAddr(0);
            }
        } else {
            std::cout << "Error while parsing query: " << parseRes << endl;
            resultword.setAddr(0);
        }

        if (resultword.addr != 0) LOG << "Query success!!!" << ENDL;
    }
    catch(const std::exception& e)
    {
        std::cout << "Catched an error while executing query..." << endl;
        resultword.setAddr(0);
    }

    return resultword;
}

/*
1.7 getRelationDescription

Converts the attribut list in the string neccessary to create a relation
in a query.

*/

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

/*
1.8 showStatus

Prints out some information.

*/

void WorkerJoin::showStatus()
{
    std::cout << "**************************************************" << endl;
    std::cout << "WorkerJoin::Status"                         << endl << endl;

    std::cout << "Current Query:" << endl << _querystring             << endl;
    
    std::cout << "Query Parts and their number in queries: "          << endl;

    for (std::vector<std::pair <std::string, std::string>>::iterator 
        it = _queryparts.begin(); 
        it != _queryparts.end(); it++)
    {
        std::cout << it->first << ": " << _queries[it->first] << endl;
    }
    
    std::cout << "**************************************************" << endl;
    
}

/*
1.9 buildQueryString

Builds the query string based on the saved user queries.

*/

void WorkerJoin::buildQueryString()
{
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

        qs.push_back(elem);
    }

    std::string filterstring = "";
    std::vector<structQuerysort>::iterator index;
    sum = grpcount1 + grpcount2 + grpcount3;

    while (grpcount1>0) 
    {
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

    LOG << "QueryString: " << _querystring << ENDL;
}

/*
1.10 getFilterStringPart

Builds the filter part of the query string based on the given element.

*/

std::string WorkerJoin::getFilterStringPart(structQuerysort elem)
{
    return "((." + elem.qname + " = [const " + elem.type +" value undefined])"+
        " or (." + elem.qname + " " + elem.comp + 
        " tpl feed extract[" + elem.tname + "]))";
}

/*
1.11 getJoinStringPart

Builds the join part of the query string based on the join condition.

*/

std::string WorkerJoin::getJoinStringPart()
{
    if (_joinCondition == "none") return "queries";

    std::string tname = _joinCondition.substr(0, _joinCondition.length()-3);
    std::string comp = _joinCondition.substr(_joinCondition.length()-2);
    
    if (comp == "eq") comp = "exactmatch";
    if (comp == "gt") comp = "leftrange";
    if (comp == "lt") comp = "rightrange";

    return "qbtree queries " + comp + 
        "[tpl feed extract[" + tname + "]]";
}


} /* namespace */
