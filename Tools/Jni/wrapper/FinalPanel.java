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
import javax.swing.event.*;
import java.awt.event.*;
import javax.swing.filechooser.*;
import java.io.*;


/** Class for storing the Algebra */
public class FinalPanel extends JPanel{


/** Creates a new Panel from the given values */
public FinalPanel(String AlgebraName,
                  String AuthorName,
                  Class[] Classes,
                  ClassMethod[] Methods,
                  OperatorSpecification[] Specs,
                  TypeDescription[] properties){

    this.AlgebraName = AlgebraName;
    this.AuthorName = AuthorName;
    this.Classes = Classes;
    this.Methods = Methods;
    this.Specifications = Specs;
    this.properties = properties; 

    InfoField= new JTextArea();
    InfoField.append("    Final Step \n\n");
    InfoField.append(" All required information is available.\n ");
    InfoField.append(" Please Select a filename and press \"save\" ");
    InfoField.append(" to generate the Algebra ");
    JPanel FilePanel = new JPanel();
    FileNameField =  new JTextField(30);
    FileChooser = new JFileChooser("."){
          public boolean accept(File F){
             String N = F.getAbsolutePath();
             return N.endsWith(".cpp");
          }
    };
    FileNameField.setText(FileChooser.getCurrentDirectory().getAbsolutePath() + 
                          File.separator+AlgebraName+"Algebra.cpp");
    ChangeFileNameBtn = new JButton("change");
    ChangeFileNameBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
            if(FileChooser.showSaveDialog(null)==JFileChooser.APPROVE_OPTION){
               String NewName = FileChooser.getSelectedFile().getAbsolutePath();
               if(!NewName.endsWith(".cpp"))
                   NewName += ".cpp";
               FileNameField.setText(NewName);
            }
       }
    });
    FilePanel.add(FileNameField);
    FilePanel.add(ChangeFileNameBtn);
    add(InfoField);
    add(FilePanel);
    SaveBtn = new JButton("Save");
    SaveBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
             File F = new File(FileNameField.getText());
             if(F.exists()){
               int ov = JOptionPane.showConfirmDialog(null,"File already exists\n overwrite it ?",
                                              "Question",JOptionPane.YES_NO_OPTION);
               if(ov==JOptionPane.NO_OPTION)
                  return;
             }
             PrintStream out=null;
             try{
                out = new PrintStream(new FileOutputStream(F)); 
                AlgebraWriter.writeAlgebra(out,FinalPanel.this.AlgebraName,
                                               FinalPanel.this.AuthorName,
                                               FinalPanel.this.Classes,
                                               FinalPanel.this.Methods, 
                                               FinalPanel.this.Specifications,
                                               FinalPanel.this.properties);
                JOptionPane.showMessageDialog(null,"AlgebraFile written");
                
             }catch(Exception e){
                e.printStackTrace();
                JOptionPane.showMessageDialog(null,"Cannot write the File",
                                              "Error",JOptionPane.ERROR_MESSAGE);
             }finally{
                try{out.close();}catch(Exception e2){}
             }
        }

    }); 
    add(SaveBtn);
}



private JTextArea InfoField;
private JTextField FileNameField;
private JButton ChangeFileNameBtn;
private JButton SaveBtn;
private JFileChooser FileChooser; 
private String AlgebraName;
private String AuthorName;
private Class[] Classes;
private ClassMethod[] Methods;
private OperatorSpecification[] Specifications;
private TypeDescription[] properties;

}
