/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen,
Faculty of Mathematics and Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]
//[_] [\_]
//[TOC] [\tableofcontents]

[1] MailAlgebra

This algebra provides Mail functions for different
purposes.


*/

/*

[TOC]

1 Overview

This file contains the implementation import / export operators.

2 Defines, includes, and constants

*/

#include <cmath>
#include <stack>
#include <limits>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "SocketIO.h"

#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "SecondoSystem.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "DateTime.h"
#include "Algebras/TopOps/TopOpsAlgebra.h"
#include "Algebras/BinaryFile/BinaryFileAlgebra.h"
#include "Tools/Flob/DbArray.h"
#include "Symbols.h"
#include "FileSystem.h"
#include "ListUtils.h"
#include "Attribute.h"
#include "StringUtils.h"
#include "version.h"

#include "DbVersion.h"
#include "Algebras/Spatial/RegionTools.h"
#include "Algebras/IMEX/NMEAImporter.h"
#include "Algebras/Stream/Stream.h"
#include <string>



extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

#define FILE_BUFFER_SIZE 1048576 

using namespace std;



/*

1 Operator ~sendmail~

1.1 Type Mapping for ~sendmail~

*/


ListExpr sendmailtypemap( ListExpr args )
{
  if(!nl->HasLength(args,5))
   {
    return listutils::typeError("five arguments expected");
   }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  
  if((!FText::checkType(arg1)) ||  (!FText::checkType(arg2))   
      ||  (!FText::checkType(arg3))  ||  (!FText::checkType(arg4))  
      ||  (!FText::checkType(arg5)) )
  {
    return listutils::typeError("all  arguments must have a FText value");
  } 
  
  return nl->SymbolAtom(CcBool::BasicType());
  
   

}

/*
2  Operator ~embedTags~

121 Type Mapping for ~embedTags~

*/
 
ListExpr embedTagstypemap( ListExpr args )
{
  

  if(nl->ListLength(args)!=4){
    ErrorReporter::ReportError("four elements expected");
    return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!IsStreamDescription(stream)){
    ErrorReporter::ReportError("first argument is not a tuple stream");
    return nl->TypeError();
  }

  

 ListExpr first = nl->Second(args);
 ListExpr second = nl->Third(args); 
 ListExpr third = nl->Fourth(args);
 
   
 
 
 
  if( (nl->ListLength(first) != -1) || (nl->ListLength(second) != -1 ) 
       || (nl->ListLength(third)  != -1) )
  
  
  {
    ErrorReporter::ReportError("three attribute name arguments expected");
    return nl->TypeError();
  }

  
 
  
  
 

 
 if (!listutils::isSymbol(first)) {
     return  listutils::typeError("Second argument must be an attribute name");
    }
     
if (!listutils::isSymbol(second)) {
     return  listutils::typeError("Third argument must be an attribute name");
     } 
      
if (!listutils::isSymbol(third)) {
    return  listutils::typeError("Fourth argument must be an attribute name");  
    }
     

  
 // copy attrlist to newattrlist
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn = newAttrList;
  attrList = nl->Rest(attrList);
  while (!(nl->IsEmpty(attrList)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(attrList));
     attrList = nl->Rest(attrList);
  }
  
  
  

  // reset attrList
  attrList = nl->Second(nl->Second(stream));
  
  ListExpr attrtype;
 
    
  // check argues
  
  
  ListExpr firstname, secondname, thirdname; 
  string firstnamestr, secondnamestr, thirdnamestr; 
  int posit, posit2, posit3;
  ListExpr typea, typeb;
  
  
  
 
  
  if (nl->AtomType(first) == SymbolType)
   {   
      firstname =  first;
      firstnamestr = nl->SymbolValue(firstname);
   }

   else 
  {
    ErrorReporter::ReportError 
                   ("Attributename in the list is not of symbol type.");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  } 
   
 
 
 
 if (nl->AtomType(second) == SymbolType)
   {   
      secondname = second;
      secondnamestr = nl->SymbolValue(secondname);
   }

   else 
    {
    ErrorReporter::ReportError 
    ("Attributename in the list is not of symbol type.");
     return nl->SymbolAtom(Symbol::TYPEERROR());
    } 
   
   
   
   
   
   if (nl->AtomType(third) == SymbolType)
   {   
      thirdname = third;
      thirdnamestr = nl->SymbolValue(thirdname);
   }

   
   else {
    ErrorReporter::ReportError 
    ("Attributename in the list is not of symbol type.");
    return nl->SymbolAtom(Symbol::TYPEERROR());
      } 
   
   
   
   
      
 
 posit2 = FindAttribute(attrList,secondnamestr,attrtype);
 if(posit2!=0){
       ErrorReporter::ReportError("Attribute "+ secondnamestr +
                                  " is already a member of the tuple");
       return nl->TypeError();
    }
 
 
 
 posit3 = FindAttribute(attrList,thirdnamestr,attrtype);
 if(posit3!=0){
       ErrorReporter::ReportError("Attribute "+ thirdnamestr +
                                  " is already a member of the tuple");
       return nl->TypeError();
    }
 
 
   

posit = FindAttribute(attrList,firstnamestr,attrtype);
  
  if(posit==0){
       ErrorReporter::ReportError("Attribute "+ firstnamestr +
                                  " must be a member of the tuple");
       return nl->TypeError();
    }
  
    
  if (!FText::checkType(attrtype)) {   
      return listutils::typeError
      ("Attribute" + firstnamestr+ " must have a FText value");          
  }
  

  

  
 

        
typea = nl->SymbolAtom(CcBool::BasicType());

typeb = nl->SymbolAtom(FText::BasicType());
           
// append attribute
 
lastlistn = nl->Append(lastlistn, (nl->TwoElemList(secondname, typeb)));
  
lastlistn = nl->Append(lastlistn, (nl->TwoElemList(thirdname, typea)));
  
  

  
return 
     nl->ThreeElemList(
            
        nl->SymbolAtom(Symbol::APPEND()),
        nl->OneElemList(nl->IntAtom(posit)),                     
        nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),newAttrList)));
            
            
        
}





 
  
