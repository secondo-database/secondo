//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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


package  viewer.hoese;

import java.io.*;
import javax.swing.*;
import sj.lang.ListExpr;
import viewer.HoeseViewer;
import tools.Reporter;


/**
 * The class for executing of external applicaation associaated with external datatypes
 */
public class Launcher
    implements SecondoFrame {
  //public JFrame getFrameInstance() {return (JFrame)TestFrame;}
  private Dsplexternal de;

  /**
   * Executes the external application 
   * @param b b is ignored
   * @see <a href="Launchersrc.html#show">Source</a>
   */
  public void show (boolean b) {
    if ((de == null) || (de.getCommand() == null))
      return;
    Runtime r = Runtime.getRuntime();
    try {
      r.exec(de.getCommand());
    } catch (IOException e) {
      Reporter.writeError("Error while starting:" + de.getCommand());
    }
  }

  /**
   * Not implemented because not all application can do it
   * @param o 
   */
  public void addObject (Object o) {}
  ;
  /**
   * Not implemented because not all application can do it
   * @param o
   */
  public void removeObject (Object o) {}
  ;
  /**
   * Sets the Dsplexternal object with the command to execute
   * @param o Instance to select
   * @see <a href="Launchersrc.html#select">Source</a>
   */
  public void select (Object o) {
    de = (Dsplexternal)o;
  }
}



