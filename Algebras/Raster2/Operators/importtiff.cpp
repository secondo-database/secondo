
/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//[mul] [$\cdot$]
//[<] [$<$]


*/




#include <iostream>
#include <fstream>
#include <stdint.h>
#include <vector>
#include <sstream>
#include <string.h>
#include "AlgebraTypes.h"
#include "../sint.h"


using namespace std;
using namespace raster2;


namespace  tiffimport{


/*
1 class unpack

This class provides some unpacking algorithms used in TIFF files.

*/
class unpack{
 public:


/*
1.1. unpackData

This function unpacks the data stored in buffer and returns them.
The length of the buffer is given by bufferSize. The targetSize is 
used for the targetSize. If the targetSize is set to be zero, it is 
detected automatically and set in the in/out parameter targetSize.
The parameter compression is the code of the compression used in the 
TIFF-file. If a problem occurs, e.g., an unknown or not implemented compression,
the result will be null. In case of success, the caller of this function 
has to delete the result of this function.

*/
  static char* unpackData(char* buffer, size_t bufferSize, 
                          size_t& targetSize, uint32_t compression){
      switch(compression){
         case 1: // uncompressed 
                 {  if(targetSize > bufferSize || targetSize==0){
                      targetSize = bufferSize;
                    }
                    char* res = new char[targetSize];
                    memcpy(res, buffer, targetSize);
                    return res;
                 }
        case 2: // CCITT_RLE
                return unpack_CCITT_RLE(buffer, bufferSize, targetSize);
        case 3: // CCITT_T4           
                return 0; // not implemented yet
        case 4: // CCITT_T6           
                return 0;
        case 5: //LZW
                return 0;
        case 6: // OJPEG
                return 0;
        case 7: // JPEG
                return 0;
        case 32766: // NEXT
                return 0;
        case 32773: // packbits
                return unpack_PACKBITS(buffer, bufferSize, targetSize);
        case 32809: // THUNDERSCAN
                 return 0;
        case 32895: // IT8CTPAD
                 return 0;
        case 32896: // IT8LW
                 return 0;
        case 32897: // IT8MP
                 return 0;
        case 32898: // IT8BL
                 return 0;
        case 32908: // PIXARFILM
                 return 0;
        case 32909: // PIXARLOG
                 return 0;
        case 32946: // DEFLATE
                 return 0;
        case 8: // ADOBE_DEFLATE
                 return 0;
        case 32947: // DCS
                 return 0;
        case 34661: // JBIG
                 return 0;
        case 34676: // SGILOG
                 return 0;
        case 34677: // SGILOG24
                 return 0;
        case 34712: // JP2000
                 return 0;
        default: return 0;  // unknown compression method
      };

  } 



 private:


  static char* unpack_CCITT_RLE(char* buffer, const size_t buffersize, 
                                size_t& targetsize){
      cout << "Compression 2 not implemented yet" << endl;
      return 0;
  }


  static char* unpack_PACKBITS(char* buffer, const size_t bufferSize, 
                               size_t& targetSize){
     if(targetSize==0){ // determine targetsize  
       if(bufferSize==0){
         return 0;
       }
       size_t pos = 0;
       while(pos < bufferSize){
          char n = buffer[pos];
          pos++;
          if(0<=n && n<=127){
             pos += (n+1);
             targetSize += n+1;
          } else if(n>=-127 && n<=-1){
            pos++; // overjump the next byte
            targetSize += (-n+1);
          } else if(n==-128){
             ; //noop
          }
       }
     }
    
    size_t pos = 0;
    size_t tpos = 0;
    char* target = new char[targetSize];
    while(tpos < targetSize){
      char n = buffer[pos];
      pos++;
      if(0<=n && n<=127){
         memcpy(target+tpos, buffer+pos, n+1);
         tpos += (n+1);
         pos += (n+1);
      } else if(-127<=n && n<=-1){
         char next = buffer[pos];
         pos++;
         memset(target+tpos, next, (-n+1));
         tpos += (-n+1);
      } else if(n==-128){
         ; // noop
      }
    }
    if(pos>bufferSize+1){
       cerr << "wrong bufferSize, expected " 
            << bufferSize << ", read " << (pos-1) << endl;
    }
    return target;
  }

};






/*
2 Auxiliary function

2.1  convertEndian

This function converts a number from big endian to little endian and vice versa.

*/
template<class T>
T convertEndian(unsigned char* input){
   size_t size = sizeof(T);
   T res = 0;
   for(size_t i=0;i<size;i++){
      ((char*)&res)[i] = input[(size-1) -i];
   }
   return res;
}

/*
2.2 getNumber

This function reads a number from an byte array. 
If convertEndian is set to true, the endian format is reversed.

*/
template<class T> 
T getNumber(unsigned char* input, bool _convertEndian){
   if( _convertEndian){
       return convertEndian<T>(input);
   } else {
      T res = *((T*)&input[0]);
      return res;
   }
}


/*
2.3. getLittleEndian

Returns the number represented by a byte array in little endian order.

*/
  template<class T>
  T getLittleEndian(unsigned char* input){
     T res = 0;
     for(int i=0;i<sizeof(T);i++){
        res = res *256 + input[i]; 
     }
     return res;
  }
 
/*
2.4. convertEndianFun

This function reverses the order of bytes in an byte array of given size.

*/
  void convertEndianFun(unsigned char* input,int size){
     for(int i=0;i<size/2;i++){
         char tmp = input[i];
         input[i] = input[(size-1)-i];
         input[(size-1)-i] = tmp;
     }  
  }



/*
3 class rational

This class is an implementation of the rational type used in tiff files.

*/

class rational{

  public:
     rational(uint32_t n, uint32_t d): nom(n), denom(d) {}
     rational(): nom(0), denom(1) {};
     ostream& print(ostream& out) const{
         out << nom << "/" << denom;
         return out;
     }

     double toDouble() const{
       return (1.0*nom) / (1.0*denom);
     }
  private:
    uint32_t nom;
    uint32_t denom;
};


rational getRational(unsigned char* field, bool convertEndian){

    uint32_t n = getNumber<uint32_t>(field,convertEndian);
    uint32_t d = getNumber<uint32_t>(field+4,convertEndian);
    return rational(n,d);
}

ostream& operator<<(ostream& o, const rational& r){
   return r.print(o);
}



/*
4. Class srational

This class is an implementation of the signed rational type used in tiff files.


*/
 
class srational{

