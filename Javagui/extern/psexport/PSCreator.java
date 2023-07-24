package extern.psexport;


import java.awt.geom.*;
import java.io.*;
import java.awt.*;
import tools.Reporter;
import java.awt.font.*;
import java.util.*;
import java.awt.image.*;
import java.text.*;
import java.awt.image.renderable.*;
import java.util.Vector;

class Int3{
  public Int3(int rgb){
     r = (rgb >> 16) & 0xFF;
     g = (rgb >> 8) & 0xFF;
     b = rgb & 0xFF;
  }

  void readFrom(int rgb){
     r = (rgb >> 16) & 0xFF;
     g = (rgb >> 8) & 0xFF;
     b = rgb & 0xFF;
  }
  int r; int g; int b;
}

 

public class PSCreator {

private static String extend(String hn){
   if(hn.length() < 2) return "0"+hn;
   return  hn;
}


private static void writeBigImage(BufferedImage img, PrintStream out,int x, int y){
    int w = img.getWidth();
    int h = img.getHeight();
    out.println("gsave");
    out.println(x+" "+ y+" translate");
    out.println(w+" "+h+"  scale");
    out.println("" + w + "  " + h + "  8 [" + w  + " 0 0 " + (-h) + " 0 " + h+"]");
    int rowData = 3 * w;
    out.println("{currentfile " + rowData + " string readhexstring pop} bind ");
    out.println("false 3 colorimage");
    Int3 i3 = new Int3(0);
    for(int y1=0;y1<h;y1++){
       for(int x1=0;x1<w;x1++){
          int rgb = img.getRGB(x1,y1);
          i3.readFrom(rgb);
          String r = extend(Integer.toHexString(i3.r));
          String g = extend(Integer.toHexString(i3.g));
          String b = extend(Integer.toHexString(i3.b));
          out.print(r);
          out.print(g);
          out.print(b);
       }
       out.println();
    }
    out.println("grestore");

}



/** Exports a Component to a file as eps graphic */

public static boolean export(Component c, File outFile){
   if(outFile==null){
      return false;
   }
  try{ 
     PrintStream out = new PrintStream(new FileOutputStream(outFile));
     Rectangle  r = c.getBounds(); 
     PSGraphics psc = new PSGraphics(out,(Graphics2D)c.getGraphics(),r.getHeight());
     PSGraphics.lastUsedContext = new PSGraphics.PaintContext();
     psc.writeHeader(r); 
     c.printAll(psc);
     out.close();
     return true;
  }catch(Exception e){
      Reporter.debug(e);
      return false;
  }
}

/** This class is a replacement for the Graphics2D implementation, meaning
  *  all methods are overwritten
  */

private static class PSGraphics extends Graphics2D {

private static AffineTransform at = new AffineTransform();

// the last used context in Postscript
private static PaintContext lastUsedContext;
private static double maxy;
// the used output stream
private PrintStream out = null;
private Graphics2D original;
/** Vector containing PostScript code for some special characters **/
private static Vector charCodes=null;

/** formatting numbers */
static NumberFormat nf;

/** initialize charCodes when class is loaded **/
static{
  DecimalFormatSymbols dfs = new DecimalFormatSymbols() ;
  dfs.setDecimalSeparator('.'); 
  nf = new DecimalFormat("#######.####################",dfs);
  charCodes = new Vector(10);
  // the backslah has to be the first replacement
  charCodes.add(new CharCode('\\',"\\134"));
  charCodes.add(new CharCode((char) 223,"\\337")); // ss
  charCodes.add(new CharCode((char) 228,"\\344")); // ae
  charCodes.add(new CharCode((char) 246,"\\366")); // oe
  charCodes.add(new CharCode((char) 252,"\\374")); // Ue
  charCodes.add(new CharCode((char) 196,"\\304")); // Ae
  charCodes.add(new CharCode((char)214,"\\326")); // Oe
  charCodes.add(new CharCode((char) 220,"\\334")); // Ue
  charCodes.add(new CharCode('(',"\\050"));
  charCodes.add(new CharCode(')',"\\051"));
}

/** replaces what in where by ByWhat */
private static String replaceAll(String what, String where, String ByWhat){
   StringBuffer res = new StringBuffer();
   int lastpos = 0;
   int len = what.length();
   int index = where.indexOf(what,lastpos);
   while(index>=0){
      if(index>0){
         res.append(where.substring(lastpos,index));
      }
      res.append(ByWhat);
      lastpos = index+len;
      index = where.indexOf(what,lastpos);
   }
   res.append(where.substring(lastpos));
   return res.toString();
}


/** Checks whether the given objects are different.
 * Both objects can be null.
 **/
private static boolean different(Object o1, Object o2){
   if( (o1==null ) && (o2!=null)){
      return true;
   }
   if( (o1!=null) && (o2==null)){
        return true;
   }
   if( (o1==null) && (o2==null)){
         return false;
   }
   // both objects are not null
   return !o1.equals(o2);
}


/* This function compares the values of the last used 
 * context with the context of the currently used class.
 * If there is a difference, postscrip code is written to out,
 * writing the current configuration.
 */
private void updateContext(){
   // background
   Color bg = original.getBackground();
   if(different(bg, lastUsedContext.background)){
      writeBackground(bg);
      lastUsedContext.background = bg;
   }

   //paint
   Paint paint = original.getPaint();
   if(different(paint, lastUsedContext.paint)){
      writePaint(paint);
      lastUsedContext.paint = paint;
   }
  
   // color
   Color color = original.getColor();
   if(different(color,lastUsedContext.color)){
     writeColor(color);
     lastUsedContext.color = color;
   }   

   // composite
   Composite composite = original.getComposite();
   if(different(composite, lastUsedContext.composite)){
      writeComposite(composite);
      lastUsedContext.composite = composite;
   }

   // graphgicsconfiguration
   GraphicsConfiguration gc = original.getDeviceConfiguration();
   if(different(gc, lastUsedContext.deviceConfiguration)){
      writeDeviceConfiguration(gc);
      lastUsedContext.deviceConfiguration = gc;
   }
   
   // fontRenderContext  
   FontRenderContext fontRenderContext = original.getFontRenderContext();
   if(different(fontRenderContext, lastUsedContext.fontRenderContext)){
       writeFontRenderContext(fontRenderContext);
       lastUsedContext.fontRenderContext = fontRenderContext;
   }
  
   // renderingHints
   RenderingHints rh = original.getRenderingHints();
   if(different(rh, lastUsedContext.renderingHints)){
       writeRenderingHints(rh);
       lastUsedContext.renderingHints = rh;
   }
   
   // stroke
   Stroke stroke = original.getStroke();
   if(different(stroke, lastUsedContext.stroke)){
       writeStroke(stroke);
       lastUsedContext.stroke = stroke;
   }
   
   // affineTransform
   AffineTransform at = original.getTransform();
   if(different(at, lastUsedContext.affineTransform)){
       writeAffineTransform(at);
       lastUsedContext.clip = null;
   }
  
   // clip
   Shape clip = original.getClip();
   if(different(clip, lastUsedContext.clip)){
       writeClip(clip);
       lastUsedContext.clip = clip;
   }

   // font 
   Font font = original.getFont();
   if(different(font,lastUsedContext.font)){
       writeFont(font);
       lastUsedContext.font = font;
   }
}






/** Changes the backgound color of the postscript image to C.
  * Because postscript does not support changing the background,
  * this method does noting in fact.
  **/
private void writeBackground(Color C){
  // nothing to do. 
  // background handlung done in clear method
}

/** Write the postscript code to change the currently used color of C
  * the the out object.
  **/
private void writeColor(Color C){
  if(C==null){
    Reporter.writeWarning("null color found");
    out.println(" 0 0 0 setrgbcolor");
  } else{
    out.println((nf.format(C.getRed()/255.0)) + " "+ 
              (nf.format(C.getGreen()/255.0)) + " " + 
              (nf.format(C.getBlue()/255.0))+ " setrgbcolor");
  }
}

/** write the given paint as PostScript code.
  * The current implementation only supports colors
  **/
private void writePaint(Paint paint){
   lastUsedContext.paint = paint;
   if(paint==null){
      Reporter.writeError("null paint found");
      return;
   }
   if(! (paint instanceof Color)){
       Reporter.writeError("PSCreator.setPaint supports only colors ");
       Reporter.writeError("the type is " + paint.getClass().getName());
   }
}

/** Write the postscript code for comp.
  * Not implemented yet. Writes only a warning.
  **/
private void writeComposite(Composite comp){
    Reporter.writeWarning("PSCreator: composite not supported");
}

/** Write the DeviceConfiguration to out.
  * Not implemented 
  **/
private void writeDeviceConfiguration(GraphicsConfiguration deviceConfiguration){
    Reporter.writeWarning("PSCreator: deviceConfiguration not supported. ");
}

/** Write the givenm fontRenderContext.
  * Not implemented yet
  **/
private void writeFontRenderContext(FontRenderContext frc){
    Reporter.writeWarning("PSCreator: fontRenderContext not supported ");
}

/** Writes the rendeingHints.
  * NOt implemented yet.
  **/
private void writeRenderingHints(RenderingHints rh){
    Reporter.writeWarning("PSCreator: RenderingHints not supported ");
}

/** Write the given stroke.
  * Only BasiStrokes are supported
  */
private void writeStroke(Stroke s){
   if(s==null){ // set to default stroke
      writeStroke(new BasicStroke());
   }

   if(!(s instanceof BasicStroke)){
      Reporter.writeError("only BasicStrokes are supported by PSCreator");
      return;
   }   

   BasicStroke bs = (BasicStroke)s;
   // set linewidth
   out.println(nf.format(bs.getLineWidth()) +" setlinewidth");
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
   } else {
       out.println(" [] 0 setdash");
   }

}

private void writeAffineTransform(AffineTransform a){
   writeAffineTransform(a,true);
}

private void writeAffineTransform(AffineTransform a1,boolean writeClip){
  if(a1==null){
     Reporter.writeError("null Matrix found");
     out.println("initgraphics");  
  } else{
     AffineTransform a = (AffineTransform) a1.clone();
     if(lastUsedContext.affineTransform==null){
      double[] m = new double[6];
      a.getMatrix(m);
      out.println(" [ "+ nf.format(m[0]) +" " + nf.format(-m[1]) + " " + nf.format(m[2]) + 
                      " " +  nf.format(-m[3]) + " " +
                                   nf.format(m[4]) + " " + nf.format((maxy-m[5])) + " ] concat ");
   //   if(writeClip){
          // adapt the clipping path to the new matrix
  //        writeClip(lastUsedContext.clip);
  //    }
       
    } else {
      try{
       AffineTransform a2 = lastUsedContext.affineTransform.createInverse(); 
       a2.concatenate(a);

       double[] m = new double[6];
       a2.getMatrix(m);
       out.println(" [ "+ nf.format(m[0]) +" " + nf.format(m[1]) + " " + nf.format(m[2]) + " " +  nf.format(m[3]) + " " +
                                   nf.format(m[4]) + " " + nf.format(m[5]) + " ] concat ");
  //     if(writeClip){
          // adapt the cliuppling path to the new matrix
  //        writeClip(lastUsedContext.clip);
  //     }
     }catch(Exception e){
        Reporter.debug(e);
     }
    }

    lastUsedContext.affineTransform=(AffineTransform) a1.clone();
  }
}

private void writeClip(Shape s){
   out.println("initclip");
   if(s==null){
     return;
   }
   
   writePath(s);
   PathIterator i = s.getPathIterator(original.getTransform());
   if(i.getWindingRule()==PathIterator.WIND_EVEN_ODD){
       out.println("clip");
   } else{
       out.println("eoclip");
   }

}

private void writeFont(Font f){
  //out.println("/Helvetica "+ f.getSize2D()+ " selectfont");
  out.println("/"+f.getName()+" " + f.getSize2D()+ " selectfont");
  //Reporter.writeWarning(" PSCreator : fonts are not supported completely, set font to "+f.getName());  
}


private PSGraphics(PrintStream out, Graphics2D original, double maxy){
    PSGraphics.maxy=maxy;
    this.out = out;
    this.original = original;
}

private PSGraphics(PrintStream out, Graphics2D original){
    this.out = out;
    this.original = original;
}



public Graphics create(){
   try{
     return new PSGraphics(out,(Graphics2D)original.create());
   }catch(Exception e){
     Reporter.writeError("Error in PSCreator.create()");
     return null;
   }
}

public Graphics create(int x, int y, int width, int height){
  try {
    return new PSGraphics(out,(Graphics2D) original.create(x,y,width,height));
  } catch(Exception e){
    e.printStackTrace();
    return this;
  }
}

public void addRenderingHints(Map hints){
    original.addRenderingHints(hints);
    Reporter.writeWarning("PSCreator: renderingHints not supported");
}

public void clipRect(int x, int y, int width, int height) {
   original.clipRect(x,y,width,height); 
}

public void clip(Shape s){
   original.clip(s);
}


public void copyArea(int x, int y, int width, int height, int dx, int dy) {
  Reporter.writeWarning("PSCreator.copyArea not implemented");
}

public void dispose(){
   //finalize();
}




public void 	clearRect(int x, int y, int width, int height){
   updateContext();
   Color bg = original.getBackground();
   if(bg==null) return;
   writeColor(bg);  
   fillRect(x,y,width,height); 
   lastUsedContext.color = bg;
}


private boolean mayBeVisible(Shape s){
  if(lastUsedContext.clip==null){
      return true;
  } else{
     Rectangle2D r1 = lastUsedContext.clip.getBounds2D();
     Rectangle2D r2 = s.getBounds2D();
     double x1 = r1.getX();
     double w1 = r1.getWidth();
     double x2 = r2.getX();
     double w2 = r2.getWidth();
     double y1 = r1.getY();
     double y2 = r2.getY();
     double h1 = r1.getHeight();
     double h2 = r2.getHeight();
     if(   (x1 > x2+w2+1)    || // r1 right from r2
           (x2 > x1+w1+1)    || // r1 left of r2
           (y1 > y2+h2+1)    || // r1 above r2
           (y2 > y1+h1+1)    ){ // r2 above r1
        return false;
     } else {
        return true; 
     }
  }
}

public void draw(Shape s){
   if(s==null) return;

   if(mayBeVisible(s)){
      updateContext();
      writePath(s);
      out.println("stroke newpath");
   }
}


public void draw3DRect(int x, int y, int width, int height, boolean raised){
    Paint p = getPaint();
    Color c = getColor();
    Color brighter = c.brighter();
    Color darker = c.darker();
    setColor(raised ? brighter : darker);
    fillRect(x, y, 1, height + 1);
    fillRect(x + 1, y, width - 1, 1);
    setColor(raised ? darker : brighter);
    fillRect(x + 1, y + height, width, 1);
    fillRect(x + width, y, 1, height);
    setPaint(p);

}

public void drawArc(int x, int y, int width, int height, 
                    int startAngle, int arcAngle) {
   draw(new Arc2D.Double(x,y,width,height,startAngle,arcAngle,Arc2D.OPEN));
}

public void drawBytes(byte[] data, int offset, int length, int x, int y){
   Reporter.writeWarning("PSCreator.drawBytes not implemented");
}

public void drawChars(char[] data, int offset, int length, int x, int y) {
   String res = "";
   for(int i = offset; i<offset+length;i++){
      res+=data[i];
   }
   drawString(res,x,y);
}


public void drawGlyphVector(GlyphVector g, float x, float y){
   Reporter.writeWarning("PSCreator.drawGlyhVector not implemented");
}

private Dimension getImageSize(Image img){
   int w = img.getWidth(null);
   int h = img.getHeight(null);
   return new Dimension(w,h);
}

public void  drawImage(BufferedImage img, BufferedImageOp op, int x, int y){
  System.out.println("drawImage/1 called width image of size " + getImageSize(img));
  // Rectangle2D r = op.getBounds2D();
   BufferedImage i2 = img;
   writeImage(i2,x,y);
}

private BufferedImage convertImage(Image img){
   int w = img.getWidth(null);
   int h = img.getHeight(null);
   BufferedImage bi = new BufferedImage(w,h,BufferedImage.TYPE_INT_RGB);
   Graphics g = bi.getGraphics();
   g.drawImage(img,0,0,null);
   return bi;
}


public boolean drawImage(Image img, AffineTransform xform, ImageObserver obs){
   //System.out.println("drawImage/2 called with image of size " + getImageSize(img));
   out.println("gsave");
   double sx = xform.getScaleX();
   double sy = xform.getScaleY();
   int w = img.getWidth(null);
   int h = img.getHeight(null);
   int wt = (int)(w*sx);
   if(wt<(w*sx)) wt++;
   int ht = (int)(h*sy);
   if(ht<(h*sy)) ht++;

   BufferedImage img2 = new BufferedImage(wt,ht, BufferedImage.TYPE_INT_RGB);
   Graphics2D g = (Graphics2D) img2.getGraphics();
   g.translate(0,ht);
   g.scale(1,-1);
   g.drawImage(img,0,0,wt,ht,null);
   
   writeImage(img2,(int)(xform.getTranslateX()),(int)(xform.getTranslateY()));
   out.println("grestore");

   return false;
}

public boolean drawImage(Image img, int x, int y, Color bgcolor, 
                      ImageObserver observer) {
   System.out.println("drawImage/3 called with image of size " + getImageSize(img));
   writeImage(convertImage(img),x,y);
   return false;
}

public boolean drawImage(Image img, int x, int y, ImageObserver observer) {
   //System.out.println("drawImage/4 called with image of size " + getImageSize(img));
   writeImage(convertImage(img),x,y);
   return  false;
}

public boolean drawImage(Image img, int x, int y, int width, int height,
                         Color bgcolor, ImageObserver observer) {
   System.out.println("drawImage/5 called with image of size " + getImageSize(img));
   BufferedImage img1 = new BufferedImage(width,height,BufferedImage.TYPE_INT_RGB);
   Graphics g = img1.getGraphics();
   boolean res = g.drawImage(img1,0,0,width,height,bgcolor,null);
   writeImage(img1,x,y);
   return res;
}

public boolean drawImage(Image img, int x, int y, int width, int height, 
               ImageObserver observer){
   System.out.println("drawImage/6 called with image of size " + getImageSize(img));
   Image img2 = img.getScaledInstance(width,height,Image.SCALE_DEFAULT);
   writeImage(convertImage(img2),x,y);
   return false;
}

public boolean drawImage(Image img, int dx1, int dy1, int dx2, 
                         int dy2, int sx1, int sy1, int sx2, int sy2, 
                         Color bgcolor, ImageObserver observer){
   int w = dx2-dx1;
   int h = dy2-dy1;
   BufferedImage img1 = new BufferedImage(w,h,BufferedImage.TYPE_INT_RGB);
   Graphics2D g = (Graphics2D)img1.getGraphics();
   boolean res = g.drawImage(img,0,0,w,h,sx1,sy1,sx2,sy2,bgcolor,null);
   writeImage(img1,dx1,dy1);
   return res;
}

public boolean 	drawImage(Image img, int dx1, int dy1, int dx2, int dy2, 
                          int sx1, int sy1, int sx2, int sy2, ImageObserver observer) {

   //System.out.println("drawImage/8 called with image of size " + getImageSize(img));

   int w = dx2-dx1;
   int h = dy2-dy1;
   BufferedImage img1 = new BufferedImage(w,h,BufferedImage.TYPE_INT_RGB);
   Graphics2D g = (Graphics2D)img1.getGraphics();
   boolean res = g.drawImage(img,0,0,w,h,sx1,sy1,sx2,sy2,null);
   writeImage(img1,dx1,dy1);
   return res;
}

public void drawLine(int x1, int y1, int x2, int y2) {
   draw(new Line2D.Double(x1,y1,x2,y2));
}

public void drawOval(int x, int y, int width, int height){
   draw(new Ellipse2D.Double(x,y,width,height));
}

public void drawPolygon(int[] xPoints, int[] yPoints, int nPoints){
  if(nPoints<3){
    return;
  }
  GeneralPath gp = new GeneralPath();
  gp.moveTo(xPoints[0],yPoints[0]);
  for(int i=1;i<nPoints; i++){
     gp.lineTo(xPoints[i],yPoints[i]);
  }
  gp.closePath();
  draw(gp); 
}

public void drawPolygon(Polygon p){
   draw(p);
}

public void drawPolyline(int[] xPoints, int[] yPoints, int nPoints) {
  if(nPoints<3){
    return;
  }
  GeneralPath gp = new GeneralPath();
  gp.moveTo(xPoints[0],yPoints[0]);
  for(int i=1;i<nPoints; i++){
     gp.lineTo(xPoints[i],yPoints[i]);
  }
  draw(gp); 
}

public void drawRect(int x, int y, int width, int height){
   draw(new Rectangle(x,y,width,height));
}

public void drawRoundRect(int x, int y, int width, int height, 
                          int arcWidth, int arcHeight){
  draw(new RoundRectangle2D.Double(x,y,width,height,arcWidth,arcHeight));
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

/** Replaces s by a string which can used in a PostScript show command.
  **/
private String encodeString(String source){
   String res = source;
   for(int i=0;i<charCodes.size();i++){
      CharCode cc = (CharCode) charCodes.get(i);
      res = replaceAll(cc.getChar(),res,cc.getCode());
   }    
   return "("+res+")";
}


public void drawString(String s, float x, float y){
  /* In Java Component display functions
   * strings seems to have a special treatment to 
   * avoid displaying strings mirrowdd at the y axis.
   * Here, i try to simulate this treatment.
   */

  updateContext(); // switch to the currently used context

  AffineTransform af = lastUsedContext.affineTransform; // get the transformation
  double[] m = new double[6];
  af.getMatrix(m);
  
  // and mirror the string itself
  m[2] = -m[2];  
  m[3] = -m[3];
  // mirrow in y direction
  // set the used clip for the new matrix 
  writeClip(lastUsedContext.clip);
  writeAffineTransform(new AffineTransform(m),false);
  // show the string itself
  out.println("newpath");
  out.println(nf.format(x) + " " + nf.format(-y) + " moveto");
  out.println(encodeString(s)+"  show");
  // return to the old clip
  writeAffineTransform(af);

}

public void drawString(String str, int x, int y) {
   drawString(str,(float)x,(float)y);
}


public void fill(Shape s){
  updateContext();
  if(mayBeVisible(s)){
    writePath(s);
    PathIterator i = s.getPathIterator(original.getTransform());
    if(i.getWindingRule()==PathIterator.WIND_EVEN_ODD){
         out.println("fill");
    } else{
         out.println("eofill");
    }
  }
}

public void fill3DRect(int x, int y, int width, int height, boolean raised){
    Paint p = getPaint();
    Color c = getColor();
    Color brighter = c.brighter();
    Color darker = c.darker();
    if (!raised) {
       setColor(darker);
    } else if (p != c) {
        setColor(c);
    }

    fillRect(x+1, y+1, width-2, height-2);
    setColor(raised ? brighter : darker);
    fillRect(x, y, 1, height);
    fillRect(x + 1, y, width - 2, 1);
    setColor(raised ? darker : brighter);
    fillRect(x + 1, y + height - 1, width - 1, 1);
    fillRect(x + width - 1, y, 1, height - 1);
    setPaint(p);
}

public void fillArc(int x, int y, int width, int height, int startAngle,
                    int arcAngle){
   fill(new Arc2D.Double(x,y,width,height,startAngle,arcAngle,Arc2D.OPEN));
}

public void fillOval(int x, int y, int width, int height){
   fill(new Ellipse2D.Double(x,y,width,height));
}

public void fillPolygon(int[] xPoints, int[] yPoints, int nPoints){
  if(nPoints<3){
    return;
  }
  GeneralPath gp = new GeneralPath();
  gp.moveTo(xPoints[0],yPoints[0]);
  for(int i=1;i<nPoints; i++){
     gp.lineTo(xPoints[i],yPoints[i]);
  }
  gp.closePath();
  fill(gp); 
}

public void fillPolygon(Polygon p){
   fill(p);
}

public void fillRect(int x, int y, int width, int height){
    fill(new Rectangle(x,y,width,height));
}

public void fillRoundRect(int x, int y, int width, int height, 
                          int arcWidth, int arcHeight){
   fill(new RoundRectangle2D.Double(x,y,width,height,arcWidth,arcHeight));
}

public Shape getClip(){
   return original.getClip();
}

public Rectangle getClipBounds(){
   return original.getClipBounds(); 
}

public Rectangle getClipBounds(Rectangle r){
   return original.getClipBounds();
}

public Color getColor(){
   return original.getColor();
}

public Font getFont(){
   return original.getFont();
}

public FontMetrics getFontMetrics(){
   return original.getFontMetrics();
}

public FontMetrics getFontMetrics(Font f){
   return original.getFontMetrics(f);
}

public  boolean hitClip(int x, int y, int width, int height) {
   return original.hitClip(x,y,width,height);
}


public Color getBackground(){
  return original.getBackground();
}

public Composite getComposite(){
   return original.getComposite();
}

public GraphicsConfiguration getDeviceConfiguration(){
    return original.getDeviceConfiguration();
}

public FontRenderContext getFontRenderContext(){
   return  original.getFontRenderContext();
}

public Paint getPaint(){
  return original.getPaint();
}

public Object getRenderingHint(RenderingHints.Key hintKey){
    return original.getRenderingHint(hintKey);
}

public RenderingHints getRenderingHints(){
    return original.getRenderingHints();
}

public Stroke getStroke(){
   return original.getStroke();
}

public AffineTransform getTransform(){
    return original.getTransform();
}


public boolean hit(Rectangle rect, Shape s, boolean onStroke){
     return original.hit(rect,s,onStroke);
}

public void rotate(double theta) {
   original.rotate(theta);
}

public  void rotate(double theta, double x, double y) {
   original.rotate(theta,x,y);;
}

public void scale(double sx, double sy) {
     original.scale(sx,sy);
}

public void setBackground(Color C){
   original.setBackground(C);
}

public void setClip(int x, int y, int width, int height){
    original.setClip(x,y,width,height);  
}

public void setClip(Shape s){
   original.setClip(s);
}

public void setColor(Color C){
   original.setColor(C);
}


public void setComposite(Composite comp){
   original.setComposite(comp);
}

public void setFont(Font f){
   original.setFont(f);
}

public void setPaintMode() {
   original.setPaintMode();
}

public void setXORMode(Color C){
    original.setXORMode(C);
}


public void setPaint(Paint paint){
   original.setPaint(paint);
}

public void setRenderingHint(RenderingHints.Key hintKey, Object hintValue) {
   original.setRenderingHint(hintKey,hintValue);
}

public void setRenderingHints(Map hints){
   original.setRenderingHints(hints);
}

public void setStroke(Stroke s){
  original.setStroke(s); 
}

public void setTransform(AffineTransform af){
    original.setTransform(af);
}

public void transform(AffineTransform af){
   original.transform(af);
}


public void shear(double shx, double shy){
   original.shear(shx,shy);
}

public void translate(double tx, double ty) {
   original.translate(tx,ty);
}

public void translate(int tx, int ty){
   original.translate(tx,ty);
}


public void writeHeader(Rectangle2D bounds){
  out.println("%!PS-Adobe-2.0 EPSF-2.0");
  out.println("%%Creator: Secondo's Javagui");
  out.println("%%Title: Exported Graphic");
//  out.println("%%BoundingBox: "+ bounds.getX()+ " " + bounds.getY()+ " "+
//                                (bounds.getWidth()+bounds.getX())+ " " +
//                                (bounds.getHeight()+bounds.getY()));
  out.println("%%BoundingBox: 0 0 "+ ((int)bounds.getWidth()) + " " + ((int)bounds.getHeight()));
  out.println("%%EndComments");
  out.println("% replace the standard Helvetica font by the ISO Latin 1 encoding ");
  out.println("/Helvetica findfont");
  out.println("dup length dict begin");
  out.println(" { 1 index /FID ne");
  out.println("          {def }");
  out.println("          {pop pop}");
  out.println("          ifelse");
  out.println("  }forall");
  out.println("/Encoding ISOLatin1Encoding def");
  out.println("currentdict");
  out.println("end");
  out.println("/Helvetica exch definefont pop");



  out.println("/Helvetica 12 selectfont");
  this.bounds = bounds;

}

/*public void finalize(){
}
*/


private void writePath(Shape s){
  try{
   out.println("newpath");
   PathIterator it =  s.getPathIterator(new AffineTransform());
 
   boolean first = true;
   double[] points = new double[6];
   while(!it.isDone()){
      
      int c = it.currentSegment(points);
      switch(c){
        case PathIterator.SEG_MOVETO:
                out.println(nf.format((points[0])) + " " + nf.format((points[1])) + " moveto");
                break;
        case  PathIterator.SEG_LINETO:
                 out.println((nf.format(points[0])) + " " + nf.format((points[1])) + " lineto");
                 break;
        case PathIterator.SEG_QUADTO:
                 Reporter.writeWarning("PSCreator SEG_QUADTO not supported");
                 out.println(nf.format(points[2]) + " " + nf.format(points[3]) + " lineto");
                 break;
        case PathIterator.SEG_CUBICTO:
                 out.println(nf.format(points[0]) +" " +  nf.format(points[1])
                             +" " + nf.format(points[2])+" "+ nf.format(points[3])+" "+ 
                             nf.format(points[4]) + " " + nf.format(points[5]) + " curveto");
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

private String getHexString(int i){
  String res = Integer.toHexString(i);
  if(res.length()<2){
     res = "0"+res;
  }
  return res;
   
}

private void writeImage(BufferedImage bi,int x , int y){

   updateContext();

   // get the image data
   int w = bi.getWidth();
   int h = bi.getHeight();
   if(w>0 || h >0){
     writeBigImage(bi,out,x,y);
     return;
   }

  

   int[] data = bi.getRGB(0,0,w,h,null,0,w);
   // write the image code
   out.println("gsave");
   out.println("/DeviceRGB setcolorspace");
   out.println(x+" "+ y+" translate");
   out.println(w+" "+h+"  scale");
   out.println(w+" "+ h + " 8 ");
   // print the matrix to use
   out.println("["+w+" 0 0 "+h+" 0 0 ]");
   // print the image data
   out.println("{<");
   for(int i=0;i<data.length;i++){
      // build the PostScript String
      Color C = new Color(data[i]);
      String res = "";
      res += getHexString(C.getRed());
      res += getHexString(C.getGreen());
      res += getHexString(C.getBlue());
      if((i+1)%66==0){
         out.println(res);
      } else {
         out.print(res); 
      }
   }
   out.println("\n>} false 3 colorimage");
   out.println("grestore");
}



/** class holding the PostScript code for a single special characters */

private static class CharCode{
  private CharCode(char c, String code){
     this.c = ""+c;
     this.code = code;
  }
  
  private String getChar(){ return c;}
  private String getCode(){ return code;}

  String c;
  String code;

}

/** Class holding the whole graphics context */

private static class PaintContext {
  private Color background = null;
  private Color color = null;
  private Paint paint = null;
  private Composite composite = null;
  private GraphicsConfiguration  deviceConfiguration =  null;
  private FontRenderContext fontRenderContext = null;
  private RenderingHints renderingHints = null;
  private Stroke stroke = null;
  private AffineTransform affineTransform = null;
  private Shape clip = null;
  private Font font = null;
}

  Rectangle2D bounds;

}

}
