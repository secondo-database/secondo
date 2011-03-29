
/*



*/


#include <vector>
#include "NestedList.h"
#include "RelationAlgebra.h"
#include "DateTime.h"
#include "NMEAImporter.h"
#include "StringUtils.h"


extern NestedList* nl;

/*
1 Abstract class NMEALineImporter

*/
class NMEALineImporter{
  public:
     virtual ~NMEALineImporter(){}

     virtual bool accept(const string& DataSetName)const {
        return DataSetName == getAcceptedType();
     }

     virtual ListExpr getTupleTypeAsList() const = 0;

     virtual Tuple* getTupleForLine(const string& line)=0;

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
     if(!st.hasNextToken()){
        return false;
     }
     string second = st.nextToken();
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

datetime::DateTime* getUtc(stringutils::StringTokenizer& st){
   datetime::DateTime* res = new datetime::DateTime(datetime::instanttype);
   datetime::DateTime zero(datetime::instanttype);
   if(!st.hasNextToken()){
       res->SetDefined(false);
       return res;
   }
   string utc = st.nextToken();
   getutc(utc, zero, *res);
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
   double t = p2 / pow(10, second.length());
   result += t;
   return true;
}

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
  result = minutes + p1 / pow(10,mmmm.length());  
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

     GGAImporter():NMEALineImporter(),
                   currentDay(datetime::instanttype), 
                   lastTime(datetime::instanttype), 
                   oneDay(1,0,datetime::durationtype),
                   tupleType(0),
                   typeId("GGA"){
        ListExpr numtype = SecondoSystem::GetCatalog()->
                           NumericType(getTupleTypeAsList());
        tupleType = new TupleType((numtype));
     }

     virtual ~GGAImporter(){
       tupleType->DeleteIfAllowed();
       tupleType=0;
     }

    string getAcceptedType() const{
      return typeId;
    }


