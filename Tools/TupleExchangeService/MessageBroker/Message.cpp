/*

*/
#include <cstring>
#include <StandardTypes.h>
#include "Message.h"
#include "../TESContext.h"

namespace distributed3 {
  TupleId Message::tupleId = 1;
  
  constexpr size_t Message::HEADER_SIZE;

  Message::Message() : body{nullptr},header{} {}

  Message::~Message() {
    if(body != nullptr){
      body->DeleteIfAllowed();
    }
  }
 
  Message::Message(Message::Header header, Tuple *body)
    : body{body}, header{header} {}
 
  Message::Message(Message&& other) 
       : body{other.body}, header{other.header}
  {
    other.body = nullptr;
    other.header = {};
  }
  
  Message& Message::operator=(Message&& other) {
    body = other.body; 
    header = other.header;
    other.body = nullptr;
    other.header = {};
    return *this;
  }
 
 
 template<typename T>
 char* readBin(char* buffer, T& v){
   memcpy(&v, buffer,sizeof(T));
   return buffer + sizeof(T);
 }
 
 Message::Header Message::Header::fromBin(char *buffer) {
   Header header;
   char* nb = buffer;
   nb = readBin(nb, header.eid);
   nb = readBin(nb, header.slot);
   nb = readBin(nb, header.type);
   nb = readBin(nb, header.length);
   return header;
 }
 
 Message::Header::Header() {}

 Message::Header::Header(int eid,
                         int slot,
                         MessageType messageType,
                         size_t messageLength)
  : eid(eid),  
    slot(slot),
    type(messageType),
    length(messageLength) {}

 Message::Header
 Message::Header::from(const int eid, const int slot) {
  return {eid, slot,MessageType::DATA, 0};
 }
 
 template<typename T>
 char* writeBin(char* buffer, T v){
   memcpy(buffer, &v, sizeof(T));
   return buffer + sizeof(T);
 }

 char* Message::Header::write(char *buffer) const {
  char* nb = buffer;
  nb = writeBin(nb, eid);
  nb = writeBin(nb, slot);
  nb = writeBin(nb, type);
  nb = writeBin(nb, length);
  
  return nb;
 }

 char* Message::Header::write() const {
  char* buffer = new char[HEADER_SIZE];
  write(buffer);
  return buffer;
 }

 bool Message::Header::operator==(
         const Message::Header& rhs) const{
  return     (slot == rhs.slot)
         &&  type == rhs.type
         && eid == rhs.eid;
       // && length = rhs.length    // only equal after serialization
 }

 bool equalTuples(Tuple* t1, Tuple* t2){
    if(t1==nullptr && t2==nullptr){
      return true;
    }
    if(t1==nullptr || t2==nullptr){
       return false;
    }
    if(t1->GetNoAttributes() != t2->GetNoAttributes()){
      return false;
    }
    for(int i=0;i<t1->GetNoAttributes(); i++){
      if(t1->GetAttribute(i)->Compare(t2->GetAttribute(i)) != 0){
         return false;
      }
    }
    return true;
 }

 
  bool Message::operator==(const Message& rhs) const{
    return    header==rhs.header
           && equalTuples(body, rhs.body);
  }


 Tuple* Message::getBody() const {
  if(body){
     body->IncReference();
  }
  return body;
 }

 void Message::setBody(Tuple *body) {
  if(this->body){
    this->body->DeleteIfAllowed();
  }
  body->IncReference();
  this->body = body;
 }
 // BinRelWriter::writeNextTuple schreibt uint32_t tsize = 
 // blocksize nach out, dann buffer.
 char* tuple2Bin(Tuple* tuple, size_t& buffersize){ // s. 
 // FileRelations BinRelWriter::writeNextTuple(std::ostream& out,Tuple* tuple)
   // retrieve sizes
   size_t coreSize;
   size_t extensionSize;
   size_t flobSize;
   size_t blocksize = tuple->GetBlockSize(coreSize, extensionSize,
                                         flobSize);
   // allocate buffer and write flob into it
   char* buffer = new char[blocksize];
   tuple->WriteToBin(buffer, coreSize, extensionSize, flobSize);
   buffersize = blocksize;
   return buffer;
 }

  size_t Message::serialize(char*& buffer) {

    size_t size = HEADER_SIZE;
    size_t bodySize = 0;
    char* bodyBuffer = nullptr;
 
    if (header.type == MessageType::DATA) {
      bodyBuffer = tuple2Bin(body,bodySize);
      header.length = bodySize;
      size += bodySize; 
    }

    buffer = new char[size];

    char *offset = header.write(buffer);

    if (header.type == MessageType::DATA) {
       memcpy(offset,bodyBuffer, bodySize);
       delete[] bodyBuffer;
    }
    return size;
  }
 
Tuple* Message::deserialize(const int eid, char* buffer) {
  TupleType* tupleType = TESContext::get().getTupleType(eid);
  Tuple* tuple = new Tuple(tupleType);
  tuple->ReadFromBin(0, buffer ); // (SmiFileId fileId, 
                                  //   char* buf, u_int32_t bSize/* = 0*/)
  tuple->SetTupleId(tupleId++);
  //delete buffer; // buffer is deleted by the caller
  return tuple;
}
 
 Message Message::deserialize2(char* buffer, Header header) {
  char* offset = buffer;
  TupleType* tupleType = TESContext::get().getTupleType(header.eid);
  Tuple* tuple = new Tuple(tupleType);
  tuple->ReadFromBin(0, offset ); // (SmiFileId fileId, 
  //                                    char* buf, u_int32_t bSize/* = 0*/)
  tuple->SetTupleId(tupleId++);
  return Message{header, tuple};
 }
 int Message::getSlot() const {
  return header.slot;
 }

 void Message::setSlot(int slot) {
  header.slot = slot;
 }
 
 int Message::getEID() const {
   return header.eid;
 }
 
 void Message::setEID(int eid) {
   header.eid = eid;
 }

 std::ostream &operator<<(std::ostream &os, const Message &message) {
  if (message.getType() != Message::MessageType::DATA) {
   os << "non data message:" << " header: " << message.header;
   return os;
  }
  ListExpr tupleType;
  nl->ReadFromString(TESContext::get().getStringTupleType(message.getEID()),
                                                                 tupleType);
  Tuple* tuple = message.getBody();
  ListExpr toList = tuple->SaveToList(tupleType);
  tuple->DeleteIfAllowed();
  
  os << "data message: body: " << nl->ToString(toList)  << " header: "
     << message.header;
  return os;
 }
 
 inline std::ostream &
 operator<<(std::ostream &os, Message::MessageType value) {
  switch (value) {
   case Message::MessageType::DATA :
    return os << "DATA";
   case Message::MessageType::FINISH :
    return os << "FINISH";
   default:
    return os << (int) value;
  }
 }
 
 std::ostream &
 operator<<(std::ostream &os, const Message::Header &header) {
  os << "slot: " << header.slot << " type: " << header.type
     << " length: "<< header.length << " eid: " << header.eid;
  return os;
 }
 Message* Message::fromTuple1( const int eid, const int slot, Tuple *tuple) {
  Header header = Header::from(eid, slot);
  return new Message{header, tuple};
 }
 
 Message Message::fromTuple2( const int eid, const int slot, Tuple *tuple) {
  Header header = Header::from(eid, slot);
  return Message{header, tuple};
 }

}
