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
 * Class for definition of a timed grid.
 */
public class Grid3 extends Grid2
{
  /**
  * Duration of a grid cell layer.
  */
  private double duration;
  
  /**
  * Number of cells in a tile (partial grid).
  */
  private int tileSizeTime = 1;
  
  /**
  * Offset of i-th cell within a tile.
  */
  private int[] offsetTime;
 
  
  public Grid3(double pOriginX, double pOriginY, double pCellLength, double pDuration)
  {
    super(pOriginX, pOriginY, pCellLength);
    this.duration = pDuration;
  }
  
  // TODO
  public void setTileDefinition(int pTileSizeX, int pTileSizeY, int pTileSizeTime)
  {
    super.setTileDefinition(pTileSizeX, pTileSizeY);
    this.tileSizeX = pTileSizeX;
    this.tileSizeY = pTileSizeY;
    this.tileSizeTime = pTileSizeTime;
    this.cellsPerTile = this.tileSizeX * this.tileSizeY;
    
    // pre-compute cell offsets on x, y and time axis depending on cell's sequence number within partial raster list expression
//     this.offsetX = new int[this.cellsPerTile * tileSizeTime];
//     this.offsetY = new int[this.cellsPerTile * tileSizeTime];
    this.offsetTime = new int[this.cellsPerTile * tileSizeTime];
    
    for (int i=0; i<cellsPerTile; i++)
    {
//       this.offsetX[i] = (i / this.tileSizeTime) % this.tileSizeX;
//       this.offsetY[i] = i % this.tileSizeY;
      this.offsetTime[i] = i / (this.tileSizeX * this.tileSizeY);
      
      Reporter.debug("Grid3.setTileDefinition(): computed offsets (X, Y, Time) for sequence no. " 
        + i + ": (" + offsetX[i] + ", " + offsetY[i] + ", " + offsetTime[i] + ")");
    }
    Reporter.debug(this.getInfo());
  }
   
  public double getDuration()
  {
    return this.duration;
  }
  
  public int getTileSizeTime()
  {
    return this.tileSizeTime;
  }
  
  public int[] getOffsetTime()
  {
    return this.offsetTime;
  }
 
  public Integer getOffsetTime(int sequenceNo)
  {
    Integer result = null;
    if (offsetTime != null && sequenceNo < this.tileSizeTime)
    {
      result = offsetTime[sequenceNo];
    }
    return result;
  }
    
  /**
  *   Get info text.
  */
  public String getInfo()
  {
    String eol = System.getProperty("line.separator", "\n");
    StringBuffer sb = new StringBuffer(super.getInfo());
    sb.append("Cells per tile duration: ").append(this.tileSizeTime).append(eol);
    sb.append("Grid interval length (duration): ").append(String.format("%2g", this.duration)).append(eol);
    return sb.toString();
  }
 
 @Override
 public String toString()
 {
    return "Grid3";
 }
}
