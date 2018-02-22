
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

1 Extending the NMEAImporter.

If there are nmea files containing non-supported sentences, the nmeaimporter can
be extended to these new sentences.

The this end, two things are to do:

1) Writing a class doing the conversion. This class must be inherited from
   class NMEALineImporter and has to overwrite all pure virtual functions.
   For more information, see this class.

2) Add an instance of the new class to the vector of avialable
   nmeaLineImporters in the constructor of the NMEAImporter class.

//[$][\$]

*/

#include <vector>
#include "NestedList.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "DateTime.h"
#include "NMEAImporter.h"
#include "StringUtils.h"
#include "Symbols.h"

extern NestedList* nl;

using namespace std;

/*
1 Abstract class NMEALineImporter

This class is the super class of all LineImporters.

*/
class NMEALineImporter{
  public:
     virtual ~NMEALineImporter(){}

/*
~accept~

This fucntion checks whether the managed sentence corresponds to the
given string.
For example, within a class managing [$]--GGA sentences, the class
checks whether
the given string is "GGA".

*/
     virtual bool accept(const string& DataSetName)const {
        return DataSetName == getAcceptedType();
     }

/*
~getTupleTypeAsList~

This functions returns the type of the tuple which is created by this
NMEALineImporter, e.g. (tuple ((Time instant)(Lat real)(Lon real))).

*/
     virtual ListExpr getTupleTypeAsList() const = 0;

/*
~createTupleForLine~

Creates a new Tuple from a description. This function must be able to
handle arbitrary strings. If the list is not of the correct format, e.g.
contains not the sentence managed by this importer, the result will be
0.

*/

     virtual Tuple* getTupleForLine(const string& line)=0;

/*
~getAcceptedType~

Returns the sentence ID of the nmea data set which can be imported by
this importer, e.g. an importer managing [$]--GGA sentences returns "GGA"
in this function.

*/

     virtual string getAcceptedType() const=0;
};


