/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] Class DServerCmdDelete Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdDelete~ provides the functionality to delete the data of a
darray index to antoher darray, keeping the index. It is used in the 
DArray::Clone() method and in the implementation of the ~put~ operator.
This class derives from ~DServerCmd~ class, which provides the basic
communication and run functionality.

The class ~DServerCmdDeleteParam~ is a data class for the parameters used 
during the execution of a ~DServerCmdDelete~ object.

*/

/*
1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERCMDDELETE_H
#define H_DSERVERCMDDELETE_H

/*
1.2 Includes

*/
#include "DServerCmd.h"

/*
2 Class ~DServerCmdDeleteParam~

The class ~DServerCmdDeleteParam~ contains the parameters used in
the ~run~ - method of the class ~DServerCmdDelete~.

  * derives from class ~DServerParam~

*/
class DServerCmdDeleteParam 
  : public DServerParam
{
/*
2.2 Constructor

*/
public:
  DServerCmdDeleteParam()
    : DServerParam()
  {}
/*
2.3 Copy - Constructor

*/
  DServerCmdDeleteParam(const DServerCmdDeleteParam & inP)
    : DServerParam(inP){}

/*
2.4 Destructor

*/
  virtual ~DServerCmdDeleteParam() {}

/*
2.6.4 Method ~bool useChilds() const~

  * not using childs, we are fast enough

*/
  bool useChilds() const { return false; }

/*
2.6 Private Section

*/
private:
/*
2.6.1 Private Methods

*/
// n/a

/*
2.6.1 Private Members

*/
// n/a
/*
2.7 End of Class

*/
};

/* 
3 Class ~DServerCmdDelete~

The class ~DServerCmdDelete~ provides the functionality of deleteing data of a
darray index.

  * derives from the class ~DServerCmd~

*/
class DServerCmdDelete 
  : public DServerCmd
{
/*
3.1 Private Default Constructor

  * inherited from the class ~DServerCmd~

  * may not be used!

*/
public:
/*
3.1 Constructor

*/

  DServerCmdDelete()
    : DServerCmd(DServerCmd::DS_CMD_DELETE)
  {}

/*
3.2 Destructor

*/

  virtual ~DServerCmdDelete() {}

/*
2.3.3 Method ~string getInfo const~

  * returns string - an informaiton string

*/
  string getInfo() const 
  {
    return string("CMD-Delete :"  + getWorker() -> getName() + 
                  getIndexStr());
  }

/*

3.4 Running

3.4.1 Method ~void run~

method definition

*/

  void run();

/*
3.5 Private Section

*/
private:

/*
3.5.1 Private Methods

*/
// n/a

/*
3.5.2 Private Members

*/
// n/a

/*
3.6 End of Class

*/

};

#endif // H_DSERVERCMDDELETE_H
