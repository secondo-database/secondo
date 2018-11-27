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
//[&] [\&]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of the Notification [&] Monitoring handler

[toc]

1 NoMo class implementation

*/

#include "NoMo.h"
#include <boost/algorithm/string.hpp>

namespace continuousqueries {

/*
1.1 Constructor

Creates a new NoMo object.

*/


NoMo::NoMo(int id, std::string tupledescr, TcpClient* coordinationClient): 
    _coordinationClient(coordinationClient),
    _id(id),
    _tupledescr(tupledescr),
    _running(false),
    _basePort(coordinationClient->GetServerPort()),
    _tupleServer(coordinationClient->GetServerPort() + (id))
{
    _monitor = new Monitor(id, "nomo", "", coordinationClient, 
            0.5 * 60 * 1000, 100);
}

// Destroy
NoMo::~NoMo()
{
    if (_fakemailfile.is_open()) _fakemailfile.close();
    Shutdown();
}

// Initialize
void NoMo::Initialize()
{
    _fakemail = false;

    // start tuple server thread
    _tupleServerThread = std::thread(
        &TcpServer::Run, 
        &_tupleServer
    );

    // wait for the server to be started
    if (!_tupleServer.IsRunning()) 
    {
        int count = 0;
        LOG << "Waiting a maximum of 60 seconds for the tuple"
            << " receiving server to start... " << ENDL;

        while (!_tupleServer.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count = count + 100;
        }
        if (_tupleServer.IsRunning()) LOG << " Done!" << ENDL;
    }

    if (!_tupleServer.IsRunning()) return;

    LOG << "Done! Worker have to push hits "
        << "to this host on port "
        << std::to_string(_tupleServer.GetMasterPort()) << ENDL;

    // confirm specialization
    (void) _coordinationClient->Send(
        NoMoGenP::confirmspecialize(true)
    );

    // start tight loop in thread
    _notificationLoopThread = std::thread(
        &NoMo::NotificationLoop,
        this
    );

    _running = true;

    Run();
}

void NoMo::NotificationLoop()
{
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
            msg.valid=false;
        }
        
        lock.unlock();

        if (hasMsg && msg.valid && msg.cmd==WorkerGenP::hit()) {
            _monitor->startWorkRound();

            int tupleId = 0;
            std::string tupleString = "";
            std::string hitlist = "";

            std::vector<std::string> parts;
            boost::split(parts, msg.params, boost::is_any_of(
                std::string(1, ProtocolHelpers::seperator)
            ));
            
            try
            {
                tupleId = std::stoi(parts[0]);
                tupleString = parts[1];
                hitlist = parts[2];
            }
            catch(...)
            {
                tupleId = 0;
                LOG << "failed to extract tupleId, "
                    << "tupleString or hitlist" << ENDL;
            }

            if (tupleId) 
            {
                handleHit(tupleId, tupleString, hitlist);
            } else {
                _monitor->endWorkRound(0, 0, 0);
            }
        }
    
        _monitor->checkBatch();
    }
}

// Run
void NoMo::Run()
{
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    while (_running) {
        std::unique_lock<std::mutex> lock(_coordinationClient->mqMutex);

        hasMsg = _coordinationClient->mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_coordinationClient->messages.empty();
        });

        if (!_running) {
            lock.unlock();
            continue;
        }
        
        if (hasMsg)
        {
            msg = ProtocolHelpers::decodeMessage(
                _coordinationClient->messages.front()
            );
            _coordinationClient->messages.pop();
        } else {
            msg.valid=false;
        }

        lock.unlock();
        
        if (hasMsg && msg.valid) {
            // get a new user query
            if (msg.cmd == CoordinatorGenP::addquery(0, "", "", "", false)) 
            {
                int queryId = 0;
                std::string function = "";
                std::string userhash = "";
                std::string email = "";

                std::vector<std::string> parts;
                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    queryId  = std::stoi(parts[0]);
                    function = parts[1];
                    userhash = parts[2];
                    email    = parts[3];
                }
                catch(...)
                {
                    queryId = 0;
                    LOG << "failed to extract id, query or email" << ENDL;
                }

                if (queryId) addUserQuery(queryId, function, userhash, email);
            } else

            // set fakemail
            if (msg.cmd == CoordinatorGenP::setfakemail())
            {
                LOG << "use f/F/0 for false, t/T/1 for std::cout only" << ENDL;
                LOG << "everything else will be handled as filename" << ENDL;
                
                if (msg.params.substr(0,1) == "F" || 
                    msg.params.substr(0,1) == "f" || 
                    msg.params.substr(0,1) == "0")
                {
                    LOG << "setting fakemail to 'false'" << ENDL;
                    _fakemail = false;
                } else {
                    _fakemail = true;
                    LOG << "setting fakemail to 'true'" << ENDL;

                    // save fakemails to file
                    if (msg.params.length() > 1)
                    {
                        std::cout << "Fake mails be saved in " 
                                  << msg.params << endl;
                        if (_fakemailfile.is_open()) _fakemailfile.close();
                        _fakemailfile.open(msg.params);
                    } else {
                        std::cout << "std::cout only" << endl;
                    }
                }
            } else

            // force shutdown
            if (msg.cmd == CoordinatorGenP::shutdown() || 
                msg.cmd == "disconnected") 
            {
                std::cout << "shutting down due to " << msg.cmd 
                    << " " << msg.params << endl;
                
                _running = false;
            } 

            // unknown command
            else 
            {
                LOG << "No handler for command " << msg.cmd << "." << ENDL;
            }

        } else {
            if (hasMsg)
            {
                LOG << "Message '" << msg.cmd << "' is invalid..." << ENDL;
            }
        }   
    }
}

