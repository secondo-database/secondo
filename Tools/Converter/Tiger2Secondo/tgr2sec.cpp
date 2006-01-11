/*
0 Licence 

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

*/

/* 
1 Includes

*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <ios>
#include <iomanip>



using namespace std;

static string srcdir="tmp";
static string dbname="tiger";
static bool oldStyle=false;


/*
2 Some helpfully functions 

*/
/*
2.1 trim 

This function returns a string resulting from the argumentstring without
any blanks at the begin or end.

*/
static string trim(const string& s){
 if(s.length() == 0)
      return s;
 unsigned int beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
 unsigned int end = s.find_last_not_of(" \a\b\f\n\r\t\v");
 if(beg == std::string::npos) // No non-spaces
      return "";
  return string(s, beg, end - beg + 1);
}

/*
3 The LineWriter class

This class can be easely used to write a line object from a point sequence.
This is done by buffering the last inserted point.

*/

class LineWriter{
public:

/*
3.1 Constructor

The constructor creates a new LineWriter instance.

*/
   LineWriter(){
     points=0;
     opened=false;
   }

/*
3.2 addPoint function

This function will have only a effect when the linewriter is opend before.
If this is the first point in the line, it is just stored. If it's not so,
the segment from the last point to the new point is written and the given
point is the new last one.

*/
   void addPoint(double x, double y){
      assert(opened);
      if(points>0 &&( x!=px || y !=py)){
          (*out) << "(" << px << " " << py << " " << x << " " << y << ") ";
      } 
      points++;
      px = x;
      py = y;
      
   }

/*
3.3 Open

The open function must be called before any points can be added to this
LineWriter. It write the opening bracket for the line.

*/
   void open(ostream* out){
      assert(!opened);
      this->out = out;
      (*out) << "(";
      opened = true;
   }


/*
3.4 Close

This function closes the line by writing the closing bracket. The 
After calling this function, the linewrite can be opened again to write a
new line.

*/
   void close(){
     assert(opened); 
     (*out) << ")";
     points=0;
     opened = false;
   }


private:
   int points;
   double px;
   double py;
   ostream* out;
   bool opened;
};

/*
1.0 The record type 1 - Complete Chain Basic Data Record 

*/
class RT1{
public: 
 
bool read(const string& line){
   if(line.length()<228){
      return false;
   }
   if(line[0]!='1')
      return false;
   // copy into data
   for(int i=0;i<228;i++){
      data[i] = line[i];
   }
   return true;
 } 

string getTLID(){ return getString(6,15); }
string getSIDE1(){ return getString(16,16);}
string getSOURCE(){ return getString(17,17);}
string getFEDIRP(){ return getString(18,19);}
string getFENAME(){ return getString(20,49); }
string getFETYPE(){ return getString(50,53); }
string getFEDIRS(){ return getString(54,55);}
string getCFCC(){ return getString(56,58); }
string getFRADDL(){ return getString(59,69); }
string getTOADDL(){ return getString(70,80); }
string getFRADDR(){ return getString(81,91); }
string getTOADDR(){ return getString(92,102); }
string getFRIADDL(){ return getString(103,103);}
string getTOIADDL(){ return getString(104,104);}
string getFRIADDR(){ return getString(105,105);}
string getTOIADDR(){ return getString(106,106);}
string getZIPL(){ return getString(107,111);}
string getZIPR(){ return getString(112,116);}
string getAIANHHL(){ return getString(117,121);}
string getAIANHHR(){ return getString(122,126);}
string getAIHHTLIL(){ return getString(127,127);}
string getAIHHTLIR(){ return getString(128,128);}
string getCENSUS1(){ return getString(129,129);}
string getCENSUS2(){ return getString(130,130);}
string getSTATEL(){ return getString(131,132);}
string getSTATER(){ return getString(133,134);}
string getCOUNTYL(){ return getString(135,137);}
string getCOUNTYR(){ return getString(138,140);}
string getCOUSUBL(){ return getString(141,145);}
string getCOUSUBR(){ return getString(146,150);}
string getSUBMCDL(){ return getString(151,155);}
string getSUBMCDR(){ return getString(156,160);}
string getPLACEL(){ return getString(161,165);}
string getPLACER(){ return getString(166,170);}
string getTRACT90L(){ return getString(171,176);}
string getTRACT90R(){ return getString(177,182);}
string getBLOCK90L(){ return getString(183,186);}
string getBLOCK90R(){ return getString(187,190);}

double getFRLONG(){
   string r1 = getString(191,200);
   return  atol(r1.c_str())/1000000.0;
}

double getFRLAT(){
   string r1 = getString(201,209);
   return atol(r1.c_str())/1000000.0;
}

double  getTOLONG(){
   string r1 = getString(210,219);
   return  atol(r1.c_str())/1000000.0;
}

double getTOLAT(){
   string r1 = getString(220,228);
   return atol(r1.c_str())/1000000.0;
}

private :
  unsigned char data[228];
  string getString(int a,int b){
     string res;
     for(int i=a-1;i<b;i++){
       res += data[i];
     }
     return trim(res);
  }
};


/*
4 The Record Type 2 

This record type describes chains.

*/
class RT2{
public:
/*

This is a function returning the id.

*/
  string  getTLID(){ return TLID;}

/*

This function returns the sequence number. 

*/
  int getRTSQ(){ return RTSQ;}


/*

This function writes all points included in this RT2 
als sequence to out.

*/
  void writeNL(ostream& out){
     for(int i=0;i<nop;i++){
        out << "(" << (points[2*i]) << " " << (points[2*i+1]) << ")";
     }
  }

/*

Adds all points included in this to lw

*/

  void addTo(LineWriter& lw){
     for(int i=0;i<nop;i++){
        lw.addPoint(points[2*i],points[2*i+1]);
     }
  }


  bool operator==(const RT2& e2)const{
    return (TLID==e2.TLID) && (RTSQ==e2.RTSQ);
  }

  bool operator<(const RT2& e2)const{
    if(TLID==e2.TLID)
        return RTSQ<e2.RTSQ;
    else
       return TLID<e2.TLID;
  }
  
  bool operator>(const RT2& e2) const{
    if(TLID==e2.TLID)
        return RTSQ>e2.RTSQ;
    else
       return TLID>e2.TLID;
  }

  RT2& operator=(const RT2& e2){
     TLID = e2.TLID;
     RTSQ = e2.RTSQ;
     nop = e2.nop;
     for(int i=0;i<20;i++)
        points[i] = e2.points[i];
     return (*this); 
  }


 

  static void writeDataFrom(vector<RT2>& sortedElems, string tlid,
                            LineWriter& lw){
     // find the first element containing tlid
     // search sequentially, later switch to binary search
     int size = sortedElems.size();
     if(size<1)
        return;


     bool found = false;
     int pos = -1;

     // perform a binary search to find the first occurence 
     // of tlid in sortedElems
     int min=0;
     int max= size-1;
     int mid = (max+min)/2;
     RT2 r;
     while(min<max){
        mid=(max+min)/2;
        r = sortedElems[mid];
        if(r.getTLID()>=tlid){
          max=mid;
        } else{
          min=mid+1;
        }
     }

     if(min<size){
       r = sortedElems[min];
       found = r.getTLID()==tlid;
       pos = min;
     }
     // check for error (debugging)
     if(pos>0){
        r = sortedElems[pos-1];
        if(r.getTLID()==tlid){
          cerr << "not the first element " << endl;
          exit( -1);
        } 
     }




     // the first element was found scan until other tlid is reached
     if(found){ // otherwise we don't have any data
        bool done = false;
        while(pos<size && !done){
            RT2 elem = sortedElems[pos];
            if(elem.getTLID()!=tlid)
               done = true;
            else
              elem.addTo(lw); 
            pos++;
        }
     }

  }

