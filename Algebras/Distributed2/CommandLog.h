/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_DISTRIBUTED2_COMMANDLOG_H_
#define ALGEBRAS_DISTRIBUTED2_COMMANDLOG_H_

// use boost for thread handling

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include "Algebras/Standard-C++/LongInt.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

class TupleType;

namespace distributed2
{

class ConnectionInfo;

class CommandLog
{

private:
    struct LogEntry
    {

        LogEntry(void* _id,
                 const std::string& _server,
                 const std::string& _home,
                 const std::string& _query,
                 const double _runtime,
                 const int _errorCode) :
                id(_id),
                server(_server),
                home(_home),
                query(_query),
                runtime(_runtime),
                errorCode(_errorCode)
        {
        }

        void* id; // used connectioninfo
        std::string server;
        std::string home;
        std::string query;
        double runtime;
        int errorCode;
    };

public:

    /*
      Class CommandLog

     This class represents the command log.

     */

    CommandLog()
    {}

    void clear()
    {
        entries.clear();
    }

    void insert(ConnectionInfo* ci,
                const std::string& server,
                const std::string& home,
                const std::string& query,
                const double& runtime,
                const int errorCode)
    {
        boost::lock_guard < boost::mutex > gurad(mtx);
        entries.push_back(
                LogEntry((void*) ci, server, home, query, runtime, errorCode));
    }

    static ListExpr getTupleDescription()
    {
        ListExpr attrList = nl->SixElemList(
                nl->TwoElemList(nl->SymbolAtom("ConnectionId"),
                                listutils::basicSymbol<LongInt>()),
                nl->TwoElemList(nl->SymbolAtom("Server"),
                                listutils::basicSymbol<FText>()),
                nl->TwoElemList(nl->SymbolAtom("Home"),
                                listutils::basicSymbol<FText>()),
                nl->TwoElemList(nl->SymbolAtom("Command"),
                                listutils::basicSymbol<FText>()),
                nl->TwoElemList(nl->SymbolAtom("RunTime"),
                                listutils::basicSymbol<CcReal>()),
                nl->TwoElemList(nl->SymbolAtom("ErrorCode"),
                                listutils::basicSymbol<CcInt>()));
        return nl->TwoElemList(listutils::basicSymbol<Tuple>(), attrList);
    }

    class Iterator
    {
    public:
        Iterator(const CommandLog& log) :
                entries(log.entries), pos(0)
        {
            ListExpr numTuple = SecondoSystem::GetCatalog()->NumericType(
                    CommandLog::getTupleDescription());
            tt = new TupleType(numTuple);
        }

        ~Iterator()
        {
            tt->DeleteIfAllowed();
        }

        Tuple* nextTuple()
        {
            if (pos >= entries.size())
            {
                return 0;
            }
            Tuple* res = new Tuple(tt);
            res->PutAttribute(0, new LongInt(true, (int64_t) entries[pos].id));
            res->PutAttribute(1, new FText(true, entries[pos].server));
            res->PutAttribute(2, new FText(true, entries[pos].home));
            res->PutAttribute(3, new FText(true, entries[pos].query));
            res->PutAttribute(4, new CcReal(true, entries[pos].runtime));
            res->PutAttribute(5, new CcInt(true, entries[pos].errorCode));
            pos++;
            return res;
        }

    private:
        std::vector<LogEntry> entries;
        size_t pos;
        TupleType * tt;

    };

private:
    std::vector<LogEntry> entries;
    boost::mutex mtx;

};

} // namespace distributed2

#endif /* ALGEBRAS_DISTRIBUTED2_COMMANDLOG_H_ */
