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
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] DServerCmdCallBackCommunication

\begin{center}
March 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerCmdCallBackCommunication ~

*/

/*

1 Preliminaries

1.1 Includes

*/

#include "DServerCmdCallBackComm.h"
#include "RelationAlgebra.h"
#include "DBAccessGuard.h"
/*

2 Implementation

2.2  Method ~bool writeTupleToCallBack~

sends a tuple (as binary stream) 

(this method is threadsave w/ regards to DB access)

  * Tuple[ast] inTupe - the tuple to be sent

  * returns true - success

*/
int str2int(const string& inStr, bool &success)
{
  int result = 0;
  success = true;
  stringstream str(inStr);
  if (!(str >> result))
    success = false;
  return result;
}

bool
DServerCmdCallBackCommunication::writeTupleToCallBack(Tuple *inTuple)
{
  if (inTuple != NULL)
    {
      //Get the tuple size
      size_t cS,eS,fS;
      size_t size = 
        DBAccess::getInstance() -> T_GetBlockSize(inTuple,
                                                  cS, eS, fS);
      string line;
               
      int num_blocks = (size / 1024) + 1;
               
      //Send the size of the tuple to the worker
               
      if (!sendTextToCallBack("TUPLE", (int)size, false))
        {
          setErrorText("Unable to send token TUPLE");
          return false;
        }
               
      if (!getTagFromCallBack("NUMBLOCKS", line, false))
        {
          setErrorText("Received unexpected token");
          return false;
        }
      bool succ = true;
      if(str2int(line.data(), succ) != num_blocks) 
        {
          setErrorText("Invalid number of blocks for sending tuples");
          return false;
        }
      char* buffer = new char[num_blocks*1024];
      memset(buffer,0,1024*num_blocks);
               
      //Get the binary data of the tuple
      DBAccess::getInstance() -> T_WriteToBin(inTuple, buffer, cS, eS, fS);
               
      //Send the tuple data to the worker
      for(int i = 0; i<num_blocks;i++)
        Write(buffer+i*1024,1024);

      delete [] buffer;
      
      if (!(getTagFromCallBack("GOTTUPLE")))
        {
          setErrorText("Received unexpected token");
          return false;
        }
      //cout << m_index << ": got " << tb -> GetNoAttributes()  << endl;
    } // if (t != NULL)
  
  return true;
}

/*

2.3 Method ~bool receiveTupleFromCallBack~

receives a tuple (binary stream) and inserts it into a relation

(this method is threadsave w/ regards to DB access)

  * ListExpr inTupleType - type of the expected tuple

  * GenericRelation[ast] inRel - pointer to the relation, where the 
tuple will be stored

  * returns true - success

*/
bool
DServerCmdCallBackCommunication::
    receiveTupleFromCallBack(TupleType* inTupleType,
                             GenericRelation *inRel)
{
  string line;
  if (getTagFromCallBack("TUPLE", line, false))
    {
      Tuple *curTuple = DBAccess::getInstance() -> T_New(inTupleType);

      bool succ = true;
      size_t size = str2int(line.data(), succ);

      int num_blocks = (size / 1024) + 1;
      
      char* buffer = new char[1024*num_blocks];
      memset(buffer,0,1024*num_blocks);

      // reading tuple data in biary format
      // from server 

      if (!sendTextToCallBack("NUMBLOCKS", num_blocks, false))
        {
          cerr << "REC ERROR SEND NUMBLOCKS" << endl;
        }

      for(int i = 0; i<num_blocks; i++)
        Read(buffer+i*1024,1024);

      // instantiating tuple
      DBAccessGuard::getInstance() -> T_ReadFromBin(curTuple, buffer);
      DBAccessGuard::getInstance() -> REL_AppendTuple(inRel,curTuple);
      DBAccessGuard::getInstance() -> T_DeleteIfAllowed(curTuple);
     
      delete [] buffer;
      
      sendTagToCallBack("GOTTUPLE");
    
         
    }
  return true;
}
