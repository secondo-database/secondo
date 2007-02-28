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

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.EditorKit;
import javax.swing.text.Document;
import tools.Base64Decoder;
import org.jpedal.*;
import tools.Reporter;
import java.io.*;
import java.util.*;
import java.util.regex.*;


/**
 * A displayclass for the html formatted code 
 */
public class Dsplhtml extends DsplGeneric implements ExternDisplay,DsplSimple{

protected boolean defined;
protected String content;
protected Dsplurl.Url url;


 /** Creates a new Instance of this.
   */ 
public  Dsplhtml(){
   if(Display==null){
      Display = new HTMLViewerFrame();
   }
}


/** Adds a button to the query which on a Click would be 
  * pop up a window
  **/
public void init (ListExpr type, ListExpr value, QueryResult qr) {
     init(type,0,value,0,qr);
}
  

public String toString(){
   return Entry;
}


protected String changeMetaTag(String orig){

   return "";  // we don't need meta tags for display
   // remove charsetinfo because of problems with this attribute 
   //String attr = "(("+pattern_string +")|("+pattern_symbol+"))";
   //String cs = "[cC][hH][aA][rR][sS][eE][tT]\\s*=\\s*"+attr;
   //return orig.replaceAll(cs,"");
}

protected String changeImgTag(String orig){
   String attr = "(("+pattern_string +")|("+pattern_symbol+"))";
   String src = "[sS][Rr][cC]\\s*=\\s*"+attr;
   return orig.replaceAll(src,"src=\"dummy\"");
}

protected String changeLinkTag(String orig){
   String attr = "(("+pattern_string +")|("+pattern_symbol+"))";
   String src = "[hH][Rr][eE][fF]\\s*=\\s*"+attr;
   return orig.replaceAll(src,"src=\"dummy\"");
}

protected String changeTag(String orig){
   if(orig.matches("<[mM][eE][tT][Aa]\\s([^a]|[a])*")){
       String res =  changeMetaTag(orig);
       return res;
   }
   if(orig.matches("<[iI][mM][gG]\\s([^a]|[a])*")){
       return changeImgTag(orig);
   }
   if(orig.matches("<[lL][iI][nN][kK]\\s([^a]|[a])*")){
       return changeLinkTag(orig);
   }
   // insert further known tags to change

   return orig;
}

protected String changeTags(String content){
    StringBuffer result = new StringBuffer();
    int lastPos = 0;

    String pattern_tag = "<[a-zA-Z!]*\\s((\\s|[^>])|("+pattern_string+"))*>"; 

    Pattern p = Pattern.compile(pattern_tag);
    Matcher m = p.matcher(content);
    while(m.find()){
        String rest = content.substring(lastPos,m.start());
        result.append(rest);
        

        String orig = m.group();
        String changedTag = changeTag(orig);
        result.append(changedTag);
        lastPos = m.end(); 
    }

    String rest = content.substring(lastPos,content.length());

    result.append(rest);

    String res = result.toString();
    return res;

}

protected boolean  scanValue(ListExpr value){
    if(value.listLength()==2 && value.first().atomType()==ListExpr.SYMBOL_ATOM &&
       value.first().symbolValue().equals("html")){
       value = value.second();
    }


    if(value.listLength()!=3){
       Reporter.debug("wrong listlength for html "+value.listLength());
       return false;
    }
    Double Time = LEUtils.readInstant(value.first());
    if(Time==null){
       Reporter.debug("instant not readable");
       return false;
    }
    this.url= new Dsplurl.Url();
    url.readFrom(value.third());



    if(value.second().atomType()!=ListExpr.TEXT_ATOM){
        Reporter.debug("not an text atom");
        return false;
    }

    String contentb64 = value.second().textValue();
    content=null;
    try{
        content = new String(Base64Decoder.decode(contentb64));
    } catch(Exception e){
      Reporter.debug("could not decode Base64 text atom");
      Reporter.debug(contentb64);
      return false;
    }

    content = changeTags(content);

    return true;    

}


public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V;

     if (value.listLength()==1){
         value = value.first();
     }
     defined = !isUndefined(value);
     if(!defined){
        V = "undefined";
        content="undefined";
     }else{

       V = "HTML";

       scanValue(value);

     }
     T=extendString(T,typewidth);
     String Text = V;

     Entry = T + " : <html> ...";
     qr.addEntry(this);
     return;
 }


public void displayExtern(){
    Display.setSource(this);
    Display.setVisible(true);    
}

public boolean isExternDisplayed(){
   return Display.isVisible() && this.equals(Display.getSource());
}

private static boolean isWhiteSpace(char c){
   return WHITESPACES.indexOf(c)>=0;
}


private static HTMLViewerFrame Display=null; 
private String Entry;

private int Type; // contains the type which is the text (probably)


private static final int MAX_DIRECT_DISPLAY_LENGTH = 30;

private static final String WHITESPACES = " \t\n\r";