namespace nmea_stringutils{

/*
~char2int~

Converts a character representing a digit into the coressponding
interger value. There is no check for validity.

*/
  int32_t char2int(const char c){
    return c - '0';
  }



/*
~getutc~

This function converts a string formatted as hhmmss.ss into an instant.
If the string is not well formatted, the result will be false. Otherwise, the
day of __currentDay__ , the hour, the minute etc. of the utc code is
used to create an instant returned in result.


*/
  bool getutc(const string& utc,
              const datetime::DateTime& currentDay,
              datetime::DateTime& result){
     stringutils::StringTokenizer st(utc, ".");
     if(!st.hasNextToken()){
        return false;
     }
     string first = st.nextToken();
     string second = "000";
     if(st.hasNextToken()){
        second = st.nextToken();
     }
     if(st.hasNextToken()){
       return false;
     }
     trim(first);
     trim(second);
     // ensure that first and second consist of digits only
     if(first.find_first_not_of("1234567890") != string::npos){
        return false;
     }
     if(second.find_first_not_of("1234567890") != string::npos){
        return false;
     }
     if(first.length()>6){
       return false;
     }
     if(second.length()>3){
       return false;
     }
     int32_t seconds = 0;
     int32_t milliseconds = 0;
     // first, compute milliseconds
     uint32_t m = 100;
     for(unsigned int i=0;i<second.length(); i++){
        milliseconds += m*char2int(second[i]);
        m = m / 10;
     }



     int ms[] = {36000, 3600, 600, 60, 10 , 1}; // mutilple for the
     for(unsigned int i=0; i<first.length(); i++){
       int factor = ms[5-i];
       char c = first[first.length()-(i+1)];
       int num = char2int(c);
       seconds += factor*num;
     }

     milliseconds = milliseconds + 1000*seconds;

     result = currentDay;
     //result.setToMidnight();
     datetime::DateTime d(0,milliseconds,datetime::durationtype);
     result += d;
     return true;
  }

/*
~getUtc~

Returns a DateTime instance corresponding to the first element of the
StringTokenizer given as argument. If the tokenizer does not have a
element or the first element does not represent a valid UTC in nmea
format, the result will be an undefined instant.

*/
datetime::DateTime* getUtc(stringutils::StringTokenizer& st){
   datetime::DateTime* res = new datetime::DateTime(datetime::instanttype);
   datetime::DateTime zero(datetime::instanttype);
   if(!st.hasNextToken()){
       res->SetDefined(false);
       return res;
   }
   string utc = st.nextToken();
   if(!getutc(utc, zero, *res)){
        res->SetDefined(false);
   }
   return res;
}


/*
~getInt~

Returns the integer represented by the given string. Leading or following
spaces are ignored. If the values does not represent a valid integer value,
 the result will be
false.

*/
bool getInt(string val, int& result){
  trim(val);
  if(val.length()==0){
    return false;
  }

  int signum=1;
  if(val[0]=='-'){
     signum = -1;
     val = val.substr(1);
     trim(val);
  }


  if(val.find_first_not_of("0123456789")!=string::npos){
      return  false;
  }
  int m = 1;
  result = 0;
  size_t len = val.length();
  if(len==0){
    return false;
  }

  for(size_t i=0; i< len; i++){
     result += m*char2int(val[len-(i+1)]);
     m = m * 10;
  }
  result = result * signum;
  return  true;
}

/*
~getReal~

Converts a string formatted as xxx.xx into a  double value.

*/
bool getReal(const string& s, double& result){
   stringutils::StringTokenizer st(s,".");
   if(!st.hasNextToken()){
     return false;
   }
   string first = st.nextToken();
   int p1;
   if(!getInt(first,p1)){
     return false;
   }
   result = p1;
   if(!st.hasNextToken()){
     return true;
   }
   string second = st.nextToken();
   stringutils::trim(second);
   if(second.length()==0){
      return true;
   }
   int p2;
   if(!getInt(second,p2)){
     return false;
   }
   double t = p2 / pow(10.0f, (int)second.length());
   result += t;
   return true;
}


/*
~getDate~

Creates an instant from the first element of the stringtokenizer.
If the stringtokenizer is exhausted or the fisrt element is not
a string containing 6 digits, the result will be an undefined instant.

*/
datetime::DateTime* getDate(stringutils::StringTokenizer& st){
   int shift = 71; // shift from 1900 to 2000

   datetime::DateTime* res = new datetime::DateTime(datetime::instanttype);

   if(!st.hasNextToken()){
     res->SetDefined(false);
     return res;
   }
   string s = st.nextToken();

   if(s.length()!=6){
     res->SetDefined(false);
     return res;
   }
   string dd = s.substr(0,2);
   string mm = s.substr(2,2);
   string yy = s.substr(4,2);
   trim(dd);
   trim(mm);
   trim(yy);
   int d;
   int m;
   int y;
   if(!getInt(dd,d) || !getInt(mm,m) || !getInt(yy,y)){
     res->SetDefined(false);
     return res;
   }
   if(y<shift){
      y += 2000;
   } else {
      y+=1900;
   }
   res->Set(y,m,d);
   return res;
}

/*
~getdecimal~

Converts a string formatted as ggmm.mmmm into its decimal value.

*/
bool getDecimal(const string& val, double& result){



  stringutils::StringTokenizer st(val,".");
  if(!st.hasNextToken()){
     return false;
  }
  string ggmm = st.nextToken();
  stringutils::trim(ggmm);
  int p1;
  if(!getInt(ggmm,p1)){
    return false;
  }
  if(p1<0){
     return false;
  }

  // convert into minutes
  int minutes = p1%100;
  minutes += (p1/100)* 60;
  if(!st.hasNextToken()){
     result = (double)(minutes)/60.0;
     return true;
  }




  string mmmm = st.nextToken();
  if(st.hasNextToken()){
    return false;
  }
  if(!getInt(mmmm,p1)){
    return false;
  }
  stringutils::trim(mmmm);
  if(p1<0){
    return false;
  }
  result = minutes + p1 / pow(10.0f, (int) mmmm.length());
  result = result/60.0; // convert minutes into degree
  return true;
}





/*
~getLat~

Converts two strings (first representing a real value, second
represneting "N" or "S") into a real number.

*/
bool getLat(const string& val, const string& p, double& result){
   if(p.length()!=1){
     return false;
   }
   char c = tolower(p[0]);
   double signum = 1.0;
   if(c=='s'){
     signum = -1.0;
   } else if(c!='n'){
     return false;
   }
   if(!getDecimal(val,result)){
     return false;
   }
   result = result * signum;
   return true;
}

/*
~getLat~

Converts the first two elements of the Stringtokenizer
into a real value. The strings must form a latitude decription in
nmea format, i.e. ddmm.mmm  R, where R is in (N,S).

*/

CcReal* getLat(stringutils::StringTokenizer& st){
  CcReal* result = new CcReal(false,0.0);

  if(!st.hasNextToken()){
     return result;
  }
  string v = st.nextToken();
  if(!st.hasNextToken()){
     return result;
  }
  string p = st.nextToken();
  trim(v);
  trim(p);
  double r;
  if(! getLat(v,p,r)){
    return result;
  }
  result->Set(true,r);
  return result;
}



/*
~getLon~

Converts two strings (first representing a real value,
 second represneting "N" or "S") into
a real number (longitude).

*/
bool getLon(const string& val, const string& p, double& result){
   if(p.length()!=1){
     return false;
   }
   char c = tolower(p[0]);
   double signum = 1.0;
   if(c=='w'){
     signum = -1.0;
   } else if(c!='e'){
     return false;
   }
   if(!getDecimal(val,result)){
     return false;
   }
   result = result * signum;
   return true;
}

/*
~getLon~

Converts the first two elements of the Stringtokenizer
into a real value. The strings must form a longitude decription in
nmea format, i.e. ddmm.mmm  R, where R is in (E,W).

*/

CcReal* getLon(stringutils::StringTokenizer& st){
  CcReal* result = new CcReal(false,0.0);

  if(!st.hasNextToken()){
     return result;
  }
  string v = st.nextToken();
  if(!st.hasNextToken()){
     return result;
  }
  string p = st.nextToken();
  trim(v);
  trim(p);
  double r;
  if(! getLon(v,p,r)){
    return result;
  }
  result->Set(true,r);
  return result;
}

/*
~getString~

Converts the first eleent of a string tokenizer to a CcString instance.
If the tokenizer is empty or the length of the first element is smaller
than minLength, an undefined CcString is returned.

*/
CcString* getString(stringutils::StringTokenizer& st,
                    const unsigned int minLength){
   CcString* result = new CcString(false,"");
   if(st.hasNextToken()){
      string s = st.nextToken();
      if(s.length()>=minLength){
         result->Set(true,s);
      }
   }
   return result;
}


/*
~getInt~

Converts the first element of ~st~ into a CcInt. If ~st~ is empty, or
does not form an integer, the result is undefined. If allowTrim is set to
be true, white spces at the begin and the end of the first element of ~st~
are ignored.

*/
CcInt* getInt(stringutils::StringTokenizer& st, const bool allowTrim = true){
  CcInt* result = new CcInt(false,0);
  if(st.hasNextToken()){
     string s = st.nextToken();
     if(allowTrim){
        trim(s);
     }
     int v;
     if(getInt(s,v)){
        result->Set(true,v);
     }
  }
  return result;
}


/*
~getReal~

Converts the first element of ~st~ into a CcReal. If ~st~ is empty, or
does not form a real numer in nmea format, i.e. xxx.xxx, the result is
undefined. If allowTrim is set to be true, white spces at the begin and
the end of the first element of ~st~
are ignored.

*/
CcReal* getReal(stringutils::StringTokenizer& st, const bool allowTrim = true){
  CcReal* result = new CcReal(false,0.0);
  if(st.hasNextToken()){
     string s = st.nextToken();
     if(allowTrim){
        trim(s);
     }
     double v;
     if(getReal(s,v)){
        result->Set(true,v);
     }
  }
  return result;
}




} // end of namespace nmea_stringutils





