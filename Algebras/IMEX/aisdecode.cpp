
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

#include "aisdecode.h"

#include <iostream>
#include <fstream> 
#include <string>
#include <sstream>
#include <set>
#include <stdlib.h> 
#include <bitset>
#include <vector>
#include <assert.h>
#include <map>
#include <stdint.h>
#include <algorithm>


namespace aisdecode{


/*
1 Auxiliary functions


*/

const char chartbl[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";

void trim(std::string& str, std::string whiteSpaces=" \r\n\t") {
    std::string::size_type pos = str.find_last_not_of(whiteSpaces);
    if(pos != std::string::npos) {
      str.erase(pos + 1);
      pos = str.find_first_not_of(whiteSpaces);
      if(pos != std::string::npos){
         str.erase(0, pos);
      }
    } else {
     str.erase(str.begin(), str.end());
    }
}


/*
2 Implementation of class Message 

*/

template<int size>
void Message<size>::setMessage(const std::string& _msg){
   msg = _msg;
   size_t ssize = getStringSize();
   if(msg.size() < ssize){
      std::cerr << "expected minimum size is " << ssize << std::endl;
      std::cerr << "got message of length " << msg.length() << std::endl;
      throw 1001;
   }
   createMsg();
}


template<int size>
void Message<size>::createMsg(){
   int s = getStringSize();
   for(int i=0;i<s;i++){
     unsigned long c = msg[i];
     c = c - '0';
     if(c>40) c = c - 8;
     message = (message << 6);
     message |= std::bitset<size>((unsigned long)c);
   }
}
     

template<int size>
std::string Message<size>::extractString(int start, int end) const{
  std::bitset<size> b =  ((message >> (size - (end+1) ) ));
  int len = (end - start)  +1;
  assert(len % 6 == 0); // must be a multiple of 6
  len = len / 6; // number of chars
  std::string chars(len,'@');
  static std::bitset<size> mask(std::string("111111"));
  for(int i=0;i<len;i++){
     unsigned char c = (char)((b & mask).to_ulong());
     assert(c<0xf3);
     chars[len-(i+1)] = chartbl[c];
     b = (b >> 6);
  }
  std::replace(chars.begin(), chars.end(),'@',' ');
  trim(chars); 
  return chars;
}



template<int size>
double Message<size>::computeLat(unsigned int rlat) const{
   if(rlat == 0x3412140){ // special value for nbot available
     return 91;
   }
   int lat = rlat;
   if(rlat >=  0x4000000){
      lat = 0x8000000 - rlat;
      lat = -lat;
   }
   return lat/600000.0;
}
     
template<int size>
double Message<size>::computeLon(unsigned int rlon) const{
   if(rlon == 0x6791AC0){ // special value
      return 181;
   }
   int lon = rlon;
   if(rlon >= 0x8000000){
      lon = 0x10000000 - rlon;
      lon = -lon;
   }
   return lon / 600000.0;
}

template<int size>
std::string Message<size>::decodeText(const std::string& text) const{
   std::string text2(text.length(),'@');
   for(size_t i=0;i<text.length();i++){
      char c = text[i] - '0';
      if(c>40)  c = c-8;
      text2[i] = chartbl[(int)c];
   }
   return text2;
}



/*
3 Implementation of MultiLineMessage

*/

MultiLineMessage::MultiLineMessage(int fragmentcount, int fragmentnumber, 
                                   const std::string& fragment,
                                   int messageId, const std::string& rcc){
   for(int i=0;i<fragmentcount;i++){
      fragments.push_back("");
   }
   fragments[fragmentnumber-1] = fragment;
   fragmentsAvailable = 1;
   this->message_Id = messageId;
   this->rcc = rcc;
}

void MultiLineMessage::add(int fragmentcount, int fragmentnumber, 
                           const std::string& fragment,
                           int messageId, const std::string& rcc){
   if(  ((size_t) fragmentcount != fragments.size())
      || (fragmentnumber < 1)
      || ((size_t) fragmentnumber > fragments.size())
      || (fragments[fragmentnumber-1] !="")
      || (this->rcc != rcc) ){
       throw 1002;
   }
   fragments[fragmentnumber-1] = fragment;
   fragmentsAvailable++;   
  
} 


std::string MultiLineMessage::getMessage() const{
   assert(isComplete());
   std::stringstream ss;
   for(size_t i=0;i<fragments.size();i++){
     ss << fragments[i];
   }
   return ss.str();
}

/*
4 Message1-3

*/
void Message1_3::print()const{
   std::cout << "-----------------------------------------------" << std::endl;
   std::cout << "messagetype " << messageType << std::endl;
   std::cout << "repeatIndicator " << repeatIndicator << std::endl;
   std::cout << "mmsi " << mmsi << std::endl; 
   std::cout << "status " << status << std::endl;
   std::cout << "rate of turn " << rot << std::endl;
   std::cout << "speed over ground " << sog << std::endl;
   std::cout << "accuracy " << accuracy << std::endl;
   std::cout << "longitude " << longitude << std::endl;
   std::cout << "latitude " << latitude << std::endl;
   std::cout << "course over ground " << cog  << std::endl;
   std::cout << "heading " << heading << std::endl;
   std::cout << "second  " << second << std::endl;
   std::cout << "maneuver " <<  maneuver << std::endl;
   std::cout << "spare " << spare << std::endl;
   std::cout << "raim " << raim << std::endl;
   std::cout << "radio status " << rstatus << std::endl;
   if(msg.length()>28){
      std::cout << "more information available " << std::endl;
   } 
   std::cout << "----------------------------------------------" << std::endl;
}


void Message1_3::extractInfos(){
   messageType = extract(0,5);
   repeatIndicator = extract(6,7);
   mmsi = extract(8,37);
   status = extract(38,41);
   rot = extract(42,49);
   sog = extract(50,59);
   accuracy = extract(60,60);
   unsigned int lon = extract(61,88);
   longitude = computeLon(lon);
   unsigned int lat = extract(89,115);
   latitude = computeLat(lat);
   cog = extract(116,127);
   heading = extract(128,136);
   second    = extract(137,142);
   maneuver = extract(143,144);
   spare = extract(145,147);
   raim = extract(148,148);
   rstatus = extract(149,167);
}


/*
4 Message4

*/


void Message4::print()const{
   std::cout << "--------------------------------------------" << std::endl;
   std::cout << "type " << type << std::endl;
   std::cout << "repeat " << repeat << std::endl;
   std::cout << "mmsi " << mmsi << std::endl;
   std::cout << "year " << year << std::endl;
   std::cout << "month " << month << std::endl;
   std::cout << "day " << day << std::endl;
   std::cout << "hour " << hour << std::endl;
   std::cout << "minute " << minute << std::endl;
   std::cout << "second " << second << std::endl;
   std::cout << "fix " << fix << std::endl;
   std::cout << "longitude " << (longitude / 600000.0) << std::endl;
   std::cout << "latitude " << (latitude / 600000.0) << std::endl;
   std::cout << "epfd " << epfd << std::endl;
   std::cout << "spare " << spare << std::endl;
   std::cout << "raim " << raim << std::endl;
   std::cout << "sotdma " << sotdma << std::endl;
   if(msg.length()>28){
     std::cout << "more information available " << std::endl;
   }
   std::cout << "--------------------------------------------" << std::endl;
}

void Message4::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   mmsi = extract(8,37);
   year = extract(38,51);
   month = extract(52,55);
   day = extract(56,60);
   hour = extract(61,65);
   minute = extract(66,71);
   second = extract(72,77);
   fix = extract(78,78);
   longitude = computeLon(extract(79,106));
   latitude = computeLat(extract(107,133));
   epfd = extract(134,137);
   spare = extract(138,147);
   raim = extract(148,148);
   sotdma = extract(149,167);
   assert(type==4);
}

/*
5 Message5

*/

void Message5::print() const{
   std::cout << "--------------------------------------" << std::endl;
   std::cout << "type : " << type << std::endl;
   std::cout << "repeat : " << repeat << std::endl;
   std::cout << "mmsi :" << mmsi << std::endl;
   std::cout << "ais_version " <<  ais_version << std::endl;
   std::cout << "imo " <<  imo << std::endl;
   std::cout << "callSign " << callSign << std::endl;
   std::cout << "vesselName " << vesselName << std::endl;
   std::cout << "shiptype " << shipType << std::endl;
   std::cout << "dimToBow " << dimToBow  << std::endl;
   std::cout << "dimToStern " << dimToStern << std::endl;
   std::cout << "dimToPort " << dimToPort << std::endl;
   std::cout << "dimToStarboard" << dimToStarboard << std::endl;
   std::cout << "epfd " << epfd << std::endl;
   std::cout << "month " << month << std::endl;
   std::cout << "day " << day << std::endl;
   std::cout << "hour " << hour << std::endl;
   std::cout << "minute " << minute << std::endl;
   std::cout << "draught " << draught  << std::endl;
   std::cout << "destination " << destination << std::endl;
   std::cout << "dte " << dte << std::endl;
   std::cout << "--------------------------------------" << std::endl;
}

void Message5::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   mmsi = extract(8,37);
   ais_version = extract(38,39);
   imo = extract(40,69);
   callSign = extractString(70,111);
   vesselName = extractString(112,231);
   shipType = extract(232,239);
   dimToBow = extract(240,248);
   dimToStern = extract(249,257);
   dimToPort = extract(258,263);
   dimToStarboard = extract(264,269);
   epfd = extract(270,273);
   month = extract(274,277);
   day = extract(278,282);
   hour = extract(283,287);
   minute = extract(288,293);
   draught = extract(294,301);
   destination = extractString(302,421);
   dte = extract(422,422);
   assert(type==5);
}


