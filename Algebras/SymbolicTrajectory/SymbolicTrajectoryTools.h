
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

 
string int2String(int i);
int str2Int(string const &text);
set<string> stringToSet(string input);
string setToString(set<string> input);
vector<string> splitPattern(string input);
char* convert(string arg);
string extendDate(string input, const bool start);
bool checkSemanticDate(const string text, const SecInterval uIv,
                       const bool resultNeeded);
bool checkDaytime(const string text, const SecInterval uIv);