/*
1 class GGAImporter

*/

class GGAImporter: public NMEALineImporter{

   public:

/*
1.1 Constructor.

Initialized some members.

*/

     GGAImporter():NMEALineImporter(),
                   currentDay(datetime::durationtype),
                   lastTime(datetime::instanttype),
                   oneDay(1,0,datetime::durationtype),
                   tupleType(0),
                   typeId("GGA"){

        lastTime.SetDefined(false);
        ListExpr numtype = SecondoSystem::GetCatalog()->
                           NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

/*
1.2 ~Destructor~

*/

     virtual ~GGAImporter(){
       tupleType->DeleteIfAllowed();
       tupleType=0;
     }

/*
1.3 ~getAcceptedType~

The GGAImporter accepts GGA.

*/

    string getAcceptedType() const{
      return "GGA";
    }

/*
1.4 ~getTupleTypeAsList~

Returns the type of the tuple when converting a GGA sentence.

*/

     ListExpr getTupleTypeAsList() const{
        ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                                    nl->SymbolAtom("SenID"),
                                        nl->SymbolAtom(CcString::BasicType())));
        ListExpr last = attrList;

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Utc"),
                                        nl->SymbolAtom(Instant::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Lat"),
                                        nl->SymbolAtom(CcReal::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Lon"),
                                        nl->SymbolAtom(CcReal::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("FixQuality"),
                                        nl->SymbolAtom(CcInt::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("NoSats"),
                                        nl->SymbolAtom(CcInt::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("HDilution"),
                                        nl->SymbolAtom(CcReal::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Alt"),
                                        nl->SymbolAtom(CcReal::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("AltUnit"),
                                        nl->SymbolAtom(CcString::BasicType())));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Height"),
                                        nl->SymbolAtom(CcReal::BasicType())));


        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("HeightUnit"),
                                        nl->SymbolAtom(CcString::BasicType())));


        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("TimeLastUpdate"),
                                        nl->SymbolAtom(CcReal::BasicType())));


        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("DGPSStationID"),
                                        nl->SymbolAtom(CcInt::BasicType())));

        // ommit checksum

        return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList);

     }
