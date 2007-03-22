 
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

You should have received a coplet page1 = [const page value ((html ((instant (10 10 2006 10 27 18)) <file>/home/sopra/secondo/Algebras/Web/bilder.htm</file--->(url ("http"<text>www.myimages.de</text---> <text>/</text---> )))) ((url ("http"<text>Garten-1.jpg</text---> <text>/</text---> ))<file>/home/sopra/secondo/Algebras/Web/Garten-1.jpg</file--->"image/jpeg")( (url ("http" <text>Garten-2.jpg</text---><text>/</text---> ))<file>/home/sopra/secondo/Algebras/Web/Garten-2.jpg</file--->"image/jpeg"))];y of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Templelet page1 = [const page value ((html ((instant (10 10 2006 10 27 18)) <file>/home/sopra/secondo/Algebras/Web/bilder.htm</file--->(url ("http"<text>www.myimages.de</text---> <text>/</text---> )))) ((url ("http"<text>Garten-1.jpg</text---> <text>/</text---> ))<file>/home/sopra/secondo/Algebras/Web/Garten-1.jpg</file--->"image/jpeg")( (url ("http" <text>Garten-2.jpg</text---><text>/</text---> ))<file>/home/sopra/secondo/Algebras/Web/Garten-2.jpg</file--->"image/jpeg"))]; Place, Suite 330, Boston, MA  02111-1307  USA
----

[1] /Web Algebra
 
November 2006

1 Preliminaries
 
1.1 Includes

*/



#define __POS__ __FILE__ << ".." << __PRETTY_FUNCTION__ << "@" << __LINE__
//#define TRACEON
#ifdef TRACEON 
#define __TRACE__ cout << __POS__ << endl;
#else
#define __TRACE__
#endif

//#define _DEBUG_JPS  //Enables Debug output used by Joerg Siegel
//#define _DEBUG_JPS_2  //Enables Debug output used by Joerg Siegel
//#define _DEBUG_JPS_3  //Enables Debug output used by Joerg Siegel

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"  //needed because we return a CcBool in an op.
#include "DBArray.h"
#include "Attribute.h"
#include "DateTime.h"
#include "FLOB.h"
#include "../FText/FTextAlgebra.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "web.h"
#include "SocketIO.h"      //used for web access
#include "Base64.h"        //to en-/ decode binary data
#include <stack>
#include <string>
#include "../BinaryFile/BinaryFileAlgebra.h"

#ifdef SECONDO_WIN32
#include "../../ClientServer/Win32Socket.h"
#else //Linux
#include "../../ClientServer/UnixSocket.h"
#endif
extern NestedList* nl;
extern QueryProcessor *qp;
using namespace datetime;

/*
1.2 Dummy Functions

No dummy function needed.

*/
/*
2.0 needed definitions

*/

/*


2.1 Implementation of WebLex

*/

WebLex::WebLex(istream *is) : yyFlexLexer (is) {switchState=-1;}

int WebLex::nextToken(){
  int symbol=0;
  
//__TRACE__
  symbol=yylex(switchState);
//__TRACE__
  switchState=-1;
  
  tokenVal= YYText();

  if (tokenVal.length() == 0)
    return symbol;
  
  if (tokenVal[0]=='"' && tokenVal[tokenVal.length()-1]=='"'){
    if (tokenVal.length() > 2){
      tokenVal.erase(0,1);
      tokenVal.erase(tokenVal.length()-1);
    }else{
      tokenVal="";
    }
  }
  
  
  return symbol;
}  

void WebLex::switchStartCond(int ns){
  switchState=ns;
}

string WebLex::getVal() {
  return tokenVal;
}
  
int WebLex::yylex(){return 0;}

 int WebLex::startElement (string& element){
  
  int symbol=0;
  switchStartCond(FINDELEMSTART);
  symbol=nextToken();
  //cout << "-" << getVal() << endl;
  while (symbol == SEARCH_ELEMENT_START){
    //cout << "-" << getVal() << endl;
    symbol=nextToken();
  }
   //cout << "ENDE startelement  " << getVal() << endl;
   element= getVal();
  
  if (symbol){
    return symbol;
  }

  return 0;

}

/*

in: attribute
out: value
return: true if ~attribute~ was found in input stream, false otherweise

Looking for the attribute in the input stream of WebLex. Param ~value~ contains the value of the attribute

*/
bool WebLex::findAttribute(string attribute, string& value){
  
  value="";
  int symbol;
  
  
  __TRACE__
  symbol=nextToken();

  //__TRACE__
  while (symbol && symbol != CLOSE_TAG){

    if (symbol == ERROR){
      cout << "findAttribute Es ist ein Fehler aufgetreten" << endl;
      return false;
    }
    //__TRACE__
    //we found an attribute identifier
    if (symbol == EIDENTIFIER){
//__TRACE__
      //is this the attribute we are looking for?
      if (isEqual(getVal(),attribute)){
        //cout << "findAttribute Atribut gefunden " << endl;
        if (symbol == ERROR){
          cout << "Fehler: " << getVal() << endl;
          return false;
        }
//__TRACE__
        symbol=nextToken();
        if (symbol == ATTVALUE){
          value = getVal();
          
          return true;
        
        }else{
          return true;
        }
      }      
    }
    //__TRACE__
    symbol=nextToken();
    //__TRACE__
  }
  return false;


}


/*

in: attributes
out: value
return: true if of of the elements of ~attributes~ was found in input stream, false otherweise

Looking for the attribute in the input stream of WebLex. Param ~value~ contains the value of the attribute

*/

bool WebLex::findAttribute(vector<string>& attributes, 
                           string& value, string& attribute){
  
  value="";
  int symbol;
  
  
  //__TRACE__
  symbol=nextToken();

  //__TRACE__
  while (symbol && symbol != CLOSE_TAG){
    
    if (symbol == ERROR){
      cout << "findAttribute Es ist ein Fehler aufgetreten" << endl;
      return false;
    }
    //__TRACE__
    //we found an attribute identifier
    if (symbol == EIDENTIFIER){
//__TRACE__
      //is this the attribute we are looking for?
      vector<string>::iterator it = attributes.begin();
      
      while (it != attributes.end()){
        //cout << "FINDATTR " << *it << endl;
        if (isEqual(*it,getVal())){
          attribute=*it;
          //cout << "findAttribute Atribut gefunden " << endl;
          if (symbol == ERROR){
            cout << "Fehler: " << getVal() << endl;
            return false;
          }
  //__TRACE__
          symbol=nextToken();
          if (symbol == ATTVALUE){
            value = getVal();
            
            return true;
          
          }else{
            return true;
          }
          
          
        }
        
        it++;
      }      
    }
    //__TRACE__
    symbol=nextToken();
    //__TRACE__
  }
  return false;


}

/*

Find Position of ~value~ in ~content~ and return flobindex Object

*/
flobindex WebLex::setPos(string value, const string& content){
  unsigned long tmp;
  flobindex i;

//__TRACE__  
  i.offset= 0;
  i.len=0;

//cout << value << pos << endl;
  
  
  tmp= (unsigned long) strstr(content.c_str() + pos, value.c_str()) ;
  
  
  if (!tmp)
    return i;
  
  
  pos = tmp - (unsigned long) content.c_str();
  i.offset=pos;
  i.len=value.length();
  
  return i;

}

/*

read content of a html element

*/
int WebLex::readContent(){
  int symbol=0;
  string value="";
  
//  __TRACE__

  symbol= nextToken();
  //cout << "******** readcontent *********" << endl;
  while (symbol == CONTENT){
    //cout << getVal() ;
    value += getVal();
    symbol= nextToken();
  }

  //cout << "readcontent: " << endl;  

  
  if (symbol){
    value += getVal();
    tokenVal= value;
    return CONTENT;
  }
  tokenVal= value;
  return symbol;
}

int WebLex::readContentTmp(){
  int symbol=0;
  string v="";
  
  
  __TRACE__
//cout << "**********TMP **************" << endl;

  
  symbol= nextToken();
  //cout << getVal()  << " " << symbol << endl;
  v += getVal();
  
  
  while (symbol == CONTENT){
    symbol= nextToken();
    v += getVal();
    //cout << ":" << getVal()  << " " << symbol << "  " << v << endl;
  }
  
  //cout << "---" << v << endl;
  return 0;


}

/*

2.2 Helping Functions

*/

bool isEqual (string s1, string s2){
  transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
  transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
  
  return s1 == s2;
}

//Taken from http://www.codeproject.com/string/stringsplit.asp
int SplitString(const string& input, 
       const string& delimiter, vector<string>& results, 
       bool includeEmpties)
{
    int iPos = 0;
    int newPos = -1;
    int sizeS2 = (int)delimiter.size();
    int isize = (int)input.size();

    if( 
        ( isize == 0 )
        ||
        ( sizeS2 == 0 )
    )
    {
        return 0;
    }

    vector<int> positions;

    newPos = input.find (delimiter, 0);

    if( newPos < 0 )
    { 
        return 0; 
    }

    int numFound = 0;

    while( newPos >= iPos )
    {
        numFound++;
        positions.push_back(newPos);
        iPos = newPos;
        newPos = input.find (delimiter, iPos+sizeS2);
    }

    if( numFound == 0 )
    {
        return 0;
    }

    for( int i=0; i <= (int)positions.size(); ++i )
    {
        string s("");
        if( i == 0 ) 
        { 
            s = input.substr( i, positions[i] ); 
        }
        int offset = positions[i-1] + sizeS2;
        if( offset < isize )
        {
            if( i == (int)positions.size() )
            {
                s = input.substr(offset);
            }
            else if( i > 0 )
            {
                s = input.substr( positions[i-1] + sizeS2, 
                      positions[i] - positions[i-1] - sizeS2 );
            }
        }
        if( includeEmpties || ( s.size() > 0 ) )
        {
            results.push_back(s);
        }
    }
    return numFound;
}

bool isWhite(char c){
  return c == ' ' || c == '\n' || c == '\t';
}


/*
3 l Definitions of ~URL, HTML, Page~

3.1 Class ~URL~

----
Example to create an object:
let url1 = [const url value ("http" <text>//www.google.de</text--->
<text>/</text--->)]
----

*/
class URL : public IndexableStandardAttribute
{
 public:
  URL();
  ~URL();
  URL(const string&);
  URL(const URL&);
  URL(const string &prot, const string &h, const string &pp);
  bool operator== (const URL& url) const;
  void setProtocol(string);
  string getProtocol() const;
  void setPath(string);
  string getPath() const;
  void setHost(string);
  string getHost() const;
  URL* Clone() const;
  friend ostream& operator<<(ostream& s, URL u);
  ListExpr ToListExpr(bool typeincluded)const;
  /* Returns whether this object is defined or not. */
  bool IsDefined() const;
  /* Sets this object as defined or undefined. */
  void SetDefined( bool Defined);
  size_t Sizeof() const;
  int Compare(const Attribute*) const;
  bool Adjacent(const Attribute*) const;
  //void operator=(const URL&);
  void Set( bool d, URL& u);
  void destroy(void);
  static bool urlFromString(const string& url,URL& myurl);
   inline virtual int NumOfFLOBs() const {__TRACE__ return 2;}
  FLOB *GetFLOB(const int);
  void WriteTo (char*)const;
  void ReadFrom(const char*);
  SmiSize SizeOfChars(void) const;
  size_t HashValue(void) const;
  void CopyFrom(const StandardAttribute *arg);
 private:
   STRING protocol;
  FLOB host;
  FLOB path;
  bool defined;
  static bool isValidURL(const string&);
  static bool isValidURL(const string&, string&, string&, string&);
};

/*
3.1.1 Implementation of Class-Operations of ~URL~

*/
URL::URL()
{
  __TRACE__
}


URL::~URL()
{
//  __TRACE__
}

URL::URL(const string& u)
:host(0),path(0)
{
//  __TRACE__
  string p;
  string h;
  string pa;
  
  if (!isValidURL(u, p, h, pa)){
  __TRACE__
    defined=false;
    return;
  }
//  __TRACE__
  //cout << p << " " << h << " " << pa << endl;
  defined = true;  
  setProtocol (p);
  setHost(h);
  setPath(pa);

  
}

URL::URL(const string &prot, const string &h, const string &p)
: host(h.length()+1), path(p.length()+2)
//: host(h.length()+1), path(p.length()+1)
{
   __TRACE__
  
  if (prot.length() > MAX_STRINGSIZE){
    defined=false;
    return;
  }
    
  __TRACE__
  //cout << "*************" << prot +  h + p << endl;
  

  if (!isValidURL(prot + "://" + h + p)){
    defined=false;
    return;
  }

  __TRACE__
  defined = true;
  setProtocol (prot);
  setHost(h);
  setPath(p);
}
  
URL::URL(const URL& u)
:host(u.getHost().length()+1),path(u.getPath().length()+1)
{
//  __TRACE__
  
  if (!u.IsDefined()){
    defined=false;
    return;
  }
  
  defined=true;
  //cout << "url: " << u.getPath() << "  " << defined << endl;
  setProtocol ( u.getProtocol());
  setHost(u.getHost());
  setPath(u.getPath());
  //cout << "url: " << getPath() << endl;
    
}

URL* URL::Clone() const
{ 
  __TRACE__
    
  URL *pUrl = new URL(getProtocol(),getHost(),getPath());
   return pUrl; 
}

string URL::getProtocol() const
{
//  __TRACE__
  if (!defined)
    return "";
  return protocol;
}
  

void URL::setProtocol(string p)
{
//  __TRACE__
  if (!defined)
    return;
  if (p.length()  <= MAX_STRINGSIZE){
    strcpy (protocol, p.c_str());
  }
}

string URL::getHost() const
{
//  __TRACE__
  if (!defined)
    return "";
  
  const char* s = 0;
  host.Get(0, &s);
  
  //cout << "getHost " << s << endl;
  return s;
}

void URL::setHost(string h)
{
//  __TRACE__
  if (!defined)
    return;
  //cout << "setHost " << h << endl;
  host.Resize (h.length() +1);
  host.Put (0,h.length() + 1, h.c_str());
}

string URL::getPath() const
{
//  __TRACE__
  if (!defined)
    return "";
  const char* s = 0;
  path.Get(0, &s);
  return s;
}

void URL::setPath(string p)
{
//  __TRACE__
  
  if (!defined)
    return;

  //cout << "setPath " << p << endl;
  if (p.length() == 0)
    p= "/";
  if (p.at(0) != '/')
    p= "/" + p;
  path.Resize (p.length() +1);
  path.Put (0,p.length() + 1, p.c_str());
}

ostream& operator<<(ostream& s, URL u)
{
//  __TRACE__
  if (!u.IsDefined())
    return s << "Value is Undefined";
  return s << "URL: [Protocol: " << u.getProtocol() << endl
    << "Host: " << u.getHost() << endl
    << "Path: " << u.getPath() << "]" << endl;
}

ListExpr URL::ToListExpr(bool typeincluded)const {
  __TRACE__
 
  ListExpr value;
  if( defined )
  {
    value = nl->ThreeElemList(
    nl->StringAtom(getProtocol()),
    nl->TextAtom(getHost()),
    nl->TextAtom(getPath()));
  }
  else
    value = nl->ThreeElemList(
    nl->StringAtom(""),
    nl->TextAtom(""),
    nl->TextAtom(""));
   if(typeincluded)
        return nl->TwoElemList(nl->SymbolAtom("url"),value);
  else
    return value;
}

bool URL::IsDefined() const {
//  __TRACE__
    return defined;
}

void URL::SetDefined( bool def) {
//  __TRACE__
    defined = def;
}

size_t URL::Sizeof() const
{
  __TRACE__
  return sizeof( *this );
}

int URL::Compare(const Attribute*) const
{
  __TRACE__
  return 0;
}

bool URL::Adjacent(const Attribute*) const
{
  __TRACE__
  
  return 0;
}

void URL::Set( bool d, URL& u)
{
  __TRACE__
  defined = d;
  
  if (!d || !u.IsDefined())
    return;


  string s = u.getProtocol();
  string h = u.getHost();
  string p = u.getPath();
  __TRACE__


  strcpy(protocol, s.c_str());
  host.Resize( h.length() + 1 );
  host.Put( 0, h.length() + 1, h.c_str() );
  path.Resize( p.length() + 1 );
  path.Put( 0, p.length() + 1, p.c_str() );
}


void URL::destroy(){
  __TRACE__
  host.Destroy();
  path.Destroy();
}

bool URL::urlFromString (const string& url,URL& myurl){
  string host;
  string protocol;
  string path;

//  __TRACE__
  
  if (!isValidURL(url, protocol, host, path)){
    myurl.SetDefined(false);
    return false;
  }
  
  
  myurl.SetDefined(true);
  myurl.setPath(path);
  myurl.setProtocol (protocol);
  myurl.setHost(host);
  
  
  return true;
  
}


bool URL::isValidURL(const string& url, string& protocol, 
                     string& host, string& path){
  stringstream is (url);
  WebLex lexer(&is);
  
//   __TRACE__
  
  lexer.switchStartCond(MSCHEME);
  //cout << url << endl;
  if (lexer.nextToken() != SCHEME){
//     __TRACE__
    return false;
  }
  
  protocol= lexer.getVal();
  protocol= protocol.erase(protocol.length()-1);
//  __TRACE__
  //cout << protocol << endl;
  
  if (lexer.nextToken() != AUTHORITY){
//     __TRACE__
    return false;
  }
  
  host= lexer.getVal();
  host=host.erase(0,2);
//  __TRACE__
  //cout  << host << endl;
  
  
  if (lexer.nextToken() == PATH){
    path= lexer.getVal();
  }else{
    path="";
  }  
  //__TRACE__
  //cout << lexer.getVal() << endl;

  return true;
}

bool URL::isValidURL(const string& url){
  string x,y,z;
  
  __TRACE__
  return isValidURL(url, x,y,z);
}

