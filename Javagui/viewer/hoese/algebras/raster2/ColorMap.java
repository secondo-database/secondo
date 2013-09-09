package  viewer.hoese.algebras.raster2;

import java.awt.geom.*;
import java.awt.image.BufferedImage;
import java.awt.image.WritableRaster;
import java.awt.Color;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Paint;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import viewer.hoese.algebras.raster2.*;
import viewer.update.StandardFormatter;
import tools.Reporter;

/**
 * Manages color representation for a set values.
 */
public class ColorMap
{
  /**
  * Color map: Comparable -> Color.
  * Once a color is computed for a certain value it will be stored here for further use.
  */
  private Map<Comparable,Color> mapValueColor = new HashMap<Comparable,Color>();
  
  /**
  * Gradient definition.
  */
  private Color minColor = null;
  private Color maxColor = null;
  private Double minValue = null;
  private Double maxValue = null;
  private boolean isUserDefined = false; 
  
  // private final int treshold = 100; // constant for Raster2 Algebra
  private final int treshold = 10; // constant for Raster2 Algebra and Tile Algebra
  private List<Comparable> values;
  private BufferedImage gradientImage;
  private List<Color> gradientColors;
  
  /**
  * Alternative constructor initializes the ColorMap corresponding to the current Category and Valueset.
  * If the category is default, does not specify a gradient or values are not numeric, 
  * the ColorMap is initialized with default values.
  */
  public ColorMap(TreeSet<Comparable> pValueSet,
                  Comparable minimumValue,
                  Comparable maximumValue,
                  Category pCategory)
  { 
    setMinimumValue(minimumValue);
    setMaximumValue(maximumValue);
    
    // If other than default Category was picked, get gradient colors from Category.
    if (!pCategory.getName().toLowerCase().equals("default"))
    {   
      GradientPaint fillStyle = this.getGradientPaint(pCategory);
      if (fillStyle != null)
      {
        this.minColor = fillStyle.getColor1();
        this.maxColor = fillStyle.getColor2();
        this.isUserDefined = true;
      }
    }
    
    this.values = new ArrayList<Comparable>();
    Iterator<Comparable> it = pValueSet.iterator();
    while(it.hasNext())
    {
      Comparable value = it.next();
      if (value != null)
      {
        this.values.add(value);
      }
    }
    
    this.initGradientColors();
    
    this.computeGradientImage();
    
    for(Comparable value : this.values)
    {
      Color color = this.computeColorForValue(value);
      this.mapValueColor.put(value, color);
    }
  }
  
  /**
  *  Initializes gradient colors for numeric values.
  */
  private void initGradientColors()
  {
      this.gradientColors = new ArrayList<Color>();
      
      if(this.isUserDefined)
      {
        this.gradientColors.add(this.minColor);
        this.gradientColors.add(this.maxColor);
      }
      
      else
      {
        // define default color sequence for gradient
        gradientColors.add(Color.BLACK);
        gradientColors.add(Color.BLUE);
        gradientColors.add(Color.CYAN);
        gradientColors.add(Color.GREEN);
        gradientColors.add(Color.YELLOW);
        gradientColors.add(Color.ORANGE);
        gradientColors.add(Color.RED);
        gradientColors.add(Color.MAGENTA);
        gradientColors.add(Color.WHITE);
      }   
  }
  
  public List<Comparable> getValuesSorted()
  {
    return this.values;
  }

  /**
  * Sets the minimum value of color map.
  */
  private void setMinimumValue(Comparable minimumValue)
  {
    if(minimumValue != null)
    {
      if(minimumValue instanceof Integer)
      {
        minValue = new Double((Integer)minimumValue);
      }

      else if(minimumValue instanceof Double)
      {
        minValue = new Double((Double)minimumValue);
      }

      else if(minimumValue instanceof Boolean)
      {
        if((Boolean)minimumValue == Boolean.FALSE)
        {
          minValue = new Double(0.0);
        }

        else
        {
          minValue = new Double(1.0);
        }
      }

      else if(minimumValue instanceof String)
      {
        minValue = new Double(((String)minimumValue).hashCode());
      }
    }
  }