/*
1.6 getTupleForLine

Creates tuple from a description if possible, 0 otherwise.

*/
     Tuple* getTupleForLine(const string& line) {

        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return 0;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return 0;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return 0;
        }
        Tuple* t = new Tuple(tupleType);
        t->PutAttribute(0, new CcString(true, s.substr(1,2)));

        datetime::DateTime* utc = nmea_stringutils::getUtc(st);
        if(utc->IsDefined()){
            utc->Add(&currentDay);
            if(lastTime.IsDefined()){
               if(*utc < lastTime){
                   utc->Add(&oneDay);
                   currentDay.Add(&oneDay);
               }
            }
            lastTime = *utc;
        }
        t->PutAttribute(1, utc);
        t->PutAttribute(2, nmea_stringutils::getLat(st));
        t->PutAttribute(3, nmea_stringutils::getLon(st));
        t->PutAttribute(4, nmea_stringutils::getInt(st)); // fixQ
        t->PutAttribute(5, nmea_stringutils::getInt(st)); // noSats
        t->PutAttribute(6, nmea_stringutils::getReal(st)); // HDil
        t->PutAttribute(7, nmea_stringutils::getReal(st)); // Alt
        t->PutAttribute(8, nmea_stringutils::getString(st,1)); // AltUnit
        t->PutAttribute(9, nmea_stringutils::getReal(st)); // Heigt
        t->PutAttribute(10, nmea_stringutils::getString(st,1)); // HeightUnit
        t->PutAttribute(11, nmea_stringutils::getReal(st)); // lastupdateTime
        CcString* lastStationId= new CcString(false,"");
        if(st.hasNextToken()){
           stringutils::StringTokenizer st2(st.nextToken(),"*");
           if(st2.hasNextToken()){
              string s = st2.nextToken();
              if(s.length()>0){
                 lastStationId->Set(true,s);
              }
           }
        }
        t->PutAttribute(12, lastStationId );


       return t;
     }


   private:
      datetime::DateTime currentDay;
      datetime::DateTime lastTime;
      datetime::DateTime oneDay;
      TupleType* tupleType;
      const string typeId;
};

/*
3 Class GSAImporter

Class importing GSA sentences.

*/
class GSAImporter : public NMEALineImporter{
 public:

