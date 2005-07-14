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


package tools;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.Vector;
import java.awt.image.*;
import java.io.*;
import javax.swing.event.*;
import sj.lang.ListExpr;
import project.WGSGK;



/**
This class provides a little tool for easy creating of tfw files.
Such files a used for georefencing of bitmaps.

**/

public class TFWCreator extends JFrame{

public TFWCreator(){
   addWindowListener(new WindowAdapter(){
      public void windowClosing(WindowEvent evt){
           System.exit(0);
      }
   });

   try{
      FC = new JFileChooser(System.getProperty("user.dir"));
      FCPoints = new JFileChooser(System.getProperty("user.dir"));
      FC = new JFileChooser(System.getProperty("user.dir"));
   }catch(Exception e){
      FC = new JFileChooser();
   }

   getContentPane().setLayout(new BorderLayout());
   pictureView = new PictureView();
   pictureView.setCursor(new Cursor(Cursor.CROSSHAIR_CURSOR));
   SP = new JScrollPane(pictureView);
   getContentPane().add(SP,BorderLayout.CENTER);

   JPanel CoordPanel = new JPanel(new GridLayout(1,2));
   JLabel CoordX = new JLabel("     ");
   JLabel CoordY = new JLabel("     ");
   CoordPanel.add(CoordX);
   CoordPanel.add(CoordY);
   crossSetter.setCurrentTextLabels(CoordX,CoordY);
   getContentPane().add(CoordPanel,BorderLayout.NORTH);
   pictureView.addMouseMotionListener(crossSetter);
   pictureView.addMouseListener(crossSetter);


   imageControlPanel = new JPanel(new GridLayout(20,1));

   imageControlPanel.add(new JLabel(" *** Image ***"));
   imageControlPanel.add(imageName);
   JButton loadButton = new JButton("load");
   imageControlPanel.add(loadButton);
   loadButton.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         if(FC.showOpenDialog(TFWCreator.this)==JFileChooser.APPROVE_OPTION){
             File f = FC.getSelectedFile();
             try{
                 BufferedImage img = javax.imageio.ImageIO.read(f);
                 if(img==null){
                   viewer.MessageBox.showMessage("error in loading image");
                 }else{
                   imageName.setText(f.getName());
                   pictureView.setImage(img);
                   TFWCreator.this.SP.revalidate();
                 }
             } catch (Exception e){
                viewer.MessageBox.showMessage("Error in loading image");
             }

         }
      }
   });
   imageControlPanel.add(new JLabel(" "));
   imageControlPanel.add(new JLabel(" "));
   imageControlPanel.add(new JLabel("--picture----"));
   JLabel P1L = new JLabel("point 1");
   P1L.setOpaque(true);
   P1L.setBackground(Color.cyan);
   imageControlPanel.add(P1L);   
   imageControlPanel.add(new JLabel ("x"));
   imageControlPanel.add(px1);
   imageControlPanel.add(new JLabel ("y"));
   imageControlPanel.add(py1);
   setP1Btn = new JButton("Set");
   setP1Btn.setOpaque(true);
   setP1Btn.setBackground(inActiv);
   imageControlPanel.add(setP1Btn); 
   JLabel P2L = new JLabel("point 2");
   P2L.setOpaque(true);
   P2L.setBackground(Color.MAGENTA);
   imageControlPanel.add(P2L);   
   imageControlPanel.add(new JLabel ("x"));
   imageControlPanel.add(px2);
   imageControlPanel.add(new JLabel ("y"));
   imageControlPanel.add(py2);
   setP2Btn = new JButton("Set");
   setP2Btn.setOpaque(true);
   setP2Btn.setBackground(inActiv);
   imageControlPanel.add(setP2Btn); 

   ActionListener AL = new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          Object src = evt.getSource();
          if(setP1Btn.equals(src)){
                if(setP1Btn.getBackground().equals(activ)){
                   setP1Btn.setBackground(inActiv);
                   crossSetter.setPVAndCross(null,null);
                }else{
                   setP2Btn.setBackground(inActiv);
                   setP1Btn.setBackground(activ);
                   crossSetter.setPVAndCross(pictureView,blueCross);
                   crossSetter.setTextField(px1,py1);
                }
           }
          if(setP2Btn.equals(src)){
                if(setP2Btn.getBackground().equals(activ)){
                   setP2Btn.setBackground(inActiv);
                   crossSetter.setPVAndCross(null,null);
                }else{
                   setP1Btn.setBackground(inActiv);
                   setP2Btn.setBackground(activ);
                   crossSetter.setPVAndCross(pictureView,redCross);
                   crossSetter.setTextField(px2,py2);
                }
           }

       }
   };

  setP1Btn.addActionListener(AL);
  setP2Btn.addActionListener(AL);

  worldControlPanel = new JPanel(new GridLayout(20,1));
  worldControlPanel.add(new JLabel(" --- world ---"));
  worldControlPanel.add(useWGS=new JRadioButton("WGS84"));
  worldControlPanel.add(useBessel=new JRadioButton("Bessel"));
  worldControlPanel.add(useGK=new JRadioButton("GaussKrueger"));
  useWGS.setSelected(true);
  ButtonGroup useProjection = new ButtonGroup();  
  useProjection.add(useWGS);
  useProjection.add(useBessel);
  useProjection.add(useGK); 
  worldControlPanel.add(new JLabel("Meridian :"));
  worldControlPanel.add(meridianTF = new JTextField("2")); 
  JLabel W1L = new JLabel("point 1");
  W1L.setOpaque(true);
  W1L.setBackground(Color.cyan);
  worldControlPanel.add(W1L);   
  worldControlPanel.add(new JLabel ("x"));
  worldControlPanel.add(wx1);
  worldControlPanel.add(new JLabel ("y"));
  worldControlPanel.add(wy1);
  loadP1Btn = new JButton("load");
  worldControlPanel.add(loadP1Btn); 
  JLabel W2L = new JLabel("point 2");
  W2L.setOpaque(true);
  W2L.setBackground(Color.MAGENTA);
  worldControlPanel.add(W2L);   
  worldControlPanel.add(new JLabel ("x"));
  worldControlPanel.add(wx2);
  worldControlPanel.add(new JLabel ("y"));
  worldControlPanel.add(wy2);
  loadP2Btn = new JButton("load");
  worldControlPanel.add(loadP2Btn); 
  ActionListener loadAL = new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        if(FCPoints.showOpenDialog(TFWCreator.this)==JFileChooser.APPROVE_OPTION){
           double x=0;
           double y=0;
           File f = FCPoints.getSelectedFile();
           String FName = f.getName().toLowerCase();
           if(FName.endsWith("pos")){ // assume pos file from 'kompass'
              BufferedReader in=null;
              try{
                in = new BufferedReader(new InputStreamReader(new FileInputStream(f)));
                String Line = in.readLine();
                in.close();
                if(Line==null){
                    viewer.MessageBox.showMessage("Error in loading file ");
                    return;
                }else{
                   if(!Line.startsWith("$GPHOM") || (Line.length()<30)){
                      viewer.MessageBox.showMessage("invalid fileformat");
                      return;
                   }else{  
                       // try to analyse the line
                       double xg = 100*c2i(Line.charAt(7)) +
                                   10*c2i(Line.charAt(8))+
                                   c2i(Line.charAt(9));
                       double xm = 10*c2i(Line.charAt(10))+
                                   c2i(Line.charAt(11)) + // pos 12 is a dot
                                   c2i(Line.charAt(13))/10.0 +
                                   c2i(Line.charAt(14))/100.0 +
                                   c2i(Line.charAt(15))/1000.0 +
                                   c2i(Line.charAt(16))/10000.0;
                       x = xg + xm/60;
                       if(Line.charAt(18)=='W' || Line.charAt(18)=='w')
                           x = -1*x;
                       double yg = 10*c2i(Line.charAt(20))+
                                   c2i(Line.charAt(21));
                       double ym = 10*c2i(Line.charAt(22)) +
                                   c2i(Line.charAt(23)) + // pos 24 is a dot
                                   c2i(Line.charAt(25))/10.0 +
                                   c2i(Line.charAt(26))/100.0 +
                                   c2i(Line.charAt(27))/1000.0 +
                                   c2i(Line.charAt(28))/10000.0; 
                       y = yg+ym/60;
                       char ns = Line.charAt(30);
                       if(ns=='S' || ns=='S')
                           y = -1*y; 
                   }
                }
              } catch(Exception e){
                viewer.MessageBox.showMessage("Error in loading file ");
                try{
                  in.close();
                }catch(Exception e2){}
                return;
              }
           } else{ // assume  point in nested list format
              ListExpr LE = ListExpr.getListExprFromFile(f.getAbsolutePath());
              if(LE==null){
                viewer.MessageBox.showMessage("Error in loading nested List");
                return;
              }else{
                int length = LE.listLength();
                ListExpr value=null;
                if(length==6  && LE.first().atomType()==ListExpr.SYMBOL_ATOM && 
                   LE.first().symbolValue().equals("OBJECT") &&
                   LE.third().atomType()==ListExpr.SYMBOL_ATOM &&
                   LE.third().symbolValue().equals("point")){
                   value = LE.fourth();
                }else if(length==2 && LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
                         LE.first().symbolValue().equals("point")){
                   value = LE.second();
                }else if(length==2){
                   value=LE;
                }else{ // error detected
                   viewer.MessageBox.showMessage("invalid nested list for point type ");
                   return;
                }
                length = value.listLength();
                if(length!=2){
                    viewer.MessageBox.showMessage("invalid list format for a point ");
                    return; 
                }
                Double X = viewer.hoese.LEUtils.readNumeric(value.first());
                Double Y = viewer.hoese.LEUtils.readNumeric(value.second());
                if(X==null || Y==null){
                    viewer.MessageBox.showMessage("invalid list format ");
                    return;
                }     
                x = X.doubleValue();
                y = Y.doubleValue();
              }
           }

           // at this point we have successful loaded the point and stored its coordinates
           // in x and y respectively
           Object src = evt.getSource();
           if(loadP1Btn.equals(src)){
              wx1.setText(""+x);
              wy1.setText(""+y);
           }
           if(loadP2Btn.equals(src)){
              wx2.setText(""+x);
              wy2.setText(""+y);
           }
        }
     }
     private int c2i(char c){
          if(c<'0' || c>'9'){
            System.err.println("character "+c+" is not a digit ");
          }
          return (int) (c-'0');
     }
  };
  loadP1Btn.addActionListener(loadAL);
  loadP2Btn.addActionListener(loadAL);  

 

  JPanel controlPanel = new JPanel(new GridLayout(1,2));
  controlPanel.add(imageControlPanel);
  controlPanel.add(worldControlPanel); 
  getContentPane().add(controlPanel,BorderLayout.EAST);

  JButton createTFW = new JButton("create TFW");
  getContentPane().add(createTFW,BorderLayout.SOUTH);
  
  createTFW.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          // read the values from the textfields 
          double pictureX1=0;
          double pictureX2=0;
          double pictureY1=0;
          double pictureY2=0;
          double worldX1=0;
          double worldX2=0;
          double worldY1=0;
          double worldY2=0;
          int meridian=0;
          try{
             pictureX1 = Double.parseDouble(px1.getText()); 
             pictureX2 = Double.parseDouble(px2.getText());
             pictureY1 = Double.parseDouble(py1.getText());
             pictureY2 = Double.parseDouble(py2.getText());
             worldX1 = Double.parseDouble(wx1.getText()); 
             worldX2 = Double.parseDouble(wx2.getText());
             worldY1 = Double.parseDouble(wy1.getText());
             worldY2 = Double.parseDouble(wy2.getText());
             if(!useGK.isSelected()){
               meridian = Integer.parseInt(meridianTF.getText());
             }
          } catch(Exception e){
              viewer.MessageBox.showMessage("all fields have to contain numerix values");
          }
          if(!useGK.isSelected()){
              Projection.enableWGS(useWGS.isSelected());
              Projection.setMeridian(meridian);  
              java.awt.geom.Point2D.Double P = new java.awt.geom.Point2D.Double();
              if(!Projection.project(worldX1,worldY1,P)){
                   viewer.MessageBox.showMessage("Error in projection of point 1");
                   return;
              }
              worldX1 = P.x;
              worldY1 = P.y;
             if(!Projection.project(worldX2,worldY2,P)){
                  viewer.MessageBox.showMessage("Error in projection of point 2");
                  return;
             }
             worldX2 = P.x;
             worldY2 = P.y;
          }
          // some checks
          if(pictureX1 == pictureX2 || pictureY1 == pictureY2){
               viewer.MessageBox.showMessage("error: points in the picture are axes parallel");
               return;
          }
          if(worldX1 == worldX2 || worldY1 == worldY2){
               viewer.MessageBox.showMessage("error: points in the  world are axes parallel");
               return;
          }

          double dxp = pictureX2-pictureX1;
          double dyp = pictureY2-pictureY1;
          double dxw = worldX2-worldX1;
          double dyw = worldY2-worldY1;

          double sizePixX = dxw / dxp;
          double sizePixY = dyw / dyp;

          double transX = worldX1-pictureX1*sizePixX;
          double transY = worldY1-pictureY1*sizePixY;
          String pre = "      ";
          String FileText= pre+sizePixX+"\n";
          FileText += pre + "0.00000000000000" + "\n";
          FileText += pre + "0.00000000000000" + "\n";
          FileText += pre + sizePixY+"\n";
          FileText += pre + transX+"\n";
          FileText += pre + transY+"\n";  

          String quality="Quality\n";
          if(pictureView.image==null){
            quality += "unavailable";
          }
          else{
             double qy = Math.abs(dyp/pictureView.image.getHeight());
             qy = (double)((int)(qy*10000+0.5))/100;
             double qx= Math.abs(dxp/pictureView.image.getWidth());
             qx = (double)((int)(qx*10000+0.5))/100;
             quality +=" x : "+ qx+" %\n y : " + qy +" %\n"; 
          }
          if(FC.showSaveDialog(TFWCreator.this)==JFileChooser.APPROVE_OPTION){
             PrintStream out=null;
             try{
                out=new PrintStream(new FileOutputStream(FC.getSelectedFile()));
                out.print(FileText);
                out.close();
                viewer.MessageBox.showMessage("File written successfully:\n "+quality);
             }catch(Exception e){
               viewer.MessageBox.showMessage("Error in writing file");
                 try{out.close();}catch(Exception e2){}
             }
          }
       }
  });
 

}




