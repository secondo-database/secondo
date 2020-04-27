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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: ThreadsafeQueue

September 2019, Fischer Thomas

1.1 Overview

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_THREADSAFE_QUEUE_H
#define SECONDO_PARTHREAD_THREADSAFE_QUEUE_H

#include <memory>
#include <queue>
#include <mutex>

namespace parthread
{

template <typename T>
class ThreadsafeQueue
{
public: //methods
  ThreadsafeQueue(size_t maxCapacity = SIZE_MAX)
  : m_maxCapacity(maxCapacity)
  {

  };

  ThreadsafeQueue(const ThreadsafeQueue &other)
  {
    std::lock_guard<std::mutex> lock(other.m_access);
    m_queue = other.m_queue;
  };

  ~ThreadsafeQueue()
  {
  };

  ThreadsafeQueue &operator=(const ThreadsafeQueue &other) = delete;

  bool TryPush(T&& value)
  {
    std::lock_guard<std::mutex> lock(m_access);
    if (m_queue.size() <= m_maxCapacity)
    {
      m_queue.push(std::move(value));
      return true;
    }
    return false;
  };

  bool TryPush(const T value)
  {
    std::lock_guard<std::mutex> lock(m_access);
    if (m_queue.size() <= m_maxCapacity)
    {
      m_queue.push(value);
      return true;
    }
    return false;
  };

  bool TryPop(T& value)
  {
    std::lock_guard<std::mutex> lock(m_access);
    if (!m_queue.empty())
    {
      value = std::move(m_queue.front());
      m_queue.pop();
      return true;
    }

    return false;
  };

  size_t Size() const
  {
    std::lock_guard<std::mutex> lock(m_access);
    return m_queue.size();
  };

  bool IsEmpty() const
  {
    std::lock_guard<std::mutex> lock(m_access);
    return m_queue.empty();
  };

private: //methods
private: //member
  const size_t m_maxCapacity;
  mutable std::mutex m_access;
  std::queue<T> m_queue;
};

} // namespace parthread
#endif
