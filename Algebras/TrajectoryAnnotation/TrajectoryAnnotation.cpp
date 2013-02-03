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


[1] Implementation of the TrajectoryAnnotation Algebra

November, 2012. Katharina Rieder


1 Overview

This implementation file essentially contains the implementation of the
operation geocode.

2 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "SpatialAlgebra.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "StringUtils.h"
#include "../FText/FTextAlgebra.h"
#include "../OSM/ShpFileReader.h"
#include "../OSM/OsmImportOperator.h"
#include <iostream> 
#include <fstream> 
#include <stdexcept> 
#include <sstream> 
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netdb.h>  
#include <errno.h> 
#include <math.h>
#include <cmath>
#include "TypeMapUtils.h"
#include "../OSM/XmlFileReader.h"
#include "../OSM/XmlParserInterface.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace mappings;

//3 methodes for creation of the socket
std::runtime_error CreateSocketError() 
{
	
std::ostringstream temp; 

    temp << "Socket-Fehler #" << errno << ": " << strerror(errno);


return std::runtime_error(temp.str()); 
}


void SendAll(int socket, const char* const buf, const int size) 
{ 
    int bytesSent = 0; // amount of Bytes already send from the Buffer
     do 
    { 
        int resulte = send(socket, buf + bytesSent, size - bytesSent, 0); 
        if(resulte < 0) // If value < 0 throw an error
         { 
            throw CreateSocketError(); 
        } 
        bytesSent += resulte; 
    } while(bytesSent < size); 
}

void GetLine(int socket, std::stringstream& line) 
{
	for(char c; recv(socket, &c, 1, 0) > 0; line << c) 
    { 
        if(c == '\n') 
        { 
            return; 
        } 
    }
	throw CreateSocketError(); 
}


