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
 * Basic display class for <CODE>istype</CODE> grid data (i.e. isint, isbool, isreal, isstring).
 * The complete format of the isType list expression is: </br>
 *( (instant) ((GridOriginX GridOriginY Length) (TileSizeX TileSizeY) ( KeyX KeyY (Value*) ))* ) </br>
 *( (instant) ((double double double) (int int) (int int ( Comparable* ) ))* )
 */
public abstract class DisplayRaster2is extends DisplayRaster2s
{
  protected String instantString;

  /**
  * {@inheritDoc}
  * Scans the list representation of a isType datatype:
  *( (instant) ((GridOriginX GridOriginY Length) (TileSizeX TileSizeY) ( KeyX KeyY (Value*) ))* )
  * @param le list representation of a isType datatype
  * @see sj.lang.ListExpr
  * @author Dirk Zacher
  */
  @Override
  protected void ScanValue(ListExpr le)
  {
    StandardFormatter f = new StandardFormatter();

    // Check if the list expression contains anything at all,  
    // if grid and tile dimensions are defined and
    // if list expression contains a list of tiles (possibly empty)
    if(le == null ||
       le.first() == null || 
       le.second() == null)
    {
      Reporter.writeError("No valid list expression for isType: " );
      err = true;
      return;
    }
    
    Dsplinstant displayInstant = new Dsplinstant();
    instantString = displayInstant.getString(le.first());

    super.ScanValue(le.second());

    // set flag so list expression will be scanned only once
    this.isScanned = true;
    
    Reporter.debug("end ScanValue");
  }

  /**
  * {@inheritDoc}
  */
  @Override
  public String getInfo()
  {
    String eol = System.getProperty("line.separator", "\n");
    StringBuffer sb = new StringBuffer(super.getInfo());
    if (this.isScanned)
    {
      sb.append(eol);
      sb.append(this.instantString).append(eol);
    }
    return sb.toString();  
  }
  
  @Override
  public String toString()
  {
    String string = new String(instantString + ": " + getAttrName());
    return string;
  }
}
