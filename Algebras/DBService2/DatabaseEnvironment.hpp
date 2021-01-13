#ifndef DATABASE_ENVIRONMENT_H
#define DATABASE_ENVIRONMENT_H

#include <string>

namespace DBService {
  struct DatabaseEnvironment {
    static std::string production;
    static std::string staging;
    static std::string test;
  };
}
#endif