/*

1.2 Value Mapping for ~sendmail~

*/
 
int sendmailVM(Word* args, Word& result,
                 int message, Word& local, Supplier s)
{
  
  
  FText* firstargsub = static_cast<FText*>(args[0].addr);
  FText* secondargsender = static_cast<FText*>(args[1].addr);  
  FText* thirdargadress = static_cast<FText*>(args[2].addr);  
  FText* fourthargmessage = static_cast<FText*>(args[3].addr);  
  FText* fifthargmessage = static_cast<FText*>(args[4].addr);  
  
  result = qp->ResultStorage(s);  
  CcBool* b = static_cast<CcBool*>( result.addr );
  bool res = false;
  

  if((!firstargsub->IsDefined()) || (! secondargsender->IsDefined()) 
      || (! thirdargadress->IsDefined()) ||  (! fourthargmessage->IsDefined()) 
      || (! fifthargmessage->IsDefined()) )
   { 
    b->SetDefined(false);
    return 0;
   } 
   
   
//constructing the postfix call   
 
string sub = firstargsub->GetValue();
string sdr = secondargsender->GetValue();
string adr = thirdargadress->GetValue();
string msg = fourthargmessage->GetValue();
string cbc = fifthargmessage->GetValue();


string subpart1 = "mail -s '";
string subpart2 = "' -r ";
string subpart3 = " <<< '";
string subpart4 = " ";
string subpart5 = "'";

string subpart6 = " -c ";

string finalcall = "";

string call = subpart1 + sub + subpart2 + sdr + 
              subpart4 + adr + subpart3 + msg + subpart5;

string  callcarbcopy = subpart1 + sub + subpart2 + sdr + subpart6 + 
                       cbc + subpart4 + adr + subpart3 + msg + subpart5;
               
if (cbc.empty())
{ 
  finalcall = call;
}    

else {
       finalcall= callcarbcopy;
       
     }

     
 
 //checking if mail adress has some syntax errors
 
                   
const char * cll1 = finalcall.c_str();
int back1 = 0; 
 
size_t at1 = sdr.find('@');
size_t at2 = adr.find('@'); 
size_t at3 = cbc.find('@'); 



size_t dot1 = sdr.find('.', at1 + 1);
size_t dot2 = adr.find('.', at2 + 1);
size_t dot3 = cbc.find('.', at3 + 1);



size_t count1 = std::count(sdr.begin(), sdr.end(), '@');
size_t count2 = std::count(adr.begin(), adr.end(), '@');
size_t count7 = std::count(cbc.begin(), cbc.end(), '@');
size_t count3 = std::count(sdr.begin(), sdr.end(), ':');
size_t count4 = std::count(adr.begin(), adr.end(), ':');
size_t count8 = std::count(cbc.begin(), cbc.end(), ':');
size_t count5 = std::count(sdr.begin(), sdr.end(), '/');
size_t count6 = std::count(adr.begin(), adr.end(), '/');
size_t count9 = std::count(cbc.begin(), cbc.end(), '/');
  



if (!(cbc.empty()))

{ 
    
 if ( (count1 == 1) && (count2 == 1) && (count7 == 1) 
     && (!((dot1 == string::npos) 
     || (dot2 == string::npos) || (dot3 == string::npos))) &&
     ((count3 == 0) && (count4 == 0) && (count5 == 0) && (count6 == 0) 
     && (count8 == 0) && (count9 == 0) ) )
       
    { 
        
     back1 = system(cll1);    //system call execution
     
     if  (!(back1 == 0)) 
       { 
         b->Set(true, res);   

         return 0;
       }
    
      
      res = true;
      b->Set(true, res);   
      return 0;
    
    
    
    }
    
 else { 
        b->Set(true, res);
        return 0;  
        
      }
    
}


else
    
{ 
  if ( (count1 == 1) && (count2 == 1) && (!((dot1 == string::npos) 
     || (dot2 == string::npos))) &&
     ((count3 == 0) && (count4 == 0) && (count5 == 0) && (count6 == 0) ))
     
       
    { 
        
     back1 = system(cll1); 
     
     if  (!(back1 == 0)) 
       { 
         b->Set(true, res);   
         return 0;
       }
    
      
      res = true;
      b->Set(true, res);   
      return 0;
    
    
    
    }
    
  else { 
        b->Set(true, res);
        return 0;  
        
      }
    
    
   }    


return 0; //never happens
 
}
  
   
   
   
   
   
struct embedTagsInfo {
 
map<string, int> toposvalue;

map<string, string> totypevalue;
    
    
};
   
   
 
   
   
int embedTagsVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  
  Tuple* tup;
  
  
  embedTagsInfo*  localInfo = (embedTagsInfo*) qp->GetLocal2(s).addr;

   
  
switch (message)
  {
    
    case OPEN : {
        
     ListExpr resultType = GetTupleResultType(s);
     TupleType *tupleType = new TupleType(nl->Second(resultType));
     local.addr = tupleType;
     
     
     ListExpr attrsave = qp->GetType(qp->GetSon(s,0));
     ListExpr attrsave2 = nl->Second(nl->Second(attrsave));
     map<string, int> toposvalue2;
     map<string, string> totypevalue2;
     
     localInfo = new embedTagsInfo;
     qp->GetLocal2(s).addr = localInfo;        
     
     int counter = 0; 
    
      
      
     
     while (!(nl->IsEmpty(attrsave2)))
     {
     ListExpr temp = nl->First(attrsave2); 
     ListExpr temp2 = nl->First(temp);   
     ListExpr temp3 = nl->Second(temp);
     string attrname = nl->SymbolValue(temp2);
     string typen = nl->SymbolValue(temp3);
     
     
     //setting up the localInfo
     localInfo->toposvalue.insert(pair<string, int> (attrname, counter));
     localInfo->totypevalue.insert(pair<string, string> (attrname, typen));
     
     attrsave2 = nl->Rest(attrsave2);
     
    
     
     counter++;
     }
     
     
     
    
     
     qp->Open(args[0].addr);
     
    
     
      return 0;
    }
    
    
   case REQUEST : {
        
      
        
      int veclen;
      bool wert = true;  
      size_t pos1 = 0;
      string brack = "<<";
      string brack2 = ">>";
      vector<string> vstr;
      map<string, int>::iterator iter=localInfo->toposvalue.begin();
      int maxattrpos = 0;
      qp->Request(args[0].addr,t);
      
      
      
      
      
      
    
    //counting number of attributes
            
    map<string, int>::iterator it2 = localInfo->toposvalue.begin();
     
     while(it2 != localInfo->toposvalue.end())
    {
                
        if (maxattrpos < it2->second) maxattrpos= it2->second;        
        
        it2++;       
    }
      
 
     
      
     
    if (qp->Received(args[0].addr))
          
      { 
        tup = (Tuple*)t.addr;
        
        TupleType *tupleType = (TupleType*)local.addr;
        Tuple *newTuple = new Tuple( tupleType );
       
           
            
        // copy old attributes
        
        
        
     for( int i = 0; i < tup->GetNoAttributes(); i++ ) {
          
          newTuple->CopyAttribute( i, tup, i );
        }
        
        
        
   
            
     // get needed tuple values
        
        
      int briefpos = ((CcInt*)args[4].addr)->GetIntval();
     
        
      Attribute* brief = tup->GetAttribute(briefpos-1);      
     
      FText* briefval = static_cast<FText*>(brief);
      string briefvalstr =  briefval->GetValue();
       
       
       
       //find the tags

       
       while ((pos1 = briefvalstr.find(brack, pos1)) != std::string::npos)
         {
            size_t found = briefvalstr.find(brack,pos1);
            size_t found2 = briefvalstr.find(brack2,pos1);
            size_t lenght = found2-found-2;
  
  
  
             string substr = briefvalstr.substr(found+2, lenght);
  
                     
             pos1 += substr.length();               
  
             vstr.push_back(substr);
  
            }  
            
       
       
        
             
           
           
       
       
            
        //get the tag related attribute values 
        
        

            veclen = vstr.size();        
            Attribute*  attrar[veclen];
            string typear[veclen];
            string strvalues[veclen];
            
        
        
         for (int i=0; i<veclen; i++)
            {
                
            iter = localInfo->toposvalue.find(vstr[i]);
         
            if (iter == localInfo->toposvalue.end())  
             {
                
                wert = false;    
                FText* brief2 = new FText (true, 
                               "Error: one tag is not related to an attribute");
                newTuple->PutAttribute(maxattrpos+1, brief2);
            
            
                CcBool* wert2 = new CcBool(true, wert);
                newTuple->PutAttribute(maxattrpos+2, wert2);      
                tup->DeleteIfAllowed();
                result = SetWord(newTuple);
        
                return YIELD;
                
             }    
                
                       
                
             
             attrar[i] = tup->GetAttribute(localInfo->toposvalue.at(vstr[i]));
             
             
              if (!(attrar[i]->IsDefined()))
                 
        
             {
            
                wert = false;    
                FText* brief2 = new FText (true,
                                "ERROR: tag related attribute undef");
                newTuple->PutAttribute(maxattrpos+1, brief2);
            
            
                CcBool* wert2 = new CcBool(true, wert);
                newTuple->PutAttribute(maxattrpos+2, wert2);      
                tup->DeleteIfAllowed();
                result = SetWord(newTuple);
        
                return YIELD;
            
             }
             
             
             
                             
                          
             typear[i] = localInfo->totypevalue.at(vstr[i]);
             
             if (  !( (typear[i] == "string") || (typear[i] == "text")
                || (typear[i] == "real") || (typear[i] == "int") 
                || (typear[i] == "date") )  )
             
             
             {  wert = false;    
                FText* brief2 = new FText (true,
                       "ERROR: tag related attribute type is not supported");
                newTuple->PutAttribute(maxattrpos+1, brief2);
            
            
                CcBool* wert2 = new CcBool(true, wert);
                newTuple->PutAttribute(maxattrpos+2, wert2);      
                tup->DeleteIfAllowed();
                result = SetWord(newTuple);
        
                return YIELD;
            
              
                 
             }   
             
             // end of error cases
             
             
             //extract the attribute values
             
             
             if (typear[i] == "string")
             {
                 
              CcString* temp1 = static_cast<CcString*>(attrar[i]);
              strvalues[i] =  temp1->GetValue();
       
             }    
             
             
                        
             
            if (typear[i] == "text")
             {
                 
              FText* temp3= static_cast<FText*>(attrar[i]);
              
              strvalues[i] =  temp3->GetValue();
       
             }    
             
             
             
            if (typear[i] == "real")
            {
             CcReal* temp4 = static_cast<CcReal*>(attrar[i]);
             double  val = temp4-> GetRealval();       
             ostringstream Str;
             Str <<  val;
             string valstr(Str.str());
             strvalues[i] = valstr;
             
            }
                
            
            
             if (typear[i] == "int")
             {
                 
             CcInt* temp5= static_cast<CcInt*>(attrar[i]);
             int  val2 = temp5-> GetIntval();       
             ostringstream Str2;
             Str2 <<  val2;
             string valstr2(Str2.str());
             strvalues[i] = valstr2;
              
              
             }    
             
             
           
             if (typear[i] == "date")
             {
                 
             string datevalue = attrar[i]->getCsvStr();
             strvalues[i] = datevalue;
             
             
              
             }    
             
             
           
        
            
                
            }   //end of for  
        
        
           
           
        
        //replace tags with attribute values
        
        
        
        for (int i= 0; i<veclen; i++)
            { string tagme = "<<" + vstr[i] + ">>";
              size_t prosit = 0;
              
             
             
             if (briefvalstr.find(tagme, prosit) != string::npos) 
             {
       
                
             while ( (prosit = briefvalstr.find(tagme, prosit)) 
                     != std::string::npos) 
                     {                 
                      briefvalstr.replace(prosit, tagme.length(), strvalues[i]);
                      prosit += strvalues[i].length();
                     }
        
             }                
                

            }
            
          
        
       // setting up the new tuple
        
        wert = true;
        FText* brief2 = new FText (true, briefvalstr);
        newTuple->PutAttribute(maxattrpos+1, brief2);
        
       
        
        CcBool* wert2 = new CcBool(true, wert);
        newTuple->PutAttribute(maxattrpos+2, wert2);      
        tup->DeleteIfAllowed();
        result = SetWord(newTuple);
        
        return YIELD;
      }   
      else  
        return CANCEL;   // no args left
      
    } 

    case CLOSE : {
        
      if(localInfo){
        delete localInfo;
         qp->GetLocal2(s).addr=0;
      }
        
      qp->Close(args[0].addr);
      
      if (local.addr)
      {
        ((TupleType*)local.addr)->DeleteIfAllowed();
        local.setAddr(0);
          
          
          
      }    
      
      
      return 0;
    }
  
  
 
  
} //end switch


  return 0;
}
   
   
   
   


   
   
   
   
   
   
   
   
   
    