     ListExpr getTupleTypeAsList() const{
        ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                                    nl->SymbolAtom("SenID"),
                                                    nl->SymbolAtom("string")));
        ListExpr last = attrList;

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Utc"), 
                                               nl->SymbolAtom("instant")));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Lat"), 
                                               nl->SymbolAtom("real")));
                                                
        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Lon"), 
                                               nl->SymbolAtom("real")));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("FixQuality"), 
                                               nl->SymbolAtom("int")));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("NoSats"), 
                                               nl->SymbolAtom("int")));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("HDilution"), 
                                               nl->SymbolAtom("real")));
   
        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Alt"), 
                                               nl->SymbolAtom("real")));
        
        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("AltUnit"), 
                                               nl->SymbolAtom("string")));

        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("Height"), 
                                               nl->SymbolAtom("real")));

        
        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("HeightUnit"), 
                                               nl->SymbolAtom("string")));

        
        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("TimeLastUpdate"),
                                               nl->SymbolAtom("real")));


        last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("DGPSStationID"), 
                                               nl->SymbolAtom("int")));

        // ommit checksum

        return nl->TwoElemList(nl->SymbolAtom("tuple"), attrList);
  
     } 

     Tuple* getTupleForLine(const string& line) {

       stringutils::StringTokenizer st(line, ",");
       if(!st.hasNextToken()){
          return 0;
       }         

       string current = st.nextToken();
       if(current.length()!= 6 || current[0]!='$' || 
          current.substr(3,3)!=typeId){
          return 0;
       }
       string senId = current.substr(1,2);
      
       if(!st.hasNextToken()){
          return 0;
       }
       string utc_str = st.nextToken();
       if(!st.hasNextToken()){
          return 0;
       }
       string lat_str = st.nextToken();
       if(!st.hasNextToken()){
          return 0;
       }
       string NS_str = st.nextToken();
       if(!st.hasNextToken()){
          return 0;
       }
       string lon_str = st.nextToken();
       if(!st.hasNextToken()){
         return 0;
       }
       string EW_str = st.nextToken();



       datetime::DateTime t(datetime::instanttype);
       if(!nmea_stringutils::getutc(utc_str, currentDay, t)){
          return 0;
       }

       double x;
       double y;
       if(!nmea_stringutils::getLat(lat_str,NS_str,y)){
          return 0;
        } 
        if( !nmea_stringutils::getLon(lon_str, EW_str, x)){
          return 0;
       }
       if(t < lastTime){
          currentDay += oneDay;
          t += oneDay;
       }       
       lastTime = t;

       // the remainding value are optional -> may be undefined
       CcInt* fixQ = new CcInt(false,0);
       CcInt* noSats = new CcInt(false,0);
       CcReal* hDil = new CcReal(false,0.0);
       CcReal* alt = new CcReal(false,0.0);
       CcString* altUnit = new CcString(false,"");
       CcReal* height = new CcReal(false,0.0);
       CcString* heightUnit = new CcString(false,"");
       CcReal* lastUp = new CcReal(false,0.0);
       CcInt* lastStationId = new CcInt(false,0);
 
       int i;
       double d;
       string s;
       if(st.hasNextToken()){
         if(nmea_stringutils::getInt(st.nextToken(),i)){
            fixQ->Set(true,i);
          }
       }
       if(st.hasNextToken()){
         if(nmea_stringutils::getInt(st.nextToken(),i)){
            noSats->Set(true,i);
          }
       }
       if(st.hasNextToken()){
         if(nmea_stringutils::getReal(st.nextToken(),d)){
            hDil->Set(true,d);
          }
       }
       if(st.hasNextToken()){
         if(nmea_stringutils::getReal(st.nextToken(),d)){
            alt->Set(true,d);
          }
       }
       if(st.hasNextToken()){
          s = st.nextToken();
          if(s.length()!=0){
            altUnit->Set(true,s);
          }
       }
       if(st.hasNextToken()){
         if(nmea_stringutils::getReal(st.nextToken(),d)){
            height->Set(true,d);
          }
       }
       if(st.hasNextToken()){
          s = st.nextToken();
          if(s.length()!=0){
            heightUnit->Set(true,s);
          }
       }
       if(st.hasNextToken()){
         if(nmea_stringutils::getReal(st.nextToken(),d)){
            lastUp->Set(true,d);
          }
       }
       if(st.hasNextToken()){
          s = st.nextToken();
          stringutils::StringTokenizer st2(s,"*");
          if(nmea_stringutils::getInt(st2.nextToken(),i)){
            lastStationId->Set(true,i);
          }
       }




       Tuple* res = new Tuple(tupleType);
       res->PutAttribute(0, new CcString(true,senId));
       res->PutAttribute(1,new datetime::DateTime(t));
       res->PutAttribute(2,new CcReal(true,y));
       res->PutAttribute(3, new CcReal(true,x));
       res->PutAttribute(4, fixQ );
       res->PutAttribute(5, noSats );
       res->PutAttribute(6, hDil );
       res->PutAttribute(7, alt );
       res->PutAttribute(8, altUnit );
       res->PutAttribute(9, height );
       res->PutAttribute(10, heightUnit );
       res->PutAttribute(11, lastUp );
       res->PutAttribute(12, lastStationId );

       // TODO evaluation of further fields of GGA 

       return res;
     }


   private:
      datetime::DateTime currentDay;
      datetime::DateTime lastTime;
      datetime::DateTime oneDay;
      TupleType* tupleType;
      const string typeId;
};


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
                                                   nl->SymbolAtom("string")));

        ListExpr last = attrList;
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("FixSel"),
                                           nl->SymbolAtom("string")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("Fix3D"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN1"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN2"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN3"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN4"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN5"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN6"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN7"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN8"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN9"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN10"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN11"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PRN12"),
                                           nl->SymbolAtom("int")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("PDop"),
                                           nl->SymbolAtom("real")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("HDop"),
                                           nl->SymbolAtom("real")));
        last = nl->Append( last, nl->TwoElemList(nl->SymbolAtom("VDop"),
                                           nl->SymbolAtom("real")));
        return nl->TwoElemList(nl->SymbolAtom("tuple"),attrList);
    
         
     }

     virtual Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return false;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return false;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return false;
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
                                  nl->SymbolAtom("string")));
       ListExpr last = attrList;
       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Time"),
                                               nl->SymbolAtom("instant")));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Status"),
                                               nl->SymbolAtom("string")));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lat"),
                                               nl->SymbolAtom("real")));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Lon"),
                                               nl->SymbolAtom("real")));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Speed"),
                                               nl->SymbolAtom("real")));
       
       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Angle"),
                                               nl->SymbolAtom("real")));


       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MagV"),
                                               nl->SymbolAtom("real")));

       last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MagS"),
                                               nl->SymbolAtom("string")));
       return nl->TwoElemList(nl->SymbolAtom("tuple"), attrList);
     }

     virtual Tuple* getTupleForLine(const string& line){
        stringutils::StringTokenizer st(line,",");
        if(!st.hasNextToken()){
          return false;
        }
        string s = st.nextToken();
        if(s.length()!=6){
           return false;
        }
        if(s.substr(3,3)!=getAcceptedType()){
          return false;
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
2 Implementation of NMEAIMPORTER

*/

     NMEAImporter::NMEAImporter():importers(), position(0), in(){
       // the GGA importer must be the first in the vector
       importers.push_back(new GGAImporter());    
       importers.push_back(new GSAImporter());    
       importers.push_back(new RMCImporter());    
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
    }


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


     bool NMEAImporter::scanFile(const string& fileName, string& errorMessage){
       in.open(fileName.c_str(), ios_base::in);
       if(!in.good()){
         errorMessage = "cannot open file " + fileName;
         return false;
       } 
       return true;
     }




     ListExpr NMEAImporter::getTupleType(){
       return importers[position]->getTupleTypeAsList();
     }

     string NMEAImporter::getKnownTypes() const{
        string res;
        for(size_t i=0;i<importers.size();i++){
           res += "  "+ importers[i]->getAcceptedType();
        }
        stringutils::trim(res);
        return res;
     }

     
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




