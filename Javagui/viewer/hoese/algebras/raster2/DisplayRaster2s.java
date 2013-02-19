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
 * Basic display class for <CODE>stype</CODE> grid data (i.e. sint, sbool, sreal, sstring).
 * The complete format of the sType list expression is: </br>
 *( (GridOriginX GridOriginY CellLength) (TileSizeX TileSizeY) ( KeyX KeyY (Value*) )* ) </br>
 *( (double double double) (int int) (int int ( Comparable* ) )* )
 */
public abstract class DisplayRaster2s extends DisplayRaster2
{
  private StandardFormatter f = new StandardFormatter();
  
  private Grid2 grid;

  protected Grid2 getGrid()
  {
    return this.grid;
  }

  /**
  * {@inheritDoc}
  * Scans the list representation of a sType datatype:
  *( (double double double) (int int) (int int ( Comparable* ) )* )
  * @param le list representation of a sType datatype
  * @see sj.lang.ListExpr
  * @author Jasmine Ahmed
  */
  @Override
  protected void ScanValue(ListExpr le)
  {
    StandardFormatter f = new StandardFormatter();
//     Reporter.debug("start ScanValue; parsing sType object from list expression: " + f.ListExprToString(le));

    // Check if the list expression contains anything at all,  
    // if grid and tile dimensions are defined and
    // if list expression contains a list of tiles (possibly empty)
    if(le == null 
      || le.first() == null 
      || le.second() == null
      || le.third() == null)
    {
      Reporter.writeError("No valid list expression for sType: " + f.ListExprToString(le));
      err = true;
      return;
    }
    
    // scan grid definition
    scanValueGridDef(le.first());
    
    // scan tile definition
    scanValueTileDef(le.second());
    
    // scan list of tiles
    ListExpr leTiles = le.rest().rest();    
    while (!leTiles.isEmpty())
    {
      ListExpr leNextTile = leTiles.first();
      Reporter.debug("Parsing tile (partial grid) from list expression: " + f.ListExprToString(leNextTile));     
      
      // scan single tile
      scanValueTile(leNextTile);
      leTiles = leTiles.rest();
    }

    // set flag so list expression will be scanned only once
    this.isScanned = true;
    
    Reporter.debug("end ScanValue");
  }
  
  /**
  * {@inheritDoc}
  * Scans the list representation of a sType grid definition, i.e. a list expression of the format:
  * (double double double) signifying (GridOriginX GridOriginY GridCellLength)
  */
  protected void scanValueGridDef(ListExpr leGridDef)
  {
    Reporter.debug("start ScanValueGridDef with list expression: " + f.ListExprToString(leGridDef));

    // Check number and type of list elements,       
    if (leGridDef.listLength() != 3 
        || leGridDef.first().atomType() != ListExpr.REAL_ATOM
        || leGridDef.second().atomType() != ListExpr.REAL_ATOM
        || leGridDef.third().atomType() != ListExpr.REAL_ATOM)
    {
      Reporter.writeError("No correct list expression for grid: " + f.ListExprToString(leGridDef));
      err = true;
      return;
    }
    double gridOriginX = leGridDef.first().realValue();
    double gridOriginY = leGridDef.second().realValue();
    double gridCellLength = leGridDef.third().realValue();
    
    this.grid = new Grid2(gridOriginX, gridOriginY, gridCellLength);
  }
    
  /**
  * Scans the list representation of a sType tile definition, i.e. a list expression of the format:
  * (int int) signifying (TileSizeX TileSizeY)
  */
  protected void scanValueTileDef(ListExpr leTileDef)
  {    
    Reporter.debug("start ScanValueTileDef with list expression: " + f.ListExprToString(leTileDef));

    // Check number and type of list elements,       
    if (leTileDef.listLength() != 2
        || leTileDef.first().atomType() != ListExpr.INT_ATOM
        || leTileDef.second().atomType() != ListExpr.INT_ATOM)
    {
      Reporter.writeError("No correct list expression for tile definition: " + f.ListExprToString(leTileDef));
      err = true;
      return;
    }
    
    int tileSizeX = leTileDef.first().intValue();
    int tileSizeY = leTileDef.second().intValue();    
    this.grid.setTileDefinition(tileSizeX, tileSizeY); 
  }
  
      
  /**
  * Scans the list representation of a sType tile, i.e. a list expression of the format:
  * (int int (Comparable*)) signifying (TileIndexX TileIndexY (Value*))
  */
  protected void scanValueTile(ListExpr leTile)
  {
      Reporter.debug("Parsing tile (partial grid) from list expression: " + f.ListExprToString(leTile));
      
      if (leTile.listLength() != 3
        || leTile.first().atomType() != ListExpr.INT_ATOM
        || leTile.second().atomType() != ListExpr.INT_ATOM)
      {
        Reporter.writeError("No correct list expression for tile: " + f.ListExprToString(leTile));
        err = true;
        return;
      }
            
      // read indices of cell in the tile's lower left corner
      // index signifying x-direction (east/west) from grid origin
      Integer indX = leTile.first().intValue();     
      // index signifying y-direction (north/south) from grid origin
      Integer indY = leTile.second().intValue(); 
      
      Reporter.debug("read cell index east/west: " + indX + " and north/south: " + indY);
            
      //*************************************************************************************
      // scan all cell values in a tile
      //*************************************************************************************
      ListExpr leValues = leTile.third();
      
      int cellsPerTile = this.getGrid().getCellsPerTile();
           
      if (leValues.listLength() != cellsPerTile)
      {
        Reporter.writeError("No correct list expression for tile values (expecting " + cellsPerTile + " values per tile). ");
        err = true;
        return;
      }
      
      Comparable[] tileValues = new Comparable[cellsPerTile];
      
      for (int i=0; i<cellsPerTile; i++)
      {        
        Comparable val = this.scanValueAtom(leValues.first());
        
        if (val != null)
        {
//           Reporter.debug("scanned cell " + i + ", value: " + val);

          tileValues[i] = val;
    
          // update statistics
          int[] offsetX = this.getGrid().getOffsetX();
          int[] offsetY = this.getGrid().getOffsetY();
          int cellCoordX = indX + offsetX[i];
          int cellCoordY = indY + offsetY[i];
          maxCellCoordX = Math.max(maxCellCoordX,cellCoordX);    
          minCellCoordX = Math.min(minCellCoordX,cellCoordX);
          maxCellCoordY = Math.max(maxCellCoordY,cellCoordY);    
          minCellCoordY = Math.min(minCellCoordY,cellCoordY);
          this.updateMinMaxValue(val);
          cellCount++;  
          
//          Reporter.writeInfo("Buffering value " + val + " in cell with key (x, y, sequence no.) (" + indX + "," + indY + ", no " + i + );                   
        }
        
        // get next value
        leValues = leValues.rest();
      }
            
      // store value array and indices of lower left cell in map
      this.setRasterValuesPartial(tileValues, new Point(indX, indY), 0);
  }
  
  /**
  * {@inheritDoc}
  * sType is not a temporal type, thus the time index will always be the same.
  */
  protected int computeTimeIndex(double time)
  {
    return 0;
  }

  
  /**
  *   Get info text.
  */
  @Override
  public String getInfo()
  {
    return super.getInfo();
  }
 
 @Override
 public String toString()
 {
    return this.getAttrName();
 }
}
