/*

*/
#ifndef SECONDO_TESMESSAGE_H
#define SECONDO_TESMESSAGE_H


#include <string>
#include <ostream>
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include <boost/log/trivial.hpp>

namespace distributed3 {
class Message {
 
public:
  friend std::ostream& operator<<(std::ostream &os, const Message &message);

  //enum MessageType {
  enum class MessageType {
   UNKNOWN,
   FINISH,
   DATA
  };
  /*
  static std::string typToString(const MessageType value) {
   switch (value) {
    case MessageType::DATA:
     return "DATA";
    case MessageType::FINISH:
     return "FINISH";
    default:
     return "no valid MessageType";
     //return std::to_string(value);
   }
  }
  */
  struct Header {
    Header(int eid, int slot, MessageType type, size_t length);

    Header();

    bool operator==(const Header& rhs) const;

    static Header from(const int eid, const int slot);

    static Header fromBin(char *buffer);

    char *write(char *buffer) const;

    char *write() const;

    friend std::ostream &operator<<(std::ostream &os, const Header &header);

    int eid = 0;
    int slot = 0;
    MessageType type = MessageType::DATA;
    size_t length = 0;
   
  };

  constexpr static size_t HEADER_SIZE = sizeof(int) + 
                                        sizeof(int) + 
                                        sizeof(MessageType) +
                                        sizeof(size_t);

  

  Message();
  Message(Header header, Tuple* body);
  // move, no copy
  Message(Message&&);
  Message& operator=(Message&&);
  
  ~Message();

  static Message* fromTuple1( const int eid, const int slot,  Tuple* tuple);
  static Message  fromTuple2( const int eid, const int slot,  Tuple* tuple);
  
  static Message deserialize2(char* buffer, Header header);
  static Tuple* deserialize(const int eid, char* buffer);
  size_t serialize(char*& buffer);
  
  static Message* constructFinishMessage1(int eid) {
    return new Message{Header{eid, -1, MessageType::FINISH, 0}, nullptr};
  }
  static Message constructFinishMessage2(int eid) {
    return Message{Header{eid, -1, MessageType::FINISH, 0}, nullptr};
  }
  
  bool operator==(const Message& rhs) const;
  
  Tuple* getBody() const;

  void setBody(Tuple* body);
  
  void setSlot(int slot);

  int getSlot() const;
  
  void setEID(int eid);
  
  int getEID() const;
  
  inline void setType(MessageType type) {
   header.type = type;
  }
  
  inline MessageType getType() const {
   return header.type;
  }
  
  inline void setLength(size_t size) {
   header.length = size;
  }

  inline size_t getLength() const {
   return header.length;
  }
  
  char* writeHeader() const {
   return header.write();
  }

 private:
  static SmiRecordId tupleId;
  Tuple *body;
  Header header;
 };
 
}


#endif //SECONDO_TESMESSAGE_H