private JPanel imageControlPanel;
private JPanel worldControlPanel;
private PictureView pictureView;
private Cross blueCross = new Cross(0,0,40,Color.BLUE);
private Cross redCross = new Cross(0,0,40,Color.RED);
private JLabel imageName =  new JLabel(".");
private JFileChooser FC;
private JScrollPane SP;
private JTextField px1 = new JTextField("0");
private JTextField py1 = new JTextField("0");
private JTextField px2 = new JTextField("0");
private JTextField py2 = new JTextField("0");
private JTextField wx1 = new JTextField("0");
private JTextField wy1 = new JTextField("0");
private JTextField wx2 = new JTextField("0");
private JTextField wy2 = new JTextField("0");
private CrossSetter crossSetter = new CrossSetter();
private JButton setP1Btn;
private JButton setP2Btn;
private JButton loadP1Btn;
private JButton loadP2Btn;
private Color inActiv = Color.LIGHT_GRAY;
private Color activ = Color.RED;
private JRadioButton useBessel;
private JRadioButton useWGS;
private JRadioButton useGK;
private JTextField meridianTF;
private JFileChooser FCPoints;
private WGSGK Projection = new WGSGK();


private class CrossSetter extends MouseInputAdapter{
   // set the textFields for displaying the mouse coordinates
   public void setCurrentTextLabels(JLabel X, JLabel Y){
       this.xLab = X;
       this.yLab = Y;
   }  

