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
#ifndef FIGURESYSTEM_H
#define FIGURESYSTEM_H

#include "str.h"

namespace dfs {

  /**
   * represents a custom figure system
   */
  class FigureSystem {
  private:
    short base;
    short length;
    char *values;

    void refactor();

  public:

    /**
     * inits new figure system with base and max amount of figures
     * @param base
     * @param length
     */
    FigureSystem(short base, short length);

    virtual ~FigureSystem();

    /**
     * increases value by one
     */
    void inc();

    /**
     * increases value by parameter
     * @param value
     */
    void inc(int value);

    /**
     * formats value as str
     * @return
     */
    Str toStr();

    /**
     * restores state from strValue
     * length needs to be as length of this
     * @param strValue
     */
    void fromStr(const Str &strValue);

    /**
     * sets current value to zero
     */
    void resetToZero();
  };

}

#endif /* FIGURESYSTEM_H */

