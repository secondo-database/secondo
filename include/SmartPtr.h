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

[11] Headerfile "SmartPtr.h"[4]

January-February 2008, Mirko Dibbert

[TOC]
\newpage

1 Overview

This headerfile contains two classes, which implement a smart pointer for any object type. Smart pointers could be used similar to regular pointers, but additionally, they use a reference count mechanism, which increments a reference counter when calling the copy constructor, clone method or assignement operator instead of copying the refered object. When a smart pointer is deleted, the reference counter will be decreased. The refered object will be deleted as soon as the reference counter reaches zero.

This file contains two implementations of smart pointers: "SmartPtr"[4] and "SaveSmartPtr"[4]. Both classes are nearly equal, but the "SaveSmartPtr"[4] class also handles overflow of the reference counter. For this, the refered object must provide a "clone"[4] method, that returns a copy of the object. A "clone"[4] method is also needed for both smart pointer types, if the "clone"[4] method of the smart pointer should be used.

To assign an object to a smart pointer, use the respctive constructur or the assingment operator.

Warning: Never assign the same object to more than one smart pointer, since each assignment creates a new reference counter and if one of these counters reaches 0, the object would be deleted, even if it is stored in another smart pointer (if needed, assign a copy of the object to the second smart pointer instead).

The following example show, how an object reference could be assigned to a smart pointer:

---- SmartPtr<myClass> ptr1(new myClass());
     SmartPtr<myClass> ptr2 = new myClass();
----
To get a new reference of an object and increment the reference couter, use the copy constructor or the assignment operator for smart pointers:

---- SaveSmartPtr<myClass> ptr1(new myClass());
     SaveSmartPtr<myClass> ptr2 = ptr1;
     SaveSmartPtr<myClass> ptr3(ptr1);
----
All these pointers refer to the same instance of "myClass"[4] and the common reference counter has a value of 3. If the pointers will be deleted, the reference counter will be automatically decreased and - if the counter reaches a value of 0 - the refered object will be deleted.

It is also possible to assign a "SmartPointer"[4] to a "SaveSmartPointer"[4] and vice versa.

The refered object could be accessed by the "{->}"[4] operator, similar to regular pointers.
\\[3ex]
The smart pointer classes are designed as template classes, which template parameters have the following meaning:

  * "BaseT"[4] : (object type, which should be stored in the pointer)

  * "CntrT"[4] : (type of the reference counter (optional), which is "unsigned long" by default)

\newpage
Both smart pointers classes provide the following methods:

---- const CntrT refs()
     const CntrT maxRefs()
     bool defined() const
     void reset()
     (Save)SmartPtr& clone()
----
The "refs"[4] and "maxRefs"[4] methods could be used to return the current count of references and the maximum count of references, that the reference counter could store, unless it overflows. The "defined"[4] method returns "true"[4] if the smart pointer curently refers to an object. The "reset"[4] method removes the reference to the object (if no reference exists, nothing happens). The "clone"[4] method returns a new smart pointer, which refers to a copy of the refered object, that means, the refered object will also be copied. If only the reference counter should be increased, use the copy constructor instead.

Beyond the mentioned methods, the following operators are avaliable:

---- operator =
     operator ->
     operator ==
     operator !=
----
The "="[4] operator could be used to assign a regular pointer, a "SmartPointer"[4] or a "SaveSmart-"[4] "Pointer"[4] to the smart pointer on left hand side. The "->"[4] operator is used to access the refered object, equal to regular pointers. The "=="[4] operator returns true, if both smart pointers refer to the same object.

2 Includes and defines

*/
#ifndef __SMART_PTR_H____
#define __SMART_PTR_H____

#include <limits>

using namespace std;

template <typename BaseT, typename CntrT = unsigned long>
class SaveSmartPtr; // forward declaration

template <typename BaseT, typename CntrT = unsigned long>
class SmartPtr; // forward declaration

/********************************************************************
3 Class declarations

3.1 Class "RCHandle"[4]

This class is used in the "SmartPtr"[4] and the "SaveSmartPtr"[4] class to provide a reference counter for a "BaseT"[4] instance. The type of the counter could be selected by the "CntrT"[4] template paremeter.

********************************************************************/
template <typename BaseT, typename CntrT>
class RCHandle
{
public:
/*
Constructor (creates a new RCHandle with one reference).

*/
  inline RCHandle(BaseT* _ptr)
    : cntr(1), ptr(_ptr)
  {}

/*
Destructor (removes the reference counter and pointer).

*/
  inline ~RCHandle()
  { delete ptr;}