  public:
     srational(int32_t n, int32_t d): nom(n), denom(d) {}
     srational(): nom(0), denom(1) {};
     ostream& print(ostream& out)const{
         out << nom << "/" << denom;
         return out;
     }
     double toDouble() const{
       return (1.0*nom) / (1.0*denom);
     }
  private:
    int32_t nom;
    int32_t denom;
};

srational getSRational(unsigned char* field, bool convertEndian){

    int32_t n = getNumber<int32_t>(field,convertEndian);
    int32_t d = getNumber<int32_t>(field+4,convertEndian);
    return srational(n,d);
}

ostream& operator<<(ostream& o, const srational& r){
   return r.print(o);
}



/*
5. Class Rectangle

This class represents a rectangle and can be used to 
store the bounding box of a geotiff-file.

*/

class Rectangle{
public:
    Rectangle(double _x1, double _y1, double _x2, double _y2):
      x1(_x1), y1(_y1), x2(_x2), y2(_y2), defined(true) {
      }

    Rectangle(): x1(0), y1(0), x2(0),y2(0), defined(false){

     }    
    Rectangle(bool def): x1(0), y1(0), x2(0),y2(0), defined(def){
     }    


    ostream& print(ostream& o)const{
       if(!defined){
           o << "undefined" << endl;
       } else {
           o << "(" << x1 << ", " << y1 << ")->(" << x2 << ", " << y2 << ")";
       }
       return o;
    }

    bool isDefined() const{
       return defined;
    }

    double getX1() const{ return x1; }   
    double getY1() const{ return y1; }   
    double getX2() const{ return x2; }   
    double getY2() const{ return y2; }   


private:
   double x1;
   double y1;
   double x2;
   double y2;
   bool defined;
};

ostream& operator<<(ostream& o, const Rectangle& r){
   return r.print(o);
}



/*
6. class IFDEntry

This class represents an image file directory entry. It consists of
 - a tag specifiing the semantic of the entry coded as an integer
 - a type specifiing the type of the entries as an integer
 - count: the number of values (integer)
 - values, a list of ~count~ values of type ~type~

*/
class IFDEntry{

  public:

/*
6.1 Constructor

This constructor creates a new directory entry with given endian order.

*/
    IFDEntry( const bool _convertEndian):
       convertEndian(_convertEndian), tag(0),
       type(0), count(0), offset(0),
       byteValues(0), asciiValues(""), shortValues(0),
       longValues(0), ratValues(0), sbyteValues(0),
       undefValues(0), sshortValues(0), slongValues(0),
       sratValues(0), floatValues(0), doubleValues(0) {}

/*
6.2 Constructor

The usual copy constructor

*/
     IFDEntry(const IFDEntry& e):
       convertEndian(false), tag(0),
       type(0), count(0), offset(0),
       byteValues(0), asciiValues(""), shortValues(0),
       longValues(0), ratValues(0), sbyteValues(0),
       undefValues(0), sshortValues(0), slongValues(0),
       sratValues(0), floatValues(0), doubleValues(0) {
       readFrom(e);
     }

/*
6.3 Assignment operator.

*/
     IFDEntry& operator=(const IFDEntry& e){
        readFrom(e);
        return *this;
     }

/*
6.4 readFrom

A function taking over the values from another entry.

*/

     void readFrom(const IFDEntry& e){
       convertEndian = e.convertEndian;
       tag = e.tag;
       type = e.type;
       count = e.count;
       offset = e.offset;
       if(byteValues) delete[] byteValues; 
       byteValues=0;
       asciiValues ="";
       if(shortValues) delete[] shortValues; 
       shortValues=0;
       if(longValues) delete[] longValues; 
       longValues=0;
       if(ratValues) delete[] ratValues; 
       ratValues=0;
       if(sbyteValues) delete[] sbyteValues; 
       sbyteValues=0;
       if(undefValues) delete[] undefValues; 
       undefValues=0;
       if(sshortValues) delete[] sshortValues; 
       sshortValues=0;
       if(slongValues) delete[] slongValues; 
       slongValues=0;
       if(sratValues) delete[] sratValues; 
       sratValues=0;
       if(floatValues) delete[] floatValues; 
       floatValues=0;
       if(doubleValues) delete[] doubleValues; 
       doubleValues=0;
       size_t s = getTypeSize(type);
    
       switch(type){
            case 1:  if(e.byteValues){
                        byteValues = new uint8_t[count];
                        memcpy((char*) byteValues, e.byteValues, count*s);
                     }
                     break; // byte
            case 2:  asciiValues = e.asciiValues;
                     break; // ascii
            case 3:  if(e.shortValues){
                         shortValues = new uint16_t[count]; 
                         memcpy((char*) shortValues, e.shortValues, count*s);
                      }
                      break; // short
                      
            case 4:  if(e.longValues) { 
                        longValues  = new uint32_t[count]; 
                        memcpy((char*) longValues, e.longValues, count*s);
                     }
                     break; // long
            case 5:  if(e.ratValues) { 
                        ratValues  = new rational[count]; 
                        memcpy((char*) ratValues, e.ratValues, count*s);
                     }
                     break;  // rational 
            case 6:  if(e.sbyteValues) { 
                        sbyteValues  = new int8_t[count]; 
                        memcpy((char*) sbyteValues, e.sbyteValues, count*s);
                     }
                     break;  // sbyte
            case 7:  if(e.undefValues) { 
                        undefValues  = new int8_t[count]; 
                        memcpy((char*) undefValues, e.undefValues, count*s);
                     }
                     break; // undefined
            case 8 : if(e.sshortValues) { 
                        sshortValues  = new int16_t[count]; 
                        memcpy((char*) sshortValues, e.sshortValues, count*s);
                     }
                     break; // sshort
            case 9 : if(e.slongValues) { 
                        slongValues  = new int32_t[count];
                        memcpy((char*) slongValues, e.slongValues, count*s);
                     }
                     break; // slong
            case 10: if(e.sratValues) { 
                        sratValues  = new srational[count]; 
                        memcpy((char*) sratValues, e.sratValues, count*s);
                     }
                     break;  // srational
            case 11 : if(e.floatValues) { 
                         floatValues  = new float[count]; 
                         memcpy((char*) floatValues, e.floatValues, count*s); 
                      } 
                      break; // float
            case 12 : if(e.doubleValues) { 
                         doubleValues  = new double[count]; 
                         memcpy((char*) doubleValues, e.doubleValues, count*s);
                      }
                      break; // double
       }
       
      
     }


/*
6.5. Destructor

*/

     ~IFDEntry(){
       if(byteValues) delete[] byteValues; 
       if(shortValues) delete[] shortValues;
       if(longValues) delete[] longValues;
       if(ratValues) delete[] ratValues;
       if(sbyteValues) delete[] sbyteValues;
       if(undefValues) delete[] undefValues;
       if(sshortValues) delete[] sshortValues;
       if(slongValues) delete[] slongValues;
       if(sratValues) delete[] sratValues;
       if(floatValues) delete[] floatValues;
       if(doubleValues) delete[] doubleValues;   
      }


/*
6.6 Read

This functions read an entry from an input file stream starting at given offset.

*/
    bool read(ifstream& in, size_t offset){
       in.seekg(offset,ios::beg);
       if(!in.good()){
          return false;
       }
       read(in);
       readValues(in); 
    }

/*
6.7 read

This function reads the content of this entry from an input stream starting 
at the current file position.

*/
    bool read(ifstream& in) {
       unsigned char content[12];
       in.read((char*)content,12);
       if(!in.good()){
          return false;
       }
       read(content);
       return true; 
    }



/*
6.8 Some access methods

6.8.1 hasTag

Check whether the tag within this entry is equal to the given one.

*/

