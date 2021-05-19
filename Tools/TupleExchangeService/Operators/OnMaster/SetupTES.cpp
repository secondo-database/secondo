/*

*/
#include "SetupTES.h"
#include <ListUtils.h>
#include <vector>
#include <regex>
#include <QueryProcessor.h>
#include "Algebras/Distributed2/DArray.h"

#include "Algebras/Distributed2/ConnectionInfo.h"
#include <boost/log/trivial.hpp>
#include "../../Helpers/Commander.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../typedefs.h"
#include "../../TESManager.h"
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>
//#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Distributed3/Distributed3Algebra.h"

namespace distributed3 {
 ListExpr SetupTES::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  const ListExpr workerRelation = nl->First(args);

  if (!Relation::checkType(workerRelation)) {
   return listutils::typeError(
    "The first argument must be of type relation");
  }

  ListExpr tupleType = nl->Second(nl->Second(workerRelation));

  ListExpr hostType;
  int hostPosition = listutils::findAttribute(tupleType, "Host", hostType);
  if (!hostPosition) {
   BOOST_LOG_TRIVIAL(error) << "No \"Host\" attribute in relation";
   return listutils::typeError("No host found in relation");
  }
  if (!CcString::checkType(hostType) /*&& !FText::checkType(hostType)*/) {
   BOOST_LOG_TRIVIAL(error) << "\"Host\" attribute not of type string";
   return listutils::typeError("Host is not of type string");
  }

  ListExpr configType;
  int configPosition = listutils::findAttribute(tupleType, "Config",
                                                configType);
  if (!configPosition) {
   BOOST_LOG_TRIVIAL(error) << "No \"Config\" attribute in relation";
   return listutils::typeError("no config attribute in relation");
  }
  if (!CcString::checkType(configType) /*&& !FText::checkType(configType)*/) {
   BOOST_LOG_TRIVIAL(error) << "\"Config\" attribute not of type string";
   return listutils::typeError("Config attribute not of type string");
  }

  ListExpr portType;
  int portPosition = listutils::findAttribute(tupleType, "Port", portType);
  if (!portPosition) {
   BOOST_LOG_TRIVIAL(error) << "No \"Port\" attribute in relation";
   return listutils::typeError("No Port attribute in relation");
  }
  if (!CcInt::checkType(portType)) {
   BOOST_LOG_TRIVIAL(error) << "\"Port\" attribute not of type int";
   return listutils::typeError("Port attribute not of type int");
  }