  CntrT cntr;       // the reference counter
  BaseT* const ptr; // pointer to refered object
};

/********************************************************************
3.2 Class "SmartPtr"[4]

********************************************************************/
template <typename BaseT, typename CntrT>
class SmartPtr
{
public:
  friend class SaveSmartPtr<BaseT, CntrT>;
  typedef CntrT counterType;

/*
Constructor (creates a undefined smart pointer).

*/
  inline SmartPtr()
    : rcHandle(0)
  {}

/*
Constructor (assigns "ptr"[4] to the new "SmartPtr"[4] instance).

*/
  inline SmartPtr(BaseT* ptr)
    : rcHandle(ptr ? new RCHandle<BaseT, CntrT>(ptr) : 0)
  {}

/*
Copy constructor (increases reference counter).

*/
  inline SmartPtr(const SmartPtr<BaseT, CntrT>& ptr)
  {
    rcHandle = ptr.rcHandle;
    if (defined())
      ++rcHandle->cntr;
  }

/*
Copy constructor for "SaveSmartPtr"[4] (increases reference counter).

*/
  inline SmartPtr(const SaveSmartPtr<BaseT, CntrT>& ptr)
  {
    rcHandle = ptr.rcHandle;
    if (defined())
      ++rcHandle->cntr;
  }

/*
Destructor (decreases reference counter and deletes the assigned object, if no further references exist).

*/
  inline ~SmartPtr()
  {
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;
  }

/*
Returns a new "SmartPointer"[4], which points to a deep copy of the refered object. The new pointer will have a reference count of 1.

*/
  inline SmartPtr& clone() const
  {
    if (!defined())
      return new SmartPtr;
    else
      return new SmartPtr(rcHandle->ptr->clone());
  }

/*
Reference operator.

*/
  inline BaseT* operator->()
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }

/*
Constant reference operator.

*/
  inline const BaseT* operator->() const
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }

/*
Assignment operator (assigns another "SmartPtr"[4] instance to the current smart pointer).

*/
  inline SmartPtr<BaseT, CntrT>& operator=(
      const SmartPtr<BaseT, CntrT>& rhs)
  {
    if (rhs.defined())
      ++rhs.rcHandle->cntr;

    reset();

    rcHandle = rhs.rcHandle;
    return *this;
  }

/*
Assignment operator (assigns a "SaveSmartPtr"[4] instance to the current pointer instance).

*/
  inline SmartPtr<BaseT, CntrT>& operator=(
      const SaveSmartPtr<BaseT, CntrT>& rhs)
  {
    if (rhs.defined())
      ++rhs.rcHandle->cntr;

    reset();

    rcHandle = rhs.rcHandle;
    return *this;
  }

/*
Assignment operator (assigns a new "BaseT"[4] object to the current smart pointer).

*/
  inline SmartPtr<BaseT, CntrT>& operator=(BaseT* rhs)
  {
    reset();

    if (rhs)
      rcHandle = new RCHandle<BaseT, CntrT>(rhs);

    return *this;
  }

/*
Returns a reference to the reference counter.

*/
  inline CntrT* getRefCntr() const
  {
    if (rcHandle)
      return rcHandle->cntr;
    else
      return 0;
  }

/*
Returns current count of references to the object.

*/
  inline CntrT refs() const
  {
    if (rcHandle)
      return *rcHandle->cntr;
    else
      return CntrT();
  }

/*
Returns the maximum number of referencses, which the counter can hold.


*/
  inline CntrT maxRefs() const
  {
    return numeric_limits<CntrT>::max();
  }

/*
Equal operator for assigned object references.

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
Unequal operator for assigned object references.

*/
  friend inline bool operator!=(
      const SmartPtr<BaseT, CntrT>& lhs,
      const SmartPtr<BaseT, CntrT>& rhs)
  { return !(lhs == rhs); }

/*
Returns "true"[4], if an object has been assigned to the pointer.

*/
  inline bool defined() const
  { return (rcHandle != 0); }

/*
Removes object assignment and deletes the assigned object,
if no further references exist.

*/
  inline void reset()
  {
    if (defined() && --rcHandle->cntr == 0)
      delete rcHandle;
    rcHandle = 0;
  }

protected:
  RCHandle<BaseT, CntrT>* rcHandle;
  // reference counter and ref. to object
};

