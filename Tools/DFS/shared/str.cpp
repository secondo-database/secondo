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
#include "str.h"
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <cstdlib>

using namespace dfs;
using namespace std;

ostream &dfs::operator<<(ostream &os, const Str &t) {
  int l = t.len();
  for (int i = 0; i < l; i++) os << t[i];
  return os;
}

Str dfs::operator+(const Str &a, const Str &b) {
  return a.append(b);
}

Str::Str() {
  length = 0;
  buffer = 0;
}

Str::Str(const char *cs) {
  length = 0;
  buffer = 0;
  fromBuffer(cs, strlen(cs));
}

Str::Str(const Str &s) {
  length = 0;
  buffer = 0;
  fromBuffer(s.buffer, s.length);
}

Str::Str(const char *buf, int l) {
  length = 0;
  buffer = 0;
  fromBuffer(buf, l);
}

Str::Str(int i) {
  length = 0;
  char tmp[32];
  int x = sprintf(tmp, "%d", i);
  fromBuffer(tmp, x);
}

Str::Str(long l) {
  length = 0;
  char tmp[32];
  int x = sprintf(tmp, "%ld", l);
  fromBuffer(tmp, x);
}

Str::Str(unsigned long l) {
  length = 0;
  char tmp[32];
  int x = sprintf(tmp, "%ld", l);
  fromBuffer(tmp, x);
}

Str::Str(UI64 l) {
  length = 0;
  char tmp[32];
  int x = sprintf(tmp, "%llu", l);
  fromBuffer(tmp, x);
}

void Str::clear() {
  delete[] buffer;
  buffer = 0;
  length = 0;
}

int Str::len() const {
  return length;
}

char *Str::cstr() const {
  if (length == 0) return 0;
  char *tmp = new char[length + 1];
  memcpy(tmp, buffer, length);
  tmp[length] = char(0);
  return tmp;
}

char &Str::operator[](int index) const {
  return buffer[index];
}

void Str::fromBuffer(const char *buf, int len) {
  if (length > 0) {
    clear();
  }
  this->length = len;
  this->buffer = new char[len];
  for (int i = 0; i < len; i++) this->buffer[i] = buf[i];
}

Str Str::append(const Str &s) const {
  if (length == 0) {
    return Str(s);
  } else {
    Str x;
    int nl = length + s.length;
    x.length = nl;
    x.buffer = new char[nl];
    for (int i = 0; i < length; i++) x.buffer[i] = buffer[i];
    for (int i = 0; i < s.length; i++) x.buffer[length + i] = s.buffer[i];
    return x;
  }
}

Str Str::prepend(const Str &s) const {
  if (length == 0) {
    return s;
  } else {
    int nl = length + s.length;
    char *tmp = new char[nl];
    for (int i = 0; i < s.length; i++) tmp[i] = s.buffer[i];
    for (int i = 0; i < length; i++) tmp[i + s.length] = buffer[i];
    Str x(tmp, nl);
    delete[] tmp;
    tmp = 0;
    return x;
  }
}

Str Str::prepend(int maxWidth, char c) {
  int d = maxWidth - length;
  if (d < 1) return *this;
  char *tmp = new char[maxWidth];
  for (int i = 0; i < d; i++) {
    tmp[i] = c;
  }
  for (int i = 0; i < length; i++) {
    tmp[i + d] = buffer[i];
  }
  Str x(tmp, maxWidth);
  delete[] tmp;
  return x;
}


bool Str::isEmpty() const {
  return length == 0;
}

Str::~Str() {
  delete[] buffer;
  buffer = 0;
  length = 0;
}

Str &Str::operator=(const Str &other) {
  if (this != &other) {
    fromBuffer(other.buffer, other.length);
  }
  return *this;
}

bool Str::operator==(const Str &b) const {
  if (this->length != b.length) return false;
  for (int i = 0; i < this->length; i++) {
    if (this->buffer[i] != b.buffer[i]) return false;
  }
  return true;
}

bool Str::operator!=(const Str &b) const {
  return !(*this == b);
}

int Str::find(const Str &s) const {
  if (length == 0 || s.length > length) return -1;
  if (length == s.length) return *this == s ? 0 : -1;
  for (int i = 0; i < length; i++) {
    int r = findAt(s, i);
    if (r > -1) return r;
  }
  return -1;
}

int Str::findAt(const Str &s, int start) const {
  for (int i = 0; i < s.length; i++) {
    int x = i + start;
    if (x >= length) return -1;
    if (buffer[x] != s.buffer[i]) return -1;
  }
  return start;
}

int Str::findLastChar(char c) const {
  for (int i = length - 1; i >= 0; i--) {
    if (buffer[i] == c) return i;
  }
  return -1;
}


Str Str::substr(int start) const {
  int nl = length - start;
  char *tmp = new char[nl];
  for (int i = start; i < length; i++) {
    tmp[i - start] = buffer[i];
  }
  Str s(tmp, nl);
  delete[] tmp;
  return s;
}