  bool readFrom(string& line){
     if(line.length()<208){
	cerr << "RT2: expected string of minimum length of 209, line has ";
        cerr << line.length() << " characters" << endl;	
        return false;
     }
     if(line[0]!='2'){
        cerr << "RT2 :  line must start with '2', but starts with ";
        cerr <<	line[0]    << endl;
        return false;
     }	
     TLID = trim(line.substr(5,10));
     char rtsq[4];
     for(int i=0;i<3;i++)
        rtsq[i] = line[i+15];
     rtsq[3] = 'z';
     RTSQ= atoi(rtsq);
     nop = 0;
     for(int i=0;i<10;i++){
        string lon = trim(line.substr(i*19+18,10));
        string lat = trim(line.substr(i*19+28,9));
        if(lon!=""){
            double x = atol(lon.c_str())/1000000.0;
            double y = atol(lat.c_str())/1000000.0;
            if(x!=0 || y!=0){
               points[2*nop] = x; 
               points[2*nop+1] = y ; 
                 nop++;
            }
        }
     }
     return true;
  }


private:
   string TLID;
   unsigned int  RTSQ;
   int   nop;
   double points[20];

};


/*
5 Record Type 3 

The record type is for complete chain geographic entity codes.

*/
class RT3{

public:
/*
5.1 Read the data from a Strring 

*/
  bool readFrom(const string& line){
      if(line.length()<111)
         return false;
      if(line[0]!='3')
         return false;
      for(int i=0;i<111;i++){
        data[i] = line[i];
      }
      return true;
  }
  
  unsigned char getRT(){ return data[0];}
  string getVERSION(){ return getString(2,5);}
  unsigned long getTLID(){ return getLong(6,15);}
  int getSTATE90L(){ return getInt(16,17); }
  int getSTATE90R(){return getInt(18,19); }
  string getCOUNTY90L(){return getString(20,22);}
  string getCOUNTY90R(){return getString(23,25);}
  string getCOUSUB90L(){return getString(26,30);} 
  string getCOUSUB90R(){return getString(31,35);}
  string getPLACE90L(){return getString(36,40);}
  string getPLACE90R(){ return getString(41,45);}
  string getTRACT90L(){return getString(46,51);}
  string getTRACT90R(){return getString(52,57);}
  string getAIANHHCE90L(){return getString(58,61);}
  string getAIANHHCE90R(){return getString(62,65);}
  string getAIHHTLI90L(){return getString(66,66);}
  string getAIHHTLI90R(){return getString(67,67);}
  string getBLOCK90L(){return getString(70,73);}
  string getBLOCK90R(){return getString(74,77);}
  string getAIANHHCEL(){return getString(78,81);}
  string getAIANHHCER(){return getString(82,85);}
  string getANRCL(){return getString(86,90);}
  string getANRCR(){return getString(91,95);}
  string getAITSCEL(){return getString(96,98);}
  string getAITSCER(){return getString(99,101);}
  string getAITSL(){return getString(102,106);}
  string getAITSR(){return getString(107,111);}

  static string Extension(){ return "RT3";}
  static void writeRelType(ostream& out){
     out << " ( rel (tuple (" << endl;
     out << "      (tlid string ) " << endl;
     out << "      (state90l int) " << endl;
     out << "      (state90r int ) " << endl;
     out << "      (county90l string ) " << endl;
     out << "      (county90r string ) " << endl;
     out << "      (cousub90l string ) " << endl;
     out << "      (cousub90r string ) " << endl;
     out << "      (place90l string ) " << endl;
     out << "      (place90r string ) " << endl;
     out << "      (tract90l string ) " << endl;
     out << "      (tract90r string ) " << endl;
     out << "      (aianhhce90l string ) " << endl;
     out << "      (aianhhce90r string ) " << endl;
     out << "      (aihhtli90l string ) " << endl;
     out << "      (aihhtli90r string ) " << endl;
     out << "      (block90l string ) " << endl;
     out << "      (block90r string ) " << endl;
     out << "      (aianhhcel string ) " << endl;
     out << "      (aianhhcer string ) " << endl;
     out << "      (anrcl string ) " << endl;
     out << "      (anrcr string ) " << endl;
     out << "      (aitscel string ) " << endl;
     out << "      (aitscer string ) " << endl;
     out << "      (aitsl string ) " << endl;
     out << "      (aitsr string ) " << endl;
     out << " )))" << endl; // close type information
  }
  void writeTuple(ostream& out){
    out << "(";
    out << "\"" << getTLID() << "\" ";
    out << " " <<  getSTATE90L() << " ";
    out << " " <<  getSTATE90R() << " ";
    out << "\"" << getCOUNTY90L() << "\" ";
    out << "\"" << getCOUNTY90R() << "\" ";
    out << "\"" << getCOUSUB90L() << "\" ";
    out << "\"" << getCOUSUB90R() << "\" ";
    out << "\"" << getPLACE90L() << "\" ";
    out << "\"" << getPLACE90R() << "\" ";
    out << "\"" << getTRACT90L() << "\" ";
    out << "\"" << getTRACT90R() << "\" ";
    out << "\"" << getAIANHHCE90L() << "\" ";
    out << "\"" << getAIANHHCE90R() << "\" ";
    out << "\"" << getAIHHTLI90L() << "\" ";
    out << "\"" << getAIHHTLI90R() << "\" ";
    out << "\"" << getBLOCK90L() << "\" ";
    out << "\"" << getBLOCK90R() << "\" ";
    out << "\"" << getAIANHHCEL() << "\" ";
    out << "\"" << getAIANHHCER() << "\" ";
    out << "\"" << getANRCL() << "\" ";
    out << "\"" << getANRCR() << "\" ";
    out << "\"" << getAITSCEL() << "\" ";
    out << "\"" << getAITSCER() << "\" ";
    out << "\"" << getAITSL() << "\" ";
    out << "\"" << getAITSR() << "\" ";
    out << ")" << endl; // close tuple
  }
private:
  unsigned char data[111];
  
  string getString(int a,int b){
     string res;
     for(int i=a-1;i<b;i++){
       res += data[i];
     }
     return trim(res);
  }

  unsigned long getLong(int a, int b){
    return atol(getString(a,b).c_str());
  }
  unsigned int getInt(int a, int b){
    return atoi(getString(a,b).c_str());
  }

};

/*

This class managed the record type 4 - index to alternate feature identifiers.

*/
class RT4{
public:
  bool readFrom(string& line){
    if(line.length()<58)
       return false;
    if(line[0]!='4')
        return false;
    data = line;
    return true;
  }

  string getTLID(){ return trim(data.substr(5,10));}
  string getRTSQ(){ return trim(data.substr(15,3));}
  string getFEAT1(){ return trim(data.substr(18,8));}
  string getFEAT2(){ return trim(data.substr(26,8));}
  string getFEAT3(){ return trim(data.substr(34,8));}
  string getFEAT4(){ return trim(data.substr(42,8));}
  string getFEAT5(){ return trim(data.substr(50,8));}

  static string Extension(){ return "RT4"; }

  static void writeRelType(ostream& out){ 
    out << " ( rel (tuple ( "  << endl;
    out << "      (tlid  string) " << endl;
    out << "      (rtsq  string) " << endl;
    out << "      (feat1  string) " << endl;
    out << "      (feat2  string) " << endl;
    out << "      (feat3  string) " << endl;
    out << "      (feat4  string) " << endl;
    out << "      (feat5  string) " << endl;
    out << " )))" << endl;
  }

  void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getTLID() << "\" " << endl;
     out << "\"" << getRTSQ() << "\" " << endl;
     out << "\"" << getFEAT1() << "\" " << endl;
     out << "\"" << getFEAT2() << "\" " << endl;
     out << "\"" << getFEAT3() << "\" " << endl;
     out << "\"" << getFEAT4() << "\" " << endl;
     out << "\"" << getFEAT5() << "\" " << endl;
     out << ")" << endl;
  }
private:
  string data;
};


