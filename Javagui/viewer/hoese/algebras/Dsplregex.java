

package viewer.hoese.algebras;

import att.grappa.*;
import viewer.hoese.*;
import java.util.TreeSet;
import java.util.Iterator;
import  sj.lang.ListExpr;
import javax.swing.*;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import tools.Reporter;
import java.io.File;
import java.io.PrintWriter;
import java.io.FileWriter;


public class Dsplregex extends DsplGeneric implements ExternDisplay{

 private Graph g;
 private GrappaPanel gp;
 private String name;
 private String source;
 

 private static JFrame externFrame;
 private static Dsplregex currentlyDisplayed=null;
 private static JFileChooser fc;
 private static JScrollPane sp;
 private static JTextField tf;


 public Dsplregex(){
    g = null;
    name ="";
    if(externFrame==null){
       externFrame = new JFrame();
       JPanel controlPanel = new JPanel();
       externFrame.setLayout(new BorderLayout());
       externFrame.add(controlPanel, BorderLayout.SOUTH);
       JButton saveAsEps = new JButton("save as EPS");
       saveAsEps.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
            save(true);
         }
       });
       JButton closeBtn = new JButton("close");
        closeBtn.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
          externFrame.setVisible(false);
         }
       });
       JButton saveAsDot = new JButton("Save as dot");
       saveAsDot.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
            save(false);
         }
       });
      
       controlPanel.add(saveAsEps);
       controlPanel.add(saveAsDot);
       controlPanel.add(closeBtn);
       sp = new JScrollPane();
       externFrame.add(sp, BorderLayout.CENTER);
       tf = new JTextField(100);
       externFrame.add(tf,BorderLayout.NORTH);
       tf.setEditable(false);
    }
    
 }

  void save( boolean eps){
     if(currentlyDisplayed==null){
         Reporter.showError("no graph available");
         return;
     }

     if(fc==null){
        fc = new JFileChooser();
     }
     if(fc.showSaveDialog(externFrame)==JFileChooser.APPROVE_OPTION){
        try{
          File F = fc.getSelectedFile();
          if(F.exists()){
            if(Reporter.showQuestion("File already exits\n overwrite it?")!=Reporter.YES){
              return;
            }   
          }  

          if(eps){ 
              boolean res = extern.psexport.PSCreator.export(currentlyDisplayed.gp,F);
              if(!res){
                 Reporter.showError("Error in Exporting image");
               }   
          } else { // dot format
             PrintWriter out = new PrintWriter(new FileWriter(F));
             currentlyDisplayed.g.printGraph(out);
             out.close();
          } 
        } catch(Exception e){ 
           Reporter.showError("Error in exporting graphic");
           Reporter.debug(e);
        } 
     }   
  }



public String toString(){
  return name;
}

private void setTo(String what, int nameWidth, int indent, QueryResult qr){
   name=extendString(what,nameWidth, indent);
   qr.addEntry(name);
}


private String getStr(int i){
   if((i>32) && (i<128)){
     return "" + Character.valueOf((char)i);
   }
   if(i==9){
      return "\\t";
   }
   if(i==10){
     String res = "\\n";
     return res;
   }
 
   if(i==13){
      return "\\r";
   }

   return "(" +  i + ")";
  
}

private String getStr(int first, int last){

   StringBuffer buf = new StringBuffer();
   if( (last-first) < 5 ){
      for(int i=first; i<=last;i++){
        buf.append(getStr(i));
      }  
   } else {
      buf.append(getStr(first));
      buf.append(" - ");
      buf.append(getStr(last));
   }
   String res = buf.toString();
   return res;
}


private String getLabel(TreeSet<Integer> t, int coverage){
  StringBuffer buf = new StringBuffer();
  Iterator<Integer> it = t.iterator();
  if(t.size()==0){
     return "";
  }
  if(t.size()==1){
    return getStr(it.next().intValue());
  }

  if(t.size()==256){
     buf.append("all");
  } else if(coverage == 256 && t.size() > 127) { // more than half
     buf.append("other");
  } else if(t.size() > 250){ // here it's easier to use [^...]A
     buf.append("[^");
     int pos = 0;
     int next = -1;
     while(pos<256){
        if(pos==next){
           pos++;
           if(it.hasNext()){
              next = it.next().intValue();
           } else {
              next = 257;
           }
        } else if(pos<next){
           buf.append(getStr(pos));
           pos++;
        } else { // pos > next
           if(it.hasNext()){
              next = it.next(); 
           } else {
              next = 257;
           }
        }
     }
     buf.append("]");
     return buf.toString();
  } else if(t.size() < 5) {
     buf.append("[");
     while(it.hasNext()){
       buf.append(getStr(it.next().intValue()));
     }
     buf.append("]");
  } else {
    int first = -2;
    int last = -2;
    buf.append("[");
    while(it.hasNext()){
      int next = it.next().intValue();
      if(last < 0){
         last = next;
         first = next;
      } else {
         if(next==last+1){
            last = next;
         }else {
            buf.append(getStr(first,last));
            buf.append(" ");
            last = next;
            first = next;
         }
      }
    }
    if(first>0){
      buf.append(getStr(first,last));
    }
    buf.append("]");
  }
  return buf.toString();
}


