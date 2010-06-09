/******************************************************************************

//paragraph [1] Title: [{\Large \bf] [}]

[1] Database generator for upload unit relations

June 2010, Daniel Brockmann

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

const int maxMoID = 10000; // Max number of moving object id's
int posXArray[maxMoID];    // Last x-position
int posYArray[maxMoID];    // Last y-position
int year, month, day, hour, minute, second, moID;

/******************************************************************************

computeTimeStamp method

******************************************************************************/

string computeTimeStamp(int i)
{
  stringstream oss;
  
  second++;
  if((second%60) == 0) 
  {
    second = 0;
    minute++;
    if((minute%60)  == 0)
    {
      minute = 0;
      hour++;
      if((hour%24) == 0)
      {
        hour = 0;
        day++;
      } 
    }
  }
  
  int newMonth = month;
  if(month == 1  || month == 3 || month == 5 || month == 7 || 
     month == 8  || month == 10 ||month == 12)
  {
    if((day%32) == 0)   { day = 1; newMonth++; }
    if((month%32) == 0) { month = 1; year++; }
  }
  if(month == 4  || month == 6 || month == 9 || month == 11)
  {
    if((day%31) == 0)   { day   = 1; newMonth++; }
    if((month%31) == 0) { month = 1; year++; }
  }
  if(month == 2)
  {
    if((day%29) == 0)   { day   = 1; newMonth++; }
    if((month%29) == 0) { month = 1; year++; }
  }
  month = newMonth;
  
  oss << "\"" << year;
  
  if (month  < 10) oss << "-0" << month;
  else oss << "-" << month;
  
  if (day    < 10) oss << "-0" << day;
  else oss << "-" << day;

  if (hour < 10) oss << "-0" << hour;
  else oss << "-" << hour;

  if (minute < 10) oss << ":0" << minute;
  else oss << ":" << minute;
  
  if (second < 10) oss << ":0" << second << "\"";
  else oss << ":" << second << "\"";
  
  return oss.str();
}

/******************************************************************************

Main function

******************************************************************************/

int main()
{
  ofstream output("UploadDB");
  output << "(DATABASE UPLOAD (TYPES)(OBJECTS" << endl;

/******************************************************************************

Relation with 10000 upload units 

******************************************************************************/
  
  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload10000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 10000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;
  
/******************************************************************************

Relation with 50000 upload units 

******************************************************************************/

  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload50000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 50000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;

/******************************************************************************

Relation with 100000 upload units 

******************************************************************************/

  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload100000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 100000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;

/******************************************************************************

Relation with 200000 upload units 

******************************************************************************/

  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload200000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 200000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;

/******************************************************************************

Relation with 300000 upload units 

******************************************************************************/

  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload300000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 300000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;

/******************************************************************************

Relation with 400000 upload units 

******************************************************************************/

  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload400000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 400000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;

/******************************************************************************

Relation with 500000 upload units 

******************************************************************************/

  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload500000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 500000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;

/******************************************************************************

Relation with 1000000 upload units 

******************************************************************************/

  year = 2010; month = 1; day = 1; hour = 0; minute = 0; second = 0;
  output << "(OBJECT Upload1000000Rel()(rel(tuple((upload uploadunit))))(";
  for (int i = 0; i < maxMoID; i++) { posXArray[i] = (int)rand()%1000; }
  for (int i = 0; i < maxMoID; i++) { posYArray[i] = (int)rand()%1000; }
  for (int i = 0; i < 1000000; i++)
  {
   output << endl;
   // moving object id
   moID = ((int)rand()%maxMoID);
   output << "(( " << moID << " ";
   // time stamp
   output << computeTimeStamp(i) << " ";
   // x position
   output << "( "<< (posXArray[moID])%1000 << "." << ((int)rand()%100) << " ";
   posXArray[moID]++;
   // y position
   output << (posYArray[moID])%1000 << "." << ((int)rand()%100) << " )))";
   posYArray[moID]++;
  }
  output << "))" << endl;

  // End of database
  output << "))" << endl;
}
