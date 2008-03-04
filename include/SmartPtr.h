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
     template<class Base2T> (Save)SmartPtr<Base2T,CntrT> dynamicCast()
     template<class Base2T> (Save)SmartPtr<Base2T,CntrT> staticCast()
----
The "refs"[4] and "maxRefs"[4] methods could be used to return the current count of references and the maximum count of references, that the reference counter could store, unless it overflows. The "defined"[4] method returns "true"[4] if the smart pointer curently refers to an object. The "reset"[4] method removes the reference to the object (if no reference exists, nothing happens). The "clone"[4] method returns a new smart pointer, which refers to a copy of the refered object, that means, the refered object will also be copied. If only the reference counter should be increased, use the copy constructor instead.

The cast methods are implemented as template methods and could be used to return a smart pointer of the template type. For this, the refered object is casted to the respective type by using "static[_]cast"[4] or "dynamic[_]cast"[4]. The new smart pointer refers to the same object and use the same reference counter.

Beyond the mentioned methods, the following operators are avaliable:

---- operator =
     operator ->
     operator ==
     operator !=
----
The "="[4] operator could be used to assign a regular pointer, a "SmartPointer"[4] or a "SaveSmart-"[4] "Pointer"[4] to the smart pointer on left hand side. The "->"[4] operator is used to access the refered object, equal to regular pointers. The "=="[4] operator returns true, if both smart pointers refer to the same object.

2 Includes and defines

*/
#ifndef __SMART_PTR_H
#define __SMART_PTR_H

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

The counter reference could be initiated with another counter reference, which is needed from the cast methods of the smart pointer classes to use the same counter for different "BaseT"[4] objects.

********************************************************************/
template <typename BaseT, typename CntrT>
class RCHandle
{
public:
  inline RCHandle(BaseT* _ptr)
  : cntr(new CntrT(1)), ptr(_ptr)
  {}
/*
Constructor (creates a new RCHandle with one reference).

*/

  inline RCHandle(BaseT* _ptr, CntrT* _cntr)
  : cntr(_cntr), ptr(_ptr)
  { ++*cntr; }
/*
Constructor (creates a new RCHandle and increases the given reference counter).

*/

  inline ~RCHandle()
  { delete cntr; delete ptr;}
/*
Destructor (removes the reference counter and pointer).

*/

  CntrT* cntr; // the reference counter
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

  inline SmartPtr()
  : rcHandle(0)
  {}
/*
Constructor (creates a undefined smart pointer).

*/

  inline SmartPtr(BaseT* ptr)
  : rcHandle(ptr ? new RCHandle<BaseT, CntrT>(ptr) : 0)
  {}
/*
Constructor (assigns "ptr"[4] to the new "SmartPtr"[4] instance).

*/

  inline SmartPtr(BaseT* ptr, CntrT* cntr)
  : rcHandle(ptr ? new RCHandle<BaseT, CntrT>(ptr, cntr) : 0)
  {}
/*
Constructor (assigns "ptr"[4] to the new "SmartPtr"[4] instance).

*/

  inline SmartPtr(const SmartPtr<BaseT, CntrT>& ptr)
  {
    rcHandle = ptr.rcHandle;
    if (defined())
      ++*rcHandle->cntr;
  }
/*
Copy constructor (increases reference counter).

*/


  inline SmartPtr(const SaveSmartPtr<BaseT, CntrT>& ptr)
  {
    rcHandle = ptr.rcHandle;
    if (defined())
      ++*rcHandle->cntr;
  }
/*
Copy constructor for "SaveSmartPtr"[4] (increases reference counter).

*/

  inline ~SmartPtr()
  {
    if (defined() && (--*rcHandle->cntr == 0))
      delete rcHandle;
  }
/*
Destructor (decreases reference counter and deletes the assigned object, if no further references exist).

*/

  inline SmartPtr& clone() const
  {
    if (!defined())
      return new SmartPtr;
    else
      return new SmartPtr(rcHandle->ptr->clone());
  }
/*
Returns a new "SmartPointer"[4], which points to a deep copy of the refered object. The new pointer will have a reference count of 1.

*/

  template<class Base2T>
  inline SmartPtr<Base2T,CntrT> staticCast() const
  {
    SmartPtr<Base2T, CntrT> result(
        static_cast<Base2T*>(rcHandle->ptr), rcHandle->cntr);
    return result;
  }
/*
Returns a new SmartPtr, which uses the same ref. counter as the current one, but refers to a "Base2T"[4] object instead of a "BaseT"[4] object ("BaseT"[4] must be castable to "Base2T"[4]).

*/

