/*


*/

#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <set>
#include <map>
#include "SymbolicTrajectoryTools.h"
#include "CharTransform.h"
#include <string.h>

using namespace std;

char* convert(string arg) {
  return strdup(arg.c_str());
}

/*
function ~splitLabel~
Splits a string {a, b, c, ...} into a set of strings a, b, c, ...

*/
set<string> splitLabel(string labelset) {
  cout << "splitLabel called" << endl;
  string labels, label;
  set<string> result;
  if (labelset.at(0) == '{')
    labels.assign(labelset.substr(1, labelset.length() - 2));
  else {
    result.insert(labelset);
    cout << "label " << labelset << " added" << endl;
    return result;
  }
  size_t pos = 0;
  while ((pos = labels.find(' ', pos)) != string::npos) {
    labels.erase(pos, 1); //delete whitespaces
  }
  istringstream iss(labels);
  while (getline(iss, label, ',')) {
    result.insert(label);
    cout << "label " << label << " added" << endl;
  }
  return result;
}

int str2Int(string const &text) {
  int result;
  stringstream ss(text);
  if((ss >> result).fail())
    result = 0;
  return result;
}



vector<string> split(string const& text, const char delemiter) {
  vector<string> result(0);
  string token;
  istringstream iss(text);
  while (getline(iss, token, delemiter))
    result.push_back(token);
  return result;
}

   // ignores zero tokens
vector<string> msplit(string const& text, const char delemiter) {
  vector<string> result;
  result.clear();
  string token;
  istringstream iss(text);
  if (text.empty())
    return result;
  while (getline(iss, token, delemiter))
    if (!token.empty())
      result.push_back(token);
  return result;
}


vector<string> split(string const& text, const string delemiter) {
  vector<string> result(0);
  size_t last_position(0);
  size_t position(0);
  position = text.find(delemiter, last_position);
  while (position != string::npos) {
    result.push_back(text.substr(last_position, position - last_position));
    last_position = position + delemiter.length();
    position = text.find(delemiter, last_position);
  }
  result.push_back(text.substr(last_position));
  return result;
}


void deleteSpaces(string &text) {
  size_t pos = 0;
  while ((pos = text.find(' ', pos)) != string::npos)
    text.erase(pos, 1);
}


vector<string> getElementsFromSet(string const set) {
  vector<string> elements;
  elements = msplit(set, ',');
  for (size_t i = 0; i < elements.size(); ++i)
    elements[i] = trim(elements[i]);
  return elements;
}

string extendDateString(const string input) {
  string result, mask;
  mask.assign("-01-01-00:00:00.000");
  result.assign(input);
  int pos = 1;
  char nextLimit = '-';
  // TODO: build a while loop
  if ((pos = input.find(nextLimit, pos)) == string::npos) {
    result.append("-01-01-00:00:00.000");
    return result;
  }
  pos++;
  if ((pos = input.find(nextLimit, pos)) == string::npos) {
    result.append("-01-00:00:00.000");
    return result;
  }
  pos++;
  return mask;
}
