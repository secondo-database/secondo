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
public class Dsplpage extends Dsplhtml{


private ListExpr files;
private TreeMap map = new TreeMap();  // mapping between urls and files
private static int count;



 /** Creates a new Instance of this.
   */ 
public  Dsplpage(){
   if(Display==null){
      Display = new HTMLViewerFrame();
   }
   count = 0;
}

public String toString(){
   return Entry;
}


protected String changeImgTag(String orig){
   String attr = "(("+pattern_string +")|("+pattern_symbol+"))";
   String src = "[sS][Rr][cC]\\s*=\\s*";
   Pattern p = Pattern.compile(src+attr);
   Matcher m = p.matcher(orig);
   if(!m.find()){
       return orig;
   } 
   String source = m.group();
   source = source.replaceAll("^"+src,"");
   Dsplurl.Url url = new Dsplurl.Url();
   source = source.trim();
   if(source.matches(pattern_string)){
      source = source.substring(1,source.length()-1);
   }

   url.readFrom(source,this.url);


   int pos = source.lastIndexOf(".");
   String ext;
   if(pos<0){
       ext = "";
   } else {
       ext = source.substring(pos);
   }
   String fname ="tmp/file"+count+ext;
   count++;
   map.put(url,fname); 
   File f = new File(fname);
   return orig.replaceAll(src+attr,"src=\"file://"+f.getAbsolutePath()+"\"");
}


protected String changeLinkTag(String orig){
   String attr = "(("+pattern_string +")|("+pattern_symbol+"))";
   String src = "[hH][rR][eE][fF]\\s*=\\s*";
   Pattern p = Pattern.compile(src+attr);
   Matcher m = p.matcher(orig);
   if(!m.find()){
       return orig;
   } 
   String source = m.group();
   source = source.replaceAll("^"+src,"");
   Dsplurl.Url url = new Dsplurl.Url();
   source = source.trim();
   if(source.matches(pattern_string)){
      source = source.substring(1,source.length()-1);
   }

   url.readFrom(source,this.url);


   int pos = source.lastIndexOf(".");
   String ext;
   if(pos<0){
       ext = "";
   } else {
       ext = source.substring(pos);
   }
   String fname ="tmp/file"+count+ext;
   count++;
   map.put(url,fname); 
   File f = new File(fname);
   return orig.replaceAll(src+attr,"src=\"file://"+f.getAbsolutePath()+"\"");
}



protected boolean  scanValue(ListExpr value){
    if(value.listLength()<1){
       Reporter.debug("wrong listlength for page "+ value.listLength());
       return false;
    }
    ListExpr html = value.first();
    if(html.listLength()!=2){ // (type value)
        return false;
    }
    html = html.second();
    if(!super.scanValue(html)){
        files = null;
        return false;
    }
    files = value.rest();
    return true;
}


public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {

     String T = new String(type.symbolValue());
     String V;
     defined = !isUndefined(value);
     
     if(!defined){
        V = "undefined";
        content="undefined";
     }else{

       V = "Page";
       scanValue(value);
     }
     T=extendString(T,typewidth);
     String Text = V;

     Entry = T + " : <page> ...";
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
          switchView(FormatBtn.getText().equals(TXT_SOURCE));
       }
  });


  JPanel ControlPanel = new JPanel();
  ControlPanel.add(CloseBtn);
  ControlPanel.add(SaveBtn);
  ControlPanel.add(FormatBtn);

  getContentPane().add(ControlPanel,BorderLayout.SOUTH);
  setSize(640,480); 
}


public void switchView(boolean toSrc){
   if(toSrc){
        FormatBtn.setText(TXT_HTML);
        Display.setContentType("text/plain");
        Display.setText(TheText); 
        Display.setCaretPosition(0);// go to top 
    } else {
        try{
           FormatBtn.setText(TXT_SOURCE);
           Display.setContentType("text/html");
           Display.setText(TheText); 
           Display.setCaretPosition(0);// go to top 
        }catch(Exception e){
            Reporter.debug(e);
            Reporter.showError("error in rendering html content");
             switchView(true);
        }
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

public void setSource(Dsplpage S){
    deleteFiles(Source);
    Source = S;
    LastSearchPos=0;
    Display.setEditable(false); 
    TheText = S.content;
    try{
       Display.setText(TheText);
       switchView(false); // show html if possible
    } catch(Exception e){
       Reporter.debug("Error in setting text");
       Display.setText(""); 
    }
    Display.setCaretPosition(0);// go to top 
    getContentPane().add(TextPanel,BorderLayout.CENTER);
    createFiles(S.files,S.map);
    invalidate();validate();repaint();
}

private void deleteFiles(Dsplpage source){
   if(source ==null){
      return;
   }
   TreeMap map = source.map;
   Iterator it = map.keySet().iterator();
   while(it.hasNext()){
      String name = (String) map.get(it.next());
      if(name==null){
        Reporter.debug("url not mapped");
      } else {
          File F = new File(name);
          if(F.exists()){
             boolean succ = F.delete();
             if(!succ){
                 Reporter.writeError("cannot delete the file "+F);
             }
          } else{
             Reporter.writeError("try to delete the file " +F + ", but it does not exist");
          }
     }
   }
}

private void createFiles(ListExpr files, TreeMap map){
   if(files.atomType()!=ListExpr.NO_ATOM){
        Reporter.debug("invalid list for embedded files");
        return;
   }
   

   while(!files.isEmpty()){
      ListExpr file = files.first();
      files = files.rest();
      if(file.listLength()!=3){
          Reporter.debug("invalid list length for embedded file");
      } else {
         Dsplurl.Url url = new Dsplurl.Url();
         ListExpr urlList = file.first();
         if(urlList.listLength()==2){
             urlList = urlList.second();
         }
         if(!url.readFrom(urlList)){
              Reporter.debug("cannot extract url from " + file.first());
         } else {
             String fName = (String) map.get(url);

             if(fName==null){
                 Reporter.debug("url " + url+" is not mapped");
             } else {
                try{
                   String text = file.second().textValue();
                   byte[] content = Base64Decoder.decode(text);
                   FileOutputStream out = new FileOutputStream(fName);
                   for(int i=0;i<content.length;i++){
                       out.write(content[i]);
                   }  
                   out.close();
                   (new File(fName)).deleteOnExit();
                } catch(Exception e){
                   Reporter.debug(e);
                   Reporter.debug("Error in extracting file");
                 
                }

             }
         }
      }

   }

}

public Dsplhtml getSource(){
     return Source;
}


private JEditorPane Display;
private JButton CloseBtn;
private JButton SaveBtn;
private JFileChooser fileChooser;
private Dsplpage Source;
private String TheText;
private static JScrollPane TextScrollPane;
private static JPanel TextPanel;
private JCheckBox CaseSensitive = new JCheckBox("case sensitive");
private JTextField SearchField = new JTextField(20);
private JButton SearchBtn = new JButton("search");
private final static String TXT_SOURCE = "source";
private final static String TXT_HTML = "html";
private JButton FormatBtn = new JButton(TXT_SOURCE);
private int LastSearchPos=0;

}

//public static final String pattern_string = "(\"((\\\\\")|([^\"]))*\")|('((\\\\')|([^']))*')";
public static final String pattern_string = "(\"((\\\\\")|([^\"]))*\")";
public static final String pattern_symbol = "[^\\s\"]+";




}