   // sets the Textfields for fixing the mouse coordinates on click
   public void setTextField(JTextField xTF, JTextField yTF){
       this.xTF = xTF;
       this.yTF = yTF;
   }


   // sets the pictureview and the cross to set
   public void setPVAndCross(PictureView pv, Cross c){
      this.pictureview = pv;
      this.cross = c;
   }


   public void mouseMoved(MouseEvent evt){
       if(xLab!=null)
             xLab.setText(""+evt.getX());
       if(yLab!=null)
             yLab.setText(""+evt.getY());
   }

   public void mouseClicked(MouseEvent evt){
      if(evt.getButton()==MouseEvent.BUTTON1){ // set Cross
          if(xTF!=null)
              xTF.setText(""+evt.getX());
          if(yTF!=null)
              yTF.setText(""+evt.getY());
          if(pictureview!=null && cross!=null){
             cross.x = evt.getX();
             cross.y = evt.getY();
             pictureView.addCross(cross);
          }
       }
       if(evt.getButton()==MouseEvent.BUTTON3){
          if(xTF!=null)
              xTF.setText("0");
          if(yTF!=null)
              yTF.setText("0");
          if(pictureview!=null && cross!=null){
             pictureView.removeCross(cross);
          }
          
       }
   }  
 
   JTextField xTF=null;
   JTextField yTF=null;
   JLabel xLab = null;
   JLabel yLab = null;
   PictureView pictureview=null;
   Cross cross = null;
}



