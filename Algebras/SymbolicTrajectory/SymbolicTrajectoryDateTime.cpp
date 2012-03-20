/*
Secondo

*/

#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include "CharTransform.h"
#include "SymbolicTrajectoryTools.h"
#include "SymbolicTrajectoryDateTime.h"
 
using namespace std;


class SymbolicTimeRange
{
public:
  SymbolicTimeRange();

  int getWeekdayNr(string weekday) {
    return weekdays_map[weekday];
  };
  
  string getWeekday(int weekday) {
    return (weekday < 1 || weekday > countWeekdays()) ? ""
           : weekdays_vector[weekday - 1];
  };
  
  int countWeekdays() {
    return weekdays_map.size();
  };

  int getDaytimeNr(string daytime) {
    return daytimes_map[daytime];
  };

  string getDaytime(int daytime) {
    return (daytime < 1 || daytime > countDaytimes()) ? ""
           : daytimes_vector[daytime - 1];
  };
  
  int countDaytimes() {
    return daytimes_map.size();
  };

  int getMonthNr(string month) {
    return months_map[month];
  };
  
  string getMonth(int month) {
    return (month < 1 || month > countMonths()) ? "" : months_vector[month - 1];
  };
  
  int countMonths() {
    return months_map.size();
  };
   
  int getDaysOfMonth(int month, int year);
  string getDaysOfMonth(string month, string year);
   
private:
  vector<string> weekdays_vector;
  map<string, int> weekdays_map;

  vector<string> daytimes_vector;
  map<string, int> daytimes_map;

  vector<string> months_vector;
  map<string, int> months_map;
};

SymbolicTimeRange::SymbolicTimeRange() {
  string weekdays[] = {"monday","tuesday","wednesday","thursday",
                       "friday","saturday","sunday"}; 
  weekdays_vector.clear();
  weekdays_map.clear();
  for (int i = 0; i < 7; ++i) {
    weekdays_map[weekdays[i]] = i + 1;
    weekdays_vector.push_back(weekdays[i]);
  }
  string daytimes[] = {"morning","afternoon","evening","night"};
  daytimes_vector.clear();
  daytimes_map.clear();
  for (int i = 0; i < 4; ++i) {
    daytimes_map[daytimes[i]] = i + 1;
    daytimes_vector.push_back(daytimes[i]);
  }
  string months[] = {"january","february","march","april","may","june","july",
                     "august","september","october","november","december"};
  months_vector.clear();
  months_map.clear();
  for (int i = 0; i < 12; ++i) {
    months_map[months[i]] = i + 1;
    months_vector.push_back(months[i]);
  }   
}


int SymbolicTimeRange::getDaysOfMonth(int month, int year) {
  int days = 0;
  switch (month) {
    case 1 :
    case 3 :
    case 5 :
    case 7 :
    case 8 :
    case 10 :
    case 12 :
      days = 31;
      break;
    case 4 :
    case 6 :
    case 9 :
    case 11 :
      days = 30;
      break;
    case 2 :
      days = ((year%4 == 0 && year%100 != 0) || (year%400 == 0)) ? 29 : 28;
      break;
  }
  return days;
}


string SymbolicTimeRange::getDaysOfMonth(string month, string year) {
  string days; 
  days = int2Str(getDaysOfMonth(str2Int(month), str2Int(year)));
  return days;
}


string getFullDate(string const &text, bool const startDate) {
  string dateDefault = (startDate) ? "0001-01-01" : "9999-12-31";
  string fullDate = "";
  SymbolicTimeRange setira;
  if (!startDate && text.length() == 7)
    fullDate = text + "-"
             + setira.getDaysOfMonth(text.substr(5,2), text.substr(0, 4));
  else
    fullDate = text
             + dateDefault.substr(0, dateDefault.length() - text.length());
  return fullDate;
}


string getFullTime(string const &text, bool const startTime)
{
   string timeDefault = (startTime) ? "00:00:00.000" : "23:59:59.999";
   
   return text + timeDefault.substr(text.length());
} 


string getFullDateTime(string const &text, bool const startDateTime) {
  vector<string> elements = split(text, '#');
  string fullDateTime = "";
  if (elements.empty())
    fullDateTime = getFullDate("", startDateTime) + "#"
                 + getFullTime("", startDateTime);
  else if (elements.size() == 1)
    fullDateTime = getFullDate(elements[0], startDateTime) + "#"
                 + getFullTime("", startDateTime);
  else if (elements.size() == 2)
    fullDateTime = getFullDate(elements[0], startDateTime) + "#"
                 + getFullTime(elements[1], startDateTime);
  return fullDateTime;
} 


