/*
----
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
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Auxiliary functions of the Symbolic Trajectory Algebra

Started March 2012, Fabio Vald\'{e}s

*/
#include "SymbolicTrajectoryTools.h"

extern QueryProcessor *qp;

using namespace std;

string int2String(int i) {
  stringstream result;
  result << i;
  return result.str();
}

int str2Int(string const &text) {
  int result;
  stringstream ss(text);
  if((ss >> result).fail())
    result = 0;
  return result;
}

void deleteSpaces(string &text) {
  size_t pos = 0;
  while ((pos = text.find(' ', pos)) != string::npos)
    text.erase(pos, 1);
}

/*
\subsection{Function ~convert~}

Converts a string into a char[*].

*/
char* convert(string arg) {
  return strdup(arg.c_str());
}

/*
\subsection{Function ~stringToSet~}

Splits a string {a, b, c, ...} into a set of strings a, b, c, ...

*/
set<string> stringToSet(string input) {
  string element, contents(input);
  int limitpos;
  set<string> result;
  contents.erase(0, input.find_first_not_of(" "));
  if (!contents.compare("_")) {
    return result;
  }
  if (contents.at(0) == '{') {
    contents.assign(contents.substr(1, contents.length() - 2));
  }
  while (!contents.empty()) {
    contents.erase(0, contents.find_first_not_of(", "));
    if (contents.at(0) == '\"') {
      limitpos = contents.find('\"', 1);
      element = contents.substr(1, limitpos - 1);
      contents.erase(0, limitpos + 1);
    }
    else {
      limitpos = contents.find_first_of(",");
      element = contents.substr(0, limitpos);
      contents.erase(0, limitpos);
    }
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
  else if (input.size() > 1) {
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
  vector<string> result;
  if (!input.size()) {
    return result;
  }
  size_t pos = input.find('{');
  if (pos == 0) { // ({2012, 2013}, ...)
    result.push_back(input.substr(0, input.find('}') + 1));
    if (input.find('{', 1) != string::npos) { // ({2012, 2013}, {a, b, c})
      result.push_back(input.substr(input.find('{', 1)));
    }
    else { // ({2012, 2013} a)
      result.push_back(input.substr(input.find('}') + 1));
    }
  }
  else if (pos == string::npos) { // no set
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
Checks whether ~text~ is a valid semantic date string or a valid database object
of type ~Periods~ and contains the time interval defined by ~uIv~. If the
parameter ~resultNeeded~ is ~false~, the function will only check the validity
of ~text~.

*/
bool checkSemanticDate(const string text, const SecInterval uIv,
                       const bool eval) {
  string weekdays[7] = {"monday", "tuesday", "wednesday", "thursday", "friday",
                        "saturday", "sunday"};
  string months[12] = {"january", "february", "march", "april", "may", "june",
                       "july", "august", "september", "october", "november",
                       "december"};
  string daytimes[4] = {"morning", "afternoon", "evening", "night"};
  Instant uStart = uIv.start;
  Instant uEnd = uIv.end;
  bool isSemanticDate = false;
  for (int i = 0; i < 12; i++) {
    if (!text.compare(months[i])) {
      isSemanticDate = true;
    }
    else if (i < 7) {
      if (!text.compare(weekdays[i])) {
        isSemanticDate = true;
      }
      else if (i < 4) {
        if (!text.compare(daytimes[i])) {
          isSemanticDate = true;
        }
      }
    }
  }
  if (isSemanticDate && !eval) {
    return true;
  }
  else if (isSemanticDate && eval) {
    if ((uStart.GetYear() == uEnd.GetYear()) // year and month of start and end
         && (uStart.GetMonth() == uEnd.GetMonth())) { //must coincode for match
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
  }
  else {
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    if (!Periods::checkType(sc->GetObjectTypeExpr(text))) {
      cout << text << " is not a periods object." << endl;
      return false;
    }
    else {
      const int errPos = 0;
      ListExpr errInfo;
      bool ok;
      Word pWord = sc->InObject(sc->GetObjectTypeExpr(text),
                                sc->GetObjectValue(text), errPos, errInfo, ok);
      if (!ok) {
        cout << "Error: InObject failed." << endl;
        return false;
      }
      else {
        Periods* period = static_cast<Periods*>(pWord.addr);
        return (eval ? period->Contains(uIv) : true);
      }
    }
  }
  return false;
}

/*
function ~checkDaytime~

Checks whether the daytime interval resulting from text (e.g., 0:00[~]8:00)
contains the one from the unit label.

*/
bool checkDaytime(const string text, const SecInterval uIv) {
  if ((uIv.start.GetYear() == uIv.end.GetYear())
       && (uIv.start.GetMonth() == uIv.end.GetMonth())
       && (uIv.start.GetGregDay() == uIv.end.GetGregDay())) {
    string startString, endString;
    stringstream startStream;
    startStream << uIv.start.GetYear() << "-" << uIv.start.GetMonth() << "-"
                << uIv.start.GetGregDay() << "-";
    startString.assign(startStream.str());
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
    bool result = pInterval->Contains(uIv);
    delete pInterval;
    delete pStart;
    delete pEnd;
    return result;
  } // no match possible in case of different days
  return false;
}

/*
\subsection{Function ~evaluate~}

In case of testing a condition's syntactical correctness, we are only
interested in the result type. Thus, ~eval~ is ~false~, an operator tree
is built, and ~true~ is returned if and only if the result type is ~boolean~.
On the other hand, if we ask for the condition result, the function executes
the appropiate query and returns its result.

*/
bool evaluate(string input, const bool eval) {
  bool isBool(false), isTrue(false);
  SecParser condParser;
  string queryStr;
  ListExpr queryList, resultType;
  Word queryResult;
  bool correct(false), evaluable(false), defined(false), isFunction(false);
  OpTree tree = 0;
  input.insert(0, "query ");
  switch (condParser.Text2List(input, queryStr)) {
    case 0:
      if (!nl->ReadFromString(queryStr, queryList)) {
        cout << "ReadFromString error" << endl;
      }
      else {
        if (nl->IsEmpty(nl->Rest(queryList))) {
          cout << "Rest of list is empty" << endl;
        }
        else {
          cout << nl->ToString(nl->First(nl->Rest(queryList))) << endl;
          if (eval) { // evaluate the condition
            string query = nl->ToString(nl->First(nl->Rest(queryList)));
            cout << "execute query '" << query << "'" << endl;
            if (!qp->ExecuteQuery(query, queryResult)) {
              cout << "execution error" << endl;
            }
            else {
              cout << "query successful processed" << endl;
              CcBool *ccResult = static_cast<CcBool*>(queryResult.addr);
              isTrue = ccResult->GetValue();
              ccResult->DeleteIfAllowed();
            }
          }
          else { // check the result type
            QueryProcessor* qpp = new QueryProcessor(nl,am);
            qpp->Construct(nl->First(nl->Rest(queryList)), correct, evaluable,
                          defined, isFunction, tree, resultType);
            if (!correct) {
              cout << "type error" << endl;
            }
            else if (!evaluable) {
              cout << "not evaluable" << endl;
            }
            else if (nl->ToString(resultType).compare("bool")) {
              cout << "wrong result type " << nl->ToString(resultType) << endl;
            }
            else {
              isBool = true;
            }
            if(tree){
               qpp->Destroy(tree,true);
            }
            delete qpp;

          }
        }
      }
      break;
    case 1:
      cout << "String cannot be converted to list" << endl;
      break;
    case 2:
      cout << "stack overflow" << endl;
      break;
    default: // should not occur
      break;
  }
  if (tree) {
    qp->Destroy(tree, true);
  }
  return eval ? isTrue : isBool;
}
