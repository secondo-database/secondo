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



interface PNode{
  void computeBounds(Graphics2D g);
  Rectangle2D getBounds();
  void paint(Graphics2D g);
  boolean saveAsLatex(File f);
}


/** This class provides a datastructure for representing a Tree.
  * Using the paint method we can draw the tree on a graphic context.
  */

class TreeNode implements PNode{

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
           sons = new TreeNode[1];
           sons[0] = new TreeNode("",null); 
           sons[0].readFrom(Sons);
       }else{ // a list of sons
           if(Sons.isEmpty()){ // non sons in list
              sons = null;
              return true;
           } else{ // a proper list of sons
              sons = new TreeNode[Sons.listLength()];
              for(int i=0;i<sons.length;i++){
                 sons[i] = new TreeNode("",null);
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

   boolean readFromPMType(ListExpr value){
      if(value.listLength()!=2){
        Reporter.debug("wrong list length");
        return false;
      }
      ListExpr subtype = value.first();
      String st = null;
      if(subtype.atomType()!=ListExpr.SYMBOL_ATOM){ // an usual submove
          Reporter.debug("Invalid list for the submove" + subtype );
          return false; 
      }
      st = subtype.symbolValue();

      if(TreeViewer.isPMType(subtype)){ // the total move
         value = value.second();
         if(value.listLength()!=2){ // Instant SubMove
           Reporter.debug("Invalid Length");
           return false;
         }  
         Double time = viewer.hoese.LEUtils.readInstant(value.first());
         if(time==null){
            Reporter.debug("invalid representation of the start time");
            return false;
         } else {
           Label = st + " " + viewer.hoese.LEUtils.convertTimeToString(time.doubleValue());
           System.out.println("MyLabel = " + Label);
           sons = new TreeNode[1];
           sons[0] = new TreeNode("",null);
           return sons[0].readFromPMType(value.second());
         }
      }
            
      if(st.equals("linear")){
          Label = "L";
          sons = null;
          return true;
      } 

      if(st.equals("period")){
          System.out.println("Period found");
          value = value.second();
          int len = value.listLength();
          if(len!=2 && len!=4){ // (reps submove)
              Reporter.debug("period:invalid length "+len);
              return false;
          } 
          ListExpr r = value.first();
          if(r.atomType()!=ListExpr.INT_ATOM){
              Reporter.debug("invalid type for repetitions");
              return  false;
          }
          Label = "P["+r.intValue()+"]";
          sons = new TreeNode[1];
          sons[0] = new TreeNode("",null);
          if(len==2){
             return sons[0].readFromPMType(value.second());
          } else{
             return sons[0].readFromPMType(value.fourth());
          }
      }
      if(st.equals("composite")){
          value = value.second();
          int len = value.listLength();
          Label = "C";
          sons = new TreeNode[len];
          for(int i=0;i<len;i++){
             sons[i] = new TreeNode("",null);
             if(!sons[i].readFromPMType(value.first())){
               return false;
             }
             value = value.rest();
          }
          return true;
      } 
      return false;

   }


   /** Sets the Label and the Sons of this tree. **/
   TreeNode(String Label, TreeNode[] Sons){
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
   public void paint(Graphics2D g){
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
    public boolean saveAsLatex(File F){
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
   TreeNode[] sons;
   double sonswidth =0;
   Rectangle2D bounds=null;   // bounds for this node and all subtrees
   static int nodesep = 10;   // distance between nodes
   static int levelsep = 20;  // distance between levels
   static Shape arrow;
   static boolean arrowDone= false;
}


class BTreeNode implements PNode{

 private String[] labels;
 private BTreeNode[] sons;
 private boolean isLeaf;
 boolean defined;
 boolean empty;
 Rectangle2D bounds;
 private static final int vDist = 30;
 private static final int hDist = 5;
 private static final int nodeHeight = 15;

 public boolean saveAsLatex(File f){
    Reporter.showError("not implemented for a btree");
    return false;
 }


 public BTreeNode(){
    defined = false;
    labels = null;
    sons = null;
    bounds = null;
 }

 public int getHeight(){
   if(!defined){
     return 0;
   } else {
     int h = 0;
     BTreeNode n = this;
     while(!n.isLeaf){
        h++;
        n = n.sons[0];
     }
     return h;  
   }
 }

 private int getWidth(Graphics2D g){
    return getWidth(determineEntryWidth(g.getFontMetrics())); 
 }

 private int getWidth(int entryWidth){
    if(isLeaf){
      int sep = 10;
      int w = (entryWidth + sep )* labels.length;
      return  w;
    } else {
     int x1 = 0;
     for(int i=0;i<sons.length;i++){
       int sw =  sons[i].getWidth(entryWidth);
       x1 += hDist + sw;
     }
     x1 -= hDist;
     return x1;
    }
 }

 public Rectangle2D getBounds(){
   return bounds;
 }

 public void  computeBounds(Graphics2D g){

   if(empty){
     bounds = new Rectangle2D.Double(0,0,10,10);
     return;
   }

   int h = getHeight();

   h = h*(nodeHeight+vDist) + nodeHeight;

   bounds = new Rectangle2D.Double(0,0,getWidth(g),h);

 }



 /** The list must be formated as (l1, l2, ...l_n ( s_1 ... s_n+1)
   * where l1...l_n are of type string or symbol or int
   */
 public boolean readFrom(ListExpr list,int level){
    if(list.atomType()!=ListExpr.NO_ATOM){
      System.out.println(" not a list");
      return false;
    }
    int len = list.listLength();
    if(len ==0 && level==0){
        // an empty tree
        defined = true;
        empty = true;
        return true;
    }
    empty = false;
    if(len!=1 && len != 2){
      System.out.println("wrong length len = " + len);
      return false;
    }
    int len1 = list.first().listLength();
    isLeaf = len==1;
    int len2 = 0;
    if(!isLeaf){
       len2 = list.second().listLength();
       if(len2<1){
          return false; // sons must be inside a list
       }

       sons = new BTreeNode[len2];
       if(len1 != len2-1){
         System.out.println("wrong length len1 = " + len1 + " len2=" + len2);
         return false;
       }
    }

    if(len1 < 0){ // error not an list for the entries
       return false;
    }

    
    // read the entries
    labels = new String[len1];
    ListExpr rest = list.first();
    ListExpr first;
    for(int i=0;i<len1; i++){
      first = rest.first();
      rest = rest.rest();
      switch(first.atomType()){
         case ListExpr.INT_ATOM : labels[i] = "" + first.intValue();break;
         case ListExpr.STRING_ATOM : labels[i] = first.stringValue(); break;
         case ListExpr.SYMBOL_ATOM : labels[i] = first.symbolValue(); break;
         default: labels = null; sons = null; return false;
      }
    }
    if(isLeaf){
      defined = true;
      return true;
    } else {
      rest = list.second();
      for(int i=0;i<len2;i++){
        first = rest.first();
        rest = rest.rest();
        sons[i] = new BTreeNode();
        if(!sons[i].readFrom(first,level+1)){
          labels = null;
          sons = null;
          return false;
        } 
      }
      defined = true;
      return true;
    }   
 } 

 private int determineEntryWidth(FontMetrics fm){
   int last = isLeaf?labels.length:labels.length-1;
   int max = 0;
   for(int i=0;i<last;i++){
      int d = fm.stringWidth(labels[i]);
      if(d>max){
        max = d;
      }
   }
   if(!isLeaf){
      for(int i=0;i<sons.length;i++){
         int d = sons[i].determineEntryWidth(fm);
         if(d>max){
           max = d;
         }
      }
   }
   return max;
 }

 public void paint(Graphics2D g){
    if(!defined){
       g.drawString("not defined",10,10);
       return;
    } else if(empty){
       g.drawRect(0,0,10,10);
       g.drawLine(0,0,10,10);
       g.drawLine(0,10,10,0);
    } else {
       int entryWidth = determineEntryWidth(g.getFontMetrics());
       paintRec(g, 0,0, entryWidth); 
    }
 }

 private static void drawStringAt(Graphics2D g, String s, int x, int y){
    int w = g.getFontMetrics().stringWidth(s);
    g.drawString(s, x - w/2 , y);

 }

 int paintRec(Graphics2D g, int x, int y, int entryWidth){
    if(isLeaf){
      int sep = 10;
      int w = (entryWidth + sep )* labels.length;
      // draw a rectangle
      g.drawRect(x,y, w, nodeHeight);

      int xl = 0;
      int xs = 0;
      // draw the entries and separators
      for(int i=0; i < labels.length;i++){
        xl = x + i * (entryWidth+sep) + sep/2 + entryWidth/2;
        drawStringAt(g,labels[i], xl , y + nodeHeight -2);
        if(i < labels.length-1){
          xs = x + (i+1) *(entryWidth+sep) ; 
          g.drawLine(xs, y , xs, y + nodeHeight);
        }
      }
      return  w;
    } else {
 
     int sep = 14; 

     int w = (entryWidth + sep)* labels.length + sep -sep/3;

     // paint sons, store middles
     Vector v = new Vector();
     int x1 = x;
     for(int i=0;i<sons.length;i++){
       int sw =  sons[i].paintRec(g,x1, y + nodeHeight + vDist, entryWidth);
       v.add(new Integer(x1 + sw/2));
       x1 += hDist + sw;
     }

     x1 -= hDist;

     // paint box
     int xs = (x+x1)/2 - w / 2;
 
     g.drawRect(xs,y, w, nodeHeight);
     // paint labels , separators and connections to the sons
     // paint first separator:
     int y1 =  y+nodeHeight;

     g.drawLine(xs+sep/2,y, xs+sep/2,y + nodeHeight);
     g.drawLine(xs+sep/4,y1, 
               ((Integer)v.get(0)).intValue(),y+nodeHeight+vDist     );

     for(int i=0; i < labels.length;i++){
        int xl =  xs + sep + entryWidth/2 + i*(entryWidth + sep);
        drawStringAt(g,labels[i], xl, y+nodeHeight -2);
        int x2 = xs + (i+1)*(entryWidth+sep) + sep /2; 
        g.drawLine(x2-sep/3, y , x2-sep/3, y + nodeHeight);
        if(i<labels.length-1){
           g.drawLine(x2+sep/3, y , x2+sep/3, y + nodeHeight);
        }
        g.drawLine(x2, y1 , ((Integer)v.get(i+1)).intValue(), y+nodeHeight+vDist);
      }

      return x1-x;
    } 
 }
}


enum NodeType{Pointer, Object, IndirectObject, Operator,Unknown };


class OpTreeNode implements  PNode{


  public OpTreeNode(){
    bounds = new Rectangle2D.Double(0,0,100,100);
    sons = new Vector<OpTreeNode>();
    label = "--";
  }

  public void computeBounds(Graphics2D g) {
     bounds = new Rectangle2D.Double(0,0, getWidth(g), getHeight(g));
  }

  int getWidth(Graphics2D g){
     int myWidth = g.getFontMetrics().stringWidth(label) + 20;
     int sonWidth = 0;
     for(int i=0;i<sons.size();i++){
       sonWidth += sons.get(i).getWidth(g) + nodeSep;
     }
     return Math.max(myWidth, sonWidth);
  }

  int getHeight(Graphics2D g){
     if(sons.size()==0){
       return nodeHeight;
     }
     int sh = 0; 
     for(int i=0;i<sons.size();i++){
        sh = Math.max(sh,sons.get(i).getHeight(g));
     }
     return sh + vSep + nodeHeight;
  }  

  public Rectangle2D getBounds(){
     return bounds;
  }


  public void paint(Graphics2D g) {
     paint(g,0,0);
  }

  private int paint(Graphics2D g, int x, int y){
     int w = g.getFontMetrics().stringWidth(label) + 20;
     if(sons.size()==0){
        g.drawRect(x,y,w,nodeHeight);
        g.drawString(label, x+10, y+nodeHeight-1);
        return w;
     }
     int sw = x;
     Vector<Integer> sws = new Vector<Integer>();
     // draw Subtrees
     for(int i=0; i < sons.size();i++){
         int k = sons.get(i).paint(g, sw , y + nodeHeight + vSep);
         sws.add(new Integer(k));
         sw  = sw + k + nodeSep;
     }
     // paint node itself
     int ux;
     int uy;
     int dx = Math.max(0, sw-w);

     g.drawRect(x+dx/2,y,w,nodeHeight);
     g.drawString(label, x+dx/2+10, y+nodeHeight-1);
     ux = x + dx/2 + w/2;
     uy = y + nodeHeight; 
     // draw connections to sons
     int sy = y + nodeHeight + vSep;
     int startX = x;
     for(int i=0;i<sws.size();i++){
        int sx = startX + sws.get(i).intValue() / 2;
        g.drawLine(ux,uy,sx,sy);
        startX = startX + sws.get(i).intValue() + nodeSep;
     }
     return Math.max(sw,w);
  } 

  public boolean saveAsLatex(File f){
    return false;
  }

  public boolean readFromString(String treeString){
     ListExpr treeList = new ListExpr();
     if(treeList.readFromString(treeString)!=0){
         return false;
     }
     return readFromList(treeList,true); 
  }

  private boolean  isSymbol(ListExpr sym, String v){
     if(sym.atomType()!=ListExpr.SYMBOL_ATOM){
        return false;
     } else {
        return sym.symbolValue().equals(v);
     }
  }

  public boolean readFromList(ListExpr treeList, boolean isRoot){
     ListExpr value;
     if(isRoot){
       if(treeList.listLength()!=2){
         return false;
       }
       if(!isSymbol(treeList.first(),"optree")){
         return false;
       }
       value = treeList.second();
     } else {
        value = treeList;
     }
     if(value.listLength()!=2){
       return false;
     }
     if(!readLabel(value.first())){
        return false;
     }
     ListExpr sons = value.second();
     while(!sons.isEmpty()){
         OpTreeNode otn = new OpTreeNode();
         if(!otn.readFromList(sons.first(),false)){
           return false;
         }
         this.sons.add(otn);
         sons = sons.rest();
     }
     return true;
  }


  private NodeType getNodeType(ListExpr t){
    if(isSymbol(t,"Pointer")) return NodeType.Pointer;
    if(isSymbol(t,"Object")) return NodeType.Object;
    if(isSymbol(t,"IndirectObject")) return NodeType.IndirectObject;
    if(isSymbol(t,"Operator")) return NodeType.Operator;
    return NodeType.Unknown;
  }

  private String getProp(ListExpr props, String name){
    while(!props.isEmpty()){
      ListExpr prop = props.first();
      props = props.rest();
      if( (prop.listLength()==2) &&
          (isSymbol(prop.first(),name))){
         if(prop.second().isEmpty()){
              return "--";
         }
         return ""+prop.second();
      }
    }
    return "?"+name+"?";
  }

  private boolean readLabel(ListExpr root){
     if(root.listLength()!=2){
        return false;
     }
     NodeType nodeType = getNodeType(root.first());
     if(nodeType==NodeType.Unknown){
       return false;
     }
     ListExpr prop = root.second();
     switch(nodeType){
        case Pointer: label = "Pointer"; break;
        case Object: label = getProp(prop,"symbol");
                     if(label.equals("--")){
                       label = getProp(prop,"typeExpr");
                       String isConst = getProp(prop,"isConstant");
                       if(isConst.trim().equals("TRUE")){
                         label = "const " + label;
                       }
                     } 
                     break;
        case IndirectObject: label = "indirectObject"; break;
        case Operator:  label = getProp(prop,"opName"); break;
        default : label = "--";
     }
     return true;
  }


  private Rectangle2D.Double bounds;
  private Vector<OpTreeNode> sons;
  private String label;
  private static final int nodeSep = 10;
  private static final int nodeHeight = 20;
  private static final int vSep = 20;
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
     if(Tree!=null){
      Tree.paint(g);
     }
 }

  /* move the drawing , values are in pixels*/
  void moveXY(int X,int Y){
     AffineTransform A = new AffineTransform();
     A.setToTranslation(X,Y);
     AT.preConcatenate(A);
     repaint();
  }


  // sets the tree to paint
   void setTree(PNode Tree){
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
 PNode Tree=null;
 // the bounding box of this tree
 Rectangle2D TreeBounds=null;
 // paint without manual zoomung 
 boolean firstPaint=true;
 // fit to window
 boolean fit=true;
 // Dimension for last paint
 Dimension lastDim = null;


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
  ControlPanel.add(EPSButton);
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
  EPSButton.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        JFileChooser FC = new JFileChooser();
        if(FC.showSaveDialog(null)==JFileChooser.APPROVE_OPTION){
            File f = FC.getSelectedFile();
            if( f.exists() && 
              (Reporter.showQuestion("File exists\n Overwrite it ?")!=Reporter.YES)){
                return;
            }
            if(!extern.psexport.PSCreator.export(TPP,f)){
                Reporter.showError("Error in eps export");
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
             TPP.setTree((PNode)Trees.get(i));
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
    ListExpr type = o.toListExpr().first();
    ListExpr LE = o.toListExpr().second(); // we need only the value
    
    PNode pN;
    if(type.atomType()==ListExpr.SYMBOL_ATOM && 
      (type.symbolValue().equals("tree") || isPMType(type))){
       TreeNode N = new TreeNode("",null);
       if(isPMType(type)){
         if(!N.readFromPMType(o.toListExpr())){
           return false;
         }
       } else if(!N.readFrom(LE)){ // error in valueList
         return false;
       } 
       pN = N;
    } else if(type.atomType()==ListExpr.SYMBOL_ATOM &&
             (type.symbolValue().equals("text") )){
       OpTreeNode op = new OpTreeNode();
       if(!op.readFromString(LE.textValue())){
          return false;
       }
       pN = op;
    } else { // b-tree
      BTreeNode bN = new BTreeNode();
      if(!bN.readFrom(LE,0)){
         return false;
      }
      pN = bN;
    }
    // values are correct
    Trees.add(pN);
    SObjects.add(o);
    ChoiceBox.addItem(o.getName());
    ChoiceBox.setSelectedIndex(Trees.size()-1);
    requestFocus();
    return true;
}

/** returns true if the type represents a periodic moving type **/
static boolean isPMType(ListExpr list){
   if(list.atomType()!=ListExpr.SYMBOL_ATOM){
       return false;
   }
   String v = list.symbolValue();
   return v.equals("pmpoint") ||
          v.equals("pmreal");  // extend if needed

}

/** removes an given object */
public void removeObject(SecondoObject o){
   int index = SObjects.indexOf(o);
   if(index < 0)
      return;
   SObjects.removeElementAt(index);
   Trees.removeElementAt(index);
   ChoiceBox.removeItemAt(index);
   if(SObjects.size()==0){
      TPP.setTree(null);
   }
}

public void removeAll(){
   SObjects.clear();
   Trees.clear();
   ChoiceBox.removeAll();
   TPP.setTree(null); 
}

public boolean canDisplay(SecondoObject o){

   ListExpr LE = o.toListExpr();
   if(LE.listLength()!=2){ // not an secondo object (type value)
     return false;
   }
   ListExpr value = LE.second();
   LE = LE.first(); // get the type

   // check for typed btree (btree (rel (...)) Attrname )
   if(LE.atomType()==ListExpr.NO_ATOM){
       if( LE.listLength()==3 && 
           LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
           LE.first().symbolValue().equals("btree")){
          return true;
       }
   }
   if(LE.atomType()!=ListExpr.SYMBOL_ATOM){
      return false;
   }
   if(!LE.symbolValue().equals("tree") && !isPMType(LE) && !LE.symbolValue().equals("btree") && !LE.symbolValue().equals("text")){
      return  false;
   }
   if(LE.symbolValue().equals("text")){
     return value.textValue().startsWith("(optree");
   }
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
private JButton EPSButton = new JButton("EPS");
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


