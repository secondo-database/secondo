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
import javax.swing.*;
import javax.swing.border.MatteBorder;
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
 * Basic display class for <CODE>mstype</CODE> grid data (i.e. msint, msbool, msreal, msstring).
 */
public abstract class DisplayRaster2ms extends DisplayRaster2 implements Timed
{

  private Grid3 grid;
  
  /**
  * Duration (the tile's "thickness" as number of grid duration units).
  */
  private int tileSizeTime;
  
  /**
  * Additional temporal statistics
  */
  int minCellIndexTime = Integer.MAX_VALUE;
  int maxCellIndexTime = Integer.MIN_VALUE;
  
  /**
  * BoundingInterval of all cells with defined values 
  */
  protected Interval boundingInterval = null;

  
  /**
  * Intervals in which the msType has defined values.
  */
  protected Vector intervals = new Vector(10, 5);
  
  
  /**
  * Formatter for list expressions (only for debugging).
  */
  private StandardFormatter f = new StandardFormatter();

  
  /********************************************************************************************************
  *     Methods of superclasses
  *********************************************************************************************************/
  
  /**
  * {@inheritDoc}
  */
  @Override
  protected Grid3 getGrid()
  {
    return this.grid;
  }
  
  /**
  * Computes the bounds of all objects displayed. 
  */
  @Override
  protected void computeBounds() 
  {   
    super.computeBounds();
    
    if (!this.intervals.isEmpty())
    {
      this.boundingInterval = ((Interval)this.intervals.get(0)).copy();
      for (Object intvl : this.intervals)
      {
        this.boundingInterval.unionInternal((Interval)intvl);
      }
    }
  }
     
  /**
  * {@inheritDoc}
  */
  @Override
  protected int computeTimeIndex(double pTime)
  {
    Double timeCoord = Math.floor(pTime / this.getGrid().getDuration());
//     Double timeCoord = Math.floor((pTime - this.getBoundingInterval().getStart()) / this.getGrid().getDuration());
    return timeCoord.intValue();
  } 

  /**
  * {@inheritDoc}
  * Scans the list representation of a msType datatype.
  * The complete format of the msType list expression is: </br>
  * ( (GridOriginX GridOriginY GridCellLength Duration) (TileSizeX TileSizeY TileSizeTime) ( KeyX KeyY KeyTime (Value*) )* ) </br>
  * ( (double double double double) (int int int) (int int int ( Comparable* ) )* )
  * @param le list representation of a msType datatype
  * @see sj.lang.ListExpr
  * @author Jasmine Ahmed
  */
  @Override
  protected void ScanValue(ListExpr le)
  {

    // Check if the list expression contains anything at all,  
    // if grid and tile dimensions are defined and
    // if list expression contains a list of tiles (possibly empty)
    if(le == null 
      || le.first() == null 
      || le.second() == null
      || le.third() == null)
    {
      Reporter.writeError("No valid list expression for msType: " );
      err = true;
      return;
    }
    
    // scan grid definition
    this.scanValueGridDef(le.first());
    
    // scan tile definition
    this.scanValueTileDef(le.second());
    
    // scan list of tiles
    ListExpr leTiles = le.rest().rest();    
    while (!leTiles.isEmpty())
    {
      ListExpr leNextTile = leTiles.first();
      
      // scan single tile
      this.scanValueTile(leNextTile);
      leTiles = leTiles.rest();
    }

    // set flag so list expression will be scanned only once
    this.isScanned = true;
    
  }
      
  /**
  *   Get info text.
  */
  @Override
  public String getInfo()
  {
    String eol = System.getProperty("line.separator", "\n");
    StringBuffer sb = new StringBuffer(super.getInfo());
    if (this.isScanned)
    {
      sb.append("Bounding time interval: ").append(this.boundingInterval.toString()).append(eol);
    }
    return sb.toString();
  }
 
  @Override
  public String toString()
  {
    return this.getAttrName();
  }
 

  
  /*********************************************************************************
  * Methods of implemented interfaces
  *********************************************************************************/  
  