FLOB *URL::GetFLOB(const int i){
//  __TRACE__
  
  
  if ( i == 0 )
    return &host;
  
  if ( i == 1 )
    return &path;
  
  return NULL;
}

void URL::WriteTo ( char* dest ) const {
  __TRACE__
  string url= getProtocol() + getHost() + getPath();
  strcpy (dest, url.c_str());
}

SmiSize URL::SizeOfChars()const {
  __TRACE__
    return (strlen (protocol) + host.Size() + path.Size());
}

void URL::ReadFrom ( const char *src){
  __TRACE__
  int erg;
  string url (src);    
  stringstream is (url);
  
  WebLex lexer (&is);
  lexer.switchStartCond(MURI);
  
  string protocol;
  string host;
  string path;
  
  erg= lexer.nextToken();
  if (erg==ERROR)
    return;
  
  protocol= lexer.getVal();
  
  erg= lexer.nextToken();
  if (erg==ERROR)
    return;
  
  host= lexer.getVal();
  
  erg= lexer.nextToken();
  if (erg==ERROR)
    return;
  
  path= lexer.getVal();
  
  setProtocol ( protocol);
  setHost (host);
  setPath (path);
}


size_t URL::HashValue(void) const{ 
  __TRACE__
  return SizeOfChars();
}

void URL::CopyFrom(const StandardAttribute *arg){
  __TRACE__
  URL *url = (URL*) arg;
  setProtocol ( url->getProtocol());
  setHost ( url->getHost());
  setPath ( url->getPath());
}

bool URL::operator== (const URL& url) const{
  return (isEqual(url.getProtocol(),getProtocol()) && 
          isEqual(url.getHost(), getHost()) && 
          isEqual(url.getPath(), getPath()));
}

/*
3.2 Class ~HTML~

----
Example to create an object:
let html1 = [const html value ((instant (10 10 2006 10 27 18)) <file>/home/sopra/secondo/Algebras/Web/bilder.htm</file---> (url ("http" <text>www.mybilder.de</text---> <text>/</text---> )))]
----

*/
class HTML : public StandardAttribute
{
 public:
  HTML(){}
  ~HTML(){}
  HTML(const string& s);
  HTML(const DateTime &d, const string &s, const URL &u);
  HTML(const HTML&);
  bool operator== (const HTML& h) const;
  URL getSource() const;
  string getContent() const;
  string getText() const;
  int getNumberOfUrls() const;
  URL getUrl(const int i) ;
  int getNumberOfEmbUrls() const;
  URL  getEmbUrl (const int i);
  URL getUrlHosts(int i, string hosts, bool& contains);
  bool containsURL( const URL*);
  datetime::DateTime getLastModified() const;
  string getMetaInfo(string name);
  int getNumberOfMetainfos() const;
  string getMetainfo( int ii, string& pContent) const;
  int getNumberOf(string);
  double similar(HTML*, int, bool);
  HTML* Clone() const;
  ListExpr ToListExpr(bool typeincluded)const;
  bool IsDefined() const;
  void SetDefined(bool d) ;
  void Set(const HTML &h);
  FLOB *GetFLOB(const int i);
  int NumOfFLOBs() const;
  size_t Sizeof() const;
  int Compare(const Attribute*) const;
  bool Adjacent (const Attribute*)const;
  const DBArray<FlobIndex>* getURLS()const;
  const DBArray<FlobIndex>* getMetainfoKeys()const;
  const DBArray<FlobIndex>* getMetainfoContents()const;
  const DBArray<FlobIndex>* getEmbededURLS() const;
  
  bool IsValid() const;
  void CopyFrom(const StandardAttribute *arg);
  size_t HashValue(void) const;



  
 private:
  DateTime lastChange;
  FLOB source;
  DBArray<flobindex> urls;
  DBArray<flobindex> emburls;
  DBArray<flobindex> metainfoKeys;
  DBArray<flobindex> metainfoContents;
  URL sourceURL;

  bool defined;
  int tiefe;

   URL findNextURI(WebLex& lexer, flobindex& i, const string&, URL& url);

  void analyseStructure(WebLex& lexer, int maxdepth, int& depth,
                              AnalyseList& al, int& error, int& symbol);
  bool checkURI(string value,URL& url);
  void getMetaInfos(const string&);
  void filterEmbUrls(URL& u, flobindex& f);
  void getUrls(const string&);
  bool valid;
};


/*
3.2.1 Implementation of Class-Operations of ~HTML~

*/
HTML::HTML(const string& s)
:lastChange(instanttype),source(s.length()+1),
 urls(0), emburls(0),metainfoKeys(0),metainfoContents(0),
 sourceURL("http://"),defined(true),
tiefe(0), valid(true)
{
  __TRACE__
  //cout << "V1" << endl;
  defined = true;
  source.Put(0,s.length()+1,s.c_str());
  //tiefe=0;
  


  //source.Put(0,s.length()+1,s.c_str());
  
  valid=true;
  
  getMetaInfos(s);
  getUrls(s);
  
  __TRACE__
  
  //creates an HTML object without lastChange and sourceURL.
  // If ~isValidHTML~ returns false, the object is not defined.
}

HTML::HTML(const DateTime &d, const string &s, const URL &u)
: lastChange(d),
source(s.length()+1),urls(0),emburls(0),metainfoKeys(0),
metainfoContents(0), sourceURL(u),defined(true),
tiefe(0),valid(true)
{
  __TRACE__
  //cout << "V2" << endl;
  source.Put(0,s.length()+1,s.c_str());
  
  //jps: Only Debug must be removed!!!!!!!!!!!
  //cout << d.ToString() << " , " << u << endl;
  //cout << "|" << s << "|" << endl; 

  valid=true;

  
  // __TRACE__
  getMetaInfos(s);
  // __TRACE__
  getUrls(s);

  __TRACE__
  //creates an HTML object. If ~isValidHTML~ returns false,
  // the object is not defined.
}

HTML::HTML(const HTML& h)
:lastChange(h.getLastModified()),
source(0), urls(0), emburls(0),metainfoKeys(0),
metainfoContents(0), sourceURL(h.getSource()),
defined(h.IsDefined()),tiefe(0),valid(h.IsValid())
{
  __TRACE__
  
  //cout << "V3" << endl;
  const FlobIndex *tmp=0;
  const DBArray<FlobIndex> *tmpArray=0;
  
  int i=0;
//__TRACE__    
  string c = h.getContent();
  source.Resize (c.length() +1 );
  source.Put(0,c.length()+1,c.c_str());
  
//  __TRACE__
  tmpArray=h.getURLS();
  for (i=0; i < tmpArray->Size();i++){
    tmpArray->Get(i,tmp);
    urls.Put(i,  *tmp);
  }

//__TRACE__  
  tmpArray=h.getMetainfoKeys();
  for (i=0; i < tmpArray->Size();i++){
    tmpArray->Get(i,tmp);
    metainfoKeys.Put( i, *tmp);
  }
//__TRACE__  
  tmpArray=h.getMetainfoContents();
  for (i=0; i < tmpArray->Size();i++){
    tmpArray->Get(i,tmp);
    metainfoContents.Put( i, *tmp);
  }
//  __TRACE__
 /*  
  tmpArray=h.getEmbededURLS();
  for (i=0; i < tmpArray->Size();i++){
    tmpArray->Get(i,tmp);
    emburls.Put( i, *tmp);
  }  
 */
//    __TRACE__
}


HTML* HTML::Clone() const
{ 
  __TRACE__
  return new HTML( *this ); 
}

bool HTML::operator== (const HTML& h) const
{
  __TRACE__
  return  (h.getContent() ==  this->getContent() && 
           h.getSource() == this->getSource() && 
           h.getLastModified() == this->getLastModified());
}
  
datetime::DateTime HTML::getLastModified() const
{
  __TRACE__
  return lastChange;
}

/*

returns the source - code of the html object

*/

string HTML::getContent() const
{
  __TRACE__
  if (!defined)
    return "";
  
  const char* s = 0;
  source.Get(0, &s);
  return s;
}

/*
  returns the content of the html - elements

*/

string HTML::getText() const
{
  //returns the content without tags, only text
  
  
  __TRACE__
  if (!valid)
  return "";
  
  
  
  int symbol=0;
  string content;

  WebLex lexer(0);
  content= getContent();
  //char out[content.length()+1];
  string out="";
  stringstream is (getContent());
  
  
  lexer.yyrestart(&is);
  
  lexer.switchStartCond (RELEM_WA);
  symbol = lexer.nextToken();
  
  while (symbol){
    //cout << lexer.getVal() << endl;
    if (symbol == ERROR){
      cout << "Fehler" << endl;
      return "";
    }
    
    if (symbol == CONTENT){
      out += lexer.getVal();
    } else{
      //cout << "Token: " << symbol << ": " << lexer.getVal() << endl;
    }
    
    if (symbol == ELEMENT){
      if (isEqual(lexer.getVal(), "script") || 
          isEqual(lexer.getVal(), "style")){
        symbol = lexer.nextToken();
        while (symbol == CONTENT)
          symbol= lexer.nextToken();
      }
    }
    symbol= lexer.nextToken();
  }
  
  //cout << "*******" << content << endl;
  return out;
}

URL HTML::getSource() const
{
//  __TRACE__
  return sourceURL;
}

ListExpr HTML::ToListExpr(bool typeincluded)const {
 
   __TRACE__
  if (!defined)
    return HTML("").ToListExpr(typeincluded);
  __TRACE__
  Base64 b;
  string content = getContent();
  string textBytes;
  b.encode( content.c_str(), content.size(), textBytes );
  
  ListExpr value = nl->ThreeElemList(
    getLastModified().ToListExpr(true),
    nl->TextAtom(textBytes),
    sourceURL.ToListExpr(true));
  if(typeincluded)
  {
    return nl->TwoElemList(nl->SymbolAtom("html"),value);
  }
  else
    return value;
}

bool HTML::IsDefined() const {
   __TRACE__
    return defined;
}

void HTML::getUrls(const string& content){
  string href;
  WebLex lexer(0);
  stringstream ss (content);
  lexer.yyrestart(&ss);
  flobindex i;
  URL url("");
  
   __TRACE__

  
  findNextURI (lexer, i, content,url);
  


  while (url.IsDefined()){
     __TRACE__
    //cout << "getUrls" << url.getPath() << endl;
    urls.Append (i);
//    filterEmbUrls(url,i); //has errors AB 11.2.07
    //url=findNextURI(lexer, i, content);
    findNextURI (lexer, i, content, url);
  }
  
   __TRACE__
}


/*
  checks' wether the URL u ist a embeded URL
  If so, the flobindex is appendes to emburls

*/
void HTML::filterEmbUrls (URL& u, flobindex& i){
  __TRACE__
  string name = u.getPath();
  //cout << "---" << u.getPath() << endl;
  
  int first =name.rfind(".");
  if (first>0){
    name= name.substr(first +1);
    //cout << name << endl;
    
    if (name == "jpg" || name == "jpeg" || name == "gif" || name == "bmp" || 
        name == "png" || name =="tif")
      emburls.Append(i);
  }
  
}

int HTML::getNumberOfUrls() const
{
  __TRACE__
  return urls.Size();
  //cout << urls.Size() << endl;
}

URL HTML::getUrl( int i) 
{
  __TRACE__
  const flobindex* ind=0;
  string content;
  URL url("");
  if (i < urls.Size()){
    const char* s = 0;
    source.Get(0, &s);
    urls.Get(i, ind);
    string tmp (s+ind->offset, ind->len);
    content= tmp;

    
    if (checkURI( content, url))
      return URL(url);
  }
  
  return URL("");
}

int HTML::getNumberOfEmbUrls() const{
  __TRACE__
  return emburls.Size();
}

URL HTML::getEmbUrl( int i) 
{
  __TRACE__
  const flobindex* ind=0;
  string content;
  URL url("");
  if (i < emburls.Size()){
    const char* s = 0;
    source.Get(0, &s);
    emburls.Get(i, ind);
    string tmp (s+ind->offset, ind->len);
    content= tmp;

    
    if (checkURI( content, url))
      return URL(url);
  }
  
  return URL("");
}

/*
checks, wether the host of getUrl(i) is equal to 
one of hosts in the parameter ~hosts~

*/

URL HTML::getUrlHosts(int i, string hosts, bool& contains){
  
  vector<string> vhosts;
  vector<string>::const_iterator it;
  string host="";
  
  hosts+=",";
  URL url= getUrl (i);
  //cout << "Hosts übergeben: " << hosts << endl;
  if( !hosts.length() )
  {
    contains = true;
    return url;
  }
  
  contains=false;
  if (!url.IsDefined() || !valid)
    return url;
  
  /*for (j=0;j < hosts.length();j++){
    if (isWhite (hosts.at(j)))
      hosts.erase(j,1);
  }*/
  
  SplitString( hosts,",",vhosts,false);
  
  it= vhosts.begin();
  host= url.getHost();
  //cout << "Host enthalten: " << host << vhosts.size() << endl;
  while(it != vhosts.end()){
    //cout << "--- Host: " << host << ", Erlaubt: " << *it << endl;
    if (isEqual(host, *it)){
      //cout << "gleich" << endl;
      contains =true;
      return url;
    }
    
    it++;
  }
  
  return url;
  
}


  
bool HTML::containsURL(const URL *url){
  string href;
  int i=0;

  __TRACE__
  
  while (i < getNumberOfUrls()){
    if (*url == getUrl (i))
      return true;
    
    i++;
  }

  return false;
}

/*
  checks, wether value is a valid URL. Returns true if so, false otherwise

*/
bool HTML::checkURI(string value,URL& url){
  WebLex lexer(0);
  stringstream ss;
  int symbol;
  
//  __TRACE__
  //cout << "Prfe URL " << value << endl;
  url.SetDefined(false);
  
  //check if this is a complete url
  if (URL::urlFromString(value,url))
      return true;
  
  
  
   __TRACE__
  //match a URL
  lexer.switchStartCond (MURI);
  ss << value;
  lexer.yyrestart(&ss);
  symbol= lexer.nextToken();
  
  
  
  // we have the Path from a URL ~value~ and the source URL
  //Now we try to build a valid url with protocol, host and path
   __TRACE__
  string path= getSource().getPath();
  string urlpath=value;
  string myurl="";
  string mypath="";
  int pos=0;

  // 
  pos = urlpath.find("./");
  if (pos == 0){
    //Unterverzeichnis der source url
    pos= path.rfind("/");
    mypath=path.substr(0,pos );
    
    urlpath= urlpath.substr (2);
  } else{
    pos= urlpath.find("/");
    if (pos == 0){
      //im Wurzelverzeichnis des Webservers 
      mypath="";
    } else {
      //Unterverzeichnis der source url
      pos= path.rfind("/");
      mypath= path.substr(0,pos );
    }
  }

  if (urlpath.find("/") == 0){
    urlpath= urlpath.substr(1);
  }

  myurl=urlpath;
  //cout << myurl << " ---  " << mypath << endl;
  while (true){
    pos = myurl.find("../");
    //parent directory
    if (pos == 0){
      myurl= myurl.substr(3);
      pos= mypath.rfind("/");
      //cout << "1:" << mypath << endl;
      if (pos < 0){
        //cout << "error parsing url" << endl;
        return false;
      } else {
        mypath= mypath.substr(0, pos );
        //cout << "2. " << mypath << endl;
      }
    }else {
      pos= myurl.find("/");
      if (pos == -1)
        break;

      mypath= mypath  + "/" + myurl.substr(0,pos);
      myurl= myurl.substr(pos +1);
    }
  }
  //__TRACE__
  url.SetDefined(true);
  url.setProtocol (getSource().getProtocol());
  url.setHost(getSource().getHost());
  
  //cout << "checkuri " << mypath << "  " << myurl;
  url.setPath(mypath + "/" + myurl);
  
  //cout << "Neue URL :" << url.getPath() << endl;    
  //__TRACE__
  return true;
}

/*

in:lexer
in:content (COontent of HTML Object)
out:i (FlobIndex for the URL)
out:url (the found URL Object)


find NextUri in the stream of ~lexer~

*/



URL HTML::findNextURI(WebLex& lexer, flobindex& i, 
                      const string& content, URL& url ){
  string element, value;
  int symbol=0;
  
  //URL url("");
  
//  __TRACE__
  url.SetDefined(false);
  //vector<string> attributes;
  
  //attributes.push_back("src");
  //attributes.push_back("href");
  
  
  
  symbol= lexer.startElement(element);
  while (symbol){
    __TRACE__
    

      
    if (isEqual(element, "img")){
      if (lexer.findAttribute("src",value)){
        if (checkURI(value,url)){
          i= lexer.setPos(value,content);
          return url;
        }
      }
    }
    
    /*if (!isEqual(element,"script")){
      
//      __TRACE__
      if (lexer.findAttribute(attributes,value)){
        __TRACE__
        if(checkURI(value,url)){
          __TRACE__
          i=lexer.setPos(value, content);
          //cout << "StartKopie" << url.getPath() << endl;
          return url;
        }
      }
    }*/

    
//    __TRACE__
    if (lexer.findAttribute("href",value)){
        //__TRACE__
        if(checkURI(value,url)){
          //__TRACE__
          i=lexer.setPos(value, content);
          return url;
        }
      }

    if (isEqual(element,"script")){
      __TRACE__
      //cout << element << endl;
      if (lexer.findAttribute("src",value)){
        if (checkURI(value,url)){
          i= lexer.setPos(value,content);
          
        }
      }
      
//      __TRACE__
      symbol= lexer.nextToken();
      element=lexer.getVal();
      while(symbol == CONTENT){
        symbol= lexer.nextToken();
        element= lexer.getVal();
      }
      //cout << "------------" << lexer.getVal() << symbol << endl;
      if (url.IsDefined())
        return url;
      
    }else{
      __TRACE__
      symbol=lexer.startElement(element);    
    }

  }
  
  return url;
}

