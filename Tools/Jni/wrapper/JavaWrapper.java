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

package wrapper;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.lang.reflect.*;
import java.io.*;


/** This class provides a graphical user interface for
  * creating a SECONDO algebra from a set of java classes
  **/
public class JavaWrapper extends JFrame{

/** Creates a new instance of the JavaWrapper **/
public JavaWrapper(){

   // add Listener to shut down if window is closing
   addWindowListener(new WindowAdapter(){
       public void windowClosing(WindowEvent evt){
            System.err.println("Bye");
            System.exit(0);
       }});
 
   // set the initial size
   setSize(640,480);
   CC = new ClassChooser();
   getContentPane().setLayout(new BorderLayout());
   getContentPane().add(CC,BorderLayout.CENTER);
   CurrentPanel = CC;
   JPanel ControlPanel = new JPanel();
   ControlPanel.add(PrevBtn);
   PrevBtn.setEnabled(false);
   ControlPanel.add(NextBtn);
   getContentPane().add(ControlPanel,BorderLayout.SOUTH);
   NextBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent et){
         if(CurrentPanel == CC){
             if(!CC.check())
                return;
             
             Class[] TypeClasses = AlgebraWriter.getWrapableClasses(CC.getClasses());
             System.err.println("found "+TypeClasses.length+" TypeClasses");
             if(TypeClasses.length>0){ 
                String[] ClassNames = new String[TypeClasses.length];
                for(int i=0;i<TypeClasses.length;i++)
                    ClassNames[i] = AlgebraWriter.getSecondoType(TypeClasses[i]);
                if(TypeDescPanel==null)
                    TypeDescPanel= new TypeDescriptionPanel(ClassNames);
                else
                    TypeDescPanel.set(ClassNames);
                setCurrentPanel(TypeDescPanel);
              } else{ // only static operators
                 ClassMethod[] CMs = AlgebraWriter.getWrapableMethods(CC.getClasses(),true);
                 if(CMs.length==0){
                      showMessage("No operators found ");
                      return; 
                 } else{ // show the Panel to select operators
                     OC = new OperationChooser(CC.getClasses());
                     setCurrentPanel(OC); 
                 }
              }
        } else if(CurrentPanel==TypeDescPanel){ 
             OC = new OperationChooser(CC.getClasses());
             TypeDescPanel.finish();
             getContentPane().remove(CC);
             setCurrentPanel(OC);
         }else
         if(CurrentPanel == OC && OC.Wrap.getClassMethods().length>0){
           String[] OperatorNames = AlgebraWriter.getOperatorNames(OC.Wrap.getClassMethods());
           SpecPanel = new SpecificationPanel(OperatorNames);
           setCurrentPanel(SpecPanel);
         }else
         if(CurrentPanel == SpecPanel || CurrentPanel==OC){
              String AlgebraName = CC.getAlgebraName();
              String AuthorName = "JavaWrapper"; 
              ClassMethod[] Ms = OC.Wrap.getClassMethods();
              Class[] Cs = CC.getClasses(); 
              TypeDescription[] Td=null; 
              if(TypeDescPanel!=null) // only static classes
                  Td = TypeDescPanel.getDescs();
              OperatorSpecification[] Specs;
              if(CurrentPanel==SpecPanel){
                   SpecPanel.finish();
                   Specs  = SpecPanel.getSpecs();
              }
              else
                   Specs = null;
              Final = new FinalPanel(AlgebraName,AuthorName,Cs,Ms,Specs,Td);           
              setCurrentPanel(Final); 
         }


       }
   });
}

/** Sets the current panel to display.
  * The JavaWrapper contains a set of panels for
  * selecting Classes and methods to be wrapped. 
  * By calling this function we can switch between the
  * different panels.
  */
private void setCurrentPanel(JPanel P){
   getContentPane().remove(CurrentPanel);
   getContentPane().add(P,BorderLayout.CENTER);
   CurrentPanel=P; 
   invalidate();
   validate();
   repaint();
}


/** The main function.
  * Creates a new instance of the javawrapper and displays it on screen.
  **/
public static void main(String[] args){
  JavaWrapper JW = new JavaWrapper();
  JW.setVisible(true); 
}


/** Shows a message on screen **/
static void showMessage(String Message){
    JOptionPane.showMessageDialog(null,Message);
}


