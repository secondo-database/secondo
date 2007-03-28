/*

[/1] Header File: Web Algebra

1 Defines and Includes

1.1 Includes

*/

#ifndef SEC_WEBLEX_H
#define SEC_WEBLEX_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <list>

#ifndef yyFlexLexer
#define yyFlexLexer webFlexLexer
#include <FlexLexer.h>
#endif
#include <string>
#include <vector>
#include <list>
/*

1.2 Defintions

1.2.1 Start conditions for WebFlex

*/

#define MSCHEME 1000
#define MAUTHORITY 1001
#define MPATH 1002
#define MURI    1003
#define PHTML    1004
#define EXTRACT 1005
#define RELEM_WA 1006
#define RELEM 1007
#define FINDELEMSTART 1008
#define RSCRIPT 1009

/*

1.2.2 Return values of lexical analyses

*/

#define SCHEME 1000
#define AUTHORITY 1001
#define PATH    1002
#define URI 1003
#define OPEN_TAG 1004
#define CLOSE_TAG 1005
#define HREF 1006
#define ANCHOR 1007
#define EIDENTIFIER 1008
#define ATTVALUE 1009
#define COMMENT 1010
#define CONTENT 1011
#define ELEMENT 1012
#define ELEMENT_SA 1013
#define ELEMENT_CLOSE 1014
#define RURI 1015
#define CONTENT_END    1016
#define SEARCH_ELEMENT_START 1017
#define ERROR    10000
            

/*

1.3 Declcaration of WebLex. 

WebLex is a scanncer derived from yyFlexLexer.
The scanner can analyse URI's, and tokens of a html document

*/
using namespace std;

bool isEqual (string s1, string s2);
int SplitString(const string& input, const string& delimiter, 
                vector<string>& results,  bool includeEmpties);
bool isWhite(char c);


typedef struct FlobIndex{
    unsigned long offset;
    unsigned long len;
} flobindex;

class WebLex : public yyFlexLexer {
    public:
        WebLex(istream*);
        int nextToken(void);
        int nextToken(int);
        string getVal(void);
        int yylex (int);
        int yylex (void);
        void switchStartCond(int);
        bool findAttribute(string attribute, string& value);
        bool findAttribute(vector<string>& attributes, 
                                   string& value, string & attribute);
        int startElement (string& element);
        flobindex setPos(string value, const string& content);
        int readContent();
        int readContentTmp();
    
    private:
        string tokenVal;
        int switchState;
        unsigned long pos;
        
};

/*
1.4 Declaration of class AnalyseElement.

Each instance of this class represent the occurences of a element in a html document.

*/

class AnalyseElement {
    public:
        AnalyseElement (string e){
            element= e;
            occurrences=1;
        }
        
        
        AnalyseElement (string e, int s){
            element= e;
            occurrences=1;
            symbol=s;
        }        

        bool operator<(AnalyseElement ae) {
            return  ( occurrences < ae.getOccurrences());
        }
        
        int getOccurrences(){
            return occurrences;
        }
        
        int increment(void){
            occurrences++;
            return occurrences;
        }
        
        string getElement() const{
            return element;
        }
        
        int getSymbol() const{
            return symbol;
        }
        
        int getOccurrences() const {
            return occurrences;
        }
        
        void setDepth(int i){
            depth=i;
        }
        
        int getDepth(){
            return depth;
        }
    
    private:
        string element;
        int occurrences;
        int symbol;
        int depth;
        
};

/*

1.5 Declaration of class AnalyseList

*/

class AnalyseList : public list<AnalyseElement> {
    public:
        void add(string e){
            AnalyseList::iterator it;
            
            it=begin();
            


            while (it != end() && (it->getElement() != e))
                it++;    

            

            if (it != end())
                it->increment();
            else
                list<AnalyseElement>::push_back(AnalyseElement (e));
            
        }
        
        
        void push_back(string el){
            AnalyseElement e(el);
            
            list<AnalyseElement>::push_back(e);
        }
        
        bool find(AnalyseList::const_iterator it, string e){

            while (it != end() && (it->getElement() != e))
                it++;    
            if (it != end())
                return true;
            
            return false;
            
        }
};


/*

1.6 

*/

#endif

