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

package viewer;

import java.awt.geom.*;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import sj.lang.ListExpr;
import java.util.*;
import gui.SecondoObject;
import gui.Environment;
import java.io.*;
import tools.Reporter;


/** This class provides a datastructure for representing a Tree.
  * Using the paint method we can draw the tree on a graphic context.
  */

class Node{

   /** Reads the Label of this Node from a nested list atom
     *
     **/
   private boolean readLabel(ListExpr LE){
      int type = LE.atomType();
      if(type==ListExpr.INT_ATOM){
         Label = ""+LE.intValue();
         return true; 
      }
      if(type==ListExpr.SYMBOL_ATOM){
         Label = LE.symbolValue();
         return true;
      }
      if(type==ListExpr.REAL_ATOM){
         Label = ""+LE.realValue();
         return true;
      }
      if(type== ListExpr.STRING_ATOM){
         Label = LE.stringValue();
         return true;
      }
      if(type==ListExpr.TEXT_ATOM){
         Label = LE.textValue();
         return true;
      }
      return false;

   }

   /** reads this (sub) tree from a nested list.
     * The return value descibes the sucess.
     * If the reading is unsuccessful, the node will be
     * a single leaf labeled with "Error".
     */
   public boolean readFrom(ListExpr LE){
       if(LE.atomType()!=ListExpr.NO_ATOM){ // a leaf
          if(!readLabel(LE)){
            Reporter.debug("Error in reading Label 1");
            Label = "Error";
            sons = null;
            return false; 
          }
          return true; 
       }

       // now we have a list descibing a tree in the from (root subtree_1 .. subtree_n)
       // empty lists are not allowed here
       if(LE.listLength()==0){
          Reporter.debug("Empty lists are not allowed as trees");
          Label = "Error";
          sons = null;
          return false;  
       }
       
       ListExpr Root = LE.first(); // the root must be only a label 
       if(!readLabel(Root)){
         Reporter.debug("The root must be an atom");
         Label = "Error";
         sons = null;
         return false;
       }    
 
       ListExpr Sons; 
       if(LE.listLength()==2){ // sons are wrapped in a list
            Sons = LE.second(); 
       } else{ // without a enclosing list
            Sons = LE.rest();
       }
       if(Sons.atomType()!=ListExpr.NO_ATOM){ // a single leaf
           sons = new Node[1];
           sons[0] = new Node("",null); 
           sons[0].readFrom(Sons);
       }else{ // a list of sons
           if(Sons.isEmpty()){ // non sons in list
              sons = null;
              return true;
           } else{ // a proper list of sons
              sons = new Node[Sons.listLength()];
              for(int i=0;i<sons.length;i++){
                 sons[i] = new Node("",null);
                 if(!sons[i].readFrom(Sons.first())){ // error in reading son
                    sons = null;
                    Label = "Error";
                    return false;
                 }
                 Sons = Sons.rest();
              } 
           }
       }
     return true;
   }


   /** Sets the Label and the Sons of this tree. **/
   Node(String Label, Node[] Sons){
      this.Label = Label;
      this.sons=Sons;
      if(!arrowDone)
         computeArrow();
   }


  /** This function fixes the form of the arrow for connections.
    **/
   void computeArrow(){
      GeneralPath gparrow = new GeneralPath();
      gparrow.moveTo(0,0);
      gparrow.lineTo(5,-1);
      gparrow.lineTo(3,0);
      gparrow.lineTo(5,1);
      gparrow.lineTo(0,0);
      arrow = new Area(gparrow);
      arrowDone = true;
  }


