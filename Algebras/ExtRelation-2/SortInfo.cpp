/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

1 Implementation File SortInfo.cpp

May 2009. S. Jungnickel. Initial version.


2 Includes

*/

#include <iomanip>
#include <sstream>
#include <queue>

#include "SortInfo.h"
#include "StopWatch.h"
#include "TupleBuffer.h"

using namespace std;

#define HLINE "---------------------------------------------------------------"

/*
3 Implementation of class ~SortedRunInfo~

*/

extrel2::SortedRunInfo::SortedRunInfo(int no)
: No(no)
, RunLength(0)
, RunSize(0)
, MinimumRunLength(0)
, AdditionalRunLength(0)
, TuplesPassedToNextRun(0)
, TuplesInMemory(0)
, TuplesOnDisk(0)
, RunRatio(0)
{
}

extrel2::SortedRunInfo::SortedRunInfo(extrel2::SortedRunInfo& info)
{
  No = info.No;
  RunLength = info.RunLength;
  RunSize = info.RunSize;
  MinimumRunLength = info.MinimumRunLength;
  AdditionalRunLength = info.AdditionalRunLength;
  TuplesPassedToNextRun = info.TuplesPassedToNextRun;
  TuplesInMemory = info.TuplesInMemory;
  TuplesOnDisk = info.TuplesOnDisk;
  RunRatio = info.RunRatio;
}

extrel2::SortedRunInfo::~SortedRunInfo()
{
}

float extrel2::SortedRunInfo::Ratio()
{
  return (float)RunLength / (float)MinimumRunLength;
}

float extrel2::SortedRunInfo::Size()
{
  return (float)RunSize / (float)( 1024 * 1024 );
}

ostream& extrel2::operator<<(ostream& stream, SortedRunInfo& info)
{
  stream << setw(2) << info.No << ": " << info.RunLength
         << " Tuples / " << info.Size() << " MB "
         << "(Minimum: " << info.MinimumRunLength
         << ", Added: " << info.AdditionalRunLength << ")"
         << " => Ratio: " << info.Ratio() << endl
         << "    TuplesPassedToNextRun: " << info.TuplesPassedToNextRun
         << " TuplesInMemory: " << info.TuplesInMemory
         << " TuplesOnDisk: " << info.TuplesOnDisk << endl;

  return stream;
}


/*
3 Implementation of class ~SortInfo~

*/

extrel2::SortInfo::SortInfo(size_t bufferSize) :
  BufferSize(bufferSize),
  IOBufferSize(TupleBuffer::GetIoBufferSize()),
  MaxMergeFanIn(0),
  F0(0),
  W(0),
  IntermediateTuples(0),
  InitialRunsCount(0),
  TotalTupleCount(0),
  TotalTupleSize(0),
  MinTupleSize(INT_MAX),
  MaxTupleSize(0),
  AvgTupleSize(0),
  TotalComparisons(0),
  TimeRunPhase(""),
  TimeMergePhase(""),
  TimeTotal(""),
  RunStatistics(true)
{
}

extrel2::SortInfo::~SortInfo()
{
  clearAll();
}

extrel2::SortedRunInfo* extrel2::SortInfo::CurrentRun()
{
  return this->InitialRunInfo[this->InitialRunsCount-1];
}

void extrel2::SortInfo::NewRun()
{
  this->InitialRunsCount++;
  this->InitialRunInfo.push_back(new SortedRunInfo(this->InitialRunsCount));
}

