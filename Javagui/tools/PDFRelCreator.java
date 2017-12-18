

package tools;


import sj.lang.*;
import java.io.*;
import java.util.Stack;
import javax.swing.*;
import java.awt.event.*;
import java.awt.*;



public class PDFRelCreator extends JFrame{


private class ServerDialog extends JDialog{

public ServerDialog(JFrame owner){
   super(owner,"Server Settings", true);
   setSize(400,300);
   setLayout(new BorderLayout());
   JPanel settings = new JPanel();
   add(settings, BorderLayout.CENTER);
   settings.setLayout(new GridLayout(4,2));
   settings.add(new JLabel("Server"));
   hostField = new JTextField(hostName);
   settings.add(hostField);
   settings.add(new JLabel("Port"));
   portField = new JTextField(""+port);
   settings.add(portField);
   settings.add(new JLabel("User Name"));
   userField = new JTextField("");
   settings.add(userField);
   settings.add(new JLabel("Password"));
   pswdField = new JPasswordField("");
   settings.add(pswdField);

   resetBtn = new JButton("reset");
   applyBtn = new JButton("apply");
   JPanel panel2 = new JPanel();
   panel2.add(resetBtn);
   panel2.add(applyBtn);
   resetBtn.addActionListener( new ActionListener(){
      public void actionPerformed(ActionEvent e){
        ServerDialog.this.reset();
      }   
   });
   applyBtn.addActionListener( new ActionListener(){
      public void actionPerformed(ActionEvent e){
        ServerDialog.this.apply();
      }
   });
   add(panel2, BorderLayout.SOUTH); 

}  


  private void reset(){
     hostField.setText(hostName);
     portField.setText(""+port);
     changed = false;
  }

  private void apply(){
     changed = false;
     try {
        int p2 = Integer.parseInt(portField.getText().trim());
        changed = port!= p2;
        port = p2;
     } catch(NumberFormatException e){
        JOptionPane.showMessageDialog(this, "Invalid PortNumber", "Error", JOptionPane.ERROR_MESSAGE);
        return;
     }
     String hn = hostField.getText().trim();
     changed = changed || !hostName.equals(hn);
     hostName = hn;
     String us = userField.getText().trim();
     changed = changed || !us.equals(userName);
     userName = us;
     String ps = pswdField.getText();
     changed = changed || !ps.equals(password);
     password = ps; 
     setVisible(false);
  }

  boolean showMe(){
     setVisible(true); // modal
     return changed;
  }

  int getPort() { return port; }
  String getHost(){ return hostName; }
  String getUser() { return userName; }
  String getPassword() { return password; }

  private String hostName="localhost";
  private int port = 1234;
  private String userName="";
  private String password="";
  
  private JTextField hostField;
  private JTextField portField;
  private JTextField userField;
  private JTextField pswdField;
  
  private JButton resetBtn;
  private JButton applyBtn;
  private boolean changed = false;
}


private static ESInterface si = null;
private static IntByReference errorCode= new IntByReference();
private static IntByReference errorPos = new IntByReference();
private static ListExpr resultList= new ListExpr();
private static StringBuffer errorMessage = new StringBuffer();
private static boolean haltOnError = true;
private static BufferedReader in;

private ServerDialog serverDialog;
private JTextField relationName;
private JTextField databaseName;
private JFileChooser fileChooser;
private JTextField dirField;
JCheckBox subdirCB;


public PDFRelCreator(){
  setSize(680,420);
  serverDialog = new ServerDialog(this);
  si = new ESInterface();

  si.setHostname(serverDialog.getHost());
  si.setPort(serverDialog.getPort());

  si.setUserName("");
  si.setPassWd("");

  si.useBinaryLists(true); 

  tools.Environment.MEASURE_TIME=false; // supress some messages
  if(!si.connect()){
    JOptionPane.showMessageDialog(this,"error in connecting with server" );
    serverDialog.showMe();
  }

  this.addWindowListener(new WindowAdapter(){
       public void windowClosing(WindowEvent evt){
          closeMe(); 
       }    
  });

  setLayout(new GridLayout(6,1));
  JButton serverBtn = new JButton("Server-Settings");
  serverBtn.addActionListener(new ActionListener(){
    public void actionPerformed(ActionEvent e){
       changeServerSettings();
    }
  }); 
  JPanel serverPanel = new JPanel();
  serverPanel.add(serverBtn);
  add(serverPanel);

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

void closeMe(){
   si.terminate();
   System.exit(0);
}

void changeServerSettings(){
   if(serverDialog.showMe()){
       si.terminate();
       si.setHostname(serverDialog.getHost());
       si.setPort(serverDialog.getPort());
       si.setUserName(serverDialog.getUser());
       si.setPassWd(serverDialog.getPassword());

       if(!si.connect()){
         JOptionPane.showMessageDialog(this,"error in connecting with server" );
       }
   }
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
  if(!si.isConnected()){
     JOptionPane.showMessageDialog(this, "You are not connected to a Secondo server");
     return;
  }
  // check whether relation already exists and have the correct type
  String relName = relationName.getText().trim();
  if(!isSymbol(relName)){
     JOptionPane.showMessageDialog(this, "Invalid Name for relation");
     return;
  }
  String db = databaseName.getText();
  if(!isSymbol(db)){
     JOptionPane.showMessageDialog(this, "Invalid Name for database");
     return;
  }
  String cmd = "close database ";
  ListExpr resultList=ListExpr.theEmptyList();
  si.secondo(cmd,resultList, errorCode, errorPos, errorMessage);
  cmd = "open database " + db;
  si.secondo(cmd,resultList, errorCode, errorPos, errorMessage);
  if(errorCode.value!=0){
     JOptionPane.showMessageDialog(this, "problem in opening database "+db+"\n"+ errorMessage);
     return;
  }
  


  cmd = "query "+ relName + " getTypeNL ";
  si.secondo(cmd,resultList, errorCode, errorPos, errorMessage);

  File f = fileChooser.getSelectedFile();
  if(f==null || !f.isDirectory()){
     JOptionPane.showMessageDialog(this, "No directory selected");
     return;
  }

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
     si.secondo(cmd,resultList, errorCode, errorPos, errorMessage);
     if(errorCode.value != 0){
        JOptionPane.showMessageDialog(this, "Problem in creating relation\n" + errorMessage);
        return;
     }
  }
  boolean sd = subdirCB.isSelected();
  Stack<File> stack = new Stack<File>();
  File[] files = f.listFiles();
  insertFiles(stack,files,sd);

  System.out.println("insertFiles(1) " + stack.size());
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

              si.secondo(cmd,resultList, errorCode, errorPos, errorMessage);
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



public static void main(String[] args){

   PDFRelCreator pdfrelator = new PDFRelCreator(); 

}



}