 /** This function computes the bounding box required for the
   * whole tree in the given context. 
   * The bounding box will start at (0,0); only width and height
   * are computed.
   **/ 
 public void computeBounds(Graphics2D g){
       // the initial is the bounding box of the label
       StringTokenizer ST = new StringTokenizer(Label,"\n");
       Rectangle2D nodebounds = new Rectangle2D.Double();
       int lineno = 0;
       while(ST.hasMoreTokens()){
         String Line = ST.nextToken();
         Rectangle2D currentBounds = g.getFontMetrics().getStringBounds(Line,g);
         currentBounds.setRect(0,0,-currentBounds.getX()+currentBounds.getWidth(),
                                   -currentBounds.getY()+currentBounds.getHeight());
         if(lineno==0)
           nodebounds=currentBounds;
         else{
             nodebounds.setRect(0,0,Math.max(nodebounds.getWidth(),currentBounds.getWidth()),
                                    nodebounds.getHeight()+currentBounds.getHeight());   

         }
         lineno++;

       }
    
       // we add a border of 5 pixels
       nodebounds.setRect(0,0,-nodebounds.getX()+nodebounds.getWidth()+10,
                              -nodebounds.getY()+nodebounds.getHeight());

       if(sons==null || sons.length==0)
         bounds = nodebounds;
       else{
          // compute the bounds of all sons
           for(int i=0;i<sons.length;i++)
              sons[i].computeBounds(g);
       
           // build the union of the bounds of the sons when drawing left to right 
           bounds = sons[0].getBounds();
           for(int i=1;i<sons.length;i++){ // compute bounds of the subtree
              Rectangle2D nextBound = sons[i].getBounds();
              bounds.setRect(0,0,
                        bounds.getWidth()+nodesep+nextBound.getWidth(),
                        Math.max(bounds.getHeight(),nextBound.getHeight()));
           }
           // store the width of the sons
           sonswidth = bounds.getWidth();
           // add the own bound
           bounds.setRect(0,0,
                          Math.max(bounds.getWidth(),nodebounds.getWidth()), // maximum of own and son bounds
                          bounds.getHeight()+levelsep+nodebounds.getHeight());
                   // bounds of sons + line + own bound
       }
   }

   /** returns the bounds computed at the last run of computeBounds */
   public Rectangle2D getBounds(){
      return bounds;
   }

   /**
    Paints this tree on the given Graphics object
    The tree is painted from (0,0). The size depends on the
    used fontsize.
   */
   public void paint(Graphics g){
       paint(g,0,0);
   }

   /**
    * Paints this tree from the given position.
    * The bounds of the tree has to been computed already.
    */
   private void paint(Graphics g1,double deltax, double deltay){
        Graphics2D g = (Graphics2D) g1;
        computeBounds( g);
        // compute the bounds for this node and move it to (0,0)
       StringTokenizer ST = new StringTokenizer(Label,"\n");
       Rectangle2D nodeBounds = new Rectangle2D.Double();
       int lineno = 0;
       while(ST.hasMoreTokens()){
         String Line = ST.nextToken();
         Rectangle2D currentBounds = g.getFontMetrics().getStringBounds(Line,g);
         currentBounds.setRect(0,0,-currentBounds.getX()+currentBounds.getWidth(),
                                   -currentBounds.getY()+currentBounds.getHeight());
         if(lineno==0)
           nodeBounds=currentBounds;
         else{
             nodeBounds.setRect(0,0,Math.max(nodeBounds.getWidth(),currentBounds.getWidth()),
                                    nodeBounds.getHeight()+currentBounds.getHeight());   

         }
         lineno++;

       }
    
       // we add a border of 5 pixels
       nodeBounds.setRect(0,0,-nodeBounds.getX()+nodeBounds.getWidth()+10,
                              -nodeBounds.getY()+nodeBounds.getHeight());
        

        // first paint the subtrees
        double sdx=deltax;
        double nextLevel = deltay + levelsep+nodeBounds.getHeight();
        // paint itself
        double xpos = (bounds.getWidth()/2 - nodeBounds.getWidth()/2+deltax); 

        double ypos = deltay;
        // draw the bounding rectangle of this node itself
        nodeBounds.setRect(xpos,ypos,nodeBounds.getWidth(),nodeBounds.getHeight());


        g.draw(nodeBounds);
        // draw the label
       ST = new StringTokenizer(Label,"\n");
       double lypos = ypos;
       double lxpos;
       while(ST.hasMoreTokens()){
         String Line = ST.nextToken();
         Rectangle2D currentBounds = g.getFontMetrics().getStringBounds(Line,g);
         currentBounds.setRect(0,0,-currentBounds.getX()+currentBounds.getWidth(),
                                   -currentBounds.getY()+currentBounds.getHeight());
         lxpos = (nodeBounds.getWidth()-currentBounds.getWidth())/2+xpos;
         g.drawString(Line,(int)lxpos,(int)(lypos-7+currentBounds.getHeight()));
         lypos = lypos+currentBounds.getHeight();
       }


        
        //g.drawString(Label,(int)xpos+5,(int)(ypos-7+nodeBounds.getHeight()));
        // compute the position for connections
        xpos = bounds.getWidth()/2;
        ypos = deltay+nodeBounds.getHeight();
        // paint the sons and connections to it
        double adddeltax = 0.0;
        if(sons!=null){
           if(sonswidth<nodeBounds.getWidth()){
              adddeltax = (nodeBounds.getWidth()-sonswidth)/(2.0);
              sdx += adddeltax;
           }
        }
        if(sons!=null){
          for(int i=0;i<sons.length;i++){
              // paint son
              sons[i].paint(g,sdx,nextLevel);
              // paint connection
              double x1 = (nodeBounds.getX()+nodeBounds.getWidth()/2);
              double y1 = (nodeBounds.getY()+nodeBounds.getHeight()+0.0);
              double x2 = sdx+sons[i].getBounds().getWidth()/2;
              double y2 = nextLevel;
              Line2D.Double L  = new Line2D.Double(x1,y1,x2,y2);
              g.draw(L);
              // transform the arrow to be in the correct angle at the end of the connection
              AffineTransform aat = new AffineTransform();
              aat.setToTranslation(x2,y2);
              // normalize
              double dx =  x1-x2;
              double dy = y1-y2;
              double len = Math.sqrt(dx*dx+dy*dy); // the length
              dx = dx / len;
              dy = dy / len;
              
              AffineTransform Rot = new AffineTransform(dx,dy,-dy,dx,0,0);   
              aat.concatenate(Rot);
              Shape S = aat.createTransformedShape(arrow); 
              // paint the arrow
              g.fill(S);
              // compute the x position for the next son
              sdx = sdx + (sons[i].getBounds().getWidth())+nodesep;
          }
        }
   }