/*
6 Message9

*/
void Message9::print()const{
   std::cout << "type " << type << std::endl;
   std::cout << "repeat " << repeat << std::endl;
   std::cout << "mmsi " << mmsi << std::endl;
   std::cout << "alt " << alt << std::endl;
   std::cout << "sog " << sog << std::endl;
   std::cout << "accuracy " << accuracy << std::endl;
   std::cout << "longitude " << longitude << std::endl;
   std::cout << "latitude " << latitude << std::endl;
   std::cout << "cog " << cog << std::endl;
   std::cout << "second " << second << std::endl;
   std::cout << "reserved " << reserved << std::endl;
   std::cout << "dte " << dte << std::endl;
   std::cout << "assigned " << assigned << std::endl;
   std::cout << "raim " << raim << std::endl;
   std::cout << "radio " << radio << std::endl;
}


void Message9::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   mmsi = extract(30,37);
   alt = extract(38,49);
   sog = extract(50,59);
   accuracy = extract(60,60);
   longitude = computeLon(extract(61,88));
   latitude = computeLat(extract(89,115));
   cog = extract(116,127);
   second = extract(128,133);
   reserved = extract(134,141);
   dte = extract(142,142);
   assigned = extract(146,146);
   raim = extract(147,147);
   radio = extract(148,167);
   assert(type==9);
}