     GSAImporter() {
        ListExpr numtype = SecondoSystem::GetCatalog()->
                           NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

     ~GSAImporter(){
         tupleType->DeleteIfAllowed();
         tupleType=0;
      }

     ListExpr getTupleTypeAsList()const{
        ListExpr attrList = nl->OneElemList(
                                   nl->TwoElemList(nl->SymbolAtom("SenId"),
                                       nl->SymbolAtom(CcString::BasicType())));

        ListExpr last = attrList;
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("FixSel"),
                                        nl->SymbolAtom(CcString::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("Fix3D"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN1"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN2"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN3"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN4"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN5"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN6"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN7"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN8"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN9"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN10"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN11"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN12"),
                                           nl->SymbolAtom(CcInt::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PDop"),
                                          nl->SymbolAtom(CcReal::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("HDop"),
                                          nl->SymbolAtom(CcReal::BasicType())));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("VDop"),
                                          nl->SymbolAtom(CcReal::BasicType())));
        return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),attrList);


     }

     virtual Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return 0;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return 0;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return 0;
        }
        Tuple* t = new Tuple(tupleType);
        t->PutAttribute(0, new CcString(true, s.substr(1,2)));
        t->PutAttribute(1, nmea_stringutils::getString(st,1));
        t->PutAttribute(2, nmea_stringutils::getInt(st));
        t->PutAttribute(3, nmea_stringutils::getInt(st));
        t->PutAttribute(4, nmea_stringutils::getInt(st));
        t->PutAttribute(5, nmea_stringutils::getInt(st));
        t->PutAttribute(6, nmea_stringutils::getInt(st));
        t->PutAttribute(7, nmea_stringutils::getInt(st));
        t->PutAttribute(8,nmea_stringutils::getInt(st));
        t->PutAttribute(9,nmea_stringutils::getInt(st));
        t->PutAttribute(10,nmea_stringutils::getInt(st));
        t->PutAttribute(11, nmea_stringutils::getInt(st));
        t->PutAttribute(12, nmea_stringutils::getInt(st));
        t->PutAttribute(13, nmea_stringutils::getInt(st));
        t->PutAttribute(14,nmea_stringutils::getInt(st));
        t->PutAttribute(15,nmea_stringutils::getReal(st));
        t->PutAttribute(16,nmea_stringutils::getReal(st));
        CcReal* vdop = new CcReal(false,0.0);
        if(st.hasNextToken()){
           string s = st.nextToken();
           stringutils::StringTokenizer st2(s,"*");
           if(st2.hasNextToken()){
               double v;
               if(nmea_stringutils::getReal(st2.nextToken(),v)){
                   vdop->Set(true,v);
               }

           }
        }
        t->PutAttribute(17,vdop);
        return t;
     }

     string getAcceptedType() const{
         return "GSA";
     }

 private:
    TupleType* tupleType;

};

/*
4 RMCImporter

Class importing RMC sentences


*/

class RMCImporter: public NMEALineImporter{

    public:


     RMCImporter() {
        ListExpr numtype = SecondoSystem::GetCatalog()->
                            NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

     ~RMCImporter(){
         tupleType->DeleteIfAllowed();
         tupleType=0;
      }

     ListExpr getTupleTypeAsList() const {
       ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                  nl->SymbolAtom("SenID"),
                                  nl->SymbolAtom(CcString::BasicType())));
       ListExpr last = attrList;
       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Time"),
                                        nl->SymbolAtom(Instant::BasicType())));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Status"),
                                        nl->SymbolAtom(CcString::BasicType())));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lat"),
                                        nl->SymbolAtom(CcReal::BasicType())));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lon"),
                                        nl->SymbolAtom(CcReal::BasicType())));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Speed"),
                                        nl->SymbolAtom(CcReal::BasicType())));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Angle"),
                                        nl->SymbolAtom(CcReal::BasicType())));


       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MagV"),
                                        nl->SymbolAtom(CcReal::BasicType())));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MagS"),
                                        nl->SymbolAtom(CcString::BasicType())));
       return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList);
     }

     virtual Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return 0;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return 0;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return 0;
        }
        Tuple* t = new Tuple(tupleType);
        t->PutAttribute(0, new CcString(true, s.substr(1,2)));
        // attribute 1 is time, put later
        datetime::DateTime* utc = nmea_stringutils::getUtc(st);
        t->PutAttribute(2, nmea_stringutils::getString(st,1));
        t->PutAttribute(3, nmea_stringutils::getLat(st));
        t->PutAttribute(4, nmea_stringutils::getLon(st));
        t->PutAttribute(5, nmea_stringutils::getReal(st));
        t->PutAttribute(6, nmea_stringutils::getReal(st));
        datetime::DateTime* date = nmea_stringutils::getDate(st);
        datetime::DateTime* time = new datetime::DateTime(date->GetDay(),
                                                 utc->GetAllMilliSeconds(),
                                                 datetime::instanttype);
        delete utc;
        delete date;
        t->PutAttribute(1,time);
        t->PutAttribute(7,nmea_stringutils::getReal(st));
        CcString* magS = new CcString(false,"");
        if(st.hasNextToken()){
           string s = st.nextToken();
           stringutils::StringTokenizer st2(s,"*");
           if(st2.hasNextToken()){
                string s2 = st2.nextToken();
                if(s2.length()>0){
                  magS->Set(true,s2);
                }
           }
        }
        t->PutAttribute(8,magS);
        return t;


     }

     virtual string getAcceptedType() const{
       return "RMC";
     }


 private:
    TupleType* tupleType;
};


