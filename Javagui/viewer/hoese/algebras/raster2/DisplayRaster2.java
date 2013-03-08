package  viewer.hoese.algebras.raster2;


import java.awt.BorderLayout;
import java.awt.event.*;
import java.awt.Font;
import java.awt.font.*;
import java.awt.geom.*;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.WritableRaster;
import java.awt.*;
import java.math.*;
import javax.swing.*;
import java.text.AttributedString;
import java.util.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import viewer.hoese.algebras.*;
import viewer.update.StandardFormatter;
import tools.Reporter;

/**
 * Basic display class for grid data.
 */
public abstract class DisplayRaster2 extends DisplayGraph
                                     implements DisplayComplex,
                                                ExternDisplay
{
  /**
  * current time index
  */
  protected Integer currentTimeIndex = 0;

  /**
  * Raster values (one array of values for each tile) are mapped to a temporal index.
  * Integer key specifies the temporal index of a raster.
  * Point key specifies the spatial indices of a tile within the raster.
  * Comparable array contains the tile's values.
  */
  protected HashMap<Integer, HashMap<Point, Comparable[]>> mapCoordValues = new HashMap<Integer, HashMap<Point, Comparable[]>>();
  
  /**
  * Set of all scanned (defined) values.
  */
  protected TreeSet<Comparable> valueSet = new TreeSet<Comparable>();
  
  
  /**
  * Statistics (regarding only cells with defined values).
  * These are computed while scanning the list representation.
  */
  // number of cells with defined values
  protected int cellCount = 0;
  // min/max indices of cells with defined values
  protected int minCellCoordX = Integer.MAX_VALUE;
  protected int maxCellCoordX = Integer.MIN_VALUE;
  protected int minCellCoordY = Integer.MAX_VALUE;
  protected int maxCellCoordY = Integer.MIN_VALUE;
  // min/max cell value
  protected Comparable minValue = null;
  protected Comparable maxValue = null;

  /**
  * Data type name of cell values
  */
  protected String typeName = "";
  
  /**
  * Boundingbox of all cells with defined values 
  */
  protected Rectangle2D.Double bounds = new Rectangle2D.Double();
  
  /**
  * Was this object scanned already?
  * Prevents repeated scans when object is redrawn.
  * Is set to TRUE after ScanValue has completed successfully, i.e. all member variables should contain meaningful values.
  */
  protected boolean isScanned = false;

  /**
  * Value-to-color mapping
  */
  protected ColorMap colorMap = null;
  

  /** 
  * Frame for external display 
  **/
  static ExternWindow extWin = new ExternWindow();

  
  /********************************************************************************************************
  *     Methods of superclasses DsplBase, DisplayGraph
  *********************************************************************************************************/
  
  /**
  * {@inheritDoc}
  */
  @Override
  public boolean isLineType(int num)
  {
    return false;
  }
  
  /**
  * {@inheritDoc}
  */
  @Override
  public boolean isPointType(int num)
  {
    return false;
  }

  /**
  * {@inheritDoc}
  */
  @Override
  public int numberOfShapes()
  {
     return 1;
  }

  /**
  * {@inheritDoc} 
  **/
  @Override
  public Shape getRenderObject(int num, AffineTransform at)
  {    
    Rectangle2D.Double renderObject = new Rectangle2D.Double();
    renderObject.setRect(0, 0, 0.0, 0.0);
           
    return renderObject;
  }
  
  
  /**
  * {@inheritDoc}
  */
  @Override
  public Rectangle2D.Double getBounds () 
  {   
    return this.bounds;
  }
  

  /**
   * {@inheritDoc}
   */
  @Override
  public void init(String name, int nameWidth,  int indent, ListExpr type, ListExpr value, QueryResult qr)
  {
        
    AttrName = extendString(name, nameWidth, indent);
    this.typeName = this.extractTypeName(name);
    
    if (!isScanned)
    {
      ScanValue(value);
    }

    if (err)
    {
      qr.addEntry(new String(AttrName + ": parse error"));
      bounds =null;
      return;
    }
    else
    {
      if(this.isEmpty())
      {
        qr.addEntry(AttrName + " : empty");
        bounds = null;
        return;
      }
    }
    
    qr.addEntry(this);   
    
    this.computeBounds();
    
  }

   /**
   * {@inheritDoc}
   */
  @Override
  public void setLayer(Layer layer)
  {
    this.RefLayer = layer;

    if(this.RefLayer != null)
    {
      this.RefLayer.addMouseListener(new MouseListener()
      {
        public void mouseClicked(MouseEvent e)
        {
          dispatchMouseEventToParent(e);
        }

        public void mousePressed(MouseEvent e)
        {
          dispatchMouseEventToParent(e);
        }

        public void mouseReleased(MouseEvent e)
        {
          dispatchMouseEventToParent(e);
        }

        public void mouseEntered(MouseEvent e)
        {
          dispatchMouseEventToParent(e);
        }

        public void mouseExited(MouseEvent e)
        {
          dispatchMouseEventToParent(e);
        }
      });

      this.RefLayer.addMouseMotionListener(new MouseMotionListener()
      {
        public void mouseDragged(MouseEvent e)
        {
          dispatchMouseEventToParent(e);
        }

        public void mouseMoved(MouseEvent e)
        {
          Point2D.Double point = new Point2D.Double();

          try
          {
            point = (Point2D.Double)CurrentState.transform.inverseTransform(e.getPoint(), point);
          }

          catch (Exception ex)
          {
            Reporter.debug("DisplayRaster2::init(...): Exception: CurrentState.transform.inverseTransform(...)");
          }
          
          Comparable value = getValue(currentTimeIndex, point);

          if(value != null)
          {
            RefLayer.setToolTipText(value.toString());
          }

          else
          {
            RefLayer.setToolTipText("");
          }
 
          dispatchMouseEventToParent(e);
        }
      });
    }

    else
    {
      Reporter.debug("DisplayRaster2::init(...): this.RefLayer == null");
    }
  }
  
  /*********************************************************************************
  * Methods of implemented interfaces
  *********************************************************************************/
  
  /**
  * {@inheritDoc}
  * Draws visual representations of all values in the grid object:
  * Defined values are drawn as filled rectangles of a colour
  * corresponding to the value. 
  */
  public void draw(Graphics g, double time, AffineTransform at)
  {
    this.currentTimeIndex = this.computeTimeIndex(time);
    
    Map<Point,BufferedImage> images = this.getRasterImage();
    
    if(images != null)
    {
      this.drawRaster(images, g, at);
    }
        
  }
  
   
  /** 
   * {@inheritDoc}
  * Shows textual raster information in an external window 
  **/
  public void displayExtern(){
    extWin.setRaster(this);
    extWin.setVisible(true);
  }

  /** 
   * {@inheritDoc}
  * Shows textual raster information in an external window 
  **/
  public boolean isExternDisplayed(){
    return this==extWin.getRaster() && extWin.isVisible();
  }
  
  /*************************************************************************************
  * Methods of class itself
  *************************************************************************************/
    
  /**
  * Computes the bounds of all objects displayed. 
  */
  protected void computeBounds() 
  {   
    double x = this.getGrid().getOriginX() + this.minCellCoordX * this.getGrid().getCellLength(); 
    double y = this.getGrid().getOriginY() + this.minCellCoordY * this.getGrid().getCellLength();
    double width = ((this.maxCellCoordX - this.minCellCoordX) + 1) * this.getGrid().getCellLength();
    double height = ((this.maxCellCoordY - this.minCellCoordY) + 1) * this.getGrid().getCellLength();
      
    this.bounds.setRect(x, y, width, height); 
      
  }
  


  
    
  /**
  * Scans the list representation of a Raster2 datatype:
  * @param le list representation of a Raster2 datatype
  * @see sj.lang.ListExpr
  */
  abstract protected void ScanValue(ListExpr le);
 
  /**
  * Scans a cell value atom in the list representation.
  */
  protected Comparable scanValueAtom(ListExpr valueLe)
  {
    Comparable val = null;    
    int atomType = valueLe.atomType();
    
    switch (atomType)
    {
      case ListExpr.INT_ATOM:
        val = valueLe.intValue();
        break;
      case ListExpr.REAL_ATOM:
        val = valueLe.realValue();
        break;
      case ListExpr.BOOL_ATOM:
        val = valueLe.boolValue();
        break;
      case ListExpr.STRING_ATOM:
        val = (valueLe.stringValue());
        break;
      case ListExpr.SYMBOL_ATOM:
        if (valueLe.symbolValue().equals("undefined"))
        {
          // symbol "undefined" is a valid cell value and does not cause error message.
        }
      default:
    }
    
    if (val != null){
      this.valueSet.add(val);
    }
    
    return val;
  }

  
  /**
  * Keeps track of minimum and maximum values of all cells
  * while scanning the raster.
  */
  @SuppressWarnings("unchecked")
  protected void updateMinMaxValue(Comparable value)
  {
    if(minValue == null)
    {
      minValue = value;
    }

    else
    {
      if(value.compareTo(minValue) < 0)
      {
        minValue = value;
      }
    }

    if(maxValue == null)
    {
      maxValue = value;
    }

    else
    {
      if(value.compareTo(maxValue) > 0) 
      {
        maxValue = value;
      }
    }
  }  
  
  /**
  * Returns the grid definition.
  */
  abstract protected Grid2 getGrid();
 
  /**
  * Creates BufferedImage for each tile.
  * The grid image is not created as a whole, because grids can get arbitrarily large which causes java heap space problems.
  */
  protected HashMap<Point, BufferedImage> buildRasterImage(Map<Point, Comparable[]> pMap)
  { 
    HashMap<Point, BufferedImage> result = new HashMap<Point, BufferedImage>();
    for (Map.Entry<Point, Comparable[]> entry : pMap.entrySet()){
      BufferedImage img = new BufferedImage(this.getGrid().getTileSizeX(), this.getGrid().getTileSizeY(), BufferedImage.TYPE_INT_ARGB);
      WritableRaster raster = (WritableRaster) img.getData();
      int[] rgb = new int[this.getGrid().getCellsPerTile()*4];
      Color color;
      Comparable[] tileValues = entry.getValue();
      
      for (int i=0; i<this.getGrid().getCellsPerTile(); i++)
      {        
        Comparable val = tileValues[i];
        
        if (val != null)
        {
          color = this.getColorMap().getColorForValue(val);
          rgb[i*4] = color.getRed();
          rgb[i*4+1] = color.getGreen();
          rgb[i*4+2] = color.getBlue();
          rgb[i*4+3] = color.getAlpha();                
        }
        else
        {
          // value not defined => set cell transparent
          rgb[i*4+3] = 0;
        }
      }
      
      raster.setPixels(0, 0, this.getGrid().getTileSizeX(), this.getGrid().getTileSizeY(), rgb);
      img.setData(raster);
      
      // store image and coordinates in map
      result.put(entry.getKey(), img);
    }
    return result;
  }
  
  /**
  * Draws the given grid to the user space.
  */
  protected void drawRaster(Map<Point,BufferedImage> imageMap, Graphics g, AffineTransform at)
  {
    if(imageMap != null &&
       g != null &&
       at != null)
    {
      Graphics2D graphics2D = (Graphics2D)g;

      if(graphics2D != null)
      {
        Long width = Math.round(getGrid().getCellLength() * getGrid().getTileSizeX() * at.getScaleX());
        Long height = Math.round(getGrid().getCellLength() * getGrid().getTileSizeY() * at.getScaleY());

        // Display all BufferedImages (tiles) in the map
        for(Map.Entry<Point,BufferedImage> entry : imageMap.entrySet())
        {
          Point index = entry.getKey();          
          BufferedImage bufferedImage = entry.getValue();
          Integer imageWidth = width.intValue();
          Integer imageHeight = height.intValue();

          Double dx = (getGrid().getOriginX() + getGrid().getCellLength() * index.x) * at.getScaleX()  + at.getTranslateX();
          Double dy = (getGrid().getOriginY() + getGrid().getCellLength() * index.y) * at.getScaleY()  + at.getTranslateY();
          Double dfloorx = new Double(Math.floor(dx));
          Double dceilx = new Double(Math.ceil(dx));
          Double droundx = new Double(Math.round(dx));
          Double dfloory = new Double(Math.floor(dy));
          Double dceily = new Double(Math.ceil(dy));
          Double droundy = new Double(Math.round(dy));
          Integer nx = null;
          Integer ny = null;

          if(imageWidth > 0)
          {
            if(dfloorx.equals(droundx))
            {
              nx = dfloorx.intValue();
            }

            else
            {
              nx = dfloorx.intValue();
              imageWidth++;
            }
          }

          else
          {
            if(dfloorx.equals(droundx))
            {
              nx = dfloorx.intValue();
            }

            else
            {
              nx = dceilx.intValue();
              imageWidth--;
            }
          }

          if(imageHeight > 0)
          {
            if(dfloory.equals(droundy))
            {
              ny = dfloory.intValue();
            }

            else
            {
              ny = dfloory.intValue();
              imageHeight++;
            }
          }

          else
          {
            if(dfloory.equals(droundy))
            {
              ny = dfloory.intValue();
            }

            else
            {
              ny = dceily.intValue();
              imageHeight--;
            }
          }

          graphics2D.drawImage(bufferedImage, nx, ny, imageWidth, imageHeight, null, null);
        }   
        
        // only for debugging
//         Graphics2D gbounds = (Graphics2D)g.create();
//         gbounds.setColor(Color.RED);
//         gbounds.draw(at.createTransformedShape((Shape)bounds));
//         gbounds.dispose();
      }
    }
  }
  
  /**
  * Returns the temporal index corresponding to the given time.
  */
  abstract protected int computeTimeIndex(double time);
  
  
  /**
  * Stores the values of a tile in the map at given temporal and spatial indices.
  */
  protected void setRasterValuesPartial(Comparable[] pValues, Point pSpatialIndices, int pTemporalIndex)
  {
    HashMap<Point, Comparable[]> valueMap = this.mapCoordValues.get(pTemporalIndex);

    if(valueMap == null)
    {
      valueMap = new HashMap<Point, Comparable[]>();
      mapCoordValues.put(pTemporalIndex, valueMap);
    }

    valueMap.put(new Point(pSpatialIndices.x, pSpatialIndices.y), pValues);
  }

  
  /**
  * Returns the BufferedImages for the grid at the given temporal index.
  */
  protected HashMap<Point,BufferedImage> getRasterImage()
  {
    HashMap<Point,BufferedImage> result = null;  
      
      HashMap<Point,Comparable[]> valueMap = this.mapCoordValues.get(this.currentTimeIndex);
      if (valueMap != null)
      {
        // values for the given time index exist => build image
        result = this.buildRasterImage(valueMap);
    }   
    return result;
  }

  
  public Comparable getMinValue()
  {
    return this.minValue;
  }
  
  public Comparable getMaxValue()
  {
    return this.maxValue;
  } 
  
  /**
  * Returns a sorted set of all values represented in the grid.
  */ 
  public TreeSet<Comparable> getValues()
  {
    return this.valueSet;
  }
  
  
 /**
 * Returns the ColorMap. 
 */
  public ColorMap getColorMap()
  {
    if (this.colorMap == null)
    {
      this.colorMap = new ColorMap(this.getValues(), this.getCategory());
    }
    return this.colorMap;
  }
 
  /**
  * Returns true if this raster contains any defined cells at all.
  */
  public boolean isEmpty()
  {
    return (this.cellCount==0);
  }
 
  /**
  *  Extracts data type name of the raster's cell values (from AttributeName).
  */
  public String extractTypeName(String pName)
  {     
    if (pName.endsWith("int")) return "int";
    if (pName.endsWith("real")) return "real";
    if (pName.endsWith("bool")) return "bool";
    if (pName.endsWith("string")) return "string";
    return "";
  }
  
  /**
  * Get info text.
  * Returns nicely formatted text. (to be displayed in external viewer)
  */
  public String getInfo()
  {
    String eol = System.getProperty("line.separator", "\n");
    StringBuffer sb = new StringBuffer("AttrName: ").append(this.getAttrName()).append(eol);
    if (this.isScanned)
    {
      sb.append(this.getGrid().getInfo());
      sb.append("Data type: ").append(this.typeName).append(eol);
      sb.append("Number of defined values: ").append(this.cellCount).append(eol);
      sb.append("Smallest value: ").append(this.getLabelText(this.minValue)).append(eol);
      sb.append("Largest value: ").append(this.getLabelText(this.maxValue)).append(eol);
      sb.append("World coordinates of lower left corner: (");
      sb.append(String.format("%2g", this.getGrid().getOriginX() + this.minCellCoordX*this.getGrid().getCellLength()));
      sb.append(", ");
      sb.append(String.format("%2g", this.getGrid().getOriginY() + this.minCellCoordY*this.getGrid().getCellLength()));
      sb.append(")").append(eol);
      sb.append("World coordinates of upper right corner: (");
      sb.append(String.format("%2g", this.getGrid().getOriginX() + (this.maxCellCoordX+1)*this.getGrid().getCellLength()));
      sb.append(", ");
      sb.append(String.format("%2g", this.getGrid().getOriginY() + (this.maxCellCoordY+1)*this.getGrid().getCellLength()));
      sb.append(")").append(eol);
    }
    else
    {
      sb.append(" not initialized ").append(eol);
    }
    return sb.toString();
  }
  
  /**
  * Makes a labeltext appropriate for the value type.
  */
  public String getLabelText(Comparable pValue)
  {
    String result = "";
    if (pValue instanceof Double){
      result = String.format("%2gs", pValue);
    }
    else {
      result = pValue.toString();
    }
    return result;
  }
 
  /**
  * {@inheritDoc}
  * Note: this is the text that will appear in the GUI's Object List.
  */
  @Override
  public String toString()
  {
      return this.getAttrName();
  }

  /**
  *  Dispatchs the mouse event to parent object.
  */
  private void dispatchMouseEventToParent(MouseEvent e)
  {
    if(e != null)
    {
      Container parent = RefLayer.getParent();
  
      if(parent != null)
      {
        parent.dispatchEvent(e);
      }
    }
  }

  /**
  *  Returns the value of the cell including given point.
  */
  private Comparable getValue(Integer temporalIndex, Point2D.Double point)
  {     
    Comparable value = null;

    if(mapCoordValues != null &&
       temporalIndex != null &&
       point != null)
    {
      HashMap<Point, Comparable[]> valueMap = mapCoordValues.get(temporalIndex);
      
      if(valueMap != null)
      {
        Set<Point> pointSet = valueMap.keySet();

        if(pointSet != null)
        {
          Point2D.Double origin = new Point2D.Double(getGrid().getOriginX(),
                                                    getGrid().getOriginY());
          Double cellLength = new Double(getGrid().getCellLength());
          Point tileSize = new Point(getGrid().getTileSizeX(),
                                     getGrid().getTileSizeY());

          for(Point onePoint : pointSet)
          {
            Point2D.Double rasterPoint = new Point2D.Double();
            rasterPoint.x = origin.x + onePoint.x * cellLength;
            rasterPoint.y = origin.y + onePoint.y * cellLength;

            if(rasterPoint.x <= point.x &&
               point.x < (rasterPoint.x + cellLength * tileSize.x) &&
               rasterPoint.y <= point.y &&
               point.y < (rasterPoint.y + cellLength * tileSize.y))
            {
              Point tileSizeIndex = new Point(0, 0);

              for(Integer i = 0; i < tileSize.x; i++)
              {
                if((rasterPoint.x + cellLength * i) <= point.x &&
                    point.x < (rasterPoint.x + cellLength * (i + 1)))
                {
                  tileSizeIndex.x = i;
                  break;
                }
              }

              for(Integer i = 0; i < tileSize.y; i++)
              {
                if((rasterPoint.y + cellLength * i) <= point.y &&
                    point.y < (rasterPoint.y + cellLength * (i + 1)))
                {
                  tileSizeIndex.y = i;
                  break;
                }
              }

              Comparable[] values = valueMap.get(onePoint);

              if(values != null)
              {
                value = values[tileSizeIndex.x + tileSizeIndex.y * tileSize.y];
                break;
              }
            }
          }
        }
      }
    }

    return value;
  }
}
