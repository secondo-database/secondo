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

*/

#ifndef DISTRIBUTE5_DINPUT_CONSUMER_H
#define DISTRIBUTE5_DINPUT_CONSUMER_H
#include "Attribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Array/ArrayAlgebra.h"
#include "SocketIO.h"
#include "Stream.h"
#include "Task.h"
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>

namespace distributed5
{
class DInputConsumer
{
public:
    DInputConsumer(DInputConsumer &&move);
    DInputConsumer(DInputConsumer &copy) = delete;
    DInputConsumer(distributed2::DArrayBase *dArray, ListExpr contentType);
    DInputConsumer(Word &stream);
    ~DInputConsumer();

    Task *request();

    static ListExpr getContentType(ListExpr darrayType)
    {
        if (distributed2::DArray::checkType(darrayType))
        {
            return nl->Second(darrayType);
        }
        else if (distributed2::DFArray::checkType(darrayType))
        {
            return nl->TwoElemList(
                listutils::basicSymbol<distributed2::frel>(),
                nl->Second(nl->Second(darrayType)));
        }
        else
        {
            throw std::invalid_argument("darrayType is not an valid array");
        }
    }

private:
    const std::vector<distributed2::DArrayElement> *workers;
    size_t workerIndex;
    distributed2::DArrayBase *dArray;
    size_t index;
    Stream<Task> *stream;
    DataStorageType storageType;
    ListExpr contentType;
};
} // namespace distributed5

#endif
