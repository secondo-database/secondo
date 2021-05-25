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

#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include <list>

#include "PropertyGraphMem.h"
#include "PropertyGraphQueryProcessor.h"
#include "QueryTree.h"



namespace pgraph2 {

class Compute;
class Message;

//-----------------------------------------------------------------------------
class Compute
{
public:
   Compute(PGraphQueryProcessor *pgp,QueryTree *tree, std::string pgstructure);
   Compute(PGraphQueryProcessor *pgp,QueryTree *tree, std::string pgstructure,
   std::string tmptuplestream);
   ~Compute();

   PGraphQueryProcessor *pgprocessor;
   QueryTree *querytree;
   GenericRelationIterator *_InputRelationIteratorPregel=NULL;

   std::map<std::string, std::vector<std::vector<std::string>>> outputfields;
   std::map<std::string, std::vector<std::vector<std::string>>> filterfields;

   int messagecounter=0;
   std::string structure="";

   bool withtuplestream=false;
   std::string tuplestream="";

   std::string tuplestring="";
   std::string projectstring="";
   std::string headerstring="";

   std::string gesamtstring="";
   std::string initialmessage="";
   std::string resultstring="";

   std::map<std::string, int> Relations;
   std::map<std::string, int> Edges;

   std::vector<Message*> messageslist;

   int GetMaxId(std::string relname, std::string attrname);

   void CreateComputeFunction();
   void CreateResults();
   void CreateInitialMessage();

   void CreateHeaderString();
   std::string GetTupleString(bool withdatatype);

   void SetOutputFields();
   std::string GetOutputFieldsWithDatatype();
   std::string GetOutputFieldsWithoutDatatype(bool initial);

   void SetFilterFields();

   void CreateProjectString();

   void CreateMessages();
   std::string ReadMessages();

   void runPregel();
   void runPregelSecondTime();
   void CreateObjects();
   void createRemoteObjectsFile();
   void ShareObjects();
   void runPregelCommands();
   void runPregelCommandsSecond();
   void getEndResults();

   Tuple *ReadNextResultTuplePregel();
};

//-----------------------------------------------------------------------------
class Message
{
public:
   Message(PGraphQueryProcessor *pgp, QueryTreeEdge *qedge, \
    std::map<std::string, std::vector<std::vector<std::string>>>* outputs, \
    std::map<std::string, std::vector<std::vector<std::string>>>* filters, \
    int indx, bool goback, uint senderid, std::string pgstructure, \
    bool withtuplestream);

   Message(PGraphQueryProcessor *pgp, std::string actnode, \
   std::map<std::string, std::vector<std::vector<std::string>>>* outputs, \
   std::map<std::string, std::vector<std::vector<std::string>>>* filters, \
   int indx, bool last, std::string pgstructure, bool withtuplestream);

   Message(PGraphQueryProcessor *pgp, QueryTreeEdge *qedge, \
   std::map<std::string, std::vector<std::vector<std::string>>>* outputs, \
   std::map<std::string, std::vector<std::vector<std::string>>>* filters, \
   int indx, uint senderid, std::string pgstructure, bool withtuplestream);

   ~Message();

   PGraphQueryProcessor *pgprocess;
   QueryTreeEdge *edge;

   std::map<std::string, std::vector<std::vector<std::string>>>* outputfieldszg;
   std::map<std::string, std::vector<std::vector<std::string>>>* filterfieldszg;

   std::string actnodename="";
   int messageindex;
   int gotosender;
   int gofromsender;
   std::string structure="";

   bool lastmessage=false;
   bool gobackmessage=false;
   bool tuplestream=false;

   RelationInfo *relinfofromnode=NULL;
   RelationInfo *relinfotonode=NULL;

   std::string name="";
   std::string tuplestrom="t feed \n";
   std::string actualtuple="";
   std::string actsuccessor="";
   std::string filterstring="";
   std::string replacestring="";
   std::string lastmessagestring="";

   std::string gesamtmessagestring="";

   void CreateMessageString();
   std::string CreateNextMessageName();

   void CreateMessageName();
   void CreateMessageActualTupleString();
   void CreateMessageActualSuccessorString();

   bool CheckForOutputFields(std::string relname);
   std::vector<std::vector<std::string>> GetOutputFields(std::string relname);

   bool CheckForGlobalFilters(std::string relname);
   std::vector<std::vector<std::string>> GetGlobalFilters(std::string relname);

   void CreateFilterString();
   void CreateReplaceString();
   void CreateLastMessageString();

   std::string GetGesamtMessageString();

};

} // namespace