  bool hasTag(uint16_t tag) const{
    return this->tag == tag;
  }

/*
6.8.2 hasIntValue

CHecks whether this entry has some integer values, e.g., if
the type is byte, short, long, sbyte, ... 

*/
  bool hasIntValue() const{
      return     type == 1 || type == 3 || type == 4 
              || type == 6 || type == 8 || type == 9;
  }
  

/*
6.8.3 getIntValue

Returns the integer value at position nr. The type of this entry 
must be one of the integer types. 

*/
  int getIntValue(const size_t nr) const{
     if(nr < 0 || nr >=count){
         throw(5);
     } 
     switch(type){
        case 1 : return byteValues[nr];
        case 3 : return shortValues[nr];
        case 4 : return longValues[nr];
        case 6 : return sbyteValues[nr];
        case 8 : return sshortValues[nr];
        case 9 : return slongValues[nr];
        default: throw(5);
     }
     return  -1;
  }

/*
6.8.4 hasDoubleValue

This function returns true if the type of this entry is 
rational, srational, float or double.

*/
  bool hasDoubleValue() const{
      return     type == 5 || type == 10 || type == 11 
              || type == 12 ;
  }

/*
6.8.5 getDoubleValue

Returns the value at position nr for double-entries.

*/
  double getDoubleValue(const size_t nr) const{
     if(nr < 0 || nr >=count){
         throw(5);
     } 
     switch(type){
        case 5 : return ratValues[nr].toDouble();
        case 10 : return sratValues[nr].toDouble();
        case 11 : return floatValues[nr];
        case 12 : return doubleValues[nr];
        default: throw(5);
     }
     return  -1;
  }

/*
~getTag~

Returns the tag.

*/

  uint16_t getTag() const{
    return tag;
  }

/*
~getCount~

Returns the number of values.

*/

  uint16_t getCount() const{
     return count;
  }

/*
~getType~

returns the type of this entry.

*/
  uint16_t getType() const{
     return type;
  }


/*
6.9 readValues

This function fills up one of the value arrays.
Values are stored in the offset if size[mul]count [<] sizeof(offset). 
Otherwise, the values are stored at position offset in the input stream.
In this case, the values are read from the stream.

*/

    void readValues(ifstream& in){
         // create the appropriate array
         int size = getTypeSize(type);
         switch(type){
            case 1:  byteValues = new uint8_t[count]; break; // byte
            case 2:  asciiValues = ""; break; // ascii
            case 3:  shortValues = new uint16_t[count]; break; // short
            case 4:  longValues = new uint32_t[count]; break; // long
            case 5:  ratValues = new rational[count]; break;  // rational 
            case 6:  sbyteValues = new int8_t[count]; break;  // sbyte
            case 7:  undefValues = new int8_t[count]; break; // undefined
            case 8 : sshortValues = new int16_t[count]; break; // sshort
            case 9 : slongValues = new int32_t[count];break; // slong
            case 10: sratValues = new srational[count]; break;  // srational
            case 11 : floatValues = new float[count]; break; // float
            case 12 : doubleValues = new double[count]; break; // double
            default: cout << "invalid type" << endl; return;
         }
         if(size*count <=4){  // values stored in offset field
            unsigned char a[4];
            memcpy(a,&offset,4);
            if(type!=2){
               for(size_t i=0;i<count;i++){
                  storeValue(a+size*i,i,type);
               }
            } else {
               storeValue(a,0,type);
            }
         } else {
            in.seekg(offset,ios::beg); // goto data
            unsigned char* buffer = new unsigned char[size*count];
            in.read((char*) buffer,size*count);
            if(type!=2){
               for(size_t i=0;i<count;i++){
                   storeValue(buffer+i*size,i,type); 
               }
            } else {
                storeValue(buffer,0,type);
            }
            delete[] buffer;
         }
    }
     
    IFDEntry copy()const{
       IFDEntry res(*this);
       return res;
     }


    ostream& print(ostream& out) const{
       out << "tag: " << getTagDescr(tag) << ", type: " 
           << getTypeDescr(type) << ", count: " 
           << count << " values: " << getValueDescr(); 
      
       return out; 
    }


  private:
     bool convertEndian;   // endian conversion required?
     uint16_t tag;         // the tag
     uint16_t type;        // the type
     uint32_t count;       // the number of values
     uint32_t offset;      // the offset to the values or the values themself
     
     // for each type, an array able to store the values is used
     // at most one of the following arrays is non-null
     // which one depend on the type
     uint8_t* byteValues;
     string asciiValues;
     uint16_t* shortValues;
     uint32_t* longValues;
     rational* ratValues;
     int8_t* sbyteValues;
     int8_t* undefValues;
     int16_t* sshortValues;
     int32_t* slongValues;
     srational* sratValues;
     float* floatValues;
     double* doubleValues;   



