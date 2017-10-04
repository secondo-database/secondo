/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen,
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


*/

#ifndef AISDECODE_H
#define AISDECODE_H

#include <fstream> 
#include <string>
#include <stdlib.h> 
#include <bitset>
#include <vector>
#include <assert.h>
#include <map>
#include <stdint.h>


namespace aisdecode{


class MessageBase{
  public:
     virtual void print() const = 0;
     virtual int getType() const = 0;
     virtual int getMMSI() const = 0;
     virtual ~MessageBase(){}
};

template<int size>
class Message: public MessageBase{

  public:
     Message(const std::string& _msg){
        setMessage(_msg);
     }

     Message(){}
  
     virtual ~Message(){}

     void setMessage(const std::string& _msg);

     int getSize()const{
        return size;
     } 
    
     inline int getStringSize() const{
        int s = (size+5) / 6;
        return s;
     }


     void createMsg();
     
    inline unsigned int extract(int start, int end)const {
      uint32_t result = 0u;
      int len = (end - start)  +1;
      assert(len<=32);
      int pos = len-1;
      for(int i=start;i<=end;i++){
         if(message[size-(i+1)]){
            result |= (1<<pos); 
         }
         pos--;
      } 
      return result;
   }

   std::string extractString(int start, int end) const;



     double computeLat(unsigned int rlat) const;

     
     double computeLon(unsigned int rlon) const;

     std::string decodeText(const std::string& text) const;

  protected: 
     std::string msg;
     std::bitset<size> message;
};


class Message1_3: public Message<168>{

  public:
    Message1_3(const std::string& _msg): Message<168>(_msg){
       extractInfos();
    }

    void print()const;
    int getType() const{ return messageType; }
    int getMMSI() const{return mmsi;}
  
    int messageType;
     int repeatIndicator;
     int mmsi;
     int status;
     int rot;
     int sog;
     int accuracy;
     double longitude;
     double latitude;
     int cog;
     int heading;
     int second  ;
     int maneuver;
     int spare;
     int raim;
     int rstatus;

  private:
     void extractInfos();
};


class Message4: public Message<168>{
  public:

  Message4(const std::string& _msg): Message<168>(_msg){
     extractInfos();
  }

  void print()const;

  int getType() const{ return type;}
    int getMMSI() const{return mmsi;}

     int type;
     int repeat;
     int mmsi  ;
     int year  ;
     int month ;
     int day   ;
     int hour  ;
     int minute;
     int second;
     int fix   ;
     double longitude;
     double latitude;
     int epfd;
     int spare;
     int raim ;
     int sotdma;

  private:
     void extractInfos();
};


class Message5: public Message<426>{

  public:

     Message5(const std::string& msg): Message<426>(msg){
        extractInfos();
     }

     void print() const;
     int getType() const{ return type;}
    int getMMSI() const{return mmsi;}

     int type;
     int repeat;
     int mmsi;
     int ais_version;
     int imo;
     std::string callSign;
     std::string vesselName;
     int shipType;
     int dimToBow;
     int dimToStern;
     int dimToPort;
     int dimToStarboard;
     int epfd;
     int month;
     int day;
     int hour;
     int minute;
     int draught;
     std::string destination;
     int dte;
  private:
     void extractInfos();
};


class Message9: public Message<168>{
   public:
      Message9(const std::string& msg): Message<168>(msg){
        extractInfos();
      }

   void print()const;
   int getType() const{ return type;}
    int getMMSI() const{return mmsi;}

   int type;
   int repeat;
   int mmsi;
   int alt;
   int sog;
   int accuracy;
   double longitude;
   double latitude;
   int cog;
   int second;
   int reserved;
   int dte;
   int assigned;
   int raim;
   int radio;

 private:
   void extractInfos();

};


class Message12: public Message<72>{

  public:
     Message12(const std::string& _msg): Message<72>(_msg.substr(0,12)),
                                         omsg(_msg){
       extractInfos();
     }

     void print()const;
     int getType() const{ return type;}
    int getMMSI() const{return source_mmsi;}

     std::string omsg;
     int type;
     int repeat;
     int source_mmsi;
     int sequence_number;
     int dest_mmsi;
     int retransmit;
     std::string text;