  template<class Base2T>
  inline SmartPtr<Base2T,CntrT> dynamicCast() const
  {
    SmartPtr<Base2T, CntrT> result(
        dynamic_cast<Base2T*>(rcHandle->ptr), rcHandle->cntr);
    return result;
  }
/*
Returns a new "SmartPtr"[4], which uses the same ref. counter as the current one, but refers to a "Base2T"[4] object instead of a "BaseT"[4] object.

"BaseT"[4] must be castable to "Base2T"[4], otherwhise the result will be set to undefined, which could be used to determine that the "dynamic[_]cast"[4] has been failed.

*/

  inline BaseT* operator->()
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }
/*
Reference operator.

*/

  inline const BaseT* operator->() const
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }
/*
Constant reference operator.

*/

  inline SmartPtr<BaseT, CntrT>& operator=(
      const SmartPtr<BaseT, CntrT>& rhs)
  {
    if (rhs.defined())
      ++*rhs.rcHandle->cntr;

    reset();

    rcHandle = rhs.rcHandle;
    return *this;
  }
/*
Assignment operator (assigns another "SmartPtr"[4] instance to the current smart pointer).

*/

  inline SmartPtr<BaseT, CntrT>& operator=(
      const SaveSmartPtr<BaseT, CntrT>& rhs)
  {
    if (rhs.defined())
      ++*rhs.rcHandle->cntr;

    reset();

    rcHandle = rhs.rcHandle;
    return *this;
  }
/*
Assignment operator (assigns a "SaveSmartPtr"[4] instance to the current pointer instance).

*/

  inline SmartPtr<BaseT, CntrT>& operator=(BaseT* rhs)
  {
    reset();

    if (rhs)
      rcHandle = new RCHandle<BaseT, CntrT>(rhs);

    return *this;
  }
/*
Assignment operator (assigns a new "BaseT"[4] object to the current smart pointer).

*/

  inline CntrT* getRefCntr() const
  {
    if (rcHandle)
      return rcHandle->cntr;
    else
      return 0;
  }
