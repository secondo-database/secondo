/*
 * JInfoMetaData.java
 *
 * Created on 10. Februar 2004, 21:35
 */

package viewer.jpeg;

import gui.idmanager.ID;
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import javax.swing.*;

/** This class holds meta-data of JInfo <strong>Secondo</strong>-objects.
 *
 *  @author Stefan Wich <mailto:stefan.wich@fernuni-hagen.de>
 */
public class JInfoMetaData extends JPGViewerMetaData implements JPEGColorSpace{

    /** Up to four distribution lists, meaning of sublist values depends on colSpace. */
    private float[][] fDistribution;
    
    /** The color-space of the jpeg. */
    private int colSpace=NO_COLOR_SPACE_YET;
    /** Creates a new instance of JInfoMetaData. 
     *  @param objID ID of the underlying <code>SecondoObject</code> for faster
     *      lookup in the <code>HashTable</code>.
     */
    public JInfoMetaData(ID objID) {
        super(objID);
    }
    
//    /** Set value in distribution list.
//     *  @param value the value to set.
//     *  @param atIdx the index where to set <code>value</code>.
//     *      0 &leq; <code>value</code> &lt; 256.
//     *  @param listIdx the list to insert value.
//     */
//    public void setDistList(float value, int atIdx,int listIdx){
//        assureIndex(atIdx);
//        assureListIndex(listIdx);
//        fDistribution[listIdx][atIdx]=value;
//    }
    /** Set values in distribution list.
     *  @param values an array of 256 integers.
     *  @param listIdx the list to insert value.
     *      Value ranging from 0-{0,2,3} depending on color-space.
     */
    public void setDistList(float[] values, int listIdx){
        if (colSpace==JPEGColorSpace.NO_COLOR_SPACE_YET){
            throw new IllegalStateException("color-space not defined yet. List can't be set.");
        }
        assureIndex(values.length-1, true);
        assureListIndex(listIdx);
        fDistribution[listIdx]=values;
    }
        
    /** Check if the index is wthinin limits.
     *  @param idx the index to check; 0 &leq <code>idx</code> &lt; must hold.
     *  @throws ArrayIndexOutOfBoundsException if either <code>idx</code> &lt; 0 or <code>idx</code> &geq; 256
     */
    private void assureIndex(int idx){
        assureIndex(idx, false);
    }
    /** Check if the index is within limits.
     *  @param idx the index to check; 0 &leq <code>idx</code> &lt; must hold.
     *  @param yieldLast if this arg is <code>true</code> only idx==255 is accepted.
     *  @throws ArrayIndexOutOfBoundsException if index is out of range.
     */
    private void assureIndex(int idx, boolean yieldLast){
        if (idx<0 || idx >=256 || (yieldLast && idx<255)) 
            throw new ArrayIndexOutOfBoundsException("Index ranges from 0 to 255, exactly.");
    }

    /** Check if the index for the list is within limits.
     *  Lower limit is always 0, upper limit depends on color-space 0 for gray-scale
     *  2 for RGB and 3 for CMYK.
     *  @param idx the index to check; 0 &lt;= <code>idx</code> &lt; {0,2,3} must hold.
     *  @throws ArrayIndexOutOfBoundsException if either <code>idx</code> &lt; 0 or <code>idx</code> &gt; {0,2,3}
     */
    private void assureListIndex(int idx){
        int maxId;
        switch(colSpace){
            case COL_SPACE_CMYK:
                maxId=3;
                break;
            case COL_SPACE_RGB:
                maxId=2;
                break;
            case COL_SPACE_GRAY:
                maxId=0;
                break;
            default:
                throw new IllegalStateException("Color-space not defined yet. List can't be set.");
        }
        if (idx<0 || idx>maxId)
            throw new ArrayIndexOutOfBoundsException(
            "Limits of list index violated; low-limit is 0, high-limit is " +
            maxId + ", found " + idx);
    }