  /**
  * Sets the maximum value of color map.
  */
  private void setMaximumValue(Comparable maximumValue)
  {
    if(maximumValue != null)
    {
      if(maximumValue instanceof Integer)
      {
        maxValue = new Double((Integer)maximumValue);
      }

      else if(maximumValue instanceof Double)
      {
        maxValue = new Double((Double)maximumValue);
      }

      else if(maximumValue instanceof Boolean)
      {
        if((Boolean)maximumValue == Boolean.FALSE)
        {
          minValue = new Double(0.0);
        }

        else
        {
          minValue = new Double(1.0);
        }
      }

      else if(maximumValue instanceof String)
      {
        maxValue = new Double(((String)maximumValue).hashCode());
      }
    }
  }
  
  /**
  * Returns the GradientPaint if user specified Category with gradient, null else.
  */
  public GradientPaint getGradientPaint(Category pCategory)
  {

    GradientPaint result = null;
    Paint fillStyle = pCategory.getFillStyle(null, 0);
    if (fillStyle != null && fillStyle instanceof GradientPaint)
    {
      result = (GradientPaint)fillStyle;
    }
    return result;
  }
  
  
  
  /**
  * Returns the color representation for the given value.
  * If the map does not yet contain the color for this value,
  * a matching color is computed and stored in the map.
  */
  public Color getColorForValue(Comparable pValue)
  {
    Color result = this.mapValueColor.get(pValue);
    
    if(result == null)
    {
      result = this.computeColorForValue(pValue);
      this.mapValueColor.put(pValue, result);
    }
    
    return result;
  }
  

  /**
  * Computes a color for the given value.
  */
  protected Color computeColorForValue(Comparable pValue)
  {
    Color valueColor = null;

    if(pValue != null)
    {
      if(pValue instanceof Integer)
      {
        valueColor = computeColorForInteger((Integer)pValue);
      }

      if(pValue instanceof Double)
      {      
        valueColor = computeColorForDouble((Double)pValue);
      }

      if(pValue instanceof Boolean)
      {
        valueColor = computeColorForBoolean((Boolean)pValue);
      }

      if(pValue instanceof String)
      {
        valueColor = computeColorForString((String)pValue);
      }
    }
    
    return valueColor;
  }
  
  /**
  * Returns a color for an Integer value.
  */
  private Color computeColorForInteger(Integer pValue)
  {
    Color result = null;
    
    int index = 0;
    double valuerange = this.maxValue - this.minValue;
    
    if(this.values.size() < this.treshold)
    {
      // pick color with same index from gradient image
      index = this.values.indexOf(pValue);
    }
    
    else
    {
      // compute relative position of value within value range 
      // and pick corresponding color from color range
      
      Double colpos;
      
      if(valuerange == 1)
      {
        colpos = 0.0;
      }
      
      else
      { 
        colpos = ((pValue - this.minValue) * (this.gradientImage.getWidth() / (valuerange)));
        
        if(colpos.intValue() == this.gradientImage.getWidth())
        {
          colpos -= 1.0;
        }
      }
      
      index = colpos.intValue();
    }
    
    int rgb = this.gradientImage.getRGB(index, 0);
    result = new Color(rgb);
    
    return result;
  }