void askGoogle(const string& street, const string& no, 
const string& postcode,const string& ci, Point& result) {
  //Check of spaces and special characters in URL
string newSt = stringutils::replaceAll(street, " ", "%20");
string newSt1 = stringutils::replaceAll(newSt, "ß", "ss");
string newSt2 = stringutils::replaceAll(newSt1, "ä", "ae");
string newSt3 = stringutils::replaceAll(newSt2, "ö", "oe");
string newSt4 = stringutils::replaceAll(newSt3, "ü", "ue");
string newSt5 = stringutils::replaceAll(newSt4, "Ä", "Ae");
string newSt6 = stringutils::replaceAll(newSt5, "Ö", "Oe");
string newSt7 = stringutils::replaceAll(newSt6, "Ü", "Ue");

string newc = stringutils::replaceAll(ci, " ", "%20");
string newc1 = stringutils::replaceAll(newc, "ß", "ss");
string newc2 = stringutils::replaceAll(newc1, "ä", "ae");
string newc3 = stringutils::replaceAll(newc2, "ö", "oe");
string newc4 = stringutils::replaceAll(newc3, "ü", "ue");
string newc5 = stringutils::replaceAll(newc4, "Ä", "Ae");
string newc6 = stringutils::replaceAll(newc5, "Ö", "Oe");
string newc7 = stringutils::replaceAll(newc6, "Ü", "Ue");

string no1 = stringutils::replaceAll(no, " ", "%20");
  
  
  //constant values of url	
  string pre="GET /maps/api/geocode/xml?address=";	
  string post="+CA&sensor=false HTTP/1.1\r\n;";
  post +="Host: maps.googleapis.com\r\nConnection: close\r\n\r\n";	
  //create url			
  string request("");
  request += pre+ newSt7+  "+";
  request += no1+ "+"   +postcode;
  request +=  "+"   +newc7+ post;


usleep(120000);  //wait because only 10 request per sec allowed
  
hostent* phe = gethostbyname("maps.googleapis.com"); 

    //if host not found
    if(phe == NULL) 
    { 
       result.SetDefined(false); 
       // return 0; 
    } 

    int Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    //error while craeating socket
    if(Socket == -1) 
    { 
        result.SetDefined(false); 
        //eturn 0; 
    } 

    sockaddr_in service; 
    service.sin_family = AF_INET; 
    service.sin_port = htons(80); // HTTP-Protocol use Port 80 

    char** p = phe->h_addr_list; 
    int resulte; 
    do 
    { 
        if(*p == NULL) 
        { 
            result.SetDefined(false); 
            //return 0; 
        } 

        service.sin_addr.s_addr = *reinterpret_cast<unsigned long*>(*p); 
        ++p; 
        resulte = 
	connect(Socket, reinterpret_cast<sockaddr*>(&service), sizeof(service));
     } 
    while(resulte == -1); 


	
	
    SendAll(Socket, request.c_str(), request.size());



    ofstream fout("../bin/output.xml", ios::trunc);  
	

 

  
if (fout.good()==true)
{	
int n = 0;
  
    while(!fout.eof()) 
    { 
        stringstream line; 
	
        try 
        { 
	    GetLine(Socket, line);
        } 
        catch(exception& e) // Ein Fehler oder Verbindungsabbruch 
        { 
            break; // Schleife verlassen 
        } 
        if (n>11)
	{

	fout << line.str() << endl; // Zeile in die Datei schreiben. 
        }
	else 
	{n++;}

  }
}
else
{	
	usleep(120000);	
	if (fout.good()==true)
	{	
		int n = 0;
  
   		 while(!fout.eof()) 
    		{ 
        		stringstream line; 
	
        		try 
        		{ 
	    		GetLine(Socket, line);
        		} 
        		catch(exception& e) 
        		{ 
            		break; // Schleife verlassen 
        		} 
        		if (n>11)
			{
				fout << line.str() << endl; 
				// Zeile in die Datei schreiben. 
        		}
			else 
			{n++;}

  		}
	}
	else
	{	
		result.SetDefined(false);   
         	//return 0; 
	}
}
//Close connection
close(Socket); 

//xml-Parser

xmlDocPtr doc;
xmlNodePtr cur;
xmlNodePtr check;
xmlNodePtr subcur;
xmlNodePtr subsubcur;
xmlNodePtr subsubsubcur;
doc = xmlParseFile("../bin/output.xml");
cur = xmlDocGetRootElement(doc);

//check if Parentnode is available

if ( cur == NULL){
result.SetDefined(false); 
//return 0;
}

cur = cur->children;
check = cur->children;
double ausg1;
double ausg2;

while (cur != NULL) {
   if ((xmlStrcmp(cur->name, (const xmlChar *)"result"))==0){
	subcur = cur->children;
     while (subcur != NULL) {
	if ((xmlStrcmp(subcur->name, (const xmlChar *)"geometry"))==0) {
	     subsubcur = subcur->children;
	while (subsubcur != NULL) {
	   if ((xmlStrcmp(subsubcur->name,(const xmlChar *)"location"))==0)
	     {
	     subsubsubcur = subsubcur->children;
		while (subsubsubcur != NULL) {
		   if 
		   ((xmlStrcmp(subsubsubcur->name,(const xmlChar *)"lat"))==0) {
		     xmlChar *val1;
		     val1 =xmlNodeListGetString(doc,subsubsubcur->children,1);
		     ausg1=OsmImportOperator::convStrToDbl((const char *)val1);
		     xmlFree(val1);
		   }
		   if 
		   ((xmlStrcmp(subsubsubcur->name,(const xmlChar *)"lng"))==0) {
		   xmlChar *val2;
		   val2 = xmlNodeListGetString(doc, subsubsubcur->children, 1);
		   ausg2= OsmImportOperator::convStrToDbl((const char *)val2);
		   xmlFree(val2);
		   }
				  
		subsubsubcur = subsubsubcur->next;
		}
	     }
	subsubcur = subsubcur->next;
 	}
      }
			  
   subcur = subcur->next;
  }
 }
cur = cur->next;
}
	
xmlFreeDoc(doc);


if (ausg2>0){
result.Set(ausg2, ausg1);
}
else{
 result.SetDefined(false); 
}


}

/*
4 Implementation of Algebra TrajectoryAnnotation in namespace tann

4.1 Implementation of operator geocode

Typemapping and Selection Funktion for geocode

*/