private void buildGraph(TreeSet<Integer>[][] transitions, TreeSet<Integer> finalStates){

  int[] coverage = new int[transitions.length];
  for(int src=0;src<transitions.length;src++){
     int sum = 0;
     for(int dest = 0; dest<transitions.length;dest++){
        if(transitions[src][dest] != null){
           sum += transitions[src][dest].size();
        }
     }
     coverage[src] = sum;
  }
  g = new Graph("regex");
  Node[] nodes = new Node[transitions.length];
  // insert nodes
  for(int i=0;i<transitions.length;i++){
    nodes[i] = new Node(g,""+i);
    nodes[i].setAttribute(GrappaConstants.LABEL_ATTR,""+i);
    if(finalStates.contains(i)){
       nodes[i].setAttribute(GrappaConstants.SHAPE_ATTR, GrappaConstants.DOUBLECIRCLE_SHAPE);
    }
    g.addNode(nodes[i]);
  }
  nodes[0].setAttribute(GrappaConstants.COLOR_ATTR,java.awt.Color.RED);
  // insert edges
  for(int src = 0; src <  transitions.length; src++){
    for(int dest = 0; dest < transitions.length; dest++){
       if(transitions[src][dest]!=null){
         Edge e = new Edge(g,nodes[src],nodes[dest]);
         e.setAttribute(GrappaConstants.LABEL_ATTR, getLabel(transitions[src][dest],coverage[src]));
         g.addEdge(e); 
       }
    }
  }

  try{
    // format the graph using dot
     String[] processArgs = {"dot"};
    //String[] processArgs = {"twopi"};
    Process formatProcess = Runtime.getRuntime().exec(processArgs,null,null);
    GrappaSupport.filterGraph(g,formatProcess);
    formatProcess.getOutputStream().close();
  } catch(Exception e){
    e.printStackTrace();  
  }


  gp = new GrappaPanel(g);
  g.addPanel(gp);
}


private boolean  insertTransition(ListExpr trans, TreeSet<Integer>[][] transitions){

  if(trans.listLength()!=3){
    return false;
  }
  if((trans.first().atomType()!=ListExpr.INT_ATOM) ||
     (trans.second().atomType()!=ListExpr.INT_ATOM) ||
     (trans.third().atomType()!=ListExpr.INT_ATOM)){
    return false;
  }
  int src = trans.first().intValue();
  int d = trans.second().intValue();
  int dest = trans.third().intValue();
  if( (src<0) ||  (dest<0) || (src>=transitions.length) || (dest>=transitions.length)){
     return false;
  }
  if((d<0) || (d>255)){
    return false;
  }
  if(transitions[src][dest]==null){
     transitions[src][dest] = new TreeSet<Integer>();
  }
  transitions[src][dest].add(new Integer(d));
  return true;
}


public void init(String name, int nameWidth, int indent, ListExpr type,ListExpr value, QueryResult qr){

  if(isUndefined(value)){
     setTo("undef",nameWidth,indent,qr);
     return;
  }
  if((value.listLength() !=3) && (value.listLength() != 4)){ // (#states <transitions> <finalstates> [source])
     setTo("error 1",nameWidth,indent,qr);
     return;
  }
  if(value.listLength()==4){
    ListExpr sourceList = value.fourth();
    if((sourceList.atomType()==ListExpr.STRING_ATOM) || 
       (sourceList.atomType()==ListExpr.TEXT_ATOM)){
      source = sourceList.stringValue();
    }else{
      setTo("error 1.1", nameWidth, indent, qr);
      return;
    }
  }

  ListExpr noStatesL = value.first();
  if(noStatesL.atomType()!=ListExpr.INT_ATOM){
    setTo("error 2",nameWidth,indent,qr);
    return;
  }
  int numStates = noStatesL.intValue();
  if(numStates<0){
    setTo("error 3",nameWidth,indent,qr);
    return;
  }
  if(numStates==0){
     setTo("empty",nameWidth,indent,qr);
     return;
  }
  TreeSet<Integer>[][] transitions = new TreeSet[numStates][numStates];
  for(int src=0;src<numStates;src++){
     for(int dest = 0; dest < numStates;dest++){
         transitions[src][dest] = null;
     }
  }

  ListExpr transList = value.second();
  if(transList.isAtom()){
     setTo("error 4",nameWidth,indent,qr);
     return;
  }
  while(!transList.isEmpty()){
    ListExpr trans = transList.first();
    transList = transList.rest();
    if(!insertTransition(trans,transitions)){
      setTo("error 5",nameWidth,indent,qr);
      return;
    }    
  }
  TreeSet<Integer> finalStates = new TreeSet<Integer>();
  ListExpr finalList = value.third();
  if(finalList.isAtom()){
    setTo("error 6",nameWidth,indent,qr);
    return;
  }
  while(!finalList.isEmpty()){
    ListExpr fin = finalList.first();
    finalList = finalList.rest();
    if(fin.atomType()!=ListExpr.INT_ATOM){
      setTo("error 7",nameWidth,indent,qr);
      return;
    }
    int f = fin.intValue();
    if(f<0 || f>=transitions.length){
      setTo("error 7",nameWidth,indent,qr);
      return;
    }
    finalStates.add(new Integer(f));
  }
  buildGraph(transitions, finalStates);
  this.name = extendString(name, nameWidth,indent);
  this.name = this.name + " : regex";
  qr.addEntry(this);
  
}


public boolean isExternDisplayed(){
   return (this == currentlyDisplayed) && externFrame.isVisible();
}

public void displayExtern(){
   if(this==currentlyDisplayed){
      Dimension d = externFrame.getSize();
      if( (d.width < 640) || (d.height<480)){
         externFrame.setSize(Math.max(d.width,640), Math.max(d.height,480));
      }
      externFrame.setVisible(true);
      return;
   }
   // remove the currently displayed regex
   if(currentlyDisplayed!=null){
      externFrame.remove(currentlyDisplayed.gp);
   }
   currentlyDisplayed = this;
   sp.setViewportView(gp);
   tf.setText(source);
   externFrame.validate();
   Dimension d = externFrame.getSize();
   if( (d.width < 640) || (d.height<480)){
      externFrame.setSize(Math.max(d.width,640), Math.max(d.height,480));
   }
   externFrame.setVisible(true);
  
 
}



}