private static class HTMLViewerFrame extends JFrame{

public HTMLViewerFrame(){

  getContentPane().setLayout(new BorderLayout());
  Display = new JEditorPane();
  Display.setContentType("text/html");
  TextScrollPane = new JScrollPane(Display);
  TextPanel = new JPanel(new BorderLayout());
  JPanel SearchPanel = new JPanel();
  SearchPanel.add(CaseSensitive);
  SearchPanel.add(SearchField);
  SearchPanel.add(SearchBtn);
  SearchBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        searchText();
     }
  });
  SearchField.addKeyListener(new KeyListener(){
     public void keyPressed(KeyEvent evt){
         if(evt.getKeyCode()==KeyEvent.VK_ENTER)
            searchText();
     }
     public void keyTyped(KeyEvent evt){}
     public void keyReleased(KeyEvent evt){}
  });
  TextPanel.add(TextScrollPane,BorderLayout.CENTER);
  TextPanel.add(SearchPanel,BorderLayout.SOUTH); 

  getContentPane().add(TextPanel,BorderLayout.CENTER);
  CloseBtn = new JButton("Close");
  CloseBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            TheText=null;
            HTMLViewerFrame.this.setVisible(false);
       }
  } );

  fileChooser = new JFileChooser();
  SaveBtn = new JButton("Save");
  SaveBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        if(fileChooser.showSaveDialog(HTMLViewerFrame.this)==JFileChooser.APPROVE_OPTION){
             File F = HTMLViewerFrame.this.fileChooser.getSelectedFile();
             try{
                 BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(F));
                 out.write(TheText.getBytes());
                 out.flush();
                 out.close();
               }catch(Exception e){
                  Reporter.debug(e);
                  Reporter.showError("Error in saving file ");
               }
        }
     }
  });
  FormatBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
          String s = ((JButton)evt.getSource()).getText();
          switchFormat(!s.equals(TXT_SOURCE));
      }
  });

  JPanel ControlPanel = new JPanel();
  ControlPanel.add(CloseBtn);
  ControlPanel.add(SaveBtn);
  ControlPanel.add(FormatBtn);

  getContentPane().add(ControlPanel,BorderLayout.SOUTH);
  setSize(640,480); 
}


public void switchFormat(boolean toHTML){
   if(toHTML){
     try{
       FormatBtn.setText(TXT_SOURCE);
       Display.setContentType("text/html");
       Display.setText(TheText);
       Display.setCaretPosition(0);
     }catch(Exception e){
       Reporter.showError("Error in rendering html ");
       switchFormat(false);
     } 
   } else {
     FormatBtn.setText(TXT_HTML);
     Display.setContentType("text/plain");
     Display.setText(TheText);
     Display.setCaretPosition(0);
   }
}


/** searchs the text in the textfield in the document and
  * marks its if found
  */
private void searchText(){
  String Text = SearchField.getText();
  if(Text.length()==0){
    Reporter.showError("no text to search");
    return;
  }
  try{
     Document Doc = Display.getDocument();
     String DocText = Doc.getText(0,Doc.getLength());
     if(!CaseSensitive.isSelected()){
        DocText = DocText.toUpperCase();
        Text = Text.toUpperCase();
     }
     int pos = DocText.indexOf(Text,LastSearchPos);
     if(pos<0){
        Reporter.showError("end of text is reached");
        LastSearchPos=0;
        return;
     }
     pos = pos;
     int i1 = pos;
     int i2 = pos+Text.length();
     LastSearchPos = pos+1;
     Display.setCaretPosition(i1);
     Display.moveCaretPosition(i2);
     Display.getCaret().setSelectionVisible(true);
  } catch(Exception e){
    Reporter.debug(e);
    Reporter.showError("error in searching text");

  }

}

public void setSource(Dsplhtml S){
    Source = S;
    LastSearchPos=0;
    Display.setEditable(false); 
    TheText = S.content;
    try{
       Display.setText(TheText);
    } catch(Exception e){
       Reporter.debug("Error in setting text");
       Display.setText(""); 
    }
    Display.setCaretPosition(0);// go to top 
    getContentPane().add(TextPanel,BorderLayout.CENTER);
    invalidate();validate();repaint();
}

public Dsplhtml getSource(){
     return Source;
}


private JEditorPane Display;
private JButton CloseBtn;
private JButton SaveBtn;
private JFileChooser fileChooser;
private Dsplhtml Source;
private String TheText;
private static JScrollPane TextScrollPane;
private static JPanel TextPanel;
private JCheckBox CaseSensitive = new JCheckBox("case sensitive");
private JTextField SearchField = new JTextField(20);
private JButton SearchBtn = new JButton("search");
private static final String TXT_SOURCE = "source";
private static final String TXT_HTML="html";
private JButton FormatBtn = new JButton(TXT_SOURCE);
private int LastSearchPos=0;

}

public static final String pattern_string = "(\"((\\\\\")|([^\"]))*\")|('((\\\\')|([^']))*')";
public static final String pattern_symbol = "[^\\s\"]+";




}