namespace tann {


ListExpr geocodeTM(ListExpr args){
int len = nl->ListLength(args);
string err = "(string x string x int x string) or "
"(string x int x int x string) or (string x int x string) expected";
if((len!=3) && (len !=4)){ // 3 oder 4 argumente
	return listutils::typeError(err);
}

if(len == 3){
	if( !CcString::checkType(nl->First(args)) || 
	!CcInt::checkType(nl->Second(args)) ||
	!CcString::checkType(nl->Third(args))){
	return listutils::typeError(err);
	} else {
	return listutils::basicSymbol<Point>();
	}
}

if(len == 4){
	if( !CcString::checkType(nl->First(args)) || 
	(!CcString::checkType(nl->Second(args)) &&
	!CcInt::checkType(nl->Second(args))) ||
	!CcInt::checkType(nl->Third(args)) ||
	!CcString::checkType(nl->Fourth(args))){
	 return listutils::typeError(err);
 	} else {
	return listutils::basicSymbol<Point>();
 	}
}

else {return listutils::typeError(err);}
}

int geocodeSelect(ListExpr args){
int len = nl->ListLength(args);
if(len == 3){ // string x int x string
	return 0; // index in umVM
	} 
if (len ==4) { // point x text x int
  if(CcString::checkType(nl->Second(args))) {
	return 1;
	}
  if (CcInt::checkType(nl->Second(args))){
	return 2;}
  else {
	return listutils::basicSymbol<Point>();
 	}

}
else {
	return listutils::basicSymbol<Point>();
 	}
}


// Map Value

//Liste aus drei Elementen

int
geocodeVM0 (Word* args, Word& result, int message,
             Word& local, Supplier s)
{

CcString* st = (CcString*)args[0].addr;
CcInt* plz = (CcInt*) args[1].addr;
CcString* c = (CcString*) args[2].addr;


result = qp->ResultStorage(s);
Point* res = (Point*) result.addr;
                                //query processor has provided
                                //a point instance for the result


if(st->IsDefined() && plz->IsDefined() && c->IsDefined()){

   string str=(string)(char*)(st)->GetStringval();
   int co = (plz)->GetIntval();
   string ci=(string)(char*)(c)->GetStringval();
   string postcode = stringutils::int2str(co);

   //search for the beginning of the housenumber
   size_t pos = str.find_last_of("0123456789");
    string street;
    string no;

    if(pos==string::npos){
       // Fehler: keine Ziffer enthalten
       res->SetDefined(false); 
       return 0;
    }  else {
      while((pos>0) && (str[pos]>='0') &&  (str[pos]<='9')){
          pos--;
      }
      }
    if( (pos>0) || ((str[pos]>='0') && (str[pos]<='0'))){
       street = str.substr(0,pos+1);
       no = str.substr(pos+1);
    } else {
      res->SetDefined(false); 
       return 0;
      }

askGoogle(street, no, postcode, ci, *res);
}

else {
 res->SetDefined(false);
 }
 return 0;
}

//Liste aus vier Elementen
int
geocodeVM1 (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
CcString* st = (CcString*)args[0].addr;
CcString* no= (CcString*) args[1].addr;
CcInt* plz = (CcInt*) args[2].addr;
CcString* c = (CcString*) args[3].addr;


result = qp->ResultStorage(s);
Point* res = (Point*) result.addr;


if(st->IsDefined() && no->IsDefined() && plz->IsDefined() && c->IsDefined()){

   string street=(string)(char*)(st)->GetStringval();
   string number=(string)(char*)(no)->GetStringval();
   int co = (plz)->GetIntval();
   string ci=(string)(char*)(c)->GetStringval();
   string postcode = stringutils::int2str(co);


askGoogle(street, number, postcode, ci, *res);
}

else {
 res->SetDefined(false);
 }
 return 0;
}


int
geocodeVM2 (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
CcString* st = (CcString*)args[0].addr;
CcInt* no= (CcInt*) args[1].addr;
CcInt* plz = (CcInt*) args[2].addr;
CcString* c = (CcString*) args[3].addr;


result = qp->ResultStorage(s);
Point* res = (Point*) result.addr;


if(st->IsDefined() && no->IsDefined() && plz->IsDefined() && c->IsDefined()){

   string street=(string)(char*)(st)->GetStringval();
   int number=(no)->GetIntval();
   int co = (plz)->GetIntval();
   string ci=(string)(char*)(c)->GetStringval();
   string postcode = stringutils::int2str(co);
   string nr = stringutils::int2str(number);

askGoogle(street, nr, postcode, ci, *res);
}

else {
 res->SetDefined(false);
 }
 return 0;
}




//5 Definition of operator geocode
ValueMapping geocodeVM[] = {
geocodeVM0,
geocodeVM1,
geocodeVM2
};

//Spezizikation
const string geocodeSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>(string x int x string) -> point or "
	"(string x [string|int] x int x string) -> point</text--->"
        "<text>geocode(_,_,_,_)</text--->"
        "<text>Point to an address</text--->"
        "<text>query geocode('Universitätsstr.', 11, 58097, 'Hagen')</text--->"
        ") )";




Operator geocodeOp(
 "geocode", // Name des Operators
geocodeSpec, // Spezifikation
 3, // Anzahl ValueMappings
geocodeVM, // Array von ValueMappings
geocodeSelect, // Selection Funktion
geocodeTM // TypeMapping
 );

/*
5 Implementation of the Algebra Class

*/

class TrajectoryAnnotationAlgebra : public Algebra
{
 public:
  TrajectoryAnnotationAlgebra() : Algebra()
  {

/*
5.1 Registration of Operators

*/

    AddOperator( &geocodeOp);

  }
  ~TrajectoryAnnotationAlgebra() {};
};


  
} //end of namespace tann

/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/
extern "C"
Algebra*
InitializeTrajectoryAnnotationAlgebra( NestedList* nlRef,
                               QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name
  return new tann::TrajectoryAnnotationAlgebra;
}
