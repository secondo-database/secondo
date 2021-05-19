/*

*/
#include "Commander.h"
#include <boost/regex.hpp>

namespace distributed3 {
 Runner::Runner(WorkerConnection *connection, std::string &query)
  : connection(connection), query(query), commandLog() {}

 Runner::~Runner() {}

 const resultMapper<bool> Commander::isTrue([](std::string &result) {
   return new bool(boost::regex_match(result, boost::regex(".*TRUE\\)?$")));
 });

 const resultMapper<void> Commander::throwWhenFalse(
  [](std::string &result) -> void * {

    boost::regex re(".*TRUE\\)?$");
    if (!boost::regex_match(result, re)) {
     throw RemoteExecutionException("Remote query returned FALSE");
    }
    return (void *) nullptr;
  });


 std::string Runner::run() noexcept(false) {
  if (hasRun) {
   BOOST_LOG_TRIVIAL(warning) << "But I have already run!?";
  }
  if (!connection->check(false, commandLog)) {
   if (!connection->reconnect(false, commandLog)) {
    throw RemoteExecutionException(
     "Couldn't execute remote command: Worker not alive!");
   }
  }

  std::string errorMessage;
  std::string resultStore;
  int err;
  connection->simpleCommand(query, err, errorMessage, resultStore, false,
                            runtime, false, commandLog);

  if (err != 0) {
   throw RemoteExecutionException(
    "Error executing remote command: " + errorMessage);
  }
  hasRun = true;

  return resultStore;
 }

 RemoteExecutionException::RemoteExecutionException(const std::string &message)
  : message(message) {}

 const std::string &RemoteExecutionException::getMessage() const {
  return message;
 }
}

