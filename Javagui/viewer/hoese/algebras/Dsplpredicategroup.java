//This file is part of SECONDO.

// Copyright (C) 2004-2007,
// University in Hagen
// Faculty of Mathematics and Computer Science, 
// Database Systems for New Applications.

// SECONDO is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// SECONDO is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with SECONDO; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


package  viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import javax.swing.*;
import javax.swing.JButton;
import java.awt.event.*;
import java.awt.BorderLayout;
import java.awt.Font;


/**
 * A displayclass for the cluster type 
 */
public class Dsplpredicategroup extends DsplGeneric implements DsplSimple, ExternDisplay{
  /** string which is used for wrong formatted nested lists **/
  static final String ERROR="wrong list";
  /** a frame for external display **/
  static ExtWin extWin = new ExtWin();
  /** the formatted text for displaying in the external entry **/
  private String text; // html content of the external window
  /** the matrix as a single line for the entry **/
  private String entry; 
  /** number of columns within the table **/
  private static final int no_columns=6;

  /* returns the used style sheet */
  private static String getCss(){
     // use the same style as a cluster
     return Dsplcluster.getCss();
  }


  /** returns this group as html-formatted text **/  
  public String getContent(ListExpr value){
      
      if(isUndefined(value)){
          return "undefined";
      }
      StringBuffer sb = new StringBuffer();
      int cell = 0;
      int row = 0;
      Dsplcluster cluster = new Dsplcluster();
      sb.append("<h1> Predicategroup </h1><br>");
      while(!value.isEmpty()){
         String clustertext = cluster.getContent(value.first(),2);
         sb.append(clustertext);
         sb.append("<br><br><hr><br><br>");
         value = value.rest();
      }
      return sb.toString();

  }  

  /* returns the string representation for the given value list */
  public String getDisplay(ListExpr value){
      StringBuffer sb = new StringBuffer();
      sb.append("<html> <head> <title> predicategroup </title>");
      sb.append(getCss());
      sb.append("</head>\n");
      sb.append("<body>\n");
      sb.append(getContent(value));
      sb.append("</body>\n");
      sb.append("</html>\n");
      return sb.toString(); 
  }


  /* Sets the entry for the queryresult.
  */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
     init(type,0,value,0,qr);
  }

  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     text = getDisplay(value);
     String V = "predicategroup";
     T=extendString(T,typewidth);
     entry = T + " : " + V;
     qr.addEntry(this);
     return;
  }

  public String toString(){
     return entry;
  }

   /** shows this matrix in an external window **/
   public void displayExtern(){
       extWin.setGroup(this);
       extWin.setVisible(true);
   }

   public boolean isExternDisplayed(){
      return this==extWin.group && extWin.isVisible();
   }

  private static class ExtWin extends JFrame{
     private JEditorPane textArea = new JEditorPane();
     private JButton   closeBtn = new JButton("close");
     private Dsplpredicategroup group = null;
     private static java.awt.Dimension dim = new java.awt.Dimension(100,100);
     private JScrollPane scrollpane;

     /** creates a new external window **/
     public ExtWin(){
       super("Predicategroup");
       getContentPane().setLayout(new BorderLayout());
       setSize(450,600);
       getContentPane().add(closeBtn,BorderLayout.SOUTH);
       closeBtn.addActionListener(new ActionListener(){
           public void actionPerformed(ActionEvent evt){
                ExtWin.this.setVisible(false);
           }
       });
       textArea.setFont(new Font("Monospaced",Font.BOLD,24));
       textArea.setEditable(false); 
       textArea.setContentType("text/html");
       scrollpane = new JScrollPane(textArea); 
       getContentPane().add(scrollpane,BorderLayout.CENTER);
     }    

     public void setGroup(Dsplpredicategroup group){
       this.group = group;
       textArea.setText(group.text);
       textArea.setCaretPosition(0);
     }     

  }


}



