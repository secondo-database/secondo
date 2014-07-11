
//This file is part of SECONDO.

//Copyright (C) 2013, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.hoese;

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import tools.Reporter;

/**
 * This class computes strings that are necessary for displaying symbolic
 * trajectories.
 */
public class SymbolicValues {

  /**
   * Returns true iff the parameter equals one of the symbolic types.
   */
  public static boolean isSymbolicType(String typeStr) {
    return (typeStr.equals("mlabel") || typeStr.equals("mlabels") ||
            typeStr.equals("mplace") || typeStr.equals("mplaces"));
  }

  /**
   * Takes an interval and returns its start instant as a string. In the non-
   * standard case (i.e., rightclosed or not leftclosed), the leftclosed
   * indicator is prepended.
   */
  public static String intervalStartToString(Interval iv) {
    String result = "";
    if (!iv.isLeftclosed() || iv.isRightclosed()) {// non-standard
      result += (iv.isLeftclosed() ? "[" : "(");
    }
    else {
      result += " ";
    }
    return result + LEUtils.convertTimeToString(iv.getStart());  
  }
  
  /**
   * Returns true iff an interval is adjacent to another.
   */
  public static boolean areIntervalsAdjacent(Interval iv1, ListExpr value) {
    if (value.isEmpty()) { // last unit
      return false;
    }
    if (value.first().listLength() == 2) {
      Interval iv2 = LEUtils.readInterval(value.first().first());
      if (iv1.getEnd() == iv2.getStart()) {
        return (iv1.isRightclosed() != iv2.isLeftclosed());
      }
    }
    return false;
  }
  
  /**
   * Takes an interval and returns its end instant as a string, starting only
   * at the time detail that differs from the start instant. In the non-standard
   * case, the rightclosed indicator is appended.
   */
  public static String intervalEndToString(Interval iv) {
    String result = "";
    String startStr = LEUtils.convertTimeToString(iv.getStart());
    String endStr = LEUtils.convertTimeToString(iv.getEnd());
    boolean print = false;
    for (int pos = 0; pos < startStr.length(); pos++) {
      if (print) {
        result += endStr.charAt(pos);
      }
      else if (!print && (startStr.charAt(pos) == endStr.charAt(pos))) {
        result += " ";
      }
      else { // first inequality
        while ((pos >= 0) && (startStr.charAt(pos) >= '0') && 
               (startStr.charAt(pos) <= '9')) {
          pos--;
        }
        result = (pos >= 0 ? result.substring(0, pos + 1) : "");
        print = true;
      }
    }
    if (!iv.isLeftclosed() || iv.isRightclosed()) {// non-standard
      result += (iv.isRightclosed() ? "]" : ")");
    }
    return " " + result;
  }
  
  public static String labelToString(ListExpr label) {
    if (label.atomType() != ListExpr.TEXT_ATOM) {
      return "ERROR: wrong label type";
    }
    return label.textValue();
  }
  
  public static String labelsToString(ListExpr labels) {
    String result = "{";
    while (!labels.isEmpty()) {
      ListExpr label = labels.first();
      if (label.atomType() != ListExpr.TEXT_ATOM) {
        return "ERROR: wrong label type";
      }
      labels = labels.rest();
      result += label.textValue() + (!labels.isEmpty() ? ", " : "}");
    }
    return result;
  }
  
  public static String placeToString(ListExpr place) {
    if (place.listLength() != 2) {
      return "ERROR: wrong list size for a place";
    }
    String result = labelToString(place.first());
    if (place.second().atomType() != ListExpr.INT_ATOM) {
      return "ERROR: wrong place reference type";
    }
    return result + " : " + place.second().intValue();
  }
  
  public static String placesToString(ListExpr places) {
    String result = "{";
    while (!places.isEmpty()) {
      result += placeToString(places.first());
      places = places.rest();
      result += (!places.isEmpty() ? ", " : "}");
    }
    return result;
  }
}