   /** Save this tree to F in LaTex format.
     * The output goes wrong when label contain any latex 
     * commands
     **/
    boolean saveAsLatex(File F){
      if(F.exists())
         return false;
      try{
          PrintWriter out = new PrintWriter(new FileWriter(F));
          // print latex header
          out.println("\\documentclass{article}");
          out.println("\\usepackage{pst-tree,pst-eps}");
          out.println("\\pagestyle{empty}");
          out.println("\\begin{document}");
          out.println("\\thispagestyle{empty}");
          out.println("\\TeXtoEPS%");
          printRecTo(out);
          out.println("\\endTeXtoEPS");
          out.println("\\end{document}");
          out.close();
      }catch(Exception e){
        Reporter.debug(e);
        return false;
      }
      return true;
   }

   /** Supports saveAsLatex **/
   private void printRecTo(PrintWriter out) {
     // this is a tree
     if(sons!=null)
         out.println("{\\pstree[levelsep=1cm,treesep=0.5cm,edge=\\ncline{->}]%");
     // save label as root
     out.println("{ \\Tr{\\psframebox[framearc=0.5]{"+Label+"}}}%");
     // perform all sons
     if(sons!=null){
        out.println("{%");
        for(int i=0;i<sons.length;i++){
            if(sons[i]!=null){
               sons[i].printRecTo(out);
            }
        }
        out.println("}}%"); // close sons and this tree
     }
   } 

   String Label;
   Node[] sons;
   double sonswidth =0;
   Rectangle2D bounds=null;   // bounds for this node and all subtrees
   static int nodesep = 10;   // distance between nodes
   static int levelsep = 20;  // distance between levels
   static Shape arrow;
   static boolean arrowDone= false;
}


/** This class provides a panel drawing a single tree.
  * They are possibilities for magnifying or moving the drawing.
  */
class TreePainterPanel extends JPanel{

 TreePainterPanel(){
     super();
     // add a mouselistener for magnifying the picture
     addMouseListener(new MouseAdapter(){
         public void mouseClicked(MouseEvent evt){
            int x = evt.getX();
            int y = evt.getY();
            int Bt = evt.getButton();
            Dimension D = getSize();
            if(Bt==MouseEvent.BUTTON1){ // left button = zoom in
                 double sc = scalefactor;
                 tmpAT.setTransform(scalefactor,0,0,scalefactor,
                                    -x*scalefactor+D.getWidth()/2,
                                    -y*scalefactor+D.getHeight()/2);
                 AT.preConcatenate(tmpAT);
                 repaint();
            }
 
            if(Bt==MouseEvent.BUTTON3){ // right button = zoom out
               double sc = 1/scalefactor;
               tmpAT.setTransform(sc,0,0,sc,
                                  -x*sc+D.getWidth()/2,
                                  -y*sc+D.getHeight()/2);
               AT.preConcatenate(tmpAT);
               repaint();
            }
            if(Bt==MouseEvent.BUTTON2){ // middle button = go to the first view
               firstPaint=true;
               repaint();
            }
          }
      });
      //  change the background to be white
      setOpaque(true);
      setBackground(Color.WHITE);
  }


