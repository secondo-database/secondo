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


//[$][\$]

*/

#ifndef PROGRESSLISTENER_H
#define PROGRESSLISTENER_H

#include "WorkerStatus.h"
#include "ProgressListener.h"
#include <string>
#include <regex>

ProgressListener::ProgressListener(std::shared_ptr<WorkerStatus> _ws)
{
    ws = _ws;
}

ProgressListener::~ProgressListener()
{
    //
}

bool ProgressListener::handleMsg(NestedList *nl, ListExpr list, int source)
{
    if (!nl->HasMinLength(list, 2))
    {
        return false;
    }

    int progressInPercent = calculateProgressInPercent(nl->ToString(list));
    if (ws)
    {
        ws->updateProgress(progressInPercent);
    }
    return true;
}

int ProgressListener::calculateProgressInPercent(std::string progress)
{
    size_t position = 0;
    std::string delimiter(" ");
    std::smatch match;
    std::regex expression("(\\d\\d|\\d|-\\d)\\s(\\d\\d\\d?)");
    if (std::regex_search(progress, match, expression))
    {
        position = match.str().find(delimiter);
        double current = std::stod(match.str().substr(0, position));
        double total = 
        std::stod(match.str().erase(0, delimiter.length() + position));
        if (total >= 0 && current > 0)
        {
            return (current / total) * 100;
        }
    }
    return 0;
}

#endif