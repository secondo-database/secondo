
/*

*/

#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include "TemporalAlgebra.h"
#include "TemporalUnitAlgebra.h"

using namespace std; 

 
//string intToString(int i);
int str2Int(string const &text);
vector<string> split2(string const& text, const char delemiter);
vector<string> split(string const& text, const char delemiter);
vector<string> msplit(string const& text, const char delemiter);
vector<string> split(string const& text, const string delemiter);
void deleteSpaces(string &text);
//string trim(string text);
vector<string> getElementsFromSet(string const set); 



set<string> stringToSet(string input);
string setToString(set<string> input);
vector<string> splitPattern(string input);
char* convert(string arg);
string extendDate(string input, const bool start);
bool checkSemanticDate(const string text, const SecInterval uInterval);
bool checkDaytime(const string text, const SecInterval uInterval);