  /**
  * {@inheritDoc}
  * Method of interface Timed.
  */
  public Interval getBoundingInterval()
  {
    return this.boundingInterval;
  }
  
  /**
  * {@inheritDoc}
  * Method of interface Timed.
  */
  public Vector getIntervals()
  {
    return this.intervals;
  }
  
  /**
  * {@inheritDoc}
  * Method of interface Timed.
  * The "time renderer" for msTypes is a panel with one label for each interval with defined values.
  */
  public JPanel getTimeRenderer(double PixelTime)
  { 
    JPanel jp = new JPanel(null);
    
    if (intervals == null)
      return  null;
    
    ListIterator li = intervals.listIterator();
    while (li.hasNext()) {
      Interval in = (Interval)li.next();
      int start = (int)((in.getStart() - this.boundingInterval.getStart())*PixelTime);
      int end = (int)((in.getEnd() - this.boundingInterval.getStart())*PixelTime);
      JLabel jc = new JLabel();
//       jc.setFont(new Font("Dialog", Font.PLAIN, 12));
      jc.setPreferredSize(new Dimension(1, 15));
      jc.setBorder(new MatteBorder(2, (in.isLeftclosed()) ? 2 : 0, 2, (in.isRightclosed()) ? 2 : 0, Color.BLACK));
      Dimension d = jc.getPreferredSize();
      jc.setBounds(start, 10, end-start, 20);
//       jc.setBounds(start, (int)d.getHeight()*0 + 7, end - start, (int)d.getHeight());
      jc.setToolTipText(LEUtils.convertTimeToString(in.getStart()) 
                        + "..." + LEUtils.convertTimeToString(in.getEnd()));
      jp.setPreferredSize(new Dimension((int)((this.boundingInterval.getEnd() 
                          - this.boundingInterval.getStart())*PixelTime), 25));
      jp.add(jc);
    }
    return jp;
  }

  
  /*************************************************************************************
  * Methods of class itself
  *************************************************************************************/ 
  
  /**
  * Scans the list representation of a msType grid definition, i.e. a list expression of the format:
  * (double double double double) signifying (GridOriginX GridOriginY GridCellLength GridOriginTime)
  */
  protected void scanValueGridDef(ListExpr leGridDef)
  {

    // Check number and type of list elements,       
    if (leGridDef.listLength() != 4 
        || leGridDef.first().atomType() != ListExpr.REAL_ATOM
        || leGridDef.second().atomType() != ListExpr.REAL_ATOM
        || leGridDef.third().atomType() != ListExpr.REAL_ATOM
        || leGridDef.fourth().atomType() != ListExpr.REAL_ATOM)
    {
      Reporter.writeError("No correct list expression for msType grid: " );
      err = true;
      return;
    }
    double gridOriginX = leGridDef.first().realValue();
    double gridOriginY = leGridDef.second().realValue();
    double gridCellLength = leGridDef.third().realValue();
    double gridDuration = leGridDef.fourth().realValue();
    
    this.grid = new Grid3(gridOriginX, gridOriginY, gridCellLength, gridDuration);
  }
    