    /**
  * Returns a color for a Double value. Gets color from a gradient.
  * Pro: Provides continuous coloring, i.e. similar values have similar colors.
  * Contra: Color resolution depends on the ratio of number of values and 
  * number of different colors in the gradient.
  */
  private Color computeColorForDouble(Double pValue)
  { 
    Color result = null;

    if(minValue != null &&
       maxValue != null &&
       pValue != null)
    {
      int index = 0;
      double valuerange = this.maxValue - this.minValue;
      
      if(this.values.size() < this.treshold)
      {
        // pick color with same index from gradient image
        index = this.values.indexOf(pValue);
      }
      else
      {
        // compute relative position of value within value range 
        // and pick corresponding color from color range
        Double colpos;
        if (valuerange==1)
        {
          colpos = 0.0;
        }
        else
        { 
          colpos = ((pValue - this.minValue) * this.gradientImage.getWidth() / (valuerange));
          if (colpos.intValue() == this.gradientImage.getWidth() ){
                  colpos = colpos-1; 
          }
        }
        index = colpos.intValue();
      }
      
      int rgb = this.gradientImage.getRGB(index,0);
      result = new Color(rgb);
    }
    return result;
  }
    
  
  /**
  * Returns a color for a Double value.
  */
  private Color computeColorForDoubleAbsolute(Double pValue)
  { 
    Color doubleColor = null;

    if(minValue != null &&
       maxValue != null &&
       pValue != null)
    {
        final Integer colorValues = 16777215; // 0 <= colorValue <= 2^24 - 1
        final Double interval = maxValue - minValue;
        
        Double colorValue = ((pValue - minValue) / interval) * colorValues;
        
        /* display optimization */
        doubleColor = new Color(colorValue.intValue());
        Integer red = doubleColor.getRed();
        Integer green = doubleColor.getGreen();
        Integer blue = doubleColor.getBlue();
        Integer sameColorInterval = 16;
        red = (red / sameColorInterval) * sameColorInterval;
        green = (green / sameColorInterval) * sameColorInterval;
        blue = (blue / sameColorInterval) * sameColorInterval;
        doubleColor = new Color(red, green, blue);
    }
    return doubleColor;
  }
  
    
  /**
  * Returns a color for a Boolean value: 
  * black for TRUE, white for FALSE and null for UNDEFINED.
  */
  private Color computeColorForBoolean(Boolean pValue)
  {
    Color booleanColor = null;

    if(pValue != null)
    {
      if (this.isUserDefined) // use user-defined gradient colors
      {
        if(pValue)
        {
          booleanColor = this.minColor;
        }
        else
        {
          booleanColor = this.maxColor;
        }
      }
      else // no user-defined gradient colors
      {
        if(pValue)
        {
          booleanColor = Color.BLACK;
        }
        else
        {
          booleanColor = Color.WHITE;
        }
      }
    } 
    return booleanColor;
  }

  /**
  * Returns a color for a String value.
  */
  private Color computeColorForString(String pValue)
  { 
    Color stringColor = null;

    if(minValue != null &&
       maxValue != null &&
       pValue != null)
    {
      final Integer colorValues = 16777215; // 0 <= colorValue <= 2^24 - 1
      final Double interval = maxValue - minValue;
      
      Double hashValue = new Double(pValue.hashCode());
      Double colorValue = ((hashValue - minValue) / interval) * colorValues;
      stringColor = new Color(colorValue.intValue());
    }

    return stringColor;
  }
  