private JPanel CurrentPanel;
private ClassChooser CC;
private TypeDescriptionPanel TypeDescPanel;
private OperationChooser OC;
private SpecificationPanel SpecPanel=null;
private JButton PrevBtn = new JButton("Prev");
private JButton NextBtn = new JButton("Next");
private FinalPanel Final;

/** The class for selecting operations */
private class OperationChooser extends JPanel{

/** Creates a OperationChooser from a given set of classes **/
public OperationChooser(Class[] theClasses){
    this.theClasses = theClasses;
    // build the content of this panel
    JPanel FilterPanel = new JPanel();
    InheritedCB.setSelected(false);
    FilterPanel.add(InheritedCB); 
    JPanel AvailablePanel  = new JPanel(new BorderLayout());
    JPanel ControlPanel = new JPanel();
    JPanel WrapPanel = new JPanel(new BorderLayout());
    ControlPanel.add(AddBtn);
    ControlPanel.add(RemoveBtn);
    AvailablePanel.add(new JLabel("Available"),BorderLayout.NORTH);
    WrapPanel.add(new JLabel("Wrap"),BorderLayout.NORTH);
    setLayout(new BorderLayout());
    add(FilterPanel,BorderLayout.NORTH);
    JPanel TablePanel = new JPanel(new GridLayout(1,2));
    TablePanel.add(AvailablePanel);
    TablePanel.add(WrapPanel);
    add(FilterPanel,BorderLayout.NORTH);
    add(TablePanel,BorderLayout.CENTER);
    add(ControlPanel,BorderLayout.SOUTH); 

    Available = new MethodTableModel();
    AvailableTable = new JTable (Available);
    fillAvailable();
    JScrollPane ASP = new JScrollPane(AvailableTable);
    AvailablePanel.add(ASP,BorderLayout.CENTER);

    Wrap = new MethodTableModel();
    WrapTable = new JTable(Wrap);
    JScrollPane WSP = new JScrollPane(WrapTable);
    WrapPanel.add(WSP,BorderLayout.CENTER);

    // add function to the add button
    AddBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           int[] Selected = AvailableTable.getSelectedRows();
           ClassMethod[] Ms = Available.getClassMethodsAt(Selected);
           Wrap.addClassMethods(Ms);  
       }
    });

    RemoveBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          int[] Selected = WrapTable.getSelectedRows();
          Wrap.removeIndexes(Selected); 
       }
    });

    InheritedCB.addChangeListener(new ChangeListener(){
        public void stateChanged(ChangeEvent evt){
           fillAvailable();
        }
    });


}

/** fills the table of available methods **/
private void fillAvailable(){
    boolean useInherited = InheritedCB.isSelected();
    ClassMethod[] CMs = AlgebraWriter.getWrapableMethods(theClasses,useInherited);
    Available.setContent(CMs);
}

/** Checks whether this method can be wrapped
  * this means that arguments and returntypes can
  * only be primitive types or classes which are
  * currently wrapped.
  **/

private boolean checkMethod(Method M){
  return AlgebraWriter.canBeWrapped(M);
}


/** returns true if the class is contained in theClasses or 
  *  Class is primitive and class is not an array 
  */
private boolean checkClass(Class c){
   return AlgebraWriter.canBeWrapped(c);
}


/** Checks whether C is contained in the managed classes. **/
private boolean contains(Class c){
    for(int i=0;i<theClasses.length;i++)
        if(c.equals(theClasses[i]))
              return true;
    return false;
}





Class[] theClasses;
JCheckBox InheritedCB = new JCheckBox("Inherited");
JButton AddBtn = new JButton("Add");
JButton RemoveBtn = new JButton("Remove");
MethodTableModel Available;
JTable AvailableTable;
JTable WrapTable;
MethodTableModel Wrap;

/** A class for displaying a set of methods in a table. */
private class MethodTableModel implements TableModel{

  /** Creates a new instance of this. */
  public MethodTableModel(){
      TME = new TableModelEvent(this);
  } 

  public void addTableModelListener(TableModelListener l){
     if(!listener.contains(l))
        listener.add(l);
  }
 
  public Class getColumnClass(int columnIndex){
     return "".getClass();
  }

  public int getColumnCount(){
      return 2;
  }
 