/*
8 Message12

*/
void Message12::print()const{
   std::cout << "-----------------------------------" << std::endl;
   std::cout << "type " << type << std::endl;
   std::cout << "repeat " << repeat << std::endl;
   std::cout << "source " << source_mmsi << std::endl;
   std::cout << "sequence number " << sequence_number << std::endl;
   std::cout << "dest " <<dest_mmsi << std::endl;
   std::cout << "retransmit " << retransmit << std::endl;
   std::cout << "text " << text << std::endl;
   std::cout << "-----------------------------------" << std::endl;
}

void Message12::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   source_mmsi = extract(8,37);
   sequence_number = extract(38,39);
   dest_mmsi = extract(40,69);
   retransmit = extract(70,70);
   text = decodeText(omsg.substr(12));
   assert(type==12);
}


/*
9 Message14

*/

void Message14::print()const{
   std::cout << "---------------------------------" << std::endl;
   std::cout << " type " << type << std::endl;
   std::cout << " repeat " << repeat << std::endl;
   std::cout << " mmsi " << mmsi << std::endl;
   std::cout << " text " << text << std::endl;
   std::cout << "---------------------------------" << std::endl;
}

void Message14::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   mmsi = extract(8,37);
   //std::cout << "tbit = " << tbit << std::endl; 
   text = extractString(40,1007-2);
   int textlength = omsg.length() - 8;
   text = text.substr(0,textlength);
   assert(type=14);
}


/*
10 Message18

*/

