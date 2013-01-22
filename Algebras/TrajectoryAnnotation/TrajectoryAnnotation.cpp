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

//methodes for replacing special characters in the url
string replaceinString(std::string str, std::string tofind, std::string toreplace)
{
        size_t position = 0;
        for ( position = str.find(tofind); position != std::string::npos; position = str.find(tofind,position) )
        {
                str.replace(position ,1, toreplace);
        }
        return(str);
} 

//int to string
std::string itoa(int n){
	std::stringstream stream;
	stream <<n;
	return stream.str();
}




//4 Implementation of Algebra TrajectoryAnnotation in namespace tann

namespace tann {

//4.1 Implementation of operator geocode

//Typemapping and Selection Funktion

const string maps_geocode[2][5] =
{
{CcString::BasicType(), CcString::BasicType(),CcInt::BasicType(),CcString::BasicType(), Point::BasicType()},
{CcString::BasicType(), CcInt::BasicType(),CcInt::BasicType(),CcString::BasicType(), Point::BasicType()}
};

ListExpr
GeoCodeTypeMap( ListExpr args )
{
return SimpleMaps<2,5>(maps_geocode, args);
}

int
GeoCodeSelect( ListExpr args )
{
return SimpleSelect<2,5>(maps_geocode, args);
}

// Map Value
int
GeoCodessis (Word* args, Word& result, int message,
             Word& local, Supplier s)
{

  string st = (string)(char*)((CcString*) args[0].addr )->GetStringval();
  string no = (string)(char*)((CcString*) args[1].addr )->GetStringval();
  int plz = ((CcInt*) args[2].addr )->GetIntval();
  string postcode= itoa(plz);
  string c = (string)(char*)((CcString*) args[3].addr) ->GetStringval(); 

  //Check of spaces and special characters in URL

string newSt = replaceinString(st, " ", "%20");
string newSt1 = replaceinString(newSt, "ß", "ss");
string newSt2 = replaceinString(newSt1, "ä", "ae");
string newSt3 = replaceinString(newSt2, "ö", "oe");
string newSt4 = replaceinString(newSt3, "ü", "ue");

string newc = replaceinString(c, " ", "%20");
string newc1 = replaceinString(newc, "ß", "ss");
string newc2 = replaceinString(newc1, "ä", "ae");
string newc3 = replaceinString(newc2, "ö", "oe");
string newc4 = replaceinString(newc3, "ü", "ue");
  
  result = qp->ResultStorage(s);
                                //query processor has provided
                                //a point instance for the result
  //constant values of url	
  string pre="GET /maps/api/geocode/xml?address=";	
  string post="+CA&sensor=false HTTP/1.1\r\nHost: maps.googleapis.com\r\nConnection: close\r\n\r\n";	
  //create url			
  string request("");
  request += pre+ newSt4+  "+";
  request += no+ "+"   +postcode;
  request +=  "+"   +newc4+ post;

  
hostent* phe = gethostbyname("maps.googleapis.com"); 

    //if host not found
    if(phe == NULL) 
    { 
        return 0; 
    } 

    int Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    //error while craeating socket
    if(Socket == -1) 
    { 
        return 0; 
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
            return 0; 
        } 

        service.sin_addr.s_addr = *reinterpret_cast<unsigned long*>(*p); 
        ++p; 
        resulte = connect(Socket, reinterpret_cast<sockaddr*>(&service), sizeof(service));
     } 
    while(resulte == -1); 


	
	
    SendAll(Socket, request.c_str(), request.size());



    ofstream fout("../bin/output.xml"); 
	

 
int n = 0;
  