  public String getColumnName(int columnIndex){
      if(columnIndex==0)
           return "class";
      return "method";
  }
  
  public int getRowCount(){
      return classmethods.size();
  }
 
  public Object getValueAt(int rowIndex, int columnIndex){
      if(columnIndex<0 || columnIndex > 1)
         return null;
      if(rowIndex<0 || rowIndex>=classmethods.size())
         return null;
      String res;
      if(columnIndex == 0){ // the class
          Class c = ((ClassMethod)classmethods.get(rowIndex)).c;
          res = AlgebraWriter.getShortString(c);
      }
      else // the method
          res = AlgebraWriter.getShortString(((ClassMethod)classmethods.get(rowIndex)).m);
            
      return res;
  }
 
  public boolean isCellEditable(int rowIndex, int columnIndex){
      return false;
  }
  
  public void removeTableModelListener(TableModelListener l){
       listener.remove(l);
  }
  public void 	setValueAt(Object aValue, int rowIndex, int columnIndex){}

   /** Informs all registered listeners about changes in the table */
   private void informListeners(){
      for(int i=0;i<listener.size();i++)
          ((TableModelListener)listener.get(i)).tableChanged(TME);
   }


   /** Adds a new method to this table **/
   public void addClassMethod(ClassMethod M){
      if(!classmethods.contains(M)){
         classmethods.add(M);
         informListeners();
      }
   }

   /** Adds a set of methods */
   public void addClassMethods(ClassMethod[] Ms){
       int count=0;
       if(Ms==null) return;
       for(int i=0;i<Ms.length;i++)
          if(!classmethods.contains(Ms[i])){
              classmethods.add(Ms[i]);
              count++;
          }
       informListeners();
   }
 
   /** Returns the ClassMethods at the specified positions. */
   public ClassMethod[] getClassMethodsAt(int[] positions){
      ClassMethod[] Res = new ClassMethod[positions.length];
      for(int i=0;i<positions.length;i++)
            Res[i] = (ClassMethod) classmethods.get(positions[i]);
      return Res; 
   }

   /** Returns all contained ClassMethods. */
   public ClassMethod[] getClassMethods(){
      ClassMethod[] res = new ClassMethod[classmethods.size()];
      for(int i=0;i<res.length;i++)
          res[i] = (ClassMethod) classmethods.get(i);
      return res;
   } 


   /** Sets the content of this table */
   public void setContent(Vector Ms){
      classmethods.clear();
      if(Ms==null){
          informListeners();
          return;
      } 
      for(int i=0;i<Ms.size();i++){
         Object obj = Ms.get(i);
         if(obj instanceof ClassMethod)
            classmethods.add(obj);
      }
      informListeners();
   }

   /** Sets the content of this table */
   public void setContent(ClassMethod[] ms){
      classmethods.clear();
      if(ms==null){
         informListeners();
         return;
      }
      for(int i=0;i<ms.length;i++)
         classmethods.add(ms[i]);
       informListeners();
   }


   /** Removes the entries at the spicified positions. */
   public void removeIndexes(int[] set){
       //sort(set);
       for(int i=0;i<set.length;i++){
           classmethods.remove(set[set.length-1-i]);
       }
       informListeners(); 
   }  

private Vector listener = new Vector();
private Vector classmethods = new Vector();
private TableModelEvent TME;

}
}








/** The class for including classes into the Algebra **/

private class ClassChooser extends JPanel{

/**
  Creates a new instance of ClassChooser.
**/
public ClassChooser(){
   Classes = new JTable(CT);
   JPanel ControlPanel = new JPanel(new GridLayout(4,2));
   ControlPanel.add(new JLabel("Name of the algebra :"));
   ControlPanel.add(AlgebraName);
   ControlPanel.add(new JLabel("package"));
   ControlPanel.add(new JLabel("class"));
   ControlPanel.add(NewPackage);
   ControlPanel.add(NewClass);
   ControlPanel.add(AddBtn);
   ControlPanel.add(RemoveBtn);
   setLayout(new BorderLayout());
   add(ControlPanel,BorderLayout.NORTH);
   add(Classes,BorderLayout.CENTER); 

   AddBtn.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent evt){
               String FullQualifiedName = NewPackage.getText();
               if(FullQualifiedName.length()>0)
                  FullQualifiedName +=".";
               FullQualifiedName += NewClass.getText();
               try{
                  Class Res =  Class.forName(FullQualifiedName);
                  // add here the check whether this class implements AlgebraType
                  if(!AlgebraWriter.canBeWrapped(Res)){
                     showMessage("Class not fulfil the preconditions");
                  }else
                     CT.add(Res);
               }catch (Exception e){
                  showMessage("cannot load class");
               } 
            }});
   RemoveBtn.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent evt){
               CT.remove(Classes.getSelectedRow());
            }});

}

