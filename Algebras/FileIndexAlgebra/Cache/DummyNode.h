/*
Dummy-Knoten zum Testen des Caches

*/

#ifndef DUMMYNODE_H
#define DUMMYNODE_H

#include <vector>


class DummyNode
{
public:

  // bytes
  //  [0-3] value1
  //  [4-7] value2
  
  // ctor
  DummyNode( int value1, int value2 ) {
    _size = sizeof( int ) + sizeof( int );
  
    _bytes = new char[_size];

    SetValue1( value1 );
    SetValue2( value2 );
  }
  DummyNode( char* bytes ) : _bytes( bytes ) {
    _size = sizeof( int ) + sizeof( int );
  }

  void SetValue1( int v ) {
    size_t s = sizeof( int );

    char* sourceBytes = (char*)&v;
    char* targetBytes = &_bytes[0];

    SetBytes( sourceBytes, targetBytes, s );
  }
  void SetValue2( int v ) {
    size_t s = sizeof( int );

    char* sourceBytes = (char*)&v;
    char* targetBytes = &_bytes[s];

    SetBytes( sourceBytes, targetBytes, s );
  }
  void SetBytes( char* sourceBytes, char* targetBytes, int length ) {
    for( int i = 0; i < length; i++ ) {
      *targetBytes = *sourceBytes;

      targetBytes++;
      sourceBytes++;
    } // for
  }

  int GetValue1() {
    int* intPtr = (int*)&_bytes[0];
    return *intPtr;
  }
  int GetValue2() {
    size_t s = sizeof( int );

    int* intPtr = (int*)&_bytes[s];
    return *intPtr;
  }

  char* GetBytes() {
    return _bytes;
  }

  size_t GetSize() {
    return _size;
  }

private:
  char* _bytes;
  size_t _size;
};

#endif // DUMMYNODE_H












