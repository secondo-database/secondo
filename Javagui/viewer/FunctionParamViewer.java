
package viewer;


import javax.swing.*;
import gui.*;
import sj.lang.ListExpr;
import java.awt.*;
import java.awt.event.*;
import tools.Reporter;


enum Type {STRING,INT,REAL,BOOL,TEXT,GENERIC};


public class FunctionParamViewer extends SecondoViewer{

  private JPanel paramPanel;
  private JButton retrieveFunctionBtn;
  private JComboBox functions; 

  private ListExpr objectList;
  private JPanel emptyPanel;
  private JPanel currentPanel;
  private ActionListener theActionListener;
  private String funName;




  public FunctionParamViewer(){
      paramPanel = new JPanel();
      retrieveFunctionBtn = new JButton("get Functions");
      functions = new JComboBox();

      setLayout(new BorderLayout());
      add(paramPanel, BorderLayout.CENTER);
      JPanel controlPanel = new JPanel();
      controlPanel.add(retrieveFunctionBtn);
      controlPanel.add(functions);
      retrieveFunctionBtn.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
            retrieveFunctions();
         }
      }); 
      add(controlPanel,BorderLayout.SOUTH);

      functions.addItemListener(new ItemListener(){
          public void itemStateChanged(ItemEvent evt){
             buildPanel();
          }             
      });

      emptyPanel = new JPanel();
      currentPanel = emptyPanel;
      paramPanel.add(currentPanel);

      theActionListener = new ActionListener(){
          public void actionPerformed(ActionEvent evt){
              doQuery();
          }
      };

      
  }


  private void doQuery(){
     int c = currentPanel.getComponentCount();
     String query = " query " + funName + "(";
     int params = 0;
     for(int i=0;i<c;i++){
         Component p = currentPanel.getComponent(i);
         if(p instanceof ParamPanel){
            ParamPanel pp = (ParamPanel) p;
            if(!pp.checkContent()){
                Reporter.showInfo("Invalid content for parameter " + pp.getName());
                return;
            }
            if(params>0){
               query += ", ";
            }
            query += pp.getParamString();
            params++;
         }
     } 
     query += ")";
     if(!VC.execUserCommand(query)){
         Reporter.showInfo("problem in query found");
     }
       
  }


  private void retrieveFunctions(){
     functions.removeAllItems();
     ListExpr funs = VC.getCommandResult("list objects");
     if(funs==null){
         Reporter.showError("Could not retrieve function. \n May be, no database is opened");
         return;
     }  
     objectList = funs.second().second().rest();
     ListExpr olist = objectList;
     while(!olist.isEmpty()){
         ListExpr o = olist.first();
         ListExpr name =  o.second();
         ListExpr type = o.fourth();
         if(type.listLength()>0 ){
           ListExpr mt = type.first();
           if(mt.listLength()>2){
              mt = mt.first();
              if(mt.atomType()==ListExpr.SYMBOL_ATOM && mt.symbolValue().equals("map")){
                 String n = name.toString();
                 functions.addItem(n); 
              }
           } 
         }
         olist=olist.rest();
     }
     functions.setSelectedIndex(0); 
  }

  private void buildPanel(){
     Object fno = functions.getSelectedItem();
     if(fno==null){
        return;
     }
     funName = functions.getSelectedItem().toString();
     ListExpr map = VC.getCommandResult("query " + funName);
     if(map==null){
        return;
     } 
     if(map.listLength()!=2){
        return;
     }
     ListExpr type = map.first();
     ListExpr fun = map.second();
     if(type.listLength()<2 || !isSymbol(type.first(),"map")){
        return;
     }
     if(fun.listLength()<2 || !isSymbol(fun.first(),"fun")){
        return;
     }
     JPanel completePanel = new JPanel();
     completePanel.setLayout(new BoxLayout(completePanel,BoxLayout.Y_AXIS));
     completePanel.setBorder(new javax.swing.border.TitledBorder("Function: "+ funName));
     fun = fun.rest(); // ignore fun keyword
     int len = fun.listLength();
     for(int i=0;i<len-1; i++){ // the last element is the return type of the function
         ListExpr param = fun.first();
         ParamPanel  p = createParamPanel(param);
         if(p!=null){
            completePanel.add(p);
         }    
         fun = fun.rest();
     }
     JButton queryBtn = new JButton("send query");
     queryBtn.addActionListener(theActionListener);
     completePanel.add(queryBtn);
     paramPanel.remove(currentPanel);
     paramPanel.add(completePanel);
     currentPanel = completePanel;
     validate();
  }


 
  public boolean addObject(SecondoObject o){ // this viewer display no objects
      return false;
  } 

  public boolean isDisplayed(SecondoObject o){ return false; }

  public void removeAll(){}
  public void removeObject(SecondoObject o){}

  public boolean canDisplay(SecondoObject o){ return false; }

  public String getName(){ return "FunctionParam"; }

  public boolean selectObject(SecondoObject o){ return false; }


  private ParamPanel createParamPanel(ListExpr param){
     if(param.listLength()!=2){
       return null;
     } else {
        return new ParamPanel(param.first(), param.second());
     }
  }

  public static boolean isSymbol(ListExpr list, String content){
       return list.atomType()==ListExpr.SYMBOL_ATOM && list.symbolValue().equals(content);
  }

 
  private class ParamPanel extends JPanel{

   

    private String name;
    private Type stype;
    private ListExpr exactType;

    private JLabel nameLabel;
    private JLabel typeLabel;
    private JTextField constField;
    private JComboBox dbObjects;
   

    public ParamPanel(ListExpr name, ListExpr type){
       super();
       this.name = name.symbolValue();
       this.exactType = type;
       if(isSymbol(exactType,"string")){
           this.stype = Type.STRING;
       } else if(isSymbol(exactType,"int")){
           this.stype=Type.INT;
       } else if(isSymbol(exactType,"real")){
           this.stype=Type.REAL;
       } else if(isSymbol(exactType,"bool")){
           this.stype=Type.BOOL;
       } else if(isSymbol(exactType,"text")){
           this.stype=Type.TEXT;
       } else {
           this.stype = Type.GENERIC;
       }
       setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
       nameLabel = new JLabel(this.name+" :   ");
       constField = new JTextField(20);
       dbObjects = new JComboBox();

        typeLabel = new JLabel("  ( "+ mainType(exactType) + ")");

       add(nameLabel);
       add(constField);
       add(dbObjects);
       add(typeLabel);

       dbObjects.addItem("const");
       ListExpr olist =  FunctionParamViewer.this.objectList;
       while(!olist.isEmpty()){
          ListExpr o = olist.first();
          olist= olist.rest();
          ListExpr otype = o.fourth();
          if(otype.first().equals(exactType)){
             dbObjects.addItem(o.second().toString());
          }
       }
       dbObjects.addItemListener(new ItemListener(){
          public void itemStateChanged(ItemEvent evt){
              changeObject();
          }
       });
       dbObjects.setSelectedIndex(0); 
    }

    private void changeObject(){
        String s = dbObjects.getSelectedItem().toString();
        if(s.equals("const")){
           constField.setEnabled(true);
        } else {
           constField.setEnabled(false);
        }
    }


 

    public String getParamString() {
        String s = dbObjects.getSelectedItem().toString();
        if(!s.equals("const")){
           return s;
        }
        String c = constField.getText();
        switch(stype){
           case STRING : c = c.replaceAll("\\\"","\\\\\""); return "\""+c+"\"";
           case INT    : return c.trim(); 
           case REAL   : return c.trim();
           case BOOL   : return  c.toUpperCase().trim(); 
           case TEXT   : c = c.replaceAll("'","\\\\'"); return "'"+c+"'";
           default     :  return "[const " + exactType+" value " + c+ "]";
        }
    }

    // enum Type {STRING,INT,REAL,BOOL,TEXT,GENERIC};
    public boolean checkContent(){
        if(!dbObjects.getSelectedItem().toString().equals("const")){
          return true;
        }
        String c = constField.getText();
        switch(stype){
           case STRING : return c.length() < 48;
           case INT    : try{Integer.parseInt(c.trim()); return true;} catch(Exception e){ return false; }
           case REAL   : try{Double.parseDouble(c.trim()); return true;} catch(Exception e){ return false; }
           case BOOL   : c = c.toUpperCase().trim(); return c.equals("TRUE") || c.equals("FALSE");
           case TEXT   : return true;
           default     : ListExpr l = new ListExpr();
                         return l.readFromString(c)==0; 
        }
    }

    String mainType(ListExpr l){
       while(l.atomType()!=ListExpr.SYMBOL_ATOM){
          l = l.first();
       }
       return l.toString();
    }



    public String getName() {
          return name;
    }

  }
 

}

