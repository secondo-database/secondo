/*

*/
#include <ostream>
#include "WorkerConfig.h"
#include "RemoteEndpoint.h"

std::ostream& operator<<(std::ostream& os, 
                         const distributed3::WorkerConfig& config){
   return config.print(os);
}

