
/*
Sept. 2008, M. Spiekermann

Some functions which support the implementation of ~Serialization~ for
attribute data types. For documentation and example usage refer to the 
files Attribute.h and StandardTypes.h

*/

#ifndef SEC_SERIALIZE_H
#define SEC_SERIALIZE_H

#include <string>
#include <sstream>

using namespace std;

/*
Auxiliary functions for debugging

*/

static string Array2HexStr(char* data, size_t size, size_t offset = 0)
{
  stringstream res;

  size_t i = 0;
  while ( i < size )
  {
    res.width(2);
    res.fill('0');    
    res << hex 
	<< (static_cast<unsigned int>( data[i+offset] ) & 255) << " ";
    i++;
  }  
  return res.str();  
} 

template<class T>
void ShowData(T v)
{
  Array2HexStr( (char*)&v , sizeof(T), 0 );
}

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
  memcpy( &value, &state[offset], len );
  offset += len;
}


/*
Some specializations which reduce the data to be transferred

*/


template<>
inline void WriteVar<const bool>(const bool& value, 
		                   char* storage, size_t& offset) 
{
  storage[offset] = value ? '1' : '0';
  offset += 1;
}	

template<>
inline void ReadVar<bool>(bool& value, char* state, size_t& offset) 
{
  value = (state[offset] == '0') ? false : true;
  offset += 1;
}	

#endif
