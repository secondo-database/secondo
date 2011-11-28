#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
 
using namespace std; 
 
// struct SemTime;
 class SymbolicTimeRange; 
 string getFullDate(string const &text, bool const startDate = true);
 string getFullTime(string const &text, bool const startTime = true);
 string getFullDateTime(string const &text, bool const startDateTime = true);
 string getFullRange(string const &text);
 string getFullTimeRange(string const &text);
 bool checkMaskOfDate(string const &mask);
 bool checkMaskOfTime(string const &mask);
 bool checkMaskOfDateTime(string const &mask);
 bool checkRangeMask(string const &mask);
 bool checkTimeRangeMask(string const &mask);
 int getType(string text, string &datetime);
 string getMask(string const &text);
 bool isDate(string const &text); 
 bool isNumeric(string const &text);
