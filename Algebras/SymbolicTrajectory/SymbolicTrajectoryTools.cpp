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

/*
function ~convert~
Converts a string into a char[*]

*/
char* convert(string arg) {
  return strdup(arg.c_str());
}

/*
function ~stringToSet~
Splits a string {a, b, c, ...} into a set of strings a, b, c, ...

*/
set<string> stringToSet(string input) {
  cout << "stringToSet called with string " << input << endl;
  string element, contents;
  set<string> result;
  if (input.at(0) == '{')
    contents.assign(input.substr(1, input.length() - 2));
  else {
    result.insert(input);
    cout << "element " << input << " added to set" << endl;
    return result;
  }
  deleteSpaces(contents);
  istringstream iss(contents);
  while (getline(iss, element, ',')) {
    result.insert(element);
    cout << "element " << element << " added" << endl;
  }
  return result;
}

/*
function ~setToString~

*/
string setToString(set<string> input) {
  set<string>::iterator i;
  stringstream result;
  if (input.size() == 1) {
    result << *(input.begin());
  }
  else { // input.size() > 1
    result << "{";
    for (i = input.begin(); i != input.end(); i++) {
      if (i != input.begin()) {
        result << ", ";
      }
      result << *i;
    }
    result << "}";
  }
  return result.str();
}

vector<string> splitPattern(string input) {
  cout << "splitPattern called with string " << input << endl;
  vector<string> result;
  size_t pos = input.find('{');
  if (pos == 0) { // ({2012, 2013}, ...)
    cout << "{ in the beginning" << endl;
    result.push_back(input.substr(0, input.find('}') + 1));
    if (input.find('{', 1) != string::npos) { // ({2012, 2013}, {a, b, c})
      result.push_back(input.substr(input.find('{', 1)));
    }
    else { // ({2012, 2013} a)
      result.push_back(input.substr(input.find('}') + 1));
    }
  }
  else if (pos == string::npos) { // no set
    cout << "no { found" << endl;
    if (input.find(' ') == string::npos) { // * or +
      result.push_back(input);
    }
    else { // (2012 a)
      result.push_back(input.substr(0, input.find(' ')));
      result.push_back(input.substr(input.find(' ')));
    }
  }
  else { // (2012 {a, b, c})
    result.push_back(input.substr(0, input.find(' ')));
    result.push_back(input.substr(input.find(' ')));
  }
  for (unsigned int i = 0; i < result.size(); i++) {
    deleteSpaces(result[i]);
    cout << "|" << result[i] << "|";
  }
  cout << endl;
  return result;
}

/*
function ~extendDate~
Takes a datetime string and extends it to the format YYYY-MM-DD-HH:MM:SS.MMM,
the variable ~start~ decides on the values.

*/
string extendDate(string input, const bool start) {
  string result, mask;
  int month, daysInMonth, year;
  if (start) {
    mask.assign("-01-01-00:00:00.000");
  }
  else {
    mask.assign("-12-31-23:59:59.999");
  }
  if (input[input.size() - 1] == '-') { // handle case 2011-04-02-
    input.resize(input.size() - 1);
  }
  result.assign(input);
  int pos = 1;
  if ((pos = input.find('-', pos)) == string::npos) {
    result.append(mask.substr(0));
    return result;
  }
  pos++;
  if ((pos = input.find('-', pos)) == string::npos) {
    if (!start) {
      stringstream yearStream(input.substr(0, input.find('-')));
      stringstream monthStream(input.substr(input.find('-') + 1));
      stringstream dayStream;
      if ((monthStream >> month).fail()) {
        cout << "month stringstream error" << endl;
        return input + mask.substr(3);
      }
      else {
        switch (month){
          case 1:
          case 3:
          case 5:
          case 7:
          case 8:
          case 10:
          case 12:
            daysInMonth = 31;
            break;
          case 4:
          case 6:
          case 9:
          case 11:
            daysInMonth = 30;
            break;
          case 2:
            if ((yearStream >> year).fail()) {
              cout << "year stringstream error" << endl;
              return input + mask.substr(3);
            }
            else {
              if (((year % 4 == 0) && (year % 100 != 0))
                || (year % 400 == 0)) {
                daysInMonth = 29;
              }
              else {
                daysInMonth = 28;
              }
            }
            break;
          default: // should not occur
            cout << "month " << month << " does not exist" << endl;
            return input + mask.substr(3);
        }
      }
    dayStream << "-" << daysInMonth;
    result.append(dayStream.str());
    result.append(mask.substr(6));
    return result;
    }
  }
  pos++;
  if ((pos = input.find('-', pos)) == string::npos) {
    result.append(mask.substr(6));
    return result;
  }
  pos++;
  if ((pos = input.find(':', pos)) == string::npos) {
    result.append(mask.substr(9));
    return result;
  }
  pos++;
  if ((pos = input.find(':', pos)) == string::npos) {
    result.append(mask.substr(12));
    return result;
  }
  pos++;
  if ((pos = input.find('.', pos)) == string::npos) {
    result.append(mask.substr(15));
    return result;
  }
  return input;
}

