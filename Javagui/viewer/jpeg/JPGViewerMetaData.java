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

package viewer.jpeg;

import gui.idmanager.ID;
import javax.swing.ImageIcon;

/** Class holding the info of a JPEG-algebras object unfolded from ListExpr. 
 *
 *  @author Stefan Wich mailto:stefan.wich@fernuni-hagen.de
 */
public class JPGViewerMetaData {
    
    /** The index of underlying <code>SecondoObject</code> used for hashing.*/
    private final ID objID;
    
    /** An array of images. More usefull in subclasses with more than only the JPEG. */
    private ImageIcon[] imgIcon;
    
    /** The index of the currently displayed image. More usefull in subclasses with more than only the JPEG.*/
    private int selIcon=0;
    
    /** Creates a new instance of JPEGMetaData. 
     *  @param ID take the object ID of underlying <code>SecondoObject</code>.
     */
    public JPGViewerMetaData(ID objID) {
        this.objID = objID;
        imgIcon=new ImageIcon[1];
    }
    
    /** Get the <code>ImageIcon</code> representing the JPEG.
     *  This JPEG can be then embedded e.g. in a <code>JLabel</code> and then be displayed.
     *  @return the JPEG as an <code>ImageIcon</code>.
     */
    public ImageIcon getImgIcon() { return imgIcon[0]; }
    /** Sets the <code>ImageIcon</code> to be displayed within a JLabel. 
     *` @param finalIcon the ImageIcon to be displayed.
     */
    void setImgIcon(ImageIcon finalIcon){ imgIcon[0] = finalIcon; }
    /** Get the next <code>ImageIcon</code> in the icons list.
     *  For the base class this will always return the JPEG.
     *  For subclasses this can give icons with informational images about the
     *  JPEG, e.g. color distributions.
     *  @return the next icon in queue as an <code>ImageIcon</code>.
     */
    ImageIcon getNextImgIcon() {
        if (++selIcon==imgIcon.length) selIcon=0;
        return imgIcon[selIcon];
    }
    /** Clear the icons except the JPEG - if one exists - and create fresh icons array of specified size.
     *  The size must include the JPEG itself. A value below 1 is therefore ignored and replaced by 1.
     *  @param size the size of the new array, the one for the JPEG included.
     */
    void resetIcons(int newSize){
        ImageIcon[] temp = new ImageIcon[Math.max(newSize,1)];
        temp[0] = imgIcon[0];
        imgIcon = temp;
    }

    /** The hashCode is the one for the objID.
     *  Only if the object changes the ID may change.
     *  @return the objID.ID.value a primitive <em>integer</em> of the underlying SecondoObject.
     */
    public int hashCode() { return objID.hashCode(); }
    
    /** Get the index of selected Icon.
     *  With this property subclasses can detect which icon is currently requested.
     *  This info is need after <code>getNextImgIcon</code> yielded <code>null</code>.
     *  In that case the icon must be created (and stored) before dispay.
     *  @return the index of the selected icon.
     */
    protected int getSelIcon() { return selIcon; }
    
    /** The given icon is stored for beeing reused.
     *  This is only usefull for subclasses with more than the JPEG as image,
     *  e.g. for brightness distribution.
     *  @param iIcon the icon to be stored.
     *  @param atIdx where to store the icon, values below 1 and above imgIcon.length
     *      are ignored without notice.
     */
    protected void storeIcon(ImageIcon iIcon){
        if (selIcon>=1 && selIcon<imgIcon.length){
            imgIcon[selIcon]=iIcon;
        }
    }       
}