/*

Record Type 5 - Complete Chain Feature Identifiers 

*/
class RT5{
public:
  bool readFrom(string& line){
    if(line.length()<52)
       return  false;
    if(line[0]!='5')
       return false;
    data=line;
    return true;
  }
  string getFILE(){ return trim(data.substr(1,5)); }
  string getFEAT(){ return trim(data.substr(6,8)); }
  string getFEDIRP(){ return trim(data.substr(14,2));}
  string getFENAME(){ return trim(data.substr(16,30));}
  string getFETYPE(){ return trim(data.substr(46,4));}
  string getFEDIRS(){ return trim(data.substr(50,2));}

  static string Extension(){ return "RT5"; }
  static void writeRelType(ostream& out){
    out << " ( rel (tuple ( "  << endl;
    out << "      (file  string) " << endl;
    out << "      (feat  string) " << endl;
    out << "      (fedirp  string) " << endl;
    out << "      (fename  string) " << endl;
    out << "      (fetype  string) " << endl;
    out << "      (fedirs  string) " << endl;
    out << " )))" << endl;
  }
  void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getFILE() << "\" " << endl;
     out << "\"" << getFEAT() << "\" " << endl;
     out << "\"" << getFEDIRP() << "\" " << endl;
     out << "\"" << getFENAME() << "\" " << endl;
     out << "\"" << getFETYPE() << "\" " << endl;
     out << "\"" << getFEDIRS() << "\" " << endl;
     out << ")" << endl;
  }

private:
   string data;
};


/*

Record Type 6 : Additional address range and zip code data 

*/
class RT6{
public: 
   bool readFrom(string& line){
      if(line.length()<76)
          return false;
      if(line[0]!='6')
          return false;
      data = line;
      return true;
   }
  string getTLID(){ return trim(data.substr(5,10)); }
  string getRTSQ(){ return trim(data.substr(15,3)); }
  string getFRADDL(){ return trim(data.substr(18,11)); }
  string getTOADDL(){ return trim(data.substr(29,11)); }
  string getFRADDR(){ return trim(data.substr(40,11)); }
  string getTOADDR(){ return trim(data.substr(51,11)); }
  string getFRIADDL(){ return trim(data.substr(62,1)); }
  string getTOIADDL(){ return trim(data.substr(63,1)); }
  string getFRIADDR(){ return trim(data.substr(64,1)); }
  string getTOIADDR(){ return trim(data.substr(65,1)); }
  string getZIPL(){ return trim(data.substr(66,5)); }
  string getZIPR(){ return trim(data.substr(71,5)); }
  static string Extension(){ return "RT6"; }
  static void writeRelType(ostream& out){
    out << " ( rel (tuple ( "  << endl;
    out << "      (tlid  string) " << endl;
    out << "      (rtsq  string) " << endl;
    out << "      (fraddl  string) " << endl;
    out << "      (toaddl  string) " << endl;
    out << "      (fraddr  string) " << endl;
    out << "      (toaddr  string) " << endl;
    out << "      (friaddl  string) " << endl;
    out << "      (toiaddl  string) " << endl;
    out << "      (friaddr  string) " << endl;
    out << "      (toiaddr  string) " << endl;
    out << "      (zipl  string) " << endl;
    out << "      (zipr  string) " << endl;
    out << " )))" << endl;
  }
  void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getTLID() << "\" " << endl;
     out << "\"" << getRTSQ() << "\" " << endl;
     out << "\"" << getFRADDL() << "\" " << endl;
     out << "\"" << getTOADDL() << "\" " << endl;
     out << "\"" << getFRADDR() << "\" " << endl;
     out << "\"" << getTOADDR() << "\" " << endl;
     out << "\"" << getFRIADDL() << "\" " << endl;
     out << "\"" << getTOIADDL() << "\" " << endl;
     out << "\"" << getFRIADDR() << "\" " << endl;
     out << "\"" << getTOIADDR() << "\" " << endl;
     out << "\"" << getZIPL() << "\" " << endl;
     out << "\"" << getZIPR() << "\" " << endl;
     out << ")";
  }
private:
   string data;
};


/*

Record Type 7 - Landmark features

*/
class RT7{
 public:
   bool readFrom(string& line){
      if(line.length()<74)
        return false;
      if(line[0]!='7')
        return false;
      data = line;      
      return true; 
   }
   string getFILE(){ return trim(data.substr(5,5)); }
   string getLAND(){ return trim(data.substr(10,10)); }
   string getSOURCE(){ return trim(data.substr(20,1)); }
   string getCFCC(){return trim(data.substr(21,3)); }
   string getLANAME(){return trim(data.substr(24,30)); }
   double getLALONG(){return atol(trim(data.substr(54,10)).c_str())/1000000.0;}
   double getLALAT(){return atol(trim(data.substr(64,9)).c_str())/1000000.0;}

   bool isPoint(){
      double x = getLALONG();
      double y = getLALAT();
      return x!=0.0 || y !=0.0;
   }
private:
   string data;
};


/*

Record Type 8  - Polygons linked to area landmarks

*/
class RT8{
 public:
   bool readFrom(string& line){
      if(line.length()<36)
        return false;
      if(line[0]!='8')
        return false;
      data = line;      
      return true; 
   }
 string getFILE(){ return trim(data.substr(5,5));}
 string getCENID(){return trim(data.substr(10,5));}
 string getPOLYID(){return trim(data.substr(15,10));}
 string getLAND(){ return trim(data.substr(25,10));}

 static string Extension(){ return "RT8"; };
 static void writeRelType(ostream& out){
    out << "(rel (tuple (" << endl;
    out << "    (file string) " << endl;
    out << "    (cenid string) " << endl;
    out << "    (polyid string) " << endl;
    out << "    (land string) " << endl;
    out << " ))) " << endl;
 }
 void writeTuple(ostream& out){
    out << "(";
    out << "\"" << getFILE() << "\" ";
    out << "\"" << getCENID() << "\" ";
    out << "\"" << getPOLYID() << "\" ";
    out << "\"" << getLAND() << "\" ";
    out << ")" << endl;
 }

 private:
   string data;
};

/*
Record Type 9 - Key geographic location features 

*/
class RT9{
  public:
    bool readFrom(string& line){
       if(line.length()<88)
          return false;
       if(line[0]!='9')
          return false;
       data=line;
       return true;
    }
   string getFILE(){ return trim(data.substr(5,5));}
   string getCENID(){ return trim(data.substr(10,5)); }
   string getPOLYID(){return trim(data.substr(15,10));}
   string getSOURCE(){return trim(data.substr(25,1));}
   string getCFCC(){return trim(data.substr(26,3));}
   string getKGLNAME(){ return trim(data.substr(29,30));}
   string getKGLADD(){return trim(data.substr(59,11));}
   string getKGLZIP(){return trim(data.substr(70,5));}
   string getKGLZIP4(){return trim(data.substr(75,4));}
   string getFEAT(){return trim(data.substr(79,8));}

   static string Extension(){ return "RT9"; }

   static void writeRelType(ostream& out){
      out << "(rel (tuple ( " << endl;
      out << "   (file     string )" << endl;
      out << "   (cenid    string )" << endl;
      out << "   (polyid   string )" << endl;
      out << "   (source   string )" << endl;
      out << "   (cfcc     string )" << endl;
      out << "   (kglname  string )" << endl;
      out << "   (kgladd   string )" << endl;
      out << "   (kglzip   string )" << endl;
      out << "   (kglzip4  string )" << endl;
      out << "   (feat     string )" << endl;
      out << ")))" << endl;
   }

   void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getFILE() << "\" ";
     out << "\"" << getCENID() << "\" ";
     out << "\"" << getPOLYID() << "\" ";
     out << "\"" << getSOURCE() << "\" ";
     out << "\"" << getCFCC() << "\" ";
     out << "\"" << getKGLNAME() << "\" ";
     out << "\"" << getKGLADD() << "\" ";
     out << "\"" << getKGLZIP() << "\" ";
     out << "\"" << getKGLZIP4() << "\" ";
     out << "\"" << getFEAT() << "\" ";
     out << ")" << endl;

   }


  private:
     string data;
};

