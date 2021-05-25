/*

*/
#include "TESManager.h"
#include "Helpers/Commander.h"
#include <boost/log/trivial.hpp>
#include <StandardTypes.h>

namespace distributed3
{

TESManager TESManager::manager;
int TESManager::exchangeId = 3;

TESManager& TESManager::getInstance() {
  return manager;
}
TESManager::~TESManager() {
  std::cout << "~TESManager() aufgerufen";
  //reset(); // funktioniert nicht mit reset(true)
}
// TODO mit BOOST_LOG_TRIVIAL immer Fehler bei der Ausführung
// TODO parallele Variante in SetupTES::resetTES()
void TESManager::reset() { // aufgerufen in setuptes
  //BOOST_LOG_TRIVIAL(debug) << "TESManager::reset()";
  std::cout << "\nTESManager::reset()";
  /*
  std::string query {"query resettes()"}; // kein semicolon am Ende der query!!
     
     for (WorkerConfig worker : workers) {
      //BOOST_LOG_TRIVIAL(info) << "Reset Worker";
      
      try {
       Commander::remoteQuery(worker.connection,
                           query,
                           Commander::throwWhenFalse);
      } catch (RemoteExecutionException &e) {
      //BOOST_LOG_TRIVIAL(warning) << "Reset worker failed: Error during query";
       std::cout << "\n RESET worker failed: Error during query " << e.what();
      } catch (std::exception &e) {
        std::cout << "\nRESET worker failed: Error during query " << e.what(); 
      }
     }*/
   {
   auto workers = getWorkers();
   supplier<Runner> runners = [&workers]() -> Runner * {
     WorkerConfig* worker;
     if ((worker = workers()) != nullptr) {
      std::string query {"query resettes()"}; 
      return new Runner(worker->connection, query);
     }
     return nullptr;
   };

    auto dummy = Commander::broadcast(runners, Commander::throwWhenFalse, true);
    // ensure to delete the result store
    auto d1 = dummy();
    while(d1 != nullptr){
      d1 = dummy();
    }
  }
  workers.clear(); 
   // unneccessary in Destuktion of Distributed3Algebra but does no harm.
}
/*
  both vectors must have the same size and the same order. 
  workers must be equal in host, port and config.
*/
bool TESManager::equalWorkers(
              const std::vector<WorkerConfig> tesWorkers, 
              const std::vector<distributed2::DArrayElement>& workers) {
  size_t size = tesWorkers.size();
  if (size != workers.size()) return false;
  for (size_t i=0; i<size; ++i) {
    if (tesWorkers[i].endpoint.host != workers[i].getHost()) return false;
    if (tesWorkers[i].endpoint.port != workers[i].getPort()) return false;
    if (tesWorkers[i].configFilePath != workers[i].getConfig()) return false;
  }
  return true;
}

int TESManager::numOfWorkers() {
  return workers.size();
}

void TESManager::addWorker(WorkerConfig worker) {// noexcept(false) {
  if (workerExists(worker.endpoint, worker.messageServerPort)) {
   throw std::exception();
  }
  workers.push_back(worker);
 }

bool TESManager::workerExists(RemoteEndpoint &endpoint,
                                  int messageServerPort) {
  for (auto it = workers.begin(); it != workers.end(); ++it) {
    if (endpoint == (*it).endpoint) {
      return true;
    }
    if (endpoint.host == (*it).endpoint.host &&
        messageServerPort == (*it).messageServerPort) {
      return true;
    }
  }
  return false;
}

std::vector<WorkerConfig>& TESManager::getWorkerVector() {
  return workers;
}

supplier<WorkerConfig> TESManager::getWorkers() {
  auto it = new std::vector<WorkerConfig>::iterator(workers.begin());
  return static_cast<supplier<WorkerConfig> > ( 
    [this, it]() mutable -> WorkerConfig * {
      if (it != nullptr && *it != workers.end()) {
       WorkerConfig *entry = &(**it);
       ++(*it);
       return entry;
      }

      delete it;
      it = nullptr;
      return nullptr;
    });
 }

int TESManager::getExchangeID() {
	return ++exchangeId;
}

} /* namespace distributed3 */
