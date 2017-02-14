/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

*/

#pragma once

#include "AlgebraTypes.h"
#include <exception>
#include "LogMsg.h"

namespace CRelAlgebra
{
  namespace Operators
  {
    template<class T>
    int StreamValueMapping(ArgVector args, Word &result, int message,
                           Word &local, Supplier s)
    {
      try
      {
        switch (message)
        {
        case OPEN:
        {
          if (local.addr != NULL)
          {
            delete (T*)local.addr;
          }

          local.addr = new T(args, s);

          break;
        }
        case REQUEST:
        {
          if ((result.addr = ((T*)local.addr)->Request()) == NULL)
          {
            return CANCEL;
          }

          return YIELD;
        }
        case CLOSE:
        {
          if (local.addr != NULL)
          {
            delete (T*)local.addr;

            local.addr = NULL;
          }

          break;
        }
        }

        return 0;
      }
      catch (const std::exception &e)
      {
        ErrorReporter::ReportError(e.what());
      }

      return FAILURE;
    };
  }
}