/********************************************************************
3.3 Class "SaveSmartPtr"[4]

This class is equal to "SmartPtr"[4], but could handle counter overflows, which leads to a slightly reduced performace of the copy-constructor and assignment method. Furthermore the refered object needs to provide a clone() method, which calls the copy constructor of the respective class.

********************************************************************/
template <typename BaseT, typename CntrT>
class SaveSmartPtr
{
public:
  friend class SmartPtr<BaseT, CntrT>;
  typedef CntrT counterType;

/*
Constructor (creates a undefined "SaveSmartPtr"[4] instance):

*/
  inline SaveSmartPtr()
  : rcHandle(0)
  {}

/*
Constructor (assigns "ptr"[4] to the new "SaveSmartPtr"[4] instance).

*/
  inline SaveSmartPtr(BaseT* ptr)
  : rcHandle(ptr ? new RCHandle<BaseT, CntrT>(ptr) : 0)
  {}

/*
Copy constructor (increases the reference counter. If neccesary, a copy of the refered object would be created instead).

*/
  SaveSmartPtr(const SaveSmartPtr<BaseT, CntrT>& ptr)
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
            ptr.rcHandle->ptr->clone());
      }
    }
    else
      rcHandle = 0;
  }

/*
Destructor (decreases reference counter and deletes the assigned object, if no further references exist).

*/
  inline ~SaveSmartPtr()
  {
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;
  }

/*
Returns a new smart pointer, which points to a deep copy of the refered object. The new pointer will have a reference count of 1.

*/
  inline SaveSmartPtr& clone() const
  {
    if (!defined())
      return new SaveSmartPtr;
    else
      return new SaveSmartPtr(rcHandle->ptr->clone());
  }

/*
Reference operator.

*/
  inline BaseT* operator->()
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }

/*
Constant reference oparator.

*/
  inline const BaseT* operator->() const
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }

/*
Assignment operator (assigns another "SaveSmartPtr"[4] instance to the current pointer instance).

*/
  SaveSmartPtr<BaseT, CntrT>& operator=(
      const SaveSmartPtr<BaseT, CntrT>& rhs)
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
            rhs.rcHandle->ptr->clone());
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
Assignment operator (assigns a "SmartPtr"[4] instance to the current pointer instance).

*/
  SaveSmartPtr<BaseT, CntrT>& operator=(
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
            rhs.rcHandle->ptr->clone());
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
Assignment operator (assigns a new object reference to the current pointer instance).

*/
  inline SaveSmartPtr<BaseT, CntrT>& operator=(BaseT* rhs)
  {
    if (defined() && (--rcHandle->cntr == 0))
      delete rcHandle;

    if (rhs)
      rcHandle = new RCHandle<BaseT, CntrT>(rhs);

    return *this;
  }

/*
Returns a reference to the reference counter.

*/
  inline CntrT* getRefCntr() const
  {
    if (rcHandle)
      return rcHandle->cntr;
    else
      return 0;
  }

/*
Returns current count of references to the object.

*/
  inline CntrT refs() const
  {
    if (rcHandle)
      return *rcHandle->cntr;
    else
      return CntrT();
  }

/*
Returns the maximum number of referencses, which the counter can hold.

*/
  inline CntrT maxRefs() const
  {
    return numeric_limits<CntrT>::max();
  }

/*
Equal operator for assigned object references.

*/
  friend inline bool operator==(
      const SaveSmartPtr<BaseT, CntrT>& lhs,
      const SaveSmartPtr<BaseT, CntrT>& rhs)
  {
    if (!lhs.defined())
      return (!rhs.defined());
    else if (!rhs.defined())
      return false;
    else
      return lhs.rcHandle->ptr == rhs.rcHandle->ptr;
  }

/*
Unequal operator for assigned object references.

*/
  friend inline bool operator!=(
      const SaveSmartPtr<BaseT, CntrT>& lhs,
      const SaveSmartPtr<BaseT, CntrT>& rhs)
  { return !(lhs == rhs); }

/*
Returns "true"[4], if an object has been assigned to the pointer.

*/
  inline bool defined() const
  { return rcHandle != 0; }

/*
Removes object assignment and deletes the assigned object, if no further references exist.

*/
  inline void reset()
  {
    if (defined() && --rcHandle->cntr == 0)
      delete rcHandle;
    rcHandle = 0;
  }

protected:
  RCHandle<BaseT, CntrT>* rcHandle;
  // reference counter and ref. to object
};

#endif // #ifndef __SMART_PTR_H__
