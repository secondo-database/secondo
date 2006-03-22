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


package  viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import javax.swing.JFrame;
import javax.swing.JTextArea;
import javax.swing.JButton;
import java.awt.event.*;
import java.awt.BorderLayout;
import java.awt.Font;


/**
 * A displayclass for the int9m type 
 */
public class Dsplint9m extends DsplGeneric implements DsplSimple, ExternDisplay{
  /** string which is used for wrong formatted nested lists **/
  static final String ERROR="wrong list";
  /** a frame for external display **/
  static ExtWin extWin = new ExtWin();
  /** the formatted text for displaying in the external entry **/
  private String text;
  /** the matrix as a single line for the entry **/
  private String entry; 

  /* returns the string representation for the given value list */
  private String getDisplay(ListExpr value){
     String display = "";
     int at = value.atomType();
     switch(at){
       case ListExpr.INT_ATOM:
          int v = value.intValue();
          int c = 256; 
          for(int i=0;i<9;i++){
             display += (v&c)>0?"1":"0";
             c = c>>1;
          }
          break;
       case ListExpr.NO_ATOM:
           int len = value.listLength();
           if(len!=9){
              display=ERROR;
           }else{
             boolean done=false;
             while(!value.isEmpty() && !done){
               ListExpr f = value.first();
               value = value.rest();
               int atf = f.atomType();
               switch(atf){
                 case ListExpr.INT_ATOM:
                    int i = f.intValue();
                    display += i==0?"0":"1";
                    break;
                case ListExpr.BOOL_ATOM:
                    display += f.boolValue()?"1":"0";
                    break;
                default:
                     display = ERROR;
                     done = true;
               }
             }
           }
           break;
       default: display = ERROR;
     }
     if(display.length()==9){
        text =   display.substring(0,3)+"\n"+display.substring(3,6)
               + "\n"+display.substring(6,9);
        text = text.replaceAll("0","0 ");
        text = text.replaceAll("1","1 ");
        text = text.trim();
     } else{
       text = display;
     }
     return display;

  }


  /* Sets the entry for the queryresult.
  */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
     entry = type.symbolValue()+" : "+getDisplay(value);
     qr.addEntry(this);
  }

  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getDisplay(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     entry = T + " : " + V;
     qr.addEntry(this);
     return;
  }

  public String toString(){
     return entry;
  }

   /** shows this matrix in an external window **/
   public void displayExtern(){
       extWin.setMatrix(this);
       extWin.setVisible(true);
   }

   public boolean isExternDisplayed(){
      return this==extWin.int9m && extWin.isVisible();
   }

  private static class ExtWin extends JFrame{
     private JTextArea textArea = new JTextArea(5,5);
     private JButton   closeBtn = new JButton("close");
     private Dsplint9m int9m = null;
     private static java.awt.Dimension dim = new java.awt.Dimension(100,100);

     /** creates a new external window **/
     public ExtWin(){
       super("9 intersection matrix");
       getContentPane().setLayout(new BorderLayout());
       setSize(100,150);
       getContentPane().add(closeBtn,BorderLayout.SOUTH);
       getContentPane().add(textArea,BorderLayout.CENTER);
       closeBtn.addActionListener(new ActionListener(){
           public void actionPerformed(ActionEvent evt){
                ExtWin.this.setVisible(false);
           }
       });
       textArea.setFont(new Font("Monospaced",Font.BOLD,24));
       textArea.setEditable(false); 
     }    

     
    /** sets the matrix to display **/
    public void setMatrix(Dsplint9m matrix){
       int9m=matrix;
       if(matrix==null){
           textArea.setText("");
       } else{
           textArea.setText(matrix.text);
       }
    }

  }


}