     IFDEntry() {}


/*
5.5 Reads the content of the entry from a 12 byte sized array.

The buffer is organized as follows

  bytes   meaning
  0-1     tag
  2-3     type
  4-7     count
  8-11    offset if size[mul]count >4 or size==0
          values otherwise

*/
     void read(unsigned char* content){
       tag = getNumber<uint16_t>(content,convertEndian);
       type = getNumber<uint16_t>(content+2, convertEndian);
       count = getNumber<uint32_t>(content+4, convertEndian);
       int size = getTypeSize(type);
       if((size*count > 4) || (size==0)){
           offset = getNumber<uint32_t>(content+8, convertEndian);
       } else {
           if(convertEndian){
             for(size_t i=0;i<count;i++){
                convertEndianFun(content+(8+i*size),size);
             } 
           }
           offset = getNumber<uint32_t>(content+8,false);
       }
     }



/*
5.6 getTypeDescr

Returns a string representation of the type number.

*/
     string getTypeDescr(uint16_t type)const{
         switch(type){
            case 1:  return "BYTE";
            case 2:  return "ASCII";
            case 3:  return "SHORT";
            case 4:  return "LONG";
            case 5:  return "RATIONAL";
            case 6:  return "SBYTE";
            case 7:  return "UNDEFINED";
            case 8 : return "SSHORT";
            case 9 : return "SLONG";
            case 10: return "SRATIONAL";
            case 11 : return "FLOAT";
            case 12 : return "DOUBLE";
            default : stringstream ss; ss << type; return ss.str();
         }
     }

/*
This function returns a textual description of all values inside this entry.


*/
     string getValueDescr() const{

        if(count > 128){
            return "BIG DATA";
        }
        stringstream ss;
        if(byteValues)  { 
          for(size_t i=0;i<count;i++) { 
              if(i>0){ 
                ss << ", ";
              } 
              ss << byteValues[i]; 
          }
        }
        if(type==2){
           ss << asciiValues;
        } 
        if(shortValues)  { 
             for(size_t i=0;i<count;i++) {
                 if(i>0){ 
                    ss << ", ";
                 } 
                 ss << shortValues[i];
              }
        }
        if(longValues)  { 
           for(size_t i=0;i<count;i++) { 
             if(i>0){ 
                ss << ", ";
             } 
             ss << longValues[i]; 
           }
        }
        if(ratValues)  { 
             for(size_t i=0;i<count;i++) { 
               if(i>0){ 
                 ss << ", ";
               } 
               ss << ratValues[i];
             }
        }
        if(sbyteValues)  { 
             for(size_t i=0;i<count;i++) { 
                if(i>0){ 
                   ss << ", ";
                } 
                ss << sbyteValues[i]; 
             }
        }
        if(undefValues)  { 
           for(size_t i=0;i<count;i++) { 
              if(i>0){ 
                ss << ", ";
              } 
              ss << undefValues[i];
           }
        }
        if(sshortValues)  { 
           for(size_t i=0;i<count;i++) { 
              if(i>0){ 
                ss << ", ";
              }
              ss << sshortValues[i];
           }
        }
        if(slongValues)  { 
            for(size_t i=0;i<count;i++) { 
               if(i>0){ 
                  ss << ", ";
               } 
              ss << slongValues[i]; 
            }
        }
        if(sratValues)  { 
           for(size_t i=0;i<count;i++) { 
              if(i>0){ 
                ss << ", ";
              } 
              ss << sratValues[i];
            }
        }
        if(floatValues) { 
            for(size_t i=0;i<count;i++) { 
                if(i>0){ 
                  ss << ", ";
                 } 
                ss << floatValues[i];
            }
         }
         if(doubleValues)  { 
             for(size_t i=0;i<count;i++) { 
                if(i>0){ 
                   ss << ", ";
                } 
                ss << doubleValues[i]; 
             }
         }  
         return ss.str(); 
     }     


/*
5.8 convert

Returns the number of type T created from an byte array without using the
system wide endian order.

*/
    template<class T>
    T convert(unsigned char* field){
       return *( (T*) &field[0]);
    }