  /**
    Fits the tree to the available size
  */
  public void fit(){
      firstPaint=true;
      repaint();
  } 

  /** Zooms the current drawing by scale factor around the middle of the screen.
    */
  public void zoomIn(){
    double sc = scalefactor;
    Dimension D = getSize();
    double x = D.getWidth()/2;
    double y = D.getHeight()/2;
    tmpAT.setTransform(scalefactor,0,0,scalefactor,
                       -x*scalefactor+D.getWidth()/2,
                       -y*scalefactor+D.getHeight()/2);
   AT.preConcatenate(tmpAT);
   repaint();
  }

  /** Zooms out the current drawing */
  public void zoomOut(){
    double sc = 1/scalefactor;
    Dimension D = getSize();
    double x = D.getWidth()/2;
    double y = D.getHeight()/2;
    tmpAT.setTransform(sc,0,0,sc,
                       -x*sc+D.getWidth()/2,
                       -y*sc+D.getHeight()/2);
   AT.preConcatenate(tmpAT);
   repaint();

  } 

  /** paints the contained tree  **/
  public void paint(Graphics g1){
     Graphics2D g = (Graphics2D) g1; 
     g.setBackground(Color.WHITE);
     super.paint(g1);
     if(Tree==null){
         String S1 = "Secondo";
         String S2 = "FernUni Hagen";
         Rectangle2D BB = g.getFontMetrics().getStringBounds(S1,g);
         BB.setRect(0,0,-BB.getX()+BB.getWidth(),(-BB.getY()+BB.getHeight()));
         double sf = Math.min( (getSize().getWidth()-20)/BB.getWidth(),
                               ((getSize().getHeight()/2)-20)/BB.getHeight());
         AffineTransform OldTransform = g.getTransform();
         AffineTransform A1 = new AffineTransform();
         A1.setToIdentity();
         A1.setToScale(sf,sf);
         g.transform(A1);
         Color OldColor = g.getColor();         
         g.setColor(Color.RED); 
         g.drawString(S1,(int)(10/sf), (int)(getSize().getHeight() / (3*sf) ));
         g.setColor(Color.BLUE);
         g.setTransform(OldTransform);
         BB = g.getFontMetrics().getStringBounds(S2,g);
         BB.setRect(0,0,-BB.getX()+BB.getWidth(),(-BB.getY()+BB.getHeight())); 
         g.rotate(0.5);
         g.scale(sf/2,sf/2);
         g.drawString(S2,(int)(20/sf+BB.getWidth()), 
                         (int)(-BB.getHeight()+getSize().getHeight() / (1.5*sf) ));
          
         g.setColor(OldColor);  
         return;
     }
     g.setColor(Color.BLACK);	 
     // in the first paint we center the tree, possible fitting to the window
     if(firstPaint){
          Dimension D = getSize();
          Tree.computeBounds(g);          
          TreeBounds=Tree.getBounds();
          if(D!=null){
             if(fit){ // move to center and fit to window
               double sc = Math.min((D.getWidth()-30)/TreeBounds.getWidth(),
                                    (D.getHeight()-20)/TreeBounds.getHeight());
               double x = TreeBounds.getWidth()/2;
               double y = TreeBounds.getHeight()/2;              
               AT.setTransform(sc,0,0,sc,
                               -x*sc+D.getWidth()/2,
                               -y*sc+D.getHeight()/2);
             }
             else{ // move to center
                 AT.setToTranslation( (D.getWidth()-TreeBounds.getWidth())/2,
                                      (D.getHeight()-TreeBounds.getHeight())/2);
             }
             firstPaint=false;
          }
     }
     g.setTransform(AT);
     // set borderwidth to 1 undepending on the zoom factor
     g.setStroke(new BasicStroke((float) (1/AT.getScaleX())));
     if(Tree!=null)
	Tree.paint(g);
 }