int HTML::getNumberOfMetainfos() const
{
   __TRACE__
  //cout << metainfoKeys.Size() << endl;
  return metainfoKeys.Size();
}

string HTML::getMetainfo( int i, string& pContent) const
{
   __TRACE__
  //returns the key of metainfo number ii
  //fills pContent with the content of the metainfo number ii
  
  const flobindex *ind=0;
  const char* content;
  
  source.Get(0, &content);
  
  if (i < metainfoKeys.Size()){
    metainfoContents.Get (i, ind);
    string tmp (content+ind->offset, ind->len);
    pContent= tmp;
    metainfoKeys.Get( i, ind);
    return string (content+ind->offset, ind->len);
  }
  return "";
}

string HTML::getMetaInfo(string name){
  __TRACE__
  int i=0;
  string content;
  for (i=0; i< getNumberOfMetainfos();i++){
    if (isEqual(getMetainfo(i, content),name)){
      return content;
    }
  }
  
  return "";
}


/*

find all Metainfos in ~content~ and append them to
the attributes ~metainfoContents~ and ~metainfoKeys~

*/

void HTML::getMetaInfos(const string& content){
//  __TRACE__
  string attname;
  flobindex ikey, icontent;
  int symbol=0;
  string value("");;
  stringstream ss (content);
  WebLex lexer (&ss);
  vector<string> attributes;
  attributes.push_back("content");
  attributes.push_back("name");
  
  //cout << "getMeta Content " << content << endl;
  
  symbol=lexer.startElement(attname);
//   __TRACE__
  while (symbol){
    //cout << "getMeta Content " << attname << endl;
    if (isEqual (attname, "/head"))
      return;
    if (symbol== EIDENTIFIER && isEqual (attname, "meta")){
//      __TRACE__
      
      string tmp("");
      if (lexer.findAttribute(attributes,value,tmp)){
          //cout << "--" << value << endl;


          if (isEqual(tmp,"name")){
            ikey= lexer.setPos(value, content);
          }else{
            icontent= lexer.setPos(value, content);
          }

          if (lexer.findAttribute(attributes,value,tmp)){

            if (isEqual(tmp,"name")){
              ikey= lexer.setPos(value, content);
              
            }else{
              icontent= lexer.setPos(value, content);
            }
            
            metainfoContents.Append (icontent);
            metainfoKeys.Append (ikey);


        
        
          }
      }
    }
    
  
    if (isEqual(attname,"script")){
      //cout << "******* Treffer **********" << endl;
      lexer.switchStartCond(RSCRIPT);
      symbol= lexer.nextToken();
      attname= lexer.getVal();
      while(symbol == CONTENT){
        symbol= lexer.nextToken();
        attname=lexer.getVal();
      }
    }else{
      symbol=lexer.startElement(attname);    
    }
  }
    

  
}

/*

  return the number of the occurences of the element in this Object

*/
int HTML::getNumberOf(string element){
  __TRACE__
  int count=0;
  string e="";
  int symbol;
  stringstream ss (getContent());
  WebLex lexer (&ss);
  
  //cout << getContent << endl;
  if (!valid)
    return 0;
  
  lexer.switchStartCond(RELEM_WA);
  symbol = lexer.nextToken();
    
  while (symbol){
    e= lexer.getVal();
    
   //read content
    __TRACE__
    lexer.readContent();
    
    if (isEqual (e, element))
      count++;
    
    //next element
    symbol= lexer.nextToken();
  }
  
  return count;  
  
}

/*
  analyse Structure of html object
*/
void HTML::analyseStructure(WebLex& lexer, int maxdepth, int& depth, 
                            AnalyseList& al, int& error, int& symbol){
  
//  __TRACE__
  int sym1=0;
  string element;
  lexer.switchStartCond (RELEM_WA);
  

  //cout << "*****  Rein " << tiefe << " ********* " << endl;
  depth++;
  
  //cout << "nextToken 1" << endl;
  symbol= lexer.nextToken();
  //cout << "analyse: " << symbol << endl;

  
  while (symbol == 10000){
    symbol= lexer.nextToken();
    //cout << "analyse: " << symbol << endl;
  }
  
  
  while (symbol && !error){
    //cout << "TAG Name: " << lexer.getVal() << "  " << symbol <<  endl;
    if (symbol != ELEMENT && symbol !=COMMENT && 
        symbol != ELEMENT_SA && symbol !=ELEMENT_CLOSE){
      error=-1;
      cout << "1 ERROR " << lexer.getVal() << symbol << endl;
      return ;
    }
    
    if (symbol != ELEMENT_CLOSE && lexer.getVal()[0] == '/'){
      cout << " 2 ERROR " << lexer.getVal() << "  " << symbol << endl;
      error=-1;
      return ;
    }
      
    
    element= lexer.getVal();
        
    if (isEqual(element, "/html")){
      symbol=0;
      return;
    }else{
      //cout << "endetest:" << element << endl;
    }
      
    //Read content of current element <tag>content<....
    //  it is maby empty <tag><tag>
    //cout << "nextToken 2" << endl;
    if ((sym1=lexer.readContent()) != CONTENT){
      symbol=sym1;
      if (!symbol)
        return;
      cout << "3 ERROR CONTENT" << lexer.getVal() << symbol << endl;
      error =-1;
      return ;
    }
    
    //cout << "Content " << lexer.getVal() << endl;
    
    if (symbol == ELEMENT_CLOSE){
      //cout << "nextToken 3" << endl;
      symbol=lexer.nextToken();
      //cout << "Element_close " << element << endl;
      element= element.substr (1);

      break;
    }
    
    if (symbol == ELEMENT){
      
      
      //we have to check every single standalone html attribute
      if (isEqual (element,"area") || isEqual (element,"base") || 
          isEqual (element,"basefont") || isEqual (element,"br") ||
          isEqual (element,"col") || isEqual (element,"frame") || 
          isEqual (element,"hr") || isEqual (element,"img") ||
          isEqual (element,"img") || isEqual (element,"input") || 
          isEqual (element,"isindex") || isEqual (element,"link") ||
          isEqual (element,"meta") || isEqual (element,"param") || 
          isEqual (element,"param")){
        
        //cout << "SA Element " << element << endl;
        //cout << "nextToken 4" << endl;
        symbol= lexer.nextToken();
        
        
      }else{
        if ((depth <= maxdepth) ||maxdepth < 0)
          al.push_back ( element );  
        analyseStructure(lexer, maxdepth, depth, al, error, symbol);
        //cout << "Zurck " << symbol << endl;

      }
    } else if (symbol == ELEMENT_SA || symbol == COMMENT){
      //cout << "SA Element " << element << endl;
      if ((depth <= maxdepth) || maxdepth < 0)
        al.push_back ( element );  
      //cout << "nextToken 5" << endl;
      symbol= lexer.nextToken();
    }
    else {
      cout << "5 Error" << element << "  " << symbol << endl;
      error=-1;
      return;
    }
  }

  depth--;
  return;
}





double HTML::similar(HTML *html, int maxdepth, bool respectOrder){
  __TRACE__
  AnalyseList *al1, *al2, *al3, *al4, *al;
  int counter=0;
  int depth=0;
  int error=0;
  int symbol=0;
  AnalyseList::const_iterator it1,it2;
  
  
  if (!valid || !html->IsValid())
    return 0;
  
  al1= new AnalyseList();
  string tmp1=getContent();

  stringstream ss1(tmp1);



  WebLex lexer (&ss1);
  analyseStructure(lexer, maxdepth, depth, *al1, error, symbol);
  


  depth=0;
  symbol=0;
  error=0;
  al2= new AnalyseList();
  string tmp2 = html->getContent();
  stringstream ss2(tmp2);
  lexer.yyrestart(&ss2);
  analyseStructure(lexer, maxdepth, depth, *al2,error, symbol);
  
  if (respectOrder){
    if (al2->size() > al1->size()){
      __TRACE__
      al= al2;
      al2= al1;
      al1= al;
    }  
    
    
    it1= al1->begin();
    it2= al2->begin();

    //cout << al1->size() << "  " << al2->size() << endl;
    
    while ((it1 != al1->end() && it2 !=al2->end())){
      if (isEqual(it1->getElement(), it2->getElement())){
        //cout << "treffer" << it1->getElement() << endl;
        counter++;
        if (!(it1 != al1->end() && it2 !=al2->end())){
          break;
          
        }
        it1++;
        it2++;
      }else {
        
        
        if (!al1->find( it1, it2->getElement())){
          //cout << "nicht gefunden " << it2->getElement() << endl;
          if (!(it1 != al1->end() && it2 !=al2->end())){
            break;
            
          }
          it2++;
        }else {
          if (!(it1 != al1->end() && it2 !=al2->end())){
            break;
            
          }
          it1++;
          //cout << "gefunden " << it2->getElement() << endl;
        }
      }
    }

    //cout << "-------" << counter << endl;
    if ((double) al1->size() == 0)
      return 0;
    return (double) counter / (double) al1->size();  
    
  }

  al3= new AnalyseList();
  al4= new AnalyseList();
  it1 = al1->begin();
  it2 = al2->begin();
  
  
  while (it1 != al1->end()){
    al3->add(it1->getElement());
    it1++;
  }
  
  
  while (it2 != al2->end()){
    al4->add(it2->getElement());
    it2++;
  }
  
  
  al1=al3;
  al2=al4;
  
  if (al2->size() > al1->size()){
    __TRACE__
    al= al2;
    al2= al1;
    al1= al;
  }

  it1= al1->begin();
  it2= al2->begin();  
  
  //cout << al1->size() << "  " << al2->size() << endl;

  while ((it1 != al1->end() && it2 !=al2->end())){
    if (isEqual(it1->getElement(), it2->getElement())){
      //cout << "treffer" << it1->getElement() << endl;
      counter++;
      if (!(it1 != al1->end() && it2 !=al2->end())){
        break;
        
      }
      it1++;
      it2++;
    }else {
      
      
      if (!al1->find( it1, it2->getElement())){
        //cout << "nicht gefunden " << it2->getElement() << endl;
        if (!(it1 != al1->end() && it2 !=al2->end())){
          break;
          
        }
        it2++;
      }else {
        if (!(it1 != al1->end() && it2 !=al2->end())){
          break;
          
        }
        it1++;
        //cout << "gefunden " << it2->getElement() << endl;
      }
    }
  }
  
  __TRACE__
  
  if (al1->size() == 0)
    return (double) 0;
  
  return (double) counter / (double) al1->size();  

}

void HTML::Set(const HTML &h)
{
    const FlobIndex *tmp=0;
  const DBArray<FlobIndex> *tmpArray=0;
  
  int i=0;
  
  

  __TRACE__
  if (!h.IsDefined())
    return;
  valid= h.IsValid();
  defined=true;
  DateTime d = h.getLastModified();
  lastChange.SetType(instanttype);
  lastChange.Set(d.GetYear(),d.GetMonth(), d.GetGregDay(), d.GetHour(), 
                 d.GetMinute(), d.GetSecond(),d.GetMillisecond());

  URL u(h.getSource());
  sourceURL.Set(true,u);
  string s = h.getContent();
  source.Resize( s.length() + 1 );
  source.Put( 0, s.length() + 1, s.c_str() );
  
  
  string c = h.getContent();
  source.Resize (c.length() +1 );
  source.Put(0,c.length()+1,c.c_str());
  

  tmpArray=h.getURLS();
  for (i=0; i < tmpArray->Size();i++){
    tmpArray->Get(i,tmp);
    urls.Put(i,  *tmp);
  }
    
  tmpArray=h.getMetainfoKeys();
  for (i=0; i < tmpArray->Size();i++){
    tmpArray->Get(i,tmp);
    metainfoKeys.Put( i, *tmp);
  }
  
  tmpArray=h.getMetainfoContents();
  for (i=0; i < tmpArray->Size();i++){
    tmpArray->Get(i,tmp);
    metainfoContents.Put( i, *tmp);
  }
  
  
  
}

  
int HTML::NumOfFLOBs() const{
  __TRACE__
    return 7;
}

FLOB *HTML::GetFLOB(const int i){
//  __TRACE__
  //assert (i < NumOfFLOBs());

  if (i==0)
    return &source;
  if (i==1)
    return &urls;
  
  if (i==2)
    return &metainfoKeys;
  
  if (i==3)
    return &metainfoContents;
  
  if (i==4)
    return &emburls;
    
  if (i==5)
    return sourceURL.GetFLOB(0);
  
  if (i==6)
    return sourceURL.GetFLOB(1);
  
  return NULL;
}

size_t HTML::Sizeof() const{
  return sizeof(HTML);
}

int HTML::Compare(const Attribute*) const{
  return 0;
}

bool HTML::Adjacent (const Attribute*)const{
  return 0;
}

void HTML::SetDefined(bool d) {
  __TRACE__
  defined=d;
}

const DBArray<FlobIndex>* HTML::getURLS() const{
  return &urls;
  
}

const DBArray<FlobIndex>* HTML::getMetainfoKeys()const{
  return &metainfoKeys;
}

const DBArray<FlobIndex>* HTML::getMetainfoContents() const{
  return &metainfoContents;
}

bool HTML::IsValid() const{
  return valid;
}

void HTML::CopyFrom(const StandardAttribute* right) 
{
  __TRACE__
  const HTML *r = (const HTML *)right;
  lastChange = r->getLastModified();
  source.Resize( r->source.Size() );
  const char *bin;
  r->source.Get( 0, &bin );
  source.Put( 0, r->source.Size(), bin );
  sourceURL.setProtocol( r->getSource().getProtocol());
  sourceURL.setHost( r->getSource().getHost());
  sourceURL.setPath( r->getSource().getPath());
  defined = r->IsDefined();
  valid=true;
  tiefe=0;
  urls.Clear();
  metainfoKeys.Clear();
  metainfoContents.Clear();
  getMetaInfos(bin);
  getUrls(bin);
}

size_t HTML::HashValue(void) const
{
  return 0;
}


const DBArray<FlobIndex>* HTML::getEmbededURLS() const{
  return &emburls;
}

/*

3.3 Class ~Page~

----
Example to create an object:
let page1 = [const page value ((html ((instant (10 10 2006 10 27 18)) <file>/home/sopra/secondo/Algebras/Web/bilder.htm</file---> (url ("http" <text>www.myimages.de</text---> <text>/</text---> )))) ((url ("http" <text>Garten-1.jpg</text---> <text>/</text---> )) <file>/home/sopra/secondo/Algebras/Web/Garten-1.jpg</file---> "image/jpeg")( (url ("http" <text>Garten-2.jpg</text---> <text>/</text---> )) <file>/home/sopra/secondo/Algebras/Web/Garten-2.jpg</file---> "image/jpeg"))]
----

*/



class Page : public HTML
{
  public:
    Page(){}
    ~Page(){}
    Page(const string &s);
    Page(const HTML &);
    Page(const Page &);
    Page(const URL &url, string &mime, string &binFile, DateTime &dt);
    
    bool operator== (const Page& h) const;
    HTML extractHTML();
    int numOfFiles() const;
    URL getUrl(int i) const;
    string getText( int i) const;
    string getMime( int i) const;
    void addEmbObject(const URL &u, const string &mime, const string &s);

    bool IsDefined() const;
    void SetDefined(bool d) ;
    FLOB *GetFLOB(const int i);
    int NumOfFLOBs() const;
    size_t SizeOf() const;
    int Compare(const Attribute*) const;
    bool Adjacent (const Attribute*)const;
    void CopyFrom(const StandardAttribute *arg);
    Page* Clone() const;


  private:
  
    /*Class ~HTTPSocket~
     
      This Page classes inner class is designed to capsulate all details
      of the socket´s implementation and the page request, depending on the 
      http protocol. It is an inner private class because, up to now, the 
      page class is the only object connecting to the web. 
    
     */
    class HTTPSocket
    {
      public:
        enum HTTPProtocol {HTTP_10, HTTP_11};
        HTTPSocket(string webAddr, string filePath, HTTPProtocol proto,
                   string port);
        inline const string getServerAddress() {return WebAddr;}
        
        //returns the string represantation of an valid http get request
        const string getGetRequest();
        inline Socket * getSocket() {return s;} 
        bool parseHTTPResponse(vector<string> serverResponse);
        inline string getContentType() {return contentType;}
        inline int getContentLength() {return contentLength;}
        inline DateTime getLastModified() {return lastModified;}
        inline bool getSuccessResponded() {return successResponded;}
          inline bool Close() { return s->Close();}
        inline bool getChunked(){ return isChunked;}
      private:
        string WebAddr;
        string FilePath;
        HTTPProtocol Protocol;
        string Port;

        string contentType;
        int contentLength;
        DateTime lastModified;
        DateTime responseDate;
        bool successResponded; 
        bool isChunked;

        Socket *s;
        bool setLastModified(string s);
        bool setResponseDate(string s);
        DateTime setDateTime(string s);
        string getMonthNumFromName(string monthName);
    };    
  public:  
    static string getFromWeb(URL url, string &mime, bool &MimeIsEqual,  
                             DateTime &dt, bool onlyHtml = false);
  private:
    struct FLOBIndex
    {
      int offset;
      int len;
    };
    int numOfEmbeddedObjects;
    DBArray<FLOBIndex> embUrlIds;
    FLOB embUrls;
    DBArray<FLOBIndex> binIDs;
    FLOB binFiles;
    DBArray<FLOBIndex> mimeIDs;
    FLOB mimeTypes;

    bool allocateOneElem(int BytesOfData, int BytesOfURL, int BytesOfMime);
    bool allocateSpaceInArray(DBArray<FLOBIndex> *dba, int numOfBytes);
    URL getURLFromString(string &s) const;
    bool checkEmbUrl(URL &u);
    static const int MAXBUFFERSIZE = 1000000;
};

