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
import tools.Base64Decoder;
import org.jpedal.*;


/**
 * A displayclass for the html formatted code 
 */
public class Dspltext extends DsplGeneric implements ExternDisplay{

 /** Creates a new Instance of this.
   */ 
public  Dspltext(){
   if(Display==null){
      Display = new TextViewerFrame();
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

public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V;

     if (value.listLength()==1)
     value = value.first();
     if(value.atomType()!=ListExpr.TEXT_ATOM){
        V =  "error in value ";
        theList = ListExpr.textAtom(V);
     }
     else{
        V =  value.textValue();
        theList = value;
     }
     T=extendString(T,typewidth);
     String Text = V;
     computeType(Text);

     if(Type==PLAIN_TYPE){
        if(Text.length()<=MAX_DIRECT_DISPLAY_LENGTH){ // short Text
            Entry = type.symbolValue()+" : "+ Text;
            qr.addEntry(Entry); // avoid the possibility to pop up a window
            return; 
         } else{  // long plain text
               Entry = T+" : "+ Text.substring(0,MAX_DIRECT_DISPLAY_LENGTH-4).replace('\n',' ')+" ...";
         }
     }else if(Type==HTML_TYPE){
           Entry = T + " : <html> ...";
     } else if(Type==RTF_TYPE){
         Entry =  T + " : RTF ...";
     } else if(Type==PDF_TYPE){
         Entry = T + " : PDF ...";
     }

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


private boolean checkForHtml(String Text,int offset){
    int restlength = Text.length()-offset-1;
    if(restlength<4)
      return false;
    String tag = Text.substring(offset,offset+5).toLowerCase();
    if("<html".equals(tag))
       return true;
    // search for "<html ignoring case
    int max = Text.length();
    int index = offset+1;
    int state = 1;
    boolean found = false;
    while(!found && index<max-5){
        char c = Text.charAt(index);
        index++;
        switch(state){
            case 1 : if(c=='<') state = 2;break;
            case 2 : if(c=='h' || c=='H') state=3; else state=1; break;
            case 3 : if(c=='t' || c=='T') state=4; else state=1; break;
            case 4 : if(c=='m' || c=='M') state=5; else state=1; break;
            case 5 : if(c=='l' || c=='L') found=true; else state=1; break;
            default: System.err.println("Undefined state");
        }
        if(c=='<') state = 2; 
    }  
    return found;
}

/** sets the type of this Text 
  * depending on some keywords 
  **/
private void computeType(String Text){
     if(Text.startsWith("JVBE") ){ // base 64 encodes "%PD"
         Type = PDF_TYPE;
         return;
     }
     // search for <html or { *\rtf at the begin of the document ignoring cases
     for(int i=0;i<Text.length()-5;i++){ 
         char c = Text.charAt(i);
         if(c=='<'){
              if(checkForHtml(Text,i))
                  Type=HTML_TYPE;
              else
                  Type=PLAIN_TYPE;
              return;
         } else if(c=='{'){ // possible rtf format
           // search for the next non-whitespace
           for(int j=i+1;j<Text.length()-4;j++){

                c = Text.charAt(j);
                if(!isWhiteSpace(c)){
                   String test = Text.substring(j,j+4);
                   if("\\rtf".equals(test))
                       Type = RTF_TYPE;
                    else
                       Type = PLAIN_TYPE;
                    return;
                }
           }
           Type = PLAIN_TYPE; // no \rtf found
           return;
         } 
         else{
             if(!isWhiteSpace(c)){ // not an html document
                Type = PLAIN_TYPE;
                return;
             }
         }
     }  
   // only whitespaces in text
   Type = PLAIN_TYPE; 
}


private static boolean isWhiteSpace(char c){
   return WHITESPACES.indexOf(c)>=0;
}


private static TextViewerFrame Display=null; 
private String Entry;
private ListExpr theList;

private int Type; // contains the type which is the text (probably)

private static final int PLAIN_TYPE=0;
private static final int HTML_TYPE=1;
private static final int RTF_TYPE=2;
private static final int PDF_TYPE=3;

private static final int MAX_DIRECT_DISPLAY_LENGTH = 30;

private static final String WHITESPACES = " \t\n\r";


private static class TextViewerFrame extends JFrame{

public TextViewerFrame(){

  getContentPane().setLayout(new BorderLayout());
  Display = new JEditorPane();
  if(EKPlain==null){
      EKPlain = Display.createEditorKitForContentType("text/plain");
      EKHtml = Display.createEditorKitForContentType("text/html");
      EKRtf = Display.createEditorKitForContentType("text/rtf");
  }

  ScrollPane = new JScrollPane(Display);
  getContentPane().add(ScrollPane,BorderLayout.CENTER);
  CloseBtn = new JButton("Close");
  CloseBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            TheText=null;
            TextViewerFrame.this.setVisible(false);
       }
  } );
  

  // create the pdf viewer
  if(pdf_viewer==null){
    try{
       pdf_viewer = new PDFPanel();
    }catch(Exception e){
      System.err.println("cannot initialize pdf-viewer");
    }
  }

  PlainBtn = new JButton("plain");
  HtmlBtn = new JButton("html");
  RtfBtn = new JButton("rtf"); 
  PdfBtn = new JButton("pdf");

  ActionListener FormatSwitcher = new ActionListener(){
     public void actionPerformed(ActionEvent evt){
         Object src = evt.getSource();
         // get the text if it is editable 
         if(TextViewerFrame.this.Display.isEditable()&&!ISPDF){
              TextViewerFrame.this.TheText = TextViewerFrame.this.Display.getText();
         }
         Container CP=TextViewerFrame.this.getContentPane();
         if(TextViewerFrame.this.PlainBtn.equals(src)){
              TextViewerFrame.this.Display.setEditorKit(TextViewerFrame.this.EKPlain);
              TextViewerFrame.this.Display.setEditable(true);
              TextViewerFrame.this.Display.setText(TextViewerFrame.this.TheText);
              TextViewerFrame.this.Display.setCaretPosition(0);
              TextViewerFrame.this.Source.Type = Dspltext.PLAIN_TYPE;
              TextViewerFrame.this.PlainBtn.setEnabled(false);
              TextViewerFrame.this.HtmlBtn.setEnabled(true);
              TextViewerFrame.this.RtfBtn.setEnabled(true);
              TextViewerFrame.this.PdfBtn.setEnabled(true);
              if(TextViewerFrame.this.ISPDF){
                 CP.remove(pdf_viewer);
                 CP.add(ScrollPane,BorderLayout.CENTER);
                 ISPDF=false;
                 TextViewerFrame.this.invalidate();
                 TextViewerFrame.this.validate();
                 TextViewerFrame.this.repaint(); 
              }
         } else
         if(TextViewerFrame.this.HtmlBtn.equals(src)){
            try{
              TextViewerFrame.this.Display.setEditorKit(TextViewerFrame.this.EKHtml);
              TextViewerFrame.this.Display.setEditable(false);
              TextViewerFrame.this.Display.setText(TextViewerFrame.this.TheText);
              TextViewerFrame.this.Display.setCaretPosition(0);
              TextViewerFrame.this.Source.Type = Dspltext.HTML_TYPE;
              TextViewerFrame.this.PlainBtn.setEnabled(true);
              TextViewerFrame.this.HtmlBtn.setEnabled(false);
              TextViewerFrame.this.RtfBtn.setEnabled(true);
              TextViewerFrame.this.PdfBtn.setEnabled(true);
              if(TextViewerFrame.this.ISPDF){
                 CP.remove(pdf_viewer);
                 CP.add(ScrollPane,BorderLayout.CENTER);
                 ISPDF=false;
                 TextViewerFrame.this.invalidate();
                 TextViewerFrame.this.validate();
                 TextViewerFrame.this.repaint(); 
              }
            }catch(Exception e){
               setToPlainBecauseError();
            }
         }else
         if(TextViewerFrame.this.RtfBtn.equals(src)){
            try{
              TextViewerFrame.this.Display.setEditorKit(TextViewerFrame.this.EKRtf);
              TextViewerFrame.this.Display.setEditable(false);
              TextViewerFrame.this.Display.setText(TextViewerFrame.this.TheText);
              TextViewerFrame.this.Display.setCaretPosition(0);
              TextViewerFrame.this.Source.Type = Dspltext.RTF_TYPE;
              TextViewerFrame.this.PlainBtn.setEnabled(true);
              TextViewerFrame.this.HtmlBtn.setEnabled(true);
              TextViewerFrame.this.RtfBtn.setEnabled(false);
              TextViewerFrame.this.PdfBtn.setEnabled(true);
              if(TextViewerFrame.this.ISPDF){
                 CP.remove(pdf_viewer);
                 CP.add(ScrollPane,BorderLayout.CENTER);
                 ISPDF=false;
                 TextViewerFrame.this.invalidate();
                 TextViewerFrame.this.validate();
                 TextViewerFrame.this.repaint(); 
              }
            } catch(Exception e){
              setToPlainBecauseError(); 
            }
         }else
         if(TextViewerFrame.this.PdfBtn.equals(src)){
             byte[]  content = Base64Decoder.decode(TheText);
             if(pdf_viewer.setPdfData(content)){   
                TextViewerFrame.this.PlainBtn.setEnabled(true);
                TextViewerFrame.this.HtmlBtn.setEnabled(true);
                TextViewerFrame.this.RtfBtn.setEnabled(true);
                TextViewerFrame.this.PdfBtn.setEnabled(false);
                if(!TextViewerFrame.this.ISPDF ){
                    CP.remove(ScrollPane);
                    CP.add(pdf_viewer,BorderLayout.CENTER);
                    ISPDF=true;
                    TextViewerFrame.this.invalidate();
                    TextViewerFrame.this.validate();
                    TextViewerFrame.this.repaint(); 
                }
             }else {
                setToPlainBecauseError();
             }

         }

     }
  };

  PlainBtn.addActionListener(FormatSwitcher);
  HtmlBtn.addActionListener(FormatSwitcher);
  RtfBtn.addActionListener(FormatSwitcher);
  PdfBtn.addActionListener(FormatSwitcher);

  JPanel ControlPanel = new JPanel(new GridLayout(2,1));
  JPanel FormatPanel = new JPanel();
  FormatPanel.add(new JLabel("show as : "));
  FormatPanel.add(PlainBtn);
  FormatPanel.add(HtmlBtn);
  FormatPanel.add(RtfBtn); 
  FormatPanel.add(PdfBtn);
  ControlPanel.add(FormatPanel);
  ControlPanel.add(CloseBtn);

  getContentPane().add(ControlPanel,BorderLayout.SOUTH);
  setSize(640,480); 
}


private void setToPlainBecauseError(){
    JOptionPane.showMessageDialog(null,"Cannot show the text in specified format\n"+
                                       ", switch to plain text","Error",JOptionPane.ERROR_MESSAGE);
    
    Display.setEditorKit(EKPlain);
    Display.setEditable(true);
    TheText = Source.theList.textValue();
    Display.setText(TheText);
    Display.setCaretPosition(0);
    Source.Type = Dspltext.PLAIN_TYPE;
    PlainBtn.setEnabled(false);
    HtmlBtn.setEnabled(true);
    RtfBtn.setEnabled(true);
    PdfBtn.setEnabled(true);
    if(ISPDF){
       getContentPane().remove(pdf_viewer); 
       getContentPane().add(ScrollPane,BorderLayout.CENTER);
       ISPDF=false;
       invalidate();
       validate();
       repaint();
    } 
}

public void setSource(Dspltext S){
    Source = S;
    PlainBtn.setEnabled(true);
    HtmlBtn.setEnabled(true);
    RtfBtn.setEnabled(true);
    PdfBtn.setEnabled(true);
    if(S.Type==S.PLAIN_TYPE){
       Display.setEditorKit(EKPlain);
       PlainBtn.setEnabled(false);
       Display.setEditable(true); 
    }else if(S.Type==S.HTML_TYPE){
       Display.setEditorKit(EKHtml);
       HtmlBtn.setEnabled(false);
       Display.setEditable(false); 
    } else if(S.Type==S.RTF_TYPE){
       Display.setEditorKit(EKRtf);
       RtfBtn.setEnabled(false);
       Display.setEditable(false); 
    }
    TheText = S.theList.textValue();
    if(S.Type!=PDF_TYPE){
       try{
          Display.setText(TheText);
       } catch(Exception e){
         setToPlainBecauseError(); 
       }
        Display.setCaretPosition(0);// go to top 
       if(ISPDF){
          ISPDF=false;
          getContentPane().remove(pdf_viewer);
          getContentPane().add(ScrollPane,BorderLayout.CENTER);
          invalidate();validate();repaint();
       }
    } else{
       try{
          byte[] content = Base64Decoder.decode(TheText);
          pdf_viewer.setPdfData(content);
          PdfBtn.setEnabled(false);
          if(!ISPDF){
             getContentPane().remove(ScrollPane);
             getContentPane().add(pdf_viewer,BorderLayout.CENTER);
             ISPDF=true;
             invalidate();validate();repaint();
           }
       }catch(Exception e){
          e.printStackTrace();
          setToPlainBecauseError();
       }
    }
}

public Dspltext getSource(){
     return Source;
}

private class PDFPanel extends JPanel{

public PDFPanel(){
     super();
     setLayout(new BorderLayout());
     JPanel CommandPanel = new JPanel();
     CommandPanel.add(FirstBtn);
     CommandPanel.add(PrevBtn);
     CommandPanel.add(NextBtn);
     CommandPanel.add(LastBtn);
     CommandPanel.add(new JLabel("    "));
     CommandPanel.add(GBtn);
     CommandPanel.add(LBtn);
     ScrollPane = new JScrollPane(CurrentPage);
     add(ScrollPane,BorderLayout.CENTER);
     add(CommandPanel,BorderLayout.SOUTH);
     ActionListener Control= new ActionListener(){
        public void actionPerformed(ActionEvent evt){
            Object src = evt.getSource();
            if(!dataAvailable)
               return;
            int newpage = page;
            if(src.equals(FirstBtn)){
                newpage = 1;
            } else if(src.equals(PrevBtn)&& page>1){
                newpage = page -1;
            } else if(src.equals(NextBtn) && page < NumberOfPages){
                newpage = page+1;
            } else if(src.equals(LastBtn)){
                newpage=NumberOfPages;
            }
            if(newpage!=page){
               page=newpage;
               try{
                  CurrentPage.setImage(pdf_decoder.getPageAsImage(page));
                  PDFPanel.this.repaint(); 
               } catch(Exception e){
                  e.printStackTrace();
                  CurrentPage.setImage(null);
                  PDFPanel.this.repaint();
               }
           }
        }
     };
     FirstBtn.addActionListener(Control);
     PrevBtn.addActionListener(Control);
     NextBtn.addActionListener(Control);
     LastBtn.addActionListener(Control);
     pdf_decoder.setExtractionMode(0,300,scale);
     ActionListener Magnifier = new ActionListener(){
         public void actionPerformed(ActionEvent evt){
            Object src = evt.getSource();
            if(src.equals(GBtn))
               scale=scale*SCALEFACTOR;
            if(src.equals(LBtn))
               scale=Math.max(0.01F,scale/SCALEFACTOR);
            pdf_decoder.setExtractionMode(0,300,scale);
            if(!dataAvailable){ 
               return;
            }
            try{
              CurrentPage.setImage(pdf_decoder.getPageAsImage(page));
              TextViewerFrame.this.invalidate();
              TextViewerFrame.this.validate();
              TextViewerFrame.this.repaint(); 
            }catch(Exception e){
               e.printStackTrace();
               CurrentPage.setImage(null);
               PDFPanel.this.repaint();
            }
         }};
     GBtn.addActionListener(Magnifier);
     LBtn.addActionListener(Magnifier);
}


private class PicturePanel extends JPanel{
         public void paint(Graphics g){
              if(image!=null)
                   g.drawImage(image,0,0,null);
         }
         public Dimension getPreferredSize(){
            return MyDimension;
         }          
         public Dimension getMinimumSize(){
            return MyDimension;
         }
         public void setImage(Image image){
             this.image = image;
             if(image!=null){
                MyDimension.width = image.getWidth(null);
                MyDimension.height = image.getHeight(null);
             }else{
                MyDimension.width=MyDimension.height=10;
             }
         }
         Image image;
         Dimension MyDimension = new Dimension();
     };

private PicturePanel CurrentPage= new PicturePanel();

public boolean setPdfData(byte[] data){
   try{
       pdf_decoder.openPdfArray(data);
       NumberOfPages = pdf_decoder.getPageCount();
       dataAvailable=true;
       CurrentPage.setImage(pdf_decoder.getPageAsImage(1));
       page = 1;
       return true;
   } catch(Exception e){
       NumberOfPages = -1;
       dataAvailable=false;
       return false;
   }
} 
private PdfDecoder pdf_decoder=new PdfDecoder();
private JScrollPane ScrollPane;
private JButton FirstBtn = new JButton("|<");
private JButton PrevBtn = new JButton("<");
private JButton NextBtn = new JButton(">");
private JButton LastBtn = new JButton(">|");
private JButton GBtn = new JButton("(+)");
private JButton LBtn = new JButton("(-)"); 
private JLabel  Pages  = new JLabel("# pages ");
private int NumberOfPages=-1;
private boolean dataAvailable = false;
private int page; // the number of the current page
private float scale=1.0F;
static final float SCALEFACTOR=1.2F;

}


private JEditorPane Display;
private JButton CloseBtn;
private Dspltext Source;
private JButton PlainBtn;
private JButton HtmlBtn;
private JButton RtfBtn;
private JButton PdfBtn;
private String TheText;
private static EditorKit EKPlain=null;
private static EditorKit EKHtml=null;
private static EditorKit EKRtf=null;
private static JScrollPane ScrollPane;
private static PDFPanel pdf_viewer;     // Panel for displaying pdf contents
private JPanel TextPanel = new JPanel();  // Panel for displaying plain text, html and rtf content
private boolean ISPDF = false;

}

}



