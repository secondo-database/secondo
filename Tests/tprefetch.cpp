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

*/
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "SecondoSMI.h"
#include "SmiCodes.h"

/*

IMPORTANT NOTE:

Before using this test program you should set the 
default buffer size in ~PrefetchingIteratorImpl~
to 1024, otherwise the code related to handling
very large tuples will not get tested properly.

*/

using namespace std;

const size_t maxDataSize = 8192;

static bool error;

#define tassert(expr) if(!(expr)){ error = true; cout << "*** Assertion " \
  << __STRING(expr) << " at line " << __LINE__ << " in file " << __FILE__ \
  << " failed. ***" << endl;};

void EnterTestFun()
{
  error = false;
}

void ExitTestFun(const char* name)
{
  if(error)
  {
    cout << "*** Test function " << name << " failed. ***" << endl;
  }
  else
  {
    cout << "    Test function " << name << " succeeded." << endl;
  }
}

typedef multimap<string, string> TestData;

typedef multimap<double, string> DoubleTestData;

typedef multimap<long, string> LongTestData;

void FillString(char* c, string& filled, size_t s)
{
  filled.clear();
  for(size_t i = 1; i <= s; i++)
  {
    filled.append(c);
  }
}

void GenerateTestData(TestData& testData)
{
  testData.clear();
  
  string key;
  string val;
  
  const size_t longKeyLength = 2048;
  const size_t longDataLength = 4096;
  
  FillString("a", key, longKeyLength);
  testData.insert(pair<string, string>(key, "i"));
  
  FillString("b", key, longKeyLength);
  testData.insert(pair<string, string>(key, "j"));
  testData.insert(pair<string, string>(key, "q"));
  testData.insert(pair<string, string>(key, "r"));
  testData.insert(pair<string, string>(key, "s"));
  testData.insert(pair<string, string>(key, "t"));

  FillString("k", val, longDataLength);
  testData.insert(pair<string, string>("c", val));

  FillString("l", val, longDataLength);
  testData.insert(pair<string, string>("d", val));
  FillString("q", val, longDataLength);
  testData.insert(pair<string, string>("d", val));
  FillString("r", val, longDataLength);
  testData.insert(pair<string, string>("d", val));
  FillString("s", val, longDataLength);
  testData.insert(pair<string, string>("d", val));
  FillString("t", val, longDataLength);
  testData.insert(pair<string, string>("d", val));

  testData.insert(pair<string, string>("e", "m"));

  testData.insert(pair<string, string>("f", "n"));
  testData.insert(pair<string, string>("f", "q"));
  testData.insert(pair<string, string>("f", "r"));
  testData.insert(pair<string, string>("f", "s"));
  testData.insert(pair<string, string>("f", "t"));
  
  FillString("g", key, longKeyLength);
  FillString("o", val, longDataLength);
  testData.insert(pair<string, string>(key, val));
  
  FillString("h", key, longKeyLength);
  FillString("p", val, longDataLength);
  testData.insert(pair<string, string>(key, val));
  FillString("q", val, longDataLength);
  testData.insert(pair<string, string>(key, val));
  FillString("r", val, longDataLength);
  testData.insert(pair<string, string>(key, val));
  FillString("s", val, longDataLength);
  testData.insert(pair<string, string>(key, val));
  FillString("t", val, longDataLength);
  testData.insert(pair<string, string>(key, val));
}

void GenerateDoubleTestData(DoubleTestData& testData)
{
  testData.clear();
  
  string key;
  string val;
  
  const size_t longDataLength = 4096;
  
  testData.insert(pair<double, string>(1.0, "i"));
  
  testData.insert(pair<double, string>(2.0, "j"));
  testData.insert(pair<double, string>(2.0, "q"));
  testData.insert(pair<double, string>(2.0, "r"));
  testData.insert(pair<double, string>(2.0, "s"));
  testData.insert(pair<double, string>(2.0, "t"));

  FillString("k", val, longDataLength);
  testData.insert(pair<double, string>(3.0, val));

  FillString("l", val, longDataLength);
  testData.insert(pair<double, string>(4.0, val));
  FillString("q", val, longDataLength);
  testData.insert(pair<double, string>(4.0, val));
  FillString("r", val, longDataLength);
  testData.insert(pair<double, string>(4.0, val));
  FillString("s", val, longDataLength);
  testData.insert(pair<double, string>(4.0, val));
  FillString("t", val, longDataLength);
  testData.insert(pair<double, string>(4.0, val));

  testData.insert(pair<double, string>(5.0, "m"));

  testData.insert(pair<double, string>(6.0, "n"));
  testData.insert(pair<double, string>(6.0, "q"));
  testData.insert(pair<double, string>(6.0, "r"));
  testData.insert(pair<double, string>(6.0, "s"));
  testData.insert(pair<double, string>(6.0, "t"));
  
  FillString("o", val, longDataLength);
  testData.insert(pair<double, string>(7.0, val));
  
  FillString("p", val, longDataLength);
  testData.insert(pair<double, string>(8.0, val));
  FillString("q", val, longDataLength);
  testData.insert(pair<double, string>(8.0, val));
  FillString("r", val, longDataLength);
  testData.insert(pair<double, string>(8.0, val));
  FillString("s", val, longDataLength);
  testData.insert(pair<double, string>(8.0, val));
  FillString("t", val, longDataLength);
  testData.insert(pair<double, string>(8.0, val));
}