/*
function ~checkSemanticDate~
Checks whether text is a valid semantic date string and contains the time
interval defined by start and end.

*/
bool checkSemanticDate(const string text, const SecInterval uInterval) {
  string weekdays[7] = {"monday", "tuesday", "wednesday", "thursday", "friday",
                        "saturday", "sunday"};
  string months[12] = {"january", "february", "march", "april", "may", "june",
                       "july", "august", "september", "october", "november",
                       "december"};
  string daytimes[4] = {"morning", "afternoon", "evening", "night"};
  Instant uStart = uInterval.start;
  Instant uEnd = uInterval.end;
  if ((uStart.GetYear() == uEnd.GetYear()) // year and month of start and end
       && (uStart.GetMonth() == uEnd.GetMonth())) { //must coincode for a match
    for (int i = 0; i < 12; i++) { // handle months
      if (!text.compare(months[i])) {
        return (i == uStart.GetMonth() - 1);
      }
    } // for weekdays and daytimes, start and end day have to coincide
    if (uStart.GetGregDay() == uEnd.GetGregDay()) {
      for (int i = 0; i < 7; i++) { // handle weekdays
        if (!text.compare(weekdays[i])) {
          return (i == uStart.GetWeekday());
        }
      }
      for (int i = 0; i < 4; i++) { // handle daytimes
        if (!text.compare(daytimes[i])) {
          switch (i) {
            case 0:
              return (uStart.GetHour() >= 0) && (uEnd.GetHour() <= 11);
            case 1:
              return (uStart.GetHour() >= 12) && (uEnd.GetHour() <= 16);
            case 2:
              return (uStart.GetHour() >= 17) && (uEnd.GetHour() <= 20);
            case 3:
              return (uStart.GetHour() >= 21) && (uEnd.GetHour() <= 23);
            default: // cannot occur
              cout << "daytime error" << endl;
              return false;
          }
        }
      }
    }
  } // different months => match impossible
  return false;
}

/*
function ~checkDaytime~
Checks whether the daytime interval resulting from text (e.g., 0:00[~]8:00)
contains the one from the unit label.

*/
bool checkDaytime(const string text, const SecInterval uInterval) {
  if ((uInterval.start.GetYear() == uInterval.end.GetYear())
       && (uInterval.start.GetMonth() == uInterval.end.GetMonth())
       && (uInterval.start.GetGregDay() == uInterval.end.GetGregDay())) {
    string startString, endString;
    stringstream startStringstream;
    startStringstream << uInterval.start.GetYear() << "-"
      << uInterval.start.GetMonth() << "-"
      << uInterval.start.GetGregDay() << "-";
    startString.assign(startStringstream.str());
    endString.assign(startString);
    startString.append(text.substr(0, text.find('~')));
    endString.append(text.substr(text.find('~') + 1));
    Instant *pStart = new DateTime(instanttype);
    Instant *pEnd = new DateTime(instanttype);
    if (!pStart->ReadFrom(extendDate(startString, true))) {
      cout << "error: ReadFrom " << extendDate(startString, true) << endl;
      return false;
    }
    if (!pEnd->ReadFrom(extendDate(endString, false))) {
      cout << "error: ReadFrom " << extendDate(endString, false) << endl;
      return false;
    }
    SecInterval *pInterval = new SecInterval(*pStart, *pEnd, true, true);
    bool result = pInterval->Contains(uInterval);
    delete pInterval;
    delete pStart;
    delete pEnd;
    return result;
  } // no match possible in case of different days
  return false;
}