  /* move the drawing , values are in pixels*/
  void moveXY(int X,int Y){
     AffineTransform A = new AffineTransform();
     A.setToTranslation(X,Y);
     AT.preConcatenate(A);
     repaint();
  }


  // sets the tree to paint
   void setTree(Node Tree){
     this.Tree=Tree;
     AT.setToIdentity();
     firstPaint=true;
     repaint();
   }
  

 // the used affine transformation
 AffineTransform AT=new AffineTransform();
 // temporal affine transformation to avoid creation of new objects
 AffineTransform tmpAT = new AffineTransform();
 // scale for one single click
 double scalefactor = 1.5;
 // the Tree to paint
 Node Tree=null;
 // the bounding box of this tree
 Rectangle2D TreeBounds=null;
 // paint without manual zoomung 
 boolean firstPaint=true;
 // fit to window
 boolean fit=true;
}


/** This class is the Viewer for Trees */
public class TreeViewer extends SecondoViewer{


private void down(){
    int d = (int) (TPP.getSize().getHeight() / 3);
    TPP.moveXY(0,-d);
}

private void up(){
    int d = (int) (TPP.getSize().getHeight() / 3);
    TPP.moveXY(0,d);
}

private void left(){
    int d = (int) (TPP.getSize().getWidth()/3);
    TPP.moveXY(d,0);
}

private void right(){
    int d = (int) (TPP.getSize().getWidth()/3);
    TPP.moveXY(-d,0);
}

private static void addKeyListenerRec(KeyListener KL, Component C){
   C.addKeyListener(KL);
   if(C instanceof Container){
      Container F = (Container) C;
      Component[] Comps = F.getComponents();
      for(int i=0;i<Comps.length;i++)
          addKeyListenerRec(KL,Comps[i]);
   }
}



/** Creates a new Instance of this Viewer **/
public TreeViewer(){
  setLayout(new BorderLayout());
  add(ChoiceBox,BorderLayout.NORTH);
  add(TPP,BorderLayout.CENTER);
  JPanel ControlPanel = new JPanel();
  ControlPanel.add(SaveBtn);
  ControlPanel.add(FitBtn);
  ControlPanel.add(ZoomInBtn);
  ControlPanel.add(ZoomOutBtn);
  ControlPanel.add(new JLabel("     ")); // distance
  ControlPanel.add(LeftBtn);
  ControlPanel.add(UpBtn);
  ControlPanel.add(DownBtn);
  ControlPanel.add(RightBtn);

  add(ControlPanel,BorderLayout.SOUTH);

  ActionListener AL = new ActionListener(){
    public void actionPerformed(ActionEvent evt){
           Object o = evt.getSource();
           if(o.equals(DownBtn)){
               down();
           }else if(o.equals(UpBtn)){
               up();
           }else if(o.equals(LeftBtn)){
               left();
           }else if(o.equals(RightBtn)){
                right();
           }else if(o.equals(FitBtn)){
                TPP.fit();
           }else if(o.equals(ZoomInBtn)){
                TPP.zoomIn();
           }else if(o.equals(ZoomOutBtn)){
                TPP.zoomOut();
           }
    }
  };
  SaveBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        if(TPP.Tree==null){
          Reporter.showError("no tree present");
        }
        JFileChooser FC = new JFileChooser();
        if(FC.showSaveDialog(null)==JFileChooser.APPROVE_OPTION){
           File F = FC.getSelectedFile();
           if(F.exists()){
              Reporter.showError("File "+ F.getName()+" alreday exists");
              return;
           }
           if(!TPP.Tree.saveAsLatex(F)){
              Reporter.showError("error occured");
           } else{
              Reporter.showInfo("tree stored");
           }
        }
      }
  });
  UpBtn.addActionListener(AL);
  DownBtn.addActionListener(AL);
  LeftBtn.addActionListener(AL);
  RightBtn.addActionListener(AL);
  FitBtn.addActionListener(AL);
  ZoomInBtn.addActionListener(AL);
  ZoomOutBtn.addActionListener(AL);

  ChoiceBox.addItemListener(new ItemListener(){
      public void itemStateChanged(ItemEvent evt){
             int i = ChoiceBox.getSelectedIndex();
             if(i<0){
                TPP.setTree(null);
                return; 
             }
             TPP.setTree((Node)Trees.get(i));
     }
 
  });

 KeyListener KL = new KeyAdapter(){
       public void keyPressed(KeyEvent evt){
          if(evt.getKeyCode()==KeyEvent.VK_L && (evt.getModifiers() & KeyEvent.CTRL_MASK)>0){
              TPP.repaint();
          } else
          if(evt.getKeyCode()==KeyEvent.VK_F && (evt.getModifiers() & KeyEvent.CTRL_MASK)>0){
               TPP.fit();
          } else
            if(evt.getKeyCode()==KeyEvent.VK_PLUS && (evt.getModifiers() & KeyEvent.CTRL_MASK)>0){
                TPP.zoomIn();
            } else
            if(evt.getKeyCode()==KeyEvent.VK_MINUS && (evt.getModifiers() & KeyEvent.CTRL_MASK)>0){
                TPP.zoomOut();
            } else
            if (evt.getKeyCode()==KeyEvent.VK_LEFT){
                int d = (int) TPP.getSize().getWidth()/3;
                TPP.moveXY(-d,0);
            } else
            if (evt.getKeyCode()==KeyEvent.VK_RIGHT){
                int d = (int) TPP.getSize().getWidth()/3;
                TPP.moveXY(d,0);
            } else
            if (evt.getKeyCode()==KeyEvent.VK_UP){
                int d = (int) TPP.getSize().getHeight()/3;
                TPP.moveXY(0,-d);
            } else
            if (evt.getKeyCode()==KeyEvent.VK_DOWN){
                int d = (int) TPP.getSize().getWidth()/3;
                TPP.moveXY(0,d);
            }
         }
  };
  addKeyListenerRec(KL,this);



}