void GenerateLongTestData(LongTestData& testData)
{
  testData.clear();
  
  string key;
  string val;
  
  const size_t longDataLength = 4096;
  
  testData.insert(pair<long, string>(1, "i"));
  
  testData.insert(pair<long, string>(2, "j"));
  testData.insert(pair<long, string>(2, "q"));
  testData.insert(pair<long, string>(2, "r"));
  testData.insert(pair<long, string>(2, "s"));
  testData.insert(pair<long, string>(2, "t"));

  FillString("k", val, longDataLength);
  testData.insert(pair<long, string>(3, val));

  FillString("l", val, longDataLength);
  testData.insert(pair<long, string>(4, val));
  FillString("q", val, longDataLength);
  testData.insert(pair<long, string>(4, val));
  FillString("r", val, longDataLength);
  testData.insert(pair<long, string>(4, val));
  FillString("s", val, longDataLength);
  testData.insert(pair<long, string>(4, val));
  FillString("t", val, longDataLength);
  testData.insert(pair<long, string>(4, val));

  testData.insert(pair<long, string>(5, "m"));

  testData.insert(pair<long, string>(6, "n"));
  testData.insert(pair<long, string>(6, "q"));
  testData.insert(pair<long, string>(6, "r"));
  testData.insert(pair<long, string>(6, "s"));
  testData.insert(pair<long, string>(6, "t"));
  
  FillString("o", val, longDataLength);
  testData.insert(pair<long, string>(7, val));
  
  FillString("p", val, longDataLength);
  testData.insert(pair<long, string>(8, val));
  FillString("q", val, longDataLength);
  testData.insert(pair<long, string>(8, val));
  FillString("r", val, longDataLength);
  testData.insert(pair<long, string>(8, val));
  FillString("s", val, longDataLength);
  testData.insert(pair<long, string>(8, val));
  FillString("t", val, longDataLength);
  testData.insert(pair<long, string>(8, val));
}

void AssertIteratorsEqualRecno
  (PrefetchingIterator* piter, 
  TestData::iterator iter,
  TestData::iterator end,
  const set<SmiRecordId>& ids)
{
  char buffer[maxDataSize];
  SmiSize nBytesRead;

  SmiKey smiKey;
  SmiRecordId secondKeyRepresentation;
  
  SmiRecordId key;
  string pval;
  bool prc;
  
  prc = piter->Next();
  tassert((iter != end) == prc);
  if(!prc)
  {
    return;
  }
  
  while(true)
  {
    nBytesRead = piter->ReadCurrentData(buffer, maxDataSize);
    pval.assign(buffer, nBytesRead); 
    tassert(pval == iter->second);
    
    piter->ReadCurrentRecordNumber(key);
    piter->CurrentKey(smiKey);
    smiKey.GetKey(secondKeyRepresentation);
    tassert(key == secondKeyRepresentation);
    tassert(ids.find(key) != ids.end());
        
    prc = piter->Next();
    ++iter;
    
    tassert((iter != end) == prc);
    if(prc == false)
    {
      return;
    }
  }  
}

void AssertIteratorsEqualString
  (PrefetchingIterator* piter, 
  TestData::iterator iter,
  TestData::iterator end)
{
  char buffer[maxDataSize];
  SmiSize nBytesRead;

  SmiKey key;
  string keyString;
  
  string pval;
  bool prc;

  prc = piter->Next();
  tassert((iter != end) == prc);
  if(!prc)
  {
    return;
  }
  
  while(true)
  {
    nBytesRead = piter->ReadCurrentData(buffer, maxDataSize);
    pval.assign(buffer, nBytesRead); 
    tassert(pval == iter->second);
    
    piter->CurrentKey(key);
    tassert(key.GetType() == SmiKey::String);
    key.GetKey(keyString);
    tassert(keyString == iter->first);
        
    prc = piter->Next();
    ++iter;
    
    tassert((iter != end) == prc);
    if(prc == false)
    {
      return;
    }
  }  
}

