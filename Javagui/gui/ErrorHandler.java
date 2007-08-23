

//This file is part of SECONDO.

//Copyright (C) 2004-2007, University in Hagen,i
//Faculty of Mathematics and  Computer Science, 
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

package gui;

import sj.lang.*;


public class ErrorHandler implements MessageListener {
  private CommandPanel cp;

  public ErrorHandler(CommandPanel cp){
      this.cp = cp;
  }

  /*
   * Implementation of interface MessageListener
   *
   * Output of a error messages from the server
   */
    public void processMessage(ListExpr in_xMessage)
    {
      // Check if it is a message (and not a progress information etc.)
      if(in_xMessage.listLength() == 0 ||
         in_xMessage.first().atomType() != ListExpr.SYMBOL_ATOM ||
         (
            !in_xMessage.first().symbolValue().equals("simple") &&
            !in_xMessage.first().symbolValue().equals("error"))
         )
      {
        return;
      }
      // Messages should be in one Atom of type TEXT_ATOM. But this listener
      // will process Messages in other formats as well.
      ListExpr xRest = in_xMessage.rest();
      String strMessage = "";
      while(!xRest.isEmpty())
      {
        ListExpr xCurrent = xRest.first();
        if(in_xMessage.second().atomType() == ListExpr.STRING_ATOM)
        {
          strMessage += xCurrent.stringValue();
        }
        if(in_xMessage.second().atomType() == ListExpr.SYMBOL_ATOM )
        {
          strMessage += xCurrent.symbolValue();
        }
        if(in_xMessage.second().atomType() == ListExpr.TEXT_ATOM )
        {
          strMessage += xCurrent.textValue();
        }
        xRest = xRest.rest();
      }
      // Output Message in Panel
      if(in_xMessage.first().symbolValue().equals("simple"))
      {
        // Message
        cp.appendText("\n" + strMessage + "\n");
      }
      else
      {
        // Error
        cp.appendErr("\n" + strMessage + "\n");
      }
    }
}