  private:
     void extractInfos();
};


class Message14 : public Message<1008>{
  public:
     Message14(const std::string& _msg): omsg(_msg){
        std::string fill(168-msg.length(),'@');
        setMessage(msg+fill);
        extractInfos();
     }

     void print()const;
     int getType() const{ return type;}
    int getMMSI() const{return mmsi;}
  
     std::string omsg;
     int type;
     int repeat;
     int mmsi;
     std::string text;

  private:
     void extractInfos();
};


class Message18: public Message<168>{
  public:
     Message18(const std::string& msg): Message<168>(msg){
         extractInfos();
     }

     void print()const;
     int getType() const{ return type;}
    int getMMSI() const{return mmsi;}

     int type;
     int repeat;
     int mmsi;
     int reserved1;
     int sog;
     int accuracy;
     double longitude;
     double latitude;
     int cog;
     int heading;
     int second;
     int reserved2;
     int cs;
     int display;
     int dsc;
     int band;
     int msg22;
     int assigned;
     int raim;
     int radio;

  private:
    void extractInfos();
};


class Message19 : public Message<312>{
  public:

    Message19(const std::string& msg) : Message<312>(msg){
       extractInfos();
    }

    void print()const;
    int getType() const{ return type;}
    int getMMSI() const{return mmsi;}

    int type;
    int repeat;
    int mmsi;
    int reserved;
    int sog;
    int accuracy;
    double longitude;
    double latitude;
    int cog;
    int heading;
    int second;
    int reserved2;
    std::string name;
    int shiptype;
    int dimToBow;
    int dimToStern;
    int dimToPort;
    int dimToStarboard;
    int epfd;
    int raim;
    int dte;
    int assigned;

  private:
    void extractInfos();
};

class Message24: public Message<162>{

  public:
    Message24(const std::string& msg): Message<162>(msg){
       extractInfos();
    }

    void print()const;
    int getType() const{ return type;}
    int getMMSI() const{return mmsi;}

     int type;
     int repeat;
     int mmsi;
     int partno;
     std::string shipsname;
     int shiptype;
     int vendorid;
     int model;
     int serial;
     std::string callsign;
     int dimToBow;
     int dimToStern;
     int dimToPort;
     int dimToStarboard;
     int mothership_mmsi;


   private:
     void extractInfos();

};




class MultiLineMessage{

  public:
     MultiLineMessage(){}

     MultiLineMessage(int fragmentcount, int fragmentnumber, 
                      const std::string& fragment,
                      int messageId, const std::string& rcc);

     void add(int fragmentcount, int fragmentnumber, 
              const std::string& fragment,
              int messageId, const std::string& rcc);

     bool isComplete()const{
       return fragmentsAvailable == fragments.size(); 
     }

     std::string getMessage() const;

  private:
    std::vector<std::string> fragments; // the fragments
    size_t fragmentsAvailable;  // number of fragments available
    int message_Id; // should be the same for all fragments
    std::string rcc;
};



class MultiLineMessage;

class aisdecoder{

  public:

  aisdecoder(const std::string& filename);

  ~aisdecoder(){
     if(buffer){
        delete[] buffer;
     }
  }

   MessageBase* getNextMessage();


 private:
    std::map<int,MultiLineMessage> incompleteMessages;    
    std::ifstream in;
    char* buffer;
    std::string line;


   MessageBase* decodeLine(const std::string& line);

   MessageBase* decodevdms(const std::string& line);
   MessageBase* decodeLine(std::string& talker, 
                            int fragmentcount, 
                            int fragmentnumber,  
                            int messageId,
                   std::string& rcc, 
                   std::string& msg, 
                            int fill, 
                            int check, 
                   std::string& rest);

   MessageBase* decode(const MultiLineMessage& mlm);

   MessageBase* decode(const int messagenumber, const std::string& msg);

   inline static int str2int(const std::string& s){
     return atoi(s.c_str());
   }

   bool getNextPart(const std::string& line, size_t& pos1, size_t&pos2,
                    std::string& result);

};

} // end of namespace aisdecode

#endif




