
//This file is part of SECONDO.

//Copyright (C) 2004-2006, University in Hagen, Faculty of Mathematics and Computer Science, 
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


package tools;

import sj.lang.ListExpr;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import viewer.hoese.algebras.periodic.Time;
import viewer.hoese.algebras.Dsplpmpoint;
import java.text.*;
import javax.swing.event.*;
import javax.swing.border.*;
import java.util.Vector;

import java.io.*;


public class DisturberGui extends JFrame{

private JTabbedPane tabbedPane;

// gui components and default values for disturbation
private Component disturbSettings;
private JTextField timeDiffTF;
private double timeDiffDefault = 1; 
private JTextField maxErrorTF;
private double maxErrorDefault = 10;
private JTextField maxErrorDiffTF;
private double maxErrorDiffDefault = 1;
private JRadioButton relErrorRB;
private boolean relErrorDefault = false;
private JTextField changeErrorProbTF;
private double changeErrorProbDefault = 1;
private JTextField maxMeasuresTF;
private int maxMeasuresDefault = 2000;    // change it to zero !!!
private JTextField maxSequenceTF;
private int maxSequenceDefault = 50;
private JTextField removeProbTF;
private double removeProbDefault = 0.01;
private JTextField removeSeqProbTF;
private double removeSeqProbDefault = 0.0001;
private JTextField writerToleranceTF;
private double writerToleranceDefault = 0;


// gui components and default values for delay
private Component delaySettings;
private JTextField eventProbTF;
private double eventProbDefault = 0.01;
private JTextField stopProbValueTF;
private JSlider stopProbSl;
private int stopProbDefault = 50;
private JTextField maxDelayTF;
private int maxDelayDefault = 2000;
private JSlider minDecSl;
private JTextField minDecValueTF;
private int minDecDefault = 1; // in percent
private JSlider maxDecSl;
private JTextField maxDecValueTF;
private int maxDecDefault = 90; // percent
private JSlider minAccSl;
private JTextField minAccValueTF;
private int minAccDefault = 1;
private JSlider maxAccSl;
private JTextField maxAccValueTF;
private int maxAccDefault = 20;


// gui components for the log area
private Component logTab;
private JTextArea logArea;
private JButton clearLogBtn;
private ScrollListener scrollListener;

// components for the IO tab
private Component IOTab;
private JTextField inFileTF;
private JButton selectInFileBtn;
private JTextField outFileTF;
private JButton selectOutFileBtn;
private JButton startBtn;

private JFileChooser fcIn;
private JFileChooser fcOut;


private PrintStream log = System.err;

private PointDataDisturber pdd;


private NumberFormat nf = new DecimalFormat("#.####", new DecimalFormatSymbols(java.util.Locale.UK));

/** creates a new Disturbergui **/
public DisturberGui(){
  super("Moving Point Data Disturber");
  pdd = new PointDataDisturber();
  setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
  setSize(640,480); // standardSize       
  createMenuBar();
  tabbedPane = new JTabbedPane();
  addTabs(tabbedPane);
  getContentPane().add(tabbedPane); 

  log = new Logger(logArea,scrollListener);
  log.println("gui started");
  pdd.setLog(log);    
}


/** creates the main menu **/
private void createMenuBar(){
   JMenuBar mainMenu= new JMenuBar();
   JMenu fileMenu = new JMenu("File");
   mainMenu.add(fileMenu);
   JMenuItem exitMenu = new JMenuItem("Exit");
   fileMenu.add(exitMenu);

   exitMenu.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         DisturberGui.this.setVisible(false);
         System.exit(0);
      }
   });

   setJMenuBar(mainMenu);
}

/** adds the tabs to the given tabbed pane **/
private void addTabs(JTabbedPane tP){
  IOTab = getInOutComponent();
  disturbSettings = getDisturbComponent();
  delaySettings = getDelayComponent();
  logTab = getLogComponent();
  tP.addTab("Input / Output", IOTab);
  tP.addTab("Disturbation", disturbSettings);
  tP.addTab("Delay", delaySettings);
  tP.addTab("Log", logTab); 
}