/********************OVERWRITING ATTRIBUTE************************/

bool Page::IsDefined() const
{
  return HTML::IsDefined();
}

void Page::SetDefined(bool d)
{
  HTML::SetDefined(d);
}

FLOB* Page::GetFLOB(const int i)
{
  #ifdef _DEBUG_JPS
    cout << "FLOB* Page::GetFLOB(const int i):"  << i << endl;
    cout << HTML::NumOfFLOBs() << endl;
    cout << NumOfFLOBs() << endl;
   #endif
  if (i < (NumOfFLOBs() - HTML::NumOfFLOBs())){
    switch (i) 
    {
      case 0: return &embUrlIds;
      case 1: return &embUrls;
      case 2: return &binIDs;
      case 3: return &binFiles;
      case 4: return &mimeIDs;
      case 5: return &mimeTypes;
      default: return NULL;
    }
  }
  if (i < NumOfFLOBs()){
//    __TRACE__
    //cout << " > "<<(i - (NumOfFLOBs() - HTML::NumOfFLOBs())) << endl;
    return HTML::GetFLOB(i - (NumOfFLOBs() - HTML::NumOfFLOBs()));
  }else{
    __TRACE__
    return NULL;
  }
}

int Page::NumOfFLOBs() const
{
  __TRACE__
  return 6 + HTML::NumOfFLOBs();
}

size_t Page::SizeOf() const
{
  return sizeof(Page);
}

int Page::Compare(const Attribute*) const
{
  return 0;
}

bool Page::Adjacent (const Attribute*)const
{
  return false;
}

Page* Page::Clone() const
{ 
  __TRACE__
  return new Page( *this ); 
}

void Page::CopyFrom(const StandardAttribute* right) 
{
  __TRACE__
  const Page *r = (const Page *)right;
  HTML::CopyFrom(right);
  
  numOfEmbeddedObjects = 0;
  for( int ii = 0; ii < r->numOfFiles(); ++ii)
  {
    addEmbObject(r->getUrl(ii), r->getMime(ii), r->getText(ii));
  }
}

/*
3.2.1 Implementation of Class-Operations of ~Page~

*/
Page::Page(const string &s)
: HTML(s), numOfEmbeddedObjects(0), embUrlIds(0), embUrls(0), 
  binIDs(0), binFiles(0), mimeIDs(0), mimeTypes(0)
{
    #ifdef _DEBUG_JPS_3
  cout << "Page::Page(const string &s)" << endl;
  #endif
  __TRACE__
}
 
Page::Page(const HTML &h)
: HTML(h), numOfEmbeddedObjects(0), embUrlIds(0), embUrls(0), 
  binIDs(0), binFiles(0), mimeIDs(0), mimeTypes(0)
{
  //NOT USED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  #ifdef _DEBUG_JPS_3
  cout << "Page::Page(const HTML &h)" << endl;
  #endif
  //generate a page object without emb.urls
  //the size of the emb obj. has to set to 0
 __TRACE__
}

Page::Page(const Page &p)
: HTML(p), numOfEmbeddedObjects(0), embUrlIds(0), embUrls(0), 
 binIDs(0), binFiles(0), mimeIDs(0), mimeTypes(0)
{
 __TRACE__
  for( int ii = 0; ii < p.numOfFiles(); ++ii)
  {
    addEmbObject(p.getUrl(ii), p.getMime(ii), p.getText(ii));
  }
}

Page::Page(const URL &url, string &mime, string &binFile, DateTime &dt) 
: HTML(dt, binFile, url), 
  numOfEmbeddedObjects(0), embUrlIds(0), embUrls(0), 
  binIDs(0), binFiles(0), mimeIDs(0), mimeTypes(0)
{
  __TRACE__
  #ifdef _DEBUG_JPS
    cout << "Page::Page(const URL &url, string &mime,"
            " string &binFile, DateTime &dt) "  
         << HTML::getNumberOfUrls() << endl;
    #endif
  for (int i= 0; i < HTML::getNumberOfUrls(); i++)
  {
    #ifdef _DEBUG_JPS
    cout << "Page::Page(const URL &url, string &mime,"
            " string &binFile, DateTime &dt) " << i<< endl;
    #endif
    URL embUrl(HTML::getUrl(i));//getEmbUrl(i);
    if( checkEmbUrl(embUrl) )
    {
      DateTime dt;
      string theMime;
      bool mustBeEqual = false;
      if (embUrl.getHost() != "error")
      {
      
      string embCont = getFromWeb(embUrl, theMime, mustBeEqual, dt);
      addEmbObject(embUrl, theMime, embCont);
      }
    }
  }
}

bool Page::checkEmbUrl(URL &u)
{
  string filename = u.getPath();
  int first =filename.rfind(".");
  if (first>0){
    string name = filename.substr(first +1);
    
    if (name == "jpg" || name == "jpeg" || name == "gif" || 
        name == "bmp" || name == "png" || name =="tif"){
      return true;
    }
  }
  return false;
}

bool Page::operator== (const Page& h) const
{
  __TRACE__
  if (this->numOfFiles() == h.numOfFiles()) 
  {
    for (int i = 0; i < this->numOfFiles(); i++)
    {
      Page &p = const_cast<Page&>(h);
      Page *self = const_cast<Page*>(this);
      if (!(self->getUrl(i) == p.getUrl(i))) return false;
      if (!(self->getMime(i) == p.getMime(i))) return false;
      if (!(self->getText(i) == p.getText(i))) return false;
    }
    return true;
  }
  return false;
}

HTML Page::extractHTML()
{
  __TRACE__
  return *this;
}

int Page::numOfFiles() const
{
  __TRACE__
  #ifdef _DEBUG_JPS_3
  cout << "Page::numOfFiles()" << numOfEmbeddedObjects <<endl;
  #endif
  return numOfEmbeddedObjects;
}

URL Page::getUrl(int i) const
{
  __TRACE__
  if(i < numOfEmbeddedObjects)
  {
    //Get the right url flobindex..
    const FLOBIndex *getThisUrl;
    embUrlIds.Get(i, getThisUrl);
    #ifdef _DEBUG_JPS
    //cout << "getUrl: " << (*getThisUrl).offset << endl;
    #endif
    //..and get the url..
    const char* c;
    embUrls.Get((*getThisUrl).offset, &c);
    string result(c);
    #ifdef _DEBUG_JPS_3
    //cout <<"getUrl: >1ind:" << i << " >2url: " << result << 
     // " >3offset: " << (*getThisUrl).offset <<endl;
    #endif
    return getURLFromString(result);
  }
  return URL("http", "error", "error"); //TODO  Handle this!
}




string Page::getText( int i) const
{
  __TRACE__
  if(i < numOfEmbeddedObjects)
  {
    //Get the right bin index..
    const FLOBIndex *getThisBin;
    binIDs.Get(i, getThisBin);
    
    //..and get the bin data..
    const char* c;
    binFiles.Get((*getThisBin).offset, &c);
    string result(c);
    #ifdef _DEBUG_JPS_3
    //cout <<"getMime: >1ind:" << i << " >2mime: " <<  
      //" >3offset: " << (*getThisMime).offset <<endl;
    #endif
    //result.erase((*getThisMime).len, result.size());
    //erase not needed cause trailing zero was saved
    return result;
  }
  return "Error Page::GetText wrong Index!"; //TODO Handle this!
}

string Page::getMime( int i) const
{
  __TRACE__
  if(i < numOfEmbeddedObjects)
  {
    //Get the right bin index..
    const FLOBIndex *getThisMime;
    mimeIDs.Get(i, getThisMime);
    
    //..and get the bin data..
    const char* c;
    mimeTypes.Get((*getThisMime).offset, &c);
    string result(c);
    #ifdef _DEBUG_JPS_3
    //cout <<"getMime: >1ind:" << i << " >2mime: " << result << 
      //" >3offset: " << (*getThisMime).offset <<endl;
    #endif
    //result.erase((*getThisMime).len, result.size());
    //erase not needed cause trailing zero was saved
    return result;
  }
  return "Error Page::GetMime wrong Index!"; //TODO Handle this!
}

//Stores embedded object, containing an url, the binaries and the mime-type
void Page::addEmbObject(const URL &u, const string &mime, const string &s)
{
  __TRACE__
  
  //If the new object is valdid..
  if (u.IsDefined() && (s.size() > 0) && (mime.size() > 0))
  {
    //Create an easy to use string represantation of the url
    string s_url = u.getProtocol() + "://" + u.getHost() + u.getPath();
    
    if (allocateOneElem(s.size() +1, s_url.size()+1, mime.size()+1))
    {
      /******************URL**********************/
      const FLOBIndex *insertUrlHere;
      embUrlIds.Get(numOfEmbeddedObjects - 1, insertUrlHere);
      embUrls.Put((*insertUrlHere).offset, 
                  (*insertUrlHere).len + 1, s_url.c_str());
      
      /******************MIME**********************/
      const FLOBIndex *insertMimeHere;
      mimeIDs.Get(numOfEmbeddedObjects - 1, insertMimeHere);
      mimeTypes.Put((*insertMimeHere).offset, 
                    (*insertMimeHere).len + 1, mime.c_str());
      
      /******************BINARY**********************/
      const FLOBIndex *insertBinHere;
      binIDs.Get(numOfEmbeddedObjects - 1, insertBinHere);
      binFiles.Put((*insertBinHere).offset, 
                   (*insertBinHere).len + 1, s.c_str());
    }
  }
}

bool Page::allocateOneElem(int BytesOfData, int BytesOfURL, int BytesOfMime)
{
  //Inc the number of embedded objects
  __TRACE__
  ++numOfEmbeddedObjects;
  
  //Prepare the bin and url DBArrays to take the new object..
  __TRACE__
  if (allocateSpaceInArray(&binIDs, BytesOfData)
    && allocateSpaceInArray(&embUrlIds, BytesOfURL)
    && allocateSpaceInArray(&mimeIDs, BytesOfMime))
  {
    //.. and allocate the right amount of memory in the flobs!
    const FLOBIndex *resizeUrlIndex;
    embUrlIds.Get(numOfEmbeddedObjects - 1, resizeUrlIndex);
    embUrls.Resize(embUrls.Size() + (*resizeUrlIndex).len + 1);

    const FLOBIndex *resizeBinIndex;
    binIDs.Get(numOfEmbeddedObjects - 1, resizeBinIndex);
    binFiles.Resize(binFiles.Size() + (*resizeBinIndex).len + 1);
    
    const FLOBIndex *resizeMimeIndex;
    mimeIDs.Get(numOfEmbeddedObjects - 1, resizeMimeIndex);
    mimeTypes.Resize(mimeTypes.Size() + (*resizeMimeIndex).len + 1);
    
    return true;
  }
  
  //Something went wrong - no element can be added (should not occur)!
  --numOfEmbeddedObjects;
  return false;
}


bool Page::allocateSpaceInArray(DBArray<FLOBIndex> *dba, int numOfBytes)
{
  //Get the index and offset of the previous element..
  __TRACE__
  FLOBIndex pIndex;
  if (numOfEmbeddedObjects > 1)
  {
  __TRACE__
    const FLOBIndex *prevIndex;  
    dba->Get(numOfEmbeddedObjects - 2, prevIndex);
    pIndex.offset = (*prevIndex).offset;
    pIndex.len = (*prevIndex).len;
  }
  
  //..or set index and length to 0 if the element is the first!
  else
  {
  __TRACE__
    pIndex.offset = 0;
    pIndex.len = 0;
  }
  
  //Now we can calculate the new offset and length..
  __TRACE__
  FLOBIndex newIndex;
  newIndex.offset = pIndex.offset + pIndex.len;
  newIndex.len = numOfBytes;
  
  //..and append it to the DBArray!
  dba->Append(newIndex);
  __TRACE__
  return true;
}

URL Page::getURLFromString(string &s) const
{
  //This method expects the following format:
  //<protocol>://<host>/<path>
  int pos1 = s.find("://", 1);
  if (pos1 != (int)string::npos)
  {
    string s_prot(""), s_myHost(""), s_path("");
    s_prot.append(s, 0, pos1);
    int pos2 = s.find("/", pos1 + 3);
    if (pos2 != (int)string::npos)
    {
      s_myHost.append(s, pos1+3, pos2 - (pos1 + 3));
      s_path.append(s, pos2, s.size());
    }
    else s_myHost.append(s, pos1+3, s.size());
    return URL(s_prot, s_myHost, s_path);
  }
  return *(new URL());
}


/*
3.2.1.1 If the Page as HTML Instance is not defined and the content type is text/html,
the data will be used to fill the instance as html object. Elsewise everything is interpreted as an embedded object of the page instance itself and so it is added as an embedded object.
TODO: The return type must be defined - it will not be a string!!!!!

*/
string Page::getFromWeb(URL url, string &mime, bool &MimeIsEqual, 
                       DateTime &dt, bool onlyHtml)
{
  __TRACE__

  //Set the HTTP Protocol
  HTTPSocket::HTTPProtocol httpProt;
  httpProt = HTTPSocket::HTTP_11;
  
  //Get an Instance of the HTTPSocket class..
  HTTPSocket httpSock(url.getHost(), url.getPath(), httpProt, "80"); 
  //TODO: only http supported!
  
  //..and use the os independent socket! 
  Socket *s = httpSock.getSocket();
  
  //Get the corresponding http GET request as a string.. 
  string req = httpSock.getGetRequest();
  string result("");
  //cout << "http request: " << req << " , " << req.size() << endl;

  if (s->IsOk())
  {

  //..and write it to the socket!
  iostream& io = s->GetSocketStream();
  io << req << endl;
  
  string line("");
  bool readyForBinData = false;
  vector<string> serverResponse;
  int size = 0;
  int packetsize = 0;
  char byte = 0x00;
  
  while(s->IsOk())
  {
    if (!readyForBinData) //Server http response not completly received yet..
    {
      getline(io,line); 
//      cout << "Line: " << line << endl;
      //..response finalized.. 
      if (line.find("\r") == 0) //..parse it!
      { 
        readyForBinData = httpSock.parseHTTPResponse(serverResponse);
        if (!readyForBinData)
        {
          result = "not ready for response";
          mime = "error";
          Base64 b;
          string binBytes;
          b.encode( result.c_str(), result.size(), binBytes );
          httpSock.Close();
          return binBytes;
        }
        if (mime.size() > 0) //stops and returns false if different mime types
        {
          if((mime.find(httpSock.getContentType(), 0) == string::npos))
          {
            if (MimeIsEqual)
            {
              MimeIsEqual = false;
              httpSock.Close();
              return "";
            }
            MimeIsEqual = false;
          }
        }
        if( onlyHtml )
        {
          mime = httpSock.getContentType();
          if((mime.find("html") == string::npos)){
            MimeIsEqual = false;
            httpSock.Close();
            return "";
          }
          onlyHtml = false;
        }
        if( !httpSock.getChunked())
        {
          result.reserve(httpSock.getContentLength()+1);
        }
      }
      else //..append the line to the server´s response!
      {
        serverResponse.push_back(line);
      }
    }
    else //..receive the binary data!
    {
    //  if (size%1000 == 0) cout << "1000 Zeichen gelesen!" << endl;
      if(httpSock.getChunked() && packetsize<=0)
      {
        getline(io,line); 
//        cout << line << endl;
        if(line.length()>1)  //perhaps empty line
        {
          //files come in packets of n-bytes
          packetsize = (int)strtol(line.c_str(),NULL,16);
//          cout << "Line Bytes: " << packetsize << endl;
          if(!packetsize){break;}
          result.reserve(result.size() + packetsize);
        }
      }
      else
      {
        io.get(byte);
        if (true)//(s->Read(&byte, 1, 1, 1) > 0)
        {
          result += byte;
          size++;
          if(httpSock.getChunked()) --packetsize;
        }
        else 
        {
          //cout << "TIMEOUT nach " << size -1 << " Zeichen!" << endl;
          httpSock.Close();
          break;
        }
        if ((httpSock.getContentLength() > 0) && 
          (size >= httpSock.getContentLength())) {break;} 
      }
    }
   }
  mime = httpSock.getContentType();
  dt = httpSock.getLastModified();
  httpSock.Close();
  __TRACE__
  }
  MimeIsEqual = false;
  if( mime.find("html") != string::npos)
  {
    MimeIsEqual = true;
  }
  if( !MimeIsEqual )
  {
    //binary data encode base64
    if( !result.size() )
    {
      result = "not found";
      mime = "error";
    }
    Base64 b;
    string binBytes;
    b.encode( result.c_str(), result.size(), binBytes );
    return binBytes;
  }
  else { return result;}
}


/*
3.2.1 Implementation of Class-Operations of ~HTTPSocket~ - private inner class of Page

*/

/*
3.2.1.1 Allocates an os dependent socket and offers an instance of
abstract Socket type, hiding the os dependancy.

*/
Page::HTTPSocket::HTTPSocket(string webAddr, string filePath, 
                            HTTPProtocol proto, string port):
  WebAddr(webAddr), FilePath(filePath), Protocol(proto), 
          Port(port), contentType(""), contentLength(-1), 
          successResponded(false), isChunked(false)
{
  lastModified.SetType(instanttype);
  responseDate.SetType(instanttype);  
  s = Socket::Connect(webAddr , port);
}

/*
3.2.1.2 Returns the http get request as const string.

*/
const string Page::HTTPSocket::getGetRequest() 
{
  string result("");
  result += "GET " + FilePath;
  (Protocol == HTTP_10) ? result += " HTTP/1.0" : result += " HTTP/1.1";
  result += "\r\nHost: " + WebAddr + ":" + Port + "\r\n";
  return result;
}

