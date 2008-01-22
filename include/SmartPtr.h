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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]
//paragraph [11] Title: [{\Large \bf \begin{center}] [\end{center}}]
//[TOC] [\tableofcontents]

[11] Headerfile "RefCntr.h"[4]

January 2008, Mirko Dibbert

[TOC]

1 Overview


2 Includes and defines

*/
#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

#include <limits>

using namespace std;

template <typename BaseT, typename CntrT = unsigned>
class SmartPtr; // forward declaration

template <typename BaseT, typename CntrT = unsigned>
class FastSmartPtr; // forward declaration

/********************************************************************
3 Class declarations

3.1 Class "RCHandle"[4]

This class is used in the "SmartPtr"[4] and the "FastSmartPtr"[4] class to
provide a reference counter for an "BaseT"[4] instance. The type of the
counter could be selected by the "CntrT"[4] template paremeter.

********************************************************************/
template <typename BaseT, typename CntrT = unsigned>
class RCHandle
{
  friend class SmartPtr<BaseT, CntrT>;
  friend class FastSmartPtr<BaseT, CntrT>;

  inline RCHandle(BaseT* _ptr)
  : cntr(1), ptr(_ptr)
  {}

  inline ~RCHandle()
  {
    delete ptr;
  }

  CntrT cntr;
  BaseT* const ptr;
};

/********************************************************************
3.2 Class "SmartPtr"[4]

This class manage references to shared objects. Smart pointers implement a
reference count mechanism and could be used similar to normal pointers.

To assign an object to a smart pointer, use the respctive constructur or the
assingment operator.

Warning: Never assign the same "BaseT"[4] object to more than one smart pointer,
since each assignment creates a new reference counter and if one of the reference
counter reaches 0, the object would be deleted, even if it is stored in
other smart pointers.

The following example show, how a reference could be assigned to the smart
pointer:

----
SmartPtr<myClass> ptr1(new myClass());
SmartPtr<myClass> ptr2 = new myClass();
----
To get a new reference of an object and increment the reference couter,
use the copy constructor or the assignment operator for smart pointers:

----
SmartPtr<myClass> ptr1(new myClass());
SmartPtr<myClass> ptr2 = ptr1;
SmartPtr<myClass> ptr3(ptr1);
----
All these pointers refer to the same instance of "myClass"[4] and the common
reference counter has a value of 3. If the pointers will be deleted, the
reference counter will automatically be decreased and - if the counter reaches
a value of 0 - the refered object will be automatically deleted.

If the reference counter would overflow, the new smart pointer would point to
a copy of the original object instead of increasing the reference count.

It is possible to assign fast smart pointers to a smart pointer and vice versa.

The contained object could be accessed by the "{->}"[4] operator,
similar to normal pointers.
\\[3ex]
Meaning of the template parameters:

  * "BaseT"[4] : (object type, which should be stored in the pointer)

  * "CntrT"[4] : (type of the reference counter (optional), which is
  "unsigned int" by default)

This class contains the following methods:

  * defined (true, if the pointer contains a reference)

  * reset (removes the contained reference)

  * clone (returns a new smart pointer, which points to a deep copy of
  the refered object)

  * refs (returns the count of references to the refered object)

  * maxRefs (returns the maximum count of references, wich the
  pointer could hold)

This class contains the following operators:

  * "->"[4] operator

  * "="[4] operator (assings a new reference or a new (fast) smart pointer
  to the smart pointer)

  * "=="[4] operator (true, if both pointers refer to the same objects)

  * "!="[4] operator (true, if both pointers refer to different objects)

********************************************************************/
template <typename BaseT, typename CntrT>
class SmartPtr
{
public:
  friend class FastSmartPtr<BaseT, CntrT>;

  inline SmartPtr()
  : rcHandle(0)
  {}
/*
Constructor (creates a undefined "SmartPtr"[4] instance):

*/

  inline SmartPtr(BaseT* ptr)
  : rcHandle(ptr ? new RCHandle<BaseT, CntrT>(ptr) : 0)
  {}
/*
Constructor (assigns "ptr"[4] to the new "SmartPtr"[4] instance).

*/

  SmartPtr(const SmartPtr<BaseT, CntrT>& ptr)
  {
    /* increment reference counter of ptr - in case of counter
       overflow a new reference counter object will be created */
    if (ptr.defined())
    {
      if(ptr.refs() < maxRefs())
      {
        rcHandle = ptr.rcHandle;
        ++rcHandle->cntr;
      }
      else
      {
        rcHandle = new RCHandle<BaseT, CntrT>(
            new BaseT(*ptr.rcHandle->ptr));
      }
    }
    else
      rcHandle = 0;
  }
/*
Copy constructor (increases the reference counter. If neccesary, a copy
of the refered object would be created instead).

*/