    while(true) 
    { 
        stringstream line; 
	
        try 
        { 
	    GetLine(Socket, line);
        } 
        catch(exception& e) 
        { 
            break; 
        } 
        if (n>11)
	{

	fout << line.str() << endl; 
        }
	else 
	{n++;}

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
//doc = xmlParseFile("/home/secondo/output.xml");
doc = xmlParseFile("../bin/output.xml");
cur = xmlDocGetRootElement(doc);

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
			      if ((xmlStrcmp(subsubcur->name, (const xmlChar *)"location"))==0) {
				subsubsubcur = subsubcur->children;
				while (subsubsubcur != NULL) {
				  if ((xmlStrcmp(subsubsubcur->name, (const xmlChar *)"lat"))==0) {
				    xmlChar *val1;
				    val1 = xmlNodeListGetString(doc, subsubsubcur->children, 1);
				    ausg1= OsmImportOperator::convStrToDbl((const char *)val1);
				    //((CcReal*)result.addr)->Set(true, ausg);
				    xmlFree(val1);
				  }
				  if ((xmlStrcmp(subsubsubcur->name, (const xmlChar *)"lng"))==0) {
				    xmlChar *val2;
				    val2 = xmlNodeListGetString(doc, subsubsubcur->children, 1);
				    ausg2= OsmImportOperator::convStrToDbl((const char *)val2);
				    //((CcReal*)result.addr)->Set(true, ausg);


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
((Point*)result.addr)->Set(ausg2, ausg1);
return 0;
}

 
 return 0; 

}

int
GeoCodesiis (Word* args, Word& result, int message,
             Word& local, Supplier s)
{

  string st = (string)(char*)((CcString*) args[0].addr )->GetStringval();
  int no = ((CcInt*) args[1].addr )->GetIntval();
  string number= itoa (no);
  int plz = ((CcInt*) args[2].addr )->GetIntval();
  string postcode = itoa (plz);
  string c = (string)(char*)((CcString*) args[3].addr) ->GetStringval(); 

  //Check of spaces and special characters in URL

string newSt = replaceinString(st, " ", "%20");
string newSt1 = replaceinString(newSt, "ß", "ss");
string newSt2 = replaceinString(newSt1, "ä", "ae");
string newSt3 = replaceinString(newSt2, "ö", "oe");
string newSt4 = replaceinString(newSt3, "ü", "ue");

string newc = replaceinString(c, " ", "%20");
string newc1 = replaceinString(newc, "ß", "ss");
string newc2 = replaceinString(newc1, "ä", "ae");
string newc3 = replaceinString(newc2, "ö", "oe");
string newc4 = replaceinString(newc3, "ü", "ue");
  
  result = qp->ResultStorage(s);
                                //query processor has provided
                                //a point instance for the result
  //constant part of url	
  string pre="GET /maps/api/geocode/xml?address=";	
  string post="+CA&sensor=false HTTP/1.1\r\nHost: maps.googleapis.com\r\nConnection: close\r\n\r\n";	
  //crate URL				
  string request("");
  request += pre+ newSt4+  "+";
  request += number+ "+"   +postcode;
  request +=  "+"   +newc4+ post;


  
hostent* phe = gethostbyname("maps.googleapis.com"); 
// host not found
    if(phe == NULL) 
    { 
        return 0; 
    } 



    int Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if(Socket == -1) 
    { 
        return 0; 
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
            return 0; 
        } 

        service.sin_addr.s_addr = *reinterpret_cast<unsigned long*>(*p); 
        ++p; 
        resulte = connect(Socket, reinterpret_cast<sockaddr*>(&service), sizeof(service));
     } 
    while(resulte == -1); 
	
	
    SendAll(Socket, request.c_str(), request.size());



ofstream fout("../bin/output.xml"); 
	

int n = 0;
  
    while(true) 
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

//close connection
close(Socket); 
 


//xml-Parser

xmlDocPtr doc;
xmlNodePtr cur;
xmlNodePtr check;
xmlNodePtr subcur;
xmlNodePtr subsubcur;
xmlNodePtr subsubsubcur;
//doc = xmlParseFile("/home/secondo/output.xml");
doc = xmlParseFile("../bin/output.xml");
cur = xmlDocGetRootElement(doc);

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
			      if ((xmlStrcmp(subsubcur->name, (const xmlChar *)"location"))==0) {
				subsubsubcur = subsubcur->children;
				while (subsubsubcur != NULL) {
				  if ((xmlStrcmp(subsubsubcur->name, (const xmlChar *)"lat"))==0) {
				    xmlChar *val1;
				    val1 = xmlNodeListGetString(doc, subsubsubcur->children, 1);
				    ausg1= OsmImportOperator::convStrToDbl((const char *)val1);
				    //((CcReal*)result.addr)->Set(true, ausg);
				    xmlFree(val1);
				  }
				  if ((xmlStrcmp(subsubsubcur->name, (const xmlChar *)"lng"))==0) {
				    xmlChar *val2;
				    val2 = xmlNodeListGetString(doc, subsubsubcur->children, 1);
				    ausg2= OsmImportOperator::convStrToDbl((const char *)val2);
				    //((CcReal*)result.addr)->Set(true, ausg);
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
((Point*)result.addr)->Set(ausg2, ausg1);
return 0;
}
 return 0; 
}







//Operator descrition

struct GeoCodeInfo : OperatorInfo {

  GeoCodeInfo()
  {
    name      = "geocode";
    signature = CcString::BasicType() + " x " + CcString::BasicType()
    + " x " + CcInt::BasicType() + " x "+ CcString::BasicType() +" -> " + Point::BasicType();
    appendSignature( CcString::BasicType() + " x " + CcInt::BasicType()
    + " x " + CcInt::BasicType() + " x "+ CcString::BasicType() +" -> " + Point::BasicType() );
    syntax    = "geocode (_, _, _, _)";
    meaning   = "The operator geocode returns the koordinates to an address. The address has to be entered in the form: street, number, plz, city. The result is a point.";
  }

}; // Don't forget the semicolon here. Otherwise the compiler
   // returns strange error messages



//5 Definition of operator geocode
ValueMapping GeoCodeFun[] =
        { GeoCodessis, GeoCodesiis, 0};
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

    AddOperator( GeoCodeInfo(), GeoCodeFun,  GeoCodeSelect, GeoCodeTypeMap);

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
