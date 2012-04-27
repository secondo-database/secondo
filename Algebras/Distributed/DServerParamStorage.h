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
[1] Class DServerParamStorage Definition


\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

This file contains the defintions for the classes ~DServerParam~ 
and ~DServerParamStorage~. The ~DServerParam~ class is a
baseclass to store parameters used for objects, which will 
be run on the workers. Derived objects should extend this class
and define the parameters they need.
The class ~DServerParamStorage~ is the container baseclass, which provides
methods to store objects derived from the class ~DServerParam~.

IMPORTANT: The class ~DServerParamStorage~ owns the ~DServerParam~ object. 
Therefor it is necessary to make a copy of the ~DServerParam~ object
when setting it in the storage class.

This is needed, because the objects derived from the ~DServerParamStorage~
class are often used in threads. A thread would automatically destory this
object. For porper cleanup this class needs to own the ~DServerParam~ object.

*/

/*

1 Preliminaries

1.1 Defines

*/

#ifndef H_DSERVERPARAMSTORAGE_H
#define H_DSERVERPARAMSTORAGE_H
/*
1.2 Debug Output

uncomment the following line, if debug output should
be written to stdout

*/
//#define DS_CMD_DEBUG 1

/*
1.3 Includes

*/

#include "StandardTypes.h"

/*

1.4 Forward Declarations

*/

class DServerParam;

/*

2 Class ~DServerParamStorage~

This class provides the basic functionality to store a pointer
of an object ot DServerParam. This class takes ownership
of that object, since objects of this class might get automatically
destroyend, if used in threads.

*/

class DServerParamStorage
{
/*
2.1 Default Constructor

*/
public:
  DServerParamStorage() : m_param(NULL) {}

/*
2.2 Destructor

*/
  virtual ~DServerParamStorage();

/*

2.3 Setter Methods

2.3.1 Method ~template [<]class T[>] void setParam~

Stores the command specific parameter object. This method makes
a copy of the parameter object, because this class must have
the ownership of it. The reason is, that
the derived class may run in threads, where the data is
deleted automatically.

  * T[ast] inParam - pointer to the parameter obejct

*/
  template <class T>
  void setParam(const T* inParam)
  
  { 
    assert (m_param == NULL);
    m_param = new T(*inParam); 
  }

/*

2.4 Getter Methods

2.4.1 Method ~template [<]class T[>] const T[ast] getParam const~ 

Provides the command specific parameter class to the derived DServerCmd object.

  * returns const T[ast] - pointer to the stored parameter object

*/
  template <class T>
  const T* getParam() const
  
  { 
    assert (m_param != NULL);
    return dynamic_cast<T*> (m_param); 
  }
/*

2.5 Private Section

*/
private:
/*

2.5.1 Private Methods

n/a

*/
/*

2.5.2 Private Members

*/
  DServerParam* m_param;

/*

2.6 End of Class 

*/
};

/*

3 Class ~DServerParam~

The class ~DServerParam~ is the base class for all parameter classes. These
objects can be stored in the class ~DServerParamStorage~.

*/

class DServerParam 
{
public:

/*

3.1 Default Constructor

*/
  DServerParam() {}
/*

3.2 Copy - Constructor

*/
  // derived classes must have copy constructor!
  DServerParam(const DServerParam&) {} 
/*

3.3 Destructor

*/
  virtual ~DServerParam() {}
/*

3.4 Private Section

*/
private:
  // n/a

/*

3.5 End of Class 

*/
};

#endif // H_DSERVERPARAMSTORAGE_H