/*
3.2.1.3 Extracts the relevant items out of the strings given
by the vector. Will return true if there is no error transmitted 
by the server.

Example:
HTTP/1.1 200 OK
Server: Apache/1.3.29 (Unix) PHP/4.3.4
Content-Length: (Größe von infotext.html in Byte)
Last-Modified: Sat, 28 Oct 2006 18:40:44 GMT
Content-Language: de
Content-Type: text/html
Connection: close

*/
bool Page::HTTPSocket::parseHTTPResponse(vector<string> serverResponse)
{
  //cout << "serverresponse:" << endl;
  bool gotLastMod = false;
  bool gotDate = false;
//  bool isChunked = false;

  for (vector<string>::iterator iter = serverResponse.begin();
    iter != serverResponse.end(); iter++)
  {
    //cout << (*iter) << endl;
    //Protocol and error code..
    if ((*iter).find("HTTP/1.0", 0) != string::npos)
    { 
      #ifdef _DEBUG_JPS
      cout << "found HTTP/1.0 " << endl;
      #endif
    }
 
    else if ((*iter).find("HTTP/1.1", 0) != string::npos)
    { 
      #ifdef _DEBUG_JPS
      cout << "found HTTP/1.1 " << endl; 
      #endif
    }

    if (((*iter).find("200", 0) != string::npos) &&
      ((*iter).find("OK", 0) != string::npos))
    {
      successResponded = true;
      #ifdef _DEBUG_JPS
      cout << "success " << endl;
      #endif
    }

    else if ((*iter).find("Content-Length:", 0) != string::npos)
    {
      int pos = (*iter).find(":", 14);
      if ((pos != (int)string::npos) && (pos < (int)((*iter).size() + 1)))
      {
        string numStr("");
        numStr.assign((*iter), pos + 2, (*iter).size() - pos + 2);
        contentLength = strtol(numStr.c_str(), 0, 10);
        #ifdef _DEBUG_JPS
        cout << "contentLength: " << contentLength << endl;
        #endif
      }
    }

    else if ((*iter).find("Transfer-Encoding: chunked", 0) != string::npos)
    {
      isChunked = true;
      contentLength = -1;
      #ifdef _DEBUG_JPS
      cout << "CHUNKED: contentLength: " << contentLength << endl;
      #endif
    }

    else if ((*iter).find("Content-Type:", 0) != string::npos)
    {
      if ((*iter).find("text/html", 13) != string::npos)
      { 
        contentType = "text/html";
        #ifdef _DEBUG_JPS
        cout << "contentType = text/html" << endl; 
        #endif
      }

      else //save the Content Type without deeper interpretation!
      {
        contentType.assign((*iter), 14, (*iter).size() - 14);
      }

    }
    else if ((*iter).find("Connection:", 0) != string::npos)
    { //TODO! 
      if ((*iter).find("close", 11) != string::npos)
      {}
      
      else if ((*iter).find("keep-alive", 11) != string::npos)
      {}

    }

    else if ((*iter).find("Last-Modified: ", 0) != string::npos)
    {
      gotLastMod = setLastModified(*iter);
    }

    else if ((*iter).find("Date: ", 0) != string::npos)
    {
      gotDate = setResponseDate(*iter);
    }

  } 
  if (successResponded && ((contentType.size() > 0) || isChunked) && gotDate)
  {
    if (!gotLastMod) lastModified = responseDate;
    __TRACE__
    #ifdef _DEBUG_JPS
    cout << "parseHTTPResponse E N D E true!" << endl; 
    #endif
    //cout << "serverresponse ende - true:" << endl;
    return true;
  }
  #ifdef _DEBUG_JPS
  cout << "parseHTTPResponse E N D E false!" << endl; 
  #endif
  //cout << "serverresponse ende - false:" << endl;
  return false;
}

bool Page::HTTPSocket::setResponseDate(string s)
{
  responseDate = setDateTime(s);
  #ifdef _DEBUG_JPS
    cout << "responseDate: " << responseDate.ToString() << endl;
  #endif
  return true;
}

DateTime Page::HTTPSocket::setDateTime(string s)
{
  /*Convert DayName, day monthName year[4 nums] hh:mm:ss GMT to 
    YEAR-MONTH-DAY-HOUR:MIN:SECOND to store it as an DateTime instance!
  */
  DateTime result;
  result.SetType(instanttype);
  int pos = s.find(",", 0);
  int gmtPos = s.find("GMT", 0);
  int dateLength = gmtPos - pos - 3;
  string dtStr("");
  dtStr.assign(s, pos + 2, dateLength);
  #ifdef _DEBUG_JPS_4
  cout << "dtStr.assign: |" << dtStr << "|" << endl;
  #endif

  //will be used to create a DateTime string!
  string dtFormattedString("");

  //..3rd the year..
  string dtElem = "";
  dtElem.assign(dtStr, 7, 4);
  dtFormattedString += dtElem + "-";
  #ifdef _DEBUG_JPS_4
  cout << "year: |" << dtElem << "|" << endl; 
  #endif

  //..2nd the month..
  dtElem = "";
  dtElem = getMonthNumFromName(dtStr);
  dtFormattedString += dtElem + "-";
  #ifdef _DEBUG_JPS_4
  cout << "month: |" << dtElem << "|" << endl; 
  #endif

  //1st store the day..
  dtElem = "";
  dtElem.assign(dtStr, 0, 2);
  dtFormattedString += dtElem + "-";
  #ifdef _DEBUG_JPS_4
  cout << "day: |" << dtElem << "|" << endl; 
  #endif
  
  //..4th the hour::minutes:seconds
  dtElem = "";
  dtElem.assign(dtStr, 12, 8);
  dtFormattedString += dtElem;
  result.ReadFrom(dtFormattedString);
  #ifdef _DEBUG_JPS_4
  cout << "h:m:s: |" << dtElem << "|" << endl; 
  cout << "secondo datetime: |" << dtFormattedString << "|" << endl; 
  cout << "dateTime: " << result.ToString() << endl;
  #endif
  return result;
}

bool Page::HTTPSocket::setLastModified(string s)
{
  lastModified = setDateTime(s);
  #ifdef _DEBUG_JPS
  cout << "lastModified: " << lastModified.ToString() << endl;
  #endif
  return true;
}

string Page::HTTPSocket::getMonthNumFromName(string monthName)
{
  if (monthName.find("Jan", 0) != std::string::npos) return "1";
  else if (monthName.find("Feb", 0) != std::string::npos) return "2";
  else if (monthName.find("Mar", 0) != std::string::npos) return "3";
  else if (monthName.find("Apr", 0) != std::string::npos) return "4";
  else if (monthName.find("May", 0) != std::string::npos) return "5";
  else if (monthName.find("Jun", 0) != std::string::npos) return "6";
  else if (monthName.find("Jul", 0) != std::string::npos) return "7";
  else if (monthName.find("Aug", 0) != std::string::npos) return "8";
  else if (monthName.find("Sep", 0) != std::string::npos) return "9";
  else if (monthName.find("Oct", 0) != std::string::npos) return "10";
  else if (monthName.find("Nov", 0) != std::string::npos) return "11";
  else if (monthName.find("Dec", 0) != std::string::npos) return "12";
  return "";
}

   
/*
4 In/Out, Checking Functions and Type Construction of URL

4.1 List Representation and In/Out Functions of ~URL~

Example: The list representation of a URL is

STRING First, text Second, text Third
where First Protocoll i.e. http or ftp
      Second Host i.e "//www.google.de"
      Third Path i.e. /


*/

ListExpr
OutURL( ListExpr typeInfo, Word value )
{
  __TRACE__
//  cout << *((URL*)(value.addr)) << endl;
  return ((URL*)(value.addr))->ToListExpr(false);
}

Word
InURL( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  __TRACE__
  if ( nl->ListLength( instance ) == 3 )
  {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);
    ListExpr Third = nl->Third(instance);

    if ( nl->IsAtom(First) && nl->AtomType(First) == StringType
      && nl->IsAtom(Second) && nl->AtomType(Second) == TextType
      && nl->IsAtom(Third) && nl->AtomType(Third) == TextType )
    {
      string prot = nl->StringValue(First);
      string host = nl->Text2String(Second);
      string path = nl->Text2String(Third);
      {
      if( host.length() >= 2 && host[0] == '/' && host[1] == '/')
      {
        host = host.c_str() + 2;
      }
      correct = true;
      URL* newUrl = new URL(prot, host, path);
      return SetWord(newUrl);
      }
    }
   else
   {
    if( !nl->IsAtom(First)) ErrorReporter::ReportError("First not an atom");
    if( !nl->IsAtom(Second)) ErrorReporter::ReportError("Second not an atom");
    if( !nl->IsAtom(Third)) ErrorReporter::ReportError("Third not an atom");
    if (!(nl->AtomType(First) == StringType))
        ErrorReporter::ReportError("First not a StringType");
    if (!(nl->AtomType(Second) == TextType))
        ErrorReporter::ReportError("Second not a TextType");
    if (!(nl->AtomType(Third) == TextType))
        ErrorReporter::ReportError("Third not a TextType");
    correct = false;
    return SetWord(Address(0));
   }
  }
  ErrorReporter::ReportError("Wrong number of"
                             " params, expecting protocol,host,path");
  correct = false;
  return SetWord(Address(0));
}

Word
CreateURL( const ListExpr typeInfo )
{
  __TRACE__
  return (SetWord( new URL( "http://" ) ));
}

void
DeleteURL( const ListExpr typeInfo, Word& w )
{
  __TRACE__
//  ((URL*)w.addr)->destroy();
  delete (URL *)w.addr;
  w.addr = 0;
}

void
CloseURL( const ListExpr typeInfo, Word& w )
{
  __TRACE__
  delete (URL *)w.addr;
//  w.addr = 0;
}

Word
CloneURL( const ListExpr typeInfo, const Word& w )
{
  __TRACE__
  return SetWord( ((URL *)w.addr)->Clone() );
}

int
SizeOfURL()
{
  __TRACE__
  return sizeof(URL);
}

/*

4.2 Kind Checking Function and Property of ~URL~

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckURL( ListExpr type, ListExpr& errorInfo )
{
  __TRACE__
  return (nl->IsEqual( type, "url" ));
}

ListExpr
URLProperty()
{
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("url"),
           nl->StringAtom("(<protocol> <host> <path>)"),
           nl->StringAtom("(http //dict.leo.org /)"),
           nl->StringAtom("prot.: STRING<46 bytes, host, path"
           "type text."))));
}

void* CastURL( void* addr ) {return (new (addr) URL);}

/*
4.3 Creation of the Type Constructor Instance of ~URL~

*/
TypeConstructor url(   "url",
                URLProperty,
                OutURL, InURL,
                0, 0,
                CreateURL, DeleteURL,
                0, 0,
                CloseURL, CloneURL,
                CastURL, SizeOfURL,
                CheckURL );


/*
5 In/Out, Checking Functions and Type Construction of HTML

5.1 List Representation and In/Out Functions of ~HTML~

Example: The list representation of a HTML is

Listenformat: ( datetime text url )
Atribute: LastChange, source, sourceURL
Example:

----
let html1 = [const html value ((instant (10 10 2006 10 27 18)) <text>test</text---> (url ("http" <text>www.xx.de</text---> <text>/</text---> )))]
----

*/

ListExpr
OutHTML( ListExpr typeInfo, Word value )
{
  __TRACE__
  return ((HTML*)(value.addr))->ToListExpr(false);
}

Word
InHTML( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  __TRACE__
  if ( nl->ListLength( instance ) == 3 )
  {
    ListExpr First = nl->First(instance);    //DateTime
    ListExpr Second = nl->Second(instance);  //Text (FLOB)
    ListExpr Third = nl->Third(instance);    //URL
   
    if ( nl->ListLength( First ) == 2 
     && nl->IsEqual(nl->First(First), "instant")
      && nl->IsAtom(Second) && nl->AtomType(Second) == TextType
      && nl->ListLength( Third ) == 2 && nl->IsEqual(nl->First(Third), "url"))
    {
      DateTime date(instanttype);
    date.ReadFrom(First,true);
      string text = nl->Text2String(Second);
//    cout << "Text: " << text << endl;
  __TRACE__
  
    Base64 b;
    int sizeDecoded = b.sizeDecoded( text.size() );
    char *bytes = (char *)malloc( sizeDecoded + 1);
    
    int result = b.decode( text, bytes );
    
    assert( result <= sizeDecoded );
    bytes[result] = 0;
    //cout << "Size: " << result << endl;
    //cout << "Dekodiert: " << bytes << endl;
    text = bytes;
    free( bytes );
    //cout << "Text: " << text << endl;
    //cout << "Size Text: " << text.size() << endl;
  __TRACE__
    correct = true;
    //string out;
    //nl->WriteToString(out, Third);
    //cout << "Typ Third: " << out << endl;
    Word u = InURL( Third, nl->Second(Third),errorPos,errorInfo, correct );
    URL *url;
    if( correct)
    {
      url = (URL*)u.addr;  
      {
        //cout << " in html " << url->IsDefined() << endl;
        HTML* newHtml = new HTML(date, text, *url);
        return SetWord(newHtml);
      }
    }
    else
    {
        ErrorReporter::ReportError("Error in reading url in InHTML");
      return SetWord(Address(0));
    }
    }
   else
   {
  __TRACE__
    if( !nl->ListLength( First ) == 2 ) 
             ErrorReporter::ReportError("First not an list of length 2");
    else if( !nl->IsAtom(Second)) 
             ErrorReporter::ReportError("Second not an atom");
    else if( !nl->ListLength( Third ) == 2) 
             ErrorReporter::ReportError("Third not a list of length 2");
    else if (!(nl->IsEqual(nl->First(First), "instant")))
        ErrorReporter::ReportError("First not an instant");
    else if (!(nl->AtomType(Second) == TextType))
        ErrorReporter::ReportError("Second not a TextType");
    else //if (!(nl->IsEqual(nl->First(Third), "url")))
        ErrorReporter::ReportError("Third not a url");
    correct = false;
    return SetWord(Address(0));
   }
  }
  __TRACE__
  ErrorReporter::ReportError("Wrong number of params, expecting"
                             " lastModified,source,sourceUrl");
  correct = false;
  return SetWord(Address(0));
}

Word
CreateHTML( const ListExpr typeInfo )
{
  __TRACE__
  return (SetWord( new HTML( "" ) ));
}

void
DeleteHTML( const ListExpr typeInfo, Word& w )
{
  __TRACE__
  delete (HTML *)w.addr;
  w.addr = 0;
}

void
CloseHTML( const ListExpr typeInfo, Word& w )
{
  __TRACE__
  delete (HTML *)w.addr;
  w.addr = 0;
}

Word
CloneHTML( const ListExpr typeInfo, const Word& w )
{
  __TRACE__
  return SetWord( ((HTML *)w.addr)->Clone() );
}

int
SizeOfHTML()
{
  __TRACE__
  return sizeof(HTML);
}

/*

5.2 Kind Checking Function and Property of ~HTML~

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckHTML( ListExpr type, ListExpr& errorInfo )
{
  __TRACE__
  return (nl->IsEqual( type, "html" ));
}

ListExpr
HTMLProperty()
{
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("html"),
           nl->StringAtom("(<datetime: lastchange><text source> <url>)"),
           nl->StringAtom("(list representation)"),
           nl->StringAtom("url has the type url"))));
}

void* CastHTML( void* addr ) {return (new (addr) HTML);}

/*
5.3 Creation of the Type Constructor Instance of ~HTML~

*/
TypeConstructor html(   "html",
                HTMLProperty,
                OutHTML, InHTML,
                0, 0,
                CreateHTML, DeleteHTML,
                0, 0,
                CloseHTML, CloneHTML,
                CastHTML, SizeOfHTML,
                CheckHTML );


/*
6 In/Out, Checking Functions and Type Construction of Page

5.1 List Representation and In/Out Functions of ~Page~

Example: The list representation of a Page is

Listenformat: (html (url text string)*)
Atribute: html wird geerbt , (EmbededURL binFile mime)*
Example:

----
see at the top of the class Page
----

*/

ListExpr
OutPage( ListExpr typeInfo, Word value )
{
  __TRACE__
  Page* pPage = (Page*)(value.addr);
  int noObjects = pPage->numOfFiles();
  ListExpr pageList = nl->OneElemList(((HTML*)pPage)->ToListExpr(true));
  ListExpr pageStart = pageList;
  for( int ii=0; ii<noObjects; ii++)
  {
  __TRACE__
    pageList = nl->Append( pageList, nl->ThreeElemList(
     pPage->getUrl(ii).ToListExpr(true),
     nl->TextAtom(pPage->getText( ii)),
      nl->StringAtom(pPage->getMime( ii))));
  
  }
  __TRACE__
  return pageStart;
}

Word
InPage( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  __TRACE__
  if ( nl->ListLength( instance ) >= 1 
        && nl->ListLength( nl->First(instance) ) == 2
        && nl->IsEqual(nl->First(nl->First(instance)), "html"))
  {
    ListExpr First = nl->First(instance);    //html
   int nrOfEmb = nl->ListLength(instance) - 1;
   correct = true;
   Word h = InHTML( First, nl->Second(First),errorPos,errorInfo, correct );
   if( correct)
   {
     HTML *html = (HTML*)h.addr;
    Page *newpage = new Page(*html);
    First = nl->Rest(instance);
    //now lists of (url text string)
    for( int ii=0; ii < nrOfEmb; ii++)
    {
      ListExpr emblist = nl->First(First);
      First = nl->Rest(First);
      
      if ( nl->ListLength( emblist ) == 3 
        && nl->IsEqual(nl->First(nl->First(emblist)), "url")
        && nl->IsAtom(nl->Second(emblist)) 
        && nl->AtomType(nl->Second(emblist)) == TextType
        && nl->IsAtom(nl->Third(emblist)) 
        && nl->AtomType(nl->Third(emblist)) == StringType)
      {
        Word u = InURL( nl->First(emblist),
          nl->Second(nl->First(emblist)),errorPos,errorInfo, correct );
        if( correct)
        {
          URL *url = (URL*)u.addr;
          string text = nl->Text2String(nl->Second(emblist));
          string mime = nl->StringValue(nl->Third(emblist));
          newpage->addEmbObject(*url,mime,text);
          delete url;
          url = NULL;
        }
        else
        {
          __TRACE__
          ErrorReporter::ReportError("emb obj has not"
                                     " the right list structure");
          return SetWord(Address(0));
        }
      }
      else
      {
        __TRACE__
        correct = false;
        return SetWord(Address(0));
      }
    }
    return SetWord(newpage);
   }
   else
   {
     __TRACE__
    ErrorReporter::ReportError("page has no correct html as first element");
    return SetWord(Address(0));
   }
  }
  __TRACE__
  ErrorReporter::ReportError("Wrong number of params or not a html"
                             " as first, expecting html,(url,text, string)*");
  correct = false;
  return SetWord(Address(0));
}