   void storeValue(unsigned char* field, int pos, uint16_t type){
         switch(type){
            case 1:  byteValues[pos] = 
                              getNumber<uint8_t>(field,convertEndian); 
                     break;
            case 2: {  string v((char*) field,count); 
                       asciiValues = v;
                     } 
                     break;
            case 3:  shortValues[pos] = 
                              getNumber<uint16_t>(field,convertEndian); 
                     break; // short
            case 4:  longValues[pos]  = 
                              getNumber<uint32_t>(field,convertEndian);
                     break; // long
            case 5:  ratValues[pos] = 
                                getRational(field,convertEndian); 
                     break;  // rational 
            case 6:  sbyteValues[pos] = 
                                getNumber<int8_t>(field,convertEndian); 
                     break;  // sbyte
            case 7:  undefValues[pos] = 
                               getNumber<int8_t>(field,convertEndian); 
                     break; // undefined
            case 8 : sshortValues[pos] = 
                              getNumber<int16_t>(field,convertEndian); 
                     break; // sshort
            case 9 : slongValues[pos] = 
                              getNumber<int32_t>(field,convertEndian);
                     break; // slong
            case 10: sratValues[pos] = 
                             getSRational(field,convertEndian); 
                     break;  // srational
            case 11 : floatValues[pos] = 
                             getNumber<float>(field,convertEndian); 
                      break; // float
            case 12 : doubleValues[pos] = 
                             getNumber<double>(field,convertEndian); 
                      break; // double
         }
   }



/*
5.11 getTypeSize

Returns the size of a numeric coded type.

*/
     uint16_t getTypeSize(uint16_t type){
         switch(type){
            case 1:  return 1; // byte
            case 2:  return 1; // ascii
            case 3:  return 2; // short
            case 4:  return 4; // long
            case 5:  return 8; // rational
            case 6:  return 1; // sbyte
            case 7:  return 1; // undefined
            case 8 : return 2; // sshort
            case 9 : return 4; // slong
            case 10: return 8; // srational
            case 11 : return 4; // float
            case 12 : return 8; // double
            default : return 0;
         }
     }


/*
5.13 getTagDescr

This function converts a numeric tag into human readable format.


*/
     string getTagDescr(uint16_t tag) const{
       switch(tag){
         case 254 : return "NewSubfileType";
         case 255 : return "SubfileType";
         case 256 : return "ImageWidth";
         case 257 : return "ImageLength";
         case 258 : return "BitsPerSample";
         case 259 : return "Compression";
         case 262 : return "PhotometricInterpretation";
         case 263 : return "Threshholding";
         case 264 : return "CellWidth";
         case 265 : return "CellLength";
         case 266 : return "FillOrder";
         case 269 : return "DocumentName";
         case 270 : return "ImageDescription";
         case 271 : return  "Make";
         case 272 : return "Model";
         case 273 : return "StripOffsets";
         case 274 : return "Orientation";
         case 277 : return "SamplesPerPixel";
         case 278 : return "RowsPerStrip";
         case 279 : return "StripByteCounts";
         case 280 : return "MinSampleValue";
         case 281 : return "MaxSampleValue";
         case 282 : return "XResolution";
         case 283 : return "YResolution";
         case 284 : return "PlanarConfiguration";
         case 285 : return "PageName";
         case 286 : return "XPosition";
         case 287 : return "YPosition";
         case 288 : return "FreeOffsets";
         case 289 : return "FreeByteCounts";
         case 290 : return "GrayResponseUnit";
         case 291 : return "GrayResponseCurve";
         case 292 : return "T4Options";
         case 293 : return "T6Options";
         case 296 : return "ResolutionUnit";
         case 297 : return "PageNumber";
         case 301 : return "TransferFunction";
         case 305 : return "Software";
         case 306 : return "DateTime";
         case 315 : return "Artist";
         case 316 : return "HostComputer";
         case 317 : return "Predictor";
         case 318 : return "WhitePoint";
         case 319 : return "PrimaryChromaticities";
         case 320 : return "ColorMap";
         case 321 : return "HalftoneHints";
         case 322 : return "TileWidth";
         case 323 : return "TileLength";
         case 324 : return "TileOffsets";
         case 325 : return "TileByteCounts";
         case 332 : return "InkSet";
         case 333 : return "InkNames";
         case 334 : return "NumberOfInks";
         case 336 : return "DotRange";
         case 337 : return "TargetPrinter";
         case 338 : return "ExtraSamples";
         case 339 : return "SampleFormat";
         case 340 : return "SMinSampleValue";
         case 341 : return "SMaxSampleValue";
         case 342 : return "TransferRange";
         case 512 : return "JPEGProc";
         case 513 : return "JPEGInterchangeFormat";
         case 514 : return "JPEGInterchangeFormatLength";
         case 515 : return "JPEGRestartInterval";
         case 517 : return "JPEGLossLessPredictors";
         case 518 : return "JPEGPointTransforms";
         case 519 : return "JPEGQTables";
         case 520 : return "JPEGDCTables";
         case 521 : return "JPEGACTables";
         case 529 : return "YCbCrCoefficients";
         case 530 : return "YCbCrSubSampling";
         case 531 : return "YCbCrPositioning";
         case 532 : return "ReferenceBlackWhite";
         case 33432 : return "Copyright";
         case 32932 : return "WangAnnotation";
         case 33445 : return "MDFileTag";
         case 33446 : return "MDScalePixel";
         case 33447 : return "MDColorTable";
         case 33448 : return "MDLabName";
         case 33449 : return "MDSampleInfo";
         case 33450 : return "MDPrepDate";
         case 33451 : return "MDPrepTime";
         case 33452 : return "MDFileUnits";
         case 33550 : return "ModelPixelScaleTag";
         case 33723 : return "ITPC";
         case 33918 : return "INGRPacketDataTag";
         case 33919 : return "INGRFlagRegisters";
         case 33920 : return "IrasBTransformationMatrix";
         case 33922 : return "ModelTiepointTag";
         case 34264 : return "ModelTransformationTag";
         case 34377 : return "Photoshop";
         case 34664 : return "ExifIFD";
         case 34675 : return "ICCProfile";
         case 34735 : return "GeoKeyDirectoryTag";
         case 34736 : return "GeoDoubleParamsTag"; 
         case 34737 : return "GeoAsciParamsTag";
         case 34853 : return "GPS IFD";
         case 34908 : return "HylaFAXFaxRecvParams";
         case 34909 : return "HylaFAXFaxSubAddress";
         case 34910 : return "HylaFAXFaxRecvTime";
         case 37724 : return "ImageSourceData";
         case 40965 : return "InteroperabilityIFD";
         case 42112 : return "GDAL_Metadata";
         case 42113 : return "GDAL_NODATA";
         case 50215 : return "Oce Scanjob Description";
         case 50216 : return "Oce Application Selector";
         case 50217 : return "Oce Identification Number";
         case 50218 : return "Oce ImageLogic Characteristics";
         case 50706 : return "DNGVersion";
         case 50707 : return "DNGBackwardVersion";
         case 50708 : return "UniqueCameraModel";
         case 50709 : return "LocalizedCameraModel";
         case 50710 : return "CFAPlaneColor";
         case 50711 : return "CFALayout";
         case 50712 : return "LinearizationTable";
         case 50713 : return "BlackLevelRepeatDim";
         case 50714 : return "BlackLevel";
         case 50715 : return "BlackLevelDeltaH";
         case 50716 : return "BlackLevelDeltaV";
         case 50717 : return "WhiteLevel";
         case 50718 : return "DefaultScale";
         case 50719 : return "DefaultCropOrigin";
         case 50720 : return "DefaultCropSize";
         case 50721 : return "ColorMatrix1";
         case 50722 : return "ColorMatrix2";
         case 50723 : return "CameraCalibration1";
         case 50724 : return "CameraCalibration2";
         case 50725 : return "ReductionMatrix1";
         case 50726 : return "ReductionMatrix2";
         case 50727 : return "AnalogBalance";
         case 50728 : return "AsShotNeutral";
         case 50729 : return "AsShotWhiteXY";
         case 50730 : return "BaselineExposure";
         case 50731 : return "BaselineNoise";
         case 50732 : return "BaselineSharpness";
         case 50733 : return "BayerGreenSplit";
         case 50734 : return "LinearResponseLimit";
         case 50735 : return "CameraSerialNumber";
         case 50736 : return "LensInfo";
         case 50737 : return "ChromaBlurRadius";
         case 50738 : return "AntiAliasStrength";
         case 50740 : return "DNGPrivateData";
         case 50741 : return "MakerNoteSafety";
         case 50778 : return "CalibrationIlluminant1";
         case 50779 : return "CalibrationIlluminant2";
         case 50780 : return "BestQualityScale";
         case 50784 : return "Alias Layer Metadata";
         default: stringstream str; str<< tag; return str.str();
       }
     }
}; // end of class IFDEntry




class StripInfo{
public:
   StripInfo(): compression(1), imageWidth(100), imageHeight(100), 
                bitsPerSample(1),rowsPerStrip(100), samplesPerPixel(1),
                stripOffsets(), stripByteCounts(), defined(false){}


   StripInfo(const StripInfo& si){
      copyFrom(si);
   }

   StripInfo& operator=(const StripInfo& si){
     copyFrom(si);
     return *this;
   }



     ostream& print(ostream& o)const{
       if(!defined){
          o << "undefined";
          return o;
       }
        o << "compression: " << compression << ", "
           << "width " << imageWidth << ", "
           << "height " << imageHeight << ", "
           << "bitsPerSample " << bitsPerSample << ", "
           << "rowsPerStrip " << rowsPerStrip << ", "
           << "samplesPerPixel " << samplesPerPixel << ", "
           << "stripes " << stripOffsets.size();
           return o; 
       
     }


    void copyFrom(const StripInfo& si){
      compression = si.compression;
      imageWidth = si.imageWidth;
      imageHeight = si.imageHeight;
      bitsPerSample = si.bitsPerSample;
      rowsPerStrip = si.rowsPerStrip;
      samplesPerPixel = si.samplesPerPixel;
      stripOffsets = si.stripOffsets;
      stripByteCounts = si.stripByteCounts;
      defined = si.defined; 
    }

public:
   uint16_t compression;
   uint16_t imageWidth;
   uint16_t imageHeight;
   uint16_t bitsPerSample;
   uint16_t rowsPerStrip;
   uint16_t samplesPerPixel;
   vector<uint32_t> stripOffsets;
   vector<uint32_t> stripByteCounts;
   bool defined; 
};



class TileInfo{
public:
   TileInfo(): compression(1), imageWidth(100), imageHeight(100), 
                bitsPerSample(1),tileWidth(100), tileHeight(100), 
                samplesPerPixel(1),
                tileOffsets(), tileByteCounts(), defined(false){}


   TileInfo(const TileInfo& si){
      copyFrom(si);
   }

