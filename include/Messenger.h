/*

*/

#ifndef MESSENGER_H
#define MESSENGER_H

#include <string>

class Messenger
{
 public:
  Messenger( const string& queueName ) : msgQueue( queueName ) {};
  virtual ~Messenger() {};
  bool Send( const string& message, string& answer );
 protected:
 private:
  string msgQueue;
};

#endif