Word
CreatePage( const ListExpr typeInfo )
{
  __TRACE__
  return (SetWord( new Page( "" ) ));
}

void
DeletePage( const ListExpr typeInfo, Word& w )
{
  __TRACE__
  delete (Page *)w.addr;
  w.addr = 0;
}

void
ClosePage( const ListExpr typeInfo, Word& w )
{
  __TRACE__
  delete (Page *)w.addr;
  w.addr = 0;
}

Word
ClonePage( const ListExpr typeInfo, const Word& w )
{
  __TRACE__
  return SetWord( ((Page *)w.addr)->Clone() );
}

int
SizeOfPage()
{
  __TRACE__
  return sizeof(Page);
}

/*

5.2 Kind Checking Function and Property of ~Page~

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckPage( ListExpr type, ListExpr& errorInfo )
{
  __TRACE__
  return (nl->IsEqual( type, "page" ));
}

ListExpr
PageProperty()
{
  __TRACE__
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("page"),
           nl->StringAtom("(<html>(<url text string>)*)"),
           nl->StringAtom("(list representation)"),
           nl->StringAtom("<url text mimetype> are the embedded objects"))));
}

void* CastPage( void* addr ) {return (new (addr) Page);}

/*
5.3 Creation of the Type Constructor Instance of ~Page~

*/
TypeConstructor page(   "page",
                PageProperty,
                OutPage, InPage,
                0, 0,
                CreatePage, DeletePage,
                0, 0,
                ClosePage, ClonePage,
                CastPage, SizeOfPage,
                CheckPage );


/*
6 Creating Operators

6.1.1 Type Mapping of Operator ~protocol,host,filename~

*/
ListExpr
protocolHostFilenameTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "url") )
      return nl->SymbolAtom("text");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.1 Type Mapping of Operator ~source~

*/
ListExpr
sourceTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "html") || nl->IsEqual(arg1,"page"))
      return nl->SymbolAtom("url");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.2 Type Mapping of Operator ~createurl~

*/
ListExpr
createurlTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "text"))
      return nl->SymbolAtom("url");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.3 Type Mapping of Operator ~content~

*/
ListExpr
contentTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "html"))
      return nl->SymbolAtom("text");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.4 Type Mapping of Operator ~urls~

*/
ListExpr
urlsTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "html") || nl->IsEqual(arg1,"page"))
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("url"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.5 Type Mapping of Operator ~containsurl~

*/
ListExpr
containsurlTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( (nl->IsEqual(arg1, "html") || nl->IsEqual(arg1,"page"))
       && nl->IsEqual(arg2,"url"))
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.6 Type Mapping of Operator ~last_modified~
----
----

*/
ListExpr
lastmodifiedTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "html"))
      return nl->SymbolAtom("instant");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.7 Type Mapping of Operator ~metainfo~

*/
ListExpr
metainfoTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "html")  && nl->IsEqual(arg2,"string"))
      return nl->SymbolAtom("text");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.8 Type Mapping of Operator ~metainfos~

*/
ListExpr
metainfosTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "html"))
   {
     ListExpr attrList = 
      nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Key"),
        nl->SymbolAtom("string")));
      nl->Append(attrList,nl->TwoElemList(nl->SymbolAtom("Content"),
        nl->SymbolAtom("text")));    
    
      return nl->TwoElemList(nl->SymbolAtom("stream"),
     nl->TwoElemList(nl->SymbolAtom("tuple"),attrList));
    }
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.9 Type Mapping of Operator ~number_of~

*/
ListExpr
numberofTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "html")  && nl->IsEqual(arg2,"string"))
      return nl->SymbolAtom("int");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.10 Type Mapping of Operator ~similar~

*/
ListExpr
similarTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 4 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);
    if ( nl->IsEqual(arg1, "html")  && nl->IsEqual(arg2,"html")
     && nl->IsEqual(arg3,"int") && nl->IsEqual(arg4,"bool"))
      return nl->SymbolAtom("real");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.11 Type Mapping of Operator ~extracthtml~

*/
ListExpr
extracthtmlTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "page"))
      return nl->SymbolAtom("html");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.12 Type Mapping of Operator ~numoffiles~

*/
ListExpr
numoffilesTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "page"))
      return nl->SymbolAtom("int");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.13 Type Mapping of Operator ~getfiles~

*/
ListExpr
getfilesTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "page"))
   {
     ListExpr attrList = 
      nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Source"),
        nl->SymbolAtom("url")));
    ListExpr lastAttrList = attrList;
    lastAttrList =
      nl->Append(lastAttrList,nl->TwoElemList(nl->SymbolAtom("Type"),
        nl->SymbolAtom("string")));    
    nl->Append(lastAttrList,nl->TwoElemList(nl->SymbolAtom("File"),
        nl->SymbolAtom("binfile")));    
    
      return nl->TwoElemList(nl->SymbolAtom("stream"),
     nl->TwoElemList(nl->SymbolAtom("tuple"),attrList));
    }
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.1.14 Type Mapping of Operator ~wget~

*/
ListExpr
wgetTypeMap( ListExpr args)
{
    __TRACE__
  if ( nl->ListLength(args) == 4 || nl->ListLength(args) == 5 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);
   if( nl->ListLength(args) == 5  )
   {
     ListExpr arg5 = nl->  Fifth(args);
    if (nl->IsAtom(arg5)
        || !nl->ListLength(arg5) == 3
        || !nl->IsEqual(nl->First(arg5), "map")
        || !nl->IsEqual(nl->Second(arg5), "url")
        || !nl->IsEqual(nl->Third(arg5), "bool") )
    {
      string out;
      nl->WriteToString(out, arg5);
      ErrorReporter::ReportError("Operator wget expects a "
        "(map -> bool) as its fifth argument. "
        "The second argument provided "
        "has type '" + out + "' instead.");
      return nl->SymbolAtom("typeerror");
    }
  }
   
  __TRACE__
    
    if ( nl->IsEqual(arg1, "url") && nl->IsEqual(arg2, "bool")
     && nl->IsEqual(arg3, "int") && nl->IsEqual(arg4, "text"))
   {
  __TRACE__
     ListExpr attrList = 
      nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Source"),
        nl->SymbolAtom("url")));
    ListExpr lastAttrList = attrList;
    lastAttrList =
      nl->Append(lastAttrList,nl->TwoElemList(nl->SymbolAtom("Type"),
        nl->SymbolAtom("string")));    
    nl->Append(lastAttrList,nl->TwoElemList(nl->SymbolAtom("File"),
        nl->SymbolAtom("binfile")));    
    
      return nl->TwoElemList(nl->SymbolAtom("stream"),
     nl->TwoElemList(nl->SymbolAtom("tuple"),attrList));
    }
  }
  __TRACE__
  return nl->SymbolAtom("typeerror");
}

/*
6.1.15 Type Mapping of Operator ~pageget~

*/
ListExpr
pagegetTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 4 || nl->ListLength(args) == 5 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);
   if( nl->ListLength(args) == 5  )
   {
     ListExpr arg5 = nl->Fifth(args);
    if (nl->IsAtom(arg5)
        || !nl->ListLength(arg5) == 3
        || !nl->IsEqual(nl->First(arg5), "map")
        || !nl->IsEqual(nl->Second(arg5), "url")
        || !nl->IsEqual(nl->Third(arg5), "bool") )
    {
      string out;
      nl->WriteToString(out, arg5);
      ErrorReporter::ReportError("Operator pageget expects a "
        "(map -> bool) as its fifth argument. "
        "The second argument provided "
        "has type '" + out + "' instead.");
      return nl->SymbolAtom("typeerror");
    }
  }
   
  __TRACE__
    
    if ( nl->IsEqual(arg1, "url") && nl->IsEqual(arg2, "bool")
     && nl->IsEqual(arg3, "int") && nl->IsEqual(arg4, "text"))
   {
  __TRACE__
     ListExpr attrList = 
      nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Source"),
        nl->SymbolAtom("url")));
      nl->Append(attrList,nl->TwoElemList(nl->SymbolAtom("Page"),
        nl->SymbolAtom("page")));    
    
      return nl->TwoElemList(nl->SymbolAtom("stream"),
     nl->TwoElemList(nl->SymbolAtom("tuple"),attrList));
    }
  }
  __TRACE__
  return nl->SymbolAtom("typeerror");
}

/*
6.1.15 Type Mapping of Operator ~htmlget~

*/
ListExpr
htmlgetTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 4 || nl->ListLength(args) == 5 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);
   if( nl->ListLength(args) == 5  )
   {
     ListExpr arg5 = nl->Fifth(args);
    if (nl->IsAtom(arg5)
        || !nl->ListLength(arg5) == 3
        || !nl->IsEqual(nl->First(arg5), "map")
        || !nl->IsEqual(nl->Second(arg5), "url")
        || !nl->IsEqual(nl->Third(arg5), "bool") )
    {
      string out;
      nl->WriteToString(out, arg5);
      ErrorReporter::ReportError("Operator htmlget expects a "
        "(map -> bool) as its fifth argument. "
        "The second argument provided "
        "has type '" + out + "' instead.");
      return nl->SymbolAtom("typeerror");
    }
  }
   
  __TRACE__
    
    if ( nl->IsEqual(arg1, "url") && nl->IsEqual(arg2, "bool")
     && nl->IsEqual(arg3, "int") && nl->IsEqual(arg4, "text"))
   {
  __TRACE__
     ListExpr attrList = 
      nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Source"),
        nl->SymbolAtom("url")));
      nl->Append(attrList,nl->TwoElemList(nl->SymbolAtom("Html"),
        nl->SymbolAtom("html")));    
    
      return nl->TwoElemList(nl->SymbolAtom("stream"),
     nl->TwoElemList(nl->SymbolAtom("tuple"),attrList));
    }
  }
  __TRACE__
  return nl->SymbolAtom("typeerror");
}

/*
6.1.16 Type Mapping of Operator ~webequal =:~

*/
ListExpr
webequalTypeMap( ListExpr args)
{
  __TRACE__
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( (nl->IsEqual(arg1, "url") && nl->IsEqual(arg2,"url")) 
     || (nl->IsEqual(arg1, "html") && nl->IsEqual(arg2,"html"))
       || (nl->IsEqual(arg1, "page")&& nl->IsEqual(arg2,"page")))
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
6.2 Value Mapping and Selection Functions

6.2.1 Value Mapping Function for Operator ~protocol~

*/
int
protocolFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  URL* u = ((URL*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((FText*)result.addr)->Set(true, u->getProtocol().c_str());
            //the first argument says the 
            //value is defined, the second is the
            //real value)
  return 0;
}

/*
6.2.2 Value Mapping Function for Operator ~host~

*/
int
hostFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  URL* u = ((URL*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((FText*)result.addr)->Set(true, u->getHost().c_str());
            //the first argument says the 
            //value is defined, the second is the
            //real value)
  return 0;
}

/*
6.2.3 Value Mapping Function for Operator ~filename~

*/
int
filenameFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  URL* u = ((URL*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((FText*)result.addr)->Set(true, u->getPath().c_str());
            //the first argument says the boolean
            //value is defined, the second is the
            //real value)
  return 0;
}

/*
6.2.4 Value Mapping Function for Operator ~source~

*/
int
sourceFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  URL *u = new URL(h->getSource());
  __TRACE__
  ((URL*)result.addr)->Set(true, *u);
            //the first argument says the boolean
            //value is defined, the second is the
            //real value)
  __TRACE__
  delete u;
  return 0;
}

/*
6.2.5 Value Mapping Function for Operator ~createurl~

*/
int
createurlFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  FText* t = ((FText*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  const char *str =  t->Get();
  URL u("");
  string sUrl = str;
  bool erg = URL::urlFromString(sUrl,u);
  //the function has to return a url. From every string
  //it has to return a valid url
  ((URL*)result.addr)->Set(erg, u);
            //the first argument says the 
            //value is defined, the second is the
            //real value)
  return 0;
}

/*
6.2.6 Value Mapping Function for Operator ~content~

*/
int
contentFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((FText*)result.addr)->Set(true, h->getText().c_str());
            //the first argument says the 
            //value is defined, the second is the
            //real value)
  return 0;
}

/*
6.2.7 Value Mapping Function for Operator ~urls~

*/
int
urlsFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);
  struct UrlAdvance {int numberOf, current;}* urladvance;

  switch( message )
  {
    case OPEN:

      urladvance = new UrlAdvance;
      urladvance->current = 0;
      urladvance->numberOf =  h->getNumberOfUrls();

      local.addr = urladvance;

      return 0;

    case REQUEST:

      urladvance = ((UrlAdvance*) local.addr);

      if ( urladvance->current < urladvance->numberOf )
      {
        URL *elem = new URL((h->getUrl(urladvance->current++)));
        result.addr = elem;
        return YIELD;
      }
      else return CANCEL;

    case CLOSE:

      urladvance = ((UrlAdvance*) local.addr);
      delete urladvance;
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
6.2.8 Value Mapping Function for Operator ~containsurl~

*/
int
containsurlFun (Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);
  URL* u = ((URL*)args[1].addr);

   result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((CcBool*)result.addr)->Set(true, h->containsURL(u));
            //the first argument says the boolean
            //value is defined, the second is the
            //real value)
  return 0;
}

/*
6.2.9 Value Mapping Function for Operator ~lastmodified~

*/
int
lastmodifiedFun (Word* args, Word& result, int message, 
                 Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  DateTime d = h->getLastModified();
  ((DateTime*)result.addr)->Set(d.GetYear(),d.GetMonth(), d.GetGregDay(),
            d.GetHour(), d.GetMinute(), d.GetSecond(),d.GetMillisecond());
  return 0;
}

/*
6.2.10 Value Mapping Function for Operator ~metainfo~

*/
int
metainfoFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);
  string key = StdTypes::GetString(args[1]);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((FText*)result.addr)->Set(true, h->getMetaInfo(key).c_str());
  return 0;
}

/*
6.2.11 Value Mapping Function for Operator ~metainfos~

*/
int
metainfosFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);
  struct MiAdvance {int numberOf, current;
                    TupleType *resultTupleType;}* miAdvance;

  ListExpr resultType;
  
  switch( message )
  {
    case OPEN:

      miAdvance = new MiAdvance;
      miAdvance->current = 0;
      miAdvance->numberOf =  h->getNumberOfMetainfos();
    resultType = GetTupleResultType( s );
    miAdvance->resultTupleType = new TupleType( nl->Second( resultType ));
      local.addr = miAdvance;

      return 0;

    case REQUEST:

      miAdvance = ((MiAdvance*) local.addr);

      if ( miAdvance->current < miAdvance->numberOf )
      {
      string content;
      string key = h->getMetainfo(miAdvance->current++,content);
      //make tuple [Key: string, Content: text]
      Tuple *elem = new Tuple( miAdvance->resultTupleType );
      STRING skey;
      strcpy(skey, key.c_str());
      CcString* cckey = new CcString(true,&skey);
      elem->PutAttribute(0,cckey);
      FText *t = new FText(true,content.c_str());
      elem->PutAttribute(1,t);
      result.addr = elem;
      return YIELD;
      }
      else return CANCEL;

    case CLOSE:

      miAdvance = ((MiAdvance*) local.addr);
    miAdvance->resultTupleType->DeleteIfAllowed();
      delete miAdvance;
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
6.2.12 Value Mapping Function for Operator ~numberof~

*/
int
numberofFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  HTML* h = ((HTML*)args[0].addr);
  string key = StdTypes::GetString(args[1]);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((CcInt*)result.addr)->Set(true, h->getNumberOf(key));
  return 0;
}

/*
6.2.13 Value Mapping Function for Operator ~similar~

*/
int
similarFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  __TRACE__
  HTML* h1 = ((HTML*)args[0].addr);
  HTML* h2 = ((HTML*)args[1].addr);
  int tiefe = StdTypes::GetInt(args[2]);
  bool doFollowOrder = StdTypes::GetBool(args[3]);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((CcReal*)result.addr)->Set(true, h1->similar(h2,tiefe,doFollowOrder));
  __TRACE__
  return 0;
}

