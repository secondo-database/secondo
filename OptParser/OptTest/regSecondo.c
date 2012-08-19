/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

This file is a Java Native Interface file for interfacing Secondo and OptTest.

*/

#include <jni.h>
#include <SecondoPL.h>

JNIEXPORT jint JNICALL Java_OptTest_registerSecondo(
                  JNIEnv * env , 
                  jclass cl){
   int res = registerSecondo();
   return res;
   
}