/*

Record Type A - Polygon Geogrphic Entity Codes 

*/
class RTA{
  public:
    bool readFrom(string& line){
       if(line.length()<98)
          return false;
       if(line[0]!='A')
          return false;
       data = line;
       return true;
    }
    string getFILE(){ return trim(data.substr(5,5)); }
    string getCENID(){ return trim(data.substr(10,5));}
    string getPOLYID(){return trim(data.substr(15,10));}
    string getAIANHH90(){return trim(data.substr(25,5));}
    string getCOUSUB90(){return trim(data.substr(30,5));}
    string getPLACE90(){return trim(data.substr(35,5));}
    string getTRACT90(){return trim(data.substr(40,6));}
    string getBLOCK90(){return trim(data.substr(46,4));}
    string getCD106(){return trim(data.substr(50,2));}
    string getCD108(){return trim(data.substr(52,2));}
    string getSDELM(){return trim(data.substr(54,5));}
    string getPUMA1(){return trim(data.substr(59,5));}
    string getSDSEC(){return trim(data.substr(64,5));}
    string getSDUNI(){return trim(data.substr(69,5));}
    string getTAZ(){return trim(data.substr(74,6));}
    string getUA90(){return trim(data.substr(80,4));}
    string getUR90(){return trim(data.substr(84,1));}
    string getSTATE90(){return trim(data.substr(89,2));}
    string getCOUNTY90(){return trim(data.substr(91,3));}
    string getAIANHHCE90(){return trim(data.substr(94,4));}

    static string Extension(){ return "RTA"; }

    static void writeRelType(ostream& out){
       out << "(rel(tuple( " << endl;
       out << "   (file string) " << endl;
       out << "   (cenid string) " << endl;
       out << "   (polyid string) " << endl;
       out << "   (aianhh90 string) " << endl;
       out << "   (cousub90 string) " << endl;
       out << "   (place90 string) " << endl;
       out << "   (tract90 string) " << endl;
       out << "   (block90 string) " << endl;
       out << "   (cd106 string) " << endl;
       out << "   (cd108 string) " << endl;
       out << "   (sdelm string) " << endl;
       out << "   (puma1 string) " << endl;
       out << "   (sdsec string) " << endl;
       out << "   (sduni string) " << endl;
       out << "   (taz string) " << endl;
       out << "   (ua90 string) " << endl;
       out << "   (ur90 string) " << endl;
       out << "   (state90 string) " << endl;
       out << "   (county90 string) " << endl;
       out << "   (aianhhce90 string) " << endl;
       out << " )))" << endl;
   }

   void writeTuple(ostream& out ){
      out << "(";
      out << "\"" << getFILE() << "\" ";
      out << "\"" << getCENID() << "\" ";
      out << "\"" << getPOLYID() << "\" ";
      out << "\"" << getAIANHH90() << "\" ";
      out << "\"" << getCOUSUB90() << "\" ";
      out << "\"" << getPLACE90() << "\" ";
      out << "\"" << getTRACT90() << "\" ";
      out << "\"" << getBLOCK90() << "\" ";
      out << "\"" << getCD106() << "\" ";
      out << "\"" << getCD108() << "\" ";
      out << "\"" << getSDELM() << "\" ";
      out << "\"" << getPUMA1() << "\" ";
      out << "\"" << getSDSEC() << "\" ";
      out << "\"" << getSDUNI() << "\" ";
      out << "\"" << getTAZ() << "\" ";
      out << "\"" << getUA90() << "\" ";
      out << "\"" << getUR90() << "\" ";
      out << "\"" << getSTATE90() << "\" ";
      out << "\"" << getCOUNTY90() << "\" ";
      out << "\"" << getAIANHHCE90() << "\" ";
      out << ")" << endl;
   }


private:
   string data;

};

/*

Record Type C - Geographic Entity Names 

*/

class RTC{
public:
   bool readFrom(string& line){
      if(line.length()<112)
         return false;
      if(line[0]!='C')
         return false;
      data=line;
      return true;
   }
  string getSTATE(){ return trim(data.substr(5,2));}
  string getCOUNTY(){ return trim(data.substr(7,3));}
  string getDATAYR(){ return trim(data.substr(10,4));}
  string getFIPS(){ return trim(data.substr(14,5));}
  string getFIPSCC(){ return trim(data.substr(19,2));}
  string getPLACEDC(){ return trim(data.substr(21,1));}
  string getLSADC(){ return trim(data.substr(22,2));}
  string getENTITY(){return trim(data.substr(24,1));}
  string getMA(){return trim(data.substr(25,4));}
  string getSD(){return trim(data.substr(29,5));}
  string getAIANHHCE(){return trim(data.substr(34,4));}
  string getVTDTRACT(){return trim(data.substr(38,6));}
  string getUAGA(){return trim(data.substr(44,5));}
  string getAITSCE(){ return trim(data.substr(49,3));}
  string getNAME(){ return trim(data.substr(62,60));}

  static string Extension(){ return "RTC"; };

  static void writeRelType(ostream& out){
     out << "(rel (tuple (" << endl;
     out << "     (state   string)" << endl;
     out << "     (county   string)" << endl;
     out << "     (datayr   string)" << endl;
     out << "     (fips   string)" << endl;
     out << "     (fipscc   string)" << endl;
     out << "     (placedc   string)" << endl;
     out << "     (lsadc   string)" << endl;
     out << "     (entity   string)" << endl;
     out << "     (ma   string)" << endl;
     out << "     (sd   string)" << endl;
     out << "     (aianhhce   string)" << endl;
     out << "     (vtdtract   string)" << endl;
     out << "     (uaga   string)" << endl;
     out << "     (aitsce   string)" << endl;
     out << "     (name   string)" << endl;
     out << ")))" << endl;
  }

  void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getSTATE() << "\" " ;
     out << "\"" << getCOUNTY() << "\" " ;
     out << "\"" << getDATAYR() << "\" " ;
     out << "\"" << getFIPS() << "\" " ;
     out << "\"" << getFIPSCC() << "\" " ;
     out << "\"" << getPLACEDC() << "\" " ;
     out << "\"" << getLSADC() << "\" " ;
     out << "\"" << getENTITY() << "\" " ;
     out << "\"" << getMA() << "\" " ;
     out << "\"" << getSD() << "\" " ;
     out << "\"" << getAIANHHCE() << "\" " ;
     out << "\"" << getVTDTRACT() << "\" " ;
     out << "\"" << getUAGA() << "\" " ;
     out << "\"" << getAITSCE() << "\" " ;
     out << "\"" << getNAME() << "\" " ;
     out << ")" << endl;
  }

private:
   string data;
};

/*

Record Type H - Tiger/line ID history 

*/

class RTH{
public:
  bool readFrom(string& line){
     if(line.length()<62)
        return false;
     if(line[0]!='H')
        return false;
     data=line;
     return true;
  }
  string getFILE(){ return trim(data.substr(5,5));}
  string getTLID(){ return trim(data.substr(10,10));}
  string getHIST(){ return trim(data.substr(20,1));}
  string getSOURCE(){ return trim(data.substr(21,1));}
  string getTLIDFR1(){ return trim(data.substr(22,10));}
  string getTLIDFR2(){ return trim(data.substr(32,10));}
  string getTLIDTO1(){return trim(data.substr(42,10));}
  string getTLIDTO2(){ return trim(data.substr(52,10));}

  static string Extension(){ return "RTH"; }

  static void writeRelType(ostream& out){
    out << " (rel (tuple ( ";
    out << "    (file  string )" << endl;
    out << "    (tlid  string )" << endl;
    out << "    (hist  string )" << endl;
    out << "    (source  string )" << endl;
    out << "    (tlidfr1  string )" << endl;
    out << "    (tlidfr2  string )" << endl;
    out << "    (tlidto1  string )" << endl;
    out << "    (tlidto2  string )" << endl;
    out << " ))) " << endl;
  }