void AssertIteratorsEqualDouble
  (PrefetchingIterator* piter, 
  DoubleTestData::iterator iter,
  DoubleTestData::iterator end)
{
  char buffer[maxDataSize];
  SmiSize nBytesRead;

  SmiKey key;
  double keyDouble;
  
  string pval;
  bool prc;
  
  prc = piter->Next();
  tassert((iter != end) == prc);
  if(!prc)
  {
    return;
  }
  
  while(true)
  {
    nBytesRead = piter->ReadCurrentData(buffer, maxDataSize);
    pval.assign(buffer, nBytesRead); 
    tassert(pval == iter->second);
    
    piter->CurrentKey(key);
    tassert(key.GetType() == SmiKey::Float);
    key.GetKey(keyDouble);
    tassert(keyDouble == iter->first);
    
    prc = piter->Next();
    ++iter;
    
    tassert((iter != end) == prc);
    if(prc == false)
    {
      return;
    }
  }  
}

void AssertIteratorsEqualLong
  (PrefetchingIterator* piter, 
  LongTestData::iterator iter,
  LongTestData::iterator end)
{
  char buffer[maxDataSize];
  SmiSize nBytesRead;

  SmiKey key;
  long keyLong;
  
  string pval;
  bool prc;
  
  prc = piter->Next();
  tassert((iter != end) == prc);
  if(!prc)
  {
    return;
  }
  
  while(true)
  {
    nBytesRead = piter->ReadCurrentData(buffer, maxDataSize);
    pval.assign(buffer, nBytesRead); 
    tassert(pval == iter->second);
    
    piter->CurrentKey(key);
    tassert(key.GetType() == SmiKey::Integer);
    key.GetKey(keyLong);
    tassert(keyLong == iter->first);
    
    prc = piter->Next();
    ++iter;
    
    tassert((iter != end) == prc);

    if(prc == false)
    {
      return;
    }
  }  
}

void TestRecnoIterator()
{
  EnterTestFun();

  SmiRecordFile* file = new SmiRecordFile(false);
  tassert(file != 0);
  tassert(file->Create());
  
  PrefetchingIterator* piter;
  SmiRecordId id;
  string str;  
  set<SmiRecordId> createdIds;
  
  TestData testData;
  TestData::iterator iter;
  
  GenerateTestData(testData);
  
  for(iter = testData.begin(); iter != testData.end(); ++iter)
  {
    SmiRecord record;
    string s = iter->second;
    
    tassert(file->AppendRecord(id, record));
    tassert(record.Write(s.c_str(), s.size()) == s.size());
    createdIds.insert(id);
  }
  
  tassert(createdIds.size() == testData.size());
  
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  AssertIteratorsEqualRecno(piter, testData.begin(), testData.end(), createdIds);
  delete piter;  
  
  tassert(file->DeleteRecord(*(createdIds.begin())));
  createdIds.erase(createdIds.begin());
  testData.erase(testData.begin());
  
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  AssertIteratorsEqualRecno(piter, testData.begin(), testData.end(), createdIds);
  delete piter;  
  
  tassert(file->Close());
  tassert(file->Drop());
  delete file;
  file = 0;

  /* Test wether iterator also works correctly for empty files */
  file = new SmiRecordFile(false);
  tassert(file != 0);
  tassert(file->Create());
  
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
  tassert(file->Close());
  tassert(file->Drop());
  delete file;

  ExitTestFun(__PRETTY_FUNCTION__);
}

