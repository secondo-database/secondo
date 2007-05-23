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

