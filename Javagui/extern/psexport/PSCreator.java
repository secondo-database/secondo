package extern.psexport;

import java.awt.geom.*;
import java.io.PrintStream;
import java.awt.*;
import tools.Reporter;
import java.awt.font.*;
import java.util.*;
import java.awt.image.*;
import java.text.*;
import java.awt.image.renderable.*;


public class PSCreator extends Graphics2D implements Cloneable{

private PrintStream out = null;
// use white background
private Color background = new Color(1,1,1);
// the current color
private Color color = new Color(1,1,1);
// the last assigned paint
private Paint paint = new Color(0,0,0);
private Composite composite;
private GraphicsConfiguration deviceConfiguration;
private FontRenderContext fontRenderContext;
private Map renderingHints;
private Stroke stroke;
private AffineTransform affineTransform = new AffineTransform();
private Shape clip;
private Font font;
// stored the maximum y value for flip the picture
private double maxy = 1000;


public PSCreator(PrintStream out){
    this.out = out;
}


public void addRenderingHints(Map hints){
    Reporter.writeWarning("PsCreator.writeRenderingHints not implemented");
}

public void 	clearRect(int x, int y, int width, int height){
   if(background==null) return;
   writeColor(background);  
   fillRect(x,y,width,height); 
   writeColor(color);
}

public void clipRect(int x, int y, int width, int height) {
   clip(new Rectangle(x,y,width,height)); 
}

public void clip(Shape s){
   clip = s;
   out.println("initclip");
   if(clip==null){
     return;
   }
   if(true){ // disbale compiler checking for unreachable code
     Reporter.writeWarning("Clipping disabled");
     return;
   }
   writePath(s);
   PathIterator i = s.getPathIterator(affineTransform);
   if(i.getWindingRule()==PathIterator.WIND_EVEN_ODD){
       out.println("clip");
   } else{
       out.println("eoclip");
   }
}

public Graphics create(){
   try{
      return (Graphics) this.clone();
   }catch(Exception e){
     Reporter.writeError("Error in PSCreator.create()");
     return null;
   }
}

public Graphics create(int x, int y, int width, int height){
  Reporter.writeWarning("PSCreator.create not implemeneted");
  return this;
}




public void copyArea(int x, int y, int width, int height, int dx, int dy) {
  Reporter.writeWarning("PSCreator.copyArea not implemented");
}

public void dispose(){
   finalize();
}


public void draw(Shape s){
   writePath(s);
   out.println("stroke");
}

public void draw3DRect(int x, int y, int width, int height, boolean raised){
   Reporter.writeWarning("PSCreator.draw3DRect not implemented");
}

public void drawArc(int x, int y, int width, int height, 
                    int startAngle, int arcAngle) {
   Reporter.writeWarning("PSCreator.drawArc not implemented");
}

public void drawBytes(byte[] data, int offset, int length, int x, int y){
   Reporter.writeWarning("PSCreator.drawBytes not implemented");
}

public void drawChars(char[] data, int offset, int length, int x, int y) {
   out.println("newpath");
   out.println(x + " " + (maxy-y) + "  moveto");
   for(int i=offset; i < length; i++){
     out.println("("+data[i]+") show");
   }
}




public void drawGlyphVector(GlyphVector g, float x, float y){
   Reporter.writeWarning("PSCreator.drawGlyhVector not implemented");
}

public void  drawImage(BufferedImage img, BufferedImageOp op, int x, int y){
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
}

public boolean drawImage(Image img, AffineTransform xform, ImageObserver obs){
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
   return false;
}

public boolean drawImage(Image img, int x, int y, Color bgcolor, 
                      ImageObserver observer) {
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
   return false;
}

public boolean drawImage(Image img, int x, int y, ImageObserver observer) {
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
   return  false;
}

public boolean drawImage(Image img, int x, int y, int width, int height,
                         Color bgcolor, ImageObserver observer) {
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
   return false;
}

public boolean drawImage(Image img, int x, int y, int width, int height, 
               ImageObserver observer){
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
   return false;
}

public boolean drawImage(Image img, int dx1, int dy1, int dx2, 
                         int dy2, int sx1, int sy1, int sx2, int sy2, 
                         Color bgcolor, ImageObserver observer){
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
   return false;
}

public boolean 	drawImage(Image img, int dx1, int dy1, int dx2, int dy2, 
                          int sx1, int sy1, int sx2, int sy2, 
                          ImageObserver observer) {
   Reporter.writeWarning("Reporter.drawImage not implemeneted");
   return false;
}

public void drawLine(int x1, int y1, int x2, int y2) {
   out.println("newpath");
   out.println(x1 + " " + (maxy-y1) + " moveto");
   out.println(x2 + " " + (maxy-y2) + " lineto");
   out.println("stroke");
}

public void drawOval(int x, int y, int width, int height){
   Reporter.writeWarning("PSCreator.drawOval not implemented");
}

public void drawPolygon(int[] xPoints, int[] yPoints, int nPoints){
  out.println("newpath");
  if(nPoints<3) return;
  out.println(xPoints[0]+ " " + (maxy-yPoints[0])+ " moveto");
  for(int i=1;i<nPoints; i++){
     out.println(xPoints[i]+ " " + (maxy-yPoints[i])+ " lineto");
  }
  out.println(xPoints[0]+ " " + (maxy-yPoints[0])+ " lineto");
  out.println("stroke");
}

public void drawPolygon(Polygon p){
   draw(p);
}

public void drawPolyline(int[] xPoints, int[] yPoints, int nPoints) {
  out.println("newpath");
  if(nPoints<3) return;
  out.println(xPoints[0]+ " " + (maxy-yPoints[0])+ " moveto");
  for(int i=1;i<nPoints; i++){
     out.println(xPoints[i]+ " " + (maxy-yPoints[i])+ " lineto");
  }
  out.println("stroke");
}

public void drawRect(int x, int y, int width, int height){
   writePath(new Rectangle(x,y,width,height));
   out.println("stroke");
}

public void drawRoundRect(int x, int y, int width, int height, 
                          int arcWidth, int arcHeight){
    Reporter.writeError("PSCreator.drawRoundRect not implemented");
}



public void drawRenderableImage(RenderableImage img, AffineTransform xform){
   Reporter.writeWarning("Reporter.drawRenderableImage not implemeneted");
}

public void drawRenderedImage(RenderedImage img, AffineTransform xform){
   Reporter.writeWarning("Reporter.drawRenderedImage not implemeneted");
}

public void drawString(AttributedCharacterIterator iterator, float x, float y){
   Reporter.writeWarning("PSCreator.drawString not implemented");
}

public void drawString(AttributedCharacterIterator iterator, int x, int y) {
   Reporter.writeWarning("PSCreator.drawString not implemented");
}

public void drawString(String s, float x, float y){
  out.println("newpath");
  out.println(x + " " + (maxy-y) + " moveto");
  out.println("("+s+")  show");
}

public void drawString(String str, int x, int y) {
  out.println("newpath");
  out.println(x + " " + (maxy-y) + " moveto");
  out.println("("+str+")  show");
}


public void fill(Shape s){
  writePath(s);
  PathIterator i = s.getPathIterator(affineTransform);
  if(i.getWindingRule()==PathIterator.WIND_EVEN_ODD){
       out.println("fill");
  } else{
       out.println("eofill");
  }
}

public void fill3DRect(int x, int y, int width, int height, boolean raised){
   Reporter.writeWarning("PSCreator.fill3DRect not implemented");
}

public void fillArc(int x, int y, int width, int height, int startAngle,
                    int arcAngle){
   Reporter.writeWarning("PSCreator.fillArc not implemented");
}

public void fillOval(int x, int y, int width, int height){
    Reporter.writeWarning("PSCreator.fillOval not implemented");
}

public void fillPolygon(int[] xPoints, int[] yPoints, int nPoints){
  out.println("newpath");
  if(nPoints<3) return;
  out.println(xPoints[0]+ " " + (maxy-yPoints[0])+ " moveto");
  for(int i=1;i<nPoints; i++){
     out.println(xPoints[i]+ " " + (maxy-yPoints[i])+ " lineto");
  }
  out.println("fill");
}

public void fillPolygon(Polygon p){
   fill(p);
}

public void fillRect(int x, int y, int width, int height){
    Rectangle R = new Rectangle(x,y,width,height);
    writePath(R);
    out.println("fill newpath");
}

public void fillRoundRect(int x, int y, int width, int height, 
                          int arcWidth, int arcHeight){
   Reporter.writeWarning("PSCreator.fillRoundRect not implemented");
}

public Shape getClip(){
   return clip;
}

public Rectangle getClipBounds(){
    if(clip==null){
      return null;
    }else{
       return  clip.getBounds();
    }
}

public Rectangle getClipBounds(Rectangle r){
   return clip.getBounds();
}

public Color getColor(){
   return color;
}

public Font getFont(){
   return font;
}

public FontMetrics getFontMetrics(){
   Reporter.writeWarning("PSCreator.getFontMetrics not implemented");
   return null;
}

public FontMetrics getFontMetrics(Font f){
   Reporter.writeWarning("PSCreator.getFontMetrics not implemented");
   return null;
}

public  boolean hitClip(int x, int y, int width, int height) {
   if(clip==null) return true; // no clipping -> all hit
   return true;
}


public Color getBackground(){
  return background;
}

public Composite getComposite(){
   return composite;
}

public GraphicsConfiguration getDeviceConfiguration(){
    return deviceConfiguration;
}

public FontRenderContext getFontRenderContext(){
   return  fontRenderContext;
}

public Paint getPaint(){
  return paint;
}

public Object getRenderingHint(RenderingHints.Key hintKey){
   Reporter.writeWarning("PSCreator.gerRenderingHint not implemeneted");
   return null;
}

public RenderingHints getRenderingHints(){
    Reporter.writeWarning("PSCreator.writeRenderingHints not implemented");
    return null;
}

public Stroke getStroke(){
   return stroke;
}

public AffineTransform getTransform(){
    return affineTransform;
}


public boolean hit(Rectangle rect, Shape s, boolean onStroke){
   Reporter.writeWarning("PSCreator.hit not implemented");
   return true;
}

public void rotate(double theta) {
   Reporter.writeWarning("PSCreator.rotate no implemenetd");
}

public  void rotate(double theta, double x, double y) {
   Reporter.writeWarning("PSCreator.rotate no implemenetd");
}

public void scale(double sx, double sy) {
  out.println(sx + " " + sy + "  scale");
  affineTransform.scale(sx, sy);
}

public void setBackground(Color C){
   background = C;
   Reporter.writeWarning("background not supported by PSCreator");
}

public void setClip(int x, int y, int width, int height){
    clipRect(x,y,width,height);  
}

public void setClip(Shape s){
   clip(s);
}

public void setColor(Color C){
   setPaint(C);
}


public void setComposite(Composite comp){
   composite = comp;
   if(comp==null) return;
   if(comp.equals(composite)) return;
   Reporter.writeWarning("PSCreator.setComposite has no effect");
}

public void setFont(Font f){
   this.font = f;
   Reporter.writeWarning("PSCreator.setFont not implemeted");
}

public void setPaintMode() {
   Reporter.writeWarning("PSCreator.setPaintMode not implemented");
}

public void setXORMode(Color C){
    Reporter.writeWarning("PSCreator.writeXORmode not implemented");
}


public void setPaint(Paint paint){
  // no paint is not accepted
  if(paint==null) return;

  // paint already used
  if(paint.equals(this.paint)) return;
  
  // assign this paint
  this.paint = paint;

  if(paint instanceof Color){
     this.color = (Color) paint;
     writeColor(this.color);
  } else{
     Reporter.writeWarning("PSCreator.setPaint supports only colors ");
  }
}

public void setRenderingHint(RenderingHints.Key hintKey, Object hintValue) {
   Reporter.writeWarning("PSCreator.setRendingHints not implemeneted");
}

public void setRenderingHints(Map hints){
   this.renderingHints = hints;
   Reporter.writeWarning("PSCreator.setRendeinghints not completely implemented");
}

public void setStroke(Stroke s){
   if(s==null) return;
   if(s.equals(this.stroke)) return;
   this.stroke=s;
   if(!(s instanceof BasicStroke)){
      Reporter.writeError("only BasicStrokes are supported by PSCreator");
      return;
   }   
   BasicStroke bs = (BasicStroke)s;
   // set linewidth
   out.println(bs.getLineWidth() +" setlinewidth");
   // set line cap
   int cap = bs.getEndCap();
   int pscap = 0;
   switch(cap) {
      case BasicStroke.CAP_BUTT: pscap = 0;break;
      case BasicStroke.CAP_ROUND: pscap = 1; break;
      case BasicStroke.CAP_SQUARE: pscap = 2; break;
      default: Reporter.writeError("unknown cap style found in PSCreator.setStroke");
   }
   out.println(pscap +" setlinecap");
   // set line join
   int join = bs.getLineJoin();
   int psjoin = 0;
   switch(join){
      case BasicStroke.JOIN_MITER: psjoin=0; break;
      case BasicStroke.JOIN_ROUND: psjoin=1; break;
      case BasicStroke.JOIN_BEVEL: psjoin=2; break;
      default: Reporter.writeError("unknown join style found in PSCreator.setStroke");
   }
   out.println(psjoin+" setlinejoin");
   
   // set dash
   float[] dash = bs.getDashArray();
   float dashphase = bs.getDashPhase();
   if(dash!=null){
       out.print("[");
       for(int i=0; i< dash.length; i++){
          out.print(dash[i]+" ");
       }
       out.println("] " + dashphase + " setdash");
   }

   

}

public void setTransform(AffineTransform af){
  if(af==null) return;
  if(af.equals(affineTransform)) return;
  affineTransform = af;
  Reporter.writeWarning("PSCreator.setTransform not implemented");
}

public void transform(AffineTransform af){
   Reporter.writeWarning("PSCreator.transform not implemented");
}


public void shear(double shx, double shy){
   out.println(shx +" "+ shx + " scale " );
   affineTransform.shear(shx, shy);
}

public void translate(double tx, double ty) {
   out.println(tx + " " + ty + " translate");
   affineTransform.translate(tx, ty);
}

public void translate(int x, int y){
   Reporter.writeWarning("PSCreator.translate not implemented");
}



private void writeColor(Color C){
  out.println((C.getRed()/255.0) + " "+ 
              (C.getGreen()/255.0) + " " + 
              (C.getBlue()/255.0)+ " setrgbcolor");

}




public void writeHeader(Rectangle2D bounds){
  out.println("%!PS-Adobe-2.0 EPSF-2.0");
  out.println("%%Creator: Secondo's Javagui");
  out.println("%%Title: Exported Graphic");
  out.println("%%BoundingBox: "+ bounds.getX()+ " " + bounds.getY()+ " "+
                                (bounds.getWidth()-bounds.getX())+ " " +
                                (bounds.getHeight()-bounds.getY()));
  out.println("%%EndComments");
  out.println("/Helvetica 12 selectfont");
  maxy = bounds.getHeight()-bounds.getY();

}

public void finalize(){
}


public void showPage(){
   out.println("showpage");
}

private void writePath(Shape s){
  try{
   out.println("newpath");
   PathIterator it = s.getPathIterator(affineTransform);
   boolean first = true;
   double[] points = new double[6];
   while(!it.isDone()){
      
      int c = it.currentSegment(points);
      switch(c){
        case PathIterator.SEG_MOVETO:
                out.println(points[0] + " " + (maxy-points[1]) + " moveto");
                break;
        case  PathIterator.SEG_LINETO:
                 out.println(points[0] + " " + (maxy-points[1]) + " lineto");
                 break;
        case PathIterator.SEG_QUADTO:
                 Reporter.writeWarning("PSCreator SEG_QUADTO not supported");
                 out.println(points[2] + " " + (maxy-points[3]) + " lineto");
                 break;
        case PathIterator.SEG_CUBICTO:
                 out.println(points[0] +" " +  (maxy-points[1])
                             +" " + points[2]+" "+ (maxy-points[3])+" "+ 
                             points[4] + " " + (maxy-points[5]) + " curveto");
                 break;
        case PathIterator.SEG_CLOSE:
                 out.println("closepath");
                 break;
        default: Reporter.writeError("unknown path connection detected");
       } 
      it.next();
   }
 } catch(Exception e){
    Reporter.debug(e);
 }
} 



}