   TileInfo& operator=(const TileInfo& si){
     copyFrom(si);
     return *this;
   }



     ostream& print(ostream& o)const{
       if(!defined){
          o << "undefined";
          return o;
       }
        o << "compression: " << compression << ", "
           << "width " << imageWidth << ", "
           << "height " << imageHeight << ", "
           << "bitsPerSample " << bitsPerSample << ", "
           << "tileWidth " << tileWidth << ", "
           << "tileHeight " << tileHeight << ", "
           << "samplesPerPixel " << samplesPerPixel << ", "
           << "stripes " << tileOffsets.size();
           return o; 
       
     }


    void copyFrom(const TileInfo& si){
      compression = si.compression;
      imageWidth = si.imageWidth;
      imageHeight = si.imageHeight;
      bitsPerSample = si.bitsPerSample;
      tileWidth = si.tileWidth;
      tileHeight = si.tileHeight;
      samplesPerPixel = si.samplesPerPixel;
      tileOffsets = si.tileOffsets;
      tileByteCounts = si.tileByteCounts;
      defined = si.defined; 
    }

public:
   uint16_t compression;
   uint16_t imageWidth;
   uint16_t imageHeight;
   uint16_t bitsPerSample;
   uint32_t tileWidth;
   uint32_t tileHeight;
   uint16_t samplesPerPixel;
   vector<uint32_t> tileOffsets;
   vector<uint32_t> tileByteCounts;
   bool defined; 
};





ostream& operator<<(ostream& o, const StripInfo& i){
   return i.print(o);
}

ostream& operator<<(ostream& o, const TileInfo& i){
   return i.print(o);
}


/*
5 Class IFD

Information about an image is represented by an IFD. 
In principle, an IFD is a set of IFD Entries.

*/

class IFD{
 public:
/*
5.1 constructor

*/
  IFD(const bool _convertEndian): convertEndian(_convertEndian), entries() {}

  IFD(const IFD& ifd): convertEndian(ifd.convertEndian), entries(){
     for(size_t i=0;i<ifd.entries.size(); i++){
        entries.push_back(ifd.entries[i]);  
     }
  }
  
  IFD& operator=(const IFD& ifd){
      convertEndian = ifd.convertEndian;
      entries.clear();          
      for(size_t i=0;i<ifd.entries.size(); i++){
         entries.push_back(ifd.entries[i]);  
      }
      return *this;
  }



/*
5.2 readIfd

Reads an ifd set, returns the offset of the next ifd
if the result is 0, the ifd read is the last one in
the file. The ifd set is build as follows:
 bytes                      meaning
  0 - 1                       number of ifds in the set (count)
  2 - 2+count[mul]12              the ifds within the set
  3+counti[mul]12 - 7+count[mul]12     offset of the next ifd set

*/

   uint32_t read(ifstream& in, size_t offset){
       in.seekg(offset, ios::beg);
       // read count = number of ifds coming directly 
       unsigned char c[2];
       in.read((char*)c,2);
       uint16_t count = getNumber<uint16_t>(c,convertEndian);
       // read entries
       entries.clear();
       for(int i=0;i<count;i++){
          IFDEntry ifde(convertEndian);
          if(!ifde.read(in)){
             cout << "Problem reading ifde " << __FILE__ 
                  << ": " << __LINE__ << endl;
             throw(4);
          }
          entries.push_back(ifde);
       }
       if(!in.good()){
          cout << "problem after reading main properties from ifd" 
               << __LINE__ << endl;
          throw(4);
       }
       // read offset of the next ifd
       unsigned char noffset[4];
       in.read((char*)noffset,4);
       uint32_t no = getNumber<uint32_t>(noffset, convertEndian);
       if(!in.good()){
          cout << "Problem reading ifde " << __FILE__ << ": " 
               << __LINE__ << endl;
          throw(4);
       }
       // read all values
       for(int i=0;i<count;i++){
          entries[i].readValues(in);
          if(!in.good()){
             cout << "problem during reading in ifd " << i << endl;
             entries[i].print(cout) << endl;
          }
       }
       return no;
   }

   ostream& print(ostream& out){
     for(size_t j=0;j<entries.size();j++){
        out << "entry " << j << " : "; entries[j].print(out) << endl;
     }
     return out;
   }

/*
 GetBBox

Returns the bounding box for this file. The following informations are required:

 ModelPixelScaleTagi(33550) : size of a pixel
 ImageWidth (256): number of pixels in X-direction
 ImageLength (257) : number of pixels in Y-direction
 ModelTiepointTag(33922) : position in the world

If one of the tags is not present, a undefined BBox is returned.

*/

 Rectangle getBBox(){
    int w= getWidth();
    int h = getHeight();
    double scale = getPixelScale();
    double ties[4];
    if(!getModelTiePointTag(ties)){
       cout << "tiePointPixelTag missing or corrupt" << endl;
       Rectangle r(false);
       return r;
    }
    double ww = w * scale;
    double wh = h * scale;
    double wx = ties[2] - ties[0]*scale;
    double wy = ties[3] - ties[1]*scale;
    Rectangle r(wx,wy, wx+ww, wy+wh);
    return r;
 }

 StripInfo getStripInfo(){
   StripInfo res;  // undefined
   res.compression = getCompression();
   res.imageWidth =  getWidth();
   res.imageHeight = getHeight();
   res.bitsPerSample = getBitsPerSample(); 
   res.rowsPerStrip = getRowsPerStrip();
   res.samplesPerPixel = getSamplesPerPixel();
   res.stripOffsets = getStripOffsets();  
   res.stripByteCounts  = getStripByteCounts(); 
   res.defined = res.imageHeight>0 && res.imageWidth>0
             && res.stripOffsets.size() >0 
             && res.stripOffsets.size() == res.stripByteCounts.size();
   return res;            
 }

 TileInfo getTileInfo(){
   TileInfo res;  // undefined
   res.compression = getCompression();
   res.imageWidth =  getWidth();
   res.imageHeight = getHeight();
   res.bitsPerSample = getBitsPerSample(); 
   res.tileWidth = getTileWidth();
   res.tileHeight = getTileHeight();
   res.samplesPerPixel = getSamplesPerPixel();
   res.tileOffsets = getTileOffsets();  
   res.tileByteCounts  = getTileByteCounts(); 
   res.defined =    res.imageHeight>0 && res.imageWidth>0
                 && res.tileOffsets.size() >0 
                 && res.tileOffsets.size() == res.tileByteCounts.size()
                 && res.tileWidth>0 && res.tileHeight>0;
   return res;            
 }

