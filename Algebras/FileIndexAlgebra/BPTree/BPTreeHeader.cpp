/*
class representing the header of a B+ Tree-Index file

*/

#include <stdexcept>   // out_of_range, runtime_error

#include "BPTreeHeader.h"

using std::out_of_range;
using std::runtime_error;
using std::to_string;

namespace fialgebra{

  size_t BPTreeHeader::constantSize =
    sizeof(char) + sizeof(char) + sizeof(size_t) + sizeof(size_t) +
    sizeof(unsigned int) + sizeof(unsigned int) +
    sizeof(unsigned long) + sizeof(unsigned long);

  // ctor
  BPTreeHeader::BPTreeHeader(char* bytes, size_t length, size_t pageSize){
    if( length < constantSize ){
      throw runtime_error( "BPTreeHeader(char*, size_t, size_t): "
                           "pageSize < constantSize" );
    }
    if( pageSize < constantSize ){
      throw runtime_error( "BPTreeHeader(char*, size_t, size_t): "
                           "pageSize < constantSize" );
    }
    else
    {
      m_bytes     = bytes;
      // Muss eigentlich immer TreeHeaderMarker.BPlusTree sein
      m_marker    = m_bytes;
      m_pageSize  = (size_t*)(m_marker + 1);
      m_valueSize = m_pageSize + 1;
      m_root      = (unsigned long*)(m_valueSize + 1);
      m_emptyPage = m_root + 1;
      m_algebraID = (unsigned int*)(m_emptyPage + 1);
      m_typeID    = m_algebraID + 1;
      m_version   = (char*)(m_typeID + 1);
    }
  }
  BPTreeHeader::BPTreeHeader(size_t pageSize,
                             size_t valueSize,
                             unsigned int algebraID,
                             unsigned int typeID,
                             unsigned long root,
                             unsigned long emptyPage) {
    // Wenn die Seitengroesse kleiner ist, als die Groesse des
    // Headers, ist es nicht moeglich, weiter zu machen.
    // Der Header steht auf der ersten Seite einer Index-Datei.
    if(pageSize < constantSize)
    {
        throw runtime_error("BPTreeHeader(size_t, size_t, unsigned int, "
                            "unsigned int, unsigned long, unsigned long): "
                            "pageSize < constantSize");
    }

    m_bytes     = new char[pageSize];
    m_marker    = m_bytes;
    m_pageSize  = (size_t*)(m_marker + 1);
    m_valueSize = m_pageSize + 1;
    m_root      = (unsigned long*)(m_valueSize + 1);
    m_emptyPage = m_root + 1;
    m_algebraID = (unsigned int*)(m_emptyPage + 1);
    m_typeID    = m_algebraID + 1;
    m_version   = (char*)(m_typeID + 1);

    (*m_marker)    = (char)TreeHeaderMarker::BPlusTree;
    (*m_pageSize)  = pageSize;
    (*m_valueSize) = valueSize;
    (*m_root)      = root;
    (*m_emptyPage) = emptyPage;
    (*m_algebraID) = algebraID;
    (*m_typeID)    = typeID;
    (*m_version)   = currentVersion;
  }
  BPTreeHeader::~BPTreeHeader()
  {
    delete[](m_bytes);
  }

  // Get/Set Version
  char BPTreeHeader::GetVersion(){
    return *m_version;
  }

  // Get/Set AlgebraID
  unsigned int BPTreeHeader::GetAlgebraID(){
    return *m_algebraID;
  }
  void BPTreeHeader::SetAlgebraID(unsigned int value){
    (*m_algebraID) = value;
  }

  // Get/Set TypeID
  unsigned int BPTreeHeader::GetTypeID(){
    return *m_typeID;
  }
  void BPTreeHeader::SetTypeID(unsigned int value){
    (*m_typeID) = value;
  }

  // Get/Set Root
  unsigned long BPTreeHeader::GetRoot(){
    return *m_root;
  }
  void BPTreeHeader::SetRoot(unsigned long value){
    (*m_root) = value;
  }

  // Get/Set EmptyPage
  unsigned long BPTreeHeader::GetEmptyPage(){
    return *m_emptyPage;
  }
  void BPTreeHeader::SetEmptyPage(unsigned long value){
    (*m_emptyPage) = value;
  }

  // Get/Set PageSize
  size_t BPTreeHeader::GetPageSize(){
    return *m_pageSize;
  }
  void BPTreeHeader::SetPageSize(size_t pageSize){
    //(*m_pageSize) = value;
  }

  // Get/Set ValueSize
  size_t BPTreeHeader::GetValueSize(){
    return *m_valueSize;
  }
  void BPTreeHeader::SetValueSize(size_t pageSize){
    //(*m_pageSize) = value;
  }

  // Get Bytes
  char* BPTreeHeader::GetBytes(){
    return m_bytes;
  }

  TreeHeaderMarker BPTreeHeader::GetMarker(){
    return (TreeHeaderMarker)*m_marker;
  }

  std::string BPTreeHeader::ToString() const
  {
      return "BPTreeHeader:\n"
             "Marker: " + to_string(*m_marker) + "\n"
             "PageSize: " + to_string(*m_pageSize) + "\n"
             "ValueSize: " + to_string(*m_valueSize) + "\n"
             "Root: " + to_string(*m_root) + "\n"
             "EmptyPage: " + to_string(*m_emptyPage) + "\n"
             "AlgebraID: " + to_string(*m_algebraID) + "\n"
             "TypeID: " + to_string(*m_typeID) + "\n"
             "Version: " + to_string(*m_version) + "\n";
  }
}
