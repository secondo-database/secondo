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
#ifndef PARAMETERHELPER_H
#define PARAMETERHELPER_H

#include "str.h"

namespace dfs {

  /**
   * a class for easy accessing command line
   */
  class ParameterHelper {
  private:
    char **argv;
    int argc;
  public:
    ParameterHelper(int argc, char **argv);

    /**
     * returns TRUE if a command is given
     * like programm.sh start - start is command
     * @param name
     * @return
     */
    bool hasCommand(const Str &name);

    /**
     * returns TRUE if parameter of name is given
     * like progra.sh -X -d - parameter X and d is present, parameter full not
     * @param name
     * @return
     */
    bool hasParameter(const Str &name);

    /**
     * parameters can be in format
     * program.sh -dir=/home
     * returns the value - getParameter(dir) will return /home
     * @param name
     * @return
     */
    Str getParameter(const Str &name);

    /**
     * same as getParameter but converts value to int
     * @param name
     * @return
     */
    int getParameterInt(const Str &name);

    /**
     * returns command line argument raw
     * @param i
     * @return
     */
    Str word(short i);

    /**
     * returns TRUE if at least given number of arguments exists
     * @param i
     * @return
     */
    bool hasNumberOfArguments(short i);

  };


};

#endif /* PARAMETERHELPER_H */

