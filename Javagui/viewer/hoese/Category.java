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

/**
 * A graph. object can have several display attributes. All possible attributes are collected in one category which
 * is associated with the object.
 */
public class Category
    implements Cloneable {
/** Some constants for possible dash-patterns */
  private static float[][] dash =  {
    {
      2.0f, 2.0f
    },  {
      4.0f, 2.0f, 2.0f, 2.0f
    },  {
      8.0f, 4.0f
    },  {
      8.0f, 4.0f, 4.0f, 4.0f
    },  {
      4.0f, 4.0f
    },  {
      12.0f, 4.0f, 4.0f, 4.0f
    },  {
      12.0f, 4.0f, 8.0f, 4.0f
    }
  };
/** The name of a category */
  String name;
/** Color of the outline */
  Color LineColor;
/** No. of the dash pattern in the dash-array */
  int LineStyle;
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

  /**
   *
   * @return The width of the outline
      * @see <a href="Categorysrc.html#">Source</a>
   */
  public float getLineWidth () {
    return  LineWidth;
  }

  /**
   *
   * @return The fillstyle as a Paint-object
   * @see <a href="Categorysrc.html#getFillStyle">Source</a>
   */
  public Paint getFillStyle () {
    return  FillStyle;
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
  public double getPointSize () {
    return  PointSize;
  }

  /**
   *
   * @return True if rectangle point
   * @see <a href="Categorysrc.html#getPointAsRect">Source</a>
   */
  public boolean getPointasRect () {
    return  PointasRect;
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
      LineStroke = new BasicStroke(LineWidth);
    else
      LineStroke = new BasicStroke(LineWidth, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER,
          10.0f, dash[i - 1], 0.0f);
    LineStyle = i;
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
  /**
   * Converts a category to a listexpr. Used in session-saving
   * @param cat The cat to convert
   * @return The result as a ListExpr
   * @see <a href="Categorysrc.html#ConvertCattoLE">Source</a>
   */

  public static ListExpr ConvertCattoLE (Category cat) {
    Color c1, c2;
    String Name = "";
    ListExpr l = ListExpr.oneElemList(ListExpr.stringAtom(cat.getName()));
    ListExpr le = ListExpr.append(l, ListExpr.threeElemList(ListExpr.intAtom(cat.getLineColor().getRed()),
        ListExpr.intAtom(cat.getLineColor().getGreen()), ListExpr.intAtom(cat.getLineColor().getBlue())));
    le = ListExpr.append(le, ListExpr.intAtom(cat.getLineStyle()));
    le = ListExpr.append(le, ListExpr.realAtom(cat.getLineWidth()));
    le = ListExpr.append(le, ListExpr.boolAtom(cat.getPointasRect()));
    le = ListExpr.append(le, ListExpr.realAtom(cat.getPointSize()));
    double f = 100.0 - ((AlphaComposite)cat.getAlphaStyle()).getAlpha()*100;
    le = ListExpr.append(le, ListExpr.realAtom(f));
    if (cat.getFillStyle() instanceof Color) {
      le = ListExpr.append(le, ListExpr.symbolAtom("solid"));
      c1 = (Color)cat.getFillStyle();
      c2 = Color.black;
    }
    else if (cat.getFillStyle() instanceof TexturePaint) {
      le = ListExpr.append(le, ListExpr.symbolAtom("texture"));
      c1 = Color.black;
      c2 = Color.black;
      Name = cat.getIconName();
    }
    else if (cat.getFillStyle() instanceof GradientPaint) {
      le = ListExpr.append(le, ListExpr.symbolAtom("gradient"));
      c1 = ((GradientPaint)cat.getFillStyle()).getColor1();
      c2 = ((GradientPaint)cat.getFillStyle()).getColor2();
    }
    else {
      le = ListExpr.append(le, ListExpr.symbolAtom("nofill"));
      c1 = Color.black;
      c2 = Color.black;
    }
    le = ListExpr.append(le, ListExpr.threeElemList(ListExpr.intAtom(c1.getRed()),
        ListExpr.intAtom(c1.getGreen()), ListExpr.intAtom(c1.getBlue())));
    le = ListExpr.append(le, ListExpr.threeElemList(ListExpr.intAtom(c2.getRed()),
        ListExpr.intAtom(c2.getGreen()), ListExpr.intAtom(c2.getBlue())));
    le = ListExpr.append(le, ListExpr.stringAtom(Name));
    return  l;
  }

  /**
   * Converts a ListExpr with a category to a Category-object
   * @param le
   * @return The created Category
   * @see <a href="Categorysrc.html#ConvertLEtoCat">Source</a>
   */
  public static Category ConvertLEtoCat (ListExpr le) {
    if (le.listLength() != 11) {
      System.out.println("Error: No correct category expression: 11 elements needed");
      return  null;
    }
    Category cat = new Category();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  null;
    cat.setName(le.first().stringValue());
    le = le.rest();
    ListExpr v = le.first();
    if ((v.isAtom()) || (v.listLength() != 3) || (v.first().atomType() != ListExpr.INT_ATOM)
        || (v.second().atomType() != ListExpr.INT_ATOM) || (v.third().atomType()
        != ListExpr.INT_ATOM))
      return  null;
    cat.setLineColor(new Color(v.first().intValue(), v.second().intValue(),
        v.third().intValue()));
    le = le.rest();
    if (le.first().atomType() != ListExpr.INT_ATOM)
      return  null;
    cat.setLineStyle(le.first().intValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.REAL_ATOM)
      return  null;
    cat.setLineWidth(le.first().realValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.BOOL_ATOM)
      return  null;
    cat.setPointasRect(le.first().boolValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.REAL_ATOM)
      return  null;
    cat.setPointSize((double)le.first().realValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.REAL_ATOM)
      return  null;
    double f = -le.first().realValue()/100 + 1.0;
    if (f > 1.0f)
      f = 1.0f;
    if (f < 0.0f)
      f = 0.0f;
    cat.setAlphaStyle(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, (float) f));
    le = le.rest();
    if (le.first().atomType() != ListExpr.SYMBOL_ATOM)
      return  null;
    String style = le.first().symbolValue();
    le = le.rest();
    v = le.first();
    if ((v.isAtom()) || (v.listLength() != 3) || (v.first().atomType() != ListExpr.INT_ATOM)
        || (v.second().atomType() != ListExpr.INT_ATOM) || (v.third().atomType()
        != ListExpr.INT_ATOM))
      return  null;
    Color Color1 = new Color(v.first().intValue(), v.second().intValue(), v.third().intValue());
    le = le.rest();
    v = le.first();
    if ((v.isAtom()) || (v.listLength() != 3) || (v.first().atomType() != ListExpr.INT_ATOM)
        || (v.second().atomType() != ListExpr.INT_ATOM) || (v.third().atomType()
        != ListExpr.INT_ATOM))
      return  null;
    Color Color2 = new Color(v.first().intValue(), v.second().intValue(), v.third().intValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  null;
    cat.IconName=le.first().stringValue();
//    ImageIcon ii = new ImageIcon(ClassLoader.getSystemResource(TexturePath+cat.IconName));
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
    else
      return  null;
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
  public BasicStroke getLineStroke () {
    return  LineStroke;
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