/** gets the name of this Viewer **/
public String getName(){ return "TreeViewer";}


/** add an object to this viewer **/
public boolean addObject(SecondoObject o){
    if(!canDisplay(o)){
       return false;
    }
    ListExpr LE = o.toListExpr().second(); // we need only the value
    Node N = new Node("",null);
    if(!N.readFrom(LE)){ // error in valueList
       return false;
    }
    // values are correct
    Trees.add(N);
    SObjects.add(o);
    ChoiceBox.addItem(o.getName());
    ChoiceBox.setSelectedIndex(Trees.size()-1);
    requestFocus();
    return true;
}

/** removes an given object */
public void removeObject(SecondoObject o){
   int index = SObjects.indexOf(o);
   if(index < 0)
      return;
   SObjects.removeElementAt(index);
   Trees.removeElementAt(index);
   ChoiceBox.removeItemAt(index);
}

public void removeAll(){
   SObjects.clear();
   Trees.clear();
   ChoiceBox.removeAll();
   TPP.setTree(null); 
}

public boolean canDisplay(SecondoObject o){
   ListExpr LE = o.toListExpr();
   if(LE.listLength()!=2)
     return false;
   LE = LE.first();
   if(LE.atomType()!=ListExpr.SYMBOL_ATOM)
      return false;
   if(!LE.symbolValue().equals("tree"))
      return  false;
   return true;
}

public boolean isDisplayed(SecondoObject o){
    return SObjects.indexOf(o)>=0;
}

public boolean selectObject(SecondoObject o){
    int index = SObjects.indexOf(o);
    if(index <0)
       return false;
    ChoiceBox.setSelectedIndex(index);
    return true;
}

public MenuVector getMenuVector(){
    return null;
}

public double getDisplayQuality(SecondoObject o){
   if(canDisplay(o))
     return  0.8;
   return 0;
}


private JComboBox ChoiceBox = new JComboBox();
private JButton SaveBtn= new JButton("LaTeX");
private JButton UpBtn=new JButton("^");
private JButton DownBtn = new JButton("v");
private JButton LeftBtn = new JButton("<");;
private JButton RightBtn = new JButton(">");
private JButton ZoomInBtn = new JButton("Zoom In");
private JButton ZoomOutBtn = new JButton("Zoom Out");
private JButton FitBtn = new JButton("Fit");
private Vector SObjects = new Vector(10);
private Vector Trees = new Vector(10);
TreePainterPanel TPP = new TreePainterPanel();


}