  /**
  * Scans the list representation of a msType tile definition, i.e. a list expression of the format:
  * (int int int) signifying (TileSizeX TileSizeY TileSizeTime)
  */
  protected void scanValueTileDef(ListExpr leTileDef)
  {    

    // Check number and type of list elements,       
    if (leTileDef.listLength() != 3
        || leTileDef.first().atomType() != ListExpr.INT_ATOM
        || leTileDef.second().atomType() != ListExpr.INT_ATOM
        || leTileDef.third().atomType() != ListExpr.INT_ATOM)
    {
      Reporter.writeError("No correct list expression for msType tile definition: " );
      err = true;
      return;
    }
    int tileSizeX = leTileDef.first().intValue();
    int tileSizeY = leTileDef.second().intValue();
    int tileSizeTime = leTileDef.third().intValue();
    
    this.grid.setTileDefinition(tileSizeX, tileSizeY, tileSizeTime);
  }
  
      
  /**
  * Scans the list representation of a msType tile, i.e. a list expression of the format:
  * (int int int (Comparable*)) signifying (TileIndexX TileIndexY TileIndexTime (Value*)) 
  */
  protected void scanValueTile(ListExpr leTile)
  {
      if (leTile.listLength() != 4
        || leTile.first().atomType() != ListExpr.INT_ATOM
        || leTile.second().atomType() != ListExpr.INT_ATOM
        || leTile.third().atomType() != ListExpr.INT_ATOM)
      {
        Reporter.writeError("No correct list expression for msType tile: " );
        err = true;
        return;
      }
            
      // read indices of cell in the tile's lower left corner
      // index signifying north/south index from grid origin
      Integer tileIndexX = leTile.first().intValue();           
      // index signifying east/west index from grid origin
      Integer tileIndexY = leTile.second().intValue(); 
      
      // index signifying start
      Integer tileIndexTime = leTile.third().intValue();
      
      //*************************************************************************************
      // scan all cell values in a partial raster
      //*************************************************************************************
      ListExpr leValues = leTile.fourth();
      
      int cellsPerTile = this.getGrid().getTileSizeX() * this.getGrid().getTileSizeY();
      int tileSizeTime = this.getGrid().getTileSizeTime();
      double duration = this.getGrid().getDuration();
           
      if (leValues.listLength() != cellsPerTile*tileSizeTime)
      {
        Reporter.writeError("No correct list expression for tile values (expected " + (cellsPerTile*tileSizeTime) + " values per tile, got " + leValues.listLength() + "). ");
        err = true;
        return;
      }
      
      for (int t=0; t<tileSizeTime; t++)
      {
        Comparable[] tileValues = new Comparable[cellsPerTile];
        boolean sliceHasDefinedValues = false;
        int[] offsetX = this.getGrid().getOffsetX();
        int[] offsetY = this.getGrid().getOffsetY();
//         int[] offsetTime = this.getGrid().getOffsetTime();
        int timeSliceIndex = tileIndexTime + t;
        this.minCellIndexTime = Math.min(minCellIndexTime,timeSliceIndex);
        this.maxCellIndexTime = Math.max(maxCellIndexTime,timeSliceIndex);
      
        for (int i=0; i<cellsPerTile; i++)
        {        
          Comparable val = this.scanValueAtom(leValues.first());
          
          if (val != null)
          {
            tileValues[i] = val;
            
            sliceHasDefinedValues = true;
            
            // update statistics
            int cellCoordX = tileIndexX + offsetX[i];
            int cellCoordY = tileIndexY + offsetY[i];
            minCellCoordX = Math.min(minCellCoordX,cellCoordX);
            maxCellCoordX = Math.max(maxCellCoordX,cellCoordX);    
            minCellCoordY = Math.min(minCellCoordY,cellCoordY);
            maxCellCoordY = Math.max(maxCellCoordY,cellCoordY);    
          
            this.updateMinMaxValue(val);
            cellCount++;                  
          }
          
          // get next value
          leValues = leValues.rest();
        }   
        
        if (sliceHasDefinedValues)
        {
          // store value array and indices of lower left cell in map
          this.setRasterValuesPartial(tileValues, new Point(tileIndexX, tileIndexY), timeSliceIndex);
        
          // store interval of the time slice
          double start = timeSliceIndex*duration;
          double end = (timeSliceIndex+1)*duration;
          Interval interval = new Interval(start, end, true, false);
          this.addInterval(interval);
        }
      }   
  }
  
  /**
  * Adds an interval to the list of defined intervals.
  * If the new interval overlaps with an existing interval, the two are merged.
  */
  protected void addInterval(Interval pInterval)
  {
    for (Object interval : this.intervals)
    {
      if (pInterval.connected((Interval)interval))
      {
        ((Interval)interval).unionInternal(pInterval);
        return;
      }
    }
    this.intervals.add(pInterval);
  }

}