string getFullRange(string const &text) {
  string resultRange = "";
  vector<string> dates = split(text, '/');
  if (dates.size() == 1)
    resultRange = getFullDateTime(dates[0], true) + "/"
                  + getFullDateTime(dates[0], false);
  else if (dates.size() == 2) {
    resultRange = getFullDateTime(dates[0], true) + "/";
    resultRange += (dates[1].size() == 10) ? getFullDateTime(dates[1], false)
                                           : getFullDateTime(dates[1], true);
  }
  return resultRange;
}


string getFullTimeRange(string const &text) {
  string resultRange = "";
  vector<string> times = split(text, '/');
  if (times.size() == 1)
    resultRange = getFullTime(times[0], true) + "/" + getFullTime("", false);
  else if (times.size() == 2)
    resultRange = getFullTime(times[0], true) + "/"
                + getFullTime(times[1], true);
  return resultRange;    
}


bool checkMaskOfDate(string const &mask) {
  string acceptedDateMask = "0000-00-00";
  if ((mask.length() != 4) && (mask.length() != 7) && (mask.length() != 10))
    return false;
  if (mask == acceptedDateMask.substr(0, mask.length()))
    return true;
  return false;
}


bool checkMaskOfTime(string const &mask) {
  string acceptedTimeMask = "00:00:00";
  if ((mask.length() != 2) && (mask.length() != 5) && (mask.length() != 8))
    return false;
  if (mask == acceptedTimeMask.substr(0, mask.length()))
    return true;
  return false;
}


bool checkMaskOfDateTime(string const &mask) 
{
  vector<string> elements = split(mask, '#');  
  switch (elements.size()) {
    case 2:
      if (elements[0].length() != 10 || !checkMaskOfTime(elements[1]))
        return false;
    case 1:
      if (checkMaskOfDate(elements[0]))
        return true;
  }  
  return false;  
}


bool checkRangeMask(string const &mask) {
  // acceptedDefaultMask = "0000-00-00#00:00:00/0000-00-00#00:00:00";
  vector<string> masks = split(mask, '/');
  switch (masks.size()) {
    case 2: // end element
      if ((!masks[1].empty() or !masks[0].empty())
          && (masks[1].empty() || checkMaskOfDateTime(masks[1]))
          && (masks[0].empty() || checkMaskOfDateTime(masks[0])))
        return true;
      break;
    case 1: // start/standalone element
      if (checkMaskOfDateTime(masks[0]))
        return true;
      break;
  }
  return false;
}
 

bool checkTimeRangeMask(string const &mask) {
  // acceptedDefaultMask = "00:00:00/00:00:00";
  vector<string> masks = split(mask, '/');
  switch (masks.size()) {
    case 2: // end element
      if ((!masks[1].empty() or !masks[0].empty())
          && (masks[1].empty() || checkMaskOfTime(masks[1]))
          && (masks[0].empty() || checkMaskOfTime(masks[0])))
        return true;
        break;
    case 1: // start/standalone element
      if (checkMaskOfTime(masks[0]))
        return true;
      break;
   }
   return false;
} 


int getType(string text, string &datetime)
{
   // 0 = cannot resolve element
   // 1 = date-time range
   // 2 = time range (of day)
   // 3 = weekday
   // 4 = day-time  
   // 5 = month
   // 6 = year
      
  string mask = getMask(text);
  SymbolicTimeRange setira;
  int type = 0; // cannot resolve element
  if (mask.empty())
    return type; // = 0, cannot resolve element
  if (mask == "AX") {
    int nr; 
    if ( (nr = setira.getWeekdayNr(text)) != 0) {
      type = 3;
      datetime = int2Str(nr);
    }
    else if ((nr = setira.getDaytimeNr(text)) != 0) {
      type = 4;
      datetime = int2Str(nr);
    }
    else if ((nr = setira.getMonthNr(text)) != 0) {
      type = 5;
      datetime = int2Str(nr);
    }
  }
  else if (isNumeric(text)) {
    type = 6;
    datetime = getFullRange(text);
  }
  else if (checkRangeMask(mask)) {
    type = 1;
    datetime = getFullRange(text);
  }
  else if (checkTimeRangeMask(mask)) {
    type = 2;
    datetime = getFullTimeRange(text);
  }
  return type;
}