void Message18::print()const{
   std::cout << "type " << type << std::endl;
   std::cout << "repeat " << repeat << std::endl;
   std::cout << "mmsi " << mmsi << std::endl;
   std::cout << "reserved1 " << reserved1 << std::endl;
   std::cout << "sog " << sog << std::endl;
   std::cout << "accuracy " << accuracy << std::endl;
   std::cout << "longitude " << longitude << std::endl;
   std::cout << "latitude " << latitude << std::endl;
   std::cout << "cog " << cog << std::endl;
   std::cout << "heading " << heading << std::endl;
   std::cout << "second " << second << std::endl;
   std::cout << "reserved2 " << reserved2 << std::endl;
   std::cout << "cs " << cs << std::endl;
   std::cout << "display " << display << std::endl;
   std::cout << "dsc " << dsc << std::endl;
   std::cout << "band " << band << std::endl;
   std::cout << "msg22 " << msg22 << std::endl;
   std::cout << "assigned " << assigned << std::endl;
   std::cout << "raim " << raim << std::endl;
   std::cout << "radio " << radio << std::endl;
}


void Message18::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   mmsi = extract(8,37);
   reserved1 = extract(38,45);
   sog = extract(46,55);
   accuracy = extract(56,56);
   longitude = computeLon(extract(57,84));
   latitude = computeLat(extract(85,111));
   cog = extract(112,123);
   heading = extract(124,132);
   second = extract(133,138);
   reserved2 = extract(139,140);
   cs = extract(141,141);
   display = extract(142,142);
   dsc = extract(143,143);
   band = extract(144,144);
   msg22 = extract(145,145);
   assigned = extract(146,146);
   raim = extract(147,147);
   radio = extract(148,167);
   assert(type=18);
}


/*
11 Message19

*/

void Message19::print()const{
   std::cout << "type" << type << std::endl;
   std::cout << "repeat" << repeat  << std::endl;
   std::cout << "mmsi " << mmsi << std::endl;
   std::cout << "reserved " << reserved << std::endl;
   std::cout << "sog " << sog  << std::endl;
   std::cout << "accuracy " << accuracy << std::endl;
   std::cout << "longitude " << longitude << std::endl;
   std::cout << "latitude " << latitude << std::endl;
   std::cout << "cog " << cog  << std::endl;
   std::cout << "heading " << heading  << std::endl;
   std::cout << "second " << second  << std::endl;
   std::cout << "reserved2 " << reserved2 << std::endl;
   std::cout << "name " << name << std::endl;
   std::cout << "shiptype " << shiptype  << std::endl;
   std::cout << "dimToBow " << dimToBow  << std::endl;
   std::cout << "dimToStern " << dimToStern << std::endl;
   std::cout << "dimToPort " << dimToPort  << std::endl;
   std::cout << "dimToStarboard " << dimToStarboard << std::endl;
   std::cout << "epfd " << epfd  << std::endl;
   std::cout << "raim " << raim  << std::endl;
   std::cout << "dte " << dte  << std::endl;
   std::cout << "assigned " << assigned << std::endl;
}

void Message19::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   mmsi = extract(8,37);
   reserved = extract(38,45);
   sog = extract(46,55);
   accuracy = extract(56,56);
   longitude = computeLon(extract(57,84));
   latitude = computeLat(extract(85,111));
   cog = extract(112,123);
   heading = extract(124,132);
   second = extract(133,138);
   reserved2 = extract(139,142);
   name = extractString(143,262);
   shiptype = extract(263,270);
   dimToBow = extract(271,279);
   dimToStern = extract(280,288);
   dimToPort = extract(289,294);
   dimToStarboard = extract(295,300);
   epfd = extract(301,304);
   raim = extract(305,305);
   dte = extract(306,306);
   assigned = extract(307,307);
   assert(type=19);
}

/*
13 Message24

*/
void Message24::print()const{
   std::cout << "type " << type << std::endl;
   std::cout << "repeat " << repeat << std::endl;
   std::cout << "mmsi " << mmsi << std::endl;
   std::cout << "partno " << partno << std::endl;
   std::cout << "shipsname " << shipsname << std::endl;
   std::cout << "shiptype " << shiptype << std::endl;
   std::cout << "vendorid " << vendorid << std::endl;
   std::cout << "model " << model << std::endl;
   std::cout << "serial " << serial << std::endl;
   std::cout << "callsign " << callsign << std::endl;
   std::cout << "dimToBow " << dimToBow << std::endl;
   std::cout << "dimToStern " << dimToStern << std::endl;
   std::cout << "dimToPort " << dimToPort << std::endl;
   std::cout << "dimToStarboard " << dimToStarboard << std::endl;
   std::cout << "mothership_mmsi " << mothership_mmsi << std::endl;
}