  /**
  * Prepares an Image with gradient for the given values.
  * The image is as many pixel wide as there are values in the list.
  * The color for the value at a given array index is found in the pixel with the corresponding index.
  */
  private void computeGradientImage()
  {
      Graphics2D g2;      
      
      // if only few values, map values directly to colors
      if(this.values.size() <= this.gradientColors.size())
      { 
        this.gradientImage = new BufferedImage(this.values.size(), 1, BufferedImage.TYPE_INT_ARGB);
        g2 = (Graphics2D) gradientImage.getGraphics();
        
        for (int index=0; index<this.values.size(); index++)
        {
          gradientImage.setRGB(index, 0, this.gradientColors.get(index).getRGB());
        }
      }
      
      // if gradient image size will not cause heap space problems
      // create gradient image with one pixel for every value
      else if (values.size() <= this.treshold)
      { 
        this.gradientImage = new BufferedImage(this.values.size(), 1, BufferedImage.TYPE_INT_ARGB);
        g2 = (Graphics2D) gradientImage.getGraphics();

        // fill image with white background
        g2.setColor(Color.WHITE);
        g2.fillRect(0, 0, this.values.size(), 1);  
        
        // fill image with gradient stripes
        int intervalSize = this.values.size()/(this.gradientColors.size()-1);
        for (int i=0; i<this.gradientColors.size()-1; i++)
        {   
          GradientPaint gradient = new GradientPaint(intervalSize * i, 0, this.gradientColors.get(i), 
                                                     intervalSize * (i+1), 1, this.gradientColors.get(i+1));
          g2.setPaint(gradient);
          g2.fillRect(intervalSize*i, 0, intervalSize+1, 1);
        }
        
        // fill probably last pixel(s) of image with last color
        boolean filled = false;
        int pixelIndex = this.gradientImage.getWidth()-1;
        Color lastColor = this.gradientColors.get(this.gradientColors.size()-1);
        while (!filled)
        {
          if (Color.WHITE.equals(new Color(gradientImage.getRGB(pixelIndex, 0))))
          {
            gradientImage.setRGB(pixelIndex, 0, lastColor.getRGB());
            pixelIndex--;
          }
          else 
          {
            filled = true;
          }
        }
      }
      
      // if too many values, gradient image size will cause heapspace problems
      // restrict gradient image Size and use rationally spaced color mapping 
      else 
      { 
        int gradientWidth = 0;
        
        // determine how many different colors can be produced with current gradient colors 
        for(int i = 0; i < this.gradientColors.size() - 1; i++)
        {
          Color color1 = this.gradientColors.get(i);
          Color color2 = this.gradientColors.get(i + 1);
          int width = Math.max(Math.max(Math.abs(color1.getRed() - color2.getRed()), 
                               Math.abs(color1.getGreen() - color2.getGreen())), 
                               Math.abs(color1.getBlue() - color2.getBlue()));
          gradientWidth = gradientWidth + width;
        }
        
        this.gradientImage = new BufferedImage(gradientWidth, 1, BufferedImage.TYPE_INT_ARGB);
        g2 = (Graphics2D) gradientImage.getGraphics();
      
        // fill image with white background
        g2.setColor(Color.WHITE);
        g2.fillRect(0, 0, gradientWidth, 1);
        
        // fill image with gradient stripes
        int index = 0;
        
        for(int i = 0; i < this.gradientColors.size() - 1; i++)
        {
          Color color1 = this.gradientColors.get(i);
          Color color2 = this.gradientColors.get(i + 1);
          int width = Math.max(Math.max(Math.abs(color1.getRed() - color2.getRed()), 
                               Math.abs(color1.getGreen() - color2.getGreen())), 
                               Math.abs(color1.getBlue() - color2.getBlue()));
          
          GradientPaint gradient = new GradientPaint(index, 0, color1, 
                                                     index + width, 1, color2);
          g2.setPaint(gradient);
          g2.fillRect(index, 0, width, 1);
          
          index = index + width;
        }
      }
      
    g2.dispose();
  }
  
  
  @Override
  public String toString()
  {
    String eol = System.getProperty("line.separator", "\n");
    StringBuffer sb = new StringBuffer("ColorMap: ").append(eol);  
    sb.append("minValue: ").append(this.minValue).append(eol);
    sb.append(", maxValue: ").append(this.maxValue).append(eol);
    sb.append(", minColor: ").append(this.minColor).append(eol);
    sb.append(", maxColor: ").append(this.maxColor).append(eol);
    sb.append(", isUserDefined: ").append(this.isUserDefined).append(eol);
    sb.append(", treshold: ").append(this.treshold).append(eol);
    sb.append(", noOfValues: ").append(this.values.size()).append(eol);
    sb.append(", noOfColors: ").append(this.gradientImage.getWidth()).append(eol);

    return sb.toString();
  }
}