string getMask(string const &text) {
  string mask = "";
  char char_cur;
  bool onlyAlphaChars = true;
  bool onlyNumericChars = true;
  for (size_t i = 0; i < text.length(); ++i) {
    char_cur = text[i];
    if  ((char_cur >= 65 && char_cur <= 90) ||
         (char_cur >= 97 && char_cur <= 122)) {
      mask += 'A';
      onlyNumericChars = false;
    }
    else if ((char_cur >= 48 && char_cur <= 57)) {
      mask += '0';
      onlyAlphaChars = false;
    } 
    else if ((char_cur == '.' || char_cur == '#' || char_cur == ':' ||
               char_cur == '/')) {
      mask += char_cur;
      onlyAlphaChars = false;
      onlyNumericChars = false;
    }
    else if  (char_cur == '-') {
      while (!mask.empty() && (mask[mask.length() - 1] == ' '))
        mask.erase(mask.length() - 1);
      mask += char_cur;
      onlyAlphaChars = false;
      onlyNumericChars = false; 
    }
    else if ((char_cur == ' ')) {
      if (mask.empty() || (mask[mask.length() - 1] != '-'))
        mask += char_cur;
      else
        // mask += '?';
        // onlyAlphaChars = false;
        // onlyNumericChars = false;
        return "?";
    }
  }
  if (onlyAlphaChars && !mask.empty())
    mask = "AX";
  if (onlyNumericChars && !mask.empty())
    mask = "0X";
  return mask;
}


bool isDate(string const &text) 
{
  return checkMaskOfDateTime(getMask(text));
}


bool isNumeric(string const &text) 
{
  return (getMask(text)  == "0X");
}


/*
is true if reftime is before time

*/
bool isBeforeTime(string const &time, string const &reftime) { 
  assert((time.size() == 12) && (reftime.size() == 12));
  if (getHourFromTime(reftime) < getHourFromTime(time))
    return true;
  else if (getHourFromTime(reftime) > getHourFromTime(time))
    return false;
  else { // equal hours
    if (getMinuteFromTime(reftime) < getMinuteFromTime(time))
      return true;
    else if (getMinuteFromTime(reftime) > getMinuteFromTime(time))
      return false;
    else { // equal minutes
      if (getSecondFromTime(reftime) < getSecondFromTime(time))
        return true;
      else if (getSecondFromTime(reftime) > getSecondFromTime(time))
        return false;
      else { // equal seconds
        if (getMillisecondFromTime(reftime) < getMillisecondFromTime(time))
          return true;
        else // equal or later
          return false;
      }
    }
  }
}


string getStartTimeFromRange(string const &time) { 
  assert(time.size() == 25);
  return time.substr(0, 12);
}


string getEndTimeFromRange(string const &time) { 
  assert(time.size() == 25);
  return time.substr(13, 12);
}


bool isPositivTimeRange(string const &range) {
  assert(range.size() == 25);
  return isBeforeTime(getEndTimeFromRange(range), getStartTimeFromRange(range));
}


int getHourFromTime(string const &time) { 
  assert(time.size() == 12);
  return str2Int(time.substr(0, 2));
} 


int getMinuteFromTime(string const &time) { 
  assert(time.size() == 12);
  return str2Int(time.substr(3, 2));
} 


int getSecondFromTime(string const &time) { 
  assert(time.size() == 12);
  return str2Int(time.substr(6, 2));
} 


int getMillisecondFromTime(string const &time) { 
  assert(time.size() == 12);
  return str2Int(time.substr(9, 3));
}


string getStartDateTimeFromRange(string const &date) {
  assert(date.size() == 47);
  return date.substr(0, 23);
} 


string getEndDateTimeFromRange(string const &date) { 
  assert(date.size() == 47);
  return date.substr(24, 23); 
}


string getSecDateTimeString(string const &date) {
  assert(date.size() == 23);
  string result = "";
  result += date.substr(0,4) + "-"; // year
  result += date.substr(5,2) + "-"; // month
  result += date.substr(8,2) + "-"; // day
  result += date.substr(11,2) + ":"; // hour
  result += date.substr(14,2) + ":"; // minute
  result += date.substr(17,2) + "."; // second
  result += date.substr(20,3); // millisecond
  return result; 
}


string getDayTimeRangeString(int const &dayTimeNr) {
  string result = "";  
  switch (dayTimeNr) {
    case 1:
      result = "00:00:00.000-12:00:00.000"; // morning
      break;
    case 2:
      result = "12:00:00.000-17:00:00.000"; // afternoon
      break;
    case 3:
      result = "17:00:00.000-21:00:00.000"; // evening
      break;
    case 4:
      result = "21:00:00.000-23:59:59.999"; // night
      break;
  }
  return result;
}