/*

*/

#ifndef SECONDO_TES_TYPEDEFS_H
#define SECONDO_TES_TYPEDEFS_H

#include <memory>
#include <functional>
#include "Algebras/Distributed2/ConnectionInfo.h"

namespace distributed3 {
 #define PRECONDITION(condition, message) \
 if (!(condition)) { \
  BOOST_LOG_TRIVIAL(error) << message; \
  ((CcBool *) result.addr)->Set(true, false); \
  return 0; \
 }

 using WorkerConnection = distributed2::ConnectionInfo;

 template<typename Type> using supplier = std::function<Type *()>;
 template<typename Type> using supplier2 
                         = std::function<std::shared_ptr<Type> ()>;
 template<typename Type> using consumer = std::function<void(Type *)>;
 template<typename Type> using consumer2 
                        = std::function<void(std::shared_ptr<Type> )>;
 using executable = std::function<void()>;
}

#endif //SECONDO_TYPEDEFS_H