  void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getFILE() << "\" "; 
     out << "\"" << getTLID() << "\" "; 
     out << "\"" << getHIST() << "\" "; 
     out << "\"" << getSOURCE() << "\" "; 
     out << "\"" << getTLIDFR1() << "\" "; 
     out << "\"" << getTLIDFR2() << "\" "; 
     out << "\"" << getTLIDTO1() << "\" "; 
     out << "\"" << getTLIDTO2() << "\" "; 
     out << ")" << endl;
  }

private:
  string data;
};


/*

Record Type I - Link between Complete Chaiens and polygons

*/
class RTI{
public:
   bool readFrom(string& line){
     if(line.length()<52)
        return false;
     if(line[0]!='I')
        return false;
     data = line;
     return true;
   }
   string getTLID(){return trim(data.substr(5,10));}
   string getFILE(){return trim(data.substr(15,5));}
   string getRTLINK(){return trim(data.substr(20,1));}
   string getCENIDL(){return trim(data.substr(21,5));}
   string getPOLYIDL(){return trim(data.substr(26,10));}
   string getCENIDR(){return trim(data.substr(36,5));}
   string getPOLYIDR(){return trim(data.substr(41,10));}

   static string Extension(){ return "RTI"; }

   static void writeRelType(ostream& out){
      out << "( rel ( tuple (" << endl;
      out << "   (tlid   string)" << endl;
      out << "   (file   string)" << endl;
      out << "   (rtlink   string)" << endl;
      out << "   (cenidl   string)" << endl;
      out << "   (polyidl   string)" << endl;
      out << "   (cenidr   string)" << endl;
      out << "   (polyidr   string)" << endl;
      out << " ))) " << endl;
   }

   void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getTLID() << "\" ";
     out << "\"" << getFILE() << "\" ";
     out << "\"" << getRTLINK() << "\" ";
     out << "\"" << getCENIDL() << "\" ";
     out << "\"" << getPOLYIDL() << "\" ";
     out << "\"" << getCENIDR() << "\" ";
     out << "\"" << getPOLYIDR() << "\" ";
     out << ")" << endl;
   }
private:
   string data;
};

/*
Record Type P - Polygon Internal Point

*/
class RTP{
  public:
    bool readFrom(string& line){
       if(line.length()<44)
          return false;
       if(line[0]!='P')
           return false;
       data = line;
       return true;
    }
    string getFILE(){ return trim(data.substr(5,5));}
    string getCENID(){ return trim(data.substr(10,5));}
    string getPOLYID(){ return trim(data.substr(15,10));}
    double getPOLYLONG(){ 
        return atol(trim(data.substr(25,10)).c_str())/1000000.0;
    }
    double getPOLYLAT(){
        return atol(trim(data.substr(35,9)).c_str())/1000000.0;
    }

    static string Extension(){ return "RTP";}

    static void writeRelType(ostream& out){
       out << "(rel (tuple ( " << endl;
       out << "   (file string) " << endl;
       out << "   (cenid string) " << endl;
       out << "   (polyid string) " << endl;
       out << "   (innerPoint point) " << endl;
       out << " ))) " << endl;
    }

    void writeTuple(ostream& out){
       out << "(";
       out << "\"" << getFILE() << "\" ";
       out << "\"" << getCENID() << "\" ";
       out << "\"" << getPOLYID() << "\" ";
       out << "(" << getPOLYLONG() << " " << getPOLYLAT() << ")";
       out << ")" << endl;
    }
  private:
    string data;
};

/*
  
Record Type R - Tiger-Line ID Record Number Range

*/
class RTR{
public:
  bool readFrom(string& line){
     if(line.length()<46)
         return false;
     if(line[0]!='R')
         return false;
     data=line;
     return true;
  }
  string getFILE(){ return trim(data.substr(5,5));}
  string getCENID(){ return trim(data.substr(10,5));}
  string getMAXID(){ return trim(data.substr(15,10));}
  string getMINID(){ return trim(data.substr(25,10));}
  string getHIGHID(){ return trim(data.substr(35,10));}

  static string Extension(){ return "RTR"; }

  static void writeRelType(ostream& out){
     out << " (rel ( tuple ( ";
     out << "    (file string) " << endl;
     out << "    (cenid string) " << endl;
     out << "    (maxid string) " << endl;
     out << "    (minid string) " << endl;
     out << "    (highid string) " << endl;
     out << " )))" << endl;
  }

  void writeTuple(ostream& out){
     out << "(";
     out << "\"" << getFILE() << "\" ";
     out << "\"" << getCENID() << "\" ";
     out << "\"" << getMAXID() << "\" ";
     out << "\"" << getMINID() << "\" ";
     out << "\"" << getHIGHID() << "\" ";
     out << ")";
  }
private:
   string data;
};

/*

Return Type S : Polygon additional geographic entity codes

*/
class RTS{
public:
  bool readFrom(string& line){
    if(line.length()<120)
      return false;
    if(line[0]!='S')
      return false;
    data = line;
    return true; 
  }
  string getFILE(){ return trim(data.substr(5,5));}
  string getCENID(){ return trim(data.substr(10,5));}
  string getPOLYID(){ return trim(data.substr(15,10));}
  string getWATER(){ return trim(data.substr(25,1));}
  string getMSACMSA(){ return trim(data.substr(26,4));}
  string getPMSA(){ return trim(data.substr(30,4));}
  string getAIANHH(){ return trim(data.substr(34,5));}
  string getAIANHHCE(){return trim(data.substr(39,4));}
  string getAIHHTLI(){return trim(data.substr(43,1));}
  string getSTATE(){return trim(data.substr(46,2));}
  string getCOUNTY(){return trim(data.substr(48,3));}
  string getCONCIT(){return trim(data.substr(51,5));}
  string getCOUSUB(){return trim(data.substr(56,5));}
  string getSUBMCD(){return trim(data.substr(61,5));}
  string getPLACE(){return trim(data.substr(66,5));}
  string getTRACT(){return trim(data.substr(71,6));}
  string getBLOCK(){return trim(data.substr(77,4));}
  string getCENSUS6(){ return trim(data.substr(81,1));}
  string getCDCU(){return trim(data.substr(82,2));}
  string getSLDU(){return trim(data.substr(84,3));}
  string getSLDL(){return trim(data.substr(87,3));}
  string getUGA(){return trim(data.substr(90,5));}
  string getBLKGRP(){return trim(data.substr(95,1));}
  string getVTD(){return trim(data.substr(96,6));}
  string getSTATECOL(){return trim(data.substr(102,2));}
  string getCOUNTYCOL(){return trim(data.substr(104,3));}
  string getBLOCKCOL(){return trim(data.substr(107,5));}
  string getBLKSUFCOL(){return trim(data.substr(112,1));}
  string getZCTA5(){return trim(data.substr(113,5));}
 

  static string Extension(){ return "RTS"; }
  static void writeRelType(ostream& out){
    out << "(rel (tuple (" << endl;
    out << "   (file  string)" << endl;
    out << "   (cenid  string)" << endl;
    out << "   (polyid  string)" << endl;
    out << "   (water  string)" << endl;
    out << "   (msacmsa  string)" << endl;
    out << "   (pmsa  string)" << endl;
    out << "   (aianhh  string)" << endl;
    out << "   (aianhhce  string)" << endl;
    out << "   (aihhtli  string)" << endl;
    out << "   (state  string)" << endl;
    out << "   (county  string)" << endl;
    out << "   (concit  string)" << endl;
    out << "   (cousub  string)" << endl;
    out << "   (submcd  string)" << endl;
    out << "   (place  string)" << endl;
    out << "   (tract  string)" << endl;
    out << "   (block  string)" << endl;
    out << "   (census6  string)" << endl;
    out << "   (cdcu  string)" << endl;
    out << "   (sldu  string)" << endl;
    out << "   (sldl  string)" << endl;
    out << "   (uga  string)" << endl;
    out << "   (blkgrp  string)" << endl;
    out << "   (vtd  string)" << endl;
    out << "   (statecol  string)" << endl;
    out << "   (countycol  string)" << endl;
    out << "   (blockcol  string)" << endl;
    out << "   (blksufcol  string)" << endl;
    out << "   (zcta5  string)" << endl;
    out << " ))) " << endl;
  }

