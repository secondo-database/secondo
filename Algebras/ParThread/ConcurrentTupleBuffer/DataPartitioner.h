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

1 Header File: DataPartitioner

September 2019, Fischer Thomas

1.1 Overview

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_DATA_PARTITONER_H
#define SECONDO_PARTHREAD_DATA_PARTITONER_H

#include "../../ExtRelation-2/HashJoin.h"

namespace parthread
{

enum class DistributionTypes
{
  SharedPartitions,
  DedicatedPartition
};

class DataPartitioner
{
public:
  DataPartitioner(size_t numPartitions, DistributionTypes distType)
      : m_numPartitions(numPartitions), m_distType(distType)
  {
  }

  virtual ~DataPartitioner() = default;

  DistributionTypes DistributionType() const
  {
    return m_distType;
  };

  size_t NumPartitions() const
  {
    return m_numPartitions;
  };

  virtual DataPartitioner *Copy() const = 0;

  virtual size_t DistributeValue(Tuple *tuple) = 0;

private: //member
  size_t m_numPartitions;
  DistributionTypes m_distType;
};

typedef std::shared_ptr<DataPartitioner> IDataPartitionerPtr;

class HashDataPartitioner : public DataPartitioner
{
public: //methods
  HashDataPartitioner(const size_t numPartitions, const int attrIndex)
      : DataPartitioner(numPartitions, DistributionTypes::DedicatedPartition),
        m_attrIndex(attrIndex), m_hashFunction(numPartitions, attrIndex)
  {
  }

  ~HashDataPartitioner() = default;

  virtual DataPartitioner *Copy() const
  {
    return new HashDataPartitioner(NumPartitions(), m_attrIndex);
  }

  virtual size_t DistributeValue(Tuple *tuple)
  {
    return m_hashFunction.Value(tuple);
  }

private: //member
  int m_attrIndex;
  extrel2::HashFunction m_hashFunction;
};

class RoundRobinDataPartitioner : public DataPartitioner
{
public: //methods
  RoundRobinDataPartitioner(const size_t numPartitions)
      : DataPartitioner(numPartitions, DistributionTypes::SharedPartitions),
        m_currentPartition(numPartitions)
  {
  }

  ~RoundRobinDataPartitioner() = default;

  virtual DataPartitioner *Copy() const
  {
    return new RoundRobinDataPartitioner(NumPartitions());
  }

  virtual size_t DistributeValue(Tuple *tuple)
  {
    m_currentPartition++;
    if (m_currentPartition >= NumPartitions())
    {
      m_currentPartition = 0;
    }

    return m_currentPartition;
  }

private: //member
  size_t m_currentPartition;
};

} // namespace parthread
#endif //SECONDO_PARTHREAD_DATA_PARTITONER_H