/*
4 Class ZDAImporter

*/

class ZDAImporter : public NMEALineImporter{
  public:

     ZDAImporter() {
        ListExpr numtype = SecondoSystem::GetCatalog()->
                            NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

     ~ZDAImporter(){
         tupleType->DeleteIfAllowed();
         tupleType=0;
      }

      string getAcceptedType() const{
         return "ZDA";
      }

      ListExpr getTupleTypeAsList() const {
         ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                    nl->SymbolAtom("SenID"),
                                    nl->SymbolAtom(CcString::BasicType())));
         ListExpr last = attrList;
         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Time"),
                                      nl->SymbolAtom(Instant::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeZoneH"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeZoneM"),
                                          nl->SymbolAtom(CcInt::BasicType())));
         return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList);
     }

     Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return 0;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return 0;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return 0;
        }
        Tuple* t = new Tuple(tupleType);
        t->PutAttribute(0, new CcString(true, s.substr(1,2)));

        // extract time
        datetime::DateTime* utc = nmea_stringutils::getUtc(st);

        int d,m,y;
        if(!utc->IsDefined() ||
           !nmea_stringutils::getInt(st.nextToken(),d) ||
           !nmea_stringutils::getInt(st.nextToken(),m) ||
           !nmea_stringutils::getInt(st.nextToken(),y)){
           utc->SetDefined(false);
        } else {
           utc->Set(y,m,d, utc->GetHour(), utc->GetMinute() ,
                    utc->GetSecond(), utc->GetMillisecond());
        }
        t->PutAttribute(1,utc);
        t->PutAttribute(2, nmea_stringutils::getInt(st));
        CcInt* tzm = new CcInt(false,0);
        if(st.hasNextToken()){
          stringutils::StringTokenizer st2(st.nextToken(),"*");
          if(st2.hasNextToken()){
             int v;
             if(nmea_stringutils::getInt(st2.nextToken(),v)){
                tzm->Set(true,v);
              }
          }
        }
        t->PutAttribute(3,tzm);
        return t;
     }


  private:
     TupleType* tupleType;

};

/*
6 class GSVImporter


*/


class GSVImporter: public NMEALineImporter{
  public:

     GSVImporter() {
        ListExpr numtype = SecondoSystem::GetCatalog()->
                            NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

     ~GSVImporter(){
         tupleType->DeleteIfAllowed();
         tupleType=0;
      }

      string getAcceptedType() const{
         return "GSV";
      }

     virtual ListExpr getTupleTypeAsList() const{
         ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                    nl->SymbolAtom("SenID"),
                                    nl->SymbolAtom(CcString::BasicType())));
         ListExpr last = attrList;
         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("NoMsg"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MsgNo"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("NoSats"),
                                          nl->SymbolAtom(CcInt::BasicType())));


         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SatNo1"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Elevation1"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Azimut1"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SNR1"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SatNo2"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Elevation2"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Azimut2"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SNR2"),
                                          nl->SymbolAtom(CcInt::BasicType())));
         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SatNo3"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Elevation3"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Azimut3"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SNR3"),
                                          nl->SymbolAtom(CcInt::BasicType())));
         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SatNo4"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Elevation4"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Azimut4"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SNR4"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList);
     }

     virtual Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return 0;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return 0;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return 0;
        }
        Tuple* t = new Tuple(tupleType);
        t->PutAttribute(0, new CcString(true, s.substr(1,2)));
        // pocess noMsg, MsgNo, NoSats
        for(int i=0;i<3;i++){
           t->PutAttribute(i+1, nmea_stringutils::getInt(st));
        }
        for(int i=0;i<15; i++){  // 4 infos for 4 satellites except
                                //  the last info
                                 // because of checksum
           t->PutAttribute(i+4, nmea_stringutils::getInt(st));
        }
        CcInt* SNR4 = new CcInt(false,0);
        if(st.hasNextToken()){
          stringutils::StringTokenizer st2(st.nextToken(),"*");
          if(st2.hasNextToken()){
              int v;
              if(nmea_stringutils::getInt(st2.nextToken(),v)){
                SNR4->Set(true,v);
              }
          }
        }
        t->PutAttribute(19,SNR4);
        return t;
     }

  private:
     TupleType* tupleType;
};