Str Str::substr(int start, int len) const {
  int r = length - start;
  if (len > r) len = r;
  char *tmp = new char[len];
  for (int i = 0; i < len; i++) {
    tmp[i] = buffer[start + i];
  }
  Str s(tmp, len);
  delete[] tmp;
  return s;
}

int Str::toInt() const {
  char *cs = cstr();
  int i = atoi(cs);
  delete[] cs;
  return i;
}

long Str::toLong() const {
  char *cs = cstr();
  long l = atol(cs);
  delete[] cs;
  return l;
}

UI64 Str::toUInt64() const {
  char *cs = cstr();
  UI64 l = strtoull(cs, 0, 10);
  delete[] cs;
  return l;
}

Str Str::serialize() const {
  return Str(length).append("_").append(*this);
}

Str Str::deserialize(const Str &ser) {
  int x = ser.find("_");
  if (x == -1) {
    throw "delimiter not found";
  }
  int l = ser.substr(0, x).toInt();
  return ser.substr(x + 1, l);
}

StrReader::StrReader(const Str *s) {
  pos = 0;
  this->s = s;
}

int StrReader::getPos() const {
  return pos;
}

void StrReader::setPos(int p) {
  pos = p;
}

int StrReader::readInt(int len) {
  int i = s->substr(pos, len).toInt();
  pos += len;
  return i;
}

long StrReader::readLong(int len) {
  long l = s->substr(pos, len).toLong();
  pos += len;
  return l;
}

UI64 StrReader::readUInt64() {
  Str s = readStr(21);
  return s.toUInt64();
}

Str StrReader::readStr(int len) {
  Str x = s->substr(pos, len);
  pos += len;
  return x;
}

Str StrReader::readStrSer() {
  int xpos = pos;
  while ((*s)[xpos] != '_' && xpos < s->len()) xpos++;
  int serlen = s->substr(pos, xpos - pos).toInt();
  Str result = serlen == 0 ? Str("") : s->substr(xpos + 1, serlen);
  this->pos = xpos + serlen + 1;
  return result;
}

Str StrReader::copyOfReminder() {
  return s->substr(pos);
}

char Str::last() const {
  return buffer[length - 1];
}

void ToStrSerializer::appendRaw(const Str &s) {
  output = output.append(s);
}

void ToStrSerializer::append(const Str &s) {
  output = output.append(Str(s.len())).append(Str("_")).append(s);
}

void ToStrSerializer::append(int value, int maxLength) {
  output = output.append(Str(value).prepend(maxLength, '0'));
}

void ToStrSerializer::appendDefaultUnsigned(int value) {
  append(value, 12);
}

void ToStrSerializer::appendDefaultUnsignedShort(unsigned short value) {
  append(value, 5);
}

void ToStrSerializer::appendUInt64(UI64 value) {
  char tmp[21];
  sprintf(tmp, "%llu", value);
  Str s(value);
  appendRaw(s.prepend(21, '0'));
}

void ToStrSerializer::appendEmptyStr() {
  this->append(Str(""));
}

Str Str::createSingleChar(int length, char c) {
  Str x;
  x.length = length;
  x.buffer = new char[length];
  for (int i = 0; i < length; i++) x.buffer[i] = c;
  return x;
}

StrBuilder::StrBuilder(int initialCapacity, int growCapacity) {
  this->growBy = growCapacity;
  this->size = initialCapacity;
  this->data = new char[this->size];
  this->pos = 0;
}

void StrBuilder::append(const Str &s) {
  char *sb = s.buf();
  int sl = s.len();
  ensureEnough(sl);

  for (int i = 0; i < sl; i++) {
    data[pos++] = sb[i];
  }
}

void StrBuilder::appendCStr(const char *buf) {
  appendCStr(buf, strlen(buf));
}

void StrBuilder::appendCStr(const char *buf, int length) {
  ensureEnough(length);
  for (int i = 0; i < length; i++) {
    data[pos++] = buf[i];
  }
}

StrBuilder::~StrBuilder() {
  delete[] data;
  data = 0;
  pos = 0;
}

int StrBuilder::currentPos() const {
  return pos;
}

int StrBuilder::currentSize() const {
  return size;
}

void StrBuilder::ensureEnough(int addedLength) {
  int needed = pos + addedLength;
  if (needed <= size) return;

  int newSize = size;
  while (newSize < needed) {
    newSize += growBy;
  }
  char *newData = new char[newSize];
  for (int i = 0; i < pos; i++) {
    newData[i] = data[i];
  }
  delete[] data;
  data = newData;
  size = newSize;
}

char *StrBuilder::buf() const {
  return data;
}

Str Str::toHex(const char *buf, int length) {
  const char *alpha = "01234567890abcdef";

  char *target = new char[length * 2];
  for (int i = 0; i < length; i++) {
    target[i * 2] = alpha[buf[i] >> 4 & 0xf];
    target[i * 2 + 1] = alpha[buf[i] & 0xf];
  }

  Str s;
  s.buffer = target;
  s.length = length * 2;
  return s;
}
