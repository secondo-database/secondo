/*
----
This file is part of SECONDO.
Realizing a simple distributed filesystem for master thesis of stephan scheide

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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
----


//[$][\$]

*/
#ifndef DFS_UTILS_H
#define DFS_UTILS_H

#include "../define.h"
#include <iostream>
#include <list>
#include <cstring>

namespace dfs {


  /**
   * @author Stephan Scheide
   * power buffer class
   */
  class Str {
  private:
    int length;
    char *buffer;

    void fromBuffer(const char *buf, int len);

    bool isEmpty() const;

  public:

    Str();

    ~Str();

    /**
     * str from cstr
     * @param cstr
     */
    Str(const char *cstr);

    /**
     * str from raw buffer
     * @param buffer
     * @param length
     */
    Str(const char *buffer, int length);

    /**
     * copy constructor
     * @param s
     */
    Str(const Str &s);

    /**
     * str from int
     * @param i
     */
    Str(int i);

    /**
     * str from long
     * @param long
     */
    Str(long l);

    /**
     * str from unsigned long
     * @param long
     */
    Str(unsigned long l);

    /**
     * str from UI64
     * @param long
     */
    Str(UI64 l);

    /**
     * returns hex string
     * @param buf
     * @param length
     * @return
     */
    static Str toHex(const char *buf, int length);

    /**
     * str from single char
     * @param length
     * @param c
     * @return
     */
    static Str createSingleChar(int length, char c);

    /**
     * returns pointer to raw data
     * please do not modify outside
     * @return
     */
    char *buf() const { return buffer; }

    /**
     * appends other str to end returns new instance of str
     * @param s
     * @return
     */
    Str append(const Str &s) const;

    void appendToThis(const Str& s);

    void appendRawBufferToThis(char* buffer, int length);

    /**
     * puts other str in front returns new instance of str
     * @param s
     * @return
     */
    Str prepend(const Str &s) const;

    Str replace(const Str &s);

    /**
     * prepends with single char until a max widht is reached
     * @param maxWidth
     * @param c
     * @return
     */
    Str prepend(int maxWidth, char c);

    /**
     * prepends with leading zero until max width is reached
     * @param maxWidth
     * @return
     */
    Str prepend(int maxWidth) { return prepend(maxWidth, '0'); }

    /**
     * returns len of buffer
     * @return
     */
    int len() const;

    /**
     * returns position of buffer in current str or -1 if not found
     * @param s
     * @return
     */
    int find(const Str &s) const;

    /**
     * like find, but does start at custom position
     * @param s
     * @param start
     * @return
     */
    int findAt(const Str &s, int start) const;

    /**
     * finds last occurrence of char
     * @param c
     * @return
     */
    int findLastChar(char c) const;

    /**
     * returns last char
     * careful if str is empty
     * @return
     */
    char last() const;

    void change(int startPos, int length, const Str &replacement);

    /**
     * makes a cstr of buffer and terminates it with \0
     * needs to be freed by caller!
     */
    char *cstr() const;

    /**
     * returns serialized version of str
     * len_content
     * so parse can read until _ occurs and then read len bytes
     * @return
     */
    Str serialize() const;

    /**
     * creates instance of serialized str
     * @param s
     * @return
     */
    static Str deserialize(const Str &s);

    Str substr(int start) const;

    void changeStartIndexNoFree(int start);

    Str substr(int start, int len) const;

    /**
     * converts buffer to int
     * @return
     */
    int toInt() const;

    /**
     * converts buffer to int
     * @return
     */
    long toLong() const;

    /**
     * converts to UI64
     * @return
     */
    UI64 toUInt64() const;

    /**
     * clears buffer, frees all memory
     */
    void clear();

    /**
     * get and set single char at index
     * index starts with 0 for first char
     * @param index
     * @return
     */
    char &operator[](int index) const;

    /**
     * assignment operator
     * @param other
     * @return
     */
    Str &operator=(const Str &other);

    /**
     * returns TRUE if current instance matches other instance
     * a match is give is same length and byte by byte are equal
     * @param b
     * @return
     */
    bool operator==(const Str &b) const;

    /**
     * returns TRUE if opposite of == occurrs
     */
    bool operator!=(const Str &b) const;

  };