/*
5 class GLLImporter

*/
class GLLImporter: public NMEALineImporter{
  public:

     GLLImporter(): currentDay(datetime::durationtype),
                    lastTime(datetime::instanttype),
                    oneDay(1,0,datetime::durationtype)  {
        lastTime.SetDefined(false);
        ListExpr numtype = SecondoSystem::GetCatalog()->
                            NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

     ~GLLImporter(){
         tupleType->DeleteIfAllowed();
         tupleType=0;
      }

      string getAcceptedType() const{
         return "GLL";
      }

     virtual ListExpr getTupleTypeAsList() const{
         ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                    nl->SymbolAtom("SenID"),
                                    nl->SymbolAtom(CcString::BasicType())));
         ListExpr last = attrList;
         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lat"),
                                        nl->SymbolAtom(CcReal::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lon"),
                                        nl->SymbolAtom(CcReal::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Time"),
                                        nl->SymbolAtom(Instant::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ActiveData"),
                                        nl->SymbolAtom(CcString::BasicType())));

         return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList);
      }

     virtual Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return 0;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return 0;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return 0;
        }
        Tuple* t = new Tuple(tupleType);
        t->PutAttribute(0, new CcString(true, s.substr(1,2)));
        t->PutAttribute(1, nmea_stringutils::getLat(st));
        t->PutAttribute(2, nmea_stringutils::getLon(st));
        datetime::DateTime* utc = nmea_stringutils::getUtc(st);
        if(utc->IsDefined()){
            utc->Add(&currentDay);
            if(lastTime.IsDefined()){
               if(*utc < lastTime){
                   utc->Add(&oneDay);
                   currentDay.Add(&oneDay);
               }
            }
            lastTime = *utc;
        }

        t->PutAttribute(3, utc);
        CcString* active = new CcString(false,"");
        if(st.hasNextToken()){
          stringutils::StringTokenizer st2(st.nextToken(),"*");
          if(st2.hasNextToken()){
              string s = st2.nextToken();
              if(s.length()>0){
                 active->Set(true,s);
              }
          }
        }
        t->PutAttribute(4, active);
        return t;
      }

  private:
     datetime::DateTime currentDay;
     datetime::DateTime lastTime;
     datetime::DateTime oneDay;
     TupleType* tupleType;

};


/*
7 Class GNSImporter

*/

class GNSImporter: public NMEALineImporter{
  public:

     GNSImporter(): currentDay(datetime::durationtype),
                    lastTime(datetime::instanttype),
                    oneDay(1,0,datetime::durationtype)  {
        lastTime.SetDefined(false);
        ListExpr numtype = SecondoSystem::GetCatalog()->
                            NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

     ~GNSImporter(){
         tupleType->DeleteIfAllowed();
         tupleType=0;
      }

      string getAcceptedType() const{
         return "GNS";
      }

     virtual ListExpr getTupleTypeAsList() const{
         ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                    nl->SymbolAtom("SenID"),
                                    nl->SymbolAtom(CcString::BasicType())));
         ListExpr last = attrList;
         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Time"),
                                        nl->SymbolAtom(Instant::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lat"),
                                        nl->SymbolAtom(CcReal::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lon"),
                                        nl->SymbolAtom(CcReal::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Mode"),
                                        nl->SymbolAtom(CcString::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("NoSats"),
                                          nl->SymbolAtom(CcInt::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("HDOP"),
                                          nl->SymbolAtom(CcReal::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Alt"),
                                          nl->SymbolAtom(CcReal::BasicType())));

         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("GeoidSep"),
                                          nl->SymbolAtom(CcReal::BasicType())));

         last = nl->Append(last, nl->TwoElemList(
                                      nl->SymbolAtom("AgeOfDiffData"),
                                      nl->SymbolAtom(CcReal::BasicType())));


         last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("StationId"),
                                      nl->SymbolAtom(CcReal::BasicType())));