void NoMo::handleHit(int tupleId, std::string tupleString, 
    std::string hitlist) 
{
    std::string from = "ssp.dontreply@fernuni-hagen.de";
    std::string subject = "Your Query Was Successfull!";
    std::string message;
    std::string email;
    int sendMessages = 0;
    int queryId = 0;

    // iterate over hitlist
    std::vector<std::string> hits;
    boost::split(hits, hitlist, boost::is_any_of(","));

    for (std::vector<std::string>::iterator it = hits.begin(); 
                it != hits.end(); it++)
    {
        // get query Id
        try {
            queryId = std::stoi(*it);
        } catch(...) {
            tupleId = 0;
            LOG << "failed to extract queryId" << ENDL;
        }
        if (!tupleId) continue;

        // create message
        message  = "You wanted to get informed if the following query was ";
        message += "successfull: \n\n" + _queries[queryId].query + "\n\n";
        message += "The tuple \n\n"+tupleBinaryStringToRealString(tupleString);
        message += "\n\nfulfilled your conditions. \n\n";

        // inform all users about hits
        for (std::vector<std::string>::iterator iit = 
            _queries[queryId].emails.begin();
            iit != _queries[queryId].emails.end(); iit++)
        {
            email = *iit;
            if (sendEmail(from, email, subject, message)) sendMessages++;
        }
    }

    _monitor->endWorkRound(1, hits.size(), sendMessages);
}

bool NoMo::sendEmail(std::string from, std::string to, std::string subject, 
    std::string message)
{
    std::string logmsg = "*************************\n";
    logmsg += "To: " + to + "\n";
    logmsg += "Subject: " + subject + "\n";
    logmsg += "Message: " + message;
    logmsg += "*************************\n\n";

    LOG << logmsg;

    if (_fakemail) {
        if (_fakemailfile.is_open()) _fakemailfile << logmsg;
        return false;
    }

    std::string querystring = "query sendmail(";
    querystring += "'" + subject+ "', ";
    querystring += "'" + from + "', ";
    querystring += "'" + to + "', ";
    querystring += "'" + message + "', '');";

    Word resultword;
    SecParser parser;
    std::string exestring;
    int parseRes = 0;
    
    try {
        parser.Text2List(querystring, exestring);

        //  0 = success, 1 = error, 2 = stack overflow
        if (parseRes == 0) 
        {
            exestring = exestring.substr(7, exestring.length() - 9);

            if ( !QueryProcessor::ExecuteQuery(exestring, resultword) ) 
            {   
                LOG << "Error while executing send query." << ENDL;
                resultword.setAddr(0);
            }
        } else {
            LOG << "Error while parsing send query: " << parseRes << ENDL;
            resultword.setAddr(0);
        }
    } catch(const std::exception& e) {
        LOG << "Catched an error while executing send query..." << ENDL;
        resultword.setAddr(0);
    }

    if (resultword.addr == 0) return false;

    CcBool* result = (CcBool*) resultword.addr;

    if (!result->IsDefined()) return false;
    return result->GetValue();
}

std::string NoMo::tupleBinaryStringToRealString(std::string tupleString)
{
    ListExpr resulttype;
    nl->ReadFromString(_tupledescr, resulttype);

    ListExpr _tupleType = nl->OneElemList(
        SecondoSystem::GetCatalog()->NumericType(
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                resulttype
            )
        )
    );

    ListExpr resultTupleType = nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
        resulttype
    );

    ListExpr numResultTupleType = SecondoSystem::GetCatalog()
        ->NumericType(resultTupleType);

    TupleType* tt = new TupleType(numResultTupleType);
    Tuple* tuple  = new Tuple(tt);

    tuple->ReadFromBinStr(0, tupleString);

    ListExpr tupleValue;
    std::string message;

    tupleValue = tuple->Out(_tupleType);
    nl->WriteToString(message, tupleValue);

    tt->DeleteIfAllowed();
    tuple->DeleteIfAllowed();

    return message;
}

void NoMo::addUserQuery(int queryId, std::string query, 
    std::string userhash, std::string email)
{
    // check if query already exists
    if (_queries.find(queryId) == _queries.end()) 
    {
        // create query
        queryStruct toAdd;
        toAdd.id = queryId;
        toAdd.query = query;

        _queries.insert(std::pair<int, queryStruct>(queryId, toAdd));
    }

    // add email to list
    _queries[queryId].emails.push_back(email);

    LOG << email << " and " << (int)_queries[queryId].emails.size()-1 
        << " others will be informed when Query " 
        << queryId << " was hit." << ENDL;
}

// Shutdown
void NoMo::Shutdown()
{
    _running = false;

    _tupleServer.Shutdown();
    _tupleServerThread.join();
    _notificationLoopThread.join();
}

}
