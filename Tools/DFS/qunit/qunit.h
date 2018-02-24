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
#pragma once

#include <iostream>
#include <cstring>

namespace qunit {

#define aq aeq

  /**
   * writes message to default out
   * @param msg
   */
  void msg(const char *msg) {
    std::cout << msg << std::endl;
  }

  /**
   * assert true
   * throws FALSE if condition is not TRUE
   * @param msg
   * @param c
   */
  void at(const char *msg, bool c) {
    std::cout << "QUNIT\t" << (c ? "SUCCESS" : "FAILED ") << "\t" << msg <<
              std::endl;
    if (!c) throw c;
  }

  /**
   * assert false
   * throws FALSE if condition is TRUE
   * @param msg
   * @param c
   */
  void af(const char *msg, bool c) {
    at(msg, !c);
  }

  /**
   * assert equals
   * throws FALSE if a == b is FALSE
   * hence a,b need to be same type and support == operator
   * writes message
   * @tparam T
   * @param msg
   * @param a
   * @param b
   */
  template<typename T>
  void aeq(const char *msg, const T &a, const T &b) {
    bool c = a == b;
    if (!c) {
      std::cout << "expected: " << a << std::endl;
      std::cout << "but is: " << b << std::endl;
    }
    at(msg, c);
  }

  /**
   * assert equals
   * throws FALSE if a == b is FALSE
   * hence a,b need to be same type and support == operator
   * @tparam T
   * @param msg
   * @param a
   * @param b
   */
  template<typename T>
  void aeq(const T &a, const T &b) {
    bool c = a == b;
    if (!c) {
      std::cout << "expected: " << a << std::endl;
      std::cout << "but is: " << b << std::endl;
    }
    at("unequal", c);
  }

  /**
   * assert equals for cstrings
   * throws FALSE if strcmp(a,b) not 0
   * @param msg
   * @param a
   * @param b
   */
  void aeqcs(const char *msg, const char *a, const char *b) {
    at(msg, strcmp(a, b) == 0);
  }

  /**
   * describes a run able test case
   */
  class TestCase {
  public:
    virtual void run() = 0;
  };

}