  /**
   * moves buffer to out stream
   * @param os
   * @param t
   * @return
   */
  std::ostream &operator<<(std::ostream &os, const Str &t);

  /**
   * concacts (a,b) to ab
   * @param a
   * @param b
   * @return
   */
  Str operator+(const Str &a, const Str &b);

  /**
   * if using map we need hash value for str
   */
  struct StrHasher {
    std::size_t operator()(const Str &s) const {
      return s.len();
    }
  };

  /**
   * reads data from a str buffer
   */
  class StrReader {
  private:
    const Str *s;
    int pos;
  public:
    StrReader(const Str *s);

    /**
     * sets position for next read operation
     * @param p
     */
    void setPos(int p);

    /**
     * returns current pos
     * @return
     */
    int getPos() const;

    /**
     * reads int by using next len bytes
     * @param len
     * @return
     */
    int readInt(int len);

    /**
     * reads long by using next len bytes
     * @param len
     * @return
     */
    long readLong(int len);

    /**
     * reads UI64
     * @param len
     * @return
     */
    UI64 readUInt64();

    /**
     * reads raw sub buffer
     * @param len
     * @return
     */
    Str readStr(int len);

    //5_Hallo --> Hallo (liest serialisierten String)

    /**
     * reads serialized str and converts to normal buffer
     * @return
     */
    Str readStrSer();

    /**
     * returns copy of the reminder of this buffer
     * @return
     */
    Str copyOfReminder();

    char* pointerToCurrentRawBuf();
  };

  /**
   * writes a serialized str
   */
  class ToStrSerializer {
  public:
    Str output;

    /**
     * appends raw buffer
     * @param s
     */
    void appendRaw(const Str &s);

    /**
     * appends str as serialized
     * @param s
     */
    void append(const Str &s);

    void appendBinaryAsSer(int length, char* buffer);

    void appendRawBinary(int length, char* buffer);

    /**
     * append int and fills with leading zero
     * @param value
     * @param maxLength
     */
    void append(int value, int maxLength);

    /**
     * adds int of default length
     * @deprecated
     * @param value
     */
    void appendDefaultUnsigned(int value);

    /**
     * adds shorts of default length
     * @deprecated
     * @param value
     */
    void appendDefaultUnsignedShort(unsigned short value);

    /**
     * adds UI64
     * @param value
     */
    void appendUInt64(UI64 value);

    /**
     * adds empty buffer
     */
    void appendEmptyStr();
  };

  /**
   * describes entity / class which needs to be serialized
   */
  class SerializeAble {
  public:
    virtual void serializeTo(ToStrSerializer &serializer) const = 0;
  };

  /**
   * a cstr wrapper around buffer
   * internal has a copy as cstr created during construction
   * used to call methods with char*
   */
  class CStr {
  private:
    char *cs;
  public:
    CStr(const Str &s) {
      cs = s.cstr();
    }

    char *cstr() {
      return cs;
    }

    ~CStr() {
      delete[] cs;
      cs = 0;
    }
  };

  /**
   * a builder for concating buffers
   */
  class StrBuilder {
  private:
    char *data;
    int size;
    int pos;
    int growBy;

    void ensureEnough(int addedLength);

  public:

    StrBuilder(int initialCapacity, int growCapacity);

    /**
     * appends buffer
     * @param s
     */
    void append(const Str &s);

    /**
     * appends buffer terminated by \0
     * @param s
     */
    void appendCStr(const char *buf);

    /**
     * appends length bytes of buffer
     * @param s
     */
    void appendCStr(const char *buf, int length);

    /**
     * returns current pos for writing
     * @return
     */
    int currentPos() const;

    /**
     * returns internal size
     * @return
     */
    int currentSize() const;

    /**
     * returns intermediate result
     * @return
     */
    char *buf() const;

    virtual ~StrBuilder();
  };

  /**
   * compares two buffers
   * needed for collections
   */
  class StrComparer {
  public:
    bool operator()(const Str &a, const Str &b) const {
      int alen = a.len();
      int blen = b.len();

      if (alen < blen) return true;
      if (alen > blen) return false;

      return memcmp(a.buf(), b.buf(), alen) < 0;
    }
  };

};

#endif