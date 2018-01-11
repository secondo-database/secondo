

package viewer;


import gui.SecondoObject;
import gui.ViewerControl;
import tools.Base64Encoder;

import sj.lang.*;
import java.io.*;
import java.util.Stack;
import javax.swing.*;
import java.awt.event.*;
import java.awt.*;




public class PDFRelCreator extends SecondoViewer {


private static IntByReference errorCode= new IntByReference();
private static IntByReference errorPos = new IntByReference();
private static ListExpr resultList= new ListExpr();
private static StringBuffer errorMessage = new StringBuffer();
private static boolean haltOnError = true;
private static BufferedReader in;

private JTextField relationName;
private JTextField databaseName;
private JFileChooser fileChooser;
private JTextField dirField;
private JCheckBox subdirCB;


public PDFRelCreator(){
  setSize(680,420);
  Frame mainFrame = VC==null?null:VC.getMainFrame();
  setLayout(new GridLayout(6,1));

  relationName = new JTextField(30);
  JPanel relNamePanel = new JPanel();
  relNamePanel.add(new JLabel("Name for relation"));
  relNamePanel.add(relationName);
  add(relNamePanel);
  
  databaseName = new JTextField(30);
  JPanel dbNamePanel = new JPanel();
  dbNamePanel.add(new JLabel("database"));
  dbNamePanel.add(databaseName);
  add(dbNamePanel);


  JPanel dirPanel = new JPanel();
  dirPanel.add(new JLabel("Directory"));
  fileChooser = new JFileChooser();
  fileChooser.setCurrentDirectory(new File("."));
  fileChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
  fileChooser.setAcceptAllFileFilterUsed(false);

  dirField = new JTextField(30);
  dirField.setEditable(false);
  JButton selectBtn = new JButton("select");
  selectBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent e){
        if(fileChooser.showOpenDialog(PDFRelCreator.this)==JFileChooser.APPROVE_OPTION){
           File f = fileChooser.getSelectedFile();
           dirField.setText(f.getAbsolutePath());
        }
     }
  }); 
  subdirCB = new JCheckBox("check subdirectories");
  JPanel filePanel = new JPanel();
  filePanel.add(new JLabel("Directory"));
  filePanel.add(dirField);
  filePanel.add(subdirCB);
  filePanel.add(selectBtn);
  add(filePanel); 

  JPanel startPanel = new JPanel();
  JButton startBtn = new JButton("start");
  startPanel.add(startBtn);
  add(startPanel);
  startBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent e){
        PDFRelCreator.this.start();
     }
   }); 

  setVisible(true);
}

boolean isChar(char c){
  return   (c=='_')
        || (c >='a' && c<='z')
        || (c >='A' && c<='Z');
}

boolean isCharOrDigit(char c){
  return isChar(c) || ( (c>='0') && (c<='9'));
}

boolean isSymbol(String value){
  if(value.length()==0) return false;
  if(!isChar(value.charAt(0))){
    return false;
  }
  for(int i=1;i<value.length();i++){
    if(!isCharOrDigit(value.charAt(i))){
       return false;
    }
  }
  return true;
}

ListExpr getRelType(){

  ListExpr attrList = ListExpr.twoElemList(
                        ListExpr.twoElemList( 
                            ListExpr.symbolAtom("File"),
                            ListExpr.symbolAtom("text")),
                        ListExpr.twoElemList( 
                            ListExpr.symbolAtom("Content"),
                            ListExpr.twoElemList(
                                  ListExpr.symbolAtom("document"),
                                  ListExpr.symbolAtom("pdf"))));

  return  ListExpr.twoElemList(
             ListExpr.symbolAtom("rel"),
             ListExpr.twoElemList(
                ListExpr.symbolAtom("tuple"),
                attrList));
}

boolean checkType(ListExpr type){
  ListExpr expected = getRelType();
  return expected.equals(type);
}

boolean checkTypeAsText(ListExpr theText){

  if(theText.listLength()!=2){
    return false;
  }
  theText = theText.second();
  if(theText.atomType()!=ListExpr.TEXT_ATOM){
    return false;
  }
  String text = theText.textValue();
  ListExpr type=ListExpr.theEmptyList();
  if(type.readFromString(text)!=0){
    return false;
  }
  return checkType(type);
}

private void insertFiles(Stack<File> stack, File[] files, boolean sd){
  for(int i=0;i<files.length;i++){
     if(files[i].isDirectory()) {
      if(sd){
        if(   !files[i].getName().equals(".") 
           && !files[i].getName().equals("..")){
          stack.push(files[i]);
        }
      }
     } else {
        String name = files[i].getName().toLowerCase();
        if(name.endsWith(".pdf")){
          stack.push(files[i]);
        }
     }
  }

}