 char* getStripData(const StripInfo& info, const size_t stripNo,ifstream& in) {
     if(!info.defined){
         throw(7);
     } 
     if(stripNo >= info.stripOffsets.size()){
        throw(7);
     }
     size_t size = info.stripByteCounts[stripNo];
     char* buffer = new char[size];
     in.seekg(info.stripOffsets[stripNo], ios::beg);
     in.read(buffer,size);
     size_t targetSize = 0;
     char* uncompressed = unpack::unpackData(buffer, size, targetSize, 
                                             info.compression);
     delete[] buffer;
     return uncompressed;
 }



 int getWidth(){
    return getInt(256,0);
 }
  
 int getHeight(){
    return getInt(257,0);
 }

 double getPixelScale(){
    return getDouble(33550,0);
 }

 int getCompression(){
    int r = getInt(259,0);
    return r<0?1:r; 
 }
 
 int getBitsPerSample(){
    int r = getInt(258,0);
    return r<0?1:r; 
 }
 
 uint32_t getRowsPerStrip(){
    int32_t i = getInt(278,0);
    return i>0?i:0xFFFFFFFFu;
 }
 
 uint32_t getTileWidth(){
    int32_t i = getInt(322,0);
    return i;
 }
 
 uint32_t getTileHeight(){
    int32_t i = getInt(323,0);
    return i;
 }

  int getSamplesPerPixel(){
    int r = getInt(277,0);
    return r<0?1:r; 
    
  }

  vector<uint32_t> getStripOffsets(){
    vector<uint32_t> r;
    try {
        IFDEntry e = getEntry(273);
        if(e.hasIntValue()){
          for(int i=0;i<e.getCount();i++){
             r.push_back(e.getIntValue(i));
          }
        }     
        return r; 
    } catch(...){
        return r;
    }
  }  
  
  vector<uint32_t> getTileOffsets(){
    vector<uint32_t> r;
    try {
        IFDEntry e = getEntry(324);
        if(e.hasIntValue()){
          for(int i=0;i<e.getCount();i++){
             r.push_back(e.getIntValue(i));
          }
        }     
        return r; 
    } catch(...){
        return r;
    }
  }  

  vector<uint32_t> getStripByteCounts(){
    vector<uint32_t> r;
    try {
        IFDEntry e = getEntry(279);
        if(e.hasIntValue()){
          for(int i=0;i<e.getCount();i++){
             r.push_back(e.getIntValue(i));
          }
        }     
        return r; 
    } catch(...){
        return r;
    }
  }  
  
  vector<uint32_t> getTileByteCounts(){
    vector<uint32_t> r;
    try {
        IFDEntry e = getEntry(325);
        if(e.hasIntValue()){
          for(int i=0;i<e.getCount();i++){
             r.push_back(e.getIntValue(i));
          }
        }     
        return r; 
    } catch(...){
        return r;
    }
  }  

 bool getModelTiePointTag(double* res) const{
   try{
      IFDEntry e = getEntry(33922);
      if(e.getCount()<6){
          return false;
      }
      res[0] = e.getDoubleValue(0);
      res[1] = e.getDoubleValue(1);
      res[2] = e.getDoubleValue(3);
      res[3] = e.getDoubleValue(4);
      return true;
   } catch(...){
      return false;
   }        
 }

 const IFDEntry& getEntry(const uint16_t tag) const{
    for(size_t i=0;i<entries.size();i++){
       if(entries[i].hasTag(tag)){
          return entries[i];
       }
    }
    throw(4);
 }


 int getInt(const uint16_t tag, const size_t valNum) const{
    for(size_t i=0;i<entries.size();i++){
       if(entries[i].hasTag(tag)){
          if(entries[i].hasIntValue()){
             return entries[i].getIntValue(valNum);
          }
       }
    }
    return -1;
 }

 double getDouble(const uint16_t tag, const size_t valNum) const{
    for(size_t i=0;i<entries.size();i++){
       if(entries[i].hasTag(tag)){
          if(entries[i].hasDoubleValue()){
             return entries[i].getDoubleValue(valNum);
          }
       }
    }
    return -1;
 }

 private:
   bool convertEndian;   
   vector<IFDEntry> entries;    
};







/*
6. CLass tiffinfo

This class analyses a tiff-file

*/

class tiffinfo{
  public:

/*
6.1 Constructor

This constructor creates a stream from a file name and detects the
endian  order used by the system.

*/
    tiffinfo(const string& filename) {
       in.open(filename.c_str(),ios::binary);
       int endian_detect=1;
       sysLittleEndian = *(char *)&endian_detect == 1;
    }


    ~tiffinfo(){
       in.close();
     }


     size_t numberOfImages(){
         return ifds.size();
     }

     Rectangle getBBox(size_t image_number){
         return ifds[image_number].getBBox();
     }

     int getWidth(size_t image_number){
         return ifds[image_number].getWidth();
     }

     
     int getHeight(size_t image_number){
         return ifds[image_number].getHeight();
     }
     
    int getPixelScale(size_t image_number){
         return ifds[image_number].getPixelScale();
     }

     StripInfo getStripInfo(size_t image_number){
         return ifds[image_number].getStripInfo();
     }

     TileInfo getTileInfo(size_t image_number){
         return ifds[image_number].getTileInfo();
     }
     ostream& print(ostream& o, size_t image_number){
        return ifds[image_number].print(o);
     }

     char* getStripData(StripInfo& info, size_t image_number, size_t stripe){
        return ifds[image_number].getStripData(info,stripe,in);
     }



/*
6.2 printInfo

Retrieves all main IDFs from the input stream and writes them to the terminal.


*/
    void readInfo(){
       if(!in.good()){
         cout << "Error in reading file" << endl;
         return;
       }
       // read and print header
       readHeader();
   
       // read all ifds
       // all ifds are stored at firstIfdOffset (stored in the header)
       // after each ifd, the offset of the next ifd follows
       // the chain ends if the offset is zero
       uint32_t offset = firstIfdOffset;
       while(offset){
           IFD ifd(sysLittleEndian!=littleEndian);
           offset = ifd.read(in, offset);
           ifds.push_back(ifd); 
       } 
    }
   

   void  printIFDs(ostream& out){
     // print out all ifds
     cout << "the file contains " << ifds.size() << " IFDs" << endl;
     for(size_t i=0;i<ifds.size();i++){
         out << " IFD : " << i << endl;
         ifds[i].print(out);
         cout << " ------------------------------ " << endl << endl;
     }
    }



    void printHeader(ostream& out){
       out << "file  Endian  : " 
           << (littleEndian?"little Endian":" big Endian") 
           << endl;
       out << "system Endian : " 
           << (sysLittleEndian?"little Endian":" big Endian") 
           << endl;
       out << "magic number  : " 
           << magicNumber << (magicNumber==42?" (ok)":" (wrong)") 
           << endl;
     //  out << "offset 1st IFD: " << firstIfdOffset << endl;
     //  out << "file size     : " << fileSize << endl;
    }