private Component getInOutComponent(){
   GridBagLayout gridBag = new GridBagLayout();
   GridBagConstraints c = new GridBagConstraints();
   c.weightx = 1.0;
   c.fill = GridBagConstraints.BOTH;
   JPanel panel = new JPanel(gridBag);

   JLabel inLabel = new JLabel("Input File");
   gridBag.setConstraints(inLabel,c);
   panel.add(inLabel);
   inFileTF = new JTextField(30);
   gridBag.setConstraints(inFileTF,c);
   panel.add(inFileTF);
   selectInFileBtn = new JButton("select"); 
   gridBag.setConstraints(selectInFileBtn,c);
   panel.add(selectInFileBtn); 

   c.gridy=2;
   JLabel outLabel = new JLabel("Output File");
   gridBag.setConstraints(outLabel,c);   

   panel.add(outLabel);


   outFileTF = new JTextField(30);
   gridBag.setConstraints(outFileTF,c);
   panel.add(outFileTF);
   selectOutFileBtn = new JButton("select"); 
   gridBag.setConstraints(selectOutFileBtn,c);
   panel.add(selectOutFileBtn); 
  
   c.gridy = 3; 
   c.gridx = 1;
   c.fill = GridBagConstraints.CENTER;
   startBtn = new JButton("Start");
   gridBag.setConstraints(startBtn,c);
   panel.add(startBtn);

   fcIn = new JFileChooser(".");
   fcOut = new JFileChooser(".");
   selectInFileBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
          if( (new File(inFileTF.getText())).exists()){
              fcIn.setSelectedFile(new File(inFileTF.getText()));
          }
          if(fcIn.showOpenDialog(DisturberGui.this)==JFileChooser.APPROVE_OPTION){
             inFileTF.setText(fcIn.getSelectedFile().toString());
          }
      }
   });

   selectOutFileBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         if( (new File(outFileTF.getText())).exists()){
              fcOut.setSelectedFile(new File(outFileTF.getText()));
         }
         if(fcOut.showSaveDialog(DisturberGui.this)==JFileChooser.APPROVE_OPTION){
            outFileTF.setText(fcOut.getSelectedFile().toString());
         }
      }
   });

   startBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         DisturberGui.this.startConversion();
      }
   });


   JPanel IOPanel = new JPanel();
   IOPanel.add(panel);
   return IOPanel;
}

/** creates the component for the disturbation tab and sets the
  * values to the defaults 
  **/
private Component getDisturbComponent(){
   JPanel panel =  new JPanel();
   panel.setLayout(new GridLayout(10,2));
   
  // build the single components
   JLabel timeDiffLabel = new JLabel("time difference");
   panel.add(timeDiffLabel);
   
   timeDiffTF = new JTextField(20);
   panel.add(timeDiffTF);

   JLabel maxErrorLabel = new JLabel("maximum error (>=0)");
   panel.add(maxErrorLabel);

   maxErrorTF = new JTextField(20);
   panel.add(maxErrorTF);

   JLabel maxErrorDiffLabel = new JLabel("maximum error difference");
   panel.add(maxErrorDiffLabel);

   maxErrorDiffTF = new JTextField(20);
   panel.add(maxErrorDiffTF);

   relErrorRB = new JRadioButton("relative error");
   panel.add(relErrorRB);

   panel.add(new JLabel(""));  // dummy


   JLabel changeErrorProbLabel = new JLabel("pobability to change the error");
   panel.add(changeErrorProbLabel);
   changeErrorProbTF = new JTextField(20);
   panel.add(changeErrorProbTF);


   JLabel maxMeasuresLabel = new JLabel("maximum measures");
   panel.add(maxMeasuresLabel);
 
   maxMeasuresTF = new JTextField(20);
   panel.add(maxMeasuresTF);

   JLabel maxSequenceLabel = new JLabel("maximum sequence");
   panel.add(maxSequenceLabel);

   maxSequenceTF = new JTextField(20);
   panel.add(maxSequenceTF);

   JLabel removeProbLabel = new JLabel("remove probability");
   panel.add(removeProbLabel);

   removeProbTF = new JTextField(20);
   panel.add(removeProbTF);

   JLabel removeSeqProbLabel = new JLabel("remove sequence probability");
   panel.add(removeSeqProbLabel);

   removeSeqProbTF = new JTextField(20);
   panel.add(removeSeqProbTF);

   JLabel writerToleranceLabel = new JLabel("unit writer tolerance");
   panel.add(writerToleranceLabel);

   writerToleranceTF = new JTextField(20);
   panel.add(writerToleranceTF);


   setDisturbDefaults();

   JPanel res = new JPanel();
   res.add(panel);
   return res;
}