void TestBtreeIteratorString()
{
  SmiKey leftBoundary("c");
  SmiKey leftBoundaryE("e");
  SmiKey rightBoundary("f");
  SmiKey farLeftBoundary("_");
  SmiKey farRightBoundary("zzz");
  
  TestData::iterator beginIter;
  TestData::iterator endIter;

  EnterTestFun();

  SmiKeyedFile* file = new SmiKeyedFile(SmiKey::String, false);
  tassert(file != 0);
  tassert(file->Create());
  
  PrefetchingIterator* piter;
  string str;  
  
  TestData testData;
  TestData::iterator iter;
  
  GenerateTestData(testData);
  
  for(iter = testData.begin(); iter != testData.end(); ++iter)
  {
    SmiKey key(iter->first);
    SmiRecord record;
    string s = iter->second;
    
    tassert(file->InsertRecord(key, record));
    tassert(record.Write(s.c_str(), s.size()) == s.size());
  }
    
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, testData.begin(), testData.end());
  delete piter;  
    
  endIter = testData.lower_bound("ff");
  piter = file->SelectLeftRangePrefetched(rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, testData.begin(), endIter);
  delete piter;  

  beginIter = testData.find("c");
  piter = file->SelectRightRangePrefetched(leftBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, beginIter, testData.end());
  delete piter;  

  beginIter = testData.find("e");
  piter = file->SelectRightRangePrefetched(leftBoundaryE);
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, beginIter, testData.end());
  delete piter;  

  beginIter = testData.find("c");
  endIter = testData.lower_bound("ff");
  piter = file->SelectRangePrefetched(leftBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, beginIter, endIter);
  delete piter;  

  piter = file->SelectRangePrefetched(farLeftBoundary, farRightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, testData.begin(), testData.end());
  delete piter;  

  beginIter = testData.find("c");
  piter = file->SelectRangePrefetched(leftBoundary, farRightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, beginIter, testData.end());
  delete piter;  

  endIter = testData.lower_bound("ff");
  piter = file->SelectRangePrefetched(farLeftBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualString(piter, testData.begin(), endIter);
  delete piter;  

  piter = file->SelectRangePrefetched(SmiKey("__"), SmiKey("_____"));
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
  
  piter = file->SelectRangePrefetched(SmiKey("yyy"), SmiKey("zzz"));
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
    
  tassert(file->Close());
  tassert(file->Drop());
  delete file;
  file = 0;

  /* Test wether iterator also works correctly for empty files */
  file = new SmiKeyedFile(SmiKey::String, false);
  tassert(file != 0);
  tassert(file->Create());
  
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
  tassert(file->Close());
  tassert(file->Drop());
  delete file;

  ExitTestFun(__PRETTY_FUNCTION__);
}

void TestBtreeIteratorDouble()
{
  SmiKey leftBoundary(3.0);
  SmiKey rightBoundary(6.0);
  SmiKey farLeftBoundary(-100.0);
  SmiKey farRightBoundary(200.0);
  DoubleTestData::iterator beginIter;
  DoubleTestData::iterator endIter;

  EnterTestFun();

  SmiKeyedFile* file = new SmiKeyedFile(SmiKey::Float, false);
  tassert(file != 0);
  tassert(file->Create());
  
  PrefetchingIterator* piter;
  string str;  
  
  DoubleTestData testData;
  DoubleTestData::iterator iter;
  
  GenerateDoubleTestData(testData);
  
  for(iter = testData.begin(); iter != testData.end(); ++iter)
  {
    SmiKey key(iter->first);
    SmiRecord record;
    string s = iter->second;
    
    tassert(file->InsertRecord(key, record));
    tassert(record.Write(s.c_str(), s.size()) == s.size());
  }
    
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, testData.begin(), testData.end());
  delete piter;  

  beginIter = testData.lower_bound(2.9);
  piter = file->SelectRightRangePrefetched(leftBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, beginIter, testData.end());
  delete piter;  

  beginIter = testData.lower_bound(2.9);
  piter = file->SelectRangePrefetched(leftBoundary, farRightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, beginIter, testData.end());
  delete piter;  

  beginIter = testData.lower_bound(2.9);
  endIter = testData.lower_bound(6.5);
  piter = file->SelectRangePrefetched(leftBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, beginIter, endIter);
  delete piter;  

  beginIter = testData.upper_bound(5.5);
  endIter = testData.upper_bound(6.5);
  piter = file->SelectRangePrefetched(rightBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, beginIter, endIter);
  delete piter;  
    
  endIter = testData.lower_bound(6.5);
  piter = file->SelectLeftRangePrefetched(rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, testData.begin(), endIter);
  delete piter;  

  endIter = testData.lower_bound(6.5);
  piter = file->SelectRangePrefetched(farLeftBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, testData.begin(), endIter);
  delete piter;  

  piter = file->SelectRangePrefetched(farLeftBoundary, farRightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualDouble(piter, testData.begin(), testData.end());
  delete piter;  

  piter = file->SelectRangePrefetched(SmiKey(-100.0), SmiKey(-50.0));
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
  
  piter = file->SelectRangePrefetched(SmiKey(100.0), SmiKey(150.0));
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
    
  tassert(file->Close());
  tassert(file->Drop());
  delete file;
  file = 0;

  /* Test wether iterator also works correctly for empty files */
  file = new SmiKeyedFile(SmiKey::Float, false);
  tassert(file != 0);
  tassert(file->Create());
  
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
  tassert(file->Close());
  tassert(file->Drop());
  delete file;

  ExitTestFun(__PRETTY_FUNCTION__);
}