  private:

/*
6.3 members

*/
     ifstream in;                         // the input stream
     bool littleEndian;                   // endian used in the file
     bool sysLittleEndian;                // endian used by the system
     int16_t magicNumber;
     uint32_t firstIfdOffset;             // offset of the first main ifd
     uint32_t fileSize;                   // size of the input file
     vector<IFD > ifds;      // vector holding all main ifds


/*
6.4 getNumber

Returns a number stored in the byte array input taking endian into account.

*/
  template<class T> 
  T getNumber(unsigned char* input){
     if(sysLittleEndian!=littleEndian){
         return convertEndian<T>(input);
     } else {
        T res = *((T*)input);
        return res;
     }
  }  




  
/*
6.6 printHeader

reads in the header of a tiff file and prints out all read in information.


The header structire is

bytes           meaning
 0-1            byte order (endian) used in the file must be either
                0x49 0x49 -> little endian  or
                0x4D 0x4D -> big endian
 2-3            the magic number (42)
 4-7            the offset of the first ifd set


*/
  void readHeader(){
     char order[2];
     in.read(order,2);
     if(order[0]==0x49 && order[1]==0x49){
         littleEndian = true;
     } else if(order[0]==0x4D && order[1]==0x4D){
         littleEndian = false;
     } else {
         cerr << "not an tiff file, endian code is wrong"  << endl;
         cerr << "Endian is read as " << hex 
              << *((uint16_t*)order) << dec << endl;
             cout << "Problem reading ifde " << __FILE__ 
                  << ": " << __LINE__ << endl;
         throw(3);
     }
     unsigned char magic[2];
     in.read((char*)magic,2);
     magicNumber = getNumber<int16_t>(magic);
     unsigned char offset[4];
     in.read((char*)offset,4);
     firstIfdOffset = getNumber<uint32_t>(offset);
     in.seekg(0,ios::end);
     fileSize = in.tellg(); 
     if(firstIfdOffset + 12 > fileSize){
             cout << "Problem reading ifde " << __FILE__ 
                  << ": " << __LINE__ << endl;
        throw 4;
     }
  }



};



class ImportTiffLocalInfo{

  public:

    ImportTiffLocalInfo(Word _stream, sint* _result, size_t _maxMem):
        stream(_stream), result(_result), first(true)
    {
        result->clear();
        result->setCacheSize(_maxMem);
    }

    ~ImportTiffLocalInfo(){
    }

    void import(){
       stream.open();
       FText* fileName;
       while( (fileName = stream.request())!=0){
          if(fileName->IsDefined()){
             import(fileName->GetValue());
          }
          fileName->DeleteIfAllowed();
       }
       stream.close();
    };

  private:

     Stream<FText> stream;
     sint* result;
     bool first;
     double pixelsize;

    void import(const string& fileName){
       try{
          tiffinfo ti(fileName);
          ti.readInfo();
          if(ti.numberOfImages()!=1){ // error
             cerr << "Tiffimport can handle single image tiffs only" << endl
                  << "number of images in " << fileName << " is " 
                  << ti.numberOfImages() << endl;
             return;
          }
          Rectangle bbox = ti.getBBox(0);
          if(!bbox.isDefined()){
             cerr << "file " << fileName << endl
                  << "bbox missing, no import possible" << endl;
             return;     
          }
          StripInfo sinfo = ti.getStripInfo(0);
          if(!sinfo.defined){
             cerr << "file " << fileName << endl
                  << " cannot be imported , only import of " 
                  << "striped images is currently implemented";
             return;
          }
          int bps = sinfo.bitsPerSample;
          if(bps!= 8 && bps!=16 && bps!=32){
             cerr << "unsupported bitsPerSample value " << bps;
             return;
          }

          // read all stripes
          const double pixelsize = (bbox.getX2() - bbox.getX1())
                                   / sinfo.imageWidth;

          if(first){
             grid2 grid(bbox.getX1(), bbox.getY1(), pixelsize);
             result->setGrid(grid); 
             this->pixelsize = pixelsize;
             cout << "set grid to " << grid << endl;
             first = false;
          } else {
            if(!AlmostEqual(pixelsize,this->pixelsize)){
               cerr << "Different pixelsize to first import" << endl;
               return;
            }
          }

          for(size_t i=0;i<sinfo.stripOffsets.size();i++){
             char* data = ti.getStripData(sinfo,0,i);
             int rowsForStrip = sinfo.rowsPerStrip;
             if((i+1)*sinfo.rowsPerStrip > sinfo.imageHeight){
                rowsForStrip = sinfo.imageHeight - i*sinfo.rowsPerStrip;
             }
             switch(bps){
               case 8 :  processStrip<uint8_t>( i, data, sinfo.imageWidth, 
                                rowsForStrip,bbox.getX1(), 
                                bbox.getY2(),pixelsize);
                    break;
               case 16 :  processStrip<uint16_t>( i, data, sinfo.imageWidth, 
                                rowsForStrip,bbox.getX1(), 
                                bbox.getY2(),pixelsize);
                    break;
               case 32 :  processStrip<uint32_t>( i, data, sinfo.imageWidth, 
                                rowsForStrip,bbox.getX1(), 
                                bbox.getY2(),pixelsize);
                    break;
               default: cerr << "invalid bitsPerPixel " << bps << endl;
                        delete[] data;
                        return;
             }
             delete[] data;
          }
       } catch(...){
          cerr << "problem  in importing file " << fileName << endl;
       }

    }

    template<class T>
    void processStrip(const size_t no, //strip number
                      const char* data,  // strip content
                      const size_t width, // image witdh
                      const size_t rowsPerStrip,  
                                        // number of rows within this strip
                      const double x1,   // leftmost pos within the image
                      const double ytop, // topmost pos within the image
                      const double pixelsize ){ // size of a single pixel
        T* tdata = (T*) (void*) data;
        for(size_t y=0;y<rowsPerStrip;y++){
           for(size_t x=0;x<width;x++){
               T v = tdata[width*y+x]; // get Value from data
               // compute location of the middle of the cell 
               double xm = x1 + x*pixelsize + pixelsize/2.0;
               double ym = ytop - ((no*rowsPerStrip+y)*pixelsize + pixelsize/2);
               result->setatlocation(xm,ym,v);
           } 
        }

    }


};
} // end of namespace tiffimport



ListExpr importTiffTM(ListExpr args){
    string err = "stream(text) expected";
    if(!nl->HasLength(args,1)){
       return listutils::typeError(err);
    }
    if(!Stream<FText>::checkType(nl->First(args))){
       return listutils::typeError(err);
    }
    return listutils::basicSymbol<sint>();
}


int importTiffVM( Word* args, Word& result, int message,
                     Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   tiffimport::ImportTiffLocalInfo importer(args[0], 
                                            (sint*) result.addr, 
                                            16000000);
   importer.import();
   return 0;
}