private Component getDelayComponent(){
   JPanel panel = new JPanel(new GridLayout(7,2));
   
   JLabel eventProbLabel = new JLabel("event probability");
   eventProbTF = new JTextField(20);
   panel.add(eventProbLabel);
   panel.add(eventProbTF);
   
   JLabel stopProbLabel = new JLabel("stop probability (%)");
   stopProbSl = new JSlider(0,100,50);
   stopProbSl.setPaintLabels(true);
   stopProbValueTF = new JTextField(3);
   stopProbValueTF.setEditable(false);
   stopProbValueTF.setBorder(new EmptyBorder(0,0,0,0));
   stopProbValueTF.setText(" "+stopProbSl.getValue());
   JPanel p1 = new JPanel();
   p1.add(stopProbSl);
   p1.add(stopProbValueTF);
   panel.add(stopProbLabel);
   panel.add(p1);

   JLabel maxDelayLabel = new JLabel("maximum delay (sec)");
   maxDelayTF = new JTextField(20);
   panel.add(maxDelayLabel);
   panel.add(maxDelayTF);

   JLabel minDecLabel = new JLabel("minimum deceleration (%)");
   minDecSl = new JSlider(1,99,10);
   panel.add(minDecLabel);
   JPanel p2 = new JPanel();
   minDecValueTF = new JTextField(""+minDecSl.getValue(),3);
   minDecValueTF.setEditable(false);
   minDecValueTF.setBorder(new EmptyBorder(0,0,0,0));
   p2.add(minDecSl);
   p2.add(minDecValueTF);
   panel.add(p2);

   JLabel maxDecLabel = new JLabel("maximum deceleration (%)");
   maxDecSl = new JSlider(1,99,80);
   panel.add(maxDecLabel);
   JPanel p3 = new JPanel();
   maxDecValueTF = new JTextField(""+maxDecSl.getValue(),3);
   maxDecValueTF.setEditable(false);
   maxDecValueTF.setBorder(new EmptyBorder(0,0,0,0));
   p3.add(maxDecSl);
   p3.add(maxDecValueTF);
   panel.add(p3);

   JLabel minAccLabel = new JLabel("minimum acceleration (%)");
   minAccSl = new JSlider(1,99,1);
   panel.add(minAccLabel);
   JPanel p4 = new JPanel();
   minAccValueTF = new JTextField(""+minAccSl.getValue(),3);
   minAccValueTF.setEditable(false);
   minAccValueTF.setBorder(new EmptyBorder(0,0,0,0));
   p4.add(minAccSl);
   p4.add(minAccValueTF);
   panel.add(p4);

   JLabel maxAccLabel = new JLabel("maximum acceleration (%)");
   maxAccSl = new JSlider(1,99,20);
   panel.add(maxAccLabel);
   JPanel p5 = new JPanel();
   maxAccValueTF = new JTextField(""+maxAccSl.getValue(),3);
   maxAccValueTF.setEditable(false);
   maxAccValueTF.setBorder(new EmptyBorder(0,0,0,0));
   p5.add(maxAccSl);
   p5.add(maxAccValueTF);
   panel.add(p5);

   // create a value listener for all sliders
   ChangeListener vcl = new ChangeListener(){
      public void stateChanged(ChangeEvent evt){
         Object src1 = evt.getSource();
         if(!(src1 instanceof JSlider)){
            log.println("wrong source for change listener");
            return;
         }
         JSlider src = (JSlider) src1;
         if(src==stopProbSl){
             String l = ""+src.getValue();
             stopProbValueTF.setText(l);
             return;
         }

         if(src==minDecSl){
            String l = ""+src.getValue();
            minDecValueTF.setText(l);
            if(src.getValue()>maxDecSl.getValue()){
               maxDecSl.setValue(src.getValue());
            }
            return;
         }
         if(src==maxDecSl){
            String l = ""+src.getValue();
            maxDecValueTF.setText(l);
            if(src.getValue()<minDecSl.getValue()){
               minDecSl.setValue(src.getValue());
            }
            return;
         }
         if(src==minAccSl){
            String l = ""+src.getValue();
            minAccValueTF.setText(l);
            if(src.getValue()>maxAccSl.getValue()){
               maxAccSl.setValue(src.getValue());
            }
            return;
         }
         if(src==maxAccSl){
            String l = ""+src.getValue();
            maxAccValueTF.setText(l);
            if(src.getValue()<minAccSl.getValue()){
               minAccSl.setValue(src.getValue());
            }
            return;
         }
      }

   };
   stopProbSl.addChangeListener(vcl);
   minDecSl.addChangeListener(vcl);
   maxDecSl.addChangeListener(vcl);
   minAccSl.addChangeListener(vcl);
   maxAccSl.addChangeListener(vcl);

   
   JPanel res = new JPanel();
   res.add(panel);   

   setDelayDefaults();
   return res;
}

