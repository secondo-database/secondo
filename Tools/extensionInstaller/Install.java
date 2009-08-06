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


/** Mainclass for installing plugins **/

public class Install{

/** The main function **/
public static void main(String[] args){
   String secDir = System.getProperty("SECONDO_BUILD_DIR");
   if(secDir==null || secDir.length()==0){
       System.out.println("SECONDO_BUILD_DIR is not defined!");
       System.exit(-1);;
   }
   System.out.println("SECONDO_BUILD_DIR = " + secDir);
   if(args.length<1){
      System.out.println("Missing argument");
      System.exit(-1);
   }
   ExtensionInstaller si = new ExtensionInstaller(secDir);
   if(!args[0].equals("-uninstall")){
      si.installExtensions(args);
   } else {
      if(args.length<2){
          System.out.println("missing argument");
          System.exit(1);
      }
      String[] args2 = new String[args.length-1];
      for(int i=0;i<args2.length;i++){
       args2[i] = args[i+1];
      }
      si.unInstallExtensions(args2);
   }
}

}
