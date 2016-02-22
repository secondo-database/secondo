/*
class representing the header of a B+ Tree-Index file

*/

#ifndef BPTREEHEADER_H
#define BPTREEHEADER_H

#include <string>
#include <cstddef>             // size_t
#include "../TreeHeaderMarker.h"


namespace fialgebra{

  class BPTreeHeader{
  public:
    BPTreeHeader(char* bytes, size_t length, size_t pageSize);
    BPTreeHeader(size_t pageSize,
                 size_t valueSize,
                 unsigned int algebraID,
                 unsigned int typeID,
                 unsigned long root,
                 unsigned long emptyPage);
    ~BPTreeHeader();

    // Get Version
    char GetVersion();

    // Get/Set AlgebraID
    unsigned int GetAlgebraID();
    void SetAlgebraID(unsigned int value);
    // Get/Set TypeID
    unsigned int GetTypeID();
    void SetTypeID(unsigned int value);

    // Get/Set Root
    unsigned long GetRoot();
    void SetRoot(unsigned long value);

    // Get/Set EmptyPage
    unsigned long GetEmptyPage();
    void SetEmptyPage(unsigned long value);

    // Get/Set PageSize
    size_t GetPageSize();
    void SetPageSize (size_t pageSize);

    size_t GetValueSize();
    void SetValueSize(size_t valueSize);

    // Get Bytes
    char* GetBytes();

    // Get HeaderSize (constantSize)
    static size_t GetHeaderSize() { return constantSize; }

    std::string ToString() const;


     TreeHeaderMarker GetMarker();


  private:
    // Groesse des Headers
    static size_t constantSize;
    const char currentVersion = 1;

    // Inhalt des Headers als bytes (char*)
    char* m_bytes;
    // Marker, der den Type des Index angibt.
    // sollte im B+-Tree TreeHeaderMarker::BPlusTree entsprechen
    char* m_marker;
    // Version der Struktur des Indexes. Die Struktur
    // kann sich theoretisch in spaeteren Version
    // aendern, deshalb hier die Info.
    char* m_version;

    // Seitengroesse / Attributgröße
    size_t* m_pageSize,
      * m_valueSize;

    // Algebra- und Type-ID des Datentypes, der
    // indiziert wird.
    unsigned int* m_algebraID;
    unsigned int* m_typeID;

    // Nummer des Wurzel-Knotens
    unsigned long* m_root;
    // Nummer der ersten Seite der Freiseitenliste
    unsigned long* m_emptyPage;
  };
}

#endif // BPTREEHEADER_H
