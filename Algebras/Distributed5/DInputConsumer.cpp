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

#include "DInputConsumer.h"

using namespace std;
using namespace distributed2;

namespace distributed5
{
/*
1 DInputConsumer 

1.1 Constructor

*/

DInputConsumer::DInputConsumer(DInputConsumer &&move)
{
    workers = move.workers;
    move.workers = 0;
    dArray = move.dArray;
    move.dArray = 0;
    stream = move.stream;
    move.stream = 0;
    workerIndex = move.workerIndex;
    index = move.index;
    storageType = move.storageType;
    contentType = move.contentType;
}

DInputConsumer::DInputConsumer(DArrayBase *dArray, ListExpr contentType)
{
    this->stream = 0;
    this->workers = &dArray->getWorker();
    workerIndex = 0;
    this->dArray = dArray;
    this->contentType = contentType;
    index = 0;
    storageType = DataStorageType::Object;
    if (dArray->getType() == arrayType::DFARRAY)
        storageType = DataStorageType::File;
}

DInputConsumer::DInputConsumer(Word &stream)
{
    this->workers = 0;
    this->dArray = 0;
    this->stream = new Stream<Task>(stream);
    this->stream->open();
}

DInputConsumer::~DInputConsumer()
{
    if (this->stream)
    {
        delete this->stream;
        this->stream = 0;
    }
}

/*
1.2 request Methode

*/
Task *DInputConsumer::request()
{
    if (workers)
    {
        Task *result = new WorkerTask((*workers)[workerIndex]);
        workerIndex++;
        if (workerIndex >= workers->size())
        {
            workers = 0;
        }
        return result;
    }
    if (dArray)
    {
        Task *result = new DataTask(
            dArray->getWorkerForSlot(index),
            dArray->getName(),
            index,
            storageType,
            contentType);
        result->setFlag(Output);
        index++;
        if (index >= dArray->getSize())
        {
            dArray = 0;
        }
        return result;
    }
    if (stream)
    {
        Task *result = stream->request();
        if (result == 0)
        {
            delete stream;
            stream = 0;
        }
        return result;
    }
    return 0;
}

} // namespace distributed5
