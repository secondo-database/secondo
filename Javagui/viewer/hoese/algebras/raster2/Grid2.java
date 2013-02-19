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
 * Basic class for grid definition
 */
public class Grid2
{
  private double originX;
  private double originY;
  
  /**
  * Length of a grid cell.
  * As the cells are square there's only one length.
  */
  private double cellLength;
  
  /**
  * Number of cells in a tile (partial grid).
  */
  protected int tileSizeX = 0;
  protected int tileSizeY = 0;
  protected int cellsPerTile = 0;
  
  /**
  * Offset of i-th cell within a tile.
  */
  protected int[] offsetX;
  protected int[] offsetY;
 
  
  public Grid2(double pOriginX, double pOriginY, double pCellLength)
  {
    this.originX = pOriginX;
    this.originY = pOriginY;
    this.cellLength = pCellLength;
  }
  
  public void setTileDefinition(int pTileSizeX, int pTileSizeY)
  {
    this.tileSizeX = pTileSizeX;
    this.tileSizeY = pTileSizeY;
    this.cellsPerTile = this.tileSizeX*this.tileSizeY;
    
    // pre-compute cell offsets within tile
    offsetX = new int[cellsPerTile];
    offsetY = new int[cellsPerTile];
    for (int i=0; i<cellsPerTile; i++)
    {
      this.offsetX[i] = i % this.tileSizeX;
      this.offsetY[i] = i / this.tileSizeY;
    } 
  }
  
  
  public double getOriginX()
  {
    return this.originX;
  }
  
  public double getOriginY()
  {
    return this.originY;
  }

  public double getCellLength()
  {
    return this.cellLength;
  }
  
  public int getTileSizeX()
  {
    return this.tileSizeX;
  }
  
  public int getTileSizeY()
  {
    return this.tileSizeY;
  }
  
  public int getCellsPerTile()
  {
    return this.cellsPerTile;
  }
  
  public int[] getOffsetX()
  {
    return this.offsetX;
  }
  
  public int[] getOffsetY()
  {
    return this.offsetY;
  }
 
  public Integer getOffsetX(int sequenceNo)
  {
    Integer result = null;
    if (offsetX != null && sequenceNo < this.tileSizeX)
    {
      result = offsetX[sequenceNo];
    }
    return result;
  }
  
  public Integer getOffsetY(int sequenceNo)
  {
    Integer result = null;
    if (offsetY != null && sequenceNo < this.tileSizeY)
    {
      result = offsetY[sequenceNo];
    }
    return result;
  }
  
  protected void setCellsPerTile(int pCells)
  {
    this.cellsPerTile = pCells;
  }

  
  /**
  *   Get info text.
  */
  public String getInfo()
  {
    String eol = System.getProperty("line.separator", "\n");
    StringBuffer sb = new StringBuffer(eol);
    sb.append("Grid origin: (").append(String.format("%2g", this.originX));
    sb.append(", ").append(String.format("%2g", this.originY)).append(")").append(eol);
    sb.append("Grid cell length: ").append(String.format("%2g", this.cellLength)).append(eol);
    sb.append("Cells per tile width: ").append(this.tileSizeX).append(eol);
    sb.append("Cells per tile height: ").append(this.tileSizeY).append(eol);
    return sb.toString();
  }
 
 @Override
 public String toString()
 {
    return "Grid2";
 }
}
