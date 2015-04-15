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

Started July 2014, Fabio Vald\'{e}s

*/
#include  "Tools.h"

namespace stj {
 
/*
\section{Implementation of class ~Tools~}

\subsection{Function ~intersect~}

*/
void Tools::intersect(const vector<set<TupleId> >& tidsets,
                      set<TupleId>& result) {
  result.clear();
  if (tidsets.empty()) {
    return;
  }
  vector<set<TupleId>::iterator> it;
  for (unsigned int i = 0; i < tidsets.size(); i++) { // initialize iterators
//     cout << "size of set " << i << " is " << tidsets[i].size() << endl;
    set<TupleId>::iterator iter = tidsets[i].begin();
    it.push_back(iter);
    if (iter == tidsets[i].end()) { // empty set
      return;
    }
  }
  while (true) {
    unsigned int min(UINT_MAX), max(0);
    for (unsigned int i = 0; i < tidsets.size(); i++) {
      if (*(it[i]) < min) {
        min = *(it[i]);
      }
      if (*(it[i]) > max) {
        max = *(it[i]);
      }
    }
    if (min == max) {
      result.insert(min);
      for (unsigned int i = 0; i < tidsets.size(); i++) {
        it[i]++;
        if (it[i] == tidsets[i].end()) {
          return;
        }
      }
    }
    else { // min < max
      for (unsigned int i = 0; i < tidsets.size(); i++) {
        while (*it[i] < max) {
          it[i]++;
          if (it[i] == tidsets[i].end()) {
            return;
          }
        }
      }
    }
  }
}

void Tools::intersectPairs(vector<set<pair<TupleId, int> > >& posVec, 
                           set<pair<TupleId, int> >*& result) {
  result->clear();
  if (posVec.empty()) {
    return;
  }
  if (posVec.size() == 1) {
    result = &(posVec[0]);
    return;
  }
  vector<set<pair<TupleId, int> >::iterator> it;
  for (unsigned int i = 0; i < posVec.size(); i++) { // initialize iterators
//     cout << "size of set " << i << " is " << posVec[i].size() << endl;
    set<pair<TupleId, int> >::iterator iter = posVec[i].begin();
    it.push_back(iter);
    if (iter == posVec[i].end()) { // empty set
      return;
    }
  }
  while (true) {
    pair<TupleId, int> min = make_pair(UINT_MAX, INT_MAX);
    pair<TupleId, int> max = make_pair(0, INT_MIN);
    for (unsigned int i = 0; i < posVec.size(); i++) {
      if (*(it[i]) < min) {
        min = *(it[i]);
      }
      if (*(it[i]) > max) {
        max = *(it[i]);
      }
    }
    if (min == max) {
      result->insert(min);
      for (unsigned int i = 0; i < posVec.size(); i++) {
        it[i]++;
        if (it[i] == posVec[i].end()) {
          return;
        }
      }
    }
    else { // min < max
      for (unsigned int i = 0; i < posVec.size(); i++) {
        while (*it[i] < max) {
          it[i]++;
          if (it[i] == posVec[i].end()) {
            return;
          }
        }
      }
    }
  }
}

void Tools::uniteLast(unsigned int size, vector<set<TupleId> >& tidsets) {
  if (tidsets.size() < size) {
    return;
  }
  for (unsigned int i = tidsets.size() - size + 1; i < tidsets.size(); i++) {
    tidsets[tidsets.size() - size].insert(tidsets[i].begin(), tidsets[i].end());
  }
  for (unsigned int i = 1; i < size; i++) {
    tidsets.pop_back();
  }
}

void Tools::uniteLastPairs(unsigned int size,
                           vector<set<pair<TupleId, int> > >& posVec) {
  if (posVec.size() < size) {
    return;
  }
  for (unsigned int i = posVec.size() - size + 1; i < posVec.size(); i++) {
    posVec[posVec.size() - size].insert(posVec[i].begin(), posVec[i].end());
  }
  for (unsigned int i = 1; i < size; i++) {
    posVec.pop_back();
  }
}

void Tools::filterPairs(set<pair<TupleId, int> >* pairs,
                   const set<TupleId>& pos, set<pair<TupleId, int> >*& result) {
  if (pos.empty() || pairs->empty()) {
    result = pairs;
    return;
  }
  set<pair<TupleId, int> >::iterator ip = pairs->begin();
  set<TupleId>::iterator it = pos.begin();
  set<pair<TupleId, int> >::iterator ir = result->begin();
  while ((ip != pairs->end()) && (it != pos.end())) {
    if (ip->first == *it) {
      ir = result->insert(ir, *ip);
      ip++;
      it++;
    }
    else if (ip->first < *it) {
      ip++;
    }
    else {
      it++;
    }
  }
  if (it == pos.end()) {
    while (ip != pairs->end()) {
      ir = result->insert(ir, *ip);
      ip++;
    }
  }
}

string Tools::int2String(int i) {
  stringstream result;
  result << i;
  return result.str();
}

int Tools::str2Int(string const &text) {
  int result;
  stringstream ss(text);
  if((ss >> result).fail())
    result = 0;
  return result;
}

void Tools::deleteSpaces(string &text) {
  size_t pos = 0;
  while ((pos = text.find(' ', pos)) != string::npos)
    text.erase(pos, 1);
}

/*
\subsection{Function ~convert~}

Converts a string into a char[*].

*/
char* Tools::convert(string arg) {
  return strdup(arg.c_str());
}

/*
\subsection{Function ~eraseQM~}

Deletes enclosing quotation marks from a string.

*/
void Tools::eraseQM(string& arg) {
  if (arg.at(0) == '"') {
    arg = arg.substr(1, arg.length() - 2);
  }
}

void Tools::addQM(string& arg) {
  if (arg.at(0) != '"') {
    arg.insert(0, "\"");
    arg.append("\"");
  }
}

void Tools::simplifyRegEx(string &regEx) {
  for (unsigned int i = 0; i < regEx.length(); i++) {
    switch (regEx[i]) {
      case '*': {
        regEx[i] = '?';
        break;
      }
      case '+': {
        regEx.erase(i, 1);
        break;
      }
      default: {
        break;
      }
    }
  }
}

/*
function ~setToString~

*/
string Tools::setToString(const set<string>& input) {
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

/*
function ~createSetMatrix~

Creates and returns a twodimensional array that is used to store the matching
positions.

*/

set<unsigned int>** Tools::createSetMatrix(unsigned int dim1,unsigned int dim2){
  set<unsigned int>** result = new set<unsigned int>*[dim1];
  for (unsigned int i = 0; i < dim1; i++) {
    result[i] = new set<unsigned int>[dim2];
  }
  return result;
};

/*
function ~deleteSetMatrix~

Deletes a twodimensional array.

*/

void Tools::deleteSetMatrix(set<unsigned int>** &victim, unsigned int dim1) {
  if (victim) {
    for (unsigned int i = 0; i < dim1; i++) {
      if (victim[i]) {
        delete[] victim[i];
      }
    }
    delete[] victim;
  }
}

/*
function ~prefixCount~

Returns the number of strings from which ~str~ is a prefix. This is needed in
MLabel::buildIndex.

*/
int Tools::prefixCount(string str, set<string> strings) {
  set<string>::iterator it;
  int result = 0;
  for (it = strings.begin(); it != strings.end(); it++) {
    if ((it->substr(0, str.length()) == str) && (*it != str)) {
      result++;
    }
  }
  return result;
}

void Tools::splitPattern(string& input, vector<string>& result) {
  result.clear();
  if (input.empty()) {
    return;
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
  else if (pos == string::npos) { // no curly brackets
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
}

/*
function ~extractVar~

Takes an assignment string like ~time:=Z.time~ and returns the variable string.

*/
string Tools::extractVar(const string& input) {
  int posEq = input.find('=');
  int posDot = input.find('.');
  return input.substr(posEq + 1, posDot - posEq - 1);
}

int Tools::getKey(const string& type) {
  if (type == "label")       return 0;
  if (type == "place")       return 1;
  if (type == "time")        return 2;
  if (type == "start")       return 3;
  if (type == "end")         return 4;
  if (type == "leftclosed")  return 5;
  if (type == "rightclosed") return 6;
  if (type == "card")        return 7;
  if (type == "labels")      return 8;
  if (type == "places")      return 9;
  else return -1; // should not occur
}

string Tools::getDataType(const int key) {
  switch (key) {
    case -1: return CcBool::BasicType();
    case 0: return "label";
    case 1: return "place";
    case 2: return SecInterval::BasicType();
    case 3: 
    case 4: return Instant::BasicType();
    case 5:
    case 6: return CcBool::BasicType();
    case 8: return "labels";
    case 9: return "places";
    default: return "error";
  }
}

DataType Tools::getDataType(const string& type) {
  if (type == "mlabel") return MLABEL;
  if (type == "mlabels") return MLABELS;
  if (type == "mplace") return MPLACE;
  return MPLACES;
}

bool Tools::isSymbolicType(ListExpr type) {
  return ((nl->ToString(type) == "mlabel") || (nl->ToString(type) == "mlabels")
     || (nl->ToString(type) == "mplace") || (nl->ToString(type) ==  "mplaces"));
}

/*
function ~extendDate~

Takes a datetime string and extends it to the format YYYY-MM-DD-HH:MM:SS.MMM,
the variable ~start~ decides on the values.

*/
string Tools::extendDate(string input, const bool start) {
  string result, mask;
  int month, daysInMonth, year;
  if (start) {
    mask.assign("-01-01-00:00:00");
  }
  else {
    mask.assign("-12-31-23:59:59.999");
  }
  if (input[input.size() - 1] == '-') { // handle case 2011-04-02-
    input.resize(input.size() - 1);
  }
  result.assign(input);
  size_t pos = 1;
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
  pos = input.find('-') + 1;
  if ((pos = input.find('-', pos)) == string::npos) {
    result.append(mask.substr(3));
    return result;
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
bool Tools::checkSemanticDate(const string &text, const SecInterval &uIv,
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
    const int errPos = 0;
    ListExpr errInfo;
    bool ok;
    if (Periods::checkType(sc->GetObjectTypeExpr(text)) ||
        SecInterval::checkType(sc->GetObjectTypeExpr(text))) {
      Word pWord = sc->InObject(sc->GetObjectTypeExpr(text),
                                sc->GetObjectValue(text), errPos, errInfo, ok);
      if (!ok) {
        cout << "Error: InObject failed." << endl;
        return false;
      }
      else {
        if (Periods::checkType(sc->GetObjectTypeExpr(text))) {
          Periods* period = static_cast<Periods*>(pWord.addr);
          return (eval ? period->Contains(uIv) : true);
        }
        else {
          SecInterval* interval = static_cast<SecInterval*>(pWord.addr);
          return (eval ? interval->Contains(uIv) : true);
        }
      }
    }
    cout << text << " is not a periods / interval object." << endl;
    return false;
  }
  return false;
}

/*
function ~checkDaytime~

Checks whether the daytime interval resulting from text (e.g., 0:00[~]8:00)
contains the one from the unit label.

*/
bool Tools::checkDaytime(const string& text, const SecInterval& uIv) {
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
    Instant pStart(instanttype);
    Instant pEnd(instanttype);
    if (!pStart.ReadFrom(extendDate(startString, true))) {
      cout << "error: ReadFrom " << extendDate(startString, true) << endl;
      return false;
    }
    if (!pEnd.ReadFrom(extendDate(endString, false))) {
      cout << "error: ReadFrom " << extendDate(endString, false) << endl;
      return false;
    }
    SecInterval pInterval(pStart, pEnd, true, true);
    return pInterval.Contains(uIv);
  } // no match possible in case of different days
  return false;
}

/*
\subsection{Function ~isInterval~}

*/
bool Tools::isInterval(const string& str) {
  if ((str[0] > 96) && (str[0] < 123)) { // 1st case: semantic date/time
    return false;
  }
  if ((str.find('-') == string::npos) // 2nd case: 19:09~22:00
          && ((str.find(':') < str.find('~')) // on each side of [~],
              || (str[0] == '~')) // there has to be either xx:yy or nothing
          && ((str.find(':', str.find('~')) != string::npos)
              || str[str.size() - 1] == '~')) {
    return false;
  }
  return true;
}

/*
\subsection{Function ~stringToInterval~}

*/
void Tools::stringToInterval(const string& str, SecInterval& result) {
  Instant pStart(instanttype);
  Instant pEnd(instanttype);
  if (str[0] == '~') { // 3rd case: ~2012-05-12
    pStart.ToMinimum();
    pEnd.ReadFrom(extendDate(str.substr(1), false));
  }
  else if (str[str.size() - 1] == '~') { // 4th case: 2011-04-02-19:09~
    pStart.ReadFrom(extendDate(str.substr(0, str.size() - 1), true));
    pEnd.ToMaximum();
  }
  else if (str.find('~') == string::npos) { // 5th case: no [~] found
    pStart.ReadFrom(extendDate(str, true));
    pEnd.ReadFrom(extendDate(str, false));
  }
  else { // sixth case: 2012-05-12-20:00~2012-05-12-22:00
    pStart.ReadFrom(extendDate(str.substr(0, str.find('~')), true));
    pEnd.ReadFrom(extendDate(str.substr(str.find('~') + 1), false));
  }
  result.Set(pStart, pEnd, true, true);
}

/*
\subsection{Function ~parseInterval~}

*/
bool Tools::parseInterval(const string& input, int &pos, int &endpos,
                          pair<Word, ValueType> &valuepair) {
  endpos = input.find(' ', pos);
  stringstream leftss(input.substr(pos + 1, endpos));
  double left = 0.0;
  leftss >> left;
  pos = input.find_first_not_of(' ', endpos);
  endpos = input.find(' ', pos);
  stringstream rightss(input.substr(pos, endpos - 1));
  double right = 0.0;
  rightss >> right;
  if (right < left) {
    cout << "invalid interval" << endl;
    return false;
  }
  endpos = input.find(' ', pos);
  pos = input.find_first_not_of(' ', endpos);
  if (input[pos] != 't' && input[pos] != 'f') {
    cout << "\"t\" or \"f\" expected for interval, instead of \"" 
        << input[pos] << "\"" << endl;
    return false;
  }
  bool lc = (input[pos] == 't');
  pos = input.find_first_not_of(' ', pos + 1);
  bool rc = (input[pos] == 't');
  pos = input.find('>', pos) + 1;
  valuepair.first.addr = new Interval<double>(left, right, lc, rc);
  valuepair.second = INTERVAL;
  cout << "interval " << (lc ? "[" : "(") << left << ", " << right
      << (rc ? "]" : ")") << " inserted" << endl;
  endpos = pos - 1;
  pos = input.find_first_not_of(' ', pos);
  return true;
}

/*
\subsection{Function ~parseBoolorObj~}

*/
bool Tools::parseBoolorObj(const string& input, int &pos, int &endpos,
                           pair<Word, ValueType> &valuepair) {
  if (!stringutils::isLetter(input[pos])) {
    return false;
  }
  endpos = input.find_first_of(" ,)}", pos) - 1;
  if (input.substr(pos, 4) == "TRUE") {
    valuepair.first.addr = new CcBool(true, true);
    valuepair.second = BOOL;
  }
  else if (input.substr(pos, 5) == "FALSE") {
    valuepair.first.addr = new CcBool(true, false);
    valuepair.second = BOOL;
  }
  else {
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    string name = input.substr(pos, endpos - pos + 1);
    if (!sc->IsObjectName(name)) {
      cout << name << " is not an object name" << endl;
      return false;
    }
    string type;
    Word value;
    bool defined;
    if (!sc->GetObject(name, value, defined)) {
      cout << "object " << name << " could not be read" << endl;
      return false;
    }
    type = nl->ToString(sc->GetObjectTypeExpr(name));
    if (type == "point") {
      valuepair.second = POINT;
    }
    else if (type == "points") {
      valuepair.second = POINTS;
    }
    else if (type == "line") {
      valuepair.second = LINE;
    }
    else if (type == "rect") {
      valuepair.second = RECT;
    }
    else if (type == "region") {
      valuepair.second = REGION;
    }
    else if (type == "label") {
      valuepair.second = LABEL;
    }
    else if (type == "labels") {
      valuepair.second = LABELS;
    }
    else if (type == "place") {
      valuepair.second = PLACE;
    }
    else if (type == "places") {
      valuepair.second = PLACES;
    }
    else if (type == "interval") {
      valuepair.second = INTERVAL;
    }
    else if (type == "bool") {
      valuepair.second = BOOL;
    }
    else {
      cout << "type " << type << " is invalid" << endl;
      return false;
    }
    valuepair.first.addr = value.addr;
    cout << "object " << name << " has type " << type << endl;
  }
  pos = input.find_first_not_of(' ', endpos + 1);
  return true;
}

/*
\subsection{Function ~timesMatch~}

Checks whether the time interval ~uIv~ is completely enclosed by each of the
intervals specified in ~ivs~. If ~ivs~ is empty, the result is ~true~.

*/
bool Tools::timesMatch(const Interval<DateTime>& iv, const set<string>& ivs) {
  if (ivs.empty()) {
    return true;
  }
  bool elementOk(false);
  SecInterval pIv(true);
  for (set<string>::iterator j = ivs.begin(); j != ivs.end(); j++) {
    if (((*j)[0] > 96) && ((*j)[0] < 123)) { // 1st case: semantic date/time
      elementOk = checkSemanticDate(*j, iv, true);
    }
    else if (((*j).find('-') == string::npos) // 2nd case: 19:09~22:00
          && (((*j).find(':') < (*j).find('~')) // on each side of [~],
              || ((*j)[0] == '~')) // there has to be either xx:yy or nothing
          && (((*j).find(':', (*j).find('~')) != string::npos)
              || (*j)[(*j).size() - 1] == '~')) {
      elementOk = checkDaytime(*j, iv);
    }
    else {
      stringToInterval(*j, pIv);
      elementOk = pIv.Contains(iv);
    }
    if (!elementOk) { // all intervals have to match
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~processQueryStr~}

Invoked by ~initOpTrees~

*/
pair<QueryProcessor*, OpTree> Tools::processQueryStr(string query, int type) {
  pair<QueryProcessor*, OpTree> result;
  result.first = 0;
  result.second = 0;
  SecParser parser;
  string qParsed;
  ListExpr qList, rType;
  bool correct(false), evaluable(false), defined(false), isFunction(false);
  if (parser.Text2List(query, qParsed)) {
    cout << "Text2List(" << query << ") failed" << endl;
    return result;
  }
  if (!nl->ReadFromString(qParsed, qList)) {
    cout << "ReadFromString(" << qParsed << ") failed" << endl;
    return result;
  }
  result.first = new QueryProcessor(nl, am);
  try {
    result.first->Construct(nl->Second(qList), correct, evaluable, defined,
                            isFunction, result.second, rType);
  }
  catch (...) {
    delete result.first;
    result.first = 0;
    result.second = 0;
  }
  if (!correct || !evaluable || !defined) {
    cout << "correct:   " << (correct ? "TRUE" : "FALSE") << endl
         << "evaluable: " << (evaluable ? "TRUE" : "FALSE") << endl
         << "defined:   " << (correct ? "TRUE" : "FALSE") << endl;
    delete result.first;
    result.first = 0;
    result.second = 0;
    return result;
  }
  if (nl->ToString(rType) != Tools::getDataType(type)) {
    cout << "incorrect result type: " << nl->ToString(rType) << endl;
    delete result.first;
    result.first = 0;
    result.second = 0;
  }
  return result;
}

/*
\subsection{Function ~evaluate~}

The string ~input~ is evaluated by Secondo. The result is returned as a Word.

*/
// Word evaluate(string input) {
//   SecParser qParser;
//   string query, queryStr;
//   ListExpr queryList;
//   Word queryResult;
//   input.insert(0, "query ");
//   if (!qParser.Text2List(input, queryStr)) {
//     if (nl->ReadFromString(queryStr, queryList)) {
//       if (!nl->IsEmpty(nl->Rest(queryList))) {
//         query = nl->ToString(nl->First(nl->Rest(queryList)));
//         if (qp->ExecuteQuery(query, queryResult)) {
//           return queryResult;
//         }
//       }
//     }
//   }
//   return queryResult;
// }



bool Tools::createTransitions(const bool dortmund,
                              map<string, set<string> >& transitions) {
  if (dortmund) {
    map<string, set<string> >::iterator im;
    set<string>::iterator it;
    transitions["Aplerbeck"].insert("Hörde");
    transitions["Aplerbeck"].insert("Innenstadt-Ost");
    transitions["Aplerbeck"].insert("Brackel");
    transitions["Brackel"].insert("Innenstadt-Nord");
    transitions["Brackel"].insert("Innenstadt-Ost");
    transitions["Brackel"].insert("Scharnhorst");
    transitions["Eving"].insert("Huckarde");
    transitions["Eving"].insert("Innenstadt-Nord");
    transitions["Eving"].insert("Mengede");
    transitions["Eving"].insert("Scharnhorst");
    transitions["Hörde"].insert("Hombruch");
    transitions["Hörde"].insert("Innenstadt-Ost");
    transitions["Hombruch"].insert("Innenstadt-Ost");
    transitions["Hombruch"].insert("Innenstadt-West");
    transitions["Hombruch"].insert("Lütgendortmund");
    transitions["Huckarde"].insert("Innenstadt-Nord");
    transitions["Huckarde"].insert("Innenstadt-West");
    transitions["Huckarde"].insert("Lütgendortmund");
    transitions["Huckarde"].insert("Mengede");
    transitions["Innenstadt-Nord"].insert("Innenstadt-Ost");
    transitions["Innenstadt-Nord"].insert("Innenstadt-West");
    transitions["Innenstadt-Nord"].insert("Scharnhorst");
    transitions["Innenstadt-Ost"].insert("Innenstadt-West");
    transitions["Innenstadt-West"].insert("Lütgendortmund");
    for (im = transitions.begin(); im != transitions.end(); im++) {
      for (it = im->second.begin(); it != im->second.end(); it++) {
        transitions[*it].insert(im->first); // add symmetric transitions
      }
      transitions[im->first].insert(im->first); // remaining in the same area
    }
  }
  else {
    Word kreis, kreisrtree;
    bool defined;
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    if (!sc->GetObject("Kreis", kreis, defined)) {
      cout << "Error: GetObject Kreis failed" << endl;
      return false;
    }
    if (!defined) {
      cout << "Error: object Kreis undefined" << endl;
      return false;
    }
    if (!sc->GetObject("Kreisrtree", kreisrtree, defined)) {
      cout << "Error: GetObject Kreisrtree failed" << endl;
      return false;
    }
    if (!defined) {
      cout << "Error: object Kreisrtree undefined" << endl;
      return false;
    }
    Relation *rel = (Relation*)(kreis.addr);
    R_Tree<2, TupleId> *rtree = (R_Tree<2, TupleId>*)(kreisrtree.addr);
    RelationIterator *it1 = new RelationIterator(*rel);
    Tuple *tuple1 = it1->GetNextTuple();
    R_TreeLeafEntry<2, TupleId> leaf;
    Tuple *tuple2 = 0;
    while (tuple1) {
      string name1 = ((CcString*)(tuple1->GetAttribute(0)))->GetValue();
      Region *r1 = (Region*)(tuple1->GetAttribute(4));
      Rectangle<2> bbox = r1->BoundingBox();
      if (rtree->First(bbox, leaf)) {
        tuple2 = rel->GetTuple(leaf.info, true);
        string name2 = ((CcString*)(tuple2->GetAttribute(0)))->GetValue();
        transitions[name1.substr(3)].insert(name2.substr(3));
        tuple2->DeleteIfAllowed();
        while (rtree->Next(leaf)) {
          tuple2 = rel->GetTuple(leaf.info, true);
          string name2 = ((CcString*)(tuple2->GetAttribute(0)))->GetValue();
          transitions[name1.substr(3)].insert(name2.substr(3));
          tuple2->DeleteIfAllowed();
        }
      }
      tuple1->DeleteIfAllowed();
      tuple1 = it1->GetNextTuple();
    }
    delete it1;
    delete rtree;
    delete rel;
  }
  return true;
}

/*
\subsection{Function ~createLabelSequence~}

Creates a vector of string containing either districts of Dortmund or German
counties in a random but sensible order.

*/
bool Tools::createLabelSequence(const int size, const int number,
                    const bool dortmund, map<string, set<string> >& transitions,
                    vector<string>& result) {
  time_t t;
  time(&t);
  srand((unsigned int)t);
  map<string, set<string> >::iterator im;
  set<string>::iterator it;
  for (int i = 0; i < number; i++) {
    im = transitions.begin();
    int choice = rand() % transitions.size();
    advance(im, choice);
    string name = im->first;
    result.push_back(name);
    for (int j = 1; j < size; j++) {
      it = transitions[name].begin();
      choice = rand() % transitions[name].size();
      advance(it, choice);
      name = *it;
      result.push_back(name);
    }
  }
  return result.size() == (unsigned int)(size * number);
}

void Tools::printNfa(vector<map<int, int> > &nfa, set<int> &finalStates) {
  map<int, int>::iterator it;
  for (unsigned int i = 0; i < nfa.size(); i++) {
    cout << (finalStates.count(i) ? " * " : "   ") << "state " << i << ":  ";
    for (it = nfa[i].begin(); it != nfa[i].end(); it++) {
      cout << "---" << it->first << "---> " << it->second << "    ";
    }
    cout << endl << endl;
  }
}

void Tools::makeNFApersistent(vector<map<int, int> > &nfa,set<int> &finalStates,
   DbArray<NFAtransition> &trans, DbArray<int> &s2p, map<int, int> &state2Pat) {
  NFAtransition tr;
  map<int, int>::iterator im;
  for (unsigned int i = 0; i < nfa.size(); i++) {
    tr.oldState = i;
    for (im = nfa[i].begin(); im != nfa[i].end(); im++) {
      tr.trigger = im->first;
      tr.newState = im->second;
      trans.Append(tr);
    }
    s2p.Append(state2Pat[i]);
  }
}

void Tools::createNFAfromPersistent(DbArray<NFAtransition> &trans, 
        DbArray<int> &s2p, vector<map<int, int> > &nfa, set<int> &finalStates) {
  nfa.clear();
  finalStates.clear();
  map<int, int> emptymap;
  NFAtransition tr;
  for (int i = 0; i < trans.Size(); i++) {
    trans.Get(i, tr);
    while (tr.oldState >= (int)nfa.size()) {
      nfa.push_back(emptymap);
    }
    nfa[tr.oldState][tr.trigger] = tr.newState;
  }
  int pat = INT_MAX;
  for (int i = 1; i < s2p.Size(); i++) {
    s2p.Get(i, pat);
    if ((pat != INT_MAX) && (pat >= 0)) {
      finalStates.insert(i);
    }
  }
}

void Tools::printBinding(const map<string, pair<int, int> > &b) {
  if (b.empty()) {
    return;
  }
  map<string, pair<int, int> >::const_iterator it;
  for (it = b.begin(); it != b.end(); it++) {
    cout << it->first << " --> [" << it->second.first << ","
         << it->second.second << "]  ";
  }
  cout << endl;
}

double Tools::distance(const string& str1, const string& str2, const int fun) {
  if (!str1.length() && !str2.length()) {
    return 0;
  }
  double ld;
  if (fun > 1) {
    string newstr1, newstr2;
    transform(str1.begin(), str1.end(), newstr1.begin(), ::tolower);
    transform(str2.begin(), str2.end(), newstr2.begin(), ::tolower);
    ld = stringutils::ld(newstr1, newstr2);
  }
  ld = stringutils::ld(str1, str2);
  return (fun % 2 == 0) ? ld / max(str1.length(), str2.length()) : ld;
}

double Tools::distance(const pair<string, unsigned int>& val1, 
    const pair<string, unsigned int>& val2, const int labelFun) {
  double ld = Tools::distance(val1.first, val2.first, labelFun);
  return (labelFun > 4 && val1.second == val2.second) ? ld / 2 : ld;
}

double Tools::distance(const set<string>& values1, const set<string>& values2,
                       const int fun, const int labelFun) {
  if (values1.empty() && values2.empty()) {
    return 0;
  }
  set<string>::iterator i1, i2;
  multiset<double> dist;
  if (values2.empty()) {
    for (i1 = values1.begin(); i1 != values1.end(); i1++) {
      dist.insert(labelFun % 2 == 0 ? 1 : i1->length());
    }
  }
  else if (values1.empty()) {
    for (i2 = values2.begin(); i2 != values2.end(); i2++) {
      dist.insert(labelFun % 2 == 0 ? 1 : i2->length());
    }
  }
  else {
    for (i1 = values1.begin(); i1 != values1.end(); i1++) {
      for (i2 = values2.begin(); i2 != values2.end(); i2++) {
        dist.insert(Tools::distance(*i1, *i2, labelFun));
      }
    }
  }
  int m = values1.size();
  int n = values2.size();
  int limit = (fun % 5) % 2 == 1 ? min(m, n) : 
              ((fun % 5) > 0 ? max(m, n) : m + n);
  multiset<double>::iterator it = dist.begin();
  double sum = 0;
  for (int k = 0; k < limit; k++) {
    sum += *it;
    it++;
  }
  return fun < 5 ? sum / limit : sum;
}

double Tools::distance(set<pair<string, unsigned int> >& values1, 
 set<pair<string, unsigned int> >& values2, const int fun, const int labelFun) {
  if (values1.empty() && values2.empty()) {
    return 0;
  }
  if (values1.empty() || values2.empty()) {
    return 1;
  }
  set<pair<string, unsigned int> >::iterator i1, i2;
  multiset<double> dist;
  for (i1 = values1.begin(); i1 != values1.end(); i1++) {
    for (i2 = values2.begin(); i2 != values2.end(); i2++) {
      double labelDist = double(stringutils::ld(i1->first, i2->first)) /
                         max(i1->first.length(), i2->first.length());
      dist.insert(labelDist + (i1->second == i2->second ? 0 : 0.5));
    }
  }
  int limit = min(values1.size(), values2.size());
  multiset<double>::iterator it = dist.begin();
  double sum = 0;
  for (int k = 0; k < limit; k++) {
    sum += *it;
    it++;
  }
  return (sum / limit + abs((int64_t)values1.size() - (int64_t)values2.size())/ 
                        max(values1.size(), values2.size())) / 2;
}

}
