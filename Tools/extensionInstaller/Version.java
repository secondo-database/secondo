/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen,
Faculty of Mathematics and Computer Science,
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

*/


/** Class encapsulating version information **/
public class Version{
  int major;
  int minor;
  int subminor;

  public Version(){
     major = 0;
     minor = 0;
     subminor = 0;
  }	  

  public Version(int major, int minor, int subminor){
     this.major = major;
     this.minor = minor;
     this.subminor = subminor;
  }	  

  /** checks whether the versions are equal **/
  public boolean isVersion(int major, int minor, int subminor){
     return (this.major == major) &&
            (this.minor == minor) &&
            (this.subminor == subminor);
  }
  

  /** Checks whether this version is smaller or equal to the given one **/
  public boolean isSmallerOrEqual(int major, int minor, int subminor){
     return (this.major < major) ||
            (this.major==major && this.minor < minor) ||
            (this.major==major && this.minor==minor && this.subminor<=subminor);
  }

  public boolean isSmallerOrEqual(Version v){
     return isSmallerOrEqual(v.major, v.minor, v.subminor);
  }	  
  

  public String toString(){
    return ""+major+"."+minor+"."+subminor;
  }	  
}
