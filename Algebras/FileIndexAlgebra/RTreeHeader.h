/*
class representing the header of a R-Tree-Index file

*/
#ifndef RTREEHEADER_H
#define RTREEHEADER_H

#include <string>

#include "TreeHeaderMarker.h"


namespace fialgebra{
  class RTreeHeader{
  public:
  RTreeHeader(char* bytes, size_t length, size_t pageSize);
  RTreeHeader(size_t pageSize, size_t valueSize, size_t dimension,
              size_t minEntries, size_t root, size_t emptyPage);
  RTreeHeader(char* bytes, size_t length, size_t pageSize, int a);

  ~RTreeHeader();

    // Get Version
    char GetVersion();

    // Get/Set Root
    size_t GetRoot();
    void SetRoot(size_t value);

    void SetPageSize (size_t pageSize);
    size_t GetPageSize();

    void SetDimension(size_t value);
    size_t GetDimension();

    void SetValueSize (size_t valueSize);
    size_t GetValueSize();

    size_t GetMinEntries();
    void SetMinEntries(size_t minEntries);

    size_t GetEmptyPage();
    void SetEmptyPage(size_t value);

    char* GetBytes();

    static size_t GetHeaderSize() { return constantSize; }

    TreeHeaderMarker GetMarker();
    void SetMarker(TreeHeaderMarker value);

    std::string ToString() const;

  private:

    // Groesse des Headers
    static size_t constantSize;
    const char currentVersion = 1;

    // Inhalt des Headers als bytes (char*)
    char* m_bytes;
    // Marker, der den Type des Index angibt.
    // sollte im RTree-Baum TreeHeaderMarker::Rtree entsprechen
    char* m_marker;
    // Version der Struktur des Indexes. Die Struktur
    // kann sich theoretisch in spaeteren Version
    // aendern, deshalb hier die Info.
    char* m_version;

    // Seitengroesse
    size_t* m_pageSize,
          * m_valueSize;

    //Mindestanzahl an Einträgen in einem Knoten
    size_t* m_minEntries;

    //Dimension von indexierten Daten
    size_t* m_dimension;

    // Nummer des Wurzel-Knotens
    size_t* m_root;
    // Nummer der ersten Seite der Freiseitenliste
    size_t* m_emptyPage;
  };
}
#endif // RTREEHEADER_H
