/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

 * JPEGColorSpace.java
 *
 * Created on 11. Februar 2004, 10:22
 */

package viewer.jpeg;

/** Interface defining color-space constants.
 *
 *  @author Stefan Wich
 *  @version 1.0
 */
public interface JPEGColorSpace {
    
    /** This is used in JInfoMetaData to state that color-space isn't yet defined. */
    static final int NO_COLOR_SPACE_YET=-1;
    /** The constant declaring <strong>gray</strong> as the pictures color-space. */
    public static final int COL_SPACE_GRAY = 0;
    /** The constant declaring <strong>RGB</strong> as the pictures color-space. */
    public static final int COL_SPACE_RGB = 1;
    /** The constant declaring <strong>CMYK</strong> as the pictures color-space. */
    public static final int COL_SPACE_CMYK = 2;
    /** Gray scale color model */ 
    public static final int GRAY = 0;
    /** RGB color models Red.*/
    public static final int RED = 0;
    /** RGB color models Green.*/
    public static final int GREEN = 1;
    /** RGB color models Blue.*/
    public static final int BLUE = 2;
    /** CMYK color models Cyan.*/
    public static final int CYAN = 0;
    /** CMYK color models Magenta.*/
    public static final int MAGENTA = 1;
    /** CMYK color models Yellow.*/
    public static final int YELLOW = 2;
    /** CMYK color models Black.*/
    public static final int BLACK =3;
    
}
