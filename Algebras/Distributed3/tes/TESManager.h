/*

*/
#ifndef ALGEBRAS_DISTRIBUTED3_TESMANAGER_H_
#define ALGEBRAS_DISTRIBUTED3_TESMANAGER_H_

//#include "../DArrayElement.h"
#include "Helpers/WorkerConfig.h"
#include "Helpers/RemoteEndpoint.h"
#include "Algebras/Distributed2/DArrayElement.h"
#include "typedefs.h"

namespace distributed3 {

class TESManager {
 private:
  TESManager() {}
  static TESManager manager;
  static int exchangeId;
  std::vector<WorkerConfig> workers = std::vector<WorkerConfig>{};
  
 public:

  static TESManager& getInstance();
  static bool equalWorkers(
                const std::vector<WorkerConfig> tesWorkers, 
                const std::vector<distributed2::DArrayElement>& workers);
  ~TESManager();
  void reset();
  std::vector<WorkerConfig>& getWorkerVector();
  supplier<WorkerConfig> getWorkers();

  int numOfWorkers();
  void addWorker(WorkerConfig worker);// noexcept(false);
  /*
    true if worker exists with same endpoint or same host and messageServerPort
  */
  bool workerExists(RemoteEndpoint& endpoint, int messageServerPort);


  std::ostream& print(std::ostream& os) const{
    os << " routes: " << workers.size() << std::endl;
    for (auto& worker : workers) {
      worker.print(os) << std::endl;
    }
    return os;
  }
        
  int getExchangeID();
  	
};

} /* namespace distributed3 */

#endif /* ALGEBRAS_DISTRIBUTED3_TESMANAGER_H_ */