/*
Returns a reference to the reference counter.

*/

  inline CntrT refs() const
  {
    if (rcHandle)
      return *rcHandle->cntr;
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
  { return !(lhs == rhs); }
/*
Unequal operator for assigned object references.

*/

  inline bool defined() const
  { return (rcHandle != 0); }
/*
Returns "true"[4], if an object has been assigned to the pointer.

*/

  inline void reset()
  {
    if (defined() && --*rcHandle->cntr == 0)
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
3.3 Class "SaveSmartPtr"[4]

This class is equal to "SmartPtr"[4], but could handle counter overflows, which leads to a slightly reduced performace of the copy-constructor and assignment method. Furthermore the refered object needs to provide a clone() method, which calls the copy constructor of the respective class.

********************************************************************/
template <typename BaseT, typename CntrT>
class SaveSmartPtr
{
public:
  friend class SmartPtr<BaseT, CntrT>;
  typedef CntrT counterType;

  inline SaveSmartPtr()
  : rcHandle(0)
  {}
/*
Constructor (creates a undefined "SaveSmartPtr"[4] instance):

*/

  inline SaveSmartPtr(BaseT* ptr)
  : rcHandle(ptr ? new RCHandle<BaseT, CntrT>(ptr) : 0)
  {}
/*
Constructor (assigns "ptr"[4] to the new "SaveSmartPtr"[4] instance).

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
        ++*rcHandle->cntr;
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
Copy constructor (increases the reference counter. If neccesary, a copy of the refered object would be created instead).

*/

  inline ~SaveSmartPtr()
  {
    if (defined() && (--*rcHandle->cntr == 0))
      delete rcHandle;
  }
/*
Destructor (decreases reference counter and deletes the assigned object, if no further references exist).

*/

  inline SaveSmartPtr& clone() const
  {
    if (!defined())
      return new SaveSmartPtr;
    else
      return new SaveSmartPtr(rcHandle->ptr->clone());
  }
/*
Returns a new smart pointer, which points to a deep copy of the refered object. The new pointer will have a reference count of 1.

*/

  template<class Base2T>
  inline SmartPtr<Base2T,CntrT> staticCast() const
  {
    SmartPtr<Base2T, CntrT> result(
        static_cast<Base2T*>(rcHandle->ptr), rcHandle->cntr);
    return result;
  }
/*
Returns a new SmartPtr, which uses the same ref. counter as the current one, but refers to a "Base2T"[4] object instead of a "BaseT"[4] object. ("BaseT"[4] must be castable to "Base2T"[4]).

*/

  template<class Base2T>
  inline SmartPtr<Base2T,CntrT> dynamicCast() const
  {
    SmartPtr<Base2T, CntrT> result(
        dynamic_cast<Base2T*>(rcHandle->ptr), rcHandle->cntr);
    return result;
  }
/*
Returns a new "SaveSmartPtr"[4], which uses the same ref. counter as the current one, but refers to a "Base2T"[4] object instead of a "BaseT"[4] object.

"BaseT"[4] must be castable to "Base2T"[4], otherwhise the result will be set to undefined, which could be used to determine that the "dynamic[_]cast"[4] has been failed.

*/

  inline BaseT* operator->()
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }
/*
Reference operator.

*/

  inline const BaseT* operator->() const
  {
    if (!defined())
      return 0;
    else
      return rcHandle->ptr;
  }
/*
Constant reference oparator.

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
        ++*rhs.rcHandle->cntr;
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
    if (defined() && (--*rcHandle->cntr == 0))
      delete rcHandle;

    // assign new reference counter
    rcHandle = newRCHandle;
    return *this;
  }
/*
Assignment operator (assigns another "SaveSmartPtr"[4] instance to the current pointer instance).

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
        ++*rhs.rcHandle->cntr;
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
    if (defined() && (--*rcHandle->cntr == 0))
      delete rcHandle;

    // assign new reference counter
    rcHandle = newRCHandle;
    return *this;
  }
/*
Assignment operator (assigns a "SmartPtr"[4] instance to the current pointer instance).

*/

  inline SaveSmartPtr<BaseT, CntrT>& operator=(BaseT* rhs)
  {
    if (defined() && (--*rcHandle->cntr == 0))
      delete rcHandle;

    if (rhs)
      rcHandle = new RCHandle<BaseT, CntrT>(rhs);

    return *this;
  }
/*
Assignment operator (assigns a new object reference to the current pointer instance).

*/

  inline CntrT* getRefCntr() const
  {
    if (rcHandle)
      return rcHandle->cntr;
    else
      return 0;
  }
/*
Returns a reference to the reference counter.

*/

  inline CntrT refs() const
  {
    if (rcHandle)
      return *rcHandle->cntr;
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
Equal operator for assigned object references.

*/

  friend inline bool operator!=(
      const SaveSmartPtr<BaseT, CntrT>& lhs,
      const SaveSmartPtr<BaseT, CntrT>& rhs)
  { return !(lhs == rhs); }
/*
Unequal operator for assigned object references.

*/

  inline bool defined() const
  { return rcHandle != 0; }
/*
Returns "true"[4], if an object has been assigned to the pointer.

*/

  inline void reset()
  {
    if (defined() && --*rcHandle->cntr == 0)
      delete rcHandle;
    rcHandle = 0;
  }
/*
Removes object assignment and deletes the assigned object, if no further references exist.

*/

protected:
  RCHandle<BaseT, CntrT>* rcHandle;
  // reference counter and ref. to object
};

/*
4 Cast methods

The following methods could be used, if the cast method of the smart pointer classes could not be used, e.g. if they are needed within template classes (calling template member methods from template classes seems curently not to work when compiling secondo under windows).

The "SrcT"[4] and "TargetT"[4] template parameter must be the "BaseT"[4] type of the "node"[4] and the result object, respectively. The "CntrT"[4] must be the common counter type of both objects, which could be obtained from the "counterType"[4] typedef in the smart pointer classes.

*/
namespace smart_pointer
{
  template<class SrcT, class TargetT, class CntrT>
  inline SmartPtr<TargetT, CntrT> staticCast(
      SmartPtr<SrcT, CntrT> node)
  {
    SmartPtr<TargetT, CntrT> result(static_cast<TargetT*>(
        node.operator->()), node.getRefCntr());
    return result;
  }

  template<class SrcT, class TargetT, class CntrT>
  inline SmartPtr<TargetT, CntrT> dynamicCast(
      SmartPtr<SrcT, CntrT> node)
  {
    SmartPtr<TargetT, CntrT> result(dynamic_cast<TargetT*>(
        node.operator->()), node.getRefCntr());
    return result;
  }
} // namespace smart_pointer

namespace save_smart_pointer
{
  template<class SrcT, class TargetT, class CntrT>
  inline SaveSmartPtr<TargetT, CntrT> staticCast(
      SaveSmartPtr<SrcT, CntrT> node)
  {
    SaveSmartPtr<TargetT, CntrT> result(static_cast<TargetT*>(
        node.operator->()), node.getRefCntr());
    return result;
  }

  template<class SrcT, class TargetT, class CntrT>
  inline SaveSmartPtr<TargetT, CntrT> dynamicCast(
      SaveSmartPtr<SrcT, CntrT> node)
  {
    SaveSmartPtr<TargetT, CntrT> result(dynamic_cast<TargetT*>(
        node.operator->()), node.getRefCntr());
    return result;
  }
} // namespace save_smart_pointer

#endif // #ifndef __SMART_PTR_H