  inline ~SmartPtr()
  {
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;
  }
/*
Destructor (decreases reference counter and deletes the assigned object,
if no further references exist).

*/

  inline SmartPtr& clone()
  {
    return new SmartPtr(new BaseT(*rcHandle->ptr));
  }
/*
Returns a new smart pointer, which points to a deep copy of the refered object.
The new pointer will have a reference count of 1.

*/

  inline BaseT* operator->()
  {
    return rcHandle->ptr;
  }
/*
Reference operator.

*/

  inline const BaseT* operator->() const
  {
    return rcHandle->ptr;
  }
/*
Constant reference oparator.

*/

  SmartPtr<BaseT, CntrT>& operator=(
      const SmartPtr<BaseT, CntrT>& rhs)
  {
    /* increment reference counter of rhs - in case of counter
       overflow a new reference counter object will be created */
    RCHandle<BaseT, CntrT>* newRCHandle;
    if (rhs.defined())
    {
      if(rhs.refs() < maxRefs())
      {
        ++rhs.rcHandle->cntr;
        newRCHandle = rhs.rcHandle;
      }
      else
      {
        newRCHandle = new RCHandle<BaseT, CntrT>(
            new BaseT(*rhs.rcHandle->ptr));
      }
    }
    else
       newRCHandle = 0;

    // remove currently stored reference
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;

    // assign new reference counter
    rcHandle = newRCHandle;
    return *this;
  }
/*
Assignment operator (assigns another "SmartPtr"[4] instance to the
current pointer instance).

*/

  SmartPtr<BaseT, CntrT>& operator=(
      const FastSmartPtr<BaseT, CntrT>& rhs)
  {
    /* increment reference counter of rhs - in case of counter
       overflow a new reference counter object will be created */
    RCHandle<BaseT, CntrT>* newRCHandle;
    if (rhs.defined())
    {
      if(rhs.refs() < maxRefs())
      {
        ++rhs.rcHandle->cntr;
        newRCHandle = rhs.rcHandle;
      }
      else
      {
        newRCHandle = new RCHandle<BaseT, CntrT>(
            new BaseT(*rhs.rcHandle->ptr));
      }
    }
    else
       newRCHandle = 0;

    // remove currently stored reference
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;

    // assign new reference counter
    rcHandle = newRCHandle;
    return *this;
  }
/*
Assignment operator (assigns a "FastSmartPtr"[4] instance to the
current pointer instance).

*/

  inline SmartPtr<BaseT, CntrT>& operator=(BaseT* rhs)
  {
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;

    if (rhs)
      rcHandle = new RCHandle<BaseT, CntrT>(rhs);

    return *this;
  }
/*
Assignment operator (assigns a new object reference to the
current pointer instance).

*/

  inline CntrT refs() const
  {
    if (rcHandle)
      return rcHandle->cntr;
    else
      return CntrT();
  }
/*
Returns current count of references to the object.

*/

  inline CntrT maxRefs() const
  {
    return numeric_limits<CntrT>::max();
  }
/*
Returns the maximum number of referencses, which the counter can hold.

*/

  friend inline bool operator==(
      const SmartPtr<BaseT, CntrT>& lhs,
      const SmartPtr<BaseT, CntrT>& rhs)
  {
    if (!lhs.defined())
      return (!rhs.defined());
    else if (!rhs.defined())
      return false;
    else
      return lhs.rcHandle->ptr == rhs.rcHandle->ptr;
  }
/*
Equal operator for assigned object references.

*/

  friend inline bool operator!=(
      const SmartPtr<BaseT, CntrT>& lhs,
      const SmartPtr<BaseT, CntrT>& rhs)
  {
    return !(lhs == rhs);
  }
/*
Unequal operator for assigned object references.

*/

  inline bool defined() const
  {
    return rcHandle != 0;
  }
/*
Returns "true"[4], if an object has been assigned to the pointer.

*/

  inline void reset()
  {
    if (defined() && --rcHandle->cntr == 0)
      delete rcHandle;
    rcHandle = 0;
  }
/*
Removes object assignment and deletes the assigned object,
if no further references exist.

*/

protected:
  RCHandle<BaseT, CntrT>* rcHandle;
  // reference counter and ref. to object
};