  ListExpr messageServerPortType;
  int messageServerPortPosition = listutils::findAttribute(
      tupleType, 
      "MessageServerPort",
      messageServerPortType
  );
  if (!messageServerPortPosition) {
   BOOST_LOG_TRIVIAL(error) 
   << "No \"MessageServerPort\" attribute in relation";
   return listutils::typeError("No MessageServerPort in Relation");
  }
  if (!CcInt::checkType(messageServerPortType)) {
   BOOST_LOG_TRIVIAL(error)
    << "\"MessageServerPort\" attribute not of type int";
   return listutils::typeError("messageServerPort not of type int");
  }

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbols::APPEND()),
   nl->FourElemList(
    nl->IntAtom(hostPosition - 1),
    nl->IntAtom(portPosition - 1),
    nl->IntAtom(messageServerPortPosition - 1),
    nl->IntAtom(configPosition - 1)
   ),
   nl->SymbolAtom(CcBool::BasicType())
  );
 }

 int SetupTES::valueMapping(Word *args, Word &result,
                               int, Word &,
                               Supplier s) {
  result = qp->ResultStorage(s);
  
  //resetTES();
  auto relation = (Relation *) args[0].addr;
  int hostIndex = ((CcInt *) args[1].addr)->GetIntval();
  int portIndex = ((CcInt *) args[2].addr)->GetIntval();
  int messageServerPortIndex = ((CcInt *) args[3].addr)->GetIntval();
  int configIndex = ((CcInt *) args[4].addr)->GetIntval();

  
  PRECONDITION(relation->GetNoTuples() > 1,
               "You can't configure the TES with less than two Workers.")
  try {
   int workerNumber = 0;
   std::string dbName = SecondoSystem::GetInstance()->GetDatabaseName();

   RelationIterator it(*relation, relation->GetTupleType());

   Tuple* tuple;
   while ((tuple = it.GetNextTuple()) != nullptr) {
    auto worker = workerFromTuple(tuple, hostIndex, portIndex,
                                  messageServerPortIndex, configIndex, dbName,
                                  workerNumber);
    TESManager::getInstance().addWorker(worker);
    tuple->DeleteIfAllowed();
    ++workerNumber;
   }
   //startMessageServersAndClients();
   startTESServers();
   startLoopbackClients();
   startTESClients();
   
  } catch (RemoteExecutionException &e) {
   BOOST_LOG_TRIVIAL(error)
    << "Failed to set up TES due to error during remote query. Will reset.";
   ((CcBool *) result.addr)->Set(true, false);
   // TODO PregelAlgebra::getAlgebra()->reset(true);
   return -1;
  } catch (std::exception &e) {
   BOOST_LOG_TRIVIAL(error) << "Failed to start Clients. Will reset.";
   ((CcBool *) result.addr)->Set(true, false);
   // TODO PregelAlgebra::getAlgebra()->reset(true);
   return -1;
  }

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec SetupTES::operatorSpec(
  "rel -> bool",
  "# (_)",
  "This operator initializes the Tuple Exchange Service on "
  "a master compute node."
  "It starts message servers and clients and thus connects the different "
  "workers."
"You must provide a Worker relation containing their hostname, port, config "
  "and additionally a message server port."
"This relation equals the specification of a worker relation that's used in "
  "the Distributed3Algebra, except for the messageServerPort."
  "The type of the relation must hence be:"
  "(rel(tuple((Host string) (Config string) (Port int) "
  "(MessageServerPort int)))"
  ""
  "The operator returns TRUE if all workers were successfully set up.",
  "query setuptes(Workers);",
  "This operator belongs to the TES API."
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
 );

 Operator SetupTES::setupTES(
  "setuptes",
  SetupTES::operatorSpec.getStr(),
  SetupTES::valueMapping,
  Operator::SimpleSelect,
  SetupTES::typeMapping
 );

void SetupTES::resetTES() {
  // TESManager has Connections to the workers. So it does it all.
  TESManager::getInstance().reset();
  /* vor dem Löschen TESManager::reset() ansehen !!!
  auto workers = TESManager::getInstance().getWorkers();
   supplier<Runner> runners = [&workers]() -> Runner * {
     WorkerConfig *worker;
     if ((worker = workers()) != nullptr) {
      std::string query = "query resettes()"; 
      return new Runner(worker->connection, query);
     }
     return nullptr;
   };

    auto dummy = Commander::broadcast(runners, 
                                      Commander::throwWhenFalse, true);
    // ensure to delete the result store
    auto d1 = dummy();
    while(d1 != nullptr){
      d1 = dummy();
    }
    */
    
}

 void SetupTES::startTESServers() {
   auto workers = TESManager::getInstance().getWorkers();
   for (WorkerConfig* worker = workers();
        worker != nullptr; worker = workers()) {
    std::string query{"query startTESServer(" +
                      std::to_string(worker->messageServerPort) + ")"};
    Commander::remoteQuery(worker->connection,
                           query,
                           Commander::throwWhenFalse);
   }
 }
 void SetupTES::startLoopbackClients() {
   auto workers = TESManager::getInstance().getWorkers();
   supplier<Runner> runners = [&workers]() -> Runner * {
     WorkerConfig *worker;
     if ((worker = workers()) != nullptr) {
      std::string query = "query startLoopbackTESClient(" +
                          std::to_string(worker->workernr) + ")"; 
      return new Runner(worker->connection, query);
     }
     return nullptr;
   };

    auto dummy = Commander::broadcast(runners, 
                                      Commander::throwWhenFalse, true);
    // ensure to delete the result store
    auto d1 = dummy();
    while(d1 != nullptr){
      d1 = dummy();
    }
 }

 void SetupTES::startTESClients() {
   std::vector<Runner *> runners;
   auto remoteWorkers = TESManager::getInstance().getWorkers();
   for (auto remote = remoteWorkers();
        remote != nullptr;
        remote = remoteWorkers()) {
    auto workers = TESManager::getInstance().getWorkers();
    for (auto worker = workers(); worker != nullptr; worker = workers()) {

     if (worker->workernr == remote->workernr) {
      continue;
     }

     const int &workernr = remote->workernr;
     const std::string &host = remote->endpoint.host;
     const int &port = remote->messageServerPort;

     std::string query{"query startTESClient(" +
                       std::to_string(workernr) + ", \"" +
                       host.c_str() + "\", " +
                       std::to_string(port) + ")"};
     auto runner = new Runner(worker->connection, query);
     runners.push_back(runner);
    }

    supplier<Runner> runnerSupplier = [&runners]() -> Runner * {
      if (runners.empty()) {
       return nullptr;
      }
      auto runner = runners.back();
      runners.pop_back();
      return runner;
    };

    // besides the broadcast, we have to iterate over the result to free
    // allocated memory for result
    auto dummy = Commander::broadcast(runnerSupplier, 
                                      Commander::throwWhenFalse, 
                                      true);
    auto d1 = dummy();
    while(d1 != nullptr){
      d1 = dummy();
    }
   }
 }
 WorkerConfig SetupTES::workerFromTuple(Tuple *tuple,
                                           int hostIndex,
                                           int portIndex,
                                           int messageServerPortIndex,
                                           int configIndex,
                                           std::string &dbName,
                                           int workerNumber) {  {
  auto hostString = (CcString *) tuple->GetAttribute(hostIndex);
  auto portInt = (CcInt *) tuple->GetAttribute(portIndex);
  auto messageServerPortInt = (CcInt *) tuple->GetAttribute(
   messageServerPortIndex);
  auto configFilePathString = (CcString *) tuple->GetAttribute(
   configIndex);

  if (!hostString->IsDefined() ||
      !portInt->IsDefined() ||
      !messageServerPortInt->IsDefined() ||
      !configFilePathString->IsDefined()) {
   BOOST_LOG_TRIVIAL(error)
    << "The extended worker relation must not contain undefined values";
   throw std::exception();
  }

  auto host = hostString->GetValue();
  auto port = portInt->GetIntval();
  auto messageServerPort = messageServerPortInt->GetIntval();
  auto configFilePath = configFilePathString->GetValue();

  //RemoteEndpoint endpoint = RemoteEndpoint(host, port);
  RemoteEndpoint endpoint{host,port};
  
  if (TESManager::getInstance().workerExists(endpoint, messageServerPort)) {
   std::cout << "\nworker existiert";
   throw std::exception();
  }
  
  distributed2::DArrayElement elem {host,port,workerNumber, configFilePath};
  auto connection = 
       Distributed3Algebra::getAlgebra()->getWorkerConnection(elem,dbName);
  
  //auto connection = 
  // WorkerConnection::createConnection(host, port,configFilePath);

  if (connection == nullptr) {
   throw std::exception();
  }

  //connection->switchDatabase(dbName, true, true);
  
  return WorkerConfig{workerNumber, endpoint,
                      messageServerPort, configFilePath,
                      connection};
 }
 /*
void SetupTES::startMessageServersAndClients() noexcept(false) {
  // wird noch nicht benötigt MessageBroker &broker = MessageBroker::get(); 
  
 
  
  // Master muss Workern keine Tupel schicken. 
  // Der nächste Teil kann also entfallen.

  {
   auto workers = TESManager::getInstance().getWorkers();
   for (auto remote = workers(); remote != nullptr; remote = workers() ) {
    const int &workernr = remote->workernr;
    const std::string &host = remote->endpoint.host;
    const int &port = remote->messageServerPort;
    bool successful = broker.startClient(workernr, RemoteEndpoint(host, port));
    if (!successful) {
     BOOST_LOG_TRIVIAL(error)

      << "FAILED starting client";
     throw

      std::exception();
    }
   }
  }
  

 }
*/
}
