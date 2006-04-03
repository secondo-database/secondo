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


package  viewer.hoese;

import  java.awt.*;
import  java.awt.geom.*;
import  javax.swing.*;
import  java.awt.image.*;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;
import java.util.Properties;
import tools.Reporter;

/**
 * A graph. object can have several display attributes. All possible attributes are collected in one category which
 * is associated with the object.
 */
public class Category
    implements Cloneable {
/** Some constants for possible dash-patterns */
  private static float[][] dash =  {
    { 2.0f, 2.0f }, 
    { 4.0f, 2.0f, 2.0f, 2.0f },
    { 8.0f, 4.0f },
    { 8.0f, 4.0f, 4.0f, 4.0f },
    { 4.0f, 4.0f },
    { 12.0f, 4.0f, 4.0f, 4.0f },
    { 12.0f, 4.0f, 8.0f, 4.0f }
  };
/** The name of a category */
  String name;
/** Color of the outline */
  Color LineColor;
/** No. of the dash pattern in the dash-array */
  int LineStyle;
/** cap Style */
  int capStyle = BasicStroke.CAP_ROUND;
/** join style */
  int joinStyle = BasicStroke.JOIN_ROUND;

/** Width of the outline */
  float LineWidth;
/** The style how areas are filled, maybe a color, texture, gradient */
  Paint FillStyle;
/** Transparency is set with the AlphaStyle */
  Composite AlphaStyle;
/** The size of a point */
  double PointSize;
/** Draw a point as rectangle */
  boolean PointasRect;
/** A buffered image for texture paint */
  BufferedImage TextureImage;
/** The BasicStroke constructed out of the LineStyle-dash */
  BasicStroke LineStroke;

/** The filename of a texture */
  String IconName;
/** the path for all Textures */
  static String TexturePath;

/** Default-Category */
  private static Category defCat;

/* The following variables control the color of this
   object at displaying
 */
 private double minValue = 0;
 private double maxValue = 0;
 private Color minColor = Color.GREEN;
 private Color maxColor = Color.RED;

 /** represents a attribute depending render method **/
 public static final int RENDER_COLOR=0;
 /** represents a attribute depending render method **/
 public static final int RENDER_POINTSIZE=1;
 /** represents a attribute depending render method **/
 public static final int RENDER_LINEWIDTH=2;

 /** the attribute depending render method used in this category **/
 private int attrRenderMethod = RENDER_COLOR;
 
 /** the minimum pointsize used in reference depending rendering **/
 private static final int minPointSize = 1;
 /** the maximum pointsize used in reference depending rendering **/
 private static final int maxPointSize = 32;
 /** the minimum linewidth used for reference depending rendering **/
 private static final int minLinewidth=1;
 /** the maximum linewidth used for reference depending rendering **/
 private static final int maxLinewidth=20; 


/** The stroke for the selection */
  public static Stroke SelectionStroke;

  /**
   *
   * @return The filename for a texture
   */
  public String getIconName () {
    return  IconName;
  }


 /**
   * @return the complete path to the used icon
   */
   public String getIconPath(){
      if (IconName.equals(""))
          return "";
      else
          return TexturePath + IconName;
   }

  /**
    * @return the path to all textures
    */
  public static String getTexturePath(){
      return TexturePath;
  }

  /**
   *
   * @return The name of the category
   * @see <a href="Categorysrc.html#getName">Source</a>
   */
  public String getName () {
    return  name;
  }

  /**
   *
   * @return The outline-color of the cat.
      * @see <a href="Categorysrc.html#">Source</a>
   */
  public Color getLineColor () {
    return  LineColor;
  }

  /**
   *
   * @return the linestyle of the cat
      * @see <a href="Categorysrc.html#">Source</a>
   */
  public int getLineStyle () {
    return  LineStyle;
  }

 /** returns the current cap style */
  public int getCapStyle(){
     return capStyle;
  }

  /** returns the current join style */
  public int getJoinStyle(){
     return joinStyle;
  }


  /**
   *
   * @return The width of the outline
      * @see <a href="Categorysrc.html#">Source</a>
   */
  public float getLineWidth (RenderAttribute renderAttribute, double time) {
    if(renderAttribute==null || attrRenderMethod!=RENDER_LINEWIDTH){
       return  LineWidth;
    }
    double value = renderAttribute.isDefined(time)?renderAttribute.getRenderValue(time):minValue;
    if(value<minValue){
        value = minValue;
    }
    if(value>maxValue){
        value = maxValue;
    }
    double delta = (maxValue>minValue)?(value-minValue)/ (maxValue-minValue):0;
    float res = (float) (minLinewidth+delta*(maxLinewidth-minLinewidth));
    return res;
    
  }

  /** Returns the Fillstyle of this category.
    * If renderAttribute is not null, the fillstyle will be 
    * computed from it using the given time. Otherwise, the 
    * fillstyle stored in this catagory is returned.
   */
  public Paint getFillStyle(RenderAttribute renderAttribute, double time){
    if(renderAttribute==null || attrRenderMethod!=RENDER_COLOR){
        return  FillStyle;
    } else {
        double value = renderAttribute.isDefined(time)?renderAttribute.getRenderValue(time):minValue;
        if(value<minValue){
             value = minValue;
        }
        if(value>maxValue){
             value = maxValue;
        }
        double r1 = minColor.getRed();
        double g1 = minColor.getGreen();
        double b1 = minColor.getBlue();
        double r2 = maxColor.getRed();
        double g2 = maxColor.getGreen();
        double b2 = maxColor.getBlue();
        double delta = (maxValue>minValue)?(value-minValue)/ (maxValue-minValue):0;
        int r = (int)(r1 + delta*(r2-r1));
        int g = (int)(g1 + delta*(g2-g1));
        int b = (int)(b1 + delta*(b2-b1));
        return new Color(r,g,b);
    }
  }

  /**
   *
   * @return A composite with the transparency
   * @see <a href="Categorysrc.html#getAlphaStyle">Source</a>
   */
  public Composite getAlphaStyle () {
    return  AlphaStyle;
  }

  /**
   *
   * @return The size of a point
   * @see <a href="Categorysrc.html#getPointSize">Source</a>
   */
  public double getPointSize (RenderAttribute renderAttribute, double time) {
    if(renderAttribute==null || attrRenderMethod!=RENDER_POINTSIZE){
       return  PointSize;
    }
    double value = renderAttribute.isDefined(time)?renderAttribute.getRenderValue(time):minValue;
    if(value<minValue){
        value = minValue;
    }
    if(value>maxValue){
        value = maxValue;
    }
    double delta = (maxValue>minValue)?(value-minValue)/ (maxValue-minValue):0;
    return (minPointSize+delta*(maxPointSize-minPointSize));
  }

  /**
   *
   * @return True if rectangle point
   * @see <a href="Categorysrc.html#getPointAsRect">Source</a>
   */
  public boolean getPointasRect () {
    return  PointasRect;
  }

  /** gets the minimum value for changing color **/
  double getMinValue(){
      return minValue;
  }   

  /** gets the maximum value for changing color **/
  double getMaxValue(){
      return maxValue;
  } 

  /** sets the  valid range for changing color **/
  void setValueRange(double min, double max){
     if(min<=max){
        minValue=min;
        maxValue=max;
     }else{
        maxValue=min;
        minValue=max;
     }
  }

  /** sets the range for colors **/
  void setColorRange(Color minColor, Color maxColor){
     this.minColor=minColor;
     this.maxColor=maxColor;
  }

  /** sets the method used for attribute depending rendering **/
  boolean setRenderMethod(int method){
    if(method<0 || method>RENDER_LINEWIDTH){
        return false;
    }
    attrRenderMethod=method;
    return true;
  }

  /** returns the method used for attribute dependend rendering **/
  int getRenderMethod(){
     return attrRenderMethod;
  }


  /**
   * Sets the cats name
   * @param s Name as string
   * @see <a href="Categorysrc.html#setName">Source</a>
   */
  public void setName (String s) {
    name = s;
  }

  /**
   * Sets the fileName for a texture
   * @param Name the FileName as string
   */
  public void setIconName (String Name) {
    IconName = Name;
  }

  /**
    * sets the path for all Textures
    */
  public static void setTexturePath( String Path){
     Properties P = System.getProperties();
     Path = Path.trim();
     String FS = P.getProperty("file.separator");
     if(!Path.endsWith(FS))
         Path = Path + FS;
      TexturePath = Path;
  }


  /**
   * Sets the outline-color for a cat
   * @param c A color as Color-object
   * @see <a href="Categorysrc.html#setLineColor">Source</a>
   */
  public void setLineColor (Color c) {
    LineColor = c;
  }

  /**
   * Sets the linestyle. The LineStroke is generated out of width and style-no. i
   * @param i
   * @see <a href="Categorysrc.html#setLineStyle">Source</a>
   */
  public void setLineStyle (int i) {
    if (i == 0)
      LineStroke = new BasicStroke(LineWidth,capStyle,joinStyle);
    else
      LineStroke = new BasicStroke(LineWidth,capStyle,joinStyle, 10.0f, dash[i - 1], 0.0f);
    LineStyle = i;
  }

  /**
    * sets the Line style 
    */
   public void setLineStyle(int dash, int cap, int join, double width){
      LineWidth = (float) width;
      if(dash == 0)
         LineStroke = new BasicStroke(LineWidth,cap,join);
      else
         LineStroke = new BasicStroke(LineWidth,cap,join,10.0f, Category.dash[dash - 1], 0.0f);
      LineStyle = dash;
      capStyle = cap;
      joinStyle = join;
   }  

  /**
   * Sets line width
   * @param f line-width as float
   * @see <a href="Categorysrc.html#setLineWidth">Source</a>
   */
  public void setLineWidth (double f) {
    LineWidth = (float) f;
    setLineStyle(LineStyle);
  }

  /**
   * Sets the fill-style
   * @param p A Paint-object as fill-style
   * @see <a href="Categorysrc.html#setFillStyle">Source</a>
   */
  public void setFillStyle (Paint p) {
    FillStyle = p;
  }

  /**
   * Sets the alphastyle e.g. for transparency
   * @param c The alpha-style as Composite
   * @see <a href="Categorysrc.html#setAlphaStyle">Source</a>
   */
  public void setAlphaStyle (Composite c) {
    AlphaStyle = c;
  }

  /**
   * Sets pointsize to d
   * @param d A size as double
   * @see <a href="Categorysrc.html#setPointSize">Source</a>
   */
  public void setPointSize (double d) {
    PointSize = d;
  }

  /**
   * Sets circle or rectangle point
   * @param b True sets rectangle style
   * @see <a href="Categorysrc.html#setPointasRect">Source</a>
   */
  public void setPointasRect (boolean b) {
    PointasRect = b;
  }
  /** Static constructor create Default-category and Selection-Stroke
   * @see <a href="Categorysrc.html#static">Source</a>
   */
  static {
    float[] dash2 =  {
      6.0f, 1.0f
    };
    SelectionStroke = new BasicStroke(5.0f, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER,
        10.0f, dash2, 0.0f);
    defCat = new Category();
    defCat.name = "Default";
    defCat.LineColor = Color.black;
    defCat.LineWidth = 1.0f;
    defCat.setLineStyle(0);
    //defCat.LineStroke=new BasicStroke(defCat.LineWidth, BasicStroke.CAP_BUTT,
    //    BasicStroke.JOIN_MITER, 10.0f, dash[0], 0.0f);
    defCat.FillStyle = Color.blue;
    /* ImageIcon ii=new ImageIcon("images/duke.gif");
     BufferedImage bi = new BufferedImage(ii.getIconWidth(), ii.getIconHeight(),
     BufferedImage.TYPE_INT_ARGB);
     Graphics2D big = bi.createGraphics();
     big.drawImage(ii.getImage(),0,0,null);
     Rectangle r = new Rectangle(0,0,ii.getIconWidth(),ii.getIconHeight());
     defCat.FillStyle=new TexturePaint(bi,r);						      * @see <a href="Categorysrc.html#">Source</a>
   */
    //defCat.TextureImage=bi;
    defCat.AlphaStyle = AlphaComposite.getInstance(AlphaComposite.SRC_OVER);
    defCat.PointSize = 16;
    defCat.PointasRect = false;
  }


  /** converts a color into a list of integer values **/
  private static ListExpr color2list(Color color){
     return ListExpr.threeElemList(
                ListExpr.intAtom(color.getRed()),
                ListExpr.intAtom(color.getGreen()),
                ListExpr.intAtom(color.getBlue())
             );
  }

  /** Converts a listExpr into a color value.
    * If the list format is not valid, null is returned.
    **/
  private static Color list2Color(ListExpr list){
    if(list.listLength()!=3){
       return null;
    }
    if(list.first().atomType()!=ListExpr.INT_ATOM ||
       list.second().atomType()!=ListExpr.INT_ATOM ||
       list.third().atomType()!=ListExpr.INT_ATOM){
         return null;
    }
    return new Color( list.first().intValue(),
                      list.second().intValue(),
                      list.third().intValue());
  }

  /** returns the listexpr defining the refence depending values **/
  private static ListExpr getRefDependendList( Category cat){
     return ListExpr.fiveElemList(
                ListExpr.realAtom(cat.minValue),
                ListExpr.realAtom(cat.maxValue),
                color2list(cat.minColor),
                color2list(cat.maxColor),
                ListExpr.intAtom(cat.attrRenderMethod));
  }

  /** Writes the values from the list to cat. 
    * if the list is not formatted valid, the result will be 
    * false and cat is not canged.
    **/
  private static boolean writeRefDep(ListExpr list, Category cat){
     if(list.listLength()!=5){
       return false;
     }
     if( list.first().atomType()!=ListExpr.REAL_ATOM ||
         list.second().atomType()!=ListExpr.REAL_ATOM ||
         list.fifth().atomType()!=ListExpr.INT_ATOM){
        return false;
     }
     Color c1 = list2Color(list.third());
     Color c2 = list2Color(list.fourth());
     if(c1==null || c2==null){
         return false;
     }
     // format ok
     cat.minValue = list.first().realValue();
     cat.maxValue = list.second().realValue();
     cat.minColor = c1;
     cat.maxColor = c2;
     cat.attrRenderMethod = list.fifth().intValue();
     return true;
  }


  /**
   * Converts a category to a listexpr. Used in session-saving
   * @param cat The cat to convert
   * @return The result as a ListExpr
   */

  public static ListExpr ConvertCattoLE (Category cat) {
    Color c1, c2;
    String Name = "";
    ListExpr l = ListExpr.oneElemList(ListExpr.stringAtom(cat.getName()));
    ListExpr le = ListExpr.append(l, color2list(cat.getLineColor()));
    le = ListExpr.append(le, ListExpr.intAtom(cat.getLineStyle()));
    le = ListExpr.append(le, ListExpr.realAtom(cat.getLineWidth(null,0)));
    le = ListExpr.append(le, ListExpr.boolAtom(cat.getPointasRect()));
    le = ListExpr.append(le, ListExpr.realAtom(cat.getPointSize(null,0)));
    double f = 100.0 - ((AlphaComposite)cat.getAlphaStyle()).getAlpha()*100;
    le = ListExpr.append(le, ListExpr.realAtom(f));
    Paint fillStyle = cat.getFillStyle(null,0); // render attributes not supported
    if (fillStyle instanceof Color) {
      le = ListExpr.append(le, ListExpr.symbolAtom("solid"));
      c1 = (Color)fillStyle;
      c2 = Color.black;
    }
    else if (fillStyle instanceof TexturePaint) {
      le = ListExpr.append(le, ListExpr.symbolAtom("texture"));
      c1 = Color.black;
      c2 = Color.black;
      Name = cat.getIconName();
    }
    else if (fillStyle instanceof GradientPaint) {
      le = ListExpr.append(le, ListExpr.symbolAtom("gradient"));
      c1 = ((GradientPaint)fillStyle).getColor1();
      c2 = ((GradientPaint)fillStyle).getColor2();
    }
    else {
      le = ListExpr.append(le, ListExpr.symbolAtom("nofill"));
      c1 = Color.black;
      c2 = Color.black;
    }
    le = ListExpr.append(le, color2list(c1));
    le = ListExpr.append(le, color2list(c2));

    le = ListExpr.append(le, ListExpr.stringAtom(Name));
    // new version
    le = ListExpr.append(le, ListExpr.intAtom(cat.capStyle));
    le = ListExpr.append(le, ListExpr.intAtom(cat.joinStyle));
    le = ListExpr.append(le, getRefDependendList(cat));
    return  l;
  }

  /**
   * Converts a ListExpr with a category to a Category-object
   * @param le
   * @return The created Category
   * @see <a href="Categorysrc.html#ConvertLEtoCat">Source</a>
   */
  public static Category ConvertLEtoCat (ListExpr le) {
    int len = le.listLength();
    if (len!= 11 && len!=14) {
      Reporter.writeError("Error: No correct category expression: 11 elements needed");
      return  null;
    }
    Category cat = new Category();

    // name
    if (le.first().atomType() != ListExpr.STRING_ATOM){
      return  null;
    }
    cat.setName(le.first().stringValue());
    
    // line color
    le = le.rest();
    Color lc = list2Color(le.first());
    if(lc==null){
       return null;
    }
    cat.setLineColor(lc);
    le = le.rest();

    // line style
    if (le.first().atomType() != ListExpr.INT_ATOM)
      return  null;
    cat.setLineStyle(le.first().intValue());
    le = le.rest();
    
    // line width 
    if (le.first().atomType() != ListExpr.REAL_ATOM)
      return  null;
    cat.setLineWidth(le.first().realValue());
    le = le.rest();

    // point as rect
    if (le.first().atomType() != ListExpr.BOOL_ATOM)
      return  null;
    cat.setPointasRect(le.first().boolValue());
    le = le.rest();
    
    // point size
    if (le.first().atomType() != ListExpr.REAL_ATOM)
      return  null;
    cat.setPointSize((double)le.first().realValue());
    le = le.rest();
    
    // alpha Style
    if (le.first().atomType() != ListExpr.REAL_ATOM)
      return  null;
    double f = -le.first().realValue()/100 + 1.0;
    if (f > 1.0f)
      f = 1.0f;
    if (f < 0.0f)
      f = 0.0f;
    cat.setAlphaStyle(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, (float) f));
    le = le.rest();

    // FillStyle
    if (le.first().atomType() != ListExpr.SYMBOL_ATOM)
      return  null;
    String style = le.first().symbolValue();
    le = le.rest();

    // color 1
    Color Color1 = list2Color(le.first());
    if(Color1==null){
       return null;
    }
    le = le.rest();

    // color 2
    Color Color2=list2Color(le.first());
    if(Color2==null){
       return null;
    }
    le = le.rest();

    // iconname
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  null;
    cat.IconName=le.first().stringValue();

    if(len ==14){ // new version
        le=le.rest();    
        if(le.first().atomType()!=ListExpr.INT_ATOM){
           return null;
        }
        cat.capStyle = le.first().intValue();
        le = le.rest();
        if(le.first().atomType()!=ListExpr.INT_ATOM){
          return null;
        }
        cat.joinStyle = le.first().intValue();
        le = le.rest();
        if(!writeRefDep(le.first(),cat)){
           return null;
        }
    }

    ImageIcon ii = new ImageIcon(TexturePath+cat.IconName);
    if (ii == null)
      style = "nofill";
    if (style.equals("nofill"))
      cat.setFillStyle(null);
    else if (style.equals("solid"))
      cat.setFillStyle(Color1);
    else if (style.equals("gradient"))
      cat.setFillStyle(new GradientPaint(0.0f, 0.0f, Color1, 20.0f, 20.0f,
          Color2, true));
    else if (style.equals("texture")) {
      if (ii.getImageLoadStatus()==MediaTracker.ABORTED ||  ii.getImageLoadStatus()==MediaTracker.ERRORED){
         style="solid";
	 cat.setFillStyle(Color1);
      } else{
        BufferedImage bi = new BufferedImage(ii.getIconWidth(), ii.getIconHeight(),
            BufferedImage.TYPE_INT_ARGB);
        Graphics2D big = bi.createGraphics();
        big.drawImage(ii.getImage(), 0, 0, null);
        Rectangle r = new Rectangle(0, 0, ii.getIconWidth(), ii.getIconHeight());
        cat.setFillStyle(new TexturePaint(bi, r));
     }
     

    }
    else{ // invalid style 
      return  null;
    }
    return  cat;
  }

 /** a Category is identified by name */
 public boolean equals(Object o){
    if(! (o instanceof Category))
       return false;
    else
       return name.equals(((Category)o).name);
 } 

   


  /**
   * 
   * @return The name of this cat as its representation in the combobox
   * @see <a href="Categorysrc.html#toString">Source</a> 
   */
  public String toString () {
    return  name;
  }

  /**
   * 
   * @return The default-cat.
   * @see <a href="Categorysrc.html#getDefaultCat">Source</a> 
   */
  public static Category getDefaultCat () {
    return  defCat;
  }

  /**
   * 
   * @return The LineStroke to render the outline.
   * @see <a href="Categorysrc.html#getLineStroke">Source</a> 
   */
  public BasicStroke getLineStroke (RenderAttribute renderAttribute, double time) {
    if(renderAttribute==null || attrRenderMethod!=RENDER_LINEWIDTH){
       return  LineStroke;
    }
    if(LineStyle!=0){
       return new BasicStroke(getLineWidth(renderAttribute,time),
                              capStyle,joinStyle,10.0f,dash[LineStyle-1],0.0f); 
    } else{
       return new BasicStroke(getLineWidth(renderAttribute,time),capStyle,joinStyle);
  }
    
  }

  /**
   * 
   * @return This duplicated cat as Object
   * @exception CloneNotSupportedException
   * @see <a href="Categorysrc.html#clone">Source</a> 
   */
  public Object clone () throws CloneNotSupportedException {
    return  super.clone();
  }
  /** The string expressions of the dash-patterns */
  public static String[] StrokeStrings =  {
    "------", "........", "-.-.-.-.-", "-- -- --", "-- - -- -", "- - - -", 
        "--- - --- -", "--- -- --- --"
  };
}