void extrel2::SortInfo::clearAll()
{
  BufferSize = 0;
  IOBufferSize = 0;
  MaxMergeFanIn = 0;
  F0 = 0;
  W = 0;
  IntermediateTuples = 0;
  InitialRunsCount = 0;
  TotalTupleCount = 0;
  TotalTupleSize = 0;
  MinTupleSize = INT_MAX;
  MaxTupleSize = 0;
  AvgTupleSize = 0;
  TotalComparisons = 0;
  TimeRunPhase = "";
  TimeMergePhase = "";
  TimeTotal = "";

  vector<SortedRunInfo*>::iterator iter;

  for( iter = InitialRunInfo.begin(); iter != InitialRunInfo.end(); iter++ )
  {
    delete *iter;
  }
  InitialRunInfo.clear();

  for( iter = FinalRunInfo.begin(); iter != FinalRunInfo.end(); iter++ )
  {
    delete *iter;
  }
  FinalRunInfo.clear();
}

void extrel2::SortInfo::UpdateStatistics(size_t s)
{
  int n = (int)s;
  this->TotalTupleCount++;
  this->TotalTupleSize += n;
  this->MinTupleSize = n < this->MinTupleSize ? n : this->MinTupleSize;
  this->MaxTupleSize = n > this->MaxTupleSize ? n : this->MaxTupleSize;
}

void extrel2::SortInfo::RunBuildPhase()
{
  this->StopWatchTotal.start();
}

void extrel2::SortInfo::MergePhase()
{
  this->AvgTupleSize += (float)this->TotalTupleSize /
                        (float)this->TotalTupleCount;
  this->TimeRunPhase = StopWatchTotal.diffTimes();
  this->StopWatchMerge.start();
}

void extrel2::SortInfo::Finished()
{
  this->TimeMergePhase = StopWatchMerge.diffTimes();
  this->TimeTotal = StopWatchTotal.diffTimes();
}

ostream& extrel2::operator<<(ostream& stream, SortInfo& info)
{
  float totalTupleSizeMB = (float)info.TotalTupleSize /
                           (float)(1024.0 * 1024.0);

  stream << endl
         << HLINE << endl
         << "Sort-Operation Statistics" << endl
         << HLINE << endl
         << "BufferSize: \t\t" << info.BufferSize << " ( "
         << info.BufferSize/1024 << " KByte)" << endl
         << "I/O-BufferSize: \t" << info.IOBufferSize << " ( "
         << info.IOBufferSize/1024 << " KByte)" << endl
         << "MaxMergeFanIn: \t\t" << info.MaxMergeFanIn << endl
         << "FirstMergeFanIn: \t" << info.F0 << endl
         << "Intermediate Merges: \t" << info.W+1 << endl
         << "Intermediate Tuples: \t" << info.IntermediateTuples << endl
         << "InitialRunsCount: \t" << info.InitialRunsCount << endl
         << "TotalTupleCount: \t" << info.TotalTupleCount << endl
         << "TotalSize: \t\t" << info.TotalTupleSize << " Byte ( "
         << totalTupleSizeMB << " MByte)" << endl
         << "MinTupleSize: \t\t" << info.MinTupleSize << " Byte" << endl
         << "MaxTupleSize: \t\t" << info.MaxTupleSize << " Byte" << endl
         << "AvgTupleSize: \t\t" << info.AvgTupleSize << " Byte" << endl
         << "TotalComparisons: \t" << info.TotalComparisons << endl
         << "TimeRunPhase: \t\t" << info.TimeRunPhase << endl
         << "TimeMergePhase: \t" << info.TimeMergePhase << endl
         << "TimeTotal: \t\t" << info.TimeTotal << endl;

  if ( info.RunStatistics == true )
  {

  stream << HLINE << endl
         << "Initial run statistics (1-" << info.InitialRunsCount << ")" << endl
         << HLINE << endl;

          for (size_t i = 0; i < info.InitialRunInfo.size(); i++)
          {
            stream << *(info.InitialRunInfo[i]);
          }
  }

  if ( info.RunStatistics == true && !info.FinalRunInfo.empty() )
  {

  stream  << HLINE << endl
          << "Final run statistics" << endl
          << HLINE << endl;

           for (size_t i = 0; i < info.FinalRunInfo.size(); i++)
           {
             stream << *(info.FinalRunInfo[i]);
           }
  }

  stream << endl;

  return stream;
}