/*
1.4 Specification  for ~sendmail~

*/

const string sendmailSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text x text x text x text, text   -> bool </text--->"
    "<text>sendmail (subject, sender, receiver, message, carboncopy) </text--->"
    "<text>Sends a mail to the address specified " 
    "with the third argument. The second argument " 
    "is the sender address. "
    "The first argument is the subject of the mail " 
    "and the fourth is the message. "
    " With the fifth argumemt you can put in a carbon copy address."
    " Just type in the empty text ('') if you do not want a copy. "
    " TRUE is returned if there are no email address syntax errors." 
    " FALSE is returned if the address is not syntactically correct" 
    " and UNDEFINED if any other error occurs.</text--->"
    "<text> query sendmail('a subject', 'sender@universe2.com', " 
    "'receiver@universe1.com', 'message' 'someone@universe3.com)  </text--->"
    ") )";


/*
 
2.4 Specification  for ~embedTags~

*/
 

const string embedTagsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream (tuple ((x1 T1) ... "
  "(xn Tn)))) (ak1, ak2, ak3)) -> (stream (tuple"
  " ((ai1 Ti1) ... (ain Tin) (ak2, Tk2) (ak3, Tk3))))</text--->"
  "<text>_ embedTags [ list ]</text--->"
  "<text>Produces a tuple for each "
  "tuple of its input stream "
  "with two additional appended attributes. "
  "The tuple stream that is fed in must have "
  "certain attributes including one "
  "that is specified by ak1. This must be a "
  "text value with some tags in << >> brackets. "
  "For example, if these tags are <<Note>>, <<Kurs>>, <<Nachname<< and <<Dr>> "
  "then Note, Kurs, Nachname, Dr must also be attributes in the "
  "tuple stream that is fed in. In the result stream the tags are "
  "replaced with the corresponding attribute values and the modified "
  "text is a value of the attribute ak2. Furthermore a bool attribute "
  "is appended which will have the value FALSE if something went wrong, "
  "or TRUE if the text was modified correctly. </text--->"  
  "<text> query pruefung feed embedTags[Brief, Brief2, Erfolg] " 
  " consume  </text--->"
  ") )";
   


                  

Operator sendmail ( "sendmail",
                   sendmailSpec,
                   sendmailVM,
                   Operator::SimpleSelect,
                   sendmailtypemap );
         
                 
Operator embedTags( "embedTags",
                   embedTagsSpec,
                   embedTagsVM,
                   Operator::SimpleSelect,
                   embedTagstypemap );
         

      
         
/*
1.6 Creating the Algebra

*/

class MailAlgebra : public Algebra
{
public:
  MailAlgebra() : Algebra()
  {   
    AddOperator( &sendmail);
    AddOperator( &embedTags);
  }
  ~MailAlgebra() {};
};

/*
1.7  Initialization 


*/

extern "C"
Algebra*
InitializeMailAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new MailAlgebra());
}