  void writeTuple(ostream& out){
    out << "(";
    out << "\"" << getFILE() << "\" ";
    out << "\"" << getCENID() << "\" ";
    out << "\"" << getPOLYID() << "\" ";
    out << "\"" << getWATER() << "\" ";
    out << "\"" << getMSACMSA() << "\" ";
    out << "\"" << getPMSA() << "\" ";
    out << "\"" << getAIANHH() << "\" ";
    out << "\"" << getAIANHHCE() << "\" ";
    out << "\"" << getAIHHTLI() << "\" ";
    out << "\"" << getSTATE() << "\" ";
    out << "\"" << getCOUNTY() << "\" ";
    out << "\"" << getCONCIT() << "\" ";
    out << "\"" << getCOUSUB() << "\" ";
    out << "\"" << getSUBMCD() << "\" ";
    out << "\"" << getPLACE() << "\" ";
    out << "\"" << getTRACT() << "\" ";
    out << "\"" << getBLOCK() << "\" ";
    out << "\"" << getCENSUS6() << "\" ";
    out << "\"" << getCDCU() << "\" ";
    out << "\"" << getSLDU() << "\" ";
    out << "\"" << getSLDL() << "\" ";
    out << "\"" << getUGA() << "\" ";
    out << "\"" << getBLKGRP() << "\" ";
    out << "\"" << getVTD() << "\" ";
    out << "\"" << getSTATECOL() << "\" ";
    out << "\"" << getCOUNTYCOL() << "\" ";
    out << "\"" << getBLOCKCOL() << "\" ";
    out << "\"" << getBLKSUFCOL() << "\" ";
    out << "\"" << getZCTA5() << "\" ";
    out << ")" << endl;
  } 
private:
   string data;
};


/*
Record Type Z - ZIP+4 Codes

*/
class RTZ{
public:
  bool readFrom(string& line){
     if(line.length()<26)
        return false;
     if(line[0]!='Z')
        return false;
     data=line;
     return true;
  }
  string getTLID(){ return trim(data.substr(5,10));}
  string getRTSQ(){ return trim(data.substr(15,3));}
  string getZIP4L(){return trim(data.substr(18,4));}
  string getZIP4R(){return trim(data.substr(22,4));}

  static string Extension(){return "RTZ";}
  static void writeRelType(ostream& out){
     out << "(rel (tuple ( " << endl;
     out << "   (tlid string )" << endl;
     out << "   (rtsq string )" << endl;
     out << "   (zip4l string )" << endl;
     out << "   (zip4r string )" << endl;
     out << ")))" << endl;
  }
  void writeTuple(ostream& out){
   out << "(";
   out << "\"" << getTLID() << "\" ";
   out << "\"" << getRTSQ() << "\" ";
   out << "\"" << getZIP4L() << "\" ";
   out << "\"" << getZIP4R() << "\" ";
   out << ")" << endl;
  }

private:
  string data;
};



/*
Functions for writing a database from the tiger files 

*/
void writeDBHeader(ostream& out){
   out << "(DATABASE " << dbname << endl;
   if(oldStyle){
      out << "   (DESCRIPTIVE ALGEBRA)" << endl;
      out << "      (TYPES)" << endl;
      out << "      (OBJECTS) " << endl;
      out << "   (EXECUTABLE ALGEBRA)" << endl;
   }
   out << "     (TYPES) " << endl;
   out << "     (OBJECTS " << endl;
}

void writeDBFinish(ostream& out){
   out << " ))" << endl; // close objects, database
}



bool writeRT1_2Relation(string filename, ostream& out){
   int lines = 0;   
   RT1 rt1;

   out << "(OBJECT " << filename << "_RT12 () " ; 
   /*
    Because the record types 2 are not ordered, we have to scan 
    the whole file for each record. To avoid it, we cache the 
    complete file in an array and sort it
    for accerelation of the conversion.
   */
   
   // cache the RT2 file
   string fnrt2 = filename+".RT2";
   vector<RT2> allRT2;
   ifstream rt2in((srcdir+fnrt2).c_str());
   if(!rt2in){
     cerr << "Error in opening rt2 file " << endl; 
   } else{
     int no=0;	   
     while(!rt2in.eof()){
       string line;
       getline(rt2in,line);
       no++;
       RT2 rt2;
       if(line!=""){
          if(rt2.readFrom(line)){
             allRT2.push_back(rt2);
          }else{
             cerr << "wrong data detectec in " << fnrt2;
	     cerr << " line " << no << endl;
          }
       }  
     }
     rt2in.close();
   }
   // sort for better finding of elements
   sort(allRT2.begin(),allRT2.end());

 
   string fnrt1 = filename+".RT1";

   ifstream fin((srcdir+fnrt1).c_str());
   if(!fin){
      cerr << " error in opening stream" << fnrt1 <<  endl;
      return false;
   } 
   // write type information
   out << " ( rel (tuple ( ";
   out << "    (tlid string)" ; 
   out << "    (side1 string)" ;
   out << "    (source string)";
   out << "    (fedirp string)";
   out << "    (name string)";
   out << "    (type string)"; 
   out << "    (fedirs string)";
   out << "    (cfcc string)";
   out << "    (fraddl string)";
   out << "    (toaddl string)";
   out << "    (fraddr string)";
   out << "    (toaddr string)";
   out << "    (friaddl string)";
   out << "    (toiaddl string)";
   out << "    (friaddr string)";
   out << "    (toiaddr string)";
   out << "    (zipl string)";
   out << "    (zipr string)";
   out << "    (aianhhl string)";
   out << "    (aianhhr string)";
   out << "    (aihhtlil string)";
   out << "    (aihhtlir string)";
   out << "    (census1 string)";
   out << "    (census2 string)";
   out << "    (statel string)";
   out << "    (stater string)";
   out << "    (countyl string)";
   out << "    (countyr string)";
   out << "    (cousubl string)";
   out << "    (cousubr string)";
   out << "    (submcdl string)";
   out << "    (submcdr string)";
   out << "    (placel string)";
   out << "    (placer string)";
   out << "    (tract90l string)";
   out << "    (tract90r string)";
   out << "    (block90l string)";
   out << "    (block90r string)";
   out << "    (shape line)";
   out << " )))" << endl;

   // write the value
   out << " (" << endl; // open value list   
   LineWriter lw;
   string line;
   while(!fin.eof() ){
      getline(fin,line);
      if(line!=""){  
        if(!rt1.read(line)){
           cerr << "wrong data detected in  " << fnrt1  << endl;
        } else{
        out << "("; // tuple
        out << "\"" << rt1.getTLID() << "\" " ;
        out << "\"" << trim(rt1.getSIDE1()) << "\" ";
        out << "\"" << trim(rt1.getSOURCE()) << "\" ";
        out << "\"" << trim(rt1.getFEDIRP()) << "\" ";
        out << "\"" << trim(rt1.getFENAME()) << "\" ";
        out << "\"" << trim(rt1.getFETYPE()) << "\" ";
        out << "\"" << trim(rt1.getFEDIRS()) << "\" ";
        out << "\"" << trim(rt1.getCFCC()) << "\" ";
        out << "\"" << trim(rt1.getFRADDL()) << "\" ";
        out << "\"" << trim(rt1.getTOADDL()) << "\" ";
        out << "\"" << trim(rt1.getFRADDR()) << "\" ";
        out << "\"" << trim(rt1.getTOADDR()) << "\" ";
        out << "\"" << trim(rt1.getFRIADDL()) << "\" ";
        out << "\"" << trim(rt1.getTOIADDL()) << "\" ";
        out << "\"" << trim(rt1.getFRIADDR()) << "\" ";
        out << "\"" << trim(rt1.getTOIADDR()) << "\" ";
        out << "\"" << trim(rt1.getZIPL()) << "\" ";
        out << "\"" << trim(rt1.getZIPR()) << "\" ";
        out << "\"" << trim(rt1.getAIANHHL()) << "\" ";
        out << "\"" << trim(rt1.getAIANHHR()) << "\" ";
        out << "\"" << trim(rt1.getAIHHTLIL()) << "\" ";
        out << "\"" << trim(rt1.getAIHHTLIR()) << "\" ";
        out << "\"" << trim(rt1.getCENSUS1()) << "\" ";
        out << "\"" << trim(rt1.getCENSUS2()) << "\" ";
        out << "\"" << trim(rt1.getSTATEL()) << "\" ";
        out << "\"" << trim(rt1.getSTATER()) << "\" ";
        out << "\"" << trim(rt1.getCOUNTYL()) << "\" ";
        out << "\"" << trim(rt1.getCOUNTYR()) << "\" ";
        out << "\"" << trim(rt1.getCOUSUBL()) << "\" ";
        out << "\"" << trim(rt1.getCOUSUBR()) << "\" ";
        out << "\"" << trim(rt1.getSUBMCDL()) << "\" ";
        out << "\"" << trim(rt1.getSUBMCDR()) << "\" ";
        out << "\"" << trim(rt1.getPLACEL()) << "\" ";
        out << "\"" << trim(rt1.getPLACER()) << "\" ";
        out << "\"" << trim(rt1.getTRACT90L()) << "\" ";
        out << "\"" << trim(rt1.getTRACT90R()) << "\" ";
        out << "\"" << trim(rt1.getBLOCK90L()) << "\" ";
        out << "\"" << trim(rt1.getBLOCK90R()) << "\" ";

    // write the line resulting from the coordinates in this rt1 
   // and also the coordinates stored in rt2 
        lw.open(&out); 
        lw.addPoint(rt1.getFRLONG(),rt1.getFRLAT());
        RT2::writeDataFrom(allRT2,rt1.getTLID(),lw);
        lw.addPoint(rt1.getTOLONG(),rt1.getTOLAT());
        lw.close(); 

        out << ")" << endl; // tuple
        }
        lines++;
      }
   }
   if(oldStyle)
      out << ") () )" << endl;  // close value and object
   else
      out << ")   )" << endl; // the same thing without model
   fin.close();
   return true;
}

