/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
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

*/

#include "Progress.h"


ProgressInfo::ProgressInfo()
{
  BTime = 0.001;	//default assumes non-blocking operator,
  BProgress = 1.0;	//time value must be > 0
}


void ProgressInfo::CopySizes(ProgressInfo p)	//copy the size fields
  {
    Size = p.Size;
    SizeExt = p.SizeExt;
    noAttrs = p.noAttrs;
    attrSize = p.attrSize;
    attrSizeExt = p.attrSizeExt;
  }


void ProgressInfo::CopySizes(ProgressLocalInfo* pli) //copy the size fields
{
  Size = pli->Size;
  SizeExt = pli->SizeExt;
  noAttrs = pli->noAttrs;
  attrSize = pli->attrSize;
  attrSizeExt = pli->attrSizeExt;
}



void ProgressInfo::CopyBlocking(ProgressInfo p)
				//copy BTime, BProgress 
				//for non blocking unary op.
{
  BTime = p.BTime;
  BProgress = p.BProgress;
}

void ProgressInfo::CopyBlocking(ProgressInfo p1, ProgressInfo p2) 
				//copy BTime, BProgress
				//for non-blocking binary op. (join)
{
  BTime = p1.BTime + p2.BTime;
  BProgress = 
    (p1.BProgress * p1.BTime + p2.BProgress * p2.BTime) / BTime;
}


void ProgressInfo::Copy(ProgressInfo p)		//copy all fields
  {
    Card = p.Card;
    CopySizes(p);
    Time = p.Time;
    Progress = p.Progress;
    BTime = p.BTime;
    BProgress = p.BProgress;
  }




ProgressLocalInfo::ProgressLocalInfo()  {
  read = 0;
  readFirst = 0;
  readSecond = 0;
  returned = 0;
  progressInitialized = false;
}

ProgressLocalInfo::~ProgressLocalInfo() 
{
  if ( progressInitialized )
  {
    delete attrSize;
    delete attrSizeExt;

	//cout << "attrSize and attrSizeExt deleted" << endl;
  }
}

void ProgressLocalInfo::SetJoinSizes( ProgressInfo& p1, ProgressInfo& p2 ) 
{
  if ( !progressInitialized )
  {
      
    Size = p1.Size + p2.Size;
    SizeExt = p1.SizeExt + p2.SizeExt;

    noAttrs = p1.noAttrs + p2.noAttrs;
    attrSize = new double[noAttrs];
    attrSizeExt = new double[noAttrs];
    for (int i = 0; i < p1.noAttrs; i++)
    {
      attrSize[i] = p1.attrSize[i];
      attrSizeExt[i] = p1.attrSizeExt[i];
    }
    for (int j = 0; j < p2.noAttrs; j++)
    {
       attrSize[p1.noAttrs + j] = p2.attrSize[j];
       attrSizeExt[p1.noAttrs + j] = p2.attrSizeExt[j];
    }
    progressInitialized = true;
  }
}




















long
Progress::getCtr() { return ctr; }

void
Progress::setCtr(long value) { ctr = value; }



long
Progress::getCtrA() { return ctrA; }
void
Progress::setCtrA(long value) { ctrA = value; }
void
Progress::incCtrA() { ctrA++; }

long
Progress::getCtrB() { return ctrB; }
void
Progress::setCtrB(long value) { ctrB = value; }
void
Progress::incCtrB() { ctrB++; }

long
Progress::getRtrn() { return rtrn; }
void
Progress::setRtrn(long value) { rtrn = value; }
void
Progress::incRtrn() { rtrn++; }

bool
Progress::getPrInit() { return prInit; }
void
Progress::setPrInit(bool value) { prInit = value; }

long
Progress::getNoAtt() { return noAtt; }
void
Progress::setNoAtt(long value) { noAtt = value; }

double*
Progress::getAttSize() { return attSize; }
double*
Progress::initAttSize(int value) { return (attSize = new double[value]); }

double*
Progress::getAttSExt() { return attSExt; }
double*
Progress::initAttSExt(int value) { return (attSExt = new double[value]); }

void*
Progress::getPtrA() { return PtrA; }
void
Progress::setPtrA(void* value) { PtrA = value; }

void*
Progress::getPtrB() { return PtrB; }
void
Progress::setPtrB(void* value) { PtrB =  value; }

