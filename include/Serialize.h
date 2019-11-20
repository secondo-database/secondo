
/*
Sept. 2008, M. Spiekermann

Some functions which support the implementation of ~Serialization~ for
attribute data types. For documentation and example usage refer to the 
files Attribute.h and StandardTypes.h

*/

#ifndef SEC_SERIALIZE_H
#define SEC_SERIALIZE_H

#include <string.h>

/*
Templates for reading and writing member variables from/to a memory block

*/   

template<class T>
inline void WriteVar(const T& value, char* storage, size_t& offset) 
{
  static size_t len = sizeof(T);
  memcpy( &storage[offset], &value, len );
  offset += len;
}


template<class T>
inline void ReadVar(T& value, char* state, size_t& offset) 
{
  static size_t len = sizeof(T);
  memcpy( (void*) &value, &state[offset], len );
  offset += len;
}


/*
Some specializations which ensure one byte for a bool value. 

*/


template<>
inline void WriteVar<bool>(const bool& value, char* storage, size_t& offset) 
{
  storage[offset] = (value ? '1' : '0');
  offset += 1;
}

template<>
inline void ReadVar<bool>(bool& value, char* state, size_t& offset) 
{
  value = ((state[offset] == '0') ? false : true);
  offset += 1;
}

template<>
inline void WriteVar<unsigned char>(const unsigned char& value, 
                                    char* storage, 
                                    size_t& offset) 
{
  storage[offset] = value;
  offset += 1;
}

template<>
inline void ReadVar<unsigned char>(unsigned char& value, 
                                   char* state, 
                                   size_t& offset) 
{
  value = state[offset];
  offset += 1;
}

#endif