void TestBtreeIteratorLong()
{
  SmiKey leftBoundary((long int)3);
  SmiKey rightBoundary((long int)6);
  SmiKey farLeftBoundary((long int)-100);
  SmiKey farRightBoundary((long int)200);
  LongTestData::iterator beginIter;
  LongTestData::iterator endIter;

  EnterTestFun();

  SmiKeyedFile* file = new SmiKeyedFile(SmiKey::Integer, false);
  tassert(file != 0);
  tassert(file->Create());
  
  PrefetchingIterator* piter;
  string str;  
  
  LongTestData testData;
  LongTestData::iterator iter;
  
  GenerateLongTestData(testData);
  
  for(iter = testData.begin(); iter != testData.end(); ++iter)
  {
    SmiKey key(iter->first);
    SmiRecord record;
    string s = iter->second;
    
    tassert(file->InsertRecord(key, record));
    tassert(record.Write(s.c_str(), s.size()) == s.size());
  }
    
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, testData.begin(), testData.end());
  delete piter;  

  beginIter = testData.lower_bound(3);
  piter = file->SelectRightRangePrefetched(leftBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, beginIter, testData.end());
  delete piter;  

  beginIter = testData.lower_bound(3);
  piter = file->SelectRangePrefetched(leftBoundary, farRightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, beginIter, testData.end());
  delete piter;  

  beginIter = testData.lower_bound(3);
  endIter = testData.lower_bound(7);
  piter = file->SelectRangePrefetched(leftBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, beginIter, endIter);
  delete piter;  

  beginIter = testData.upper_bound(5);
  endIter = testData.upper_bound(6);
  piter = file->SelectRangePrefetched(rightBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, beginIter, endIter);
  delete piter;  
    
  endIter = testData.lower_bound(7);
  piter = file->SelectLeftRangePrefetched(rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, testData.begin(), endIter);
  delete piter;  

  endIter = testData.lower_bound(7);
  piter = file->SelectRangePrefetched(farLeftBoundary, rightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, testData.begin(), endIter);
  delete piter;  

  piter = file->SelectRangePrefetched(farLeftBoundary, farRightBoundary);
  tassert(piter != 0);
  AssertIteratorsEqualLong(piter, testData.begin(), testData.end());
  delete piter;  

  piter = file->SelectRangePrefetched(SmiKey((long int)-100), SmiKey((long int)-50));
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
  
  piter = file->SelectRangePrefetched(SmiKey((long int)100), SmiKey((long int)150));
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
    
  tassert(file->Close());
  tassert(file->Drop());
  delete file;
  file = 0;

  /* Test wether iterator also works correctly for empty files */
  file = new SmiKeyedFile(SmiKey::Integer, false);
  tassert(file != 0);
  tassert(file->Create());
  
  piter = file->SelectAllPrefetched();
  tassert(piter != 0);
  tassert(!piter->Next());
  delete piter;  
  tassert(file->Close());
  tassert(file->Drop());
  delete file;

  ExitTestFun(__PRETTY_FUNCTION__);
}

int main( int argc, char* argv[] )
{
  SmiError rc;
  bool ok;

  rc = SmiEnvironment::StartUp( SmiEnvironment::SingleUser,
                                "SecondoConfig.ini", cerr );
  if ( rc == 1 )
  {
    ok = SmiEnvironment::OpenDatabase( "testprefetch" );
    if(ok)
    {
      cout << "OpenDatabase testprefetch ok." << endl;
    }
    else
    {
      cout << "OpenDatabase testprefetch failed, try to create." << endl;
      ok = SmiEnvironment::CreateDatabase( "testprefetch" );
      if ( ok )
        cout << "CreateDatabase testprefetch ok." << endl;
      else
        cout << "CreateDatabase testprefetch failed." << endl;
    }
    if ( ok )
    {
      tassert(SmiEnvironment::BeginTransaction());

      /* the test code proper */
      TestRecnoIterator();     
      TestBtreeIteratorString();
      TestBtreeIteratorDouble();
      TestBtreeIteratorLong();
      /* End of test code proper */

      tassert(SmiEnvironment::CommitTransaction());

      if ( SmiEnvironment::CloseDatabase() )
        cout << "CloseDatabase testprefetch ok." << endl;
      else
        cout << "CloseDatabase testprefetch failed." << endl;

      if ( SmiEnvironment::EraseDatabase( "testprefetch" ) )
        cout << "EraseDatabase testprefetch ok." << endl;
      else
        cout << "EraseDatabase testprefetch failed." << endl;
    }
  }
  else
  {
    cout << "Startup failed." << endl;
  }
  cout << "Shutdown ... " << (rc = SmiEnvironment::ShutDown()) << endl;
}

