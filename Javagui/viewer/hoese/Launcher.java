

package  viewer.hoese;

import  java.io.*;
import  javax.swing.*;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;


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
      System.out.println("Error while starting:" + de.getCommand());
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