/*
6.2.14 Value Mapping Function for Operator ~extracthtml~

*/
int
extracthtmlFun (Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  __TRACE__
  Page* p = ((Page*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  HTML h( p->extractHTML());
  ((HTML*)result.addr)->Set(h);
  return 0;
}

/*
6.2.15 Value Mapping Function for Operator ~numoffiles~

*/
int
numoffilesFun (Word* args, Word& result, int message, 
               Word& local, Supplier s)
{
  __TRACE__
  Page* p = ((Page*)args[0].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((CcInt*)result.addr)->Set(true, p->numOfFiles());
  return 0;
}

/*
6.2.16 Value Mapping Function for Operator ~getfiles~

*/
int
getfilesFun (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  __TRACE__
  ListExpr resultType;
  Page* p = ((Page*)args[0].addr);
  struct EmbAdvance {int numberOf, current; 
                     TupleType *resultTupleType;}* embAdvance;

  switch( message )
  {
    case OPEN:

      embAdvance = new EmbAdvance;
      embAdvance->current = 0;
      embAdvance->numberOf =  p->numOfFiles();
    resultType = GetTupleResultType( s );
    embAdvance->resultTupleType = new TupleType( nl->Second( resultType ) );

      local.addr = embAdvance;

      return 0;

    case REQUEST:

      embAdvance = ((EmbAdvance*) local.addr);

      if ( embAdvance->current < embAdvance->numberOf )
      {
      URL *u = new URL((p->getUrl(embAdvance->current)));
      string type = p->getMime( embAdvance->current);
      string src = p->getText( embAdvance->current++);
      
      //make tuple [Source: url, Type: string, File: binfile]
      Tuple *elem = new Tuple( embAdvance->resultTupleType );
      elem->PutAttribute(0,u);
      STRING stype;
      strcpy(stype, type.c_str());
      CcString* cctype = new CcString(true,&stype);
      elem->PutAttribute(1,cctype);
      //BinaryFile *file = new BinaryFile( src.length()+1 );
      //file->Put(0,src.length()+1,src.c_str());
      BinaryFile *file = new BinaryFile( 0 );
      file->Decode(src);
      elem->PutAttribute(2,file);
      result.addr = elem;
      return YIELD;
      }
      else return CANCEL;

    case CLOSE:

      embAdvance = ((EmbAdvance*) local.addr);
    embAdvance->resultTupleType->DeleteIfAllowed();
      delete embAdvance;
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
6.2.17.1 class definitions for hashtable for operators wget and pageget

*/
class HashUrl
{
private:
  static const size_t NO_BUCKETS = 50;//255;
  size_t nBuckets;

  vector<vector< string*> > *bucketsU;

  size_t GetHashVal(string* s)
  {
    int size = 0;
    for( unsigned int i = 0; i < s->length(); i++)
    {
      size += (*s)[i];
    }
    return size % nBuckets;
  }

  void ClearBucketsU()
  {
  
    vector< vector<string*> >::iterator iterBuckets = bucketsU->begin();

    while(iterBuckets != bucketsU->end() )
    {
      vector<string*>::iterator iter = (*iterBuckets).begin();
      while(iter != (*iterBuckets).end())
      {
       delete *iter;
        iter++;
      }
      iterBuckets++;
    }
   }

public:
  HashUrl()
  {
    nBuckets = NO_BUCKETS;
    bucketsU = new vector< vector< string*> >(nBuckets);
  }

  ~HashUrl()
  {
    ClearBucketsU();
  }

  bool IsDuplicate( string &s)
  {
    //prüft ob sring schon im Hash ist
    //Wenn ja wird true returnt, 
    //sonst false und der übergeb.String wird eingefügt
   
   char* str = new char[s.length() + 1];
   char *pstr = str;
   const char* ps = s.c_str();
   while ((*pstr++ = toupper(*ps++)) != 0);
   string *hashstring = new string(str);
   delete[] str;

    size_t hashVal = GetHashVal(hashstring);
   //cout << "Wert: " << hashVal << "Hash: " << *hashstring << endl;
    vector<string*>::iterator iter = (*bucketsU)[hashVal].begin(); 
    while(iter != (*bucketsU)[hashVal].end())
    {
     //cout << "iter: " << **iter << endl;
     if( **iter == *hashstring)
    {
      return true; //Die Strings sind gleich
    }
      iter++;
    }
    //hier daher kein gleiches gefunden
    (*bucketsU)[hashVal].push_back(hashstring);
    return false;
  }
};

/*
6.2.19 Selection functions  for Operator ~wget, pageget, htmlget~

*/
int webwget_pagegetSelect( ListExpr args)
{
  if ( nl->ListLength(args) == 4  )
    return(0);
  if ( nl->ListLength(args) == 5  )
    return(1);
  return(-1); //This point should never be reached
}


/*
6.2.17 Value Mapping Functions for Operator ~wget~

*/

struct PageAdvance {int numberOfEmb, currentEmb, 
                numberOfLinks,currentLink; Page *p;};
    
int
wgetFun (Word* args, Word& result, int message, Word& local, Supplier s,
  bool hasFunction)
{
  ListExpr resultType;
  
  struct GetAdvance {stack<PageAdvance*>* myDepthStack; 
                HashUrl *myHash; TupleType *resultTupleType;
              int depth; bool isnew;; string *host;}* getAdvance;
  __TRACE__
  
  switch( message )
  {
    case OPEN:
    {
  __TRACE__
      getAdvance = new GetAdvance;
      getAdvance->myHash = new HashUrl;
      getAdvance->myDepthStack = new stack<PageAdvance*>;
      resultType = GetTupleResultType( s );
      getAdvance->resultTupleType = new TupleType( nl->Second( resultType ) );
      getAdvance->depth = 0;
      getAdvance->isnew = true;
      FText* t = ((FText*)args[3].addr);
      URL* u = ((URL*)args[0].addr);
      string s = t->Get();
      if( s.length() > 0) 
      {
        getAdvance->host = new string(u->getHost() + "," + t->Get());
      }
      else
      {
        getAdvance->host = new string(u->getHost());
      }
      
      local.addr = getAdvance;
    }
      return 0;

    case REQUEST:
     //cout << "In wget Request" << endl;
  __TRACE__
    {
      getAdvance = ((GetAdvance*) local.addr);
    PageAdvance *pa = NULL;
    bool extLinks = StdTypes::GetBool(args[1]);
    int depth = StdTypes::GetInt(args[2]);
    bool isUnlimited = (depth < 0);
    URL *exturl = NULL;
    if( !getAdvance->myDepthStack->empty() )
    {
      pa = getAdvance->myDepthStack->top();
    }
    while( !exturl && pa)
    {
  __TRACE__
      while ( pa && pa->currentEmb < pa->numberOfEmb )
      {
    __TRACE__
        URL *u = new URL((pa->p->getUrl(pa->currentEmb)));
        string type = pa->p->getMime( pa->currentEmb);
        string src = pa->p->getText( pa->currentEmb++);
        
        string hashstring = u->getProtocol() + ":" 
          + u->getHost() + u->getPath();
        if( !getAdvance->myHash->IsDuplicate(hashstring) )
        {
          cout << *u << endl;
          //make tuple [Source: url, Type: string, File: binfile]
          Tuple *elem = new Tuple( getAdvance->resultTupleType );
          elem->PutAttribute(0,u);
          STRING stype;
          strcpy(stype, type.c_str());
          CcString* cctype = new CcString(true,&stype);
          elem->PutAttribute(1,cctype);
          BinaryFile *file = new BinaryFile( 0 );
          if( src.length() )
            file->Decode(src);
          elem->PutAttribute(2,file);
          result.addr = elem;
          return YIELD;
        }
        else
        {
          delete u;
          u = 0;
        }
      }
      //check if there is a link (a href) to load
      //after the emb obj. are handelt
      while( !exturl && pa && pa->currentLink < pa->numberOfLinks  )
      {
        //check if the right host und check if the 
        //url is not loaded before with the hash. 
        //Also check of the function
        bool hostOk = true;
        URL *checkUrl = new URL((pa->p->getUrlHosts(pa->currentLink++,
            *getAdvance->host,hostOk)));
        cout << *checkUrl << endl;
        

        if( checkUrl->IsDefined() && hostOk)
        {
  __TRACE__
          string hashstring = checkUrl->getProtocol() + "://" 
            + checkUrl->getHost() + checkUrl->getPath();
          if(!getAdvance->myHash->IsDuplicate(hashstring))
          {
            cout << "Defined and host o.k. and not duplicate" << endl;
            if( hasFunction )
            {
              ArgVectorPointer funargs = qp->Argument(args[4].addr);
              (*funargs)[0] = SetWord(checkUrl);
              Word funresult;
              qp->Request(args[4].addr, funresult);
              bool funerg;
              if (((StandardAttribute*)funresult.addr)->IsDefined())
              {
                funerg = ((CcBool*)funresult.addr)->GetBoolval();
              }
              else
                funerg = false;
                
              if( funerg)
              {
                exturl = checkUrl;
              }
              else
              {
                delete checkUrl;
                checkUrl = NULL;
              }
            }
            else
              exturl = checkUrl;
          }
          else
          {
            delete checkUrl;
            checkUrl = NULL;
          }
        }
        else
          delete checkUrl;
          checkUrl = NULL;
      }
      if( !exturl )
      {
        delete pa->p;
        delete pa;
        pa = 0;
        getAdvance->myDepthStack->pop();
        --getAdvance->depth;
        if( !getAdvance->myDepthStack->empty() )
        {
          pa = getAdvance->myDepthStack->top();
        }
      }
    }
    if(getAdvance->isnew || exturl)
    {
  __TRACE__
      //load the URL und make Page-Objekt if is HTML
      //else return the loaded file

      URL* u;
      if(getAdvance->isnew)
      {
  __TRACE__
        u = ((URL*)args[0].addr);
        /*if( hasFunction )
        {
          ArgVectorPointer funargs = qp->Argument(args[4].addr);
          (*funargs)[0] = args[0];
          Word funresult;
          qp->Request(args[4].addr, funresult);
          bool funerg;
          if (((StandardAttribute*)funresult.addr)->IsDefined())
          {
            funerg = ((CcBool*)funresult.addr)->GetBoolval();
          }
          else
            funerg = false;
            
          if( !funerg)
          {
            return CANCEL;
          }
        }*/
        
        string hashstring = u->getProtocol() + "://" 
          + u->getHost() + u->getPath();
        getAdvance->myHash->IsDuplicate(hashstring);
        getAdvance->isnew = false;
        exturl = new URL(*u);
      }
      u = exturl;
      string type;// = "text/html";
      bool isHtml = false;
      DateTime dt;
      cout << "load url from web" << endl;
      string src = Page::getFromWeb(*u, type, isHtml, dt); 
      //cout << "ready loading url" << endl;
      #ifdef _DEBUG_JPS_2
      cout << "DEBUG_JPS_2" << src  << "DEBUG_JPS_2 ends"<< endl;
      #endif
      Tuple *elem = new Tuple( getAdvance->resultTupleType );
      elem->PutAttribute(0,u);
      STRING stype;
      strcpy(stype, type.c_str());
      CcString* cctype = new CcString(true,&stype);
      elem->PutAttribute(1,cctype);
      if( !isHtml && (int)type.find("html") != -1)
        isHtml = true;
      cout << "isHTML: " << isHtml << ", " << type << endl;
      BinaryFile *file;
      if( isHtml )
      {
        file = new BinaryFile( src.length()+1 );
        file->Put(0,src.length()+1,src.c_str());
      }
      else
      {
        file = new BinaryFile( 0 );
        if( src.length() )
          file->Decode(src);
      }
      elem->PutAttribute(2,file);
      result.addr = elem;
      
      if( isHtml)
      {
  __TRACE__
        //make page object of the html data
        //const char* s = 0;
        //file->Get(0, &s);
        //string str = s;
        DateTime dt;
        Page *p = new Page(*u, type, src, dt);
        PageAdvance *pa = new PageAdvance();
        pa->numberOfEmb = p->numOfFiles();
        if( extLinks && (isUnlimited || getAdvance->depth < depth ))
          pa->numberOfLinks = p->getNumberOfUrls();
        else
          pa->numberOfLinks = 0;
        cout << "Links: " << pa->numberOfLinks << endl;
        pa->currentEmb = 0;
        pa->currentLink = 0;
        pa->p = p;
        ++getAdvance->depth;
        getAdvance->myDepthStack->push(pa);
      }
      return YIELD;
    }
      else 
      return CANCEL;
      
    }

    case CLOSE:
  __TRACE__
    {
      getAdvance = ((GetAdvance*) local.addr);
    delete getAdvance->myHash;
    getAdvance->myHash = 0;
    delete getAdvance->host;
    getAdvance->host = 0;
    while( !getAdvance->myDepthStack->empty())
    {
      PageAdvance *pa = getAdvance->myDepthStack->top();
      if( pa->p)
        delete pa->p;
      delete pa;
      pa = 0;
      getAdvance->myDepthStack->pop();
    }
    delete getAdvance->myDepthStack;
    getAdvance->myDepthStack = 0;
    getAdvance->resultTupleType->DeleteIfAllowed();
      delete getAdvance;
    getAdvance = 0;
      return 0;
    }
  }
  /* should not happen */
  return -1;
}

int
ISWebWgetFourParam (Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  return wgetFun(args,result,message,local,s,false);
}
int
ISWebWgetFiveParam (Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  return wgetFun(args,result,message,local,s,true);
}

/*
6.2.18 Value Mapping Function for Operator ~pageget, htmlget~

*/
int
pagegetFun (Word* args, Word& result, int message, Word& local, Supplier s, 
  bool hasFunction, bool onlyhtml)
{
//to check with map not ready
  ListExpr resultType;
  
  struct GetAdvance {stack<PageAdvance*>* myDepthStack; 
                HashUrl *myHash; TupleType *resultTupleType;
              int depth; bool isnew; string *host;}* getAdvance;
  __TRACE__
  
  switch( message )
  {
    case OPEN:
    {
  __TRACE__
      getAdvance = new GetAdvance;
      getAdvance->myHash = new HashUrl;
      getAdvance->myDepthStack = new stack<PageAdvance*>;
      resultType = GetTupleResultType( s );
      getAdvance->resultTupleType = new TupleType( nl->Second( resultType ) );
      getAdvance->depth = 0;
      getAdvance->isnew = true;
      FText* t = ((FText*)args[3].addr);
      URL* u = ((URL*)args[0].addr);
      string s = t->Get();
      if( s.length() > 0) 
      {
        getAdvance->host = new string(u->getHost() + "," + t->Get());
      }
      else
      {
        getAdvance->host = new string(u->getHost());
      }
      
      local.addr = getAdvance;
    }
      return 0;

    case REQUEST:
  __TRACE__
    {
      getAdvance = ((GetAdvance*) local.addr);
    PageAdvance *pa = NULL;
    bool extLinks = StdTypes::GetBool(args[1]);
    int depth = StdTypes::GetInt(args[2]);
    bool isUnlimited = (depth < 0);
    URL *exturl = NULL;
    while( getAdvance->isnew || !getAdvance->myDepthStack->empty() )
    {
  __TRACE__
      if( !getAdvance->myDepthStack->empty() )
        pa = getAdvance->myDepthStack->top();
      
      while( !exturl && pa)
      {
    __TRACE__
        //check if there is a link (a href) to load
        //after the emb obj. are handelt
        while( !exturl && pa->currentLink < pa->numberOfLinks)
        {
          //check if the right host und check if the 
          //url is not loaded before with the hash. 
          //Also check of the function
          bool hostOk = true;
          URL *checkUrl = new URL((pa->p->getUrlHosts(pa->currentLink++,
              *getAdvance->host,hostOk)));
          //cout << *checkUrl << endl;
          cout << ".";
          if( checkUrl->IsDefined() && hostOk)
          {
            string hashstring = checkUrl->getProtocol() + "://" 
              + checkUrl->getHost() + checkUrl->getPath();
            if(!getAdvance->myHash->IsDuplicate(hashstring))
            {
              //cout << "Defined and host o.k. and not duplicate" << endl;
              cout << hashstring << endl;
              if( hasFunction )
              {
                ArgVectorPointer funargs = qp->Argument(args[4].addr);
                (*funargs)[0] = SetWord(checkUrl);
                Word funresult;
                qp->Request(args[4].addr, funresult);
                bool funerg;
                if (((StandardAttribute*)funresult.addr)->IsDefined())
                {
                  funerg = ((CcBool*)funresult.addr)->GetBoolval();
                }
                else
                  funerg = false;
                  
                if( funerg)
                {
                  exturl = checkUrl;
                }
                else
                {
                  delete checkUrl;
                  checkUrl = NULL;
                }
              }
              else
                exturl = checkUrl;
            }
            else
            {
              delete checkUrl;
              checkUrl = NULL;
            }
          }
          else
          {
            delete checkUrl;
            checkUrl = NULL;
          }
        }
        if( !exturl )
        {
          delete pa->p;
          delete pa;
          pa = 0;
          getAdvance->myDepthStack->pop();
          --getAdvance->depth;
          if( !getAdvance->myDepthStack->empty() )
          {
            pa = getAdvance->myDepthStack->top();
          }
        }
      }
      if(getAdvance->isnew || exturl)
      {
  __TRACE__
        //load the URL und make Page-Objekt if is HTML
        //else return the loaded file
        URL* u;
        if(getAdvance->isnew)
        {
  __TRACE__
          u = ((URL*)args[0].addr);
          /*if( hasFunction )
          {
            ArgVectorPointer funargs = qp->Argument(args[4].addr);
            (*funargs)[0] = args[0];
            Word funresult;
            qp->Request(args[4].addr, funresult);
            bool funerg;
            if (((StandardAttribute*)funresult.addr)->IsDefined())
            {
              funerg = ((CcBool*)funresult.addr)->GetBoolval();
            }
            else
              funerg = false;
              
            if( !funerg)
            {
  __TRACE__
              return CANCEL;
            }
          }*/
          
          string hashstring = u->getProtocol() + "://" 
            + u->getHost() + u->getPath();
          getAdvance->myHash->IsDuplicate(hashstring);
          getAdvance->isnew = false;
          exturl = new URL(*u);
          cout << *u << endl;
        }
        u = exturl;
        string type;// = "text/html";
        bool isHtml = false;
          DateTime dt(instanttype);
    __TRACE__
        cout << "load url from web" << endl;
        string src = Page::getFromWeb(*u, type, isHtml, dt, true);  
        //cout << "ready loading url" << endl;
        
//    __TRACE__
        if( !isHtml && (int)type.find("html") != -1)
          isHtml = true;
        cout << "isHTML: " << isHtml << ", " << type << endl;
        
        if( isHtml)
        {
    __TRACE__
          //make page or html object depends on value onlyhtml 
          //of the html data
          Page *p;
          PageAdvance *pa = new PageAdvance();
          if( onlyhtml ) 
          {
            HTML h(dt, src, *u);
            
            p = new Page( h );
            //cout << "Inhalt" << p->getContent() << endl;
             pa->numberOfEmb = 0;
          }
          else
          {
            p = new Page(*u, type, src, dt);
            pa->numberOfEmb = p->numOfFiles();
          }
          if( extLinks && (isUnlimited || getAdvance->depth < depth ))
            pa->numberOfLinks = p->getNumberOfUrls();
          else
            pa->numberOfLinks = 0;
          cout << "Links: " << pa->numberOfLinks << endl << endl;
          pa->currentEmb = 0;
          pa->currentLink = 0;
          pa->p = p;
          ++getAdvance->depth;
          getAdvance->myDepthStack->push(pa);
          
          Tuple *elem = new Tuple( getAdvance->resultTupleType );
          if( onlyhtml )
          {
            HTML *hh = (HTML*)p;
            elem->PutAttribute(0,u);
            elem->PutAttribute(1,new HTML(*hh));
          }
          else
          {
            elem->PutAttribute(0,u);
            elem->PutAttribute(1,new Page(*p));
          }
    
          result.addr = elem;
          return YIELD;
        }
        else
        {
          pa = NULL;
          delete exturl;
          exturl = NULL;
        }
      }
    }
    
    return CANCEL;
      
    }

    case CLOSE:
  __TRACE__
    {
      getAdvance = ((GetAdvance*) local.addr);
    delete getAdvance->myHash;
    getAdvance->myHash = 0;
    delete getAdvance->host;
    getAdvance->host = 0;
    while( !getAdvance->myDepthStack->empty())
    {
      PageAdvance *pa = getAdvance->myDepthStack->top();
      if( pa->p)
        pa->p->DeleteIfAllowed();
      delete pa;
      pa = 0;
      getAdvance->myDepthStack->pop();
    }
    delete getAdvance->myDepthStack;
    getAdvance->myDepthStack = 0;
    getAdvance->resultTupleType->DeleteIfAllowed();
      delete getAdvance;
    getAdvance = 0;
      return 0;
    }
  }
  /* should not happen */
  __TRACE__
  return -1;
}

int
ISWebPagegetFourParam (Word* args, Word& result, int message, 
                       Word& local, Supplier s)
{
  return pagegetFun(args,result,message,local,s,false,false);
}
int
ISWebPagegetFiveParam (Word* args, Word& result, int message, 
                       Word& local, Supplier s)
{
  return pagegetFun(args,result,message,local,s,true,false);
}
int
ISWebHtmlgetFourParam (Word* args, Word& result, int message, 
                       Word& local, Supplier s)
{
  return pagegetFun(args,result,message,local,s,false,true);
}
int
ISWebHtmlgetFiveParam (Word* args, Word& result, int message, 
                       Word& local, Supplier s)
{
  return pagegetFun(args,result,message,local,s,true,true);
}

/*
6.2.19 Selection functions  for Operator ~webequal~

*/
int webequalSelect( ListExpr args)
{
  ListExpr arg1 = nl->First( args);
  ListExpr arg2 = nl->Second( args);
  if ( nl->IsEqual(arg1, "url") && nl->IsEqual(arg2, "url") )
    return(0);
  if ( nl->IsEqual(arg1, "html") && nl->IsEqual(arg2, "html") )
    return(1);
  if ( nl->IsEqual(arg1, "page") && nl->IsEqual(arg2, "page") )
    return(2);
  return(-1); //This point should never be reached
}

/*
6.2.20 Value Mapping Functions for Operators ~webequal~

*/
int
ISWebequalUrlFun (Word* args, Word& result, int message, 
                  Word& local, Supplier s)
{
  __TRACE__
  URL* u1 = ((URL*)args[0].addr);
  URL* u2 = ((URL*)args[1].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((CcBool*)result.addr)->Set(true, *u1 == *u2);
  return 0;
}

int
ISWebequalHtmlFun (Word* args, Word& result, int message, 
                   Word& local, Supplier s)
{
  __TRACE__
  HTML* h1 = ((HTML*)args[0].addr);
  HTML* h2 = ((HTML*)args[1].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((CcBool*)result.addr)->Set(true, *h1 == *h2);
  return 0;
}

int
ISWebequalPageFun (Word* args, Word& result, int message, 
                   Word& local, Supplier s)
{
  __TRACE__
  Page* p1 = ((Page*)args[0].addr);
  Page* p2 = ((Page*)args[1].addr);

  result = qp->ResultStorage(s);  //query processor has provided
            //a result instance to take the result

  ((CcBool*)result.addr)->Set(true, *p1 == *p2);
  return 0;
}

/*
6.2.21 Value Mapping Array for Operators ~webequal, wget, pageget,htmlget~

*/
ValueMapping webequalMap[] = 
{ISWebequalUrlFun,ISWebequalHtmlFun,ISWebequalPageFun};
ValueMapping webwgetMap[] = 
{ISWebWgetFourParam,ISWebWgetFiveParam};
ValueMapping webpagegetMap[] = 
{ISWebPagegetFourParam,ISWebPagegetFiveParam};
ValueMapping webhtmlgetMap[] = 
{ISWebHtmlgetFourParam,ISWebHtmlgetFiveParam};


/*
6.3 Specifications

6.3.1 Specification of Operator ~protocol~

*/

const string protocolSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(url) -> text</text--->"
       "<text>protocol( url )</text--->"
       "<text>Returns the protocol of the url</text--->"
       "<text>protocol( url1 )</text--->"
       ") )";

/*
6.3.2 Specification of Operator ~host~

*/
const string hostSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(url) -> text</text--->"
       "<text>host( url )</text--->"
       "<text>Returns the host of the url</text--->"
       "<text>host( url1 )</text--->"
       ") )";

/*
6.3.3 Specification of Operator ~filename~

*/
const string filenameSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(url) -> text</text--->"
       "<text>filename( url )</text--->"
       "<text>Returns the filename with path</text--->"
       "<text>filename( url1 )</text--->"
       ") )";

/*
6.3.4 Specification of Operator ~source~

*/
const string sourceSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html or page) -> url</text--->"
       "<text>source( html/page )</text--->"
       "<text>Returns the url of the html/page</text--->"
       "<text>source( html1 )</text--->"
       ") )";

/*
6.3.5 Specification of Operator ~createurl~

*/
const string createurlSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(text) -> url</text--->"
       "<text>createurl( text )</text--->"
       "<text>Creates an url of the given text</text--->"
       "<text>createurl(text.../text--- )</text--->"
       ") )";

/*
6.3.6 Specification of Operator ~content~

*/
const string contentSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html) -> text</text--->"
       "<text>content( html )</text--->"
       "<text>Returns the content without tags</text--->"
       "<text>content(html1)</text--->"
       ") )";

/*
6.3.7 Specification of Operator ~urls~

*/
const string urlsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html or page) -> stream(url)</text--->"
       "<text>urls( html/page )</text--->"
       "<text>Returns all urls of the given object</text--->"
       "<text>urls(html1)</text--->"
       ") )";

/*
6.3.8 Specification of Operator ~containsurl~

*/
const string containsurlSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html or page x url) -> bool</text--->"
       "<text>containsurl( html/page, url )</text--->"
       "<text>Checks if the given html contains the given url</text--->"
       "<text>containsurl(html1,url1)</text--->"
       ") )";

/*
6.3.9 Specification of Operator ~lastmodified~

*/
const string lastmodifiedSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html) -> instant</text--->"
       "<text>lastmodified( html )</text--->"
       "<text>Returns the last modified date of the given html</text--->"
       "<text>lastmodified(html1)</text--->"
       ") )";

/*
6.3.10 Specification of Operator ~metainfo~

*/
const string metainfoSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html x string) -> text</text--->"
       "<text>metainfo( html, string )</text--->"
       "<text>Returns the metainfo for the key or an empty string</text--->"
       "<text>metainfo(html1, \"content\")</text--->"
       ") )";

/*
6.3.11 Specification of Operator ~metainfos~

*/
const string metainfosSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html) -> stream(tuple([Key:string,Content:text]))</text--->"
       "<text>metainfos( html )</text--->"
       "<text>Returns all metainfos of the given html with key</text--->"
       "<text>metainfos(html1)</text--->"
       ") )";