/** Class painting an image in its original size. 
  * Additionaly, this class offers the possibility to draw
  * crosses at given positions;
  **/

private class PictureView extends JPanel implements ImageObserver{

   public void paint(Graphics g){
       super.paint(g);
       if(image!=null){
         g.drawImage(image,0,0,this);
       }
       ((Graphics2D)g).setStroke(stroke);
       for(int i=0;i<crosses.size();i++){
          paintCross(g,(Cross)crosses.get(i));
       }
   }

   private void paintCross(Graphics g, Cross c){
      Color oldColor = g.getColor();
      g.setColor(c.color);
      int s2 = c.size/2;
      g.drawLine(c.x-s2,c.y,c.x+s2,c.y);
      g.drawLine(c.x,c.y-s2,c.x,c.y+s2);
      g.setColor(oldColor);
   } 

   public void setImage(BufferedImage img){
       image=img;
       if(image!=null){
          setPreferredSize(new Dimension(img.getWidth(),img.getHeight()));
       }
       repaint(); 
   }
   

    public boolean imageUpdate(Image img, int infoflags, int x, int y, int width, int height){
        if(infoflags== ALLBITS){
          setPreferredSize(new Dimension(width,height));
          repaint();
          return false;
        }
        return true;
    } 

    public void addCross(Cross c){
        if(!crosses.contains(c)){
           crosses.add(c);
        }
        repaint();
    } 

   public void removeCross(Cross c){
       crosses.remove(c);
       repaint();
   }


    private BufferedImage image;
    private Vector crosses = new Vector();
    private Stroke stroke = new BasicStroke(3.0f);

}

private class Cross{
  public Cross(int x, int y, int size, Color color){
     this.x = x;
     this.y = y;
     this.size = size;
     this.color = color;
  }
  public boolean equals(Object o){
      if(!(o instanceof Cross))
          return false;
      Cross c = (Cross)o;
      return (x==c.x  && y==c.y && c.size==size && color.equals(c.color));
  }

  private int x;
  private int y;
  private int size;
  private Color color;

}


public static void main(String[] args){
    TFWCreator tfwc = new TFWCreator();
    tfwc.setSize(640,480);
    tfwc.setVisible(true);
}


}
