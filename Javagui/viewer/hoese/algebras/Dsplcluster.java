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
public class Dsplcluster extends DsplGeneric implements  ExternDisplay{
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
  public static String getCss(){
    return "<style type=\"text/css\">\n" 
             + "body{"
             + "font-size:14pt; "
             + "font-style:normal;"
             + "font-weight:normal;"
             + "font-stretch:normal;"
             + "font-family:serif;"
             + "color:black;"
             + "background-color:white;"
             + "text-align:justify;"
             + "margin-left:1em;"
             + "margin-right:1em;"
             + "}"

             + "h1{ color:blue;"
             + "text-align:left;"
             + "font-size:xx-large;"
             + "font-style:normal;"
             + "font-weight:bold;"
             + "font-stretch:wider;"
             + "font-family:serif; }"

             + "h2{ color:blue;"
             + "font-size:x-large;"
             + "font-style:normal;"
             + "font-weight:bold;"
             + "font-stretch:wider;"
             + "font-family:serif; }"

             + "table{ "
             + "font-size:14pt;"
             + "font-family:monospace;"
             + "fonst-style:normal;"
             + "border-spacing:14pt;"
             + "}"
             + "td{ "
             + "font-size:14pt;"
             + "font-family:monospace;"
             + "fonst-style:normal;"
             + "border-color:blue;"
             + "border-style:solid;"
             + "border-width:1pt;"
             + "}"
           + "</style>\n";
  }

  /** Extracts the name from the list.
    * If the structure of value does not allow to extract the name
    * (i.e. the structure does not represent a valid cluster),
    * null is returned.
    **/
  public String getName(ListExpr value){
      if(value.listLength()<1){
          return null;
      }      
      ListExpr namelist = value.first();
      if(namelist.atomType()!=ListExpr.STRING_ATOM){
          return null;
      }
      return namelist.stringValue(); 
  }

  
  public String getContent(ListExpr value,int headLevel){
      
      if(isUndefined(value)){
          return "undefined";
      }
      String name = getName(value);
      if(name==null){
         return ERROR;
      }
      if(headLevel<1){
        headLevel=1;
      }
      if(headLevel>9){
        headLevel=9;
      }
      StringBuffer sb = new StringBuffer();
      sb.append("<h"+headLevel+"> "+name + "</h"+headLevel+">\n");
      int cell = 0;
      int row = 0;
      Dsplint9m int9m = new Dsplint9m();
      value = value.rest(); // jump over the name
      sb.append("<table> \n");
      while(!value.isEmpty()){
         if( cell % no_columns==0){
            sb.append("<tr>");
            row++;
         }
         String celltext = int9m.getDisplay(value.first());
         celltext=celltext.replaceAll("\n","<br>\n"); 
         sb.append("<td>");
         sb.append(celltext);
         sb.append("</td>");
         cell++;
         row++;
         value = value.rest();
         if(cell % no_columns ==0 || value.isEmpty()){
             sb.append("</tr>");
         }
      }
      sb.append("</table>\n");
      return sb.toString();

  }  

  /* returns the string representation for the given value list */
  public String getDisplay(ListExpr value){
      String name = getName(value);
      if(name==null){ // error in listExpr
         name ="error";
      }
      StringBuffer sb = new StringBuffer();
      sb.append("<html> <head> <title> cluster: "+name+"</title>");
      sb.append(getCss());
      sb.append("</head>\n");
      sb.append("<body>\n");
      sb.append(getContent(value,1));
      sb.append("</body>\n");
      sb.append("</html>\n");
      return sb.toString(); 
  }


  /* Sets the entry for the queryresult.
  */
  public void init (String name, int nameWidth, int indent,
                    ListExpr type, ListExpr value,
                    QueryResult qr)
  {
     String T = name;
     text = getDisplay(value);
     String V = "cluster";
     T=extendString(T,nameWidth, indent);
     entry = T + " : " + V;
     qr.addEntry(this);
     return;
  }

  public String toString(){
     return entry;
  }

   /** shows this matrix in an external window **/
   public void displayExtern(){
       extWin.setCluster(this);
       extWin.setVisible(true);
   }

   public boolean isExternDisplayed(){
      return this==extWin.cluster && extWin.isVisible();
   }

  private static class ExtWin extends JFrame{
     private JEditorPane textArea = new JEditorPane();
     private JButton   closeBtn = new JButton("close");
     private Dsplcluster cluster = null;
     private static java.awt.Dimension dim = new java.awt.Dimension(100,100);
     private JScrollPane scrollpane;

     /** creates a new external window **/
     public ExtWin(){
       super("Cluster");
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

     public void setCluster(Dsplcluster cluster){
       this.cluster = cluster;
       textArea.setText(cluster.text);
       textArea.setCaretPosition(0);
     }     

  }


}