/********************************************************************
3.3 Class "FastSmartPtr"[4]

This class is equal to "SmartPtr"[4], but do not handle counter overflows, which
leads to a slightly improved performace of the copy-constructor and assignment
method.

Another advantage is, that this class could handle objects, which have no public
copy constructor (unless the "clone"[4] method is called).

********************************************************************/
template <typename BaseT, typename CntrT>
class FastSmartPtr
{
public:
  friend class SmartPtr<BaseT, CntrT>;

  inline FastSmartPtr()
  : rcHandle(0)
  {}
/*
Constructor (creates a undefined smart pointer).

*/

  inline FastSmartPtr(BaseT* ptr)
  : rcHandle(ptr ? new RCHandle<BaseT, CntrT>(ptr) : 0)
  {}
/*
Constructor (assigns "ptr"[4] to the new "FastSmartPtr"[4] instance).

*/

  inline FastSmartPtr(const FastSmartPtr<BaseT, CntrT>& ptr)
  {
    rcHandle = ptr.rcHandle;
    if (defined())
      ++rcHandle->cntr;
  }
/*
Copy constructor (increases reference counter).

*/

  inline FastSmartPtr(const SmartPtr<BaseT, CntrT>& ptr)
  {
    rcHandle = ptr.rcHandle;
    if (defined())
      ++rcHandle->cntr;
  }
/*
Copy constructor for "SmartPtr"[4] (increases reference counter).

*/

  inline ~FastSmartPtr()
  {
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;
  }
/*
Destructor (decreases reference counter and deletes the assigned object,
if no further references exist).

*/

  inline FastSmartPtr& clone()
  {
    return new FastSmartPtr(new BaseT(*rcHandle->ptr));
  }
/*
Returns a new "FastSmartPointer"[4], which points to a deep copy
of the refered object. The new pointer will have a reference count of 1.

*/

  inline BaseT* operator->()
  {
    return rcHandle->ptr;
  }
/*
Reference operator.

*/

  inline const BaseT* operator->() const
  {
    return rcHandle->ptr;
  }
/*
Constant reference operator.

*/

  inline FastSmartPtr<BaseT, CntrT>& operator=(
      const FastSmartPtr<BaseT, CntrT>& rhs)
  {
    if (rhs.defined())
      ++rhs.rcHandle->cntr;

    reset();

    rcHandle = rhs.rcHandle;
    return *this;
  }
/*
Assignment operator (assigns another "FastSmartPtr"[4] instance to the
current pointer instance).

*/

  inline FastSmartPtr<BaseT, CntrT>& operator=(
      const SmartPtr<BaseT, CntrT>& rhs)
  {
    if (rhs.defined())
      ++rhs.rcHandle->cntr;

    reset();

    rcHandle = rhs.rcHandle;
    return *this;
  }
/*
Assignment operator (assigns a "SmartPtr"[4] instance to the
current pointer instance).

*/

  inline FastSmartPtr<BaseT, CntrT>& operator=(BaseT* rhs)
  {
    reset();

    if (rhs)
      rcHandle = new RCHandle<BaseT, CntrT>(rhs);

    return *this;
  }
/*
Assignment operator (assigns a new object reference to the
current pointer instance).

*/

  inline CntrT refs() const
  {
    if (rcHandle)
      return rcHandle->cntr;
    else
      return CntrT();
  }
/*
Returns current count of references to the object.

*/

  inline CntrT maxRefs() const
  {
    return numeric_limits<CntrT>::max();
  }
/*
Returns the maximum number of referencses, which the counter can hold.


*/

  friend inline bool operator==(
      const FastSmartPtr<BaseT, CntrT>& lhs,
      const FastSmartPtr<BaseT, CntrT>& rhs)
  {
    if (!lhs.defined())
      return (!rhs.defined());
    else if (!rhs.defined())
      return false;
    else
      return lhs.rcHandle->ptr == rhs.rcHandle->ptr;
  }
/*
Equal operator for assigned object references.

*/

  friend inline bool operator!=(
      const FastSmartPtr<BaseT, CntrT>& lhs,
      const FastSmartPtr<BaseT, CntrT>& rhs)
  {
    return !(lhs == rhs);
  }
/*
Unequal operator for assigned object references.

*/

  inline bool defined() const
  {
    return rcHandle != 0;
  }
/*
Returns "true"[4], if an object has been assigned to the pointer.

*/

  inline void reset()
  {
    if (defined() && --rcHandle->cntr == 0)
      delete rcHandle;
    rcHandle = 0;
  }
/*
Removes object assignment and deletes the assigned object,
if no further references exist.

*/

protected:
  RCHandle<BaseT, CntrT>* rcHandle;
  // reference counter and ref. to object
};
#endif