/*
6.3.12 Specification of Operator ~numberof~

*/
const string numberofSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(html x string)-> int</text--->"
       "<text>numberof( html, string )</text--->"
       "<text>counts the given string in the html</text--->"
       "<text>numberof(html1,\"test\")</text--->"
       ") )";

/*
6.3.13 Specification of Operator ~similar~

*/
const string similarSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(html x html x int x bool) -> real</text--->"
       "<text>similar( html,html,depth,follow order )</text--->"
       "<text>calc.how similar the two htmls are to the given depth</text--->"
       "<text>similar(html1,html2,0,true)</text--->"
       ") )";


/*
6.3.14 Specification of Operator ~extracthtml~

*/
const string extracthtmlSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>page -> html</text--->"
       "<text>extracthtml( page )</text--->"
       "<text>returns the html file of the given page</text--->"
       "<text>extracthtml(page1)</text--->"
       ") )";

/*
6.3.15 Specification of Operator ~numoffiles~

*/
const string numoffilesSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>page -> int</text--->"
       "<text>numoffiles( page )</text--->"
       "<text>returns the number of the embedded objects</text--->"
       "<text>numoffiles(page1)</text--->"
       ") )";

/*
6.3.16 Specification of Operator ~getfiles~

*/
const string getfilesSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>page -> stream(tuple([Source:url,"
       " Type:string, File:binfile]))</text--->"
       "<text>getfiles( page1 )</text--->"
       "<text>returns a stream of tuples with all embedded files</text--->"
       "<text>getfiles(page1)</text--->"
       ") )";

/*
6.3.16 Specification of Operator ~wget~

*/
const string wgetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(url x bool x int x text x map:url->bool) ->"
       " stream(tuple([Source:url, Type:string, File:binfile]))</text--->"
       "<text>wget( url,extLinks,depth,hosts[,filterFkt] )</text--->"
       "<text>loads the given url and dependent files to depth d</text--->"
       "<text>wget(url1,TRUE,2, <text...</text...,\n"
       "fun(u:url) host(u) contains \"www\") consume</text--->"
       ") )";

/*
6.3.16 Specification of Operator ~pageget~

*/
const string pagegetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(url x bool x int x text x map:url->bool) ->"
       " stream(tuple([Source:url, Page:page]))</text--->"
       "<text>pageget( url,extLinks,depth,hosts[,filterFkt] )</text--->"
       "<text>loads the given html-url and dependent html pages</text--->"
       "<text>pageget(url1,TRUE,2, <text...</text...) consume</text--->"
       ") )";

/*
6.3.16 Specification of Operator ~htmlget~

*/
const string htmlgetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
        "\"Example\" ) "
        "( <text>(url x bool x int x text x map:url->bool)"
        " -> stream(tuple([Source:url, Html:html]))</text--->"
        "<text>htmlget( url,extLinks,depth,hosts[,filterFkt] )</text--->"
        "<text>loads the given html-url and dependent html pages</text--->"
        "<text>htmlget(url1,TRUE,2, <text...</text...) consume</text--->"
        ") )";

/*
6.3.16 Specification of Operator ~webequal~

*/
const string webequalSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>t element of {url,html,page} ->t</text--->"
       "<text>webequal( html1, html )</text--->"
       "<text>returns true if the params equal else false</text--->"
       "<text>webequal(html1, html2)</text--->"
       ") )";

/*
6.4 Definition of Operators

6.4.1 Definition of Operator ~protocol~

*/

Operator webprotocol (
  "protocol",     //name
  protocolSpec,         //specification
  protocolFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  protocolHostFilenameTypeMap    //type mapping
);

/*
6.4.2 Definition of Operator ~host~

*/

Operator webhost (
  "host",     //name
  hostSpec,         //specification
  hostFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  protocolHostFilenameTypeMap    //type mapping
);

/*
6.4.3 Definition of Operator ~filename~

*/

Operator webfilename (
  "webfilename",     //name
  filenameSpec,         //specification
  filenameFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  protocolHostFilenameTypeMap    //type mapping
);

/*
6.4.4 Definition of Operator ~source~

*/

Operator websource (
  "source",     //name
  sourceSpec,         //specification
  sourceFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  sourceTypeMap    //type mapping
);

/*
6.4.5 Definition of Operator ~createurl~

*/

Operator webcreateurl (
  "createurl",     //name
  createurlSpec,         //specification
  createurlFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  createurlTypeMap    //type mapping
);

/*
6.4.6 Definition of Operator ~content~

*/

Operator webcontent (
  "content",     //name
  contentSpec,         //specification
  contentFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  contentTypeMap    //type mapping
);

/*
6.4.7 Definition of Operator ~urls~

*/

Operator weburls (
  "urls",     //name
  urlsSpec,         //specification
  urlsFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  urlsTypeMap    //type mapping
);

/*
6.4.8 Definition of Operator ~containsurl~

*/

Operator webcontainsurl (
  "containsurl",     //name
  containsurlSpec,         //specification
  containsurlFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  containsurlTypeMap    //type mapping
);

/*
6.4.9 Definition of Operator ~lastmodified~

*/

Operator weblastmodified (
  "lastmodified",     //name
  lastmodifiedSpec,         //specification
  lastmodifiedFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  lastmodifiedTypeMap    //type mapping
);

/*
6.4.10 Definition of Operator ~metainfo~

*/

Operator webmetainfo (
  "metainfo",     //name
  metainfoSpec,         //specification
  metainfoFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  metainfoTypeMap    //type mapping
);

/*
6.4.11 Definition of Operator ~metainfos~

*/

Operator webmetainfos (
  "metainfos",     //name
  metainfosSpec,         //specification
  metainfosFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  metainfosTypeMap    //type mapping
);

/*
6.4.12 Definition of Operator ~numberof~

*/

Operator webnumberof (
  "numberof",     //name
  numberofSpec,         //specification
  numberofFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  numberofTypeMap    //type mapping
);

/*
6.4.13 Definition of Operator ~similar~

*/

Operator websimilar (
  "similar",     //name
  similarSpec,         //specification
  similarFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  similarTypeMap    //type mapping
);

/*
6.4.14 Definition of Operator ~extracthtml~

*/

Operator webextracthtml (
  "extracthtml",     //name
  extracthtmlSpec,         //specification
  extracthtmlFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  extracthtmlTypeMap    //type mapping
);

/*
6.4.15 Definition of Operator ~numoffiles~

*/

Operator webnumoffiles (
  "numoffiles",     //name
  numoffilesSpec,         //specification
  numoffilesFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  numoffilesTypeMap    //type mapping
);

/*
6.4.16 Definition of Operator ~getfiles~

*/

Operator webgetfiles (
  "getfiles",     //name
  getfilesSpec,         //specification
  getfilesFun,    //value mapping
  Operator::SimpleSelect,  //trivial selection function
  getfilesTypeMap    //type mapping
);

/*
6.4.17 Definition of Operator ~wget~

*/

Operator webwget (
  "wget",     //name
  wgetSpec,         //specification
  2,      //number of functions
  webwgetMap,  //value mapping
  webwget_pagegetSelect,  //trivial selection function
  wgetTypeMap    //type mapping
);

/*
6.4.18 Definition of Operator ~pageget~

*/

Operator webpageget (
  "pageget",     //name
  pagegetSpec,         //specification
  2,      //number of functions
  webpagegetMap,  //value mapping
  webwget_pagegetSelect,  //trivial selection function
  pagegetTypeMap    //type mapping
);

/*
6.4.18 Definition of Operator ~htmlget~

*/

Operator webhtmlget (
  "htmlget",     //name
  htmlgetSpec,         //specification
  2,      //number of functions
  webhtmlgetMap,  //value mapping
  webwget_pagegetSelect,  //trivial selection function
  htmlgetTypeMap    //type mapping
);

/*
6.4.19 Definition of Operator ~wegequal~

*/

Operator webequal (
  "webequal",     //name
  webequalSpec,  //specification
  3,      //number of functions
  webequalMap,  //value mapping
  webequalSelect,  //trivial selection function
  webequalTypeMap    //type mapping
);

/*
7. Algebra

*/
class WebAlgebra : public Algebra
{
 public:
  WebAlgebra() : Algebra()
  {
    AddTypeConstructor( &url );
    url.AssociateKind("DATA");       
    AddTypeConstructor( &html );
    html.AssociateKind("DATA");       
    AddTypeConstructor( &page );
    page.AssociateKind("DATA");       
  
    AddOperator( &webprotocol );
    AddOperator( &webhost );
    AddOperator( &webfilename );
    AddOperator( &websource );
    AddOperator( &webcreateurl );
    AddOperator( &webcontent );
    AddOperator( &weburls );
    AddOperator( &webcontainsurl );
    AddOperator( &weblastmodified );
    AddOperator( &webmetainfo );
    AddOperator( &webmetainfos );
    AddOperator( &webnumberof );
    AddOperator( &websimilar );
    AddOperator( &webextracthtml );
    AddOperator( &webnumoffiles );
    AddOperator( &webgetfiles );
    AddOperator( &webwget );
    AddOperator( &webpageget );
    AddOperator( &webhtmlget );
    AddOperator( &webequal );
  }
  ~WebAlgebra() {};
};
 
WebAlgebra webAlgebra;



/*
8. Initialization

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
InitializeWebAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&webAlgebra);
}


