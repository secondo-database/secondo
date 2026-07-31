/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

1 CheckExamples

Checks that every example in the given ~.examples~ files is syntactically
valid SECONDO. Exits non-zero and reports file, line and operator for each
example that does not parse.

The kernel used to do this at every startup, once per example, and threw the
result away. It is a build-time property of the ~.examples~ files, so it is
checked here instead -- see the ~checkexamples~ target in Algebras/makefile.

This tool deliberately needs no database and no algebras: it links only
against the parser and the utility library.

*/

#include <iostream>
#include <string>

#include "ExampleReader.h"
#include "SecParser.h"

int main( int argc, char** argv )
{
  if ( argc < 2 )
  {
    std::cerr << "usage: " << argv[0] << " <file.examples> ..." << std::endl;
    return 2;
  }

  int failures = 0;
  int checked  = 0;

  for ( int i = 1; i < argc; i++ )
  {
    const std::string fileName( argv[i] );

    ExampleReader reader( fileName );
    if ( !reader.parse() )
    {
      std::cerr << fileName << ": cannot be parsed as an examples file"
                << std::endl;
      failures++;
      continue;
    }

    // getCurrentList() yields every example of the current operator, not just
    // the first, matching what the kernel used to check.
    for ( reader.initScan(); !reader.endOfScan(); reader.nextOfScan() )
    {
      const ExampleReader::ExampleList& examples = reader.getCurrentList();
      ExampleReader::ExampleList::const_iterator e;
      for ( e = examples.begin(); e != examples.end(); e++ )
      {
        const ExampleInfo& ex = **e;
        checked++;

        SecParser sp;
        std::string parsed = "";
        if ( sp.Text2List( ex.example, parsed ) != 0 )
        {
          std::cerr << fileName << ":" << ex.lineNo
                    << ": operator " << ex.opName
                    << ": example does not parse" << std::endl
                    << "  " << ex.example << std::endl;
          failures++;
        }
      }
    }
  }

  if ( failures > 0 )
  {
    std::cerr << std::endl
              << "CheckExamples: " << failures << " of " << checked
              << " examples are invalid." << std::endl;
    return 1;
  }

  std::cout << "CheckExamples: " << checked << " examples ok." << std::endl;
  return 0;
}