template<class RT>
void writeSimpleRelation(string filename,ostream& out){
   string fn = filename +"." + RT::Extension();
   ifstream in((srcdir+fn).c_str());
   if(!in){
      cerr << "Warning: can't not open file " << fn << endl;
      return;
   } else{
      out << "(OBJECT " << filename << "_" << RT::Extension() << " () " << endl;
      RT::writeRelType(out);
      out << "(";
      string line;
      int no = 0;
      RT rt;
      while(!in.eof()){
        getline(in,line);
        no++;
        if(line!=""){
           if(!rt.readFrom(line)){
             cerr << "Wrong data detected in " << fn << " line " << no << endl;
           }else{
              rt.writeTuple(out);
           }
        }
      }
      in.close();
   }
   if(oldStyle)
       out << " )  () ) " << endl;
   else
       out << " )  )" << endl;
}


/*

Writes the relation resulting from the .rt7 file to out.

The rt7 file constains landmark features which are points and other
ones which are linked to polygons. The records to extract can be
selected via the poinst argument.

*/
void writeRT7Relation(string filename,ostream& out, const bool points){
  string fnrt7 = filename+".RT7";
  ifstream in((srcdir+fnrt7).c_str());
  if(!in){
     cerr << " Warning: Can't open file " << fnrt7 << endl;
     return;
  }
  out << " ( OBJECT " << filename << "_RT7";
  if(points)
     out << "_points";
  out << " () " << endl;
  out << " ( rel (tuple ( "  << endl;
  out << "      (file  string) " << endl;
  out << "      (land  string) " << endl;
  out << "      (source  string) " << endl;
  out << "      (cfcc  string) " << endl;
  out << "      (laname  string) " << endl;
  if(points)
     out << "      (location point) " << endl;
  out << " )))" << endl;
  out << " (" << endl;
   string line;
   RT7 rt7;
   int no = 0;
   while(!in.eof()){
      getline(in,line);
      no++;
      if(line!=""){
        if(!rt7.readFrom(line)){
          cerr << " Wrong data detected in " << fnrt7 << " line " << no << endl;
        }else{
           if(rt7.isPoint()==points){
              out << "(";
              out << "\"" << rt7.getFILE() << "\" " << endl;
              out << "\"" << rt7.getLAND() << "\" " << endl;
              out << "\"" << rt7.getSOURCE() << "\" " << endl;
              out << "\"" << rt7.getCFCC() << "\" " << endl;
              out << "\"" << rt7.getLANAME() << "\" " << endl;
              if(points){
                  out << "(" << rt7.getLALONG() << " ";
                  out << rt7.getLALAT() << ") " << endl;
              }
              out << ")";
           }
        }
      }
   }
   in.close();
  if(oldStyle)
      out << ") () )" << endl;
  else
      out << " )  )" << endl;
}



void showUsage(){
   cerr << "tgr2sec [options] [files] " << endl;
   cerr << "where options are: " << endl;
   cerr << " --oldstyle : use old styled database and ";
   cerr << "object representations" << endl;
   cerr << " -o <filename>  : write to <filename>";
   cerr << " instead of cout " << endl;
   cerr << " -src <directoryname> : read the files from";
   cerr << " the given directory (default: tmp" << endl;
   cerr << " -sep <speparator> : specifiy the directory ";
   cerr << " separator (default '/') " << endl;
   cerr << " -objects : avoid to write the database header";
   cerr << " as well as the closing brackets" << endl;
   cerr << " -name: set the name of the database " << endl;
   cerr << " -rt? : write the relation reuslting from this file " << endl;
   cerr << " -all: write all posiible relations " << endl;
   cerr << " -help : print this message" << endl;  
   cerr << endl;
   cerr << " The tgr2line tool converts files in the";
   cerr << " TIGER/Line format (1999) into a " << endl;
   cerr << " database of the Secondo system. The database";
   cerr << " will contain up to 17 relations. " << endl;
   cerr << " The tool expected files whith extensions";
   cerr << " .RT1 ... .RTZ in a single directory. " << endl;
   cerr << " The given filenames which are arguments";
   cerr << " of this tool or given via the " << endl;
   cerr << " cin can't have a extension. " << endl;
   exit(0);  
}

/*

Function checking whether the given string is a symbol for the
Secondo parser.

*/
bool isLetter(char c){
  return (c>='a' && c<='z') || (c>='A' && c<='Z'); 
}

bool isDigit(char c){
  return c>='0' && c<='9';
}

bool isIdent(const string& s){
 if(s=="")
     return false;
 if(!isLetter(s[0]))
     return false;
 int len = s.length();
 for(int i=1;i<len;i++){
   char c = s[i];
   if(!isDigit(c) && !isLetter(c))
       return false;
 }
 return true;
}


/*

Some global variables which are 
set by the processArgument function

*/
ostream* out;
bool writeToFile;
bool writeDatabase;
bool writeRelation[17];
char separator;
bool ready;

/*
10 processArguments

This function evaluates the argument at position argpos in the 
argumentVector. It changes some global variables which are only used
in the writeObjects as well as in the main function. 
It returns a value of false  when the scanning of the arguments is finished,
and true when more arguments are possible. In case  
of an error, the showUsage function is called.

*/