/** Returns the selected classes. */
public Class[] getClasses(){
    return CT.getClasses();
}

/** Returns the name of the Algebra. */
public String getAlgebraName(){
    return AlgebraName.getText();
}

/** This method checks whether all required information is given.
    If information is missing a appropriate Dialog is showed an screen.
 */ 
public boolean check(){
   if(AlgebraName.getText().trim().length()<1){
        showMessage("Algebra name is missing");
        return false;
   }
   if(CT.numberOfClasses()<1){
       showMessage("No class specified");
       return false;
   }
   StringBuffer Twice = new StringBuffer();
   if(!AlgebraWriter.unique(CT.getClasses(),Twice)){
        showMessage("name conflikt in classes\n"+Twice);
        return false;
   }
   return true;
}



private ClassTable CT = new ClassTable();
private JTable Classes;
private JButton AddBtn= new JButton("add");
private JButton RemoveBtn = new JButton("remove");
private JTextField NewClass = new JTextField(30);
private JTextField NewPackage = new JTextField(30);
private JTextField AlgebraName = new JTextField(20);

/**
This class provides a table containing all classes 
which to want to wrap.
*/
private class ClassTable implements TableModel{
   /** Returns the count of contained classes. */
   int numberOfClasses(){
      return classes.size();
   }

   /** Returns all selected Classes */
   Class[] getClasses(){
     Class[] res = new Class[classes.size()];
     for(int i=0;i<classes.size();i++)
        res[i] = (Class) classes.get(i);
     return res;
    }

   /**
    Creates a new Instance.
   */
   public ClassTable(){
      TME = new TableModelEvent(this);
   }

   /** Adds a TableModelListener */
   public void addTableModelListener(TableModelListener l){
      Listener.add(l);
   }

   /*
    Returns the class of class.
   */ 
   public  Class getColumnClass(int columnIndex){return new Object().getClass().getClass(); }

   
   /**
    * Returns 1;
   **/
   public int getColumnCount() {return 1;}

   /** Returns the name of the column "Class"
   */
   public  String getColumnName(int columnIndex) {
      if(columnIndex!=0)
          return null;
      else 
          return "Classes";
    }

    /** Return the number of entries */
    public int getRowCount(){return classes.size();}
 
    /** Returns the entry at the specidied position */ 
    public  Object getValueAt(int rowIndex, int columnIndex) {
          if(columnIndex!=0)
               return null;
          if(rowIndex<0 | rowIndex>=classes.size())
               return null;
          return classes.get(rowIndex);
     }
     
     /** Returns false for all arguments */
     public   boolean isCellEditable(int rowIndex, int columnIndex) {
         return false;
     }

     /** Removes the TableModelListener l.
     */
     public  void removeTableModelListener(TableModelListener l) {
       Listener.remove(l);
     }

     /** Makes nothing because we only allow to change the table via
       * add and remove.
     */ 
     public void setValueAt(Object aValue, int rowIndex, int columnIndex) {}

     
     /** Informs all registered Listeners about changes in the table.
     */
     private void informListeners(){
         for(int i=0;i<Listener.size();i++)
            ((TableModelListener)Listener.get(i)).tableChanged(TME);
     }    

     
     /** Adds a new entry into the table **/
     public void add(Class C){
         if(!classes.contains(C))
            classes.add(C);
         informListeners();
     }


     /** Removes a class from the table.
      **/
      public void remove(Class C){
          int index = classes.indexOf(C);
          if(index<0) return;
          classes.remove(C);
          informListeners();
      }

      /** Removes the entry at the specified position **/
      public void remove(int index){
          if(index<0 || index >=classes.size())
            return;
          classes.removeElementAt(index);
          informListeners();
      }


     Vector classes = new Vector();
     Vector Listener = new Vector();
     TableModelEvent TME;
}

}


}
