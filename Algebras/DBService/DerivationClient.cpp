/*

----
This file is part of SECONDO.

Copyright (C) 2017,
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


1 Implementation of class DerivationClient

*/

#include "Algebras/DBService/DerivationClient.hpp"
#include "Algebras/DBService/MetadataObject.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/CommunicationClient.hpp"
#include "SecondoSystem.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"

namespace DBService{

DerivationClient::DerivationClient(
         const std::string& _DBName,
         const std::string& _targetName,
         const std::string& _relName,
         const std::string& _fundef): DBName(_DBName), targetName(_targetName),
                                      relName(_relName), fundef(_fundef){
    relId = MetadataObject::getIdentifier(DBName, relName);
    targetId = MetadataObject::getIdentifier(DBName, targetName);
}


void DerivationClient::start() {

   SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
   // some basic checks
   if(ctlg->IsObjectName(targetId)){
     derivationFailed("target " + targetName + " already exists");
     return;
   }
   if(!ctlg->IsObjectName(relId)){
     derivationFailed("argument relation " + relName + " does not exist");
     return;
   }
  
   // convert function string into nested list
   // and check for valid format
   ListExpr funlist;
   if(!nl->ReadFromString(fundef, funlist)){
       derivationFailed("cannot parse function definition");
       return;
   }  
   if(!nl->HasLength(funlist,3)){
       derivationFailed("invalid function description");
       return;
   }
   ListExpr funarg = nl->Second(funlist);
   if(!nl->HasLength(funarg,2)){
       derivationFailed("invalid function description");
       return;
   }
   ListExpr argNameL = nl->First(funarg);
   if(nl->AtomType(argNameL)!=SymbolType){
      derivationFailed("invalid argument name");
   }

   // replace the argument's name in the function 
   // definition by the relation's id
   std::string argName = nl->SymbolValue(argNameL);
   ListExpr fundeflist = nl->Third(funlist);
   ListExpr relSymb = nl->SymbolAtom(relId);
   fundeflist = listutils::replaceSymbol(fundeflist,argName, relSymb,nl);

   // try to evaluate the function
   Word QueryResult;
   std::string typeString, errorString;
   bool correct, evaluable, defined, isFunction;

   SecondoSystem::BeginTransaction();
   try{
     QueryProcessor::ExecuteQuery(fundeflist, QueryResult, typeString, 
                                  errorString, correct, 
                                  evaluable,defined, isFunction);
   } catch(...){
      correct = false;
      errorString = "exception during executeQuery";
   }
   if(!correct){
      derivationFailed(errorString);
      return;
   }

   // insert result into the database
   ListExpr typeExpr;
   nl->ReadFromString(typeString, typeExpr);

   if(!ctlg->InsertObject(targetId, "",typeExpr, QueryResult, true)) {
       derivationFailed("could not insert target object");
       return;
   }
   ctlg->CleanUp(false,true);
   SecondoSystem::CommitTransaction(true);
   // report success of operation
   derivationSuccessful();
}
         


void DerivationClient::derivationFailed(const std::string& error){
  // ignore in the moment
}

void DerivationClient::derivationSuccessful(){

    std::string dbServiceHost;
    std::string dbServicePort;
    if(!SecondoUtilsLocal::lookupDBServiceLocation(
            dbServiceHost,
            dbServicePort))
    {
        throw new SecondoException("Unable to connect to DBService");
    }

    CommunicationClient clientToDBServiceMaster(
            dbServiceHost,
            atoi(dbServicePort.c_str()),
            0);
    clientToDBServiceMaster.reportSuccessfulDerivation(
            DBName,
            targetName);
}


}

