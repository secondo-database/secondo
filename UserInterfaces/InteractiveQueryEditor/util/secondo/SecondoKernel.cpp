/*
---- 
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, Faculty of Mathematics and 
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

This file implements the methods which are defined in the SecondoKernel.h file.
These methods get called by a Java program via JNI.

*/
#include <jni.h>
#include "SecondoKernel.h"
#include "DisplayTTY.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "SecondoInterface.h"
#include "SecondoInterfaceTTY.h"
#include "NestedList.h"
#include "NList.h"
#include "JNITool.h"

using namespace std;

SecondoInterfaceTTY* si = 0;
JNITool* jnitool = 0;
DisplayTTY* displayTTY = 0;
NestedList* nestedList = 0;
string lasterror = "";
ofstream nirvana("/dev/null");
streambuf* stdo_buffer = cout.rdbuf();
streambuf* stde_buffer = cerr.rdbuf();
streambuf* stdl_buffer = clog.rdbuf();

/*
	Method used to intialize the secondo interface and the global variables
	of this file.
	
*/
JNIEXPORT jboolean JNICALL Java_util_secondo_SecondoKernel_initialize
(JNIEnv *env, jclass clazz, jstring str)
{
	si = new SecondoInterfaceTTY(false);
	string config = env->GetStringUTFChars(str, 0);
	//si->InitRTFlags(config);

	string user = "";
	string passwd = "";
	string host = "";
	string port = "";
	string errMsg = "";
	bool multiUser = false;
	
	if (!si->Initialize(user, passwd, host, port, config, "",errMsg, 
		multiUser))
	{
		// connection failed, handle error
		cerr << "Cannot initialize secondo system" << endl;
		cerr << "Error message = " << errMsg << endl;
		return  false;
	}
	nestedList = si->GetNestedList();
	NList::setNLRef(nestedList);
	jnitool = new JNITool(env, nestedList);
	displayTTY = &DisplayTTY::GetInstance();
	DisplayTTY::Set_NL( nestedList );
	DisplayTTY::Initialize();
	return true;
}

/*
Method used to execute a given statement.
The result is prcessed by the class DisplayTTY

*/
JNIEXPORT jobject JNICALL Java_util_secondo_SecondoKernel_query
(JNIEnv *env, jclass clazz, jstring str, jboolean show)
{
	string command = env->GetStringUTFChars(str, 0);
	if ((command != "quit") && (command.length()>0))
	{
		ListExpr res = nestedList->TheEmptyList();
		SecErrInfo err;
		if ((bool)show) {
		  	cout.rdbuf (stdo_buffer);
		  	cerr.rdbuf (stde_buffer);
		  	clog.rdbuf (stdl_buffer);
		} else {
		 	cout.rdbuf(nirvana.rdbuf());
		 	cerr.rdbuf(nirvana.rdbuf());
		 	clog.rdbuf(nirvana.rdbuf());		
		}
		
		si->Secondo(command, res, err);
		NList::setNLRef(nestedList);

		if (err.code != 0)
		{
			cout << "Error during command. Error code :"
				<< err.code << endl;
			cout << "Error message = " << err.msg << endl;
			lasterror = err.msg;
		}
		else
		{
			jobject obj = NULL;
			try
			{
				obj = jnitool->GetJavaList(env, res);
                if ( nestedList->IsEmpty( res ) ||
                	(nestedList->ListLength( res ) != 2) ||
                	 ((nestedList->ListLength( res ) == 2) && 
                	 (nestedList->IsEmpty(nestedList->Second( res ))))
                   )
  				{
    				cout << "=> []" << endl;
  				}
  				else
  				{
    				displayTTY->DisplayResult(nestedList->
				First( res ),
    				nestedList->Second( res ));
  				}
			}
			catch (...)
			{
				cout << "Error during c++ -> Java conversion"
				<< endl;
				return NULL;
			};
			return obj;
		}
	}
	return NULL;
}

/*
Changes the secondo debug level

*/
JNIEXPORT void JNICALL Java_util_secondo_SecondoKernel_setDebugLevel
(JNIEnv *env, jclass clazz, jint debugLevel)
{
    si->SetDebugLevel( (int)debugLevel );
    cout << "*** Debug level set to " << debugLevel << "." << endl;
}

/*
Retrieves the last occurred error message. 

*/
JNIEXPORT jstring JNICALL Java_util_secondo_SecondoKernel_errorMessage
(JNIEnv *env, jclass clazz)
{
	jstring errorMessage = env->NewStringUTF(lasterror.c_str());
	return errorMessage;
}

/*
Used to close the interface connection

*/
JNIEXPORT void JNICALL Java_util_secondo_SecondoKernel_shutdown
(JNIEnv *env, jclass clazz)
{
	si->Terminate();
	delete si;
}