private Component getLogComponent(){
   logArea= new JTextArea();
   logArea.setEditable(false);
   JScrollPane sp = new JScrollPane(logArea);
   scrollListener = new ScrollListener(sp);
   JPanel panel = new JPanel(new BorderLayout());
   panel.add(sp,BorderLayout.CENTER);
   clearLogBtn = new JButton("clear");
   JPanel p2 = new JPanel();
   p2.add(clearLogBtn);
   panel.add(p2,BorderLayout.SOUTH);
   clearLogBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          if(JOptionPane.showConfirmDialog(
                 DisturberGui.this,
                  "This will clean the complete log \n" +
                  "it's not possible to reconstruct it \n\n"+
                  "Are you want to continue ?",
                  " Continue ?",
                   JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION){
               return;

          }
          if(JOptionPane.showConfirmDialog(
                 DisturberGui.this,
                  "You are sure ? ",
                   " sure ? ",
                   JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION){
               return;

          }
          if(JOptionPane.showConfirmDialog(
                 DisturberGui.this,
                  "You are really sure ? \n",
                   "really sure ?", 
 
                   JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION){
               return;
          }
          JOptionPane.showMessageDialog(
                    DisturberGui.this,
                    "ok, log will be deleted","message", JOptionPane.INFORMATION_MESSAGE);
          logArea.setText("");

                   
                                    
       }
   });

   return panel;
}


/** Sets the data disturber and the corresponding gui elements 
  * to the default values.
  * Its assumed that all default values are valid. 
  */
private void setDisturbDefaults(){
   pdd.setTimeDiff(new Time(0,(int)(timeDiffDefault*1000)));
   timeDiffTF.setText(nf.format(timeDiffDefault));

   maxErrorTF.setText(""+maxErrorDefault);
   maxErrorDiffTF.setText(nf.format(maxErrorDiffDefault));

   pdd.setMaxErrors(maxErrorDefault, maxErrorDiffDefault); 

   relErrorRB.setSelected(relErrorDefault);
   pdd.enableRelativeError(relErrorDefault);

   maxMeasuresTF.setText(nf.format(maxMeasuresDefault));
   pdd.setMaximumMeasures(maxMeasuresDefault);

   maxSequenceTF.setText(nf.format(maxSequenceDefault));
   pdd.setMaxSeqLength(maxSequenceDefault);  

   removeProbTF.setText(nf.format(removeProbDefault));
   pdd.setRemoveProb(removeProbDefault);

   removeSeqProbTF.setText(nf.format(removeSeqProbDefault));
   pdd.setRemoveSeqProb(removeSeqProbDefault);

   changeErrorProbTF.setText(nf.format(changeErrorProbDefault));
   pdd.setChangeErrorProb(changeErrorProbDefault);

   writerToleranceTF.setText(nf.format(writerToleranceDefault));
   pdd.setWriterTolerance(writerToleranceDefault);
}

/** Sets all settings in the Delay tab to its default values **/
private void setDelayDefaults(){
   eventProbTF.setText(""+eventProbDefault);
   pdd.setEventProb(eventProbDefault);
   
   stopProbSl.setValue(stopProbDefault);
   pdd.setStopProb(stopProbDefault/100.0);

   maxDelayTF.setText(nf.format(maxDelayDefault));
   pdd.setMaxDelay(maxDelayDefault);

   minDecSl.setValue(minDecDefault);
   maxDecSl.setValue(maxDecDefault);
   pdd.setDecs(minDecSl.getValue()/100.0, maxDecSl.getValue()/100.0);

   minAccSl.setValue(minAccDefault);
   maxAccSl.setValue(maxAccDefault);
   pdd.setAccs(minAccSl.getValue()/100.0, maxAccSl.getValue()/100.0);
}


/** Starts the conversion.
  * First, the validity of all settings is checked. If successsful,
  * the settings are moved into the appropriate underlying implementation.
  * After that, the conversion is started and the gui is switched to
  * show the log tab.
  **/