bool processArgument(int argc, char** argv, int& argpos){
  if(argpos>=argc){ // no more arguments
     return false;
  }
  string arg=argv[argpos];
  if(arg[0]!='-'){ // no option, assuming here starts the file section
     return false;
  }

  if(arg=="-oldstyle" || arg=="--oldstyle"){
     oldStyle=true;
     argpos++;
     return true;
  }

  if(arg=="-help" || arg=="-?" || arg=="-h"){
    showUsage();
  }

  if(arg=="-objects"){
     writeDatabase=false;
     argpos++;
     return true;
  }
  
  if(arg=="-all"){
     for(int i=0;i<17;i++){
       writeRelation[i]=true;
     }
     argpos++;
     return true;
  }

  if(arg=="rt1"){
     writeRelation[0]=true;
     argpos++;
     return true;
  }
  if(arg=="rt3"){
     writeRelation[1]=true;
     argpos++;
     return true;
  }
  if(arg=="rt4"){
     writeRelation[2]=true;
     argpos++;
     return true;
  }
  if(arg=="rt5"){
     writeRelation[3]=true;
     argpos++;
     return true;
  }
  if(arg=="rt6"){
     writeRelation[4]=true;
     argpos++;
     return true;
  }
  if(arg=="rt7p"){
     writeRelation[5]=true;
     argpos++;
     return true;
  }
  if(arg=="rt7"){
     writeRelation[6]=true;
     argpos++;
     return true;
  }
  if(arg=="rt8"){
     writeRelation[7]=true;
     argpos++;
     return true;
  }
  if(arg=="rt9"){
     writeRelation[8]=true;
     argpos++;
     return true;
  }
  if(arg=="rta"){
     writeRelation[9]=true;
     argpos++;
     return true;
  }
  if(arg=="rtc"){
     writeRelation[10]=true;
     argpos++;
     return true;
  }
  if(arg=="rth"){
     writeRelation[11]=true;
     argpos++;
     return true;
  }
  if(arg=="rti"){
     writeRelation[12]=true;
     argpos++;
     return true;
  }

  if(arg=="rtp"){
     writeRelation[13]=true;
     argpos++;
     return true;
  }
  if(arg=="rtr"){
     writeRelation[14]=true;
     argpos++;
     return true;
  }
  if(arg=="rts"){
     writeRelation[15]=true;
     argpos++;
     return true;
  }
  if(arg=="rtz"){
     writeRelation[16]=true;
     argpos++;
     return true;
  }

  // handle options whith an argument
  string arg2;
  if(arg=="-o"){
     if(argc<=argpos+1){
        cerr << " Missing filename for '-o' option " << endl;
        cerr << " tgr2sec -help  for more information" << endl;
        exit(64);
     }
     if(writeToFile){ // -o option already occured
        ((ofstream*)out)->close();
        cerr << "-o option already used" << endl;
        exit(64);
     }
     writeToFile= true;
     arg2=argv[argpos+1];
     out = new ofstream(arg2.c_str());
     if(!(*out)){
       cerr << "cannot open file " << arg2 << " for writing " << endl;
       exit(73);
     }
     argpos = argpos+2;
     return true;
  }

  if(arg=="-sep"){
     if(argc<=argpos+1){
        cerr << " Missing argument for '-sep' option " << endl;
        exit(64);
     }
     arg2 = argv[argpos+1];
     if(arg2.length()!=1){
       cerr << " The argument for the -sep option";
       cerr << " must be a single character " << endl;
       exit(64);
     }
     separator=arg2[0];
     argpos=argpos+2;
     return true;
  }
  
  if(arg=="-src"){
     if(argc<=argpos+1){
        cerr << " Missing argument for '-src' option " << endl;
        exit(64);
     }
     arg2 = argv[argpos+1];
     srcdir=arg2;
     argpos+=2;
     return true;
  }
  if(arg=="-name"){
     if(argc<=argpos+1){
        cerr << " Missing argument for '-name' option " << endl;
        exit(64);
     }
     arg2 = argv[argpos+1];
     if(!isIdent(arg2)){
        cerr << "The name for the database must be an identifer" << endl;
        exit(64);
     }
     if(arg2.length()>47){
         cerr << "The name of the database is too long " << endl;
         exit(64);
     }
     dbname=arg2;
     argpos+=2;
     return true;
  }
  cerr << "Unknown Option " << endl;
  showUsage(); 
  return false; 
}



void writeObjects(string& filename,ostream& out){
   if(writeRelation[0]) writeRT1_2Relation(filename,out);  
   if(writeRelation[1]) writeSimpleRelation<RT3>(filename,out); 
   if(writeRelation[2]) writeSimpleRelation<RT4>(filename,out);  
   if(writeRelation[3]) writeSimpleRelation<RT5>(filename,out); 
   if(writeRelation[4]) writeSimpleRelation<RT6>(filename,out); 
   if(writeRelation[5]) writeRT7Relation(filename,out,true); 
   if(writeRelation[6]) writeRT7Relation(filename,out,false); 
   if(writeRelation[7]) writeSimpleRelation<RT8>(filename,out); 
   if(writeRelation[8]) writeSimpleRelation<RT9>(filename,out);
   if(writeRelation[9]) writeSimpleRelation<RTA>(filename,out);
   if(writeRelation[10]) writeSimpleRelation<RTC>(filename,out); 
   if(writeRelation[11]) writeSimpleRelation<RTH>(filename,out);
   if(writeRelation[12]) writeSimpleRelation<RTI>(filename,out);
   if(writeRelation[13]) writeSimpleRelation<RTP>(filename,out);
   if(writeRelation[14]) writeSimpleRelation<RTR>(filename,out);
   if(writeRelation[15]) writeSimpleRelation<RTS>(filename,out);
   if(writeRelation[16]) writeSimpleRelation<RTZ>(filename,out);
}

void onExit(int sig){
  if(!ready){
     cerr << "received interrupt while writing " << endl;
     cerr << " the content of the created file will be wrong" << endl;
     exit(12);
  }
  cerr << " received interrupt, finish the database " << endl;
  if(writeDatabase)
    writeDBFinish(*out);
  exit(0);
}


int main(int argc,char** argv){
 // initialize the values whith defaults
   out = &cout;  
   int firstfilepos=1;
   writeToFile=false;
   writeDatabase=true;
   for(int i=0;i<17;i++)
     writeRelation[i]=false;
   separator='/';
   ready = false;

  // process the argumentlist if any
  while(processArgument(argc,argv,firstfilepos)){
    ;
  }

  (*out) << fixed; // print out doubles with a fixed format
  (*out) << setprecision(8); // print out 8 digits after the comma

  bool rel=false;
  for(int i=0;i<17;i++){
     rel = rel || writeRelation[i];
  }
  if(!rel){ // no relation selected, assuming -all option
    for(int i=0;i<17;i++)
        writeRelation[i] = true;
  }

   srcdir=trim(srcdir);
   if(srcdir!=""){
      if(srcdir[srcdir.length()-1]!=separator)
         srcdir=srcdir+separator;
   }
   if(writeDatabase){ 
      writeDBHeader((*out));
   }

   ready=true; // here we can finish the database

   signal(SIGTERM,onExit);
   signal(SIGABRT,onExit);
   signal(SIGINT,onExit);

   if(argc>firstfilepos){ // read files as arguments
      string filename;
      for(int i=firstfilepos;i<argc;i++){
        filename=argv[i];
        cerr << "process file " << filename << endl;
        ready=false; 
        writeObjects(filename,(*out));
        ready=true;
      }
   }else{ // read files from stdin
       string filename;
       while(!cin.eof()){
          getline(cin,filename);
          if(filename!=""){
             cerr << "process file " << filename << endl;
             ready=false; 
             writeObjects(filename,(*out));
             ready=true;
          }
       }
   }

   if(writeDatabase){
      writeDBFinish((*out));
   }
   if(writeToFile){
      ((ofstream*)out)->close();
   }
 
} 