String getBase64(String fileName){

 try{
   BufferedInputStream in = new BufferedInputStream(new FileInputStream(new File(fileName)));
   Base64Encoder encoder = new Base64Encoder(in);
   int r;
   StringBuffer res=new StringBuffer();
   while( (r = encoder.getNext()) > -1){
     char c = (char) r;
     res.append(c);
   }
   in.close();
   return  res.toString();
 } catch(Exception e){
    return null;
 }

}

void start(){
  if(VC==null){
     JOptionPane.showMessageDialog(this, "Internal error, ViewerControl not set");
     return;
  }

  // check whether relation already exists and have the correct type
  String relName = relationName.getText().trim();
  if(!isSymbol(relName)){
     JOptionPane.showMessageDialog(this, "Invalid Name for relation");
     return;
  }
  String cmd = "";
  String db = databaseName.getText().trim();
  if(db.length()>0){
    if(!isSymbol(db)){
       JOptionPane.showMessageDialog(this, "Invalid Name for database");
       return;
    }
    cmd = "close database "; // ignore error, may be no db open
    resultList = VC.getCommandResult(cmd);
    cmd = "create database " + db; // ignore error, may be database exists
    resultList = VC.getCommandResult(cmd);
    resultList = new ListExpr();
    cmd = "open database " + db;
    if(!VC.execCommand(cmd,errorCode,resultList,errorMessage)){
       JOptionPane.showMessageDialog(this, "problem in opening database "+db+"\n"+ errorMessage);
       return;
    }
  }

  File f = fileChooser.getSelectedFile();
  if(f==null || !f.isDirectory()){
     JOptionPane.showMessageDialog(this, "No directory selected");
     return;
  }

  cmd = "query "+ relName + " getTypeNL ";
  VC.execCommand(cmd,errorCode, resultList, errorMessage);

  boolean present = false;
  ListExpr noP = ListExpr.twoElemList( ListExpr.symbolAtom("text"),
                                       ListExpr.textAtom(relName));

  if(errorCode.value == 0){
     if(!noP.equals(resultList)){
       present = true;
     } 
     if(present){
       if(!checkTypeAsText(resultList)){
          JOptionPane.showMessageDialog(this, "Relation exists with different schema");
          return;
       } else {
          if(JOptionPane.showConfirmDialog(this, "Relation exists, append files?")!=JOptionPane.YES_OPTION){
             return;
          }
       }
    }
  }
  if(!present){ // create new Relation
     cmd = "let " + relName + " = [const rel(tuple([File : text, Content : document(pdf)])) value ()]";
     VC.execCommand(cmd, errorCode, resultList , errorMessage);
     if(errorCode.value != 0){
        JOptionPane.showMessageDialog(this, "Problem in creating relation\n" + errorMessage);
        return;
     }
  }
  boolean sd = subdirCB.isSelected();
  Stack<File> stack = new Stack<File>();
  File[] files = f.listFiles();
  insertFiles(stack,files,sd);

  int length = f.getAbsolutePath().length()+1;
  int count = 0;
  while(!stack.empty()){
    File top = stack.pop();
    if(top.isDirectory()){
       if(sd){
          insertFiles(stack, top.listFiles(),sd);
       }
    } else {
        String name = top.getAbsolutePath();
        if(name.startsWith(f.getAbsolutePath())){
           name = name.substring(length);
           System.out.println("insert file " + name + " of size " + top.length() + " bytes");
           String content = getBase64(top.getAbsolutePath());
           if(content!=null){
              name = name.replace("'"," ");
              cmd = "query " + relName + " inserttuple[ [const text value '"+name+"'],"
                  +                                    "[const document(pdf) value '"+content+"']] count";

              VC.execCommand(cmd,errorCode, resultList, errorMessage);
              if(errorCode.value != 0){
                 JOptionPane.showMessageDialog(this, "Problem  relation update\n" + errorMessage);
                 System.out.println("name = " + name); 
                 return;
              } else {
                 count++;
              }
           } else {
             System.err.println("problem in reading file " + top);
           }
        }
    }
  }
  JOptionPane.showMessageDialog(this, "Inserted " + count + " files");

}

// some dummy implementations to work as an SecondoViewer
// because this viewer display nothing, these methods do
// no senseful things

public boolean selectObject(SecondoObject O){
  return false;
}

public boolean isDisplayed(SecondoObject o){
  return false;
}

public boolean canDisplay(SecondoObject o){
  return false;
}

public void removeAll(){

}

public void removeObject(SecondoObject o){

}

public boolean addObject(SecondoObject o){
  return false;
}

public String getName(){
  return "PDF-Relation-Creator";
}


 



}