         return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList);
     }

     virtual Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return 0;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return 0;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return 0;
        }
        Tuple* t = new Tuple(tupleType);
        t->PutAttribute(0, new CcString(true, s.substr(1,2)));

        datetime::DateTime* utc = nmea_stringutils::getUtc(st);
        if(utc->IsDefined()){
            utc->Add(&currentDay);
            if(lastTime.IsDefined()){
               if(*utc < lastTime){
                   utc->Add(&oneDay);
                   currentDay.Add(&oneDay);
               }
            }
            lastTime = *utc;
        }
        t->PutAttribute(1, utc);
        t->PutAttribute(2, nmea_stringutils::getLat(st));
        t->PutAttribute(3, nmea_stringutils::getLon(st));
        t->PutAttribute(4, nmea_stringutils::getString(st,0));
        t->PutAttribute(5, nmea_stringutils::getInt(st));
        t->PutAttribute(6, nmea_stringutils::getReal(st));
        t->PutAttribute(7, nmea_stringutils::getReal(st));
        t->PutAttribute(8, nmea_stringutils::getReal(st));
        t->PutAttribute(9, nmea_stringutils::getReal(st));
        CcReal* SID = new CcReal(false,0.0);
        if(st.hasNextToken()){
          stringutils::StringTokenizer st2(st.nextToken(),"*");
          if(st2.hasNextToken()){
              double v;
              if(nmea_stringutils::getReal(st.nextToken(),v)){
                SID->Set(true,v);
              }
          }
        }
        t->PutAttribute(10, SID);


        return t;
     }

  private:
     datetime::DateTime currentDay;
     datetime::DateTime lastTime;
     datetime::DateTime oneDay;
     TupleType* tupleType;

};



/*
2 Implementation of NMEAIMPORTER

This class manages a set of NMEALineImporters

*/

     NMEAImporter::NMEAImporter():importers(), position(0), in(){
       // the GGA importer must be the first in the vector
       importers.push_back(new GGAImporter());
       importers.push_back(new GSAImporter());
       importers.push_back(new RMCImporter());
       importers.push_back(new ZDAImporter());
       importers.push_back(new GSVImporter());
       importers.push_back(new GLLImporter());
       importers.push_back(new GNSImporter());
       // add further importers here
     }

/*
~Destructor~

*/
    NMEAImporter::~NMEAImporter(){
        for(size_t i = 0; i< importers.size();i++){
          if(importers[i]){
             delete importers[i];
          }
        }
        importers.clear();
        if(in.is_open()){
          in.close();
        }
    }

/*
~setType~

Sets the LineImporter to be used. If the given string does not represent a
accepted Type of an included line importer, the result will be false and
the GGA importer is used instead.

*/
     bool NMEAImporter::setType(const string& type /*="GGA"*/){
        position = -1;
        for(size_t i=0;i<importers.size();i++){
           if(importers[i]->accept(type)){
              position = i;
              return true;
           }
        }
        position = 0;
        return false;
     }

/*
~scanFile~

Starts to read a file. If the file cannot be opened or other
problems occurs, the function will return false and the errorMessage
is set to the recognized error.

*/
     bool NMEAImporter::scanFile(const string& fileName, string& errorMessage){
       if(in.is_open()){
          in.close();
       }
       in.open(fileName.c_str(), ios_base::in);
       if(!in.good()){
         errorMessage = "cannot open file " + fileName;
         return false;
       }
       return true;
     }

/*
~getTupleType~

Returns the tuple type for the currently used line importer.

*/
     ListExpr NMEAImporter::getTupleType(){
       return importers[position]->getTupleTypeAsList();
     }

/*
~getKnownTypes~

Returns all sentences which can be evaluated using the currently installed
line importers as a single string (separated by white spaces).

*/

     string NMEAImporter::getKnownTypes() const{
        string res;
        for(size_t i=0;i<importers.size();i++){
           res += "  "+ importers[i]->getAcceptedType();
        }
        stringutils::trim(res);
        return res;
     }

/*
~nextTuple~

Returns the Next Tuple corresponding to the currently selected sentence.

*/

     Tuple* NMEAImporter::nextTuple(){
       if(!in.is_open()){
          return 0;
       }
       string line;
       while(in.good()){
          getline(in, line);
          Tuple* t = importers[position]->getTupleForLine(line);
          if(t!=0){
             return t;
          }
       }
       in.close();
       return 0;
     }


/*
~getTuple~

Returns the tuple represented by this arguments. If __line__ does not match
the current used importer, 0 is returned.

*/
    Tuple* NMEAImporter::getTuple(const string& line){
      return importers[position]->getTupleForLine(line);
    }


/*
~finishScan~

Closes the unfderlying file if open.

*/
     void  NMEAImporter::finishScan(){
        if(in.is_open()){
          in.close();
        }
     }