private void startConversion(){


   // check inputFileName and OutputFileName
   String inFileName = inFileTF.getText();
   String outFileName = outFileTF.getText();

   // switch to log
   tabbedPane.setSelectedComponent(logTab);

   log.println("\n\n\nstart conversion");
   // check InputFile
   File inFile = new File(inFileName);
   if(!inFile.exists()){
      JOptionPane.showMessageDialog(this,"Input file does not exist.", 
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("input file not exists");
      return;
   } 

   ListExpr theList = new ListExpr();
   if(theList.readFromFile(inFileName)!=0){
      JOptionPane.showMessageDialog(this,"Problem in parsing input file.",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("problem in parsing input file");
      return;
   }

  
   // check outputFile
   File outFile = new File(outFileName);
   if(outFile.exists()){
      if(JOptionPane.showConfirmDialog(this,"File "+ outFileName+" exists\n"+
                                    "Overwrite it?","Please confirm",
                                     JOptionPane.YES_NO_OPTION)!=JOptionPane.YES_OPTION){

          log.println("conversion canceled by user");
          return;
      }   
   } 


   // check values

   // time diffence
   double timeDiffValue=0.0;
   try{
     timeDiffValue = Double.parseDouble(timeDiffTF.getText());
   }catch(Exception e){
     timeDiffValue = 0.0; // an invalid value
   }
   // check for valid range
   if(timeDiffValue <= 0.001){  // minimum value is one millisecond
      JOptionPane.showMessageDialog(this,"Invalid value for time difference",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid value dor time diffence");
      tabbedPane.setSelectedComponent(disturbSettings);
      timeDiffTF.requestFocus(); 
      return;        
   }


   // maximum error
   double maxErrorValue=-1.0;
   try{
     maxErrorValue = Double.parseDouble(maxErrorTF.getText());
   }catch(Exception e){
     maxErrorValue = -1.0; // an invalid value
   }
   // check for valid range
   if(maxErrorValue < 0){
      JOptionPane.showMessageDialog(this,"Invalid value for maximum error",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid value for maximum error");
      tabbedPane.setSelectedComponent(disturbSettings);
      maxErrorTF.requestFocus(); 
      return;        
   }

   // maximum error difference per measure
   double maxErrorDiffValue=-1.0;
   try{
     maxErrorDiffValue = Double.parseDouble(maxErrorDiffTF.getText());
   }catch(Exception e){
     maxErrorDiffValue = -1.0; // an invalid value
   }
   // check for valid range
   if( (maxErrorDiffValue < 0) || (maxErrorDiffValue > maxErrorValue)){
      JOptionPane.showMessageDialog(this,"Invalid value for maximum error difference",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid value for maximum error difference");
      tabbedPane.setSelectedComponent(disturbSettings);
      maxErrorDiffTF.requestFocus(); 
      return;        
   }
  
   // use relative error, no check, just get the value
   boolean relativeErrorValue = relErrorRB.isSelected();


   double changeErrorProbValue = -1;
   try{
      changeErrorProbValue = Double.parseDouble(changeErrorProbTF.getText());
   }catch(Exception e){
     changeErrorProbValue = -1;
   }
   // check for valid range
   if( (changeErrorProbValue < 0) || (changeErrorProbValue > 1)){
      JOptionPane.showMessageDialog(this,"Invalid value for probability to change the error",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid value for probability to chnage the error");
      tabbedPane.setSelectedComponent(disturbSettings);
      changeErrorProbTF.requestFocus(); 
      return;        
   }



   // maximum number of created measures
   int maxMeasures=0;
   try{
     maxMeasures = Integer.parseInt(maxMeasuresTF.getText());
   }catch(Exception e){
      JOptionPane.showMessageDialog(this,"Invalid value for maximum measures\n ( not an int)",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("maximum measure is not an int");
      tabbedPane.setSelectedComponent(disturbSettings);
      maxMeasuresTF.requestFocus(); 
      return;        
   }

   
   int maxSequenceValue = -1;
   try{
     maxSequenceValue = Integer.parseInt(maxSequenceTF.getText());
   }catch(Exception e){
       maxSequenceValue = -1;
   }
   if(maxSequenceValue <1){
      JOptionPane.showMessageDialog(this,"Invalid value for maximum lenth of a removed sequence",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid value for maximum length of a removes sequence");
      tabbedPane.setSelectedComponent(disturbSettings);
      maxSequenceTF.requestFocus(); 
      return;        
   }

   double removeProbValue=-1;
   try{
      removeProbValue = Double.parseDouble(removeProbTF.getText());
   } catch(Exception e){
      removeProbValue = -1; 
   }
   if((removeProbValue<0) || (removeProbValue>=1)){
      JOptionPane.showMessageDialog(this,"Invalid value for removing probability ",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid remove probability");
      tabbedPane.setSelectedComponent(disturbSettings);
      removeProbTF.requestFocus(); 
      return;        
   }

   double removeSeqProbValue = -1;
   try{
      removeSeqProbValue = Double.parseDouble(removeSeqProbTF.getText());
   } catch(Exception e){
      removeSeqProbValue = -1; 
   }
   if((removeSeqProbValue<0) || (removeSeqProbValue>=1)){
      JOptionPane.showMessageDialog(this,"Invalid value for removing sequence probability ",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid remove sequence probability");
      tabbedPane.setSelectedComponent(disturbSettings);
      removeSeqProbTF.requestFocus(); 
      return;        
   }

   double writerToleranceValue =-1;
   try{
     writerToleranceValue=Double.parseDouble(writerToleranceTF.getText());
   }catch(Exception e){
     writerToleranceValue = -1;
   }
   if(writerToleranceValue <0){
      JOptionPane.showMessageDialog(this,"Invalid value for writer tolerance ",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid writer tolerance");
      tabbedPane.setSelectedComponent(disturbSettings);
      writerToleranceTF.requestFocus(); 
      return;        

   }



   // delay settings
   double eventProbValue = -1;
   try{
       eventProbValue = Double.parseDouble(eventProbTF.getText());
   }catch(Exception e){
       eventProbValue = -1;
   }
   if((eventProbValue <0) || (eventProbValue>1)){
      JOptionPane.showMessageDialog(this,"Invalid value for delay event probability ",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid delay event probability");
      tabbedPane.setSelectedComponent(delaySettings);
      eventProbTF.requestFocus(); 
      return;        
   }

   int maxDelayValue = -1;
   try{
       maxDelayValue = Integer.parseInt(maxDelayTF.getText());
   }catch(Exception e){
       maxDelayValue = -1;
   }
   if(maxDelayValue < 1){
      JOptionPane.showMessageDialog(this,"Invalid value maximum delay",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("invalid maximum delay ");
      tabbedPane.setSelectedComponent(delaySettings);
      maxDelayTF.requestFocus(); 
      return;        
   }
    
   double minDecValue = minDecSl.getValue()/100.0;
   double maxDecValue = maxDecSl.getValue()/100.0;
   double minAccValue = minAccSl.getValue()/100.0;
   double maxAccValue = maxAccSl.getValue()/100.0;
   double stopProbValue = stopProbSl.getValue()/100.0;


   // set all values to the processing component
   // value from the disturbance component
   Time tD = new Time(0,(int)(timeDiffValue)*1000);
   pdd.setTimeDiff(tD);
   pdd.setMaxErrors(maxErrorValue,maxErrorDiffValue);
   pdd.enableRelativeError(relativeErrorValue);
   pdd.setChangeErrorProb(changeErrorProbValue);
   pdd.setMaximumMeasures(maxMeasures);
   pdd.setMaxSeqLength(maxSequenceValue);
   pdd.setRemoveProb(removeProbValue);
   pdd.setRemoveSeqProb(removeSeqProbValue);
   pdd.setWriterTolerance(writerToleranceValue);
   // delay settings
   pdd.setEventProb(eventProbValue);
   pdd.setMaxDelay(maxDelayValue);
   pdd.setStopProb(stopProbValue);
   pdd.setDecs(minDecValue,maxDecValue);
   pdd.setAccs(minAccValue,maxAccValue);


   // try to open the output file
   PrintStream out=null;
   try{
     out = new PrintStream(new BufferedOutputStream(new FileOutputStream(outFile)));
   }catch(Exception e){
      JOptionPane.showMessageDialog(this,"Cannot open output file.",
                                    "Error", JOptionPane.ERROR_MESSAGE);
      log.println("cannot open output file");
      return;
   }

  tabbedPane.setEnabled(false);
  clearLogBtn.setEnabled(false);
  Component[] comps = {tabbedPane,clearLogBtn};
  FinishListener finisher = new FinishListener(comps);

  Runner runner = new Runner(pdd,out,log,theList,this,finisher);
  Thread thread = new Thread(runner);
  thread.start();
}

/** The main function **/
public static void main(String[] args){
   DisturberGui dg = new DisturberGui();
   dg.setVisible(true);

}


private static class FinishListener{
   private Component[] components;
   public FinishListener(Component[] ComponentsToEnable){
        this.components = ComponentsToEnable;
   }

   public void finish(boolean success){
      for(int i=0;i<components.length;i++){
        components[i].setEnabled(true);
      }

   }
}



private static class Runner implements Runnable{
   private PointDataDisturber pdd;
   private PrintStream out;
   private Component parent;
   private PrintStream log;
   private ListExpr theList;
   private boolean error;
   private FinishListener finisher;

   public Runner(PointDataDisturber pdd,
                 PrintStream out,
                 PrintStream log,
                 ListExpr theList,
                 Component parent,
                 FinishListener finisher){
       this.pdd = pdd;
       this.out = out;
       this.log = log; 
       this.theList = theList;
       this.parent = parent;
       this.finisher = finisher;
   }

    public void run(){
       error = false;
       pdd.setOut(out);
         error = !pdd.processList(theList); 
         if(error){
             JOptionPane.showMessageDialog(parent,"Error in converting list.\n"+
                                    "Possible error in list structure. \n"+
                                    "please consult the log messages",
                                    "failed",
                                     JOptionPane.ERROR_MESSAGE);
             log.println("There was errors during conversion"); 
       }
       pdd.setOut(System.out);
       try{
            out.close();
       }catch(Exception e){
            JOptionPane.showMessageDialog(parent,"Cannot close output file.",
                                    "Error", JOptionPane.ERROR_MESSAGE);
            log.println("error in closing output file ");
        return; 
  } 

  if(!error){
    JOptionPane.showMessageDialog(parent,"Conversion finished","Success",
                                JOptionPane.INFORMATION_MESSAGE);
  }
  finisher.finish(error);
  log.println("conversion finished");
  }

  public boolean getError(){
       return error;
  }
   
}


private class ScrollListener{
  JScrollPane sp;

  public ScrollListener(JScrollPane sp){
     this.sp = sp;
  }

  public void scrollDown(){
     JScrollBar sb = sp.getVerticalScrollBar();
     if(sb!=null){
        sb.setValue(sb.getMaximum());
     }
  }

}


/** class for logging **/
private static class Logger extends PrintStream{
   
   private JTextArea area;
   private ScrollListener sl;

   /** creates a new logging area */
   public Logger(JTextArea area,ScrollListener sl){
      super(System.err);
      this.area = area;
      this.sl = sl;
   }

   
   public boolean checkError(){return false;}

   public void close(){}

   public void flush(){}

   public void print(boolean b){
     String text = b?"true":"false";
     area.append(text);
     sl.scrollDown();
   }
   
   public void print(char c){
       area.append(""+c);
       sl.scrollDown();
   }
   
   public void print(char[] s){
       area.append(""+s);
       sl.scrollDown();
   }
   
   public void print(double d){
      area.append(""+d);
      sl.scrollDown();
   }

   public void print(float f){
      area.append(""+f);
      sl.scrollDown();
   }

   public void print(int i){
      area.append(""+i);
      sl.scrollDown();
   }

   public void print(long l){
     area.append(""+l);
     sl.scrollDown();
   }

   public void print(Object o){
      area.append(""+o);
      sl.scrollDown();
   }

   public void print(String s){
      area.append(s);
      sl.scrollDown();
   }

   public void println(){
      area.append("\n");
      sl.scrollDown();
   }
 
   public void println(boolean x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }

   public void println(char x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }
   public void println(char[] x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }
   public void println(double x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }
   public void println(float x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }
   
   public void println(int x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }
   public void println(long x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }
   public void println(Object x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }
   public void println(String x){
      area.append(""+x+"\n");
      sl.scrollDown();
   }

   public void setError(boolean err){}

   public void write(byte[] buf, int off, int len){
      for(int i=off; i<len;i++){
        area.append(""+buf[i]);
      }
      sl.scrollDown();
   }

   public void write(int b){
      area.append(""+b);
      sl.scrollDown();
   }
   
   public void write(byte[] b){
     for(int i=0;i<b.length;i++){
       area.append(""+b[i]);
     }
      sl.scrollDown();
   }



}




}