void Message24::extractInfos(){
   type = extract(0,5);
   repeat = extract(6,7);
   mmsi = extract(8,37);
   partno = extract(38,39);
   shipsname = extractString(40,159);
   shiptype = extract(40,47);
   vendorid = extract(48,65);
   model = extract(66,69);
   serial = extract(70,89);
   callsign = extractString(90,131);
   dimToBow = extract(132,140);
   dimToStern = extract(141,149);
   dimToPort = extract(150,155);
   dimToStarboard = extract(156,161);
   mothership_mmsi = extract(132,161); 
   assert(type==24);
}


/*
20 class aisdecoder

*/

aisdecoder::aisdecoder(const std::string& filename) {
    in.open(filename.c_str(),std::ios::in);
     buffer = 0;
     if(in){
        const int FILE_BUFFER_SIZE = 10485760; 
        buffer = new char[FILE_BUFFER_SIZE];
        in.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
     }
}

MessageBase* aisdecoder::getNextMessage(){
  while(!in.eof() && in.good()){
    getline(in,line);
    MessageBase* msg = 0;
    try{
       msg = decodeLine(line,0);
    } catch(...){
       std::cerr << "Exception during decoding line " << line << std::endl;
    }
    if(msg){
       return msg;
    }
  }
  try{
     in.close();
  } catch(...) {}
  return 0;
}

MessageBase* aisdecoder::getNextMessage(const int type){
  while(!in.eof() && in.good()){
    getline(in,line);
    MessageBase* msg = 0;
    try{
       msg = decodeLine(line,type);
    } catch(...){
       std::cerr << "Exception during decoding line " << line << std::endl;
    }
    if(msg){
       return msg;
    }
  }
  try{
     in.close();
  } catch(...) {}
  return 0;
}


MessageBase* aisdecoder::decodeLine(const std::string& line, const int type){
   if(line.size()==0){ // ignore empty lines
     return 0;
   }   
   size_t pos = line.find(",");
   if(pos==std::string::npos){
     std::cerr << "invalid line format, no comma inside" << std::endl;
     std::cerr << "line is " << line << std::endl; 
     return 0;
   }
   std::string talker = line.substr(0,pos);
   if(talker.length()==6 && talker[0]=='!' && talker.substr(3,2)=="VD"){
      return decodevdms(line, type);
   } else if(talker=="$PVOL"){
      return 0; // heartbeat message, ignore
   } else {
      std::cerr << "unknown talker: " << talker << std::endl;
      return 0;
   }
}

MessageBase* aisdecoder::decodevdms(const std::string& line, const int type){
   std::string talker = line.substr(1,2);
   size_t pos1 = 0;
   size_t pos2 = line.find(",");
   std::string tmp;
   if(!getNextPart(line,pos1,pos2,tmp)){
     std::cerr << "cannot extract framecount" << std::endl;
     std::cerr << "line is " << line << std::endl; 
     return 0;
   }
   int fragmentcount = str2int(tmp);
   if(!getNextPart(line,pos1,pos2,tmp)){
     std::cerr << "cannot extract frame number in line '" 
               << line << "'" <<  std::endl;
     std::cerr << "line is " << line << std::endl; 
     return 0;
   }
   int fragmentnumber = str2int(tmp);
   if(!getNextPart(line,pos1,pos2,tmp)){
     std::cerr << "cannot extract message id"  << std::endl;
     std::cerr << "line is " << line << std::endl; 
     return 0;
   }
   int messageId = str2int(tmp);
   if(!getNextPart(line,pos1,pos2,tmp)){
      std::cerr << "cannot extract radio channel code" << std::endl;
      std::cerr << "line is " << line << std::endl; 
      return 0;
   }
   std::string rcc = tmp;
   if(!getNextPart(line,pos1,pos2,tmp)){
     std::cerr << "cannot extract message " << std::endl;
     std::cerr << "line is " << line << std::endl; 
     return 0;
   }
   std::string msg = tmp;
   if(msg.length()<1){
       return 0;
   }
   // extract type and filter out if type not fits
   int t = msg[0] - '0';
   if(t>40) t -= 8;
   if(type>0 && t!=type){
       return 0;
   }

 
   pos1 = pos2;
   pos1++;
   pos2 = line.find(",",pos1);
   std::string fill = pos2==std::string::npos
                        ?  line.substr(pos1, std::string::npos)
                        :  line.substr(pos1, pos2-pos1);

   if(fill.size()!=4){
     std::cerr << "invalid length of fill" << std::endl;
     std::cerr << "fill is " << fill << std::endl;
     std::cerr << "line is " << line << std::endl; 
     return 0;
   }
   if( fill[1] != '*'){
      std::cerr << "missing '*' in last field" << std::endl;
      std::cerr << "line is " << line << std::endl; 
      return 0;
   }
   
   int check = atoi(("0x"+fill.substr(2)).c_str());

   std::string rest = pos2 == std::string::npos
                              ? line.substr(pos1,std::string::npos) 
                              : "";

   return decodeLine(talker, fragmentcount, fragmentnumber, messageId, rcc,
              msg, fill[0]-'0', check, rest);
}

