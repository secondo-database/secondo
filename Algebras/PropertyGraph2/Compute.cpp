/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

*/

#include "Compute.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "Utils.h"

#include "PropertyGraphMem.h"
#include "PropertyGraphQueryProcessor.h"
#include "QueryTree.h"

#include "SecondoInterfaceTTY.h"
#include "SecondoInterface.h"
#include "SecParser.h"
#include "QueryProcessor.h"

#include <fstream>


using namespace std;

namespace pgraph2 {

//----------------------------------------------------------------------------
Compute::Compute(PGraphQueryProcessor *pgp, QueryTree *tree, \
std::string pgstructure)
{
    pgprocessor=pgp;
    querytree=tree;
    structure=pgstructure;
}

//----------------------------------------------------------------------------
Compute::Compute(PGraphQueryProcessor *pgp,QueryTree *tree, \
   std::string pgstructure, std::string tmptuplestream)
{
    pgprocessor=pgp;
    querytree=tree;
    structure=pgstructure;
    tuplestream = tmptuplestream;
    withtuplestream=true;
}

//----------------------------------------------------------------------------
Compute::~Compute()
{
    for (uint i=0; i<messageslist.size(); i++)
    {
        delete messageslist.at(i);
    }

    if (pgprocessor!=NULL) delete pgprocessor;
    if (querytree!=NULL) querytree=NULL;
    if (_InputRelationIteratorPregel!=NULL) _InputRelationIteratorPregel=NULL;
}

//----------------------------------------------------------------------------
void Compute::CreateComputeFunction()
{

    SetOutputFields();
    SetFilterFields();

    CreateHeaderString();
    CreateProjectString();
    CreateResults();
    CreateInitialMessage();

    CreateMessages();
    std::string messagestring=ReadMessages();
    gesamtstring=headerstring+messagestring;

}

//----------------------------------------------------------------------------
void Compute::CreateResults()
{
    string outputf=GetOutputFieldsWithDatatype();
    resultstring="let Results = [const rel(tuple(["+outputf+"])) value ()];";
}

//----------------------------------------------------------------------------
void Compute::CreateInitialMessage()
{
    //Senderliste
    int counter = pgprocessor->poslist.size();
    string sender = "";
    for (int i=0; i<counter; i++)
    {
        stringstream ss;
        ss << i+1;
        string strcounter = ss.str();
        if (i == 0)
        {
            sender=sender+" Sender"+strcounter+": 0";
        }
        else
        {
            sender=sender+", Sender"+strcounter+": 0";
        }

    }


    string filterf="";
    std::vector<std::vector<std::string>> v;
    std::map<std::string, std::vector<std::vector<std::string>>>::iterator\
     itf = filterfields.find(querytree->Root->TypeName);
    if (!withtuplestream)
    {
    if (itf != filterfields.end())
    {
        v=filterfields[querytree->Root->TypeName];
        std::vector<std::vector<std::string>> tmpvector;
        std::vector<std::string> tmpvector2;
        uint vsize=v.size();
        for (int i=vsize-1; i>=0; i--)
        {
            bool samefilter=false;
            for (uint i2=0; i2<tmpvector.size();i2++)
            {
                if (v.at(i).at(1) == tmpvector.at(i2).at(0) && \
                    v.at(i).at(2) == tmpvector.at(i2).at(1)) samefilter=true;
            }

            if (!samefilter)
            {
                if (v.at(i).at(4) == "string" || v.at(i).at(4) == "text")
                {
                    filterf=filterf+"filter[."+v.at(i).at(1)+v.at(i).at(2)+"\""
                    +v.at(i).at(3)+"\"] ";
                }
                else
                {
                    filterf=filterf+"filter[."
                    +v.at(i).at(1)+v.at(i).at(2)+v.at(i).at(3)+"] ";
                }
                tmpvector2.erase(tmpvector2.begin(),tmpvector2.end());
                tmpvector2.push_back(v.at(i).at(1));
                tmpvector2.push_back(v.at(i).at(2));
                tmpvector.push_back(tmpvector2);
                if (v.size() > 1)
                {
                    v.erase(v.begin()+i);
                    filterfields[querytree->Root->TypeName]=v;
                }
                else
                {
                    filterfields.erase(querytree->Root->TypeName);
                    break;
                }
            }

        }
    }
    }


    string outputfi = GetOutputFieldsWithoutDatatype(true);

    RelationInfo *relinforoot = pgprocessor->pgraphMem->\
        RelRegistry.GetRelationInfo(querytree->Root->TypeName);
    if (!withtuplestream)
    {
    initialmessage="let InitialMessages = "+querytree->Root->TypeName
        +"P feed "+filterf+"projectextend[; NodeId: ."+relinforoot->IdFieldName
        +", Partition: part(."+relinforoot->IdFieldName
        +"), Message: \""+querytree->Root->TypeName+"1\","
        +sender+", "+outputfi+"] consume;\n\n\n";
    }
    else
    {

    static const size_t npos = -1;
    size_t pos;
    string initialtuple=tuplestream;
    string rest="";
    RelationInfo *relinfoinitial;
    pos = initialtuple.find(" ");

    while (pos != std::string::npos)
    {
    pos = initialtuple.find(" ");
    rest = initialtuple.substr(pos+1);
    initialtuple.erase(pos,npos);
    cout << "erster Tupelstrom: " << initialtuple << endl;

    relinfoinitial = pgprocessor->pgraphMem->\
        RelRegistry.GetRelationInfo(initialtuple);
    if (relinfoinitial != NULL) break;

    initialtuple = initialtuple.substr(0, initialtuple.size()-1);
    cout << "zweiter Tupelstrom: " << initialtuple << endl;
    relinfoinitial = pgprocessor->pgraphMem->\
        RelRegistry.GetRelationInfo(initialtuple);
    if (relinfoinitial != NULL) break;

    initialtuple=rest;
    cout << "dritter Tupelstrom: " << initialtuple << endl;
    pos = initialtuple.find(" ");
    }

    if (relinfoinitial == NULL)
    {
        throw PGraph2Exception("initial relation not found");
    }
    else
    {
    initialmessage="let InitialMessages = "+tuplestream
        +" projectextend[; NodeId: ."+relinfoinitial->IdFieldName
        +", Partition: part(."+relinfoinitial->IdFieldName
        +"), Message: \""+querytree->Root->TypeName+"1\","
        +sender+", "+outputfi+"] consume;\n\n\n";
    }
    }
}

//----------------------------------------------------------------------------
void Compute::CreateHeaderString()
{
    bool withdatatype=true;
    tuplestring=GetTupleString(withdatatype);
    headerstring="query fun (messages: stream(tuple(["+tuplestring+"]))) \n"
    "messages \n"
    "loopsel[fun(t: TUPLE)\n"
    "attr(t, Message) \n"
    "switch[ \n";

}

//----------------------------------------------------------------------------
void Compute::CreateProjectString()
{
    bool withdatatype=false;
    string tuplestringwithoutdatatype=GetTupleString(withdatatype);
    projectstring="project["+tuplestringwithoutdatatype+"] \n";
}

//----------------------------------------------------------------------------
std::string Compute::GetTupleString(bool withdatatype)
{
    string messagebegin="";
    if (withdatatype)
    {
        messagebegin="NodeId: int, Partition: int, Message: string,";
    }
    else
    {
        messagebegin="NodeId, Partition, Message,";
    }

    //Senderliste
    int counter = pgprocessor->poslist.size();
    string sender = "";
    for (int i=0; i<counter; i++)
    {
        stringstream ss;
        ss << i+1;
        string strcounter = ss.str();
        if (withdatatype)
        {
            if (i == 0)
            {
                sender=sender+" Sender"+strcounter+": int";
            }
            else
            {
                sender=sender+", Sender"+strcounter+": int";
            }
        }
        else
        {
            if (i == 0)
            {
                sender=sender+" Sender"+strcounter;
            }
            else
            {
                sender=sender+", Sender"+strcounter;
            }
        }
    }

    //value
    string messagevalues=", ";

    //OutputFields
    string outputf="";
    if (withdatatype)
    {
        outputf=GetOutputFieldsWithDatatype();
    }
    else
    {
        outputf=GetOutputFieldsWithoutDatatype(false);
    }

    string komplett = messagebegin+sender+messagevalues+outputf;

    return komplett;

}
//----------------------------------------------------------------------------
void Compute::SetOutputFields()
{
    for(auto&& outputfield:querytree->outputFields.Fields)
    {
        string alias=outputfield->NodeAlias;
        string relname="";
        //hole für jeden Alias den Relationenname
        for (auto&& aliasfield:querytree->AliasList.list)
        {
            if (aliasfield.AliasName==alias)
            {
            relname=aliasfield.TypeName;
            break;
            }
        }
        RelationInfo *relinfo=pgprocessor->pgraphMem->\
            RelRegistry.GetRelationInfo(relname);
        AttrInfo *ainfo=relinfo->RelSchema.\
            GetAttrInfo(outputfield->PropertyName);
        string attrtype=ainfo->TypeName;

        std::vector<string> relvector;
        relvector.push_back(outputfield->NodeAlias); //alias
        relvector.push_back(outputfield->OutputName); //outputname
        relvector.push_back(outputfield->PropertyName); //attributname
        relvector.push_back(attrtype); //attributtype

        std::map<std::string, std::vector<std::vector \
            <std::string>>>::iterator it;
        it = outputfields.find(relname);
        if (it != outputfields.end())
        {
        std::vector v = outputfields[relname];
        v.push_back(relvector);
        outputfields[relname]=v;
        }
        else
        {
        std::vector<std::vector<std::string>> v;
        v.push_back(relvector);
        outputfields[relname]=v;
        }
    }
}

//----------------------------------------------------------------------------
void Compute::SetFilterFields()
{
    string filteroperator="=";
    bool filterexists=false;
    std::vector<std::vector<std::string>> v;
    RelationInfo *relinfo;
    AttrInfo *ainfo;
    string attrtype="";
    std::map<std::string, std::vector<std::vector<std::string>>>::iterator it;
    for (uint i=0; i<pgprocessor->poslist.size(); i++)
    {
        QueryTreeEdge *filteredge = pgprocessor->poslist.at(i);
        std::vector<string> relvector;
        for (auto&& filterfield:filteredge->FromNode->Filters)
        {
            relinfo=pgprocessor->pgraphMem->RelRegistry.\
                GetRelationInfo(filteredge->FromNode->TypeName);
            ainfo=relinfo->RelSchema.GetAttrInfo(filterfield->Name);
            attrtype=ainfo->TypeName;
            filterexists=false;
            relvector.erase(relvector.begin(),relvector.end());
            relvector.push_back(filteredge->FromNode->Alias); //alias
            relvector.push_back(filterfield->Name); //attributname
            relvector.push_back(filteroperator);
            //Operator - Bei Nodefilter immer "="
            relvector.push_back(filterfield->Value); // filtervalue
            relvector.push_back(attrtype); //Attributtype

            it = filterfields.find(filteredge->FromNode->TypeName);
            if (it != filterfields.end())
            {
                v = filterfields[filteredge->FromNode->TypeName];
                for (uint i=0; i<v.size(); i++)
                {
                    if (relvector.at(1) == v.at(i).at(1) && \
                        relvector.at(2) == v.at(i).at(2) && \
                        relvector.at(3) == v.at(i).at(3))
                    {
                        filterexists=true;
                        break;
                    }
                }
                if (!filterexists) v.push_back(relvector);
            }
            else
            {
                v.push_back(relvector);
            }

            filterfields[filteredge->FromNode->TypeName]=v;
        }


        for (auto&& filterfield:filteredge->ToNode->Filters)
        {
            relinfo=pgprocessor->pgraphMem->RelRegistry.\
                GetRelationInfo(filteredge->ToNode->TypeName);
            ainfo=relinfo->RelSchema.GetAttrInfo(filterfield->Name);
            attrtype=ainfo->TypeName;
            filterexists=false;
            v.erase(v.begin(),v.end());
            //v durch ersten Durchlauf ggf. noch gesetzt.
            relvector.erase(relvector.begin(),relvector.end());
            relvector.push_back(filteredge->ToNode->Alias); //alias
            relvector.push_back(filterfield->Name); //attributname
            relvector.push_back(filteroperator);
            //Operator - Bei Nodefilter immer "="
            relvector.push_back(filterfield->Value); // filtervalue
            relvector.push_back(attrtype); //Attributtype

            it = filterfields.find(filteredge->ToNode->TypeName);
            if (it != filterfields.end())
            {
                v = filterfields[filteredge->ToNode->TypeName];
                for (uint i=0; i<v.size(); i++)
                {
                    if (relvector.at(1) == v.at(i).at(1) && \
                        relvector.at(2) == v.at(i).at(2) && \
                        relvector.at(3) == v.at(i).at(3))
                    {
                        filterexists=true;
                        break;
                    }
                }
                if (!filterexists) v.push_back(relvector);
            }
            else
            {
                v.push_back(relvector);
            }

            filterfields[filteredge->ToNode->TypeName]=v;
        }
    }

    for(auto&& filterfield:querytree->filterList.Fields)
    {
        string alias=filterfield->NodeAlias;
        string relname="";
        //hole für jeden Alias den Relationenname
        for (auto&& aliasfield:querytree->AliasList.list)
        {
            if (aliasfield.AliasName==alias)
            {
            relname=aliasfield.TypeName;
            break;
            }
        }
        relinfo=pgprocessor->pgraphMem->\
            RelRegistry.GetRelationInfo(relname);
        ainfo=relinfo->RelSchema.GetAttrInfo(filterfield->PropertyName);
        attrtype=ainfo->TypeName;

        std::vector<string> relvector;
        v.erase(v.begin(),v.end());
        //v durch ersten Durchlauf ggf. noch gesetzt.
        relvector.push_back(filterfield->NodeAlias); //alias
        relvector.push_back(filterfield->PropertyName); //attributname
        relvector.push_back(filterfield->Operator); //operator
        relvector.push_back(filterfield->FilterValue); //filtervalue
        relvector.push_back(attrtype); //attributtype

        it = filterfields.find(relname);
        if (it != filterfields.end())
        {
            v = filterfields[relname];
            v.push_back(relvector);
            filterfields[relname]=v;
        }
        else
        {
            v.push_back(relvector);
            filterfields[relname]=v;
        }
    }
}


//----------------------------------------------------------------------------
std::string Compute::GetOutputFieldsWithDatatype()
{
    string outputstring="";
    for (std::map<std::string, std::vector<std::vector<string>>>::iterator \
        it = outputfields.begin(); it != outputfields.end(); it++)
    {
        for (uint i=0; i<it->second.size(); i++)
        {
            if (i==0 && it==outputfields.begin())
            {
                outputstring=outputstring+it->second.at(i).at(1)
                    +": "+it->second.at(i).at(3);
            }
            else
            {
                outputstring=outputstring+", "+it->second.at(i).at(1)
                    +": "+it->second.at(i).at(3);
            }
        }
    }
    return outputstring;
}

//----------------------------------------------------------------------------
std::string Compute::GetOutputFieldsWithoutDatatype(bool initial)
{
    string outputstring="";
    if (!initial)
    {
        for (std::map<std::string, std::vector<std::vector<string>>>::iterator \
            it = outputfields.begin(); it != outputfields.end(); it++)
        {
            for (uint i=0; i<it->second.size(); i++)
            {
                if (i==0 && it==outputfields.begin())
                {
                    outputstring=outputstring+it->second.at(i).at(1);
                }
                else
                {
                    outputstring=outputstring+", "+it->second.at(i).at(1);
                }
            }
        }
    }
    else
    {
        for (std::map<std::string, std::vector<std::vector<string>>>::iterator \
            it = outputfields.begin(); it != outputfields.end(); it++)
        {
            for (uint i=0; i<it->second.size(); i++)
            {
                if (it->second.at(i).at(3) == "string")
                {
                    if (i==0 && it==outputfields.begin())
                    {
                        outputstring=outputstring
                            +it->second.at(i).at(1)+": \"\"";
                    }
                    else
                    {
                        outputstring=outputstring+", "
                            +it->second.at(i).at(1)+": \"\"";
                    }
                }
                if (it->second.at(i).at(3) == "int")
                {
                    if (i==0 && it==outputfields.begin())
                    {
                        outputstring=outputstring
                            +it->second.at(i).at(1)+": 0";
                    }
                    else
                    {
                        outputstring=outputstring
                            +", "+it->second.at(i).at(1)+": 0";
                    }
                }
                if (it->second.at(i).at(3) == "text")
                {
                    if (i==0 && it==outputfields.begin())
                    {
                        outputstring=outputstring
                            +it->second.at(i).at(1)+": ''";
                    }
                    else
                    {
                        outputstring=outputstring+", "
                            +it->second.at(i).at(1)+": ''";
                    }
                }
            }
        }

    }
    return outputstring;
}

//----------------------------------------------------------------------------
void Compute::CreateMessages()
{
    std::vector<bool> sendervec;
    std::map<std::string, std::vector<std::vector<std::string>>>* \
        outputs=&outputfields;
    std::map<std::string, std::vector<std::vector<std::string>>>* \
        filters=&filterfields;
    uint senderindex=0;
    int messagecounter=1;
    string actnodename=querytree->Root->TypeName;
    bool goback=false;
    bool last=false;
    for (uint posindex=0; posindex<pgprocessor->poslist.size(); posindex++)
    {
        QueryTreeEdge *actedge=pgprocessor->poslist.at(posindex);
        if (actedge->FromNode->TypeName == actnodename)
        {

                senderindex=sendervec.size()+1;
                Message *message=new Message(pgprocessor, \
                    actedge,outputs,filters,messagecounter, \
                    senderindex, structure, withtuplestream);
                message->CreateMessageString();
                messageslist.push_back(message);
                sendervec.push_back(true);
                actnodename=actedge->ToNode->TypeName;
                messagecounter++;

        }
        else
        {
            goback=true;
            for (senderindex=sendervec.size()-1; senderindex>=0; senderindex--)
            {
                if (sendervec.at(senderindex)) break;
            }
            Message *message=new Message(pgprocessor, \
                pgprocessor->poslist.at(senderindex),outputs,filters, \
                messagecounter, goback, senderindex+1, structure, \
                withtuplestream);
            message->CreateMessageString();
            messageslist.push_back(message);
            actnodename=pgprocessor->poslist.at(senderindex)->\
                FromNode->TypeName;
            messagecounter++;
            sendervec.at(senderindex)=false;
            posindex--;
            goback=false;
        }
    }
    last=true;
    Message *message=new Message(pgprocessor,actnodename, \
        outputs,filters,messagecounter, last, structure, withtuplestream);
    message->CreateMessageString();
    messageslist.push_back(message);

}

//----------------------------------------------------------------------------
std::string Compute::ReadMessages()
{
    std::string messagestring="";
    for (uint i=0; i<messageslist.size(); i++)
    {
        messagestring=messagestring+
            messageslist.at(i)->GetGesamtMessageString();
        if (messageslist.at(i)->lastmessage == true)
        {
            std::string tmpprojectstring="projectextend[; "+tuplestring+"] \n";
            ReplaceStringInPlace(tmpprojectstring," int","0");
            ReplaceStringInPlace(tmpprojectstring," string","\"\"");
            ReplaceStringInPlace(tmpprojectstring," text","''");
            messagestring=messagestring+tmpprojectstring;
        }
        else
        {
            messagestring=messagestring+projectstring;
        }
    }
    messagestring=messagestring+"; NoMessages()] \n"
    "] \n";
    return messagestring;
}

//---------------------------------------------------------------------------
int Compute::GetMaxId(string relname, string attrname)
{
   Word resultword;
   string querystring="(max (feed " +relname+ ")"+attrname+")";
   QueryProcessor::ExecuteQuery(querystring, resultword);
   CcInt* intzeiger = (CcInt*) resultword.addr;

   int relcount = intzeiger->GetIntval();
   delete intzeiger;
   return relcount;
}

//----------------------------------------------------------------------------
void Compute::CreateObjects()
{

    std::ofstream ofs;

    if (structure == "pregelmemory")
    {
        std::map<std::string, int>::iterator mapiterator;

        Relations[querytree->Root->TypeName] = 0;
        for (uint i=0; i<pgprocessor->poslist.size(); i++)
        {
            mapiterator = Edges.find(pgprocessor->poslist.at(i)->TypeName);
            if (mapiterator == Edges.end())
                Edges[pgprocessor->poslist.at(i)->TypeName] = 0;

            mapiterator = Relations.find(pgprocessor->\
                poslist.at(i)->ToNode->TypeName);
            if (mapiterator == Relations.end())
                Relations[pgprocessor->poslist.at(i)->ToNode->TypeName] = 0;
        }


        RelationInfo* relinfo=NULL;
        AttrInfo *ainfo;
        AttrInfo *ainfo2;
        ofs.open("creatememoryobjectspregellocal.sec", std::ofstream::out);

        for (mapiterator = Relations.begin(); mapiterator != Relations.end(); \
            mapiterator++)
        {
            relinfo=pgprocessor->pgraphMem->RelRegistry.\
                GetRelationInfo(mapiterator->first);
            ofs << "query isdefined(deleteObject(\""
                +mapiterator->first+"M\"));" << endl;
            ofs << "query isdefined(deleteObject(\""
                +mapiterator->first+"MI\"));" << endl;
            ofs << "let "+mapiterator->first+"M = "
                +mapiterator->first+" feed head[1] mconsume;" << endl;
            ofs << "let "+mapiterator->first+"MI = "+mapiterator->first
                +"M mcreateAVLtree["+relinfo->IdFieldName+"]; \n" << endl;
        }

        for (mapiterator = Edges.begin(); mapiterator != Edges.end(); \
            mapiterator++)
        {
            relinfo=pgprocessor->pgraphMem->RelRegistry.\
                GetRelationInfo(mapiterator->first);
            ainfo=relinfo->RelSchema.GetAttrInfo(0);
            ainfo2=relinfo->RelSchema.GetAttrInfo(1);
            int maxid = GetMaxId(relinfo->FromName,ainfo->Name);
            int maxid2 = GetMaxId(relinfo->ToName,ainfo2->Name);
            if (maxid >= maxid2)
            {
                Edges[mapiterator->first]=maxid+1;
            }
            else
            {
                Edges[mapiterator->first]=maxid2+1;
            }
            stringstream ss;
            ss << Edges[mapiterator->first];
            string globalmaxid = ss.str();
            //ss << relinfo->statistics->cardinality;
            //string strcardinality = ss.str();
            ofs << "query isdefined(deleteObject(\""
                +mapiterator->first+"M\"));" << endl;
            ofs << "query isdefined(deleteObject(\""
                +mapiterator->first+"_BM\"));" << endl;
            ofs << "let "+mapiterator->first+"M = "+mapiterator->first
                +" feed head[1] createmgraph3["+ainfo->Name+", "
                +ainfo2->Name+", Cost, "+globalmaxid+"];" << endl;
            ofs << "let "+mapiterator->first+"_BM = "+mapiterator->first
                +"_B feed head[1] createmgraph3["+ainfo2->Name+", "
                +ainfo->Name+", Cost, "+globalmaxid+"]; \n" << endl;
        }

        ofs.close();

        string executescriptlocal="(executeScript "
            "'creatememoryobjectspregellocal.sec' TRUE FALSE)";
        Word reswordlocal;
        QueryProcessor::ExecuteQuery(executescriptlocal,reswordlocal);
        CcBool* zeigerlocal = (CcBool*) reswordlocal.addr;
        delete zeigerlocal;

    }

    ofs.open("createobjectspregel.sec", std::ofstream::out);
   // f.open("createobjectspregel.sec", ios::out);
    ofs << "query isdefined(deleteObject(\"Results\"));" << endl;
    ofs << "query isdefined(deleteObject(\"ResultsMemory\"));" << endl;
    ofs << "query isdefined(deleteObject(\"InitialMessages\"));" << endl;
    ofs << "query isdefined(deleteObject(\"NoMessages\"));" << endl;
    ofs << "query isdefined(deleteObject(\"Compute\"));" << endl;
    ofs << "query isdefined(deleteObject(\"EndResults\"));" << endl;

    ofs << resultstring << "\n" << endl;
    ofs << "let ResultsMemory = Results feed mconsume;" << "\n" << endl;
    ofs << initialmessage << "\n" << endl;
    ofs.close();

    string executescript="(executeScript 'createobjectspregel.sec' TRUE FALSE)";
    Word resw;
    QueryProcessor::ExecuteQuery(executescript,resw);
    CcBool* zeiger = (CcBool*) resw.addr;
    delete zeiger;

    string nomessagelist="(fun (head (feed InitialMessages)0))";
    DoLet("NoMessages",nomessagelist);

    SecParser sp;
    string computelist="";
    sp.Text2List( gesamtstring, computelist );
    ReplaceStringInPlace(computelist, "(query ","");
    computelist = computelist.substr(0, computelist.size()-2);
    cout << "Compute: \n" << endl;
    cout << gesamtstring << endl;
    cout << computelist << endl;
    if (structure =="pregelmemory")
    {
        DoLetCompute("Compute",computelist);
    }
    else if (structure == "pregelpersistent")
    {
        DoLetCompute("Compute",computelist);
    }

}

//----------------------------------------------------------------------------
void Compute::ShareObjects()
{

    std::ofstream ofs;
    ofs.open("shareobjects.sec", std::ofstream::out);
   // f.open("createobjectspregel.sec", ios::out);
    ofs << "query share(\"Compute\", TRUE, Workers);" << endl;
    ofs << "query share(\"part\", TRUE, Workers);" << endl;
    ofs << "query share(\"NWorkers\", TRUE, Workers);" << endl;
    ofs << "query share(\"InitialMessages\", TRUE, Workers);" << endl;
    ofs << "query share(\"NoMessages\", TRUE, Workers);" << endl;
    ofs << "query share(\"Results\", TRUE, Workers);" << endl;
    ofs.close();

    string sharescript="(executeScript 'shareobjects.sec' TRUE FALSE)";
    Word resw;
    QueryProcessor::ExecuteQuery(sharescript,resw);
    CcBool* zeiger = (CcBool*) resw.addr;
    delete zeiger;

}
//----------------------------------------------------------------------------
void Compute::runPregelCommands()
{
    Word resultword;

    string setuppregel="(setupPregel Workers)";
    string remoteResMem="(remotePregelCommand 'let ResultsMemory"
        " = Results feed mconsume;')";
    string setpregelfunc="(setPregelFunction Compute Partition)";
    string initpregelmes="(initPregelMessages(feed InitialMessages))";
    string startpregel="(startPregel -1)";
    string remoteResUp="(remotePregelCommand 'update Results := "
        "ResultsMemory mfeed consume')";
    string dsummarize="(consume (dsummarize (createSDArray "
        "\"Results\" Workers)))";

    QueryProcessor::ExecuteQuery(setuppregel, resultword);
    QueryProcessor::ExecuteQuery(remoteResMem, resultword);

    if (structure == "pregelmemory")
    {
        string createremoteobj="(executeScript 'createremoteobjects.sec'"
            " TRUE FALSE)";
        Word reswremote;
        QueryProcessor::ExecuteQuery(createremoteobj,reswremote);
        CcBool* zeigerremote = (CcBool*) reswremote.addr;
        delete zeigerremote;
    }

    QueryProcessor::ExecuteQuery(setpregelfunc, resultword);
    QueryProcessor::ExecuteQuery(initpregelmes, resultword);
    QueryProcessor::ExecuteQuery(startpregel, resultword);
    QueryProcessor::ExecuteQuery(remoteResUp, resultword);

    CcBool* zeiger = (CcBool*) resultword.addr;
    delete zeiger;

}

//----------------------------------------------------------------------------
void Compute::runPregelCommandsSecond()
{
    Word resultword;

    string deleteResMem="(remotePregelCommand"
    "'query isdefined(deleteObject(\"ResultsMemory\"));')";
    string remoteResMem="(remotePregelCommand 'let ResultsMemory"
        " = Results feed mconsume;')";
    string setpregelfunc="(setPregelFunction Compute Partition)";
    string initpregelmes="(initPregelMessages(feed InitialMessages))";
    string startpregel="(startPregel -1)";
    string remoteResUp="(remotePregelCommand 'update Results := "
        "ResultsMemory mfeed consume')";
    string dsummarize="(consume (dsummarize (createSDArray "
        "\"Results\" Workers)))";

    QueryProcessor::ExecuteQuery(deleteResMem, resultword);
    QueryProcessor::ExecuteQuery(remoteResMem, resultword);

    if (structure == "pregelmemory")
    {
        string createremoteobj="(executeScript 'createremoteobjects.sec'"
            " TRUE FALSE)";
        Word reswremote;
        QueryProcessor::ExecuteQuery(createremoteobj,reswremote);
        CcBool* zeigerremote = (CcBool*) reswremote.addr;
        delete zeigerremote;
    }

    QueryProcessor::ExecuteQuery(setpregelfunc, resultword);
    QueryProcessor::ExecuteQuery(initpregelmes, resultword);
    QueryProcessor::ExecuteQuery(startpregel, resultword);
    QueryProcessor::ExecuteQuery(remoteResUp, resultword);

    CcBool* zeiger = (CcBool*) resultword.addr;
    delete zeiger;

}

//----------------------------------------------------------------------------
void Compute::getEndResults()
{
    std::ofstream ofs;
    ofs.open("createendresults.sec", std::ofstream::out);
    ofs << "let EndResults = createSDArray(\"Results\", Workers)"
        " dsummarize consume;";
    ofs.close();

    string endresults="(executeScript 'createendresults.sec' TRUE FALSE)";
    Word resw;
    QueryProcessor::ExecuteQuery(endresults,resw);
    CcBool* zeiger = (CcBool*) resw.addr;
    delete zeiger;


}

//----------------------------------------------------------------------------
void Compute::createRemoteObjectsFile()
{
    RelationInfo* relinfo=NULL;
    AttrInfo *ainfo;
    AttrInfo *ainfo2;
    std::ofstream ofs;
    ofs.open("createremoteobjects.sec", std::ofstream::out);
    std::map<std::string, int>::iterator mapiterator;

    ofs << "query remotePregelCommand('query meminit (1524)');" << endl;

    for (mapiterator = Relations.begin(); mapiterator != Relations.end(); \
        mapiterator++)
    {
        relinfo=pgprocessor->pgraphMem->RelRegistry.\
            GetRelationInfo(mapiterator->first);
        ofs << "query remotePregelCommand('let "+mapiterator->first
            +"M = "+mapiterator->first+" feed mconsume;');" << endl;
        ofs << "query remotePregelCommand('let "+mapiterator->first
            +"MI = "+mapiterator->first+"M mcreateAVLtree["
            +relinfo->IdFieldName+"];');" << endl;
    }

    for (mapiterator = Edges.begin(); mapiterator != Edges.end(); \
        mapiterator++)
    {
        relinfo=pgprocessor->pgraphMem->RelRegistry.\
            GetRelationInfo(mapiterator->first);
        ainfo=relinfo->RelSchema.GetAttrInfo(0);
        ainfo2=relinfo->RelSchema.GetAttrInfo(1);
        stringstream ss;
        ss << Edges[mapiterator->first];
        string globalmaxid = ss.str();
        //ss << relinfo->statistics->cardinality;
        //string strcardinality = ss.str();
        ofs << "query remotePregelCommand('let "+mapiterator->first
            +"M = "+mapiterator->first+" feed createmgraph3["
            +ainfo->Name+", "+ainfo2->Name+", Cost, "
            +globalmaxid+"];');" << endl;
        ofs << "query remotePregelCommand('let "+mapiterator->first
            +"_BM = "+mapiterator->first+"_B feed createmgraph3["
            +ainfo2->Name+", "+ainfo->Name+", Cost, "
            +globalmaxid+"];');" << endl;
    }

    ofs.close();
}

//----------------------------------------------------------------------------
void Compute::runPregel()
{
    CreateObjects();

    ShareObjects();
    if (structure == "pregelmemory")
        createRemoteObjectsFile();
    runPregelCommands();
    //getEndResults();
}

//----------------------------------------------------------------------------
void Compute::runPregelSecondTime()
{
    CreateObjects();

    ShareObjects();
    if (structure == "pregelmemory")
        createRemoteObjectsFile();
    runPregelCommandsSecond();
    //getEndResults();
}

//----------------------------------------------------------------------------
Tuple *Compute::ReadNextResultTuplePregel()
{
    Tuple* t = NULL;
    Word resultword;
    Relation* r = NULL;
    Tuple* resulttuple = new Tuple(pgprocessor->_OutputTupleType);
    string querystring="(consume (feed EndResults))";

    if (_InputRelationIteratorPregel==NULL)
    {
        QueryProcessor::ExecuteQuery(querystring, resultword);
        r = (Relation*) resultword.addr;
        _InputRelationIteratorPregel = r->MakeScan();
        t = _InputRelationIteratorPregel->GetNextTuple();
    }
    else
    {
        t = _InputRelationIteratorPregel->GetNextTuple();
    }

    if (t!=NULL)
    {
        int attrcounter=0;
        std::map<std::string, std::vector<std::vector<std::string>>>::iterator \
            it;
        for (it = outputfields.begin(); it != outputfields.end(); it++)
        {
            for (uint i=0; i<it->second.size(); i++)
            {
                 if (it->second.at(i).at(3) == "int")
                 {
                    resulttuple->PutAttribute(attrcounter, \
                        new CcInt( true, t->GetAttribute(attrcounter)));
                    attrcounter++;
                 }
                 if (it->second.at(i).at(3) == "string")
                 {
                    resulttuple->PutAttribute(attrcounter, \
                        new CcString( true, ((CcString*)t->\
                        GetAttribute(attrcounter))->GetValue()));
                    attrcounter++;
                 }
            }
        }
        return resulttuple;
    }
    else
    {
        delete r;
        _InputRelationIteratorPregel=NULL;
        return NULL;
    }
}


//----------------------------------------------------------------------------
Message::Message(PGraphQueryProcessor *pgp, QueryTreeEdge *qedge, \
    std::map<std::string, std::vector<std::vector<std::string>>>* outputs, \
    std::map<std::string, std::vector<std::vector<std::string>>>* filters, \
    int indx, bool goback, uint senderid, std::string pgstructure, \
    bool withtuplestream)
{
    pgprocess=pgp;
    edge=qedge;
    outputfieldszg=outputs;
    filterfieldszg=filters;
    messageindex=indx;
    gobackmessage=goback;
    gotosender=senderid;
    structure=pgstructure;
    tuplestream=withtuplestream;

    relinfofromnode=pgprocess->pgraphMem->RelRegistry.\
        GetRelationInfo(edge->FromNode->TypeName);
    relinfotonode=pgprocess->pgraphMem->RelRegistry.\
        GetRelationInfo(edge->ToNode->TypeName);
}

//----------------------------------------------------------------------------
Message::Message(PGraphQueryProcessor *pgp, std::string actnode, \
    std::map<std::string, std::vector<std::vector<std::string>>>* outputs, \
    std::map<std::string, std::vector<std::vector<std::string>>>* filters, \
    int indx, bool last, std::string pgstructure, bool withtuplestream)
{
    pgprocess=pgp;
    actnodename=actnode;
    outputfieldszg=outputs;
    filterfieldszg=filters;
    messageindex=indx;
    lastmessage=last;
    structure=pgstructure;
    tuplestream=withtuplestream;
}

//----------------------------------------------------------------------------
Message::Message(PGraphQueryProcessor *pgp, QueryTreeEdge *qedge, \
    std::map<std::string, std::vector<std::vector<std::string>>>* outputs, \
    std::map<std::string, std::vector<std::vector<std::string>>>* filters, \
    int indx, uint senderid, std::string pgstructure, bool withtuplestream)
{
    pgprocess=pgp;
    edge=qedge;
    outputfieldszg=outputs;
    filterfieldszg=filters;
    messageindex=indx;
    gofromsender=senderid;
    structure=pgstructure;
    tuplestream=withtuplestream;

    relinfofromnode=pgprocess->pgraphMem->RelRegistry.\
        GetRelationInfo(edge->FromNode->TypeName);
    relinfotonode=pgprocess->pgraphMem->RelRegistry.\
        GetRelationInfo(edge->ToNode->TypeName);
}

//----------------------------------------------------------------------------
Message::~Message()
{
    pgprocess=NULL;
    edge=NULL;
    outputfieldszg=NULL;
    filterfieldszg=NULL;
    relinfofromnode=NULL;
    relinfotonode=NULL;
}

//----------------------------------------------------------------------------
void Message::CreateMessageString()
{
    CreateMessageName();
    CreateMessageActualTupleString();
    if (!gobackmessage && !lastmessage) CreateMessageActualSuccessorString();
    CreateFilterString();
    CreateReplaceString();
    if (lastmessage) CreateLastMessageString();
}

//----------------------------------------------------------------------------
void Message::CreateMessageName()
{
    stringstream ss;
    ss << messageindex;
    string strmessageindex = ss.str();
    if (!lastmessage)
    {
        if (!gobackmessage)
        {
            if (messageindex == 1)
            {
                name="  \""+edge->FromNode->TypeName+strmessageindex+"\", \n";
            }
            else
            {
                name="; \""+edge->FromNode->TypeName+strmessageindex+"\", \n";
            }
        }
        else
        {
            name="; \""+edge->ToNode->TypeName+strmessageindex+"\", \n";
        }
    }
    else
    {
        name="; \""+actnodename+strmessageindex+"\", \n";
    }
}

//----------------------------------------------------------------------------
std::string Message::CreateNextMessageName()
{
    string nextmessage="";
    stringstream ss;
    ss << messageindex+1;
    string strmessageindex = ss.str();
    if (!lastmessage)
    {
        if (!gobackmessage)
        {
            nextmessage="\""+edge->ToNode->TypeName+strmessageindex+"\"";
        }
        else
        {
            nextmessage="\""+edge->FromNode->TypeName+strmessageindex+"\"";
        }
    }
    return nextmessage;
}

//----------------------------------------------------------------------------
void Message::CreateMessageActualTupleString()
{
    if (!lastmessage)
    {
        if (!gobackmessage)
        {
            if (structure=="pregelpersistent")
            {
                actualtuple="loopjoin["+edge->FromNode->TypeName
                    +" orange[.NodeId; .NodeId] {a}] \n";
            }
            else if (structure=="pregelmemory")
            {
                actualtuple="loopjoin["+edge->FromNode->TypeName+"MI "
                    +edge->FromNode->TypeName+"M mexactmatch[.NodeId] {a}] \n";
            }
        }
        else
        {
            if (structure=="pregelpersistent")
            {
                actualtuple="loopjoin["+edge->ToNode->TypeName
                    +" orange[.NodeId; .NodeId] {a}] \n";
            }
            else if (structure=="pregelmemory")
            {
                actualtuple="loopjoin["+edge->ToNode->TypeName+"MI "
                    +edge->ToNode->TypeName+"M mexactmatch[.NodeId] {a}] \n";
            }
        }
    }
    else
    {
        if (structure=="pregelpersistent")
        {
            actualtuple="loopjoin["+actnodename
                +" orange[.NodeId; .NodeId] {a}] \n";
        }
        else if (structure=="pregelmemory")
        {
           actualtuple="loopjoin["+actnodename+"MI "+actnodename
            +"M mexactmatch[.NodeId] {a}] \n";
        }
    }
}

//----------------------------------------------------------------------------
void Message::CreateMessageActualSuccessorString()
{
    if (edge->Reverse)
    {
        if (structure=="pregelpersistent")
        {
            actsuccessor="loopjoin["+edge->TypeName
                +"_B orange[.NodeId; .NodeId] {ab}] \n";
        }
        else if (structure=="pregelmemory")
        {
            actsuccessor="loopjoin["+edge->TypeName
                +"_BM mg3successors[.NodeId] {ab}] \n";
        }
    }
    else
    {
        if (structure=="pregelpersistent")
        {
            actsuccessor="loopjoin["+edge->TypeName
                +" orange[.NodeId; .NodeId] {ab}] \n";
        }
        else if (structure=="pregelmemory")
        {
            actsuccessor="loopjoin["+edge->TypeName
                +"M mg3successors[.NodeId] {ab}] \n";
        }
    }
}

//----------------------------------------------------------------------------
bool Message::CheckForOutputFields(std::string relname)
{
    std::map<std::string, std::vector<std::vector<std::string>>>::iterator \
        it = outputfieldszg->find(relname);
    if (it != outputfieldszg->end())
    {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
std::vector<std::vector<std::string>> \
    Message::GetOutputFields(std::string relname)
{
    return outputfieldszg->at(relname);
}

//----------------------------------------------------------------------------
bool Message::CheckForGlobalFilters(std::string relname)
{
    std::map<std::string, std::vector<std::vector<std::string>>>::iterator \
        it = filterfieldszg->find(relname);
    if (it != filterfieldszg->end())
    {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
std::vector<std::vector<std::string>> \
    Message::GetGlobalFilters(std::string relname)
{
    return filterfieldszg->at(relname);
}

//----------------------------------------------------------------------------
void Message::CreateFilterString()
{
    std::vector<std::vector<std::string>> filterv;

    if (!lastmessage)
    {
        if (!gobackmessage)
        {
            //Baue Filterstring zusammen
            if (tuplestream || messageindex>1)
            //Root rausgenommen, da Filter schon in InitialMessage
            {
                if (CheckForGlobalFilters(edge->FromNode->TypeName))
                {
                    filterv=GetGlobalFilters(edge->FromNode->TypeName);
                    std::vector<std::vector<std::string>> tmpvector;
                    std::vector<std::string> tmpvector2;
                    uint vsize=filterv.size();
                    for (int i=vsize-1; i>=0; i--)
                    {
                        bool samefilter=false;
                        for (uint i2=0; i2<tmpvector.size();i2++)
                        {
                        if (filterv.at(i).at(1) == tmpvector.at(i2).at(0) && \
                                filterv.at(i).at(2) == "=" && \
                                tmpvector.at(i2).at(1) == "=") samefilter=true;
                        }

                        if (!samefilter)
                        {
                            if (filterv.at(i).at(4) == "string" ||
                                filterv.at(i).at(4) == "text")
                            {
                                if (filterv.at(i).at(2) == "<>")
                                {
                                    filterstring=filterstring+"filter[not(."
                                        +filterv.at(i).at(1)+"_a"
                                        +filterv.at(i).at(2)+"\""
                                        +filterv.at(i).at(3)+"\")] \n";
                                }
                                else
                                {
                                    filterstring=filterstring+"filter[."
                                        +filterv.at(i).at(1)+"_a"
                                        +filterv.at(i).at(2)+"\""
                                        +filterv.at(i).at(3)+"\"] \n";
                                }
                            }
                            else
                            {
                                if (filterv.at(i).at(2) == "<>")
                                {
                                    filterstring=filterstring+"filter[not(."
                                        +filterv.at(i).at(1)+"_a"
                                        +filterv.at(i).at(2)
                                        +filterv.at(i).at(3)+")] \n";
                                }
                                else
                                {
                                    filterstring=filterstring+"filter[."
                                        +filterv.at(i).at(1)+"_a"
                                        +filterv.at(i).at(2)
                                        +filterv.at(i).at(3)+"] \n";
                                }
                            }
//filterstring=filterstring+"filter[."+filterv.at(i).at(1)+"_a = .Value] \n";
                            tmpvector2.erase(tmpvector2.begin(), \
                                tmpvector2.end());
                            tmpvector2.push_back(filterv.at(i).at(1));
                            tmpvector2.push_back(filterv.at(i).at(2));
                            tmpvector.push_back(tmpvector2);
                            if (filterv.size() > 1)
                            {
                                filterv.erase(filterv.begin()+i);
                                filterfieldszg->at(edge->FromNode->TypeName)=\
                                    filterv;
                            }
                            else
                            {
                              filterfieldszg->erase(edge->FromNode->TypeName);
                              break;
                            }
                        }
                    }
                }
            }
            if (CheckForGlobalFilters(edge->TypeName))
                {
                    filterv=GetGlobalFilters(edge->TypeName);
                    std::vector<std::vector<std::string>> tmpvector;
                    std::vector<std::string> tmpvector2;
                    uint vsize=filterv.size();
                    for (int i=vsize-1; i>=0; i--)
                    {
                        bool samefilter=false;
                        for (uint i2=0; i2<tmpvector.size();i2++)
                        {
                            if (filterv.at(i).at(1) == tmpvector.at(i2).at(0)\
                             && filterv.at(i).at(2) == "=" \
                             && tmpvector.at(i2).at(1) == "=") samefilter=true;
                        }

                        if (!samefilter)
                        {
                            if (filterv.at(i).at(4) == "string"||
                                filterv.at(i).at(4) == "text")
                            {
                                if (filterv.at(i).at(2) == "<>")
                                {
                                    filterstring=filterstring+"filter[not(."
                                    +filterv.at(i).at(1)+"_ab"
                                    +filterv.at(i).at(2)+"\""
                                    +filterv.at(i).at(3)+"\")] \n";
                                }
                                else
                                {
                                    filterstring=filterstring+"filter[."
                                    +filterv.at(i).at(1)+"_ab"
                                    +filterv.at(i).at(2)+"\""
                                    +filterv.at(i).at(3)+"\"] \n";
                                }
                            }
                            else
                            {
                                if (filterv.at(i).at(2) == "<>")
                                {
                                    filterstring=filterstring+"filter[not(."
                                    +filterv.at(i).at(1)+"_ab"
                                    +filterv.at(i).at(2)
                                    +filterv.at(i).at(3)+")] \n";
                                }
                                else
                                {
                                    filterstring=filterstring+"filter[."
                                    +filterv.at(i).at(1)+"_ab"
                                    +filterv.at(i).at(2)
                                    +filterv.at(i).at(3)+"] \n";
                                }

                            }
//filterstring=filterstring+"filter[."+filterv.at(i).at(1)+"_ab "
//+filterv.at(i).at(2)+" "+filterv.at(i).at(3)+"] \n";
                            tmpvector2.erase(tmpvector2.begin(), \
                                tmpvector2.end());
                            tmpvector2.push_back(filterv.at(i).at(1));
                            tmpvector2.push_back(filterv.at(i).at(2));
                            tmpvector.push_back(tmpvector2);
                            if (filterv.size() > 1)
                            {
                                filterv.erase(filterv.begin()+i);
                                filterfieldszg->at(edge->TypeName)=filterv;
                            }
                            else
                            {
                                filterfieldszg->erase(edge->TypeName);
                                break;
                            }
                        }
                    }
                }
        }
        else
        {
            //Baue Filterstring zusammen
            if (CheckForGlobalFilters(edge->ToNode->TypeName))
            {
                filterv=GetGlobalFilters(edge->ToNode->TypeName);
                std::vector<std::vector<std::string>> tmpvector;
                std::vector<std::string> tmpvector2;
                uint vsize=filterv.size();
                for (int i=vsize-1; i>=0; i--)
                {
                    bool samefilter=false;
                    for (uint i2=0; i2<tmpvector.size();i2++)
                    {
                        if (filterv.at(i).at(1) == tmpvector.at(i2).at(0) && \
                            filterv.at(i).at(2) == "=" && \
                            tmpvector.at(i2).at(1) == "=") samefilter=true;
                    }
                    if (!samefilter)
                    {
                        if (filterv.at(i).at(4) == "string"||
                            filterv.at(i).at(4) == "text")
                        {
                            if (filterv.at(i).at(2) == "<>")
                            {
                                 filterstring=filterstring+"filter[not(."
                                    +filterv.at(i).at(1)+"_a"
                                    +filterv.at(i).at(2)+"\""
                                    +filterv.at(i).at(3)+"\")] \n";
                            }
                            else
                            {
                                filterstring=filterstring+"filter[."
                                    +filterv.at(i).at(1)+"_a"
                                    +filterv.at(i).at(2)+"\""
                                    +filterv.at(i).at(3)+"\"] \n";
                            }
                        }
                        else
                        {
                            if (filterv.at(i).at(2) == "<>")
                            {
                                 filterstring=filterstring+"filter[not(."
                                    +filterv.at(i).at(1)+"_a"
                                    +filterv.at(i).at(2)
                                    +filterv.at(i).at(3)+")] \n";
                            }
                            else
                            {
                                filterstring=filterstring+"filter[."
                                    +filterv.at(i).at(1)+"_a"
                                    +filterv.at(i).at(2)
                                    +filterv.at(i).at(3)+"] \n";
                            }
                        }
  //filterstring=filterstring+"filter[."+filterv.at(i).at(1)+"_a = .Value] \n";
                        tmpvector2.erase(tmpvector2.begin(), \
                            tmpvector2.end());
                        tmpvector2.push_back(filterv.at(i).at(1));
                        tmpvector2.push_back(filterv.at(i).at(2));
                        tmpvector.push_back(tmpvector2);
                        if (filterv.size() > 1)
                        {
                            filterv.erase(filterv.begin()+i);
                            filterfieldszg->at(edge->ToNode->TypeName)=filterv;
                        }
                        else
                        {
                            filterfieldszg->erase(edge->ToNode->TypeName);
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        //Baue Filterstring zusammen
        if (CheckForGlobalFilters(actnodename))
        {
            filterv=GetGlobalFilters(actnodename);
            for (uint i=0; i<filterv.size(); i++)
            {
                if (filterv.at(i).at(4) == "string"||
                    filterv.at(i).at(4) == "text")
                {
                    if (filterv.at(i).at(2) == "<>")
                    {
                        filterstring=filterstring+"filter[not(."
                            +filterv.at(i).at(1)+"_a"
                            +filterv.at(i).at(2)+"\""
                            +filterv.at(i).at(3)+"\")] \n";
                    }
                    else
                    {
                        filterstring=filterstring+"filter[."
                            +filterv.at(i).at(1)+"_a"
                            +filterv.at(i).at(2)+"\""
                            +filterv.at(i).at(3)+"\"] \n";
                    }
                }
                else
                {
                    if (filterv.at(i).at(2) == "<>")
                    {
                        filterstring=filterstring+"filter[not(."
                            +filterv.at(i).at(1)+"_a"
                            +filterv.at(i).at(2)
                            +filterv.at(i).at(3)+")] \n";
                    }
                    else
                    {
                        filterstring=filterstring+"filter[."
                            +filterv.at(i).at(1)+"_a"
                            +filterv.at(i).at(2)
                            +filterv.at(i).at(3)+"] \n";
                    }
                }
//filterstring=filterstring+"filter[."+filterv.at(i).at(1)+"_a = .Value] \n";
            }
        }
    }
}

//----------------------------------------------------------------------------
void Message::CreateReplaceString()
{
    string nextmessage=CreateNextMessageName();

    if (!gobackmessage && !lastmessage)
    {
        stringstream ss;
        ss << gofromsender;
        string strgofromsender = ss.str();

        std::vector<std::vector<std::string>> outputv;
        string outputstring="";

        //Baue Outputstring zusammen
        if (CheckForOutputFields(edge->FromNode->TypeName))
        {
            outputv=GetOutputFields(edge->FromNode->TypeName);
            for (uint i=0; i<outputv.size(); i++)
            {
                outputstring=outputstring+", "+outputv.at(i).at(1)+": ."
                    +outputv.at(i).at(2)+"_a";
            }
        }
        if (CheckForOutputFields(edge->TypeName))
        {
            outputv=GetOutputFields(edge->TypeName);
            for (uint i=0; i<outputv.size(); i++)
            {
                outputstring=outputstring+", "+outputv.at(i).at(1)
                    +": ."+outputv.at(i).at(2)+"_ab";
            }
        }

        std::vector<std::vector<std::string>> filterv;
        string filterstringtmp="";

        replacestring="replaceAttr[NodeId: ."+relinfotonode->IdFieldName
            +"_ab, Partition: part(."+relinfotonode->IdFieldName+"_ab),"
            " Message: "+nextmessage+", Sender"+strgofromsender+": ."
            +relinfofromnode->IdFieldName+"_a"+filterstringtmp+outputstring+
            "] \n";
    }

    if (gobackmessage)
    {
        stringstream ss;
        ss << gotosender;
        string strgotosender = ss.str();

        std::vector<std::vector<std::string>> outputv;
        string outputstring="";

        //Baue Outputstring zusammen
        if (CheckForOutputFields(edge->ToNode->TypeName))
        {
            outputv=GetOutputFields(edge->ToNode->TypeName);
            for (uint i=0; i<outputv.size(); i++)
            {
                outputstring=outputstring+", "+outputv.at(i).at(1)+": ."
                    +outputv.at(i).at(2)+"_a";
            }
        }

        replacestring="replaceAttr[NodeId: .Sender"+strgotosender
            +", Partition: part(.Sender"+strgotosender+"), "
            "Message: "+nextmessage+""+outputstring+
            "] \n";
    }

    if (lastmessage)
    {
        std::vector<std::vector<std::string>> outputv;
        string outputstring="";

        //Baue Outputstring zusammen
        if (CheckForOutputFields(actnodename))
        {
            outputv=GetOutputFields(actnodename);

            for (uint i=0; i<outputv.size(); i++)
            {
                if (i == 0)
                {
                    outputstring=outputstring+outputv.at(i).at(1)
                        +": ."+outputv.at(i).at(2)+"_a";
                }
                else
                {
                    outputstring=outputstring+", "+outputv.at(i).at(1)
                        +": ."+outputv.at(i).at(2)+"_a";
                }
            }
            replacestring="replaceAttr["+outputstring+"] \n";
        }

    }
}

//----------------------------------------------------------------------------
void Message::CreateLastMessageString()
{
    std::map<std::string, std::vector<std::vector<std::string>>>::iterator it;
    string outputs="";
    for (it = outputfieldszg->begin(); it != outputfieldszg->end(); it++)
    {
        for (uint i=0; i<it->second.size(); i++)
        {
            if (it == outputfieldszg->begin() && i==0)
            {
                outputs=outputs+it->second.at(i).at(1);
            }
            else
            {
                outputs=outputs+", "+it->second.at(i).at(1);
            }
        }

    }

    lastmessagestring="project["+outputs+"] \n"
    "minsert[ResultsMemory] \n"
    "filter[FALSE] \n";


}

//----------------------------------------------------------------------------
std::string Message::GetGesamtMessageString()
{
    gesamtmessagestring=name+tuplestrom+actualtuple+actsuccessor
        +filterstring+replacestring+lastmessagestring;
    return gesamtmessagestring;
}


} // namespace
