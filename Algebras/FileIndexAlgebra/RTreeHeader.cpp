/*
class representing the header of a R-Tree-Index file

*/

#include "RTreeHeader.h"

#include <cstring>
#include <stdexcept>

using std::out_of_range;
using std::runtime_error;
using std::to_string;


namespace fialgebra{
  size_t RTreeHeader::constantSize =
    (2 * sizeof(char)) + (6 * sizeof(size_t));

  RTreeHeader::RTreeHeader(char* bytes, size_t length, size_t pageSize){
    if (length < constantSize){
      throw runtime_error("RTreeHeader(char*, size_t, size_t): "
                          "length < constantSize");
    }
    if (pageSize < constantSize){
      throw runtime_error("RTreeHeader(char*, size_t, size_t): "
                          "pageSize < constantSize");
    }
    else{
      m_bytes = bytes;
      m_marker    = m_bytes;
      m_pageSize  = (size_t*)(m_marker + 1);
      m_valueSize = m_pageSize + 1;
      m_root = m_valueSize + 1;
      m_emptyPage = m_root + 1;
      m_dimension = m_emptyPage + 1;
      m_minEntries = m_dimension + 1;
      m_version   = (char*)(m_minEntries + 1);
    }
  }

  RTreeHeader::RTreeHeader(size_t pageSize, size_t valueSize, size_t dimension,
                           size_t minEntries, size_t root, size_t emptyPage){
    if (pageSize < constantSize){
      throw runtime_error("RTreeHeader(size_t, size_t, size_t, size_t, "
                          "size_t, size_t): pageSize < constantSize");
    }
    else{
      m_bytes     = new char[pageSize];
      m_marker    = m_bytes;
      m_pageSize  = (size_t*)(m_marker + 1);
      m_valueSize = m_pageSize + 1;
      m_root      = m_valueSize + 1;
      m_emptyPage = m_root + 1;
      m_dimension = m_emptyPage + 1;
      m_minEntries = m_dimension + 1;
      m_version   = (char*)(m_minEntries + 1);

      (*m_marker)    = (char)TreeHeaderMarker::Rtree;
      (*m_pageSize)  = pageSize;
      (*m_valueSize) = valueSize;
      (*m_root)      = root;
      (*m_emptyPage) = emptyPage;
      (*m_dimension) = dimension;
      (*m_minEntries) = minEntries;
      (*m_version)   = currentVersion;
    }
  }
  
  
   /* This constructor is exlusively used by the RTree operator 
   * and checks the dimension of the RTree which is stored in 
   * passed file. If there is no valid RTree included an error 
   * message will be returned */
    RTreeHeader::RTreeHeader(char* bytes, size_t length, 
                 size_t pageSize, int a){
    if (length < constantSize){
      throw runtime_error("This is not a valid RTree!");
    }
    if (pageSize < constantSize){
      throw runtime_error("This is not a valid RTree!");
    }
    else{
      m_bytes = bytes;
      m_marker    = m_bytes;
      m_pageSize  = (size_t*)(m_marker + 1);
      m_valueSize = m_pageSize + 1;
      m_root = m_valueSize + 1;
      m_emptyPage = m_root + 1;
      m_dimension = m_emptyPage + 1;
      m_minEntries = m_dimension + 1;
      m_version   = (char*)(m_minEntries + 1);
    }
  }

  RTreeHeader::~RTreeHeader(){
    delete[](m_bytes);
  }

  TreeHeaderMarker RTreeHeader::GetMarker(){
    return (TreeHeaderMarker)*m_marker;
  }

  void RTreeHeader::SetMarker(TreeHeaderMarker value){
    (*m_marker) = (char)value;
  }

  char RTreeHeader::GetVersion(){
    return *m_version;
  }

  void RTreeHeader::SetPageSize(size_t pageSize){
    (*m_pageSize) = pageSize;
  }
  size_t RTreeHeader::GetPageSize(){
    return *m_pageSize;
  }

  void RTreeHeader::SetValueSize(size_t valueSize){
    (*m_valueSize) = valueSize;
  }

  size_t RTreeHeader::GetValueSize(){
    return *m_valueSize;
  }

  size_t RTreeHeader::GetDimension(){
    return *m_dimension;
  }

  void RTreeHeader::SetDimension(size_t dimension){
    (*m_dimension) = dimension;
  }

  size_t RTreeHeader::GetMinEntries(){
    return *m_minEntries;
  }
  void RTreeHeader::SetMinEntries(size_t minEntries){
    *m_minEntries = minEntries;
  }

  size_t RTreeHeader::GetRoot(){
    return *m_root;
  }
  void RTreeHeader::SetRoot(size_t value){
    (*m_root) = value;
  }

  size_t RTreeHeader::GetEmptyPage(){
    return *m_emptyPage;
  }
  void RTreeHeader::SetEmptyPage(size_t value){
    (*m_emptyPage) = value;
  }

  char* RTreeHeader::GetBytes(){
    return m_bytes;
  }

  std::string RTreeHeader::ToString() const
  {
      return "RTreeHeader:\n"
             "Marker: " + to_string(*m_marker) + "\n"
             "PageSize: " + to_string(*m_pageSize) + "\n"
             "ValueSize: " + to_string(*m_valueSize) + "\n"
             "Root: " + to_string(*m_root) + "\n"
             "EmptyPage: " + to_string(*m_emptyPage) + "\n"
             "Dimension: " + to_string(*m_dimension) + "\n"
             "MinEntries: " + to_string(*m_minEntries) + "\n"
             "Version: " + to_string(*m_version) + "\n";
  }
}