    private static final ImageIcon getDistributionIcon(float [] distList, Color whichColor){
        int height = 100;
        float max=0;
        for (int i=0; i<distList.length; i++){
            max=Math.max(max, distList[i]);
        }
        float scaleBy = (max==0.0 ? 1 : height/max);
        BufferedImage distImg = new BufferedImage(256, height, BufferedImage.TYPE_INT_RGB);
        Graphics2D drawMe = distImg.createGraphics();
        int incRed = 1, incGreen = 1, incBlue = 1;
        if(whichColor==Color.GRAY || whichColor==Color.BLACK){
            drawMe.setBackground(Color.ORANGE);
        }else{
            drawMe.setBackground(whichColor);
            if(whichColor==Color.RED){
                incGreen = 0;
                incBlue = 0;
            }
            if(whichColor==Color.GREEN){
                incRed = 0;
                incBlue = 0;
            }
            if(whichColor==Color.BLUE){
                incRed = 0;
                incGreen = 0;
            }
            if(whichColor==Color.CYAN){
                incRed = 0;
            }
            if(whichColor==Color.MAGENTA){
                incGreen = 0;
            }
            if(whichColor==Color.YELLOW){
                incBlue = 0;
            }
        }
        drawMe.clearRect(0,0, 256,height);
        float[] myPaint= new float[3];
        
        for (int i=0;i<distList.length; i++){
            Color.RGBtoHSB(i, i, i, myPaint);
            drawMe.setPaint(Color.getHSBColor(myPaint[0], myPaint[1], myPaint[2]));
            drawMe.draw(new Line2D.Float(i, height, i, height-distList[i]*scaleBy));
        }
        return new ImageIcon(distImg);
    }

    /** Get the color-space. One of {GRAY, RGB, CMYK} given as integer constants 
     *  in JPEGColorSpace interface.
     *  @return the color-space.
     *  @see JPEGColorSpace
     */
    public int getColSpace() {
        return colSpace;
    }
    
    /** Set the color-space.
     *  This will also remove any distribution lists given beforehand.
     *  The corresponding <code>ImageIcon</code>s will also be deleted.
     *  That doesn't affect the JPEGs <code>ImageIcon</code>.
     *  @param colSpace new color-space, one of <code>JPEGColorSpace</code>s constants.
     *  @see JPEGColorSpace
     */
    public void setColSpace(int colSpace) {
        switch (colSpace){
            case JPEGColorSpace.COL_SPACE_GRAY:
                fDistribution = new float[1][];
                resetIcons(2);
                break;
            case JPEGColorSpace.COL_SPACE_RGB:
                fDistribution = new float[3][];
                resetIcons(4);
                break;
            case JPEGColorSpace.COL_SPACE_CMYK:
                fDistribution = new float[4][];
                resetIcons(5);
                break;
            default:
                throw new IllegalArgumentException(colSpace + " is not a valid colorspace argument, see JPEGColorSpace constants.");
        }
        this.colSpace = colSpace;
    }
    
    ImageIcon getNextImgIcon() {
        ImageIcon retValue;
        
        retValue = super.getNextImgIcon();
        String toolTipText = null;
        if (retValue==null){//No such object yet.
            float [] distList = null;
            Color whichColor = null;
            if (colSpace==JPEGColorSpace.COL_SPACE_GRAY){
                distList=fDistribution[0];
                whichColor=Color.GRAY;
                toolTipText = "Brightness distribution. Click for picture.";
            } else {
                distList=fDistribution[getSelIcon()-1];
                if (colSpace==JPEGColorSpace.COL_SPACE_RGB){
                    switch(getSelIcon()){
                        case 1:
                            whichColor=Color.RED;
                            toolTipText = "Color distribution, RED. Click for GREEN.";
                            break;
                        case 2:
                            whichColor=Color.GREEN;
                            toolTipText = "Color distribution, GREEN. Click for BLUE.";
                            break;
                        case 3:
                            whichColor=Color.BLUE;
                            toolTipText = "Color distribution, BLUE. Click for picture.";
                            break;
                        default:
                            throw new IllegalStateException("No image for such a color available");
                    }
                }else if(colSpace==JPEGColorSpace.COL_SPACE_CMYK){
                    switch(getSelIcon()){
                        case 1:
                            whichColor=Color.CYAN;
                            toolTipText = "Color distribution, CYAN. Click for MAGENTA.";
                            break;
                        case 2:
                            whichColor=Color.MAGENTA;
                            toolTipText = "Color distribution, MAGENTA. Click for YELLOW.";
                            break;
                        case 3:
                            whichColor=Color.YELLOW;
                            toolTipText = "Color distribution, YELLOW. Click for BLACK.";
                            break;
                        case 4:
                            whichColor=Color.BLACK;
                            toolTipText = "Color distribution, BLACK. Click for picture.";
                            break;
                        default:
                            throw new IllegalStateException("No image for such a color available");
                    }
                }else
                    throw new IllegalArgumentException(colSpace + " is not a valid colorspace argument, see JPEGColorSpace constants.");
            }
            retValue = getDistributionIcon(distList, whichColor);
            retValue.setDescription(toolTipText);
            storeIcon(retValue);
        }
        
        return retValue;
    }
    
//    public static final ImageIcon getDistributionIcon(float [] distList, JPEGColor color){}
}