MessageBase* aisdecoder::decodeLine(std::string& talker, 
                            int fragmentcount, 
                            int fragmentnumber,  
                            int messageId,
                   std::string& rcc, 
                   std::string& msg, 
                            int fill, 
                            int check, 
                   std::string& rest){
  if(fragmentcount > 1){
     if(incompleteMessages.find(messageId)==incompleteMessages.end()){
        MultiLineMessage mlm(fragmentcount, fragmentnumber, msg, messageId,rcc);
        incompleteMessages[messageId] = mlm;
     }  else {
        try{ 
            incompleteMessages[messageId].add(fragmentcount, fragmentnumber,
                                               msg, messageId,rcc);
        } catch(int err){
            incompleteMessages.erase(messageId);
            MultiLineMessage mlm(fragmentcount, fragmentnumber, msg, 
                                 messageId,rcc);
            incompleteMessages[messageId] = mlm;
        }
     }
     MultiLineMessage mlm = incompleteMessages[messageId]; 
     if(mlm.isComplete()){
        incompleteMessages.erase(messageId);
        return decode(mlm);
     }
     return 0;
  }

  if(msg.length() < 1){
    std::cerr << "empty message" << std::endl;
    return 0;
  }
  int messagenumber = msg[0] - 48;
  if(messagenumber > 40){
     messagenumber = messagenumber - 8;
  }
  if(messagenumber <1 || messagenumber > 27){
    std::cerr << "invalid message number " << messagenumber << std::endl;
    return 0;
  }
  return decode(messagenumber, msg);
}


MessageBase* aisdecoder::decode(const MultiLineMessage& mlm){
   assert(mlm.isComplete());
   std::string msg = mlm.getMessage();
   if(msg.size()<1){
      std::cerr << "found empty multi line message";
      return 0;
   }
   int messagenumber = msg[0] - 48;
   if(messagenumber > 40){
      messagenumber = messagenumber - 8;
   }
   return decode(messagenumber,msg);
}


MessageBase* aisdecoder::decode(const int messagenumber, 
                                const std::string& msg) {
  try{
  switch(messagenumber){
     case 1:
     case 2:
     case 3: return new Message1_3(msg);
     case 4: return new Message4(msg);
     case 5: return new Message5(msg);
     case 9: return new Message9(msg);
     case 12: return new Message12(msg);
     case 14: return new Message14(msg); 
     case 18: return new Message18(msg);
     case 19: return new Message19(msg);
     case 24: return new Message24(msg);
     default: std::cerr << "aisdecoder: message type " << messagenumber 
                        << "not implemented yet" << std::endl;
                        return 0;
  }
  } catch(int ex){
    std::cerr << "exception during processing message " << messagenumber
              << std::endl;
    std::cerr << "message is " << msg << std::endl;
    std::cerr << "message's length : " << msg.length() << std::endl;
    std::cerr << "Exception is " << ex << std::endl;
    return 0;
  } catch(...){
    std::cerr << " unknown exception during processing message " 
              << messagenumber << std::endl;
    return 0; 
  }

  return 0;
}


bool aisdecoder::getNextPart(const std::string& line, size_t& pos1, 
                             size_t&pos2, std::string& result){
  pos1 = pos2;
  pos1++;
  pos2 = line.find(",",pos1);
  if(pos2==std::string::npos){
    result = "";
    return false;
  }
  result = line.substr(pos1, pos2-pos1);
  return true;
}



} // end of namespace

