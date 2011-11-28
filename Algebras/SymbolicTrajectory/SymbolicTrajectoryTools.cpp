#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include "SymbolicTrajectoryTools.h"
#include "CharTransform.h"

using namespace std;

/* 
string int2Str(int i) 
{
   string result;
   ostringstream oss;
   
   oss << i;
   result = oss.str();   
   
   return result; 
}
*/


int str2Int(string const &text) 
{

  int result;
  stringstream sstream(text);

  if( (sstream >> result).fail() )
  {
      result = 0;
  }   
   
   return result; 
}



vector<string> split(string const& text, const char delemiter)
{
    vector<string> result(0);
    string token;
    istringstream iss(text);

    while ( getline(iss, token, delemiter) )
    {
      result.push_back(token);
    }

	return result;
}


vector<string> msplit(string const& text, const char delemiter)
{   // ignores zero tokens
    vector<string> result;
    result.clear();
    string token;
    istringstream iss(text);

    if (text.empty()) return result; 
    
    while ( getline(iss, token, delemiter) )
    {
      if (!token.empty()) result.push_back(token);
    }

	return result;
}


vector<string> split(string const& text, const string delemiter)
{
    vector<string> result(0);
    size_t last_position(0);
    size_t position(0);
	
	position = text.find(delemiter, last_position);
	while (position != string::npos) 
	{
       result.push_back(text.substr(last_position, position - last_position));
	   last_position = position + delemiter.length();
	   position = text.find(delemiter, last_position);	   
	}
    result.push_back(text.substr(last_position));	

	return result;
}


void deleteSpaces(string &text) 
{
   size_t pos = 0;
   
   while ( (pos = text.find(' ', pos)) !=  string::npos)
   {
      text.erase(pos, 1);
   }
}

/*
string trim(string text) 
{
   int start = 0;
   int end = text.length() - 1;
   
   while (start != (text.length() - 1) && text[start] == ' ') ++start;
   while (end != 0 && text[end] == ' ') --end;

   text = text.substr(start, end - start + 1);
   
   return text;
}
*/

vector<string> getElementsFromSet(string const set)
{
   vector<string> elements;
   
   elements = msplit(set, ',');

   for (size_t i = 0; i < elements.size(); ++i) elements[i] = trim(elements[i]);		
  
   return elements